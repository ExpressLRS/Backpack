#ifdef NAMIMNO_TX_BACKPACK
#include "stmUpdateClass.h"
#endif

class UpdateWrapper {
public:
    void setSTMUpdate(bool stmMode) {
        _stmMode = stmMode;
    }

    bool begin(size_t size) {
        _running = true;
#ifdef NAMIMNO_TX_BACKPACK
        if (_stmMode)
            return STMUpdate.begin(0); // we don't know the size!
#endif
        return Update.begin(size, U_FLASH);
    }

    size_t write(uint8_t *data, size_t len) {
#ifdef NAMIMNO_TX_BACKPACK
        if (_stmMode)
            return STMUpdate.write(data, len);
#endif
        return Update.write(data, len);
    }

    bool end(bool evenIfRemaining = false) {
        _running = false;
#ifdef NAMIMNO_TX_BACKPACK
        if (_stmMode)
            return STMUpdate.end(evenIfRemaining);
#endif
        return Update.end(evenIfRemaining);
    }

    void printError(Print &out) {
#ifdef NAMIMNO_TX_BACKPACK
        if (_stmMode)
            return STMUpdate.printError(out);
#endif
        return Update.printError(out);
    }

    bool hasError() {
#ifdef NAMIMNO_TX_BACKPACK
        if (_stmMode)
            return STMUpdate.hasError();
#endif
        return Update.hasError();
    }

    void runAsync(bool async) {
        if (!_stmMode) Update.runAsync(async);
    }

    bool isRunning() {
        return _running;
    }

private:
    bool _stmMode = false;
    bool _running = false;
};
