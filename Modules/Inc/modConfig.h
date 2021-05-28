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

#ifndef __MODCONFIG_H
#define __MODCONFIG_H

#include "stdint.h"
#include "stdbool.h"
#include "mainDataTypes.h"
#include "driverSWStorageManager.h"
#include "generalDefines.h"

#define modConfigNoOfNTCTypes      5
#define modConfigNTCGroupLTCExt    2
#define modConfigNTCGroupMasterPCB 3
#define modConfigNTCGroupExp 4


typedef struct {
	// Master BMS
	uint8_t	 noOfCellsSeries;																											// Total number of cells in series in the battery pack
	uint8_t  noOfCellsParallel;                                                   // Number of cells in parallel group
	uint8_t  noOfCellsPerModule;                                                  // Number of cell levels monitored per LTC68XX
	uint8_t  noOfParallelModules;                                                 // Number of parallel modules
	float    batteryCapacity;																											// Battery capacity in Ah
	float    cellHardUnderVoltage;																								// If the lowest cell is under this voltage -> Error situation, turn all off and power down
	float    cellHardOverVoltage;																									// If the upper cell is above this voltage -> Error situation, turn all off and power down
	float    cellLCSoftUnderVoltage;																							// If the lowest cell is under this voltage -> disable low current load
	float    cellSoftOverVoltage;																								  // If the upper cell is above this voltage -> disable charging, but keep bms enabled
	float    cellBalanceDifferenceThreshold;																			// If the upper cell is more than this voltage away from the average -> start discharging this cell
	float    cellBalanceStart;																										// If an upper cell is above this voltage and higher than the cellBalanceDifferenceThreshold voltage then average, start discharging 
	bool 		 cellBalanceAllTime;																									// Enable balancing under all opstate
	float    cellThrottleUpperStart;																							// Charge throttle range
	float    cellThrottleLowerStart;																							// Discharge throttle rande
	float    cellThrottleUpperMargin;																							// Margin from the upper cell voltage extremes
  float    cellThrottleLowerMargin;                                             // Margin from the lower cell voltage extremes
	uint8_t  packVoltageDataSource;                                               // Enum value of pack voltage data source (what source to derive the voltage information from different ADC options)
	uint8_t  packCurrentDataSource;                                               // Enum value of pack current data source (what source to derive the current information from LC/HC/LC+HC/CAN)
	uint8_t  buzzerSignalSource;                                                  // - Stores what source shoud be taken to trigger
	uint8_t  buzzerSignalPersistant;                                              // - Stores whether the buzzer should stay on after triggering
	float    shuntLCFactor;                                                       // Shunt multiplication factor Low current
	float    voltageLCFactor;                                                     // Battery Voltage multiplication factor Low current
	int16_t  voltageLCOffset;                                                     // Battery Voltage low current offset
	float    loadVoltageFactor;                                                   // Load Voltage multiplication factor 
	float		 loadVoltageOffset;                                                   // Load Voltage offset
	float    chargerVoltageFactor;                                                // Charger Voltage multiplication factor 
	float		 chargerVoltageOffset;                                                // Charger Voltage offset
	uint8_t  throttleChargeIncreaseRate;																					// The rate of charge throttle percentage increase per 100ms
	uint8_t  throttleDisChargeIncreaseRate;                                       // The rate of discharge throttle percentage instrease per 100ms
	uint32_t cellBalanceUpdateInterval;																						// Amount of time that the balance resistor enable mask is kept
	uint8_t  maxSimultaneousDischargingCells;																			// Set the maximum amount of discharging cells. This is to limit dissepated power (and thus board temperature)
	uint32_t timeoutDischargeRetry;																								// If soft lower threshold limit was tripped wait this amount of time to re-enable load if cell is within threshold
	float    hysteresisDischarge;																									// If the lowest cell voltage rises this amount of mV re enable output
	uint32_t timeoutChargeRetry;																									// If soft higher threshold limit was tripped and cell is within acceptable limits wait this amount of time before re-enabling charge input
	float    hysteresisCharge;																										// If the highest cell voltage loweres this amount of mW re enable charge input
	uint32_t timeoutChargeCompleted;																							// If tricklecharging > this threshold timer declare the pack charged but keep balancing if nessesary
	uint32_t timeoutChargingCompletedMinimalMismatch;															// If charger is disabled and cellvoltagemismatch is under threshold determin charged after this timeout time
	float    maxMismatchThreshold;                                                // If the mismatch is below this threshold the battery SoC is set to 100% 
	float    chargerEnabledThreshold;																							// Minimal current to stay in charge mode
	uint32_t timeoutChargerDisconnected;																					// Timeout for charger disconnect detection
	float    minimalPrechargePercentage;																					// Output voltage threshold for precharging
	uint32_t timeoutLCPreCharge;																									// If threshold is not reached within this time in ms goto error state
	float    maxAllowedCurrent;																										// Max allowed current passing trough BMS, if limit is exceded disable output
	float    allowedTempBattDischargingMax;                                       // Max battery temperature where discharging is still allowed
	float    allowedTempBattDischargingMin;                                       // Min battery temperature where discharging is still allowed
	float    allowedTempBattChargingMax;                                          // Max battery temperature where charging is still allowed
	float    allowedTempBattChargingMin;                                          // Min battery temperature where heating is activated
	float    allowedTempBattCoolingMax;                                          	// Max battery temperature where cooling is activated
	float    allowedTempBattCoolingMin;                                          	// Min battery temperature where charging is still allowed
	float    allowedTempBMSMax;                                                   // Max BMS operational temperature
	float    allowedTempBMSMin;                                                   // Min BMS operational temperature
	uint32_t displayTimeoutBatteryDead;																						// Duration of displaying battery dead symbol
	uint32_t displayTimeoutBatteryError;																					// Duration of displaying error symbol
	uint32_t displayTimeoutBatteryErrorPreCharge;																	// Duration of displaying error symbol
	uint32_t displayTimeoutSplashScreen;																					// Duration of displaying splash screen + First few samples of ADC's
	uint8_t  displayStyle;                    				                            // Enum value for the SSD1306 display style option used
	uint8_t  maxUnderAndOverVoltageErrorCount;																		// Threshold that defines max amount of hard over / under voltage errors
	uint8_t  maxUnderAndOverTemperatureErrorCount;																// Threshold that defines max amount of hard over / under temperature errors
	float    notUsedCurrentThreshold;																							// Threshold that defines whether or not pack is in use.
	uint32_t notUsedTimeout;																											// Delay time that defines max amount of no operation on-time. When absolute battery curren < notUsedCurrentThreshold for longer than this amount of time -> the system is disabled
	uint32_t stateOfChargeStoreInterval;																					// Interval to store state of charge information.
	uint8_t  stateOfChargeMethod;																									// Stores the desired type of state of charge algorithm
	uint8_t  CANID;																																// Stores the CAN ID of the device.
	uint8_t  CANIDStyle;                                                          // Store the ID style used to communicate over CAN
	uint8_t  emitStatusOverCAN;                                                   // Enable or disable sending of status over CAN
	uint8_t  emitStatusProtocol;																									// The protocol type / format to send the status data
	uint32_t tempEnableMaskBMS;																								    // Stores the mask to select what temperature sensor is enabled for the BMS.
	uint32_t tempEnableMaskBattery;																								// Stores the mask to select what temperature sensor is enabled for the battery.
	uint32_t tempEnableMaskExpansion;																								// Stores the mask to select what temperature sensor is enabled for the battery.
	uint8_t  noOfTempSensorPerModule;                                             // Number of temperature sensor monitored per LTC68XX
	uint8_t  noOfExpansionBoard;                     							                // Number of expansion board
	uint8_t  noOfTempSensorPerExpansionBoard;                                     // Number of temperature sensor monitored per expansion board
	uint8_t  LCUseDischarge;                                                      // Enable or disable switch output.
	uint8_t  LCUsePrecharge;                                                      // choice whether to precharge or not.
	uint8_t  allowChargingDuringDischarge;																				// Allow charging during discharge.
	uint8_t  allowForceOn;                                                        // Enable or disable the option to be forced on.
	uint8_t  pulseToggleButton;                                                   // Pulse or Toggle power button.
	uint8_t  togglePowerModeDirectHCDelay;                                        // Toggle switch controls onstate directly or controls HC output with delayed turn off.
	uint8_t  useCANSafetyInput;                                                   // Use the safe input state received from the CAN bus.
	uint8_t  useCANDelayedPowerDown;                                              // Config that holds the preference whethet to used delayes power down
	uint8_t  cellMonitorType;                                  										// The cell monitor IC type
  uint8_t  cellMonitorICCount;																									// The amount of cell monitor IC's connected
	uint8_t	 lastICNoOfCells;																											// Store number of cells in the last module (Used for odd number of cells in series only)
	uint32_t lastICMask;																													// Stores the mask to select what cell should be monitored when last IC is not equal to other ones (Used for odd number of cells in series only)
	uint8_t  externalEnableOperationalState;                  										// The state to enter when externally enabled
	uint32_t powerDownDelay;																											// The delay time between going to power down and turning off
	uint8_t  canBusSpeed;																													// CAN bus baudrate
	uint8_t  chargeEnableOperationalState;                                        // State to enter when BMS is turned on due to charger
	uint32_t NTCTopResistor[modConfigNoOfNTCTypes];                               // NTC Pullup resistor value
	uint32_t NTC25DegResistance[modConfigNoOfNTCTypes];                           // NTC resistance at 25 degree
	uint16_t NTCBetaFactor[modConfigNoOfNTCTypes];                                // NTC Beta factor
	uint8_t  humidityICType;																													// Humidity sensor IC type
} modConfigGeneralConfigStructTypedef;

modConfigGeneralConfigStructTypedef* modConfigInit(void);
bool modConfigStoreAndLoadDefaultConfig(void);
bool modConfigStoreConfig(void);
bool modConfigLoadConfig(void);
bool modConfigStoreDefaultConfig(void);
void modConfigLoadDefaultConfig(modConfigGeneralConfigStructTypedef *configLocation);
void modconfigHardwareLimitsApply(modConfigGeneralConfigStructTypedef *configLocation);

#endif
