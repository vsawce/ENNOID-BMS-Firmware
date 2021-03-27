#pragma once

#include "modPowerElectronics.h"

void report_status__init(modPowerElectronicsPackStateTypedef *copy_pack_state); 
void report_status__task(void);
void report__safety_status(void);
void reoprt__cell_voltage(void); 
void report__cell_temperature_celsius(void);
void report__bms_board_temperature_celsius(void); 
void report__battery_pack_voltage_and_current(void); 
void report__load_voltage_and_current(void);
void report__load_power_w(void);
void report__soc(void); 
void report__cell_and_bms_average_temperature_celsius(void); 
