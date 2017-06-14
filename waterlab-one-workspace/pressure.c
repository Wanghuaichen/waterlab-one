/*
    Carl Lindquist
    May 13, 2017

*/
    
#include "pressure.h"
#include <stdio.h>

#define DEFAULT_MAX_PSI 14.5
#define DEFAULT_MIN_PSI -14.5
#define DEFAULT_MAX_CUR 0.020
#define DEFAULT_MIN_CUR 0.004
#define DEFAULT_MEAS_RESISTOR 105.5

#define PRESSURE_AVG_SIZE 4
#define NEW (PRESSURE_AVG_SIZE - 1)

double resistorOhms;
double maxPSI;
double minPSI;
double maxCur;
double minCur;


//––––––  Private Declarations  ––––––//
CY_ISR_PROTO(Pressure_ISR);
double readPSI(uint8 sensorIndex);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void pressureInit(void) {
    maxPSI = DEFAULT_MAX_PSI;
    minPSI = DEFAULT_MIN_PSI;
    maxCur = DEFAULT_MAX_CUR;
    minCur = DEFAULT_MIN_CUR;
    resistorOhms = DEFAULT_MEAS_RESISTOR;
    
    ADC_Pressure_Start();
    ADC_Pressure_StartConvert();
    AMux_Pressure_Start();
}

double getPressure(uint8 sensorIndex) {
    AMux_Pressure_Select(sensorIndex);
    ADC_Pressure_StartConvert();
    ADC_Pressure_IsEndConversion(ADC_Pressure_WAIT_FOR_RESULT);
    double volts = ADC_Pressure_CountsTo_Volts(ADC_Pressure_GetResult16());
    return (volts/resistorOhms - minCur)*(maxPSI - minPSI)/(maxCur-minCur)  + minPSI ;    
}

void setPSIRange(double min, double max) {
    maxPSI = max;
    minPSI = min;
}

void setCurrentRange(double min, double max) {
    maxCur = max;
    minCur = min;
}

void setMeasurementOhms(double ohms) {
    resistorOhms = ohms;
}


/* EOF */