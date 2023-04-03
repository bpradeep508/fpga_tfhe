#ifdef POLYNOMIAL_FPGA_H
#define POLYNOMIAL_FPGA_H

struct IntPolynomial_FPGA {
    int32_t coefs[param_N];
} IntPolynomial_FPGA;

struct TorusPolynomial_FPGA {
    Torus32 coefsT[param_N];
} TorusPolynomial_FPGA;

struct LagrangeHalfCPolynomial_FPGA {
    cplx coefsC[param_Ns2];
} LagrangeHalfCPolynomial;

#endif 