#include <Arduino.h>
#include "A9G.h"
#include "sensors.h"
#include "TinyGPS++.h"
#include "BLE.h"

using namespace rtos;

const uint32_t sendPeriod = 30000;

uint32_t A9G_state = A9G_RESET;
static unsigned char A9G_threadStack[4096];
static Thread A9G_thread(osPriorityAboveNormal3, sizeof(A9G_threadStack), A9G_threadStack, "A9G Thread");

TinyGPSPlus gps;
TinyGPSCustom frameEnd(gps, "GNVTG", 1);
TinyGPSCustom gnssQuality(gps, "GNGGA", 6);

String serial1ATsendFor(String command, unsigned long timeoutMs);
String serial1ReadTill(String endWith, unsigned long timeoutMs);
void A9G_powerOff(void);
void A9G_powerOn(void);

void A9G_thread_func()
{
    String inStr;

    A9G_powerOff();
    delay(2500);
    A9G_powerOn();

    Serial.println(serial1ReadTill("READY\r\n", 60000));
    A9G_state = A9G_PWR_ON;
    delay(2000);

    // Begin link to internet
    Serial.println(serial1ATsendFor("AT+CGATT=1", 20000));
    
    //Configure IP
    Serial.println(serial1ATsendFor("AT+CGDCONT=1,\"IP\",\"CMNET\"", 10000));

    //Activate PDP
    Serial.println(serial1ATsendFor("AT+CGACT=1,1", 10000));

    //Connect to MQTT server
    inStr = serial1ATsendFor("AT+MQTTCONN=\"ieda2100epi.duckdns.org\",1883,\"12345\",1200,0,\"IEDA2100E_TAG\",\"IEDA2100E\"", 10000);
    Serial.println(inStr);
    if (inStr.endsWith("OK\r\n"))
        A9G_state = A9G_MQTT_READY;

    //Publish MQTT message
    //Param: topic, payload, QOS, dup, remain
    // A9G_MQTT_sendStr("test", "HELLO lol!!!");

    Serial.println(serial1ATsendFor("AT+GPS=0", 10000));
    Serial.println(serial1ATsendFor("AT+AGPS=1", 50000));
    Serial.println(serial1ReadTill("\r\n", 50000));
    Serial.println(serial1ATsendFor("AT+GPSLP=0", 10000));
    Serial.println(serial1ATsendFor("AT+GPSMD=2", 10000));
    Serial.println(serial1ATsendFor("AT+GPSRD=1", 25000));    

    // Serial.println(A9G_MQTT_sendStr("test", "LOLOLOL"));

    String latStr;
    String lngStr;
    String MQTTmsg;
    unsigned long lastSent = millis() - sendPeriod;

    while (true)
    {
        inStr = serial1ReadTill("\n", ULONG_MAX);
        Serial.println(inStr);
        // delay(100);
        for (unsigned int i = 0; i < inStr.length(); i++)
            gps.encode(inStr[i]);
        inStr = "";        

        if (millis() - lastSent > sendPeriod)
        {

            if (frameEnd.isUpdated())
            {
                String latStr = String(gps.location.lat(), 6);
                String lngStr = String(gps.location.lng(), 6);
                MQTTmsg =   "id=" + BLEgetAddr() + ";" +
                            sensorGetString() +
                            "lat=" + latStr + ";" +
                            "lng=" + lngStr + ";" +
                            "fix=" + String(gnssQuality.value());

            }

            Serial.println(MQTTmsg);

            while (!A9G_MQTT_sendStr("boxTag", MQTTmsg))
            {
                Serial.println("GSM reconnect");

                // Begin link to internet
                Serial.println(serial1ATsendFor("AT+CGATT=1", 20000));
                
                //Configure IP
                Serial.println(serial1ATsendFor("AT+CGDCONT=1,\"IP\",\"CMNET\"", 10000));

                //Activate PDP
                Serial.println(serial1ATsendFor("AT+CGACT=1,1", 10000));

                //Connect to MQTT server
                inStr = serial1ATsendFor("AT+MQTTCONN=\"ieda2100epi.duckdns.org\",1883,\"12345\",1200,0,\"IEDA2100E_TAG\",\"IEDA2100E\"", 10000);
                Serial.println(inStr);
                if (inStr.endsWith("OK\r\n"))
                    A9G_state = A9G_MQTT_READY;
            }

            lastSent = millis();
        }

    }
}

bool A9G_MQTT_sendStr(String topic, String message)
{
    String inStr, outStr;
    if (A9G_state == A9G_MQTT_READY)
    {
        outStr += "AT+MQTTPUB=\"";
        outStr += topic;
        outStr += "\",\"";
        outStr += message;
        outStr += "\",1,0,0";
        inStr = serial1ATsendFor(outStr, 10000);
        if (inStr.endsWith("OK\r\n"))
            return true;
        else
            return false;
    } else {
        return false;
    }
}

String serial1ATsendFor(String command, unsigned long timeoutMs)
{
    Serial1.flush();
    Serial1.println(command);
    unsigned long startTime = millis();
    String inStr;
    while ((inStr.indexOf("OK\r\n") == -1) && (millis() < (startTime + timeoutMs)))
    {
        if (Serial1.available())
            inStr += Serial1.readString();
        if (inStr.indexOf("ERROR") != -1)
        {
            // serial1ReadTill("\r\n");
            Serial.println(inStr);
            inStr = "";
            Serial.print("retry : ");
            Serial.println(command);
            Serial1.println(command);
        }
        delay(10);
    }
    // Serial1.flush();
    if (inStr.indexOf("OK\r\n") == -1)
    {
        return "TIMEOUT : " + command;
    } else {
        return inStr;
    }
}

String serial1ReadTill(String endWith, unsigned long timeoutMs)
{
    String inStr;
    unsigned long startTime = timeoutMs == ULONG_MAX ? 0 : millis();
    while(!inStr.endsWith(endWith) && (millis() < (startTime + timeoutMs)))
    {
        if (Serial1.available())
            inStr += Serial1.readString();
        delay(10);
    }
    Serial1.flush();
    if (!inStr.endsWith(endWith))
    {
        return "TIMEOUT";
    } else {
        return inStr;
    }
}

void A9G_start_thread()
{
    A9G_thread.start(&A9G_thread_func);
}

void A9G_powerOff(void) 
{
    //configure pin mode
    pinMode(A9G_NRST_PIN, OUTPUT);

    //reset A9G module
    digitalWrite(A9G_NRST_PIN, HIGH);
    delay(500);
    digitalWrite(A9G_NRST_PIN, LOW);
    delay(500);

    //float nRst pin
    pinMode(A9G_NRST_PIN, INPUT);
    A9G_state = A9G_RESET;
}

void A9G_powerOn(void) 
{
    //configure pin mode
    pinMode(A9G_PWR_PIN, OUTPUT);

    //hold power button
    digitalWrite(A9G_PWR_PIN, LOW);
    delay(3000);

    //float power pin
    pinMode(A9G_PWR_PIN, INPUT);

    Serial1.begin(115200);
    // Serial1.setTimeout(ULONG_MAX);
    Serial1.setTimeout(1000);

    A9G_state = A9G_PWR_ON;
}

uint32_t A9G_returnState(void) 
{
    return A9G_state;
}