/*Parameters

*/

#ifndef PARAMETERS_h
#define PARAMETERS_h

#define END_OF_CHARGE_VOLTAGE           4.10 	//end of charge voltage for full charge mode
#define STORAGE_VOLTAGE 				        3.85 	//end of charge voltage for storage mode
#define END_OF_CHARGE_CURRENT_RATIO 	  0.05 	//percentage of charging current for triggering the end of charge state
#define END_OF_CHARGE_MIN_CURRENT       0.1 	// in Amp. Define the min charge current. Below this value, the end_of_charge state is trigerred
#define RAMP_SCALE                      0.07 	//current ramping when switching the charge on (in %/s)
#define STARTING_CURRENT                0.1 	//Starting current for charging. The ramp up begins at this value.
#define POWER_LIMIT                     400.0 	//Limit of power (not the DPS one, but can be useful for limiting the power in case of weaker power supply

//Timers / delays
#define BTN_ENTER_LONGPUSH_TIMER        2000	//Amount of time in ms for considering the enter button as "long push"
#define SLEEP_TIMER                     300000	//Dim both screens after that amount of time in Standby mode without any key press

//Safety
#define CELL_UNDERVOLTAGE               2.8 //Shut the output off if the average voltage per cell is below this threshold

#endif
