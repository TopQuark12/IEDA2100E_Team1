#include <Arduino.h>
#include "serialCDC.h"
#include "A9G.h"

using namespace rtos;

static unsigned char serialCDC_threadStack[4096];
static Thread serialCDC_thread(osPriorityBelowNormal5, sizeof(serialCDC_threadStack), serialCDC_threadStack, "Serial CDC Thread");

String serialReadTill(String endWith);

void serialCDC_thread_func()  
{

    String inStr;

    Serial.begin(115200);
    // Serial.setTimeout(ULONG_MAX);
    while (true)
    {
        // inStr = Serial.readStringUntil('\n');
        inStr = serialReadTill("\n");
        Serial.println(inStr);
        if (inStr.startsWith("AT"))
        {
            if(A9G_returnState() == A9G_PWR_ON)
            {
                Serial1.println(inStr);
            }
        }
    }
    
}

String serialReadTill(String endWith)
{
    String inStr;
    while(!inStr.endsWith(endWith))
    {
        if (Serial.available())
        {
            inStr += Serial.readString();
            // Serial.println("got sth");
        }
        delay(10);
    }
    return inStr;
}

void serialCDC_start_thread()
{
    serialCDC_thread.start(&serialCDC_thread_func);
}