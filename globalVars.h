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

#include "FATFileSystem.h"
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"

extern SPIFBlockDevice spif;
extern LittleFileSystem fs;

void formatSPIFlash(FileSystem *fs); 
void print_SPIF_info();
void print_dir(FileSystem *fs, const char* dirname);

extern Rotor rotor1;

extern GlobalVars globalVars;

extern DigitalOut      led1;
extern DigitalOut      testPin;



#endif