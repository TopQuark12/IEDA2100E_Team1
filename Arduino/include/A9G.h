#pragma once

#define A9G_PWR_PIN         2
#define A9G_NRST_PIN        3

void A9G_powerOn(void);
void A9G_powerOff(void);

enum A9G_state_enum
{
    RESET = 0,
    PWR_ON
};
