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
    
#include "tank.h"
#include <stdio.h>

#define DEBOUNCE_ARRAY_SIZE (FSWITCH_DEBOUNCE_PERIOD + 2)
#define NEW (DEBOUNCE_ARRAY_SIZE - 1)
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
    
    FSWITCH_EVENT_4_ON = 0x100,
    FSWITCH_EVENT_4_OFF = 0x200,
    FSWITCH_EVENT_5_ON = 0x400,
    FSWITCH_EVENT_5_OFF = 0x800,
    FSWITCH_EVENT_6_ON = 0x1000,
    FSWITCH_EVENT_6_OFF = 0x2000,
    FSWITCH_EVENT_7_ON = 0x4000,
    FSWITCH_EVENT_7_OFF = 0x8000,
} FloatSwitchEventFlags;


//––––––  Private Declarations  ––––––//

uint16 fswitchGetStates(void);
uint16 fswitchCheckEvents(void);
uint8 fswtichEventOccurred(uint16 fsEvent, uint16 fswitchEvents);

uint8 tankDetermineState(uint8 emptySwitch, uint8 fullSwitch, uint16 fswitchStates);
uint16 tankCheckEvents(uint16 fswitchEvents);

CY_ISR_PROTO(FSwitch_ISR);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void tankInit(void) {
    FSwitch_Interrupt_StartEx(FSwitch_ISR);
    tankEvents = TANK_EVENT_NONE;
}


tankStruct tankGetStates(void) {
    uint16 fswitchStates = fswitchGetStates();
    tankStruct tankStates = {};
    
    #ifdef TANK_0_ACTIVE
        tankStates.tank[0] = tankDetermineState(FSWITCH_1, FSWITCH_0, fswitchStates);
    #endif
    
    #ifdef TANK_1_ACTIVE
        tankStates.tank[1] = tankDetermineState(FSWITCH_3, FSWITCH_2, fswitchStates);
    #endif
    
    #ifdef TANK_2_ACTIVE
        tankStates.tank[2] = tankDetermineState(FSWITCH_5, FSWITCH_4, fswitchStates);
    #endif
    
    #ifdef TANK_3_ACTIVE
        tankStates.tank[3] = tankDetermineState(FSWITCH_7, FSWITCH_6, fswitchStates);
    #endif
    
    return tankStates;
}


uint8 tankEventOccured(uint16 tankEventFlag) {
    return ((tankEvents & tankEventFlag) != 0);
}


uint8 tankClearEvent(uint16 tankEventFlag) {
    uint8 tmp = ((tankEvents & tankEventFlag) != 0);
    tankEvents &= (~tankEventFlag);
    return tmp;
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
    #ifdef TANK_0_ACTIVE
        if (!Switch0_In_Pin_Read()) {
            state |= FSWITCH_0;
        }
        if (!Switch1_In_Pin_Read()) {
            state |= FSWITCH_1;
        }
    #endif
    
    #ifdef TANK_1_ACTIVE
        if (!Switch2_In_Pin_Read()) {
            state |= FSWITCH_2;
        }
        if (!Switch3_In_Pin_Read()) {
            state |= FSWITCH_3;
        }
    #endif
    
    #ifdef TANK_2_ACTIVE
        if (!Switch4_In_Pin_Read()) {
            state |= FSWITCH_4;
        }
        if (!Switch5_In_Pin_Read()) {
            state |= FSWITCH_5;
        }
    #endif
    
    #ifdef TANK_3_ACTIVE
        if (!Switch6_In_Pin_Read()) {
            state |= FSWITCH_6;
        }
        if (!Switch7_In_Pin_Read()) {
            state |= FSWITCH_7;
        }
    #endif
    
    return state;
}


