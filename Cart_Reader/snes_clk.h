#ifndef _SNES_CLK_H
#define _SNES_CLK_H

#include <si5351.h>
#include "SdFat.h"
#include "atoi32.h"
#include "options.h"

extern Si5351 clockgen;
extern bool i2c_found;

int32_t readClockOffset();
int32_t initializeClockOffset();
#endif
