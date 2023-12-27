Import("env", "projenv")
import os
import shutil
import upload_via_esp8266_backpack
import esp_compress
import UnifiedConfiguration

platform = env.get('PIOPLATFORM', '')

target_name = env['PIOENV'].upper()
print("PLATFORM : '%s'" % platform)
print("BUILD ENV: '%s'" % target_name)

def copy_bootfile(source, target, env):
    FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
    shutil.copyfile(FRAMEWORK_DIR + "/tools/partitions/boot_app0.bin", env.subst("$BUILD_DIR") + "/boot_app0.bin")

if platform in ['espressif8266']:
    env.AddPreAction("${BUILD_DIR}/spiffs.bin",
                     [esp_compress.compress_files])
    env.AddPreAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_files])
    env.AddPostAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_fs_bin])
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=upload_via_esp8266_backpack.on_upload)

elif platform in ['espressif32']:
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=upload_via_esp8266_backpack.on_upload)
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_bootfile)

if "_WIFI" in target_name:
    if "_TX_" in target_name:
        env.SetDefault(UPLOAD_PORT="elrs_txbp.local")
    else:
        env.SetDefault(UPLOAD_PORT="elrs_vrx.local")

# Remove stale binary so the platform is forced to build a new one and attach options/hardware-layout files
try:
    os.remove(env['PROJECT_BUILD_DIR'] + '/' + env['PIOENV'] +'/'+ env['PROGNAME'] + '.bin')
except FileNotFoundError:
    None
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", UnifiedConfiguration.appendConfiguration)
if platform in ['espressif8266'] and "_WIFI" in target_name:
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", esp_compress.compressFirmware)
