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
 
#include "modDisplay.h"

modDisplayInfoType modDisplayCurrentState;
uint32_t modDisplayLastRefresh;
uint32_t modDisplayStartupDelay;
uint32_t toggleButtonPressedLastTick;
bool     modDisplayPresent;
uint8_t modDisplayDispLoadShuffle;
bool toggleButtonPressed;
bool toggleButtonReleased;
bool toggleButtonPressedCounter;

extern modDisplayDataTypedef modDisplayData;

void modDisplayInit(void) {
	modDisplayStartupDelay = HAL_GetTick();
	libGraphicsInit(SSD1306_LCDWIDTH,SSD1306_LCDHEIGHT);
	while(!modDelayTick1ms(&modDisplayStartupDelay,STARTUPDELAY)) {};
	driverSWSSD1306Init(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	driverSWSSD1306ClearDisplay();
	driverSWSSD1306ClearDisplayBuffers();
		
	modDisplayPresent = true;
			
	modDisplayCurrentState = DISP_MODE_OFF;												//  Default content is nothing
	modDisplayLastRefresh = HAL_GetTick();
	toggleButtonPressedLastTick = HAL_GetTick();
	modDisplayDispLoadShuffle = 0;
};

void modDisplayShowInfo(modDisplayInfoType newState, modDisplayDataTypedef modDisplayData) {
	static modDisplayDataTypedef modDisplayDataLast;
	
	toggleButtonPressed = driverHWPowerStateReadInput(P_STAT_BUTTON_INPUT);
		if(toggleButtonPressed == 0){
			toggleButtonReleased = 1 ;
		}

		if(toggleButtonPressed && toggleButtonReleased && modDelayTick1ms(&toggleButtonPressedLastTick,500)){
			toggleButtonPressedLastTick = HAL_GetTick();
			modDisplayDispLoadShuffle++;
			toggleButtonReleased = 0;
			toggleButtonPressedCounter = true;
		}	
		
		if(modDisplayDispLoadShuffle == 9)
				modDisplayDispLoadShuffle = 0;
		
	if(((modDisplayCurrentState != newState || memcmp(&modDisplayDataLast,&modDisplayData,sizeof(modDisplayDataTypedef))) && ((modDelayTick1ms(&modDisplayLastRefresh,3000)) || modDisplayCurrentState != DISP_MODE_LOAD || newState != DISP_MODE_LOAD)) || toggleButtonPressedCounter) {											// Different state than last state?
		toggleButtonPressedCounter = false;
		memcpy(&modDisplayDataLast,&modDisplayData,sizeof(modDisplayDataTypedef));
		switch(newState) {
			case DISP_MODE_OFF:
				driverSWSSD1306ClearDisplay();
				break;
			case DISP_MODE_SPLASH:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_ENNOID],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);
				libGraphicsSetTextSize(2);
				libGraphicsSetTextColor_0(WHITE);
				libGraphicsSetCursor(10,SSD1306_LCDHEIGHT/2+14);

				libGraphicsWrite('V');
				libGraphicsWrite('1');
				libGraphicsWrite('.');
				libGraphicsWrite('2');
				break;
			case DISP_MODE_LOAD:
				driverSWSSD1306ClearDisplay();
				if(modDisplayData.DisplayStyle==0){
					driverSWSSD1306FillBuffer(libLogos[LOGO_LOAD],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);
					libGraphicsFillRect(7,7,(uint16_t)(modDisplayData.StateOfCharge/100*106),50,WHITE);
				}else{
					libGraphicsSetTextSize(2);
					if(modDisplayDispLoadShuffle == 0){
						libGraphicsSetTextColor_0(WHITE);
		
					//Display state of charge
						libGraphicsSetCursor(15,15);
						libGraphicsWrite('S');
						libGraphicsWrite('O');
						libGraphicsWrite('C');
						libGraphicsWrite(':');
				
						if(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
							libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge));
						}
						if(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
								libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge));
						}		
							libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.StateOfCharge));	
							libGraphicsWrite('%');
					}else if(modDisplayDispLoadShuffle == 1){
					//Display current
						libGraphicsSetCursor(15,15);
						libGraphicsWrite('I');
						libGraphicsWrite(':');
						if(modDisplayData.Current <= 0.0f){
							libGraphicsWrite('-');
						}
						if(modDisplay100ConvertValueToASCII(modDisplayData.Current)!= 48){
							libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.Current));
						}
						if(modDisplay10ConvertValueToASCII(modDisplayData.Current)!= 48 || modDisplay100ConvertValueToASCII(modDisplayData.Current)!= 48){
							libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.Current));
							}						 
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.Current));	
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.Current));
						libGraphicsWrite('A');							
					}else if(modDisplayDispLoadShuffle == 2){
					//Display Battery Voltage
						libGraphicsSetCursor(15,15);
						libGraphicsWrite('V');
						libGraphicsWrite(':');
						if(modDisplay100ConvertValueToASCII(modDisplayData.PackVoltage) !=48){
							libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.PackVoltage));	
						}
						if(modDisplay10ConvertValueToASCII(modDisplayData.PackVoltage)!=48 || modDisplay100ConvertValueToASCII(modDisplayData.PackVoltage) != 48){
							libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.PackVoltage));
						}
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.PackVoltage));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.PackVoltage));
						libGraphicsWrite('V');
					}else if (modDisplayDispLoadShuffle == 3){
				//Display cell Voltage high
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('C');
						libGraphicsWrite('V');
						libGraphicsWrite('H');
						libGraphicsWrite(':');
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.HighestCellVoltage));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.HighestCellVoltage));
						libGraphicsWrite(modDisplay0_01ConvertValueToASCII(modDisplayData.HighestCellVoltage));
						libGraphicsWrite(modDisplay0_001ConvertValueToASCII(modDisplayData.HighestCellVoltage));
						libGraphicsWrite('V');
					}else if(modDisplayDispLoadShuffle == 4){
				//Display cell Voltage low
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('C');
						libGraphicsWrite('V');
						libGraphicsWrite('L');
						libGraphicsWrite(':');
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.LowestCellVoltage));	
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.LowestCellVoltage));	
						libGraphicsWrite(modDisplay0_01ConvertValueToASCII(modDisplayData.LowestCellVoltage));	
						libGraphicsWrite(modDisplay0_001ConvertValueToASCII(modDisplayData.LowestCellVoltage));
						libGraphicsWrite('V');
					}else if(modDisplayDispLoadShuffle == 5){
				//Display Max battery temperature
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('T');
						libGraphicsWrite('m');
						libGraphicsWrite('a');
						libGraphicsWrite('x');
						libGraphicsWrite(':');
						if(modDisplayData.HighestTemp <= 0.0f){
								libGraphicsWrite('-');
							}
						if(modDisplay100ConvertValueToASCII(modDisplayData.HighestTemp) !=48){
								libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.HighestTemp));	
							}
						if(modDisplay10ConvertValueToASCII(modDisplayData.HighestTemp)!=48 || modDisplay100ConvertValueToASCII(modDisplayData.HighestTemp) != 48){
								libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.HighestTemp));
							}							
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.HighestTemp));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.HighestTemp));
						libGraphicsWrite('C');
					}else if(modDisplayDispLoadShuffle == 6){
				//Display Avg battery temperature
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('T');
						libGraphicsWrite('a');
						libGraphicsWrite('v');
						libGraphicsWrite('g');
						libGraphicsWrite(':');
						if(modDisplayData.AverageTemp <= 0.0f){
								libGraphicsWrite('-');
							}						
						if(modDisplay100ConvertValueToASCII(modDisplayData.AverageTemp) !=48){
								libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.AverageTemp));	
							}
						if(modDisplay10ConvertValueToASCII(modDisplayData.AverageTemp)!=48 || modDisplay100ConvertValueToASCII(modDisplayData.AverageTemp) != 48){
								libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.AverageTemp));
							}							
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.AverageTemp));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.AverageTemp));
						libGraphicsWrite('C');
					}else if(modDisplayDispLoadShuffle == 7){
					//Display low battery temperature
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('T');
						libGraphicsWrite('m');
						libGraphicsWrite('i');
						libGraphicsWrite('n');
						libGraphicsWrite(':');
						if(modDisplayData.LowestTemp <= 0.0f){
								libGraphicsWrite('-');
							}									
						if(modDisplay100ConvertValueToASCII(modDisplayData.LowestTemp) !=48){
								libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.LowestTemp));	
							}
						if(modDisplay10ConvertValueToASCII(modDisplayData.LowestTemp)!=48 || modDisplay100ConvertValueToASCII(modDisplayData.LowestTemp) != 48){
								libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.LowestTemp));
							}							
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.LowestTemp));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.LowestTemp));
						libGraphicsWrite('C');
					}else if (modDisplayDispLoadShuffle == 8){
					//Display humidity
						libGraphicsSetCursor(0,15);
						libGraphicsWrite('H');
						libGraphicsWrite('u');
						libGraphicsWrite('m');
						libGraphicsWrite(':');					
						if(modDisplay100ConvertValueToASCII(modDisplayData.Humidity) !=48){
								libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.Humidity));	
							}
						if(modDisplay10ConvertValueToASCII(modDisplayData.Humidity)!=48 || modDisplay100ConvertValueToASCII(modDisplayData.Humidity) != 48){
								libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.Humidity));
							}							
						libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.Humidity));
						libGraphicsWrite('.');
						libGraphicsWrite(modDisplay0_1ConvertValueToASCII(modDisplayData.Humidity));
						libGraphicsWrite('%');
						}
				};
				break;
			case DISP_MODE_CHARGE:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_CHARGING],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);
				libGraphicsSetTextSize(1);
				libGraphicsSetCursor(15,43);
				if(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
					libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge));
				}
				if(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
					libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge));
				}		
				libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.StateOfCharge));	
				libGraphicsWrite('%');
				break;
			case DISP_MODE_POWEROFF:
				driverSWSSD1306ClearDisplay();
				libGraphicsSetTextSize(2);
				libGraphicsSetTextColor_0(WHITE);
				libGraphicsSetCursor(SSD1306_LCDWIDTH/2-16,SSD1306_LCDHEIGHT/2-6);
			
				libGraphicsWrite('B');  
				libGraphicsWrite('y');  
				libGraphicsWrite('e');  
				break;
			case DISP_MODE_ERROR:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_ERROR],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);  // Error logo uses half of the screen
			
				libGraphicsSetTextSize(1);
				libGraphicsSetTextColor_0(WHITE);
				libGraphicsSetCursor(68,0);																						// Display text on the other side of the screen
			
				libGraphicsWrite('E');  
				libGraphicsWrite('R');  
				libGraphicsWrite('R');  
				libGraphicsWrite('O');  
				libGraphicsWrite('R');  
				break;
			case DISP_MODE_ERROR_PRECHARGE:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_ERROR],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);  // Error logo uses half of the screen
			
				libGraphicsSetTextSize(1);
				libGraphicsSetTextColor_0(WHITE);
				libGraphicsSetCursor(68,0);																						// Display text on the other side of the screen
			
				libGraphicsWrite('E');  
				libGraphicsWrite('R');  
				libGraphicsWrite('R');  
				libGraphicsWrite('O');  
				libGraphicsWrite('R');  
			
				libGraphicsSetCursor(68,15);																						// Display text on the other side of the screen
			
				libGraphicsWrite('P');
				libGraphicsWrite('R');
				libGraphicsWrite('E');
				libGraphicsWrite('C');
				libGraphicsWrite('H');
				libGraphicsWrite('A');
				libGraphicsWrite('R');
				libGraphicsWrite('G');
				libGraphicsWrite('E');
				break;			
			case DISP_MODE_EXTERNAL:
				driverSWSSD1306ClearDisplay();
				libGraphicsSetTextSize(1);
				libGraphicsSetTextColor_0(WHITE);
				libGraphicsSetCursor(0,0);
			
				libGraphicsWrite('E');  
				libGraphicsWrite('x');  
				libGraphicsWrite('t');  
				break;
			case DISP_MODE_BATTERY_DEAD:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_BATTERY_DEAD],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);  
				break;
			case DISP_MODE_BALANCING:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_CHARGING],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);
				libGraphicsSetTextSize(0);
				libGraphicsSetTextColor_0(INVERSE);
				libGraphicsSetCursor(35,0);
			
				libGraphicsWrite('B');  
				libGraphicsWrite('A');  
				libGraphicsWrite('L'); 
				libGraphicsWrite('A');  
				libGraphicsWrite('N');  
				libGraphicsWrite('C'); 
				libGraphicsWrite('I');  
				libGraphicsWrite('N');  
				libGraphicsWrite('G');
				libGraphicsSetTextSize(1);
				libGraphicsSetCursor(15,43);
				if(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
						libGraphicsWrite(modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge));
				}
				if(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertValueToASCII(modDisplayData.StateOfCharge) !=48){
						libGraphicsWrite(modDisplay10ConvertValueToASCII(modDisplayData.StateOfCharge));
				}		
				libGraphicsWrite(modDisplay1ConvertValueToASCII(modDisplayData.StateOfCharge));	
				libGraphicsWrite('%');
				break;
			case DISP_MODE_CHARGED:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_BATTERY_FULL],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8); 
				break;
			case DISP_MODE_FORCED_ON:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_FORCEON],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);			
				break;
			default:
				break;
		};
		modDisplayLastRefresh = HAL_GetTick();
	}
	
	modDisplayCurrentState = newState;
};

