/*
    Carl Lindquist
    Mar 3, 2017
*/

#ifndef TANK_H
#define TANK_H
    
#include "project.h"


#define TANK_0_ACTIVE
#define TANK_1_ACTIVE
//#define TANK_2_ACTIVE
//#define TANK_3_ACTIVE

#define FSWITCH_DEBOUNCE_PERIOD 4 /* Milliseconds */
    
typedef enum {
    TANK_EVENT_NONE = 0x00,
    TANK_EVENT_0_EMPTY = 0x01,
    TANK_EVENT_0_FULL = 0x02,
    TANK_EVENT_1_EMPTY = 0x04,
    TANK_EVENT_1_FULL = 0x08,
    TANK_EVENT_2_EMPTY = 0x10,
    TANK_EVENT_2_FULL = 0x20,
    TANK_EVENT_3_EMPTY = 0x40,
    TANK_EVENT_3_FULL = 0x80,
} TankEventFlags;

uint16 tankEvents;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initialization function for the tank library. Starts the interrupt
        for testing float switch events.
*/
void tankInit(void);


/*
[desc]  Tests whether or not a TANK_EVENT has occurred. Avoids the ugly
        'and-ing' of tankEvents and the event in question explicitly.

[tankEvent] A TankEventFlag to test for occurance
    
[ret]   1 if the event occurred, 0 otherwise.
*/
uint8 tankEventOccured(uint16 tankEvent);




#endif /* TANK_H */
