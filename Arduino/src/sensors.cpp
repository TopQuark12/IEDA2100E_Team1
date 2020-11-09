#include <Arduino.h>
#include "sensors.h"
#include "Arduino_LSM9DS1.h"
#include "Arduino_HTS221.h"
#include "Arduino_APDS9960.h"
#include "Arduino_LPS22HB.h"
#include "A9G.h"
#include "Mutex.h"

using namespace rtos;

static unsigned char sensors_threadStack[4096];
static Thread sensors_thread(osPriorityAboveNormal1, sizeof(sensors_threadStack), sensors_threadStack, "sensors Thread");

Mutex sensorStrMtx;
String sensorStr = "";
float peakA = 0;
int maxProx = 0;

void sensors_thread_func()  
{

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

    float x, y, z, accelMag;
    float temperature;
    float humidity;
    int proximity = 255;
    float pressure;

    String sensorReadingsStrLocal;

    while (1)
    {    
        
        temperature = HTS.readTemperature(CELSIUS);
        humidity    = HTS.readHumidity();

        sensorReadingsStrLocal += "temp=";
        sensorReadingsStrLocal += temperature;
        sensorReadingsStrLocal += ';';

        sensorReadingsStrLocal += "humi=";
        sensorReadingsStrLocal += humidity;
        sensorReadingsStrLocal += ';';

        if (APDS.proximityAvailable())
        {
            proximity = APDS.readProximity();
            if (proximity > maxProx)
                maxProx = proximity;
        }

        pressure = BARO.readPressure(KILOPASCAL);
        sensorReadingsStrLocal += "pres=";
        sensorReadingsStrLocal += pressure;
        sensorReadingsStrLocal += ";";

        if (IMU.accelerationAvailable()) {

            IMU.readAcceleration(x, y, z);
            accelMag = sqrt(x * x + y * y + z * z);
            if (accelMag > peakA)
                peakA = accelMag;
        }

        sensorStrMtx.lock();
        sensorReadingsStrLocal += "peakA=";
        sensorReadingsStrLocal += peakA;
        sensorReadingsStrLocal += ';';
        sensorReadingsStrLocal += "prox=";
        sensorReadingsStrLocal += maxProx;
        sensorReadingsStrLocal += ';';
        sensorStr = sensorReadingsStrLocal;        
        sensorStrMtx.unlock();
        
    
        // Serial.println(sensorReadingsStrLocal);
        sensorReadingsStrLocal = "";
        osDelayUntil(osKernelGetTickCount() + 20);
        // digitalWrite(LEDB, HIGH);
    }

}

String sensorGetString(void)
{
    String sensorReadingsStrLocal;
    sensorStrMtx.lock();
    peakA = 0;
    maxProx = 0;
    sensorReadingsStrLocal = sensorStr;
    sensorStrMtx.unlock();
    return sensorReadingsStrLocal;
}

void sensors_start_thread()
{
    sensors_thread.start(&sensors_thread_func);
}