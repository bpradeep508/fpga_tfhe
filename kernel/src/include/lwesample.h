#ifdef LWESAMPLES_FPGA_H
#define LWESAMPLES_FPGA_H

#include "fpga_constans.h"

struct LweSample_FPGA {
	Torus32 a[param_N]; //-- the n coefs of the mask
    Torus32 b;  //
   	double current_variance; 
} LweSample_FPGA;

#endif 