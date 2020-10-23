#include <Arduino.h>
#include "A9G.h"

using namespace rtos;

uint32_t A9G_state = A9G_RESET;
static unsigned char A9G_threadStack[4096];
static Thread A9G_thread(osPriorityAboveNormal3, sizeof(A9G_threadStack), A9G_threadStack, "A9G Thread");

String serial1ReadTill(String endWith);
void A9G_powerOff(void);
void A9G_powerOn(void);

void A9G_thread_func()
{
    String inStr;

    A9G_powerOff();
    delay(2500);
    A9G_powerOn();

    inStr = serial1ReadTill("READY\r\n");
    Serial.println("A9G Ready");

    //Begin link to internet
    Serial1.println("AT+CGATT=1");
    serial1ReadTill("OK\r\n");
    Serial.println("CGATT OK");

    //Configure IP
    Serial1.println("AT+CGDCONT=1,\"IP\",\"CMNET\"");
    serial1ReadTill("OK\r\n");
    Serial.println("IP CONFIG OK");

    //Activate PDP
    Serial1.println("AT+CGACT=1,1");
    serial1ReadTill("OK\r\n");
    Serial.println("PDP ACTIVATION OK");

    //Connect to MQTT server
    //Param: host, port, clientID, aliveSeconds, cleanSession, userName, password
    Serial1.println("AT+MQTTCONN=\"ieda2100epi.duckdns.org\",1883,\"12345\",1200,0,\"IEDA2100E_TAG\",\"IEDA2100E\"");
    serial1ReadTill("OK\r\n");
    Serial.println("MQTT CONNECTION OK");

    //Publish MQTT message
    //Param: topic, payload, QOS, dup, remain
    Serial1.println("AT+MQTTPUB=\"test\",\"Sensor 1 alive\",1,0,0");
    digitalWrite(LEDB, LOW);
    serial1ReadTill("OK\r\n");
    Serial.println("ALIVE MESSAGE SENT");

    while (true)
    {
        inStr = serial1ReadTill("\n");
        Serial.println(inStr);
        inStr = "";
    }
}

String serial1ReadTill(String endWith)
{
    String inStr;
    while(!inStr.endsWith(endWith))
    {
        if (Serial1.available())
            inStr += Serial1.readString();
        delay(10);
    }
    return inStr;
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