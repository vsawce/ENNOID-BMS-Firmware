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
bool     modDisplayPresent;

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
};

void modDisplayShowInfo(modDisplayInfoType newState, modDisplayDataTypedef modDisplayData) {
	static modDisplayDataTypedef modDisplayDataLast;
	
	if(((modDisplayCurrentState != newState) || memcmp(&modDisplayDataLast,&modDisplayData,sizeof(modDisplayDataTypedef))) && modDelayTick1ms(&modDisplayLastRefresh,REFRESHTIMOUT && newState!=DISP_MODE_SPLASH)) {											// Different state than last state?
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
					libGraphicsSetTextSize(1);
					libGraphicsSetTextColor_0(WHITE);
		
			//Display state of charge
					libGraphicsSetCursor(0,7);
					libGraphicsWrite('S');
					libGraphicsWrite('O');
					libGraphicsWrite('C');
					libGraphicsWrite(':');
			
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
						libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge));
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge));
						}		
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.StateOfCharge));	
					libGraphicsWrite('%');
					
			//Display current
					libGraphicsSetCursor(0,17);
					libGraphicsWrite('I');
					libGraphicsWrite(':');
					if(modDisplayData.Current <= 0.0f){
							libGraphicsWrite('-');
						}
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.Current)!= 48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.Current));
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.Current)!= 48 || modDisplay100ConvertIntegerToASCII(modDisplayData.Current)!= 48){
						libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.Current));
						}						 
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.Current));	

				//Display Battery Voltage
					libGraphicsSetCursor(0,27);
					libGraphicsWrite('V');
					libGraphicsWrite(':');
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.PackVoltage) !=48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.PackVoltage));	
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.PackVoltage)!=48 || modDisplay100ConvertIntegerToASCII(modDisplayData.PackVoltage) != 48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.PackVoltage));
						}
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.PackVoltage));
	/*					
				//Display cell Voltage high
					libGraphicsSetCursor(0,37);
					libGraphicsWrite('C');
					libGraphicsWrite('V');
					libGraphicsWrite('H');
					libGraphicsWrite(':');
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.HighestCellVoltage));
						
				//Display cell Voltage low
					libGraphicsSetCursor(0,47);
					libGraphicsWrite('C');
					libGraphicsWrite('V');
					libGraphicsWrite('L');
					libGraphicsWrite(':');
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.LowestCellVoltage));	
		*/				
			//Display Max battery temperature
					libGraphicsSetTextSize(1);
					libGraphicsSetCursor(74,07);
					libGraphicsWrite('T');
					libGraphicsWrite('m');
					libGraphicsWrite('a');
					libGraphicsWrite('x');
					libGraphicsWrite(':');
					if(modDisplayData.HighestTemp <= 0.0f){
							libGraphicsWrite('-');
						}
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.HighestTemp) !=48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.HighestTemp));	
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.HighestTemp)!=48 || modDisplay100ConvertIntegerToASCII(modDisplayData.HighestTemp) != 48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.HighestTemp));
						}							
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.HighestTemp));
						
			//Display Avg battery temperature
					libGraphicsSetCursor(74,17);
					libGraphicsWrite('T');
					libGraphicsWrite('a');
					libGraphicsWrite('v');
					libGraphicsWrite('g');
					libGraphicsWrite(':');
					if(modDisplayData.AverageTemp <= 0.0f){
							libGraphicsWrite('-');
						}						
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.AverageTemp) !=48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.AverageTemp));	
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.AverageTemp)!=48 || modDisplay100ConvertIntegerToASCII(modDisplayData.AverageTemp) != 48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.AverageTemp));
						}							
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.AverageTemp));
						
				//Display low battery temperature
					libGraphicsSetCursor(74,27);
					libGraphicsWrite('T');
					libGraphicsWrite('m');
					libGraphicsWrite('i');
					libGraphicsWrite('n');
					libGraphicsWrite(':');
					if(modDisplayData.LowestTemp <= 0.0f){
							libGraphicsWrite('-');
						}									
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.LowestTemp) !=48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.LowestTemp));	
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.LowestTemp)!=48 || modDisplay100ConvertIntegerToASCII(modDisplayData.LowestTemp) != 48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.LowestTemp));
						}							
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.LowestTemp));
				
				//Display humidity
					libGraphicsSetCursor(74,37);
					libGraphicsWrite('H');
					libGraphicsWrite('u');
					libGraphicsWrite('m');
					libGraphicsWrite(':');					
					if(modDisplay100ConvertIntegerToASCII(modDisplayData.Humidity) !=48){
							libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.Humidity));	
						}
					if(modDisplay10ConvertIntegerToASCII(modDisplayData.Humidity)!=48 || modDisplay100ConvertIntegerToASCII(modDisplayData.Humidity) != 48){
							libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.Humidity));
						}							
					libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.Humidity));
						
				};
				break;
			case DISP_MODE_CHARGE:
				driverSWSSD1306ClearDisplay();
				driverSWSSD1306FillBuffer(libLogos[LOGO_CHARGING],SSD1306_LCDHEIGHT*SSD1306_LCDWIDTH/8);
				libGraphicsSetTextSize(1);
				libGraphicsSetCursor(15,43);
				if(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
					libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge));
				}
				if(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
					libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge));
				}		
				libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.StateOfCharge));	
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
				if(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
						libGraphicsWrite(modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge));
				}
				if(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge) != 48 || modDisplay100ConvertIntegerToASCII(modDisplayData.StateOfCharge) !=48){
						libGraphicsWrite(modDisplay10ConvertIntegerToASCII(modDisplayData.StateOfCharge));
				}		
				libGraphicsWrite(modDisplay1ConvertIntegerToASCII(modDisplayData.StateOfCharge));	
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

float modDisplay100ConvertIntegerToASCII(float value) {
		value = (int)value/100;
		value = value+48;
	return value;
};

float modDisplay10ConvertIntegerToASCII(float value) {
		value = (int)value % 100;
		value = (int)value/10;
		value = value+48;
	return value;
};

float modDisplay1ConvertIntegerToASCII(float value) {
		value = (int)value % 10;
		value = value+48;
	return value;
};
