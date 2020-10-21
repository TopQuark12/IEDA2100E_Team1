#include <Arduino.h>
#include "A9G.h"

using namespace rtos;

uint32_t A9G_state = A9G_RESET;
static unsigned char A9G_threadStack[4096];
static Thread A9G_thread(osPriorityAboveNormal3, sizeof(A9G_threadStack), A9G_threadStack, "A9G Thread");

String serial1ReadTill(String endWith);

void A9G_thread_func()
{
    String inStr;

    A9G_powerOff();
    A9G_powerOn();

    inStr = serial1ReadTill("READY\r\n");
    Serial.println(inStr);

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