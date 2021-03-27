#include "report_status.h"

#include "stm32f3xx_hal.h"

#include "safety_check.h"
#include "modCAN.h"

static const modPowerElectronicsPackStateTypedef *pack_state;

int32_t send_index = 0; 
uint8_t data_buffer[8]; 
static  uint32_t timestamp_1hz;
static  uint32_t timestamp_10hz;

void report_status__init(modPowerElectronicsPackStateTypedef *copy_pack_state)
{
    pack_state = copy_pack_state;
    timestamp_1hz = HAL_GetTick();
    timestamp_10hz = HAL_GetTick();
}

void report_status__task(void)
{
    // 1hz, 10ms
    if ((HAL_GetTick() - timestamp_1hz) >= 10) {
        timestamp_1hz = HAL_GetTick(); 
        report__safety_status(); 
    }

    // 10hz, 100ms
    if ((HAL_GetTick() - timestamp_10hz) >= 100) {
        timestamp_10hz = HAL_GetTick();
        report__battery_pack_voltage_and_current(); 
        report__load_voltage_and_current(); 
        reoprt__cell_voltage();
        report__soc(); 
        report__cell_and_bms_average_temperature_celsius(); 
        report__cell_temperature_celsius();
        report__bms_board_temperature_celsius();
        report__load_power_w();    
    }
}

void report__safety_status(void)
{
    safety_status_s safety_status = safety_check_get_status(); 
    modCANTransmitStandardID((0x620 & 0xFFFFFFFF), safety_status.bms_fault_data, 6);
}

void report__battery_pack_voltage_and_current(void) 
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->packVoltage,1e3,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->packCurrent,1e2,&send_index);
	modCANTransmitStandardID((0x623 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__load_voltage_and_current(void)
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->loCurrentLoadVoltage,1e3,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->loCurrentLoadCurrent,1e2,&send_index);
	modCANTransmitStandardID((0x624 & 0xFFFFFFFF), data_buffer, send_index);
}

void reoprt__cell_voltage(void) 
{
	send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->cellVoltageHigh,1e5,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->cellVoltageLow,1e5,&send_index);
	modCANTransmitStandardID((0x625 & 0xFFFFFFFF), data_buffer, send_index);

	send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->cellVoltageAverage,1e5,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->cellVoltageMisMatch,1e5,&send_index);
	modCANTransmitStandardID((0x626 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__soc(void)
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->SoC,1,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->SoCCapacityAh,1e2,&send_index);
	modCANTransmitStandardID((0x627 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__cell_and_bms_average_temperature_celsius(void)
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->tempBatteryAverage,1e2,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->tempBMSAverage,1e2,&send_index);
	modCANTransmitStandardID((0x628 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__cell_temperature_celsius(void)
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->tempBatteryHigh,1e2,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->tempBatteryLow,1e2,&send_index);
	modCANTransmitStandardID((0x629 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__bms_board_temperature_celsius(void)
{
    send_index = 0; 
    libBufferAppend_float32(data_buffer, pack_state->tempBMSHigh,1e2,&send_index);
	libBufferAppend_float32(data_buffer, pack_state->tempBMSLow,1e2,&send_index);
	modCANTransmitStandardID((0x630 & 0xFFFFFFFF), data_buffer, send_index);
}

void report__load_power_w(void)
{
    float load_power;
    load_power = pack_state->loCurrentLoadVoltage * pack_state->loCurrentLoadCurrent;

    send_index = 0; 
	libBufferAppend_float32(data_buffer, load_power,1e1,&send_index);
	modCANTransmitStandardID((0x631 & 0xFFFFFFFF), data_buffer, send_index);
}
