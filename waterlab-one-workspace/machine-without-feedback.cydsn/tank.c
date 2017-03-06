/*
    Carl Lindquist
    Mar 3, 2017
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
CY_ISR_PROTO(FSwitch_ISR);
uint16 fswitchCheckEvents(void);
uint8 fsEventOccured(uint16 fsEvent, uint16 fswitchEvents);
uint16 fswitchGetStates(void);
uint8 tankDetermineState(uint8 emptySwitch, uint8 fullSwitch, uint16 fswitchStates);
uint16 tankCheckEvents(uint16 fswitchEvents);



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

uint8 tankEventOccured(uint16 tankEvent) {
    return ((tankEvents & tankEvent) != 0);
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
[desc]  Returns a uin16 describing which events have occurred since the last time
        fswitchEvents was cleared. This function has debouncing built in, though
        it is not strictly necessary in the case of float switches.

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
        
        /*  FSwitch event checker is split first by tank -> TANK_0 has FSWITCH_0 and FSWITCH_1.
            Then split by going from ON to OFF and vice versa */
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
   ******************* NEEDS DESCRIPTION  ************************
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
   ******************* NEEDS DESCRIPTION  ************************
*/
uint16 tankCheckEvents(uint16 fswitchEvents) {
    uint16 events = TANK_EVENT_NONE;
    
    #ifdef TANK_0_ACTIVE
        if ( fsEventOccured(FSWITCH_EVENT_0_ON, fswitchEvents)) {
            events |= TANK_EVENT_0_FULL;
        }
        if ( fsEventOccured(FSWITCH_EVENT_1_OFF, fswitchEvents)) {
            events |= TANK_EVENT_0_EMPTY;
        }
    #endif
    
    #ifdef TANK_1_ACTIVE
        if ( fsEventOccured(FSWITCH_EVENT_2_ON, fswitchEvents)) {
            events |= TANK_EVENT_1_FULL;
        }
        if ( fsEventOccured(FSWITCH_EVENT_3_OFF, fswitchEvents)) {
            events |= TANK_EVENT_1_EMPTY;
        }
    #endif
    
    #ifdef TANK_2_ACTIVE
        if ( fsEventOccured(FSWITCH_EVENT_4_ON, fswitchEvents)) {
            events |= TANK_EVENT_2_FULL;
        }
        if ( fsEventOccured(FSWITCH_EVENT_5_OFF, fswitchEvents)) {
            events |= TANK_EVENT_2_EMPTY;
        }
    #endif
    
    #ifdef TANK_3_ACTIVE
        if ( fsEventOccured(FSWITCH_EVENT_6_ON, fswitchEvents)) {
            events |= TANK_EVENT_3_FULL;
        }
        if ( fsEventOccured(FSWITCH_EVENT_7_OFF, fswitchEvents)) {
            events |= TANK_EVENT_3_EMPTY;
        }
    #endif
        
    return events;
}


/*
[desc]  Tests whether or not an event has occurred. Avoids the ugly
        'and-ing' of fswitchEvents and the event in question explicitly.

[FS_EVENT] A FloatSwitchEventFlag to test the truth of.
    
[ret]   1 if the event occurred, 0 otherwise.
*/
uint8 fsEventOccured(uint16 fsEvent, uint16 fswitchEvents) {
    return ((fswitchEvents & fsEvent) != 0);
}


/*
[desc]  ISR which adds events to the tankEvents buffer variable.
*/
CY_ISR(FSwitch_ISR) {
    tankEvents |= tankCheckEvents(fswitchCheckEvents());
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
