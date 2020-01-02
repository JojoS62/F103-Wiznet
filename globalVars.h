#ifndef __globalVars_h__
#define __globalVars_h__

#include "mbed.h"
#include "Rotor.h"

typedef struct 
{
    float adcValues[4];
    float rotorPosActual;
    float rotorPosSetpoint;
} GlobalVars;


extern Rotor rotor1;

extern GlobalVars globalVars;

#endif