#ifndef LWEPARAMS_FPGA_H
#define LWEPARAMS_FPGA_H

struct LweParams_FPGA {
	int32_t n;
	double alpha_min;//le plus petit bruit tq sur
	double alpha_max;//le plus gd bruit qui permet le déchiffrement
} LweParams_FPGA;

#endif 