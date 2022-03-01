#include <Arduino.h>
#include "device.h"

#if defined(PLATFORM_ESP8266) || defined(PLATFORM_ESP32)

#if defined(PLATFORM_ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Update.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define wifi_mode_t WiFiMode_t
#endif
#include <DNSServer.h>

#include <set>
#include <StreamString.h>

#include <ESPAsyncWebServer.h>

#include "common.h"
#include "logging.h"
#include "options.h"
#include "helpers.h"

#include "UpdateWrapper.h"

#include "WebContent.h"

#include "config.h"
#if defined(TARGET_VRX_BACKPACK)
extern VrxBackpackConfig config;
#else
extern TxBackpackConfig config;
#endif
extern unsigned long rebootTime;

#define QUOTE(arg) #arg
#define STR(macro) QUOTE(macro)

#if defined(TARGET_VRX_BACKPACK)
static const char *myHostname = "elrs_vrx";
static const char *wifi_ap_ssid = "ExpressLRS VRx Backpack";
#elif defined(TARGET_TX_BACKPACK)
static const char *myHostname = "elrs_txbp";
static const char *wifi_ap_ssid = "ExpressLRS TX Backpack";
#else
#error Unknown target
#endif
static const char *wifi_ap_password = "expresslrs";

static const char *home_wifi_ssid = ""
#ifdef HOME_WIFI_SSID
STR(HOME_WIFI_SSID)
#endif
;
static const char *home_wifi_password = ""
#ifdef HOME_WIFI_PASSWORD
STR(HOME_WIFI_PASSWORD)
#endif
;

static char station_ssid[33];
static char station_password[65];

static bool wifiStarted = false;
bool webserverPreventAutoStart = false;
extern bool InBindingMode;

static wl_status_t laststatus = WL_IDLE_STATUS;
static volatile WiFiMode_t wifiMode = WIFI_OFF;
static volatile WiFiMode_t changeMode = WIFI_OFF;
static volatile unsigned long changeTime = 0;

static const byte DNS_PORT = 53;
static IPAddress apIP(10, 0, 0, 1);
static IPAddress netMsk(255, 255, 255, 0);
static DNSServer dnsServer;

static AsyncWebServer server(80);
static bool servicesStarted = false;

static bool target_seen = false;
static uint8_t target_pos = 0;
static String target_found;
static bool target_complete = false;
static bool force_update = false;
static bool do_flash = false;
static uint32_t totalSize;
static UpdateWrapper updater = UpdateWrapper();

static AsyncEventSource logging("/logging");
static char logBuffer[256];
static int logPos = 0;

/** Is this an IP? */
static boolean isIp(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9'))
    {
      return false;
    }
  }
  return true;
}

/** IP to String? */
static String toStringIp(IPAddress ip)
{
  String res = "";
  for (int i = 0; i < 3; i++)
  {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

static bool captivePortal(AsyncWebServerRequest *request)
{
  extern const char *myHostname;

  if (!isIp(request->host()) && request->host() != (String(myHostname) + ".local"))
  {
    DBGLN("Request redirected to captive portal");
    request->redirect(String("http://") + toStringIp(request->client()->localIP()));
    return true;
  }
  return false;
}

static struct {
  const char *url;
  const char *contentType;
  const uint8_t* content;
  const size_t size;
} files[] = {
  {"/mui.css", "text/css", (uint8_t *)MUI_CSS, sizeof(MUI_CSS)},
  {"/elrs.css", "text/css", (uint8_t *)ELRS_CSS, sizeof(ELRS_CSS)},
  {"/mui.js", "text/javascript", (uint8_t *)MUI_JS, sizeof(MUI_JS)},
  {"/scan.js", "text/javascript", (uint8_t *)SCAN_JS, sizeof(SCAN_JS)},
  {"/logo.svg", "image/svg+xml", (uint8_t *)LOGO_SVG, sizeof(LOGO_SVG)},
  {"/log.js", "text/javascript", (uint8_t *)LOG_JS, sizeof(LOG_JS)},
  {"/log.html", "text/html", (uint8_t *)LOG_HTML, sizeof(LOG_HTML)},
};

static void WebUpdateSendContent(AsyncWebServerRequest *request)
{
  for (size_t i=0 ; i<ARRAY_SIZE(files) ; i++) {
    if (request->url().equals(files[i].url)) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, files[i].contentType, files[i].content, files[i].size);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
      return;
    }
  }
  request->send(404, "text/plain", "File not found");
}

