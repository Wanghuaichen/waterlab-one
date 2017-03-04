/*
    Carl Lindquist
    Feb 27, 2017

    Interface for using float switches with the 
    PSoC 5LP. This could absolutely represent any two-state
    switch, FSWITCH specification is only used for verbosity.

    This library is currently entirely event-driven. It has some
    debouncing built in.
*/

#ifndef FLOAT_SWITCH_H
#define FLOAT_SWITCH_H

#include "project.h"

#define FSWITCH_DEBOUNCE_PERIOD 4 /* Milliseconds */


typedef enum {
    FSWITCH_EVENT_NONE = 0x00,
    FSWITCH_EVENT_0_ON = 0x01,
    FSWITCH_EVENT_0_OFF = 0x02,
    FSWITCH_EVENT_1_ON = 0x04,
    FSWITCH_EVENT_1_OFF = 0x08,
    FSWITCH_EVENT_2_ON = 0x10,
    FSWITCH_EVENT_2_OFF = 0x20,
    FSWITCH_EVENT_3_ON = 0x40,
    FSWITCH_EVENT_3_OFF = 0x80,
} FloatSwitchEventFlags;

uint16 fswitchEvents;

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initialization function for the floatSwitch library. Starts the interrupt
        for testing float switch events.
*/
void floatSwitchInit(void);

/*
[desc]  Returns the current state of float switches in a uint16 with one-hot encoding.
        States are represented in the FloatSwitchBitmasks above.
    
[ret]   A uint16 representing states of the switches. Use with FloatSwitchBitmasks.
*/
uint16 fswitchGetStates(void);

/*
[desc]  #Macro for testing whether or not an event has occurred. Avoids the ugly
        'and-ing' of fswitchEvents and the event in question explicitly. Note that
        the fswitchEvents variable should be cleared of events externally, it will
        never clear itself.

[FS_EVENT] A FloatSwitchEventFlag to test the truth of.
    
[ret]   1 if the event occurred, 0 otherwise.
*/
#define fsEventOccured(FS_EVENT) ((fswitchEvents & FS_EVENT) != 0)



#endif /* FLOAT_SWITCH_H */