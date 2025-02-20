
#include "ArduinoOTA.h"
#include "webserial.hpp"

class ota_handler
{
    private:
        bool ota_running = false;
        unsigned long ota_timer = 0;
        int last_perc = 0;
        
        void onStart();
        void onEnd();
        void onProgress(unsigned int progress, unsigned int total);
        void onError(ota_error_t error);

    public:
        bool OTAOnly = false;

        void Start();
        void Loop();
};