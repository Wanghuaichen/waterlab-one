/*
    Carl Lindquist
    Feb 27, 2017
*/

#include "project.h"

#define FSWITCH_DEBOUNCE_PERIOD 4

#define fsEventOccured(FS_EVENT) ((fswitchEvents & FS_EVENT) != 0)

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

void floatSwitchInit(void);




/* EOF */