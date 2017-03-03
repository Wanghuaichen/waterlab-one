/*
    Carl Lindquist
    Feb 27, 2017
    
    Interface for using float switches with the 
    PSoC 5LP. This could absolutely represent any two-state
    switch, the FSWITCH specification is only used for verbosity.

    This library is currently entirely event-driven. It has some
    debouncing built in.
*/

#include "floatSwitch.h"
#include <stdio.h>

#define DEBOUNCE_ARRAY_SIZE (FSWITCH_DEBOUNCE_PERIOD + 2)
#define NEW (FSWITCH_DEBOUNCE_PERIOD + 1)
#define OLD 0


typedef enum {
    FSWITCH_0 = 0x01,
    FSWITCH_1 = 0x02,
    FSWITCH_2 = 0x04,
    FSWITCH_3 = 0x08,
    FSWITCH_4 = 0x10,
    FSWITCH_5 = 0x20,
    FSWITCH_6 = 0x40,
    FSWITCH_7 = 0x80,
} FloatSwitchBitmasks;


//––––––  Private Declarations  ––––––//
CY_ISR_PROTO(FSwitch_ISR);
uint16 fswitchGetStates(void);
uint16 fswitchCheckEvents(void);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void floatSwitchInit(void) {
    FSwitch_Interrupt_StartEx(FSwitch_ISR);
    fswitchEvents = FSWITCH_EVENT_NONE;
}


