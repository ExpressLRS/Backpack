#pragma once

#include <Updater.h>
#include <FS.h>

class STMUpdateClass
{
public:
    STMUpdateClass() {
        SPIFFS.begin();
    }
    void setFilename(const String& filename);
    bool begin(size_t size);
    size_t write(uint8_t *data, size_t len);
    bool end(bool evenIfRemaining = false);
    void printError(Print &out);
    bool hasError() { return _error != UPDATE_ERROR_OK; }

private:
    int8_t flashSTM32(uint32_t flash_addr);
    String filename;
    File fsUploadFile;
    uint8_t _error = UPDATE_ERROR_OK;
    const __FlashStringHelper *_errmsg = NULL;
};

extern STMUpdateClass STMUpdate;
