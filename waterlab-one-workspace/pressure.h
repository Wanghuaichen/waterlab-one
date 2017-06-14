/*
    Carl Lindquist
    May 13, 2017

*/

#ifndef PRESSURE_H
#define PRESSURE_H

    
#include "project.h"
    
typedef enum {
    PSENSOR_ZERO,
    PSENSOR_ONE,
    PSENSOR_TWO,
    PSENSOR_THREE,
} PressureSensorIndexes;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initialization function for the pressure module.
*/
void pressureInit(void);

double getPressure(uint8 sensorIndex);

void setPSIRange(double min, double max);

void setMeasurementOhms(double ohms);



#endif /* PRESSURE_H */