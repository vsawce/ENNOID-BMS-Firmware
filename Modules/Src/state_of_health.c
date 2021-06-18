#include "state_of_health.h"

#include "state_of_charge.h"

// our battery pack has a total capacity of 27Ah

// cycle time coloumb counting charge + discharge == 27Ah 
// battery_cycle_time++; 


// we need to measure the top capacity and voltage 
// for example each cell is 3000mAh and the max voltage is 4.2V
// by state of health if we only use 2700mAh charge battery to full the soh is 90%
// if soh is 100% and soc is 90% 
// 3000 * 100% (SoH) * 90%(SoC) = 2700mAh

// if soh is 90% and soc is 80%
3000 * 90% (SoH) * 80%(SoC) = 2700 * 0.8 = 2160mAh

uint16_t battery_cycle_time = 0; 

float soh = 99; // percentage of soh


void state_of_health_run_calibration();
