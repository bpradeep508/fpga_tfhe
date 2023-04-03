#ifndef TLWEPARAMS_FPGA_H
#define TLWEPARAMS_FPGA_H

struct TLweParams_FPGA {
    int32_t N; ///< a power of 2: degree of the polynomials
    int32_t k; ///< number of polynomials in the mask
    double alpha_min; ///< minimal noise s.t. the sample is secure
    double alpha_max; ///< maximal noise s.t. we can decrypt
    LweParams_FPGA extracted_lweparams; ///< lwe params if one extracts
} TLweParams_FPGA;

#endif 