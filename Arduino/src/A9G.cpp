#include <Arduino.h>
#include "A9G.h"

using namespace rtos;

uint32_t A9G_state = A9G_RESET;
static unsigned char A9G_threadStack[4096];
static Thread A9G_thread(osPriorityAboveNormal3, sizeof(A9G_threadStack), A9G_threadStack, "A9G Thread");

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

    // inStr = serial1ReadTill("READY\r\n", 10000);
    Serial.println(serial1ReadTill("READY\r\n", 60000));

    //Begin link to internet
    // Serial1.println("AT+CGATT=1");
    // serial1ReadTill("OK\r\n");
    // Serial.println("CGATT OK");
    Serial.println(serial1ATsendFor("AT+CGATT=1", 20000));
    

    //Configure IP
    // Serial1.println("AT+CGDCONT=1,\"IP\",\"CMNET\"");
    // serial1ReadTill("OK\r\n");
    // Serial.println("IP CONFIG OK");
    Serial.println(serial1ATsendFor("AT+CGDCONT=1,\"IP\",\"CMNET\"", 10000));

    //Activate PDP
    // Serial1.println("AT+CGACT=1,1");
    // serial1ReadTill("OK\r\n");
    // Serial.println("PDP ACTIVATION OK");
    Serial.println(serial1ATsendFor("AT+CGACT=1,1", 10000));

    //Connect to MQTT server
    //Param: host, port, clientID, aliveSeconds, cleanSession, userName, password
    // Serial1.println("AT+MQTTCONN=\"ieda2100epi.duckdns.org\",1883,\"12345\",1200,0,\"IEDA2100E_TAG\",\"IEDA2100E\"");
    // serial1ReadTill("OK\r\n");
    // Serial.println("MQTT CONNECTION OK");
    Serial.println(serial1ATsendFor("AT+MQTTCONN=\"ieda2100epi.duckdns.org\",1883,\"12345\",1200,0,\"IEDA2100E_TAG\",\"IEDA2100E\"", 10000));

    //Publish MQTT message
    //Param: topic, payload, QOS, dup, remain
    // Serial1.println("AT+MQTTPUB=\"test\",\"Sensor 1 alive\",1,0,0");
    // digitalWrite(LEDB, LOW);
    // serial1ReadTill("OK\r\n");
    Serial.println(serial1ATsendFor("AT+MQTTPUB=\"test\",\"Sensor 1 alive\",1,0,0", 10000));
    // Serial.println("ALIVE MESSAGE SENT");

    while (true)
    {
        inStr = serial1ReadTill("\n", ULONG_MAX);
        Serial.println(inStr);
        inStr = "";
    }
}

String serial1ATsendFor(String command, unsigned long timeoutMs)
{
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
            inStr = "";
            Serial.print("retry : ");
            Serial.println(command);
            Serial1.println(command);
        }
        delay(10);
    }
    Serial1.flush();
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