/*
    Carl Lindquist
    Mar 17, 2017
*/
    
#include "ezoProtocol.h"
#include <string.h>
#include <stdio.h>
    
//––––––  Private Declarations  ––––––//
    

    
//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//
    
void ezoStart(void) {
    I2CM_Start();
}

void i2cSendString(uint8 slaveAddress, char string[]) {
    /* Initiate transfer */
    uint8 temp = 0;
    LED4_Pin_Write(1);
    do {
        I2CM_MasterWriteBuf(slaveAddress, (uint8*)string, strlen(string), I2CM_MODE_COMPLETE_XFER);
    } while(temp != I2CM_MSTR_NO_ERROR);
    LED4_Pin_Write(0);
    
    while(I2CM_MasterStatus() & I2CM_MSTAT_XFER_INP); /* Wait for transfer to finish */
}


arrStruct i2cReadString(uint8 slaveAddress) {
    /* Initiate transfer */
    uint8 temp = 0;
    char tempArray[MAX_RESPONSE_LENGTH] = {};
    arrStruct response = {};
    
    LED3_Pin_Write(1);
    do {
		temp = I2CM_MasterReadBuf(slaveAddress, (uint8*)tempArray, MAX_RESPONSE_LENGTH, I2CM_MODE_COMPLETE_XFER);
    } while (temp != I2CM_MSTR_NO_ERROR);
    LED3_Pin_Write(0);
  
    while(I2CM_MasterStatus() & I2CM_MSTAT_XFER_INP); /* Wait for transfer to finish */
    
    /* Convert EZO status byte to number */
    temp = tempArray[0];
    sprintf((char*)response.r, "%03d%s", tempArray[0], &tempArray[1]);
    return response;
}


    

    
//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

/* [] END OF FILE */