static void WebUpdateHandleRoot(AsyncWebServerRequest *request)
{
  if (captivePortal(request))
  { // If captive portal redirect instead of displaying the page.
    return;
  }
  force_update = request->hasArg("force");
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", (uint8_t*)INDEX_HTML, sizeof(INDEX_HTML));
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

static void WebUpdateSendMode(AsyncWebServerRequest *request)
{
  String s;
  if (wifiMode == WIFI_STA) {
    s = String("{\"mode\":\"STA\",\"ssid\":\"") + station_ssid;
  } else {
    s = String("{\"mode\":\"AP\",\"ssid\":\"") + station_ssid;
  }
  #if defined(NAMIMNO_TX_BACKPACK)
  s += "\",\"stm32\":\"yes";
  #endif
  #if defined(TARGET_RX)
  s += "\",\"modelid\":\"" + String(config.GetModelId());
  #endif
  s += "\"}";
  request->send(200, "application/json", s);
}

static void WebUpdateGetTarget(AsyncWebServerRequest *request)
{
  String s = String("{\"target\":\"") + (const char *)&target_name[4] + "\",\"version\": \"" + VERSION + "\"}";
  request->send(200, "application/json", s);
}

static void WebUpdateSendNetworks(AsyncWebServerRequest *request)
{
  int numNetworks = WiFi.scanComplete();
  if (numNetworks >= 0) {
    DBGLN("Found %d networks", numNetworks);
    std::set<String> vs;
    String s="[";
    for(int i=0 ; i<numNetworks ; i++) {
      String w = WiFi.SSID(i);
      DBGLN("found %s", w.c_str());
      if (vs.find(w)==vs.end() && w.length()>0) {
        if (!vs.empty()) s += ",";
        s += "\"" + w + "\"";
        vs.insert(w);
      }
    }
    s+="]";
    request->send(200, "application/json", s);
  } else {
    request->send(204, "application/json", "[]");
  }
}

static void sendResponse(AsyncWebServerRequest *request, const String &msg, WiFiMode_t mode) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", msg);
  response->addHeader("Connection", "close");
  request->send(response);
  request->client()->close();
  changeTime = millis();
  changeMode = mode;
}

static void WebUpdateAccessPoint(AsyncWebServerRequest *request)
{
  DBGLN("Starting Access Point");
  String msg = String("Access Point starting, please connect to access point '") + wifi_ap_ssid + "' with password '" + wifi_ap_password + "'";
  sendResponse(request, msg, WIFI_AP);
}

static void WebUpdateConnect(AsyncWebServerRequest *request)
{
  DBGLN("Connecting to home network");
  String msg = String("Connecting to network '") + station_ssid + "', connect to http://" +
    myHostname + ".local from a browser on that network";
  sendResponse(request, msg, WIFI_STA);
}

static void WebUpdateSetHome(AsyncWebServerRequest *request)
{
  String ssid = request->arg("network");
  String password = request->arg("password");

  DBGLN("Setting home network %s", ssid.c_str());
  strcpy(station_ssid, ssid.c_str());
  strcpy(station_password, password.c_str());
  // Only save to config if we don't have a flashed wifi network
  if (home_wifi_ssid[0] == 0) {
    config.SetSSID(ssid.c_str());
    config.SetPassword(password.c_str());
    config.Commit();
  }
  WebUpdateConnect(request);
}

static void WebUpdateForget(AsyncWebServerRequest *request)
{
  DBGLN("Forget home network");
  config.SetSSID("");
  config.SetPassword("");
  config.Commit();
  // If we have a flashed wifi network then let's try reconnecting to that otherwise start an access point
  if (home_wifi_ssid[0] != 0) {
    strcpy(station_ssid, home_wifi_ssid);
    strcpy(station_password, home_wifi_password);
    String msg = String("Temporary network forgotten, attempting to connect to network '") + station_ssid + "'";
    sendResponse(request, msg, WIFI_STA);
  }
  else {
    station_ssid[0] = 0;
    station_password[0] = 0;
    String msg = String("Home network forgotten, please connect to access point '") + wifi_ap_ssid + "' with password '" + wifi_ap_password + "'";
    sendResponse(request, msg, WIFI_AP);
  }
}

