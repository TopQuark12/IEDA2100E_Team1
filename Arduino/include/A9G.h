#pragma once

#define A9G_PWR_PIN         2
#define A9G_NRST_PIN        3

void A9G_start_thread(void);
void A9G_powerOn(void);
void A9G_powerOff(void);
uint32_t A9G_returnState(void);

enum A9G_state_enum
{
    A9G_RESET = 0,
    A9G_PWR_ON
};
