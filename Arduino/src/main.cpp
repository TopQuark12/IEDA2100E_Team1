#include <Arduino.h>

using namespace rtos;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LEDG, OUTPUT);
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
    setup();
    Thread test_thread;
    test_thread.start(&test_thread_func);
    while (1)
        loop();
}