void modDisplayTask(void) {
	static uint32_t displayRefreshLastTick;
	
	if(modDelayTick1ms(&displayRefreshLastTick,10000)){
		if(!modDisplayPresent){
			driverSWSSD1306Init(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
			modDisplayPresent = true;
		}
		driverSWSSD1306ClearDisplayBuffers();
	}
	
	if(driverSWSSD1306DisplayAsync() != HAL_OK)
		modDisplayPresent = false;
};

float modDisplay100ConvertValueToASCII(float value) {
		value = (int)value/100;
		value = value+48;
	return value;
};

float modDisplay10ConvertValueToASCII(float value) {
		value = (int)value % 100;
		value = (int)value/10;
		value = value+48;
	return value;
};

float modDisplay1ConvertValueToASCII(float value) {
		value = (int)value % 10;
		value = value+48;
	return value;
};

float modDisplay0_1ConvertValueToASCII(float value) {
		value = (int)(value*10.0f) % 10;
		value = value+48;
	return value;
};

float modDisplay0_01ConvertValueToASCII(float value) {
		value = (int)(value*100.0f) % 10;
		value = value+48;
	return value;
};

float modDisplay0_001ConvertValueToASCII(float value) {
		value = (int)(value*1000.0f) % 10;
		value = value+48;
	return value;
};
