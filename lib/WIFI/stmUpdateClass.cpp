#include "stmUpdateClass.h"

#include <FS.h>
#include "stm32Updater.h"
#include "stk500.h"

void STMUpdateClass::setFilename(String filename)
{
    this->filename = filename;
}

bool STMUpdateClass::begin(size_t size, int command, int ledPin, uint8_t ledOn)
{
    clearError();

    /* Remove old file */
    if (SPIFFS.exists(filename.c_str()))
      SPIFFS.remove(filename.c_str());

    FSInfo fs_info;
    if (SPIFFS.info(fs_info))
    {
        String output = "Filesystem: used: ";
        output += fs_info.usedBytes;
        output += " / free: ";
        output += fs_info.totalBytes;

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
    if (fsUploadFile)
    {
        return fsUploadFile.write(data, len);
    }
    return -1;
}

bool STMUpdateClass::end(bool evenIfRemaining)
{
    fsUploadFile.close(); // Close the file again

    int8_t success = flashSTM32(BEGIN_ADDRESS);
    if (success < 0)
    {
        _error = UPDATE_ERROR_WRITE;
    }

    if (SPIFFS.exists(filename.c_str()))
        SPIFFS.remove(filename.c_str());
    return success == 0;
}

void STMUpdateClass::printError(Print &out){
  out.printf_P(PSTR("ERROR[%u]: "), _error);
  if(_error == UPDATE_ERROR_OK){
    out.println(F("No Error"));
  } else if(_error == UPDATE_ERROR_READ){
    out.println(F("SPIFFS Read Failed"));
  } else if(_error == UPDATE_ERROR_WRITE){
    out.println(F("Flash Write Failed"));
  } else if(_error == UPDATE_ERROR_SPACE){
    out.println(F("Not Enough Space"));
  } else {
    out.println(F("UNKNOWN"));
  }
}

int8_t STMUpdateClass::flashSTM32(uint32_t flash_addr)
{
  int8_t result = -1;

  if (filename.endsWith(".elrs")) {
    result = stk500_write_file(filename.c_str());
  } else if (filename.endsWith(".bin")) {
    result = esp8266_spifs_write_file(filename.c_str(), flash_addr);
  } else {

  }
  Serial.begin(460800);
  return result;
}

STMUpdateClass STMUpdate;
