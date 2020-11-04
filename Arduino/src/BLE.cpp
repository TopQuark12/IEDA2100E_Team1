#include "Arduino.h"
#include "BLE.h"
#include "ArduinoBLE.h"

static String deviceAddr = "";

void startBLE()
{
    BLE.begin();
    deviceAddr = BLE.address();
}

String BLEgetAddr()
{
    return deviceAddr;
}