static void WebUpdateHandleNotFound(AsyncWebServerRequest *request)
{
  if (captivePortal(request))
  { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += String(F(" ")) + request->argName(i) + F(": ") + request->arg(i) + F("\n");
  }
  AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  request->send(response);
}

static void WebUploadResponseHandler(AsyncWebServerRequest *request) {
  if (updater.hasError()) {
    StreamString p = StreamString();
    updater.printError(p);
    p.trim();
    DBGLN("Failed to upload firmware: %s", p.c_str());
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", String("{\"status\": \"error\", \"msg\": \"") + p + "\"}");
    response->addHeader("Connection", "close");
    request->send(response);
    request->client()->close();
  } else {
    if (target_seen) {
      DBGLN("Update complete, rebooting");
      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\": \"ok\", \"msg\": \"Update complete. Please wait for LED to turn on before disconnecting power.\"}");
      response->addHeader("Connection", "close");
      request->send(response);
      request->client()->close();
      do_flash = true;
    } else {
      String message = String("{\"status\": \"mismatch\", \"msg\": \"<b>Current target:</b> ") + (const char *)&target_name[4] + ".<br>";
      if (target_found.length() != 0) {
        message += "<b>Uploaded image:</b> " + target_found + ".<br/>";
      }
      message += "<br/>Flashing the wrong firmware may lock or damage your device.\"}";
      request->send(200, "application/json", message);
    }
  }
}

static void WebUploadDataHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (index == 0) {
    DBGLN("Update: %s, %s", filename.c_str(), request->arg("type").c_str());
    target_seen = false;
    target_found.clear();
    target_complete = false;
    target_pos = 0;
    totalSize = 0;
    #ifdef NAMIMNO_TX_BACKPACK
      if (request->arg("type").equals("tx"))
      {
        DBGLN("Set updater to STM");
        updater.setSTMUpdate(true);
        target_seen = true;     // TODO get target_name from TX so we know what we're flashing
        STMUpdate.setFilename(filename);
      }
      else
      {
        DBGLN("Set updater to ESP");
        updater.setSTMUpdate(false);
      }
    #endif
    #if defined(PLATFORM_ESP8266)
      updater.runAsync(true);
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      DBGLN("Free space = %u", maxSketchSpace);
      if (!updater.begin(maxSketchSpace)){ //start with max available size
        updater.printError(Serial);
      }
    #else
      if (!updater.begin()) { //start with max available size
        updater.printError(Serial);
      }
    #endif
  }
  if (len) {
    DBGVLN("writing %d", len);
    if (updater.write(data, len) == len) {
      if (force_update || (totalSize == 0 && *data == 0x1F)) // forced or gzipped image, we can't check
        target_seen = true;
      if (!target_seen) {
        for (size_t i=0 ; i<len ;i++) {
          if (!target_complete && (target_pos >= 4 || target_found.length() > 0)) {
            if (target_pos == 4) {
              target_found.clear();
            }
            if (data[i] == 0 || target_found.length() > 50) {
              target_complete = true;
            }
            else {
              target_found += (char)data[i];
            }
          }
          if (data[i] == target_name[target_pos]) {
            ++target_pos;
            if (target_pos >= target_name_size) {
              target_seen = true;
            }
          }
          else {
            target_pos = 0; // Startover
          }
        }
      }
      totalSize += len;
    }
  }
}

static void WebUploadForceUpdateHandler(AsyncWebServerRequest *request) {
  target_seen = true;
  if (request->arg("action").equals("confirm")) {
    WebUploadResponseHandler(request);
  }
  else {
    #if defined(PLATFORM_ESP32)
      updater.abort();
    #endif
    request->send(200, "application/json", "{\"status\": \"ok\", \"msg\": \"Update cancelled\"}");
  }
}

static void wifiOff()
{
  wifiStarted = false;
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  #if defined(PLATFORM_ESP8266)
  WiFi.forceSleepBegin();
  #endif
}

