#include <Arduino.h>
#include "A9G.h"
#include "serialCDC.h"

using namespace rtos;

void test_thread_func();
static unsigned char test_threadStack[4096];    
static Thread test_thread(osPriorityAboveNormal1, sizeof(test_threadStack), test_threadStack, "Test Thread");

void setup()
{
    // Serial.begin(115200);
    // delay(800);             //delay(800) must be added after Serial.begin()
    serialCDC_start_thread();
    A9G_start_thread();
    test_thread.start(&test_thread_func);
}

void loop()
{
    // Serial.println("Hello World");
    delay(2000);
}

void test_thread_func()
{
    while (1) {
        digitalWrite(LEDG, !HIGH);
        delay(100);
        digitalWrite(LEDG, !LOW);
        delay(2000);
    }
}
