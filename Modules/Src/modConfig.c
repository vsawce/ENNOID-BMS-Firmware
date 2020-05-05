#include "modConfig.h"

modConfigGeneralConfigStructTypedef modConfigGeneralConfig;

modConfigGeneralConfigStructTypedef* modConfigInit(void) {
	driverSWStorageManagerConfigStructSize = (sizeof(modConfigGeneralConfigStructTypedef)/sizeof(uint16_t)); // Calculate the space needed for the config struct in EEPROM
	return &modConfigGeneralConfig;
};

bool modConfigStoreAndLoadDefaultConfig(void) {
	bool returnVal = false;
	if(driverSWStorageManagerConfigEmpty) {
		returnVal = modConfigStoreDefaultConfig();
	}
	
	modConfigLoadConfig();
	return returnVal;
};

bool modConfigStoreConfig(void) {
	return driverSWStorageManagerStoreStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
	// TODO_EEPROM
};

bool modConfigLoadConfig(void) {
	return driverSWStorageManagerGetStruct(&modConfigGeneralConfig,STORAGE_CONFIG);
};

bool modConfigStoreDefaultConfig(void) {
	// VALUES WILL ONLY AUTIMATICALLY UPDATE AFTER FLASH ERASE!
	modConfigGeneralConfigStructTypedef defaultConfig;
  modConfigLoadDefaultConfig(&defaultConfig);
	
	driverSWStorageManagerConfigEmpty = false;
	return driverSWStorageManagerStoreStruct(&defaultConfig,STORAGE_CONFIG);
}

void modconfigHardwareLimitsApply(modConfigGeneralConfigStructTypedef *configLocation) {
	configLocation->maxSimultaneousDischargingCells = MIN(configLocation->maxSimultaneousDischargingCells,HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE);
	configLocation->cellMonitorICCount              = MIN(configLocation->cellMonitorICCount             ,HW_LIM_CELL_MONITOR_IC_COUNT);
	configLocation->lastICMask 											= 0;
	
	if(configLocation->notUsedTimeout)
		configLocation->notUsedTimeout                = MAX(configLocation->notUsedTimeout                 ,HW_LIM_MIN_NOT_USED_DELAY);
	
// Check for feasable parameters
	if((configLocation->cellMonitorICCount % configLocation->noOfParallelModules) == 0){ // Check if feasable configuration
			if(configLocation->noOfCellsSeries % (configLocation->cellMonitorICCount / configLocation->noOfParallelModules) == 0){
				configLocation->noOfCellsPerModule = configLocation->noOfCellsSeries/(configLocation->cellMonitorICCount / configLocation->noOfParallelModules);
				configLocation->lastICNoOfCells = configLocation->noOfCellsPerModule;
				
				for(int bitPointer = 0; bitPointer < configLocation->lastICNoOfCells ; bitPointer++){
					configLocation->lastICMask = (configLocation->lastICMask << 1)| 1 ; // This contains the last cells mask  used for enabling UV & OV flag in the last serie IC 
				}
			}else{ //Odd number of cells in series
				configLocation->noOfCellsPerModule = (configLocation->noOfCellsSeries / (configLocation->cellMonitorICCount / configLocation->noOfParallelModules))+1;
				
				configLocation->lastICNoOfCells = configLocation->noOfCellsSeries % configLocation->noOfCellsPerModule; // This contains the last cells number monitored by the last serie IC 
				
				for(int bitPointer = 0; bitPointer < configLocation->lastICNoOfCells ; bitPointer++){
					configLocation->lastICMask = (configLocation->lastICMask << 1)| 1 ; // This contains the last cells mask  used for enabling UV & OV flag in the last serie IC 
				}	
			};			
		}else{
			configLocation->noOfCellsSeries    = 12;
			configLocation->cellMonitorICCount = 1;
			configLocation->noOfCellsPerModule = 1;
			configLocation->lastICNoOfCells = 0;
			configLocation->lastICMask = 0;
		};
		
		if (configLocation->noOfCellsPerModule <= 4){
			configLocation->noOfCellsSeries    = 12;
			configLocation->cellMonitorICCount = 1;
			configLocation->noOfCellsPerModule = 1;
			configLocation->lastICNoOfCells = 0;
			configLocation->lastICMask = 0;
		}
}

