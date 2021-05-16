/*
	Copyright 2017 - 2018 Danny Bokma	danny@diebie.nl
	Copyright 2019 - 2020 Kevin Dionne	kevin.dionne@ennoid.me

	This file is part of the DieBieMS/ENNOID-BMS firmware.

	The DieBieMS/ENNOID-BMS firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The DieBieMS/ENNOID-BMS firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "modOperationalState.h"

OperationalStateTypedef modOperationalStateLastState;
OperationalStateTypedef modOperationalStateCurrentState;
OperationalStateTypedef modOperationalStateNewState;
bms_fault_state modOperationalStateLastFaultState;
bms_fault_state modOperationalStateCurrentFaultState;
bms_fault_state modOperationalStateNewFaultState;
modPowerElectronicsPackOperationalCellStatesTypedef packOperationalCellStateLastErrorState;
modPowerElectronicsPackStateTypedef *modOperationalStatePackStatehandle;
modConfigGeneralConfigStructTypedef *modOperationalStateGeneralConfigHandle;
modStateOfChargeStructTypeDef *modOperationalStateGeneralStateOfCharge;
modDisplayDataTypedef modOperationalStateDisplayData;
uint32_t modOperationalStateChargerTimeout;
uint32_t modOperationalStateChargedTimeout;
uint32_t modOperationalStatePreChargeTimeout;
uint32_t modOperationalStateStartupDelay;
uint32_t modOperationalStateChargerDisconnectDetectDelay;
uint32_t modOperationalStateBatteryDeadDisplayTime;
uint32_t modOperationalStateErrorDisplayTime;
uint32_t modOperationalStateNotUsedResetDelay;
uint32_t modOperationalStateNotUsedTime;
uint32_t modOperationalStatePSPDisableDelay;
uint32_t modOperationalStateWatchDogCountdownLastTick;
bool modOperationalStateForceOn;

void modOperationalStateInit(modPowerElectronicsPackStateTypedef *packState, modConfigGeneralConfigStructTypedef *generalConfigPointer, modStateOfChargeStructTypeDef *generalStateOfCharge) {
	modOperationalStatePackStatehandle = packState;
	modOperationalStateGeneralConfigHandle = generalConfigPointer;
	modOperationalStateGeneralStateOfCharge = generalStateOfCharge;
	modOperationalStateSetAllStates(OP_STATE_INIT);
	modOperationalStateStartupDelay = HAL_GetTick();
	modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	packOperationalCellStateLastErrorState = PACK_STATE_NORMAL;
	modOperationalStateForceOn = false;
	modDisplayInit();
	
	//Init Expansion temperature modules
	driverSWADC128D818Init(modOperationalStateGeneralConfigHandle->noOfExpansionBoard,  modOperationalStateGeneralConfigHandle->noOfTempSensorPerExpansionBoard);
	
	
	modOperationalStateNotUsedTime = HAL_GetTick();
	modOperationalStateNotUsedResetDelay = HAL_GetTick();
};

void modOperationalStateTask(void) {	
	switch(modOperationalStateCurrentState) {
		case OP_STATE_INIT:
			if(modPowerStateChargerDetected()) {																		// Check to detect charger
				switch(modOperationalStateGeneralConfigHandle->chargeEnableOperationalState){
				  case opStateChargingModeCharging:
						modOperationalStateSetNewState(OP_STATE_CHARGING);								// Go to charge state

						#if DISABLE_POWER_LED
							modEffectChangeState(STAT_LED_POWER,STAT_FLASH);	
						#endif 								// Flash power LED when charging
						modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
						break;
					case opStateChargingModeNormal:
					default:					
						modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);							// Prepare to goto operational state
						#if DISABLE_POWER_LED
							modEffectChangeState(STAT_LED_POWER,STAT_SET);		
						#endif 								// Turn LED on in normal operation
						break;
				}
			}else if(modPowerStateButtonPressedOnTurnon()) {												// Check if button was pressen on turn-on
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);									// Prepare to goto operational state
				#if DISABLE_POWER_LED
					modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
				#endif 
			}else if(modOperationalStateNewState == OP_STATE_INIT){								  // USB or CAN origin of turn-on
				switch(modOperationalStateGeneralConfigHandle->externalEnableOperationalState){
					case opStateExtNormal:
						modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);							// Prepare to goto normal operational state
						break;
					case opStateExternal:
					default:
						modOperationalStateSetNewState(OP_STATE_EXTERNAL);								// Serve external control
						break;
				}
				#if DISABLE_POWER_LED
					modEffectChangeState(STAT_LED_POWER,STAT_SET);												// Turn LED on in normal operation
				#endif 
			}
			
			if(modDelayTick1ms(&modOperationalStateStartupDelay,modOperationalStateGeneralConfigHandle->displayTimeoutSplashScreen)) {// Wait for a bit than update state. Also check voltage after main fuse? followed by going to error state if blown?		
				if(!modOperationalStatePackStatehandle->disChargeLCAllowed && !modPowerStateChargerDetected()) {						// If discharge is not allowed
					modOperationalStateSetNewState(OP_STATE_ERROR);							// Then the battery is dead
					modOperationalStateBatteryDeadDisplayTime = HAL_GetTick();
				}
				modOperationalStateUpdateStates();																		// Sync states
			};
			
			modDisplayShowInfo(DISP_MODE_SPLASH,modOperationalStateDisplayData);
			break;
		case OP_STATE_CHARGING:
			// If chargeAllowed = false -> operational state balancing
			if(modOperationalStatePackStatehandle->balanceActive){
				modOperationalStateSetNewState(OP_STATE_BALANCING);	
			}
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			modPowerElectronicsSetCharge(true);
			if(modOperationalStatePackStatehandle->packCurrent >= 0.5f | modOperationalStatePackStatehandle->packCurrent >= modOperationalStateGeneralConfigHandle->chargerEnabledThreshold){
				modPowerElectronicsSetChargePFET(true);
			}else{
				modPowerElectronicsSetChargePFET(false);
			};
			#ifndef HWVersion_SS
				//Allow main contactors to close if load voltage is above pack voltage & below max allowed voltage, that means that the charger is connected to the load
				if(modOperationalStatePackStatehandle->packVoltage-modOperationalStatePackStatehandle->loCurrentLoadVoltage < (modOperationalStatePackStatehandle->packVoltage*0.1f) && modOperationalStatePackStatehandle->loCurrentLoadVoltage < (modOperationalStateGeneralConfigHandle->noOfCellsSeries*modOperationalStateGeneralConfigHandle->cellHardOverVoltage+10)){ 
					modPowerElectronicsSetDisCharge(true);
					if(modOperationalStateGeneralConfigHandle->LCUsePrecharge==forced){
						modPowerElectronicsSetPreCharge(true);
					}
				}
			#endif
			//Cooling/Heating
			if(modOperationalStatePackStatehandle->coolingAllowed )
				modPowerElectronicsSetCooling(true);
			else{
				modPowerElectronicsSetCooling(false);
			}
			
			modOperationalStateUpdateStates();
			modOperationalStateDisplayData.StateOfCharge = modOperationalStateGeneralStateOfCharge->generalStateOfCharge;
			modOperationalStateDisplayData.Current = fabs(modOperationalStatePackStatehandle->packCurrent);
			modOperationalStateDisplayData.ChargerVoltage = fabs(modOperationalStatePackStatehandle->chargerVoltage);
			modDisplayShowInfo(DISP_MODE_CHARGE,modOperationalStateDisplayData);
			break;
		case OP_STATE_PRE_CHARGE:
			// in case of timeout: disable pre charge & go to error state
			if(modOperationalStateLastState != modOperationalStateCurrentState) { 	  // If discharge is not allowed pre-charge will not be enabled, therefore reset timeout every task call. Also reset on first entry
				modOperationalStatePreChargeTimeout = HAL_GetTick();										// Reset timeout
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
		
			if(modOperationalStatePackStatehandle->disChargeLCAllowed || modOperationalStateForceOn)
				modPowerElectronicsSetPreCharge(true);
			else{
				modPowerElectronicsSetPreCharge(false);
				modOperationalStatePreChargeTimeout = HAL_GetTick();
			}
			
			if((modOperationalStatePackStatehandle->loCurrentLoadVoltage > modOperationalStatePackStatehandle->packVoltage*modOperationalStateGeneralConfigHandle->minimalPrechargePercentage) && (modOperationalStatePackStatehandle->disChargeLCAllowed || modOperationalStateForceOn)) {
				if(modOperationalStateForceOn) {
					modOperationalStateSetNewState(OP_STATE_FORCEON);								// Goto force on
				}else{
					modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);					// Goto normal load enabled operation
				}
			}else if(modDelayTick1ms(&modOperationalStatePreChargeTimeout,modOperationalStateGeneralConfigHandle->timeoutLCPreCharge)){
				if(modOperationalStateGeneralConfigHandle->LCUsePrecharge>=1){
				  modOperationalStateSetNewState(OP_STATE_ERROR_PRECHARGE);				// An error occured during pre charge
					modOperationalStatePackStatehandle->faultState = FAULT_CODE_PRECHARGE_TIMEOUT;
				}else
					modOperationalStateSetNewState(OP_STATE_LOAD_ENABLED);					// Goto normal load enabled operation
			}
		
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_LOAD_ENABLED:
			if(modPowerElectronicsSetDisCharge(true)) {
				
				if(modOperationalStateGeneralConfigHandle->LCUsePrecharge==forced){
					modPowerElectronicsSetPreCharge(true);
				}else{
					modPowerElectronicsSetPreCharge(false);
				}
			  if(modPowerStateChargerDetected()){
					modPowerElectronicsSetCharge(modOperationalStateGeneralConfigHandle->allowChargingDuringDischarge);
					if(modOperationalStatePackStatehandle->packCurrent >= 0.5f| modOperationalStatePackStatehandle->packCurrent >= modOperationalStateGeneralConfigHandle->chargerEnabledThreshold){
						modPowerElectronicsSetChargePFET(true);
					}
				}
			}else{
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
				modPowerElectronicsSetChargePFET(false);
			}
			
			//Cooling/Heating
			if(modOperationalStatePackStatehandle->coolingAllowed )
				modPowerElectronicsSetCooling(true);
			else{
				modPowerElectronicsSetCooling(false);
			}
		//Charger detect
			if(modPowerStateChargerDetected() && !modOperationalStateGeneralConfigHandle->allowChargingDuringDischarge) {
				modOperationalStateSetNewState(OP_STATE_INIT);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			};
			
			
			// Battery is empty or battery temp is out of range?
			if(!modOperationalStatePackStatehandle->disChargeLCAllowed) {							
				//modOperationalStateSetNewState(OP_STATE_ERROR);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
				modOperationalStatePackStatehandle->faultState = FAULT_CODE_DISCHARGE_RETRY;
				
			}

			
			if(fabs(modOperationalStatePackStatehandle->packCurrent) >= modOperationalStateGeneralConfigHandle->notUsedCurrentThreshold) {
				if(modDelayTick1ms(&modOperationalStateNotUsedResetDelay,1000))
					modOperationalStateNotUsedTime = HAL_GetTick();
			}else{
				modOperationalStateNotUsedResetDelay = HAL_GetTick();
			}
			
			if(modOperationalStatePowerDownDelayCheck()) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
			
			if(modOperationalStatePackStatehandle->balanceActive) {
				if(!modOperationalStatePackStatehandle->chargeAllowed && (modOperationalStatePackStatehandle->cellVoltageMisMatch < modOperationalStateGeneralConfigHandle->maxMismatchThreshold)){
					if(modDelayTick1ms(&modOperationalStateChargedTimeout,modOperationalStateGeneralConfigHandle->timeoutChargingCompletedMinimalMismatch)) {
						modStateOfChargeVoltageEvent(EVENT_FULL);
						modOperationalStateChargedTimeout = HAL_GetTick();
					}
				}else{
					modOperationalStateChargedTimeout = HAL_GetTick();
				};			
			}
			
			modOperationalStateUpdateStates();
			
			modOperationalStateDisplayData.StateOfCharge = modOperationalStateGeneralStateOfCharge->generalStateOfCharge;
			modOperationalStateDisplayData.Current = fabs(modOperationalStatePackStatehandle->packCurrent);
			modOperationalStateDisplayData.PackVoltage = fabs(modOperationalStatePackStatehandle->packVoltage);
			modOperationalStateDisplayData.HighestTemp = fabs(modOperationalStatePackStatehandle->tempBatteryHigh);
			modOperationalStateDisplayData.AverageTemp = fabs(modOperationalStatePackStatehandle->tempBatteryAverage);
			modOperationalStateDisplayData.LowestTemp = fabs(modOperationalStatePackStatehandle->tempBatteryLow);
			modOperationalStateDisplayData.Humidity = fabs(modOperationalStatePackStatehandle->humidity);
			modOperationalStateDisplayData.LowestCellVoltage = fabs(modOperationalStatePackStatehandle->cellVoltageLow);
			modOperationalStateDisplayData.HighestCellVoltage = fabs(modOperationalStatePackStatehandle->cellVoltageHigh);
			modOperationalStateDisplayData.DisplayStyle = modOperationalStateGeneralConfigHandle->displayStyle;
			
			modDisplayShowInfo(DISP_MODE_LOAD,modOperationalStateDisplayData);
			break;
		case OP_STATE_BATTERY_DEAD:
			modDisplayShowInfo(DISP_MODE_BATTERY_DEAD,modOperationalStateDisplayData);
			if(modDelayTick1ms(&modOperationalStateBatteryDeadDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryDead)){
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
				modOperationalStatePackStatehandle->faultState = FAULT_CODE_PACK_UNDER_VOLTAGE;
			}
			modOperationalStateUpdateStates();
			break;
		case OP_STATE_POWER_DOWN:
			if(modOperationalStateLastState != modOperationalStateCurrentState) {
			  modOperationalStatePSPDisableDelay = HAL_GetTick();
			}
			modPowerElectronicsDisableAll();	
			#if DISABLE_POWER_LED																			// Disable all power paths
				modEffectChangeState(STAT_LED_POWER,STAT_RESET);												// Turn off power LED
			#endif 
			modEffectChangeState(STAT_LED_DEBUG,STAT_RESET);
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
		  if(modDelayTick1ms(&modOperationalStatePSPDisableDelay,modOperationalStateGeneralConfigHandle->powerDownDelay))	{					// Wait for the power down delay time to pass
			  modOperationalStateTerminateOperation();															// Disable powersupply and store SoC
			}
			break;
		case OP_STATE_EXTERNAL:																										// BMS is turned on by external force IE CAN or USB
			if(modOperationalStateLastState != modOperationalStateCurrentState) {
				modPowerElectronicsSetPreCharge(false);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetCharge(false);
			}
		
			modOperationalStateTerminateOperation();																// Disable power and store SoC
			modDisplayShowInfo(DISP_MODE_EXTERNAL,modOperationalStateDisplayData);
			
			break;
		case OP_STATE_ERROR:
			// Go to save state and in the future -> try to handle error situation
			if(modOperationalStateLastState != modOperationalStateCurrentState)
				modOperationalStateErrorDisplayTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateErrorDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryError)) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
		
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			#if DISABLE_POWER_LED
				modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			#endif 
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modOperationalStateDisplayData.FaultCode = modOperationalStatePackStatehandle->faultState;
			modDisplayShowInfo(DISP_MODE_ERROR,modOperationalStateDisplayData);

			break;
		case OP_STATE_ERROR_PRECHARGE:
			// Go to save state and in the future -> try to handle error situation
			if(modOperationalStateLastState != modOperationalStateCurrentState)
				modOperationalStateErrorDisplayTime = HAL_GetTick();
			
			if(modDelayTick1ms(&modOperationalStateErrorDisplayTime,modOperationalStateGeneralConfigHandle->displayTimeoutBatteryErrorPreCharge)) {
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
		
			modEffectChangeState(STAT_LED_DEBUG,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			#if DISABLE_POWER_LED
				modEffectChangeState(STAT_LED_POWER,STAT_FLASH_FAST);										// Turn flash fast on debug and power LED
			#endif 
			modPowerElectronicsDisableAll();
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_ERROR_PRECHARGE,modOperationalStateDisplayData);
			break;
		case OP_STATE_BALANCING:
			// update timeout time for balancing and use charging manager for enable state charge input
			if(modOperationalStatePackStatehandle->packCurrent < modOperationalStateGeneralConfigHandle->chargerEnabledThreshold && modOperationalStatePackStatehandle->chargeAllowed){
				if(modDelayTick1ms(&modOperationalStateChargerTimeout,modOperationalStateGeneralConfigHandle->timeoutChargeCompleted)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
					modStateOfChargeVoltageEvent(EVENT_FULL);
				}
			}else{
				modOperationalStateChargerTimeout = HAL_GetTick();
			};
			
			if(!modOperationalStatePackStatehandle->chargeAllowed && (modOperationalStatePackStatehandle->cellVoltageMisMatch < modOperationalStateGeneralConfigHandle->maxMismatchThreshold)){
				if(modDelayTick1ms(&modOperationalStateChargedTimeout,modOperationalStateGeneralConfigHandle->timeoutChargingCompletedMinimalMismatch)) {
					modOperationalStateSetAllStates(OP_STATE_CHARGED);
					modStateOfChargeVoltageEvent(EVENT_FULL);
				}
			}else{
				modOperationalStateChargedTimeout = HAL_GetTick();
			};
		
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			if(modOperationalStatePackStatehandle->chargeAllowed){
				modPowerElectronicsSetCharge(true);
				if(modOperationalStatePackStatehandle->packCurrent >= 0.5f| modOperationalStatePackStatehandle->packCurrent >= modOperationalStateGeneralConfigHandle->chargerEnabledThreshold){
					modPowerElectronicsSetChargePFET(true);
				}
				
				#ifndef HWVersion_SS
				if(modOperationalStatePackStatehandle->packVoltage-modOperationalStatePackStatehandle->loCurrentLoadVoltage < (modOperationalStatePackStatehandle->packVoltage*0.1f) && modOperationalStatePackStatehandle->loCurrentLoadVoltage < (modOperationalStateGeneralConfigHandle->noOfCellsSeries*modOperationalStateGeneralConfigHandle->cellHardOverVoltage+10.0f)){ 
					modPowerElectronicsSetDisCharge(true);
					if(modOperationalStateGeneralConfigHandle->LCUsePrecharge==forced){
						modPowerElectronicsSetPreCharge(true);
					}
				}	
				#endif
			}else{
				modPowerElectronicsSetChargePFET(false);
				modPowerElectronicsSetCharge(false);
				modPowerElectronicsSetDisCharge(false);
				modPowerElectronicsSetPreCharge(false);
				modOperationalStatePackStatehandle->faultState = FAULT_CODE_CHARGE_RETRY;
			};
			
			//Cooling/Heating
			if(modOperationalStatePackStatehandle->coolingAllowed )
				modPowerElectronicsSetCooling(true);
			else{
				modPowerElectronicsSetCooling(false);
			}
			
			modOperationalStateUpdateStates();
			modOperationalStateDisplayData.StateOfCharge = modOperationalStateGeneralStateOfCharge->generalStateOfCharge;
			modOperationalStateDisplayData.CellMismatch = fabs(modOperationalStatePackStatehandle->cellVoltageMisMatch);
			modOperationalStateDisplayData.AverageCellVoltage = fabs(modOperationalStatePackStatehandle->cellVoltageAverage);
			modDisplayShowInfo(DISP_MODE_BALANCING,modOperationalStateDisplayData);
			#if DISABLE_POWER_LED
				modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_100_20);								// Indicate balancing
			#endif 
			break;
		case OP_STATE_CHARGED:
			// Sound the beeper indicating charging done
			modOperationalStateHandleChargerDisconnect(OP_STATE_POWER_DOWN);
			#if DISABLE_POWER_LED
				modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_1000_4);								// Indicate Charged
			#endif 
			modOperationalStateUpdateStates();
			modDisplayShowInfo(DISP_MODE_CHARGED,modOperationalStateDisplayData);
			break;
		case OP_STATE_FORCEON:
			if(modPowerElectronicsSetDisCharge(true))
				modPowerElectronicsSetPreCharge(false);
			else {
				modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
				modPowerElectronicsSetDisCharge(false);
			}
						
			if(fabs(modOperationalStatePackStatehandle->packCurrent) >= modOperationalStateGeneralConfigHandle->notUsedCurrentThreshold) {
				if(modDelayTick1ms(&modOperationalStateNotUsedResetDelay,1000))
					modOperationalStateNotUsedTime = HAL_GetTick();
			}else{
				modOperationalStateNotUsedResetDelay = HAL_GetTick();
			}
			
			if(modOperationalStatePowerDownDelayCheck()) {
				modOperationalStateSetNewFaultState(FAULT_CODE_NOT_USED_TIMEOUT);
				modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
				modOperationalStatePackStatehandle->powerDownDesired = true;
			}
			
			modDisplayShowInfo(DISP_MODE_FORCED_ON,modOperationalStateDisplayData);
			#if DISABLE_POWER_LED
				modEffectChangeState(STAT_LED_POWER,STAT_BLINKSHORTLONG_1000_4);								// Turn flash fast on debug and power LED
			#endif 
			modOperationalStateUpdateStates();
			break;
		default:
			modOperationalStateSetAllStates(OP_STATE_ERROR);
			break;
	};
	
	if(modPowerStateForceOnRequest()){
		modOperationalStateForceOn = true;
		modPowerElectronicsAllowForcedOn(true);
		modOperationalStateSetNewState(OP_STATE_PRE_CHARGE);
		driverSWStorageManagerEraseData();
	};
	
	// Check for power button longpress -> if so power down BMS
	if(modPowerStatePowerdownRequest()) {
		modOperationalStatePackStatehandle->powerDownDesired = true;
		
		if(modOperationalStateDelayedDisable(modOperationalStateGeneralConfigHandle->useCANDelayedPowerDown)) {
			modOperationalStateSetNewFaultState(FAULT_CODE_CAN_DELAYED_POWER_DOWN);
			modOperationalStateSetNewState(OP_STATE_POWER_DOWN);
			modDisplayShowInfo(DISP_MODE_POWEROFF,modOperationalStateDisplayData);
			modOperationalStateUpdateFaultStates();
			
		}
	};
	
	// In case of extreme cellvoltages or temperatures goto error state
	if((modOperationalStatePackStatehandle->packOperationalCellState == PACK_STATE_ERROR_HARD_CELLVOLTAGE || modOperationalStatePackStatehandle->packOperationalCellState == PACK_STATE_ERROR_TEMPERATURE) && (modOperationalStatePackStatehandle->packOperationalCellState != packOperationalCellStateLastErrorState) && !modOperationalStateForceOn){
		packOperationalCellStateLastErrorState = modOperationalStatePackStatehandle->packOperationalCellState; // Meganism to make error situation only trigger once
		modOperationalStateSetNewState(OP_STATE_ERROR);
		modOperationalStateUpdateStates();		
	}
	
	// In case of extreme currents goto error state
	if((modOperationalStatePackStatehandle->packOperationalCellState == PACK_STATE_ERROR_OVER_CURRENT) && (modOperationalStatePackStatehandle->packOperationalCellState != packOperationalCellStateLastErrorState)){
		packOperationalCellStateLastErrorState = modOperationalStatePackStatehandle->packOperationalCellState; // Meganism to make error situation only trigger once
		modOperationalStatePackStatehandle->faultState = FAULT_CODE_OVER_CURRENT;
		modOperationalStateSetNewState(OP_STATE_ERROR);	
		modOperationalStateUpdateStates();
	}
	
	// Move the button pressed state to the status struct
	modOperationalStatePackStatehandle->powerOnLongButtonPress = modPowerStateGetLongButtonPressState(); 
	
	// Handle subtask-display to update display content
	modDisplayTask();
};

void modOperationalStateUpdateStates(void) {
	modOperationalStateLastState = modOperationalStateCurrentState;
	modOperationalStatePackStatehandle->operationalState = modOperationalStateCurrentState = modOperationalStateNewState;
};

void modOperationalStateSetAllStates(OperationalStateTypedef newState) {
	modOperationalStatePackStatehandle->operationalState = modOperationalStateLastState = modOperationalStateCurrentState = modOperationalStateNewState = newState;
};

void modOperationalStateSetNewState(OperationalStateTypedef newState) {
	modOperationalStateNewState = newState;
};

void modOperationalStateHandleChargerDisconnect(OperationalStateTypedef newState) {
	if(modPowerStateChargerDetected()){
		modOperationalStateChargerDisconnectDetectDelay = HAL_GetTick();
	}else{
		if(modDelayTick1ms(&modOperationalStateChargerDisconnectDetectDelay,modOperationalStateGeneralConfigHandle->timeoutChargerDisconnected)){
			modOperationalStateSetAllStates(newState);
			modOperationalStatePackStatehandle->powerDownDesired = true;
		}
	}
};

void modOperationalStateTerminateOperation(void) {	
	// Store the state of charge data
	modStateOfChargePowerDownSave();																						// Store the SoC data
	
	// Disable the power supply
	modPowerStateSetState(P_STAT_RESET);																				// Turn off the power
}

bool modOperationalStateDelayedDisable(bool delayedPowerDownDesired) {
	if(delayedPowerDownDesired){
		if(modOperationalStatePackStatehandle->watchDogTime){
			if(modDelayTick1ms(&modOperationalStateWatchDogCountdownLastTick,1000))
				modOperationalStatePackStatehandle->watchDogTime--;
			
			return false;
		}else{
			return true;
		}
	}else{
	  return true;
	}
}

bool modOperationalStatePowerDownDelayCheck(void){
	return modDelayTick1ms(&modOperationalStateNotUsedTime,modOperationalStateGeneralConfigHandle->notUsedTimeout) && modOperationalStateGeneralConfigHandle->notUsedTimeout;
}

void modOperationalStateUpdateFaultStates(void) {
	modOperationalStateLastFaultState = modOperationalStateCurrentFaultState;
	modOperationalStatePackStatehandle->faultState = modOperationalStateCurrentFaultState = modOperationalStateNewFaultState;
};

void modOperationalStateSetAllFaultStates(bms_fault_state newFaultState) {
	modOperationalStatePackStatehandle->faultState = modOperationalStateLastFaultState = modOperationalStateCurrentFaultState = modOperationalStateNewFaultState = newFaultState;
};

void modOperationalStateSetNewFaultState(bms_fault_state newFaultState) {
	modOperationalStateNewFaultState = newFaultState;
};