static void startWiFi(unsigned long now)
{
  if (wifiStarted) {
    return;
  }
  if (connectionState < FAILURE_STATES) {
    connectionState = wifiUpdate;
  }

  INFOLN("Begin Webupdater");

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  #if defined(PLATFORM_ESP8266)
    WiFi.setOutputPower(13);
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  #elif defined(PLATFORM_ESP32)
    WiFi.setTxPower(WIFI_POWER_13dBm);
  #endif
  if (home_wifi_ssid[0] != 0) {
    strcpy(station_ssid, home_wifi_ssid);
    strcpy(station_password, home_wifi_password);
  }
  else {
    strcpy(station_ssid, config.GetSSID());
    strcpy(station_password, config.GetPassword());
  }
  if (station_ssid[0] == 0) {
    changeTime = now;
    changeMode = WIFI_AP;
  }
  else {
    changeTime = now;
    changeMode = WIFI_STA;
  }
  laststatus = WL_DISCONNECTED;
  wifiStarted = true;
}

static void startMDNS()
{
  if (!MDNS.begin(myHostname))
  {
    DBGLN("Error starting mDNS");
    return;
  }

  String instance = String(myHostname) + "_" + WiFi.macAddress();
  instance.replace(":", "");
  #ifdef PLATFORM_ESP8266
    // We have to do it differently on ESP8266 as setInstanceName has the side-effect of chainging the hostname!
    MDNS.setInstanceName(myHostname);
    MDNSResponder::hMDNSService service = MDNS.addService(instance.c_str(), "http", "tcp", 80);
    MDNS.addServiceTxt(service, "vendor", "elrs");
    MDNS.addServiceTxt(service, "target", (const char *)&target_name[4]);
    MDNS.addServiceTxt(service, "version", VERSION);
    MDNS.addServiceTxt(service, "options", String(FPSTR(compile_options)).c_str());
    #if defined(TARGET_VRX_BACKPACK)
      MDNS.addServiceTxt(service, "type", "vrx");
    #elif defined(TARGET_TX_BACKPACK)
      MDNS.addServiceTxt(service, "type", "txbp");
    #endif
    // If the probe result fails because there is another device on the network with the same name
    // use our unique instance name as the hostname. A better way to do this would be to use
    // MDNSResponder::indexDomain and change wifi_hostname as well.
    MDNS.setHostProbeResultCallback([instance](const char* p_pcDomainName, bool p_bProbeResult) {
      if (!p_bProbeResult) {
        WiFi.hostname(instance);
        MDNS.setInstanceName(instance);
      }
    });
  #else
    MDNS.setInstanceName(instance);
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "vendor", "elrs");
    MDNS.addServiceTxt("http", "tcp", "target", (const char *)&target_name[4]);
    MDNS.addServiceTxt("http", "tcp", "version", VERSION);
    MDNS.addServiceTxt("http", "tcp", "options", String(FPSTR(compile_options)).c_str());
    #if defined(TARGET_VRX_BACKPACK)
       MDNS.addServiceTxt("http", "tcp", "type", "vrx");
    #elif defined(TARGET_TX_BACKPACK)
       MDNS.addServiceTxt("http", "tcp", "type", "txbp");
    #endif
  #endif
}

static void startServices()
{
  if (servicesStarted) {
    #if defined(PLATFORM_ESP32)
      MDNS.end();
      startMDNS();
    #endif
    return;
  }

  server.on("/", WebUpdateHandleRoot);
  server.on("/mui.css", WebUpdateSendContent);
  server.on("/elrs.css", WebUpdateSendContent);
  server.on("/mui.js", WebUpdateSendContent);
  server.on("/scan.js", WebUpdateSendContent);
  server.on("/logo.svg", WebUpdateSendContent);
  server.on("/mode.json", WebUpdateSendMode);
  server.on("/networks.json", WebUpdateSendNetworks);
  server.on("/sethome", WebUpdateSetHome);
  server.on("/forget", WebUpdateForget);
  server.on("/connect", WebUpdateConnect);
  server.on("/access", WebUpdateAccessPoint);
  server.on("/target", WebUpdateGetTarget);

  server.on("/generate_204", WebUpdateHandleRoot); // handle Andriod phones doing shit to detect if there is 'real' internet and possibly dropping conn.
  server.on("/gen_204", WebUpdateHandleRoot);
  server.on("/library/test/success.html", WebUpdateHandleRoot);
  server.on("/hotspot-detect.html", WebUpdateHandleRoot);
  server.on("/connectivity-check.html", WebUpdateHandleRoot);
  server.on("/check_network_status.txt", WebUpdateHandleRoot);
  server.on("/ncsi.txt", WebUpdateHandleRoot);
  server.on("/fwlink", WebUpdateHandleRoot);

  server.on("/update", HTTP_POST, WebUploadResponseHandler, WebUploadDataHandler);
  server.on("/forceupdate", WebUploadForceUpdateHandler);

  server.on("/log.js", WebUpdateSendContent);
  server.on("/log.html", WebUpdateSendContent);
  server.addHandler(&logging);

  server.onNotFound(WebUpdateHandleNotFound);

  server.begin();

  dnsServer.start(DNS_PORT, "*", apIP);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

  startMDNS();

  servicesStarted = true;
  DBGLN("HTTPUpdateServer ready! Open http://%s.local in your browser", myHostname);
}

