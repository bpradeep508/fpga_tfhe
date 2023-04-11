#ifndef LWESAMPLES_FPGA_H
#define LWESAMPLES_FPGA_H

//#pragma once 

#include "constants.h"

typedef struct LweSample_FPGA_Struct {
	Torus32_FPGA a[N]; //-- the n coefs of the mask
    Torus32_FPGA b;  //
   	double current_variance; 
} LweSample_FPGA;

#endif 