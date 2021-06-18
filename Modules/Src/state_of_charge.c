#include "state_of_charge.h"
#include "state_of_health.h"

typedef struct {
	float state_of_charge; // accuraty 0.1 


}; 

static modPowerElectronicsPackStateTypedef *pack_state;

bool state_of_charge_init(modPowerElectronicsPackStateTypedef *copy_pack_state)
{
	if (copy_pack_state != NULL) {
		pack_state = copy_pack_state;
		return true; 
	} else {
		return false; 
	}


}

void state_of_charge_task(void) 
{
	state_of_charge_calculate(); 
	state_of_charge_update(); 
}

/* update the state of charge info based on current reading from shunt 

todo: implement state of charge algorithm 

*/
static float state_of_charge_calculate()
{
	// state_of_charge_read_from_eeprom(); will give the current soc status
	
}

// update the soc value to main variable 
static float state_of_charge_update()
{
	pack_state->SoC = ; 
	pack_state->SoCCapacityAh = ; 


}

static void state_of_charge_store_info_to_eeprom();
// we need to store the SoC info in the EEPROM bacically is a flash memory 





