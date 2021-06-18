#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "modPowerElectronics.h"
#include "modConfig.h"

bool state_of_charge_init(modPowerElectronicsPackStateTypedef *copy_pack_state); 
void state_of_charge_task(void); 

