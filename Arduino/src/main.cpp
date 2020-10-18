#include <Arduino.h>
#include <USB/PluggableUSBDevice.h>
#include "A9G.h"

using namespace rtos;

void A9G_thread_func();
void test_thread_func();

void setup()
{

}

void loop()
{
    delay(2000);
}



void A9G_thread_func()
{
    while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
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

int main(void)
{
	init();
	initVariant();

    #if defined(SERIAL_CDC)
        PluggableUSBD().begin();
        SerialUSB.begin(115200);
    #endif

	setup();

    static unsigned char A9G_threadStack[4096];
    Thread A9G_thread(osPriorityAboveNormal3, sizeof(A9G_threadStack), A9G_threadStack, "A9G Thread");
    A9G_thread.start(&A9G_thread_func);
    static unsigned char test_threadStack[4096];
    Thread test_thread(osPriorityAboveNormal1, sizeof(test_threadStack), test_threadStack, "Test Thread");
    test_thread.start(&test_thread_func);

	for (;;) {
		loop();
		if (arduino::serialEventRun) arduino::serialEventRun();
	}

	return 0;
}