static void HandleWebUpdate()
{
  unsigned long now = millis();
  wl_status_t status = WiFi.status();
  if (status != laststatus && wifiMode == WIFI_STA) {
    DBGLN("WiFi status %d", status);
    switch(status) {
      case WL_NO_SSID_AVAIL:
      case WL_CONNECT_FAILED:
      case WL_CONNECTION_LOST:
        changeTime = now;
        changeMode = WIFI_AP;
        break;
      case WL_DISCONNECTED: // try reconnection
        changeTime = now;
        break;
      default:
        break;
    }
    laststatus = status;
  }
  if (status != WL_CONNECTED && wifiMode == WIFI_STA && (now - changeTime) > 30000) {
    changeTime = now;
    changeMode = WIFI_AP;
    DBGLN("Connection failed %d", status);
  }
  if (changeMode != wifiMode && changeMode != WIFI_OFF && (now - changeTime) > 500) {
    switch(changeMode) {
      case WIFI_AP:
        DBGLN("Changing to AP mode");
        WiFi.disconnect();
        wifiMode = WIFI_AP;
        #if defined(PLATFORM_ESP8266)
          WiFi.mode(WIFI_AP_STA);
        #else
          WiFi.mode(WIFI_AP);
        #endif
        changeTime = now;
        WiFi.softAPConfig(apIP, apIP, netMsk);
        WiFi.softAP(wifi_ap_ssid, wifi_ap_password);
        WiFi.scanNetworks(true);
        startServices();
        break;
      case WIFI_STA:
        DBGLN("Connecting to home network '%s' '%s'", station_ssid, station_password);
        wifiMode = WIFI_STA;
        WiFi.mode(wifiMode);
        WiFi.setHostname(myHostname); // hostname must be set after the mode is set to STA
        changeTime = now;
        WiFi.begin(station_ssid, station_password);
        startServices();
      default:
        break;
    }
    #if defined(PLATFORM_ESP8266)
      MDNS.notifyAPChange();
    #endif
    changeMode = WIFI_OFF;
  }

  if (servicesStarted)
  {
    while (Serial.available()) {
      int val = Serial.read();
      logBuffer[logPos++] = val;
      logBuffer[logPos] = 0;
      if (val == '\n' || logPos == sizeof(logBuffer)-1) {
        logging.send(logBuffer);
        logPos = 0;
      }
    }

    dnsServer.processNextRequest();
    #if defined(PLATFORM_ESP8266)
      MDNS.update();
    #endif
    // When in STA mode, a small delay reduces power use from 90mA to 30mA when idle
    // In AP mode, it doesn't seem to make a measurable difference, but does not hurt
    if (!updater.isRunning())
      delay(1);
    if (do_flash) {
      do_flash = false;
      if (updater.end(true)) { //true to set the size to the current progress
        DBGLN("Upload Success: %ubytes\nPlease wait for LED to turn on before disconnecting power", totalSize);
      } else {
        updater.printError(Serial);
      }
      rebootTime = millis() + 200;
    }
  }
}

static int start()
{
  return DURATION_NEVER;
}

static int event()
{
  if (connectionState == wifiUpdate || connectionState > FAILURE_STATES)
  {
    if (!wifiStarted) {
      startWiFi(millis());
      return DURATION_IMMEDIATELY;
    }
  }
  return DURATION_IGNORE;
}

static int timeout()
{
  if (wifiStarted)
  {
    HandleWebUpdate();
    return DURATION_IMMEDIATELY;
  }
  return DURATION_NEVER;
}

device_t WIFI_device = {
  .initialize = wifiOff,
  .start = start,
  .event = event,
  .timeout = timeout
};

#endif