/*
[desc]  Returns a uint16 describing which events have occurred since the last time
        this function was called. This function has debouncing built in, though
        it is not strictly necessary in the case of float switches. This means that
        the function keeps track of states over multiple calls. The 
        FSWITCH_DEBOUNCE_PERIOD is the number of calls for which a state must be the 
        same before an event will be recognized.

[ret]   A uint16 representing all the events which have occurred since this function
        was last called. Use with the FloatSwitchEventFlags bitmasks in this file header.
*/
uint16 fswitchCheckEvents(void) {
    static uint16 states[DEBOUNCE_ARRAY_SIZE];
    states[NEW] = fswitchGetStates();
    
    uint16 events = FSWITCH_EVENT_NONE;
    
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
        
        /*  FSwitch event checker is split first by tank:
            TANK_0 has FSWITCH_0 (FULL) and FSWITCH_1 (EMPTY)
            Then split by going from OFF to ON and vice versa */
        #ifdef TANK_0_ACTIVE
            /* Going from OFF to ON */
            if ( !(states[OLD] & FSWITCH_0) && (states[NEW] & FSWITCH_0) ) {
                events |= FSWITCH_EVENT_0_ON;
            }
            if ( !(states[OLD] & FSWITCH_1) && (states[NEW] & FSWITCH_1) ) {
                events |= FSWITCH_EVENT_1_ON;
            }
            /* Going from ON to OFF */
            if ( (states[OLD] & FSWITCH_0) && !(states[NEW] & FSWITCH_0) ) {
            events |= FSWITCH_EVENT_0_OFF;
            }
            if ( (states[OLD] & FSWITCH_1) && !(states[NEW] & FSWITCH_1) ) {
                events |= FSWITCH_EVENT_1_OFF;
            }
        #endif
        
        #ifdef TANK_1_ACTIVE
            if ( !(states[OLD] & FSWITCH_2) && (states[NEW] & FSWITCH_2) ) {
                events |= FSWITCH_EVENT_2_ON;
            }
            if ( !(states[OLD] & FSWITCH_3) && (states[NEW] & FSWITCH_3) ) {
                events |= FSWITCH_EVENT_3_ON;
            }
                  
            if ( (states[OLD] & FSWITCH_2) && !(states[NEW] & FSWITCH_2) ) {
                events |= FSWITCH_EVENT_2_OFF;
            }
            if ( (states[OLD] & FSWITCH_3) && !(states[NEW] & FSWITCH_3) ) {
                events |= FSWITCH_EVENT_3_OFF;
            } 
        #endif
        
        #ifdef TANK_2_ACTIVE
            if ( !(states[OLD] & FSWITCH_4) && (states[NEW] & FSWITCH_4) ) {
                events |= FSWITCH_EVENT_4_ON;
            }
            if ( !(states[OLD] & FSWITCH_5) && (states[NEW] & FSWITCH_5) ) {
                events |= FSWITCH_EVENT_5_ON;
            }
                  
            if ( (states[OLD] & FSWITCH_4) && !(states[NEW] & FSWITCH_4) ) {
                events |= FSWITCH_EVENT_4_OFF;
            }
            if ( (states[OLD] & FSWITCH_5) && !(states[NEW] & FSWITCH_5) ) {
                events |= FSWITCH_EVENT_5_OFF;
            } 
        #endif
        
        #ifdef TANK_3_ACTIVE
            if ( !(states[OLD] & FSWITCH_6) && (states[NEW] & FSWITCH_6) ) {
                events |= FSWITCH_EVENT_6_ON;
            }
            if ( !(states[OLD] & FSWITCH_7) && (states[NEW] & FSWITCH_7) ) {
                events |= FSWITCH_EVENT_7_ON;
            }
                  
            if ( (states[OLD] & FSWITCH_6) && !(states[NEW] & FSWITCH_6) ) {
                events |= FSWITCH_EVENT_6_OFF;
            }
            if ( (states[OLD] & FSWITCH_7) && !(states[NEW] & FSWITCH_7) ) {
                events |= FSWITCH_EVENT_7_OFF;
            } 
        #endif
    }
    return events;
}


