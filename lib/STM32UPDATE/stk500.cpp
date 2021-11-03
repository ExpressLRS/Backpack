#include "stk500.h"
#include "stm32Updater.h"
#include <Arduino.h>
#include <FS.h>
#include "logging.h"

uint8_t stk_data_buff[STK_PAGE_SIZE];

int sync_get(void);
int prog_params_set(void);
int prog_params_ext_set(void);
int prog_mode_enter(void);
int prog_mode_exit(void);
int prog_flash(uint32_t offset, uint8_t* data, uint32_t length);
int command_send(uint8_t cmd, uint8_t* params, size_t count, uint32_t timeout = STK_TIMEOUT);
int verify_sync(uint32_t timeout = STK_TIMEOUT);

void serial_empty_rx(void)
{
    while(Serial.available())
        Serial.read();
}

const __FlashStringHelper *stk500_write_file(const char *filename)
{
    uint8_t sync_iter;
    if (!SPIFFS.exists(filename)) {
        return F("file does not exist!");
    }
    File fp = SPIFFS.open(filename, "r");
    uint32_t filesize = fp.size();
    DBGLN("filesize: %d", filesize);
    if (FLASH_SIZE < filesize) {
        fp.close();
        return F("file is too big!");
    }

    // Put MCU to reset
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);
    delay(100);
    // Init UART
    Serial.begin(STK_BAUD_RATE);
    serial_empty_rx();
    // Release reset
    digitalWrite(RESET_PIN, HIGH);
    //pinMode(RESET_PIN, INPUT);

    // Try to get sync...
    for (sync_iter = 0; sync_iter < STK_SYNC_CTN; sync_iter++) {
        DBGLN("Waiting sync...");
        if (0 <= sync_get()) {
            break;
        }
    }
    if (STK_SYNC_CTN <= sync_iter) {
        fp.close();
        return F("no sync! stopping");
    }
    DBGLN("Sync OK");

#if 0 // These are not used at the moment
    if (prog_params_set() < 0) {
        return F("prog params!");
    }
    if (prog_params_ext_set() < 0) {
        return F("prog params!");
    }
    if (prog_mode_enter() < 0) {
        return F("enter prog mode!");
    }
#endif

    DBGLN("begin to write.");
    int result;
    while (fp.position() < filesize) {
        uint8_t nread = sizeof(stk_data_buff);
        if ((filesize - fp.position()) < sizeof(stk_data_buff)) {
            nread = filesize - fp.position();
        }
        nread = fp.readBytes((char *)stk_data_buff, nread);
        result = prog_flash((fp.position() - nread), stk_data_buff, nread);
        if (result < 0) {
            fp.close();
            return F("write failed.");
        }
    }
    fp.close();

    if (prog_mode_exit() < 0) {
        // Seems we don't get the response!
        // return F("exit prog mode!");
    }

    DBGLN("write succeeded.");
    return NULL;
}

int sync_get(void)
{
    serial_empty_rx(); // clean rx buffer
    return command_send(STK_GET_SYNC, NULL, 0, STK_SYNC_TIMEOUT);
}

int prog_params_set(void)
{
    uint8_t params[] = {
        0x86, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0xff, 0xff,
        0xff, 0xff, 0x00, 0x80, 0x04, 0x00, 0x00, 0x00, 0x80, 0x00};
    return command_send(STK_SET_DEVICE, params, sizeof(params));
}

int prog_params_ext_set(void)
{
    uint8_t params[] = {0x05, 0x04, 0xd7, 0xc2, 0x00};
    return command_send(STK_SET_DEVICE_EXT, params, sizeof(params));
}

int prog_mode_enter(void)
{
    return command_send(STK_ENTER_PROGMODE, NULL, 0);
}

int prog_mode_exit(void)
{
    return command_send(STK_LEAVE_PROGMODE, NULL, 0);
}

int prog_flash(uint32_t offset, uint8_t* data, uint32_t length)
{
    offset = (offset + 1) >> 1; // convert to words

    uint8_t header[] = {STK_PROG_PAGE,
        (uint8_t)(length >> 8), (uint8_t)length, 0x46};

    // Send write address
    uint8_t addr[] = {(uint8_t)offset, (uint8_t)(offset >> 8)};
    if (command_send(STK_LOAD_ADDRESS, addr, sizeof(addr)) < 0) {
        DBGLN("[ERROR] addr set!");
        return -1;
    }

    // Send data
    Serial.write(header, sizeof(header));
    Serial.write(data, length);
    Serial.write(CRC_EOP);

    return verify_sync();
}

int command_send(uint8_t const cmd, uint8_t* params, size_t const count, uint32_t timeout)
{
    Serial.write(cmd);
    if (params && count) {
        Serial.write(params, count);
    }
    Serial.write(CRC_EOP);
    return verify_sync(timeout);
}

int wait_data_timeout(int const count, uint32_t timeout)
{
    uint32_t const start = millis();
    while ((millis() - start) < timeout) {
        ESP.wdtFeed();
        if (count <= Serial.available()) {
            return 0;
        }
    }
    return -1;
}

int verify_sync(uint32_t timeout)
{
    if ( 0 <= wait_data_timeout(2, timeout)) {
        uint8_t sync = Serial.read();
        uint8_t ok = Serial.read();
        if (sync == STK_INSYNC && ok == STK_OK) {
            return 0;
        }
    }
    DBGLN("[ERROR] sync fail...");
    return -1;
}
