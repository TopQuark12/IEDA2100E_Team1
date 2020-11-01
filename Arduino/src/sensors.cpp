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

    while (1)
    {    
        if (IMU.accelerationAvailable()) {

            IMU.readAcceleration(x, y, z);

            Serial.print("ax: ");
            Serial.print(x);
            Serial.print('\t');
            Serial.print("ay: ");
            Serial.print(y);
            Serial.print('\t');
            Serial.print("az: ");
            Serial.print(z);
            Serial.print('\t');
        }

        temperature = HTS.readTemperature(CELSIUS);
        humidity    = HTS.readHumidity();

        Serial.print("temp: ");
        Serial.print(temperature);
        Serial.print('\t');

        Serial.print("humi: ");
        Serial.print(humidity);
        Serial.print('\t');

        if (APDS.proximityAvailable())
        {
            proximity = APDS.readProximity();
        }

        Serial.print("prox: ");
        Serial.print(proximity);
        Serial.print('\t');
        if (proximity < 10)
            Serial.print('\t');

        pressure = BARO.readPressure(KILOPASCAL);

        Serial.print("pres: ");
        Serial.print(pressure);
        Serial.print("kPa\t");

        Serial.println();
        delay(500);
    }

}

void sensors_start_thread()
{
    sensors_thread.start(&sensors_thread_func);
}