uint16 fswitchCheckEvents(void) {
    static uint16 states[FSWITCH_DEBOUNCE_PERIOD + 1];
    states[NEW] = fswitchGetStates();
    
    /*  Remember your old value. This effectively buffers events
        so that an external machine can run slower than the interrupt */
    uint16 events = fswitchEvents;
    
    /* Shift your old values back for debouncing, check newer states for equality */
    int eventOccurred = 1;
    int i, j;
    for (i = OLD, j = OLD + 2; i < NEW; i++, j++) {
        if (j < NEW && eventOccurred) {
            if (states[j] != states[j+1]) { //tests for equality of the newest states
                eventOccurred = 0;
            }
            
        }
        states[i] = states[i + 1]; //shift values back, after checking for equality
    }

    if (states[OLD] == states[NEW]) {
        eventOccurred = 0;
    }
    
    if (eventOccurred) {
        /* For FSWITCH_ON events */
        if ( !(states[OLD] & FSWITCH_0) && (states[NEW] & FSWITCH_0) ) {
            events |= FSWITCH_EVENT_0_ON;
        }
        if ( !(states[OLD] & FSWITCH_1) && (states[NEW] & FSWITCH_1) ) {
            events |= FSWITCH_EVENT_1_ON;
        }
        if ( !(states[OLD] & FSWITCH_2) && (states[NEW] & FSWITCH_2) ) {
            events |= FSWITCH_EVENT_2_ON;
        }
        if ( !(states[OLD] & FSWITCH_3) && (states[NEW] & FSWITCH_3) ) {
            events |= FSWITCH_EVENT_3_ON;
        }
        
        /* For FSWITCH_OFF events */
        if ( (states[OLD] & FSWITCH_0) && !(states[NEW] & FSWITCH_0) ) {
            events |= FSWITCH_EVENT_0_OFF;
        }
        if ( (states[OLD] & FSWITCH_1) && !(states[NEW] & FSWITCH_1) ) {
            events |= FSWITCH_EVENT_1_OFF;
        }
        if ( (states[OLD] & FSWITCH_2) && !(states[NEW] & FSWITCH_2) ) {
            events |= FSWITCH_EVENT_2_OFF;
        }
        if ( (states[OLD] & FSWITCH_3) && !(states[NEW] & FSWITCH_3) ) {
            events |= FSWITCH_EVENT_3_OFF;
        } 
    }
    
    return events;
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Returns the current state of float switches in a uint16 with one-hot encoding.
        States are represented in the FloatSwitchBitmasks above.
    
[ret]   A uint16 representing states of the switches. Use with FloatSwitchBitmasks.
*/
uint16 fswitchGetStates(void) {
    uint16 state = 0x00;
    
    /*  Note that float switches read in backwards with the resistive
        drain configuration, hence the negation */
    if (!Switch0_In_Pin_Read()) {
        state |= FSWITCH_0;
    }
    if (!Switch1_In_Pin_Read()) {
        state |= FSWITCH_1;
    }
    if (!Switch2_In_Pin_Read()) {
        state |= FSWITCH_2;
    }
    if (!Switch3_In_Pin_Read()) {
        state |= FSWITCH_3;
    }
    return state;
}

/*
[desc]  ISR which adds events to the fswitchEvents buffer variable.
*/
CY_ISR(FSwitch_ISR) {
    fswitchEvents = fswitchCheckEvents();
}

//––––––––––––––––––––––––––––––  OUTDATED  ––––––––––––––––––––––––––––––//
#ifdef OUTDATED

    /* For FSWITCH_ON events */
    if ( !(prevStates & FSWITCH_0) && (curStates & FSWITCH_0) ) {
        events |= FSWITCH_EVENT_0_ON;
    }
    if ( !(prevStates & FSWITCH_1) && (curStates & FSWITCH_1) ) {
        events |= FSWITCH_EVENT_1_ON;
    }
    if ( !(prevStates & FSWITCH_2) && (curStates & FSWITCH_2) ) {
        events |= FSWITCH_EVENT_2_ON;
    }
    if ( !(prevStates & FSWITCH_3) && (curStates & FSWITCH_3) ) {
        events |= FSWITCH_EVENT_3_ON;
    }
    
    /* For FSWITCH_OFF events */
    if ( (prevStates & FSWITCH_0) && !(curStates & FSWITCH_0) ) {
        events |= FSWITCH_EVENT_0_OFF;
    }
    if ( (prevStates & FSWITCH_1) && !(curStates & FSWITCH_1) ) {
        events |= FSWITCH_EVENT_1_OFF;
    }
    if ( (prevStates & FSWITCH_2) && !(curStates & FSWITCH_2) ) {
        events |= FSWITCH_EVENT_2_OFF;
    }
    if ( (prevStates & FSWITCH_3) && !(curStates & FSWITCH_3) ) {
        events |= FSWITCH_EVENT_3_OFF;
    } 



/* An event has occurred */
if (states[1] == states[2] && states[2] == states[3] && states[3] == states[4] && states[0] != states[4]) {
    /* For FSWITCH_ON events */
    if ( !(states[OLD] & FSWITCH_0) && (states[NEW] & FSWITCH_0) ) {
        events |= FSWITCH_EVENT_0_ON;
    }
    if ( !(states[OLD] & FSWITCH_1) && (states[NEW] & FSWITCH_1) ) {
        events |= FSWITCH_EVENT_1_ON;
    }
    if ( !(states[OLD] & FSWITCH_2) && (states[NEW] & FSWITCH_2) ) {
        events |= FSWITCH_EVENT_2_ON;
    }
    if ( !(states[OLD] & FSWITCH_3) && (states[NEW] & FSWITCH_3) ) {
        events |= FSWITCH_EVENT_3_ON;
    }
    
    /* For FSWITCH_OFF events */
    if ( (states[OLD] & FSWITCH_0) && !(states[NEW] & FSWITCH_0) ) {
        events |= FSWITCH_EVENT_0_OFF;
    }
    if ( (states[OLD] & FSWITCH_1) && !(states[NEW] & FSWITCH_1) ) {
        events |= FSWITCH_EVENT_1_OFF;
    }
    if ( (states[OLD] & FSWITCH_2) && !(states[NEW] & FSWITCH_2) ) {
        events |= FSWITCH_EVENT_2_OFF;
    }
    if ( (states[OLD] & FSWITCH_3) && !(states[NEW] & FSWITCH_3) ) {
        events |= FSWITCH_EVENT_3_OFF;
    } 
}
        
#endif //OUTDATED

/* EOF */