void modConfigLoadDefaultConfig(modConfigGeneralConfigStructTypedef *configLocation) {
	#ifdef ENNOID_LV
	configLocation->noOfCellsSeries														     = 12;											// Total number of cells in series in the battery pack
	configLocation->noOfCellsParallel                              = 10;                      // Number of cells in parallel
	configLocation->noOfCellsPerModule                             = 12;                      // Number of cell levels monitored per LTC68XX
	configLocation->noOfParallelModules                       	   = 1;                     	// Number of parallel modules
	configLocation->batteryCapacity														     = 22.00f;									// XXAh battery
	configLocation->cellHardUnderVoltage											     = 2.30f;										// Worst case X.XXV as lowest cell voltage
	configLocation->cellHardOverVoltage												     = 4.20f;										// Worst case X.XXV as highest cell voltage
	configLocation->cellLCSoftUnderVoltage											   = 2.70f;										// Lowest cell voltage X.XXV.
	configLocation->cellSoftOverVoltage												     = 4.10f;										// Normal highest cell voltage X.XXV.
	configLocation->cellBalanceDifferenceThreshold                 = 0.005f;										// Start balancing @ XmV difference, stop if below.
	configLocation->cellBalanceStart													     = 4.0f;										// Start balancing above X.XXV.
	configLocation->cellThrottleUpperStart										     = 0.03f;										// Upper range of cell voltage for charge throttling.
	configLocation->cellThrottleLowerStart										     = 0.20f;									  // Lower range of cell voltage for discharge throttling.
	configLocation->cellThrottleUpperMargin										     = 0.01f;										// Margin of throttle from upper soft limits.
	configLocation->cellThrottleLowerMargin										     = 0.50f;									  // Margin of throttle from lower soft limits.	
	configLocation->packVoltageDataSource                          = sourcePackVoltageISL28022_2_0X40_LVBatteryIn; // Packvoltage source.
	configLocation->packCurrentDataSource                          = sourcePackCurrentLowCurrentShunt; // The pack current is the same as the current through the low current shunt
	configLocation->buzzerSignalSource                             = buzzerSourceOff;         // Stores what source shoud be taken to trigger
	configLocation->buzzerSignalType                               = buzzerSignalTypeOn;      // Stores what sound pattern should be made
	configLocation->buzzerSingalPersistant                         = true;                    // Stores whether the buzzer should stay on after triggering
  configLocation->shuntLCFactor                                  = -0.051f;              		// Shunt factor low current
	configLocation->shuntLCOffset                                  = 2;                       // Shunt offset low current
	configLocation->voltageLCFactor	                               = 3.50f;                   // Pack voltage factor Master-HV: 125, Master-LV: 3.5
	configLocation->voltageLCOffset                                = 0;                       // Pack voltage offset 
	configLocation->loadVoltageFactor	                             = 49.2f;                   // Load voltage factor Master-HV: 135, Master-LV: 50
	configLocation->loadVoltageOffset                              = 0.0f;                    // Load voltage offset
	configLocation->chargerVoltageFactor	                         = 49.2f;                   // Charger voltage factor Master-HV: 135, Master-LV: 50
	configLocation->chargerVoltageOffset                           = 0.0f;                    // Charger voltage offset
	configLocation->throttleChargeIncreaseRate                     = 1;                       // Percentage charge throttle increase rate per 100ms (cell voltage loop time)  
	configLocation->throttleDisChargeIncreaseRate                  = 2;                       // Percentage discharge throttle increase rate per 100ms (cell voltage loop time)  	
	configLocation->cellBalanceUpdateInterval									     = 4*1000;									// Keep calculated resistors enabled for this amount of time in miliseconds.
	configLocation->maxSimultaneousDischargingCells						     = 5;												// Allow a maximum of X cells simultinous discharging trough bleeding resistors.
	configLocation->timeoutDischargeRetry											     = 10*1000;									// Wait for X seconds before retrying to enable load.
	configLocation->hysteresisDischarge 											     = 0.02f;										// Lowest cell should rise XXmV before output is re enabled.
	configLocation->timeoutChargeRetry												     = 100*1000;								// Wait for XX seconds before retrying to enable charger.
	configLocation->hysteresisCharge													     = 0.01f;										// Highest cell should lower XXmV before charger is re enabled.
	configLocation->timeoutChargeCompleted										     = 30*60*1000;							// Wait for XX minutes before setting charge state to charged.
	configLocation->timeoutChargingCompletedMinimalMismatch 	     = 6*1000;									// If cell mismatch is under threshold and (charging is not allowed) wait this delay time to set "charged" state.
	configLocation->maxMismatchThreshold											     = 0.010f;									// If mismatch is under this threshold for timeoutChargingCompletedMinimalMismatch determin fully charged.
	configLocation->chargerEnabledThreshold										     = 0.5f;										// If charge current > X.XA stay in charging mode and dont power off.
	configLocation->timeoutChargerDisconnected								     = 2000;										// Wait for X seconds to respond to charger disconnect.
	configLocation->minimalPrechargePercentage								     = 0.60f;										// output should be at a minimal of 80% of input voltage.
	configLocation->timeoutLCPreCharge												     = 1.5*1000;								// Precharge error timeout, allow 1.5 seconds pre-charge time before declaring load error.
	configLocation->maxAllowedCurrent													     = 120.0f;									// Allow max XXXA trough BMS.
	configLocation->allowedTempBattDischargingMax                  = 75.0f;                   // Max battery temperature where discharging is still allowed
	configLocation->allowedTempBattDischargingMin                  = 0.0f;                    // Min battery temperature where discharging is still allowed
	configLocation->allowedTempBattChargingMax                     = 50.0f;                   // Max battery temperature where charging is still allowed
	configLocation->allowedTempBattChargingMin                     = 0.0f;                    // Min battery temperature where charging is still allowed
	configLocation->allowedTempBattCoolingMax                      = 5.0f;                    // Max battery temperature where cooling is activated
	configLocation->allowedTempBattCoolingMin                      = 50.0f;                   // Min battery temperature where heating is activated
	configLocation->allowedTempBMSMax                              = 80.0f;                   // Max BMS operational temperature
	configLocation->allowedTempBMSMin                              = 0.0f;                    // Min BMS operational temperature
	configLocation->displayTimeoutBatteryDead									     = 5000;										// Show battery dead symbol X seconds before going to powerdown in cell voltage error state.
	configLocation->displayTimeoutBatteryError								     = 5000;										// Show error symbol for X seconds before going to powerdown in general error state.
	configLocation->displayTimeoutBatteryErrorPreCharge				     = 10000;										// Show pre charge error for XX seconds.
	configLocation->displayTimeoutSplashScreen								     = 3000;										// Display / INIT splash screen time.
	configLocation->maxUnderAndOverVoltageErrorCount 					     = 5;												// Max count of hard cell voltage errors.
	configLocation->notUsedCurrentThreshold										     = 1.0f;										// If abs(packcurrent) < X.XA consider pack as not used.
	configLocation->notUsedTimeout														     = 20*60*1000;							// If pack is not used for longer than XX minutes disable bms.
	configLocation->stateOfChargeStoreInterval								     = 60*1000;									// Interval in ms to store state of charge information.
	configLocation->stateOfChargeMethod                            = socCoulomb;              // Use coulomb counting for SoC calculation
	configLocation->CANID																			     = 10;											// CAN ID for CAN communication.
	configLocation->CANIDStyle                                     = CANIDStyleVESC;          // CAN ID default Style.
	configLocation->emitStatusOverCAN                              = false;                   // Send status over can.
	configLocation->emitStatusProtocol                             = canEmitProtocolDieBieEngineering; // Can emit protocol set to MG style for backwards compatibility
	configLocation->tempEnableMaskBMS                              = 0x0008;									// Bitwise select what sensor to enable for the BMS (internal sensors).
	configLocation->tempEnableMaskBattery                          = 0x0001;									// Bitwise select what sensor to enable for the battery (external sensors).
  configLocation->noOfTempSensorPerModule            						 = 1;												// Number of temperature sensors monitored per LTC68XX
	configLocation->LCUseDischarge                                 = enabled;                 // Enable or disable the solid state output
	configLocation->LCUsePrecharge                                 = enabled;              		// Use precharge before enabling main output
	configLocation->NTCTopResistor[modConfigNTCGroupLTCExt]        = 100000;                  // NTC Pullup resistor value
	configLocation->NTCTopResistor[modConfigNTCGroupMasterPCB]     = 100000;                  // NTC Pullup resistor value
	configLocation->NTC25DegResistance[modConfigNTCGroupLTCExt]    = 100000;                  // NTC resistance at 25 degree
	configLocation->NTC25DegResistance[modConfigNTCGroupMasterPCB] = 100000;                  // NTC resistance at 25 degree
	configLocation->NTCBetaFactor[modConfigNTCGroupLTCExt]         = 4250;                    // NTC Beta factor
	configLocation->NTCBetaFactor[modConfigNTCGroupMasterPCB]      = 4250;                    // NTC Beta factor
	configLocation->allowChargingDuringDischarge                   = true;                    // Allow the battery to be charged in normal mode
	configLocation->allowForceOn                                   = false;                   // Allow the BMS to be forced ON by long actuation of the power button
	configLocation->pulseToggleButton                              = true;                    // Select either pulse or toggle power button
	configLocation->togglePowerModeDirectHCDelay                   = true;                    // Select either direct power state control or HC output control with delayed turn off.
	configLocation->useCANSafetyInput                              = false;                   // Use the safety input status from CAN
	configLocation->useCANDelayedPowerDown                         = false;                   // Use delayed power down
	configLocation->cellMonitorType                                = CELL_MON_LTC6811_1;      // Use the new cell voltage monitor
	configLocation->cellMonitorICCount                             = 1;                       // Only one slave IC
	configLocation->lastICNoOfCells																 = 0;
	configLocation->lastICMask																		 = 0;
	configLocation->externalEnableOperationalState                 = opStateExtNormal;        // Go to normal enable mode
	configLocation->powerDownDelay                                 = 3000;                    // Wait only minimal to turn off
	configLocation->canBusSpeed                                    = canSpeedBaud500k;        // 500k CAN baud
	configLocation->chargeEnableOperationalState                   = opStateChargingModeCharging;// Go to charging mode when a charger is connected
#endif

#ifdef ENNOID_HV
	configLocation->noOfCellsSeries														     = 12;											// Total number of cells in series in the battery pack
	configLocation->noOfCellsParallel                              = 10;                      // Number of cells in parallel
	configLocation->noOfCellsPerModule                             = 12;                      // Number of cell levels monitored per LTC68XX
	configLocation->noOfParallelModules                       	   = 1;                     	// Number of parallel modules
	configLocation->batteryCapacity														     = 22.00f;									// XXAh battery
	configLocation->cellHardUnderVoltage											     = 2.30f;										// Worst case X.XXV as lowest cell voltage
	configLocation->cellHardOverVoltage												     = 4.20f;										// Worst case X.XXV as highest cell voltage
	configLocation->cellLCSoftUnderVoltage											   = 2.70f;										// Lowest cell voltage X.XXV.
	configLocation->cellSoftOverVoltage												     = 4.15f;										// Normal highest cell voltage X.XXV.
	configLocation->cellBalanceDifferenceThreshold                 = 0.01f;										// Start balancing @ XmV difference, stop if below.
	configLocation->cellBalanceStart													     = 4.1f;										// Start balancing above X.XXV.
	configLocation->cellThrottleUpperStart										     = 0.03f;										// Upper range of cell voltage for charge throttling.
	configLocation->cellThrottleLowerStart										     = 0.20f;									  // Lower range of cell voltage for discharge throttling.
	configLocation->cellThrottleUpperMargin										     = 0.01f;										// Margin of throttle from upper soft limits.
	configLocation->cellThrottleLowerMargin										     = 0.50f;									  // Margin of throttle from lower soft limits.	
	configLocation->packVoltageDataSource                          = sourcePackVoltageISL28022_2_0X40_LVBatteryIn; // Packvoltage source.
	configLocation->packCurrentDataSource                          = sourcePackCurrentLowCurrentShunt; // The pack current is the same as the current through the low current shunt
	configLocation->buzzerSignalSource                             = buzzerSourceOff;         // Stores what source shoud be taken to trigger
	configLocation->buzzerSignalType                               = buzzerSignalTypeOn;      // Stores what sound pattern should be made
	configLocation->buzzerSingalPersistant                         = true;                    // Stores whether the buzzer should stay on after triggering
  configLocation->shuntLCFactor                                  = -0.07f;              		// Shunt factor low current
	configLocation->shuntLCOffset                                  = -7;                       // Shunt offset low current
	configLocation->voltageLCFactor	                               = 50.00f;                   // Pack voltage factor Master-HV: 125, Master-LV: 3.5
	configLocation->voltageLCOffset                                = 0;                       // Pack voltage offset 
	configLocation->loadVoltageFactor	                             = 200.0f;                   // Load voltage factor Master-HV: 135, Master-LV: 50
	configLocation->loadVoltageOffset                              = 11.0f;                    // Load voltage offset
	configLocation->throttleChargeIncreaseRate                     = 1;                       // Percentage charge throttle increase rate per 100ms (cell voltage loop time)  
	configLocation->throttleDisChargeIncreaseRate                  = 2;                       // Percentage discharge throttle increase rate per 100ms (cell voltage loop time)  	
	configLocation->cellBalanceUpdateInterval									     = 4*1000;									// Keep calculated resistors enabled for this amount of time in miliseconds.
	configLocation->maxSimultaneousDischargingCells						     = 5;												// Allow a maximum of X cells simultinous discharging trough bleeding resistors.
	configLocation->timeoutDischargeRetry											     = 10*1000;									// Wait for X seconds before retrying to enable load.
	configLocation->hysteresisDischarge 											     = 0.02f;										// Lowest cell should rise XXmV before output is re enabled.
	configLocation->timeoutChargeRetry												     = 30*1000;									// Wait for XX seconds before retrying to enable charger.
	configLocation->hysteresisCharge													     = 0.01f;										// Highest cell should lower XXmV before charger is re enabled.
	configLocation->timeoutChargeCompleted										     = 30*60*1000;							// Wait for XX minutes before setting charge state to charged.
	configLocation->timeoutChargingCompletedMinimalMismatch 	     = 6*1000;									// If cell mismatch is under threshold and (charging is not allowed) wait this delay time to set "charged" state.
	configLocation->maxMismatchThreshold											     = 0.010f;									// If mismatch is under this threshold for timeoutChargingCompletedMinimalMismatch determin fully charged.
	configLocation->chargerEnabledThreshold										     = 0.5f;										// If charge current > X.XA stay in charging mode and dont power off.
	configLocation->timeoutChargerDisconnected								     = 2000;										// Wait for X seconds to respond to charger disconnect.
	configLocation->minimalPrechargePercentage								     = 0.80f;										// output should be at a minimal of 80% of input voltage.
	configLocation->timeoutLCPreCharge												     = 1.5*1000;								// Precharge error timeout, allow 1.5 seconds pre-charge time before declaring load error.
	configLocation->maxAllowedCurrent													     = 120.0f;									// Allow max XXXA trough BMS.
	configLocation->allowedTempBattDischargingMax                  = 75.0f;                   // Max battery temperature where discharging is still allowed
	configLocation->allowedTempBattDischargingMin                  = 0.0f;                    // Min battery temperature where discharging is still allowed
	configLocation->allowedTempBattChargingMax                     = 50.0f;                   // Max battery temperature where charging is still allowed
	configLocation->allowedTempBattChargingMin                     = 0.0f;                    // Min battery temperature where charging is still allowed
	configLocation->allowedTempBattCoolingMax                      = 5.0f;                    // Max battery temperature where cooling is activated
	configLocation->allowedTempBattCoolingMin                      = 50.0f;                   // Min battery temperature where heating is activated
	configLocation->allowedTempBMSMax                              = 80.0f;                   // Max BMS operational temperature
	configLocation->allowedTempBMSMin                              = 0.0f;                    // Min BMS operational temperature
	configLocation->displayTimeoutBatteryDead									     = 5000;										// Show battery dead symbol X seconds before going to powerdown in cell voltage error state.
	configLocation->displayTimeoutBatteryError								     = 5000;										// Show error symbol for X seconds before going to powerdown in general error state.
	configLocation->displayTimeoutBatteryErrorPreCharge				     = 10000;										// Show pre charge error for XX seconds.
	configLocation->displayTimeoutSplashScreen								     = 3000;										// Display / INIT splash screen time.
	configLocation->maxUnderAndOverVoltageErrorCount 					     = 5;												// Max count of hard cell voltage errors.
	configLocation->notUsedCurrentThreshold										     = 1.0f;										// If abs(packcurrent) < X.XA consider pack as not used.
	configLocation->notUsedTimeout														     = 20*60*1000;							// If pack is not used for longer than XX minutes disable bms.
	configLocation->stateOfChargeStoreInterval								     = 60*1000;									// Interval in ms to store state of charge information.
	configLocation->stateOfChargeMethod                            = socCoulomb;              // Use coulomb counting for SoC calculation
	configLocation->CANID																			     = 10;											// CAN ID for CAN communication.
	configLocation->CANIDStyle                                     = CANIDStyleVESC;          // CAN ID default Style.
	configLocation->emitStatusOverCAN                              = false;                   // Send status over can.
	configLocation->emitStatusProtocol                             = canEmitProtocolDieBieEngineering; // Can emit protocol set to MG style for backwards compatibility
	configLocation->tempEnableMaskBMS                              = 0x0008;									// Bitwise select what sensor to enable for the BMS (internal sensors).
	configLocation->tempEnableMaskBattery                          = 0x0001;									// Bitwise select what sensor to enable for the battery (external sensors).
  configLocation->noOfTempSensorPerModule            						 = 1;												// Number of temperature sensors monitored per LTC68XX
	configLocation->LCUseDischarge                                 = enabled;                 // Enable or disable the solid state output
	configLocation->LCUsePrecharge                                 = enabled;              		// Use precharge before enabling main output
	configLocation->NTCTopResistor[modConfigNTCGroupLTCExt]        = 100000;                  // NTC Pullup resistor value
	configLocation->NTCTopResistor[modConfigNTCGroupMasterPCB]     = 100000;                  // NTC Pullup resistor value
	configLocation->NTC25DegResistance[modConfigNTCGroupLTCExt]    = 100000;                  // NTC resistance at 25 degree
	configLocation->NTC25DegResistance[modConfigNTCGroupMasterPCB] = 100000;                  // NTC resistance at 25 degree
	configLocation->NTCBetaFactor[modConfigNTCGroupLTCExt]         = 4250;                    // NTC Beta factor
	configLocation->NTCBetaFactor[modConfigNTCGroupMasterPCB]      = 4250;                    // NTC Beta factor
	configLocation->allowChargingDuringDischarge                   = true;                    // Allow the battery to be charged in normal mode
	configLocation->allowForceOn                                   = false;                   // Allow the BMS to be forced ON by long actuation of the power button
	configLocation->pulseToggleButton                              = true;                    // Select either pulse or toggle power button
	configLocation->togglePowerModeDirectHCDelay                   = true;                    // Select either direct power state control or HC output control with delayed turn off.
	configLocation->useCANSafetyInput                              = false;                   // Use the safety input status from CAN
	configLocation->useCANDelayedPowerDown                         = false;                   // Use delayed power down
	configLocation->cellMonitorType                                = CELL_MON_LTC6811_1;      // Use the new cell voltage monitor
	configLocation->cellMonitorICCount                             = 1;                       // Only one slave IC
	configLocation->lastICNoOfCells																 = 0;
	configLocation->lastICMask																		 = 0;
	configLocation->externalEnableOperationalState                 = opStateExtNormal;        // Go to normal enable mode
	configLocation->powerDownDelay                                 = 3000;                    // Wait only minimal to turn off
	configLocation->canBusSpeed                                    = canSpeedBaud500k;        // 500k CAN baud
	configLocation->chargeEnableOperationalState                   = opStateChargingModeCharging;// Go to charging mode when a charger is connected
#endif
}

