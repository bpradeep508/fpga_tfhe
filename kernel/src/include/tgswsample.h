#ifndef TGSWSAMPLE_FPGA_H
#define TGSWSAMPLE_FPGA_H

#pragma once 
#include "constants.h"
#include "tlwesample.h"

typedef struct TGswSampleFFT_FPGA {
    TLweSampleFFT_FPGA all_samples[(k+1) * bk_l]; ///< TLweSample* all_sample; (k+1)l TLwe Sample
    // TODO: Reimplement `sample` when needed
    // TLweSampleFFT_FPGA *sample; ///< accès optionnel aux différents blocs de taille l. (optional access to the various blocks of size l.)
    //double current_variance;
    //int32_t k;
    //int32_t l;

    //TGswSampleFFT_FPGA(): all_samples{{}, {}, {}, {}, {}, {}}, k(0), l(0) {}

    //TGswSampleFFT_FPGA(const TGswSampleFFT_FPGA *other): all_samples{other->all_samples[0], other->all_samples[1], other->all_samples[2], other->all_samples[3], other->all_samples[4], other->all_samples[5]}, k(other->k), l(other->l) {}
} TGswSampleFFT_FPGA;

#endif 