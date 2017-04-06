/*
    Carl Lindquist
    Mar 17, 2017

    Library for interfacing with the Atlas-Scientific EZO class circuits
    over I2C. This library will poll the sensors using a hardware timer 
    and store data points for immediate public access to recent readings.
*/
    
#ifndef EZO_PROTOCOL_H
#define EZO_PROTOCOL_H

#include "project.h"
    
    
#define MAX_RESPONSE_LENGTH 128
#define EC_SENSOR_ADDRESS 100
#define DO_SENSOR_ADDRESS 97
    

typedef struct arrStruct { char d[MAX_RESPONSE_LENGTH]; } arrStruct;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initialization code for the ezoLibrary. This library's funcationality is
		undefined if this is not called before other functions.
*/
void ezoStart(void);


/*
[desc]  Turns auto polling for data from the two EZO sensors ON or OFF
        according to the value passed in. Passing in a 0 disables 
        autopolling, anything else will enable it.

[value] Value to set autopolling to.
*/
void ezoSetAutoPoll(uint8 value);


/*
[desc]  Use this function to send single requests to a slave device. This
        method will wait until any autopolling is finished, then send 'string'
        to the 'slaveAddress'. It will then wait for 'delay' amount of time
        to read data from the slave.

[slaveAddress] Slave address to send and request from.
[string] A string to send to the slave.
[delay] Time is uS to wait between sending and requesting.

[ret]   An arrStruct containing the slave's response.
*/
arrStruct ezoSendAndPoll(uint8 slaveAddress, char string[], uint16 delay);

/*
[desc]  Returns the most recently recorded data from a sensor addressed by
        its I2C slave address. Note that this function does not directly
        communicate with a sensor, as data is collected automatically every
        2 seconds from each sensor. This method is non-blocking.

[slaveAddress] The I2C slave address of to ask for data.

[ret] The most recently recorded data from the slave sensor.
*/
double ezoGetData(uint8 slaveAddress);











#endif /* EZO_PROTOCOL_H */