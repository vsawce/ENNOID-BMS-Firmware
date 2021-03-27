#include "safety_check.h"

#include <stdio.h>

static safety_status_s safety_status; 

static const modPowerElectronicsPackStateTypedef *pack_state;

void safety_check__init(modPowerElectronicsPackStateTypedef *copy_pack_state) 
{
    pack_state = copy_pack_state; 
}

void safty_check__task(void) 
{
    safety_status.status_is_ok = safety_check__cell_over_voltage() && safety_check__cell_under_voltage()
                                && safety_check__cell_over_temperature() && safety_check__bms_board_over_temperature()
                                && safety_check__power_exceed_80kw(); 

    if (safety_status.status_is_ok) {
        safety_status.bms_fault_data[0] = 0 & 0xFF; 
    } else {
        safety_status.bms_fault_data[0] = 0xFF; 
    }
}

bool safety_check__cell_over_voltage(void)
{
    if (pack_state->cellVoltageHigh >= 4.2f) {
        safety_status.bms_fault_data[cell_over_voltage] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[cell_over_voltage] = 0 & 0xFF;
    return true; 
}

bool safety_check__cell_under_voltage(void)
{
    if (pack_state->cellVoltageLow <= 2.5f) {
        safety_status.bms_fault_data[cell_under_voltage] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[cell_under_voltage] = 0 & 0xFF;
    return true; 
}

bool safety_check__cell_over_temperature(void)
{
    if (pack_state->tempBatteryHigh >= 60.0f) {
        safety_status.bms_fault_data[cell_over_temperature] = 0xFF; 
        return false;
    }
    safety_status.bms_fault_data[cell_over_temperature] = 0 & 0xFF; 
    return true; 
}

bool safety_check__bms_board_over_temperature(void)
{
    // It can be bms master board or bms slave boards
    if (pack_state->tempBMSHigh >= 60.0f) {
        safety_status.bms_fault_data[bms_board_over_temperature] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[bms_board_over_temperature] = 0 & 0xFF; 
    return true; 
}

bool safety_check__power_exceed_80kw(void)
{
    float load_power;
    float maximum_allowed_power = 80 * 1000; 
    load_power = pack_state->loCurrentLoadVoltage * pack_state->loCurrentLoadCurrent;
    if (load_power >= maximum_allowed_power) {
        safety_status.bms_fault_data[power_exceed_80kw] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[power_exceed_80kw] = 0 & 0xFF; 
    return true; 
}

safety_status_s safety_check_get_status(void)
{
    return safety_status; 
}
