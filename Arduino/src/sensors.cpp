#include <Arduino.h>
#include "sensors.h"
#include "Arduino_LSM9DS1.h"
#include "Arduino_HTS221.h"
#include "Arduino_APDS9960.h"
#include "Arduino_LPS22HB.h"

using namespace rtos;

static unsigned char sensors_threadStack[4096];
static Thread sensors_thread(osPriorityAboveNormal1, sizeof(sensors_threadStack), sensors_threadStack, "sensors Thread");

void sensors_thread_func()  
{

    while (!Serial);

    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
    }

    if (!HTS.begin()) {
        Serial.println("Failed to initialize humidity temperature sensor!");
    }

    if (!APDS.begin()) {
        Serial.println("Error initializing APDS9960 sensor.");
    }

    if (!BARO.begin()) {
        Serial.println("Failed to initialize pressure sensor!");
    }

    float x, y, z;
    float temperature;
    float humidity;
    int proximity = 255;
    float pressure;

    String sensorReadingsStr;

    while (1)
    {    
        if (IMU.accelerationAvailable()) {

            IMU.readAcceleration(x, y, z);

            sensorReadingsStr += "ax: ";
            sensorReadingsStr += x;
            sensorReadingsStr += '\t';
            sensorReadingsStr += "ay: ";
            sensorReadingsStr += y;
            sensorReadingsStr += '\t';
            sensorReadingsStr += "az: ";
            sensorReadingsStr += z;
            sensorReadingsStr += '\t';
        }

        temperature = HTS.readTemperature(CELSIUS);
        humidity    = HTS.readHumidity();

        sensorReadingsStr += "temp: ";
        sensorReadingsStr += temperature;
        sensorReadingsStr += '\t';

        sensorReadingsStr += "humi: ";
        sensorReadingsStr += humidity;
        sensorReadingsStr += '\t';

        if (APDS.proximityAvailable())
        {
            proximity = APDS.readProximity();
        }

        sensorReadingsStr += "prox: ";
        sensorReadingsStr += proximity;
        sensorReadingsStr += '\t';
        if (proximity < 10)
            sensorReadingsStr += '\t';

        pressure = BARO.readPressure(KILOPASCAL);

        sensorReadingsStr += "pres: ";
        sensorReadingsStr += pressure;
        sensorReadingsStr += "kPa\t";

        Serial.println(sensorReadingsStr);
        sensorReadingsStr = "";
        delay(500);
    }

}

void sensors_start_thread()
{
    sensors_thread.start(&sensors_thread_func);
}