#ifndef TLWESAMPLE_FPPGA_H
#define TLWESAMPLE_FPGA_H


struct TLweSampleFFT_FPGA {
    LagrangeHalfCPolynomial a[param_k + 1]; ///< array of length k+1: mask + right term
    // TODO: Reimplement `b` once needed...
    // LagrangeHalfCPolynomial_Collapsed b; ///< alias of a[k] to get the right term
    double current_variance; ///< avg variance of the sample
} TLweSampleFFT_FPGA;

struct TLweSample_FPGA {
    TorusPolynomial a[param_k + 1]; ///< array of length k+1: mask + right term
    // TODO: Reimplement `b` once needed...
    // TorusPolynomial *b; ///< alias of a[k] to get the right term
    double current_variance; ///< avg variance of the sample
} TLweSample_FPGA;

#endif