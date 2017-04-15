/*
    Carl Lindquist
    Mar 17, 2017

    Library for interfacing with the Atlas-Scientific EZO class circuits
    over I2C. This library will poll the sensors using a hardware timer 
    and store data points for immediate public access to recent readings.

    The I2C prefixed functions in this library are generic, and can be
    adapted for any I2C master functionality.
*/

#include "ezoProtocol.h"
#include <string.h>
#include <stdio.h>


//––––––  Private Variables  ––––––//
uint8 autoPollEn;
uint8 dataRequested;
double recentECData;
double recentDOData;


//––––––  Private Declarations  ––––––//

void i2cSendString(uint8 slaveAddress, char string[]);
arrStruct i2cReadString(uint8 slaveAddress);
CY_ISR_PROTO(I2C_DATA_ISR);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void ezoStart(void) {
    I2CM_Start();
    autoPollEn = 1;
    dataRequested = 0;
    One_Sec_Timer_Start();
    I2C_Data_Interrupt_StartEx(I2C_DATA_ISR);
}


void ezoSetAutoPoll(uint8 value) {
    if (value) {
        autoPollEn = 1;
    } else {
        autoPollEn = 0;
    }
}


arrStruct ezoSendAndPoll(uint8 slaveAddress, char string[], uint16 delay) {
    uint8 tmp = autoPollEn;
     
    while(dataRequested && autoPollEn); /* Wait for any auto polling to complete */
    autoPollEn = 0;
    
    i2cSendString(slaveAddress, string);
    CyDelay(delay);
    arrStruct response = i2cReadString(slaveAddress);
    
    autoPollEn = tmp;
    return response;
}


double ezoGetData(uint8 slaveAddress) {
    if (slaveAddress == EC_SENSOR_ADDRESS) {
        return recentECData;
    } else if (slaveAddress == DO_SENSOR_ADDRESS) {
        return recentDOData;    
    } else {
        return -1.0;
    }
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Sends a string to the slave specified by slaveAddress in I2C protocol. Assumes
		this device is set up as an I2C Master with a hardware block named I2CM.

[slaveAddress] Address of an I2C device between 0 and 127
[string] A string to send over I2C
*/
void i2cSendString(uint8 slaveAddress, char string[]) {
    /* Initiate transfer */
    uint8 temp = 0;
    do {
        I2CM_MasterWriteBuf(slaveAddress, (uint8*)string, strlen(string), I2CM_MODE_COMPLETE_XFER);
    } while(temp != I2CM_MSTR_NO_ERROR);
    
    while(I2CM_MasterStatus() & I2CM_MSTAT_XFER_INP); /* Wait for transfer to finish */
}


/*
[desc]  Reads a buffer from the slave specified by slaveAddress in I2C protocol. Assumes
		this device is set up as an I2C Master with a hardware block named I2CM.

[slaveAddress] Address of an I2C device between 0 and 127

[ret]	An arrStruct holding the buffer of data read from the slave.
*/
arrStruct i2cReadString(uint8 slaveAddress) {
    /* Initiate transfer */
    uint8 temp = 0;
//    char tempArray[MAX_RESPONSE_LENGTH] = {};
    arrStruct response = {};
    
    do {
		temp = I2CM_MasterReadBuf(slaveAddress, (uint8*)response.d, MAX_RESPONSE_LENGTH, I2CM_MODE_COMPLETE_XFER);
    } while (temp != I2CM_MSTR_NO_ERROR);
    
    while(I2CM_MasterStatus() & I2CM_MSTAT_XFER_INP); /* Wait for transfer to finish */
    
    /* Convert EZO status byte to a letter inside standard ASCII table */
    if (response.d[0] == 1) {           /* EZO SUCCESS */ 
        response.d[0] = 'S';    
    } else if (response.d[0] == 2) {    /* EZO ERROR, invalid command or other problem */
        response.d[0] = 'E';
    } else if (response.d[0] == 255) {  /* EZO NO data to send */
        response.d[0] = 'N';
    } else if (response.d[0] == 254) {  /* EZO PROCESSING, not ready */
        response.d[0] = 'P';
    }
    
    return response;
}


/*
[desc]  ISR which records data from both EZO sensors every other time it is called.
*/
CY_ISR(I2C_DATA_ISR) {
    if (autoPollEn) {
        if (!dataRequested) {
            /* Ask each sensor to take a reading */
            i2cSendString(EC_SENSOR_ADDRESS, "R");
            i2cSendString(DO_SENSOR_ADDRESS, "R"); 
        } else {
            LCD_ClearDisplay();
            char outstring[30] = {};
            /* Ask the sensors for the last readings they took */
            arrStruct ecResponse = i2cReadString(EC_SENSOR_ADDRESS);
            arrStruct doResponse = i2cReadString(DO_SENSOR_ADDRESS);
            
            /* Successful EC Read */
            if (ecResponse.d[0] == 'S' && 48 <= ecResponse.d[1] && ecResponse.d[1] <= 57) {
                sscanf(&ecResponse.d[1], "%lf", &recentECData);
                sprintf(outstring, "EC: %lf", recentECData);
                LCD_PrintString(outstring);
            }
            /* Successful DO Read */
            if (doResponse.d[0] == 'S' && 48 <= doResponse.d[1] && doResponse.d[1] <= 57) {
                sscanf(&doResponse.d[1], "%lf", &recentDOData);
                sprintf(outstring, "DO: %lf", recentDOData);
                LCD_Position(1,0);
                LCD_PrintString(outstring);
            }
        }
        dataRequested = ~dataRequested;
    }
    One_Sec_Timer_STATUS; /* Clear ISR */
}

/* EOF */