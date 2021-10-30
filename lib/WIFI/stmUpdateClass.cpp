#include "stmUpdateClass.h"

#include <FS.h>
#include "stm32Updater.h"
#include "stk500.h"

#include "logging.h"

void STMUpdateClass::setFilename(const String& filename)
{
    this->filename = filename;
}

bool STMUpdateClass::begin(size_t size)
{
    _error = UPDATE_ERROR_OK;

    /* Remove old file */
    if (SPIFFS.exists(filename.c_str()))
      SPIFFS.remove(filename.c_str());

    FSInfo fs_info;
    if (SPIFFS.info(fs_info))
    {
        if (fs_info.usedBytes > 0) {
            SPIFFS.format();
            if (!SPIFFS.info(fs_info))
            {
                _error = UPDATE_ERROR_READ;
                return false;
            }
        }
        if (fs_info.totalBytes < size)
        {
            _error = UPDATE_ERROR_SPACE;
            return false;
        }
    }
    else
    {
        _error = UPDATE_ERROR_READ;
        return false;
    }
    fsUploadFile = SPIFFS.open(filename.c_str(), "w");      // Open the file for writing in SPIFFS (create if it doesn't exist)
    return true;
}

size_t STMUpdateClass::write(uint8_t *data, size_t len)
{
    return fsUploadFile.write(data, len);
}

bool STMUpdateClass::end(bool evenIfRemaining)
{
    fsUploadFile.close(); // Close the file again

    _error = flashSTM32(BEGIN_ADDRESS);
    if (SPIFFS.exists(filename.c_str()))
        SPIFFS.remove(filename.c_str());
    return !hasError();
}

void STMUpdateClass::printError(Print &out){
  if(_error == UPDATE_ERROR_OK){
    out.println(F("No Error"));
  } else if(_error == UPDATE_ERROR_READ){
    out.println(F("ERROR: SPIFFS Read Failed"));
  } else if(_error == UPDATE_ERROR_WRITE){
    out.println(F("ERROR: Flash Write Failed"));
  } else if(_error == UPDATE_ERROR_SPACE){
    out.println(F("ERROR: Not Enough Space"));
  } else {
    out.print(F("ERROR: "));
    out.println(_errmsg);
  }
}

int8_t STMUpdateClass::flashSTM32(uint32_t flash_addr)
{
  if (filename.endsWith(".elrs")) {
    _errmsg = stk500_write_file(filename.c_str());
  } else if (filename.endsWith(".bin")) {
    _errmsg = esp8266_spiffs_write_file(filename.c_str(), flash_addr);
  }
  Serial.begin(460800);
  if (_errmsg != NULL)
    return UPDATE_ERROR_NO_DATA;
  return UPDATE_ERROR_STREAM;
}

STMUpdateClass STMUpdate;
