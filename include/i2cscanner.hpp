#include <Arduino.h>
#include <Wire.h>

// Derived from:
//
//    FILE: MultiSpeedI2CScanner.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.16
// PURPOSE: I2C scanner at different speeds
//    DATE: 2013-11-05
//     URL: https://github.com/RobTillaart/MultiSpeedI2CScanner
//     URL: http://forum.arduino.cc/index.php?topic=197360
//

class I2CScanner
{
    private:
        //  FOR INTERNAL I2C BUS NANO 33 BLE
        //  #define WIRE_IMPLEMENT_WIRE1 1
        //  extern TwoWire Wire1;

        TwoWire *wire;

        const char version[7] = "0.1.16";


        //  INTERFACE COUNT (TESTED TEENSY 3.5 AND ARDUINO DUE ONLY)
        int wirePortCount = 1;
        int selectedWirePort = 0;


        //  scans devices from 50 to 800 KHz I2C speeds.
        //  speed lower than 50 and above 400 can cause problems
        long speed[10] = { 100, 200, 300, 400 };
        int speeds;

        int addressStart = 8;
        int addressEnd = 119;


        //  DELAY BETWEEN TESTS
        //  for delay between tests of found devices.
        #ifndef RESTORE_LATENCY
        #define RESTORE_LATENCY       5
        #endif

        bool delayFlag = false;


        //  MINIMIZE OUTPUT
        bool printAll = true;
        bool header = true;
        bool disableIRQ = false;


        //  STATE MACHINE
        enum states {
        STOP, ONCE, CONT, HELP
        };
        states state = STOP;


        //  TIMING
        uint32_t startScan;
        uint32_t stopScan;

        void I2Cscan();
        char getCommand();
        void displayHelp();
        void setSpeed(char sp);
        void reset();
        void setAddress();
        
    public:

        void setup();
        void loop();
};