/*
    Carl Lindquist
    Mar 17, 2017
*/
    
#ifndef EZO_PROTOCOL_H
#define EZO_PROTOCOL_H

#include "project.h"
    
    
#define MAX_RESPONSE_LENGTH 128
    
typedef struct arrStruct { char r[MAX_RESPONSE_LENGTH]; } arrStruct;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//


void ezoStart(void);

void i2cSendString(uint8 slaveAddress, char string[]);

arrStruct i2cReadString(uint8 slaveAddress);






#endif /* EZO_PROTOCOL_H */