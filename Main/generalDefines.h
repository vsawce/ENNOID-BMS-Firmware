//#define HWVersion_SS

// Define the hardware version here:
#define ENNOID_LV
//#define ENNOID_HV
//#define ENNOID_SS

// Firmware version
#define FW_VERSION_MAJOR		1
#define FW_VERSION_MINOR		2
#define FW_REAL_VERSION			"V1.2"

// UUID
#define STM32_UUID					((uint32_t*)0x1FFFF7AC)
#define STM32_UUID_8				((uint8_t*)0x1FFFF7AC)

// Hardware names and limits

#ifdef ENNOID_LV
  #define HW_NAME "ENNOID-LV"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 18
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   18
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef ENNOID_HV
  #define HW_NAME "ENNOID-HV"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 18
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   18
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#ifdef ENNOID_SS
  #define HW_NAME "ENNOID-SS"
	#define HW_LIM_CELL_BALANCE_MAX_SIMULTANEOUS_DISCHARGE 18
	#define HW_LIM_CELL_MONITOR_IC_COUNT                   1
	#define HW_LIM_MIN_NOT_USED_DELAY                      5000
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
