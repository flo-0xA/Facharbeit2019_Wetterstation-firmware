#ifndef __TX23_CUSTOM__
#define __TX23_CUSTOM__

//#include <HardwareSerial.h>

class TX23_Custom
{
   public:
        bool begin(int rx, int tx);
        float readSpeed();
        void readDirection();
};

bool TX23_Custom::begin(int rx, int tx)
{
    // Serial2.begin(4800, SERIAL_8N1, rx, tx);

    // readSpeed();

    return true;
}

float TX23_Custom::readSpeed()
{
    // while (Serial2.available())
    // {
    //     Serial.println(Serial2.read());
    // }
    
    return 0.0;
}

void TX23_Custom::readDirection() {}

#endif