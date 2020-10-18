#include <Arduino.h>
#include "A9G.h"

using namespace rtos;

void setup()
{
    // pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(LEDG, OUTPUT);
    A9G_powerOn();
    delay(10000);
    A9G_powerOff();
    delay(10000);
    A9G_powerOn();
    delay(10000);
    A9G_powerOff();
    delay(10000);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

void test_thread_func()
{
    while (1) {
        digitalWrite(LEDG, HIGH);
        delay(500);
        digitalWrite(LEDG, LOW);
        delay(500);
    }
}

int main()
{
    Thread test_thread;
    test_thread.start(&test_thread_func);
    setup();
    while (1)
        loop();
}