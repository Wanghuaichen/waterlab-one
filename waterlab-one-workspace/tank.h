/*
    Carl Lindquist
    Mar 3, 2017

    Interface for using tanks and float switches in a state-machine for the
    PSoC 5LP. Handles logic for interfacing with two float switches per tank, 
    one at the top and another at the bottom. This code is specific to the PSoC
    5LP. 

    For this library to work correctly, float switches must be mounted in such a
    fashion that they become closed (allowing current) when they float, and open
    (stopping current) when they are not floating.

    To avoid floating point errors, you can and should specify the number of tanks
    you are using (up to a maxiumum of MAX_TANK_COUNT), where each tank has two 
    float switches.

    This library has event-driven and state-based methods.
*/

#ifndef TANK_H
#define TANK_H
    
#include "project.h"


#define TANK_0_ACTIVE
#define TANK_1_ACTIVE
#define TANK_2_ACTIVE
#define TANK_3_ACTIVE
  
#define MAX_TANK_COUNT 4
#define FSWITCH_DEBOUNCE_PERIOD 4 /* Milliseconds */
    

typedef struct tankStruct { uint8 tank[MAX_TANK_COUNT]; } tankStruct;

typedef enum {
    TANK_STATE_EMPTY,
    TANK_STATE_MID,
    TANK_STATE_FULL,
    TANK_STATE_UNDEF,
} TankStates;
    
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
        for testing float switch events which is specific to the PSoC 5LP.
*/
void tankInit(void);


/*
[desc]  Returns the current state of the tanks in a struct containing a single array.
        The return type { tankStruct } is defined above. Use with the TANK_STATE enum
        above when checking the state of a tank.
            Example:
                if tank is the array returned within the tankStruct,
                TANK_1 is FULL if :: (tank[1] == TANK_STATE_FULL)
    
[ret]   A tankStruct holding a single array with the state of each tank. Use with the
        TankStates enum above.
*/
tankStruct tankGetStates(void);


/*
[desc]  Tests whether or not a TANK_EVENT has occurred. Avoids the ugly
        'and-ing' of tankEvents and the event in question explicitly. Note that
        tankEvents must be cleared externally by setting tankEvents to TANK_EVENT_NONE.

[tankEventFlag] A TankEventFlag to test for occurance
    
[ret]   1 if the event occurred, 0 otherwise.
*/
uint8 tankEventOccured(uint16 tankEventFlag);


/*
[desc]  Clears a specific event from the record of tank events. This 
        method also returns the value of the event before it was cleared.

[tankEventFlag] A TankEventFlag to check and clear.
    
[ret]   1 if the event occurred, 0 otherwise.
*/
uint8 tankClearEvent(uint16 tankEventFlag);



#endif /* TANK_H */