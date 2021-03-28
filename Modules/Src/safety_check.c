#include "stm32f3xx_hal.h"

#include "safety_check.h"

#include <stdio.h>

static safety_status_s safety_status; 

static modPowerElectronicsPackStateTypedef *pack_state;

void safety_check_init(modPowerElectronicsPackStateTypedef *copy_pack_state) 
{
    pack_state = copy_pack_state; 

//     GPIO_InitTypeDef GPIO_InitStruct;

//   __HAL_RCC_GPIOB_CLK_ENABLE();
//     HAL_GPIO_WritePin(GPIOB, OLED_RST_Pin, GPIO_PIN_RESET);																								// The enable FET of the DC DC converter is connected to the RST pin
//     GPIO_InitStruct.Pin = OLED_RST_Pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void safety_check_task(void) 
{

    safety_status.status_is_ok = safety_check_cell_over_voltage() && safety_check_cell_under_voltage()
                                && safety_check_cell_over_temperature() && safety_check_bms_board_over_temperature()
                                && safety_check_power_exceed_80kw(); 

    if (safety_status.status_is_ok) {
        safety_status.bms_fault_data[0] = 0 & 0xFF; 
    } else {
        safety_status.bms_fault_data[0] = 0xFF; 
    }
}

bool safety_check_cell_over_voltage(void)
{
    if (pack_state->cellVoltageHigh >= 4.2f) {
        safety_status.bms_fault_data[cell_over_voltage] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[cell_over_voltage] = 0 & 0xFF;
    return true; 
}

bool safety_check_cell_under_voltage(void)
{
    if (pack_state->cellVoltageLow <= 2.5f) {
        safety_status.bms_fault_data[cell_under_voltage] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[cell_under_voltage] = 0 & 0xFF;
    return true; 
}

bool safety_check_cell_over_temperature(void)
{
    if (pack_state->tempBatteryHigh >= 60.0f) {
        safety_status.bms_fault_data[cell_over_temperature] = 0xFF; 
        return false;
    }
    safety_status.bms_fault_data[cell_over_temperature] = 0 & 0xFF; 
    return true; 
}

bool safety_check_bms_board_over_temperature(void)
{
    // It can be bms master board or bms slave boards
    if (pack_state->tempBMSHigh >= 60.0f) {
        safety_status.bms_fault_data[bms_board_over_temperature] = 0xFF; 
        return false; 
    }
    safety_status.bms_fault_data[bms_board_over_temperature] = 0 & 0xFF; 
    return true; 
}

bool safety_check_power_exceed_80kw(void)
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
