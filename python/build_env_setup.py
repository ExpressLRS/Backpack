Import("env", "projenv")
import upload_via_esp8266_backpack
import esp_compress
import ETXinitPassthrough

platform = env.get('PIOPLATFORM', '')

target_name = env['PIOENV'].upper()
print("PLATFORM : '%s'" % platform)
print("BUILD ENV: '%s'" % target_name)

if platform in ['espressif8266']:
    env.AddPostAction("buildprog", esp_compress.compressFirmware)
    env.AddPreAction("${BUILD_DIR}/spiffs.bin",
                     [esp_compress.compress_files])
    env.AddPreAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_files])
    env.AddPostAction("${BUILD_DIR}/${ESP8266_FS_IMAGE_NAME}.bin",
                     [esp_compress.compress_fs_bin])
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=upload_via_esp8266_backpack.on_upload)
    if "_ETX" in target_name:
        env.AddPreAction("upload", ETXinitPassthrough.init_passthrough)

elif platform in ['espressif32']:
    if "_WIFI" in target_name:
        env.Replace(UPLOAD_PROTOCOL="custom")
        env.Replace(UPLOADCMD=upload_via_esp8266_backpack.on_upload)

if "_WIFI" in target_name:
    if "_TX_" in target_name:
        env.SetDefault(UPLOAD_PORT="elrs_txbp.local")
    else:
        env.SetDefault(UPLOAD_PORT="elrs_vrx.local")
