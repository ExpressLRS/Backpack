#include <Updater.h>
#include <FS.h>

class STMUpdateClass : public UpdaterClass
{
public:
    void setFilename(String filename);
    bool begin(size_t size, int command = U_FLASH, int ledPin = -1, uint8_t ledOn = LOW);
    size_t write(uint8_t *data, size_t len);
    bool end(bool evenIfRemaining = false);
    void printError(Print &out);

private:
    int8_t flashSTM32(uint32_t flash_addr);
    String filename;
    File fsUploadFile;
    uint8_t _error = UPDATE_ERROR_OK;
};

extern STMUpdateClass STMUpdate;