/*
[desc]  Tests whether or not an event has occurred. Avoids the ugly
        'and-ing' of fswitchEvents and the event in question explicitly.

[fsEvent] A FloatSwitchEventFlag to test the truth of.
    
[ret]   1 if the event occurred, 0 otherwise.
*/
uint8 fswtichEventOccurred(uint16 fsEvent, uint16 fswitchEvents) {
    return ((fswitchEvents & fsEvent) != 0);
}


/*
[desc]  Helper function for tankGetStates(). Analyzes a single tank's switches
        and returns its state.

[emptySwitch] The switch associated with a tank becoming empty. A FloatSwitchBitmask.
[fullSwitch] The switch associated with a tank becoming full. A FloatSwitchBitmask.
[fswitchStates] A uint16 representing the state of ALL switches. Use fswitchGetStates().

[ret]   A TankState (enum in header) representing the state of a tank.
*/
uint8 tankDetermineState(uint8 emptySwitch, uint8 fullSwitch, uint16 fswitchStates) {
    if ( !(fullSwitch & fswitchStates) && !(emptySwitch & fswitchStates) ) {
        return TANK_STATE_EMPTY;
    } else if ( !(fullSwitch & fswitchStates) && (emptySwitch & fswitchStates) ) {
        return TANK_STATE_MID;
    } else if ( (fullSwitch & fswitchStates) && (emptySwitch & fswitchStates) ) {
        return TANK_STATE_FULL;
    } else {
        return TANK_STATE_UNDEF;
    }
}


/*
[desc]  Returns a uint16 describing which events have occurred since the last time
        tankEvents was cleared. This function should be called with fswitchCheckEvents()
        as its argument inside an ISR. To interface with a state machine, this function's
        return should be stored into a public variable. Note that tankEvents must be cleared
        externally, they will never clear themselves.

[fswitchEvents] The float switch events used to determine check events. This must adhere
                to a specific enum format for bitmasking. Use fswitchCheckEvents().

[ret]   A uint16 representing all the tank events which have occurred since
        fswitchCheckEvents() was last called. Use with the TankEventFlags bitmasks
        in this file's header.
*/
uint16 tankCheckEvents(uint16 fswitchEvents) {
    uint16 events = TANK_EVENT_NONE;
    
    #ifdef TANK_0_ACTIVE
        if (fswtichEventOccurred(FSWITCH_EVENT_0_ON, fswitchEvents)) {
            events |= TANK_EVENT_0_FULL;
        }
        if (fswtichEventOccurred(FSWITCH_EVENT_1_OFF, fswitchEvents)) {
            events |= TANK_EVENT_0_EMPTY;
        }
    #endif
    
    #ifdef TANK_1_ACTIVE
        if (fswtichEventOccurred(FSWITCH_EVENT_2_ON, fswitchEvents)) {
            events |= TANK_EVENT_1_FULL;
        }
        if (fswtichEventOccurred(FSWITCH_EVENT_3_OFF, fswitchEvents)) {
            events |= TANK_EVENT_1_EMPTY;
        }
    #endif
    
    #ifdef TANK_2_ACTIVE
        if (fswtichEventOccurred(FSWITCH_EVENT_4_ON, fswitchEvents)) {
            events |= TANK_EVENT_2_FULL;
        }
        if (fswtichEventOccurred(FSWITCH_EVENT_5_OFF, fswitchEvents)) {
            events |= TANK_EVENT_2_EMPTY;
        }
    #endif
    
    #ifdef TANK_3_ACTIVE
        if (fswtichEventOccurred(FSWITCH_EVENT_6_ON, fswitchEvents)) {
            events |= TANK_EVENT_3_FULL;
        }
        if (fswtichEventOccurred(FSWITCH_EVENT_7_OFF, fswitchEvents)) {
            events |= TANK_EVENT_3_EMPTY;
        }
    #endif
        
    return events;
}


/*
[desc]  ISR which buffers events into the tankEvents variable. Note that tankEvents
        must be cleared external to this library by setting tankEvents = TANK_EVENT_NONE.
*/
CY_ISR(FSwitch_ISR) {
    tankEvents |= tankCheckEvents(fswitchCheckEvents());
}


/* EOF */