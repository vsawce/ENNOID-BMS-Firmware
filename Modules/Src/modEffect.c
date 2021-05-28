#include "modEffect.h"

STATStatusStruct STATStatuses[NoOfSTATs] = 								// Hold all STAT StateIndicator data
{
	{STAT_OFF,0},																						// STAT_LED_DEBUG
	{STAT_OFF,0},																						// STAT_LED_Power
	{STAT_OFF,0}																						// STAT_BUZZER
};

void modEffectInit(void) {
	driverHWStatusInit();																		// Init GPIO direction and state
	modEffectTask();
	modDelayInit();																					// Init delay mechanism
}

void modEffectChangeState(STATIDTypedef id, STATStateTypedef NewState) {
	if(STATStatuses[id].State != NewState) {
		STATStatuses[id].State = NewState;
		STATStatuses[id].Count = 1;
	}else{
		STATStatuses[id].Count++;
	}
}

void modEffectChangeStateError(STATIDTypedef id, STATStateTypedef NewState, uint8_t errorCode) {
	static uint32_t error = 0;
	
	if(STATStatuses[id].State != NewState || error != errorCode) {
		STATStatuses[id].State = NewState;
		STATStatuses[id].Count = (uint32_t)errorCode;
		error = errorCode;
	}
}

void modEffectTask(void) {
	uint32_t LEDPointer;
	
	for(LEDPointer = 0; LEDPointer < NoOfSTATs; LEDPointer++) {
		switch(STATStatuses[LEDPointer].State) {
			case STAT_RESET:
			case STAT_OFF:
				driverHWSetOutput((STATIDTypedef)LEDPointer,STAT_RESET);
				break;
			case STAT_SET:
			case STAT_ON:
				driverHWSetOutput((STATIDTypedef)LEDPointer,STAT_SET);
				break;
			case STAT_FLASH:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskFlash());
				break;
			case STAT_FLASH_FAST:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskFlashFast());
				break;
			case STAT_BLINKSHORT:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskBlinkShort(LEDPointer,50));
				break;
			case STAT_BLINKLONG:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskBlinkLong(LEDPointer,500));
				break;
			case STAT_BLINKSHORTLONG_100_20:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskBlinkShortLong(200,10));
				break;
			case STAT_BLINKSHORTLONG_1000_4:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskBlinkShortLong(1000,4));
				break;
			case STAT_ERROR:
				driverHWSetOutput((STATIDTypedef)LEDPointer,modEffectTaskError(200,1,LEDPointer));
				break;				
			default:
				break;
		}
	}
}

STATStateTypedef modEffectTaskFlash(void) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	
	if(modDelayTick100ms(&lastTick,5)) {
		if(LEDOnState){
			LEDOnState = STAT_RESET;
		}else{
			LEDOnState = STAT_SET;
		}
	}
	
	return LEDOnState;
}

STATStateTypedef modEffectTaskFlashFast(void) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	
	if(modDelayTick1ms(&lastTick,100)) {
		if(LEDOnState){
			LEDOnState = STAT_RESET;
		}else{
			LEDOnState = STAT_SET;
		}
	}
	
	return LEDOnState;
}

STATStateTypedef modEffectTaskBlinkShort(uint32_t LEDPointer, uint32_t blinkTime) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	
	if(LEDOnState) {
		if(modDelayTick1ms(&lastTick,blinkTime)) {
			LEDOnState = STAT_RESET;
			lastTick = HAL_GetTick();
		}
	}else{
		if(modDelayTick1ms(&lastTick,blinkTime)) {
			if(STATStatuses[LEDPointer].Count > MAX_BLINKSHORT_CYCLES) {
				STATStatuses[LEDPointer].Count = MAX_BLINKSHORT_CYCLES;
			}
			if(STATStatuses[LEDPointer].Count > 0) {
				STATStatuses[LEDPointer].Count--;
				LEDOnState = STAT_SET;
			}
			lastTick = HAL_GetTick();
		}
	}
	
	return LEDOnState;
}

STATStateTypedef modEffectTaskBlinkLong(uint32_t LEDPointer, uint32_t blinkTime) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	
	if(LEDOnState) {
		if(modDelayTick1ms(&lastTick,blinkTime)) {
			LEDOnState = STAT_RESET;
			lastTick = HAL_GetTick();
		}
	}else{
		if(modDelayTick1ms(&lastTick,blinkTime)) {
			if(STATStatuses[LEDPointer].Count > MAX_BLINKLONG_CYCLES) {
				STATStatuses[LEDPointer].Count = MAX_BLINKLONG_CYCLES;
			}
			if(STATStatuses[LEDPointer].Count > 0) {
				STATStatuses[LEDPointer].Count--;
				LEDOnState = STAT_SET; 
			}
			lastTick = HAL_GetTick();
		}
	}
	
	return LEDOnState;
}

STATStateTypedef modEffectTaskBlinkShortLong(uint32_t blinkTimeShort, uint32_t blinkRatio) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	
	if(LEDOnState) {
		if(modDelayTick1ms(&lastTick,blinkTimeShort)) {
			LEDOnState = STAT_RESET;
			lastTick = HAL_GetTick();
		}
	}else{
		if(modDelayTick1ms(&lastTick,blinkTimeShort*blinkRatio)) {
			LEDOnState = STAT_SET;
			lastTick = HAL_GetTick();
		}
	}
	
	return LEDOnState;
}

STATStateTypedef modEffectTaskError(uint32_t blinkTimeShort, uint32_t blinkRatio, uint32_t LEDPointer) {
	static uint32_t lastTick;
	static STATStateTypedef LEDOnState = STAT_RESET;
	static uint8_t errorCounter = 0;
	
	if(LEDOnState) {
		if(modDelayTick1ms(&lastTick,blinkTimeShort)) {
			LEDOnState = STAT_RESET;
			errorCounter++;
		}
	}else{
			if(errorCounter >= STATStatuses[LEDPointer].Count){
				if(modDelayTick1ms(&lastTick,blinkTimeShort*blinkRatio*5)) {
					LEDOnState = STAT_SET;
					errorCounter = 0;
				}
			}else{
				if(modDelayTick1ms(&lastTick,blinkTimeShort*blinkRatio)) {
					LEDOnState = STAT_SET;				
				}
			}
	}
	
	return LEDOnState;
}
