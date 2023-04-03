#include<fpga_constants.h>
/**
 * result = LWE(mu) iff phase(x)>0, LWE(-mu) iff phase(x)<0
 * @param result The resulting LweSample
 * @param bk The bootstrapping + keyswitch key
 * @param mu The output message (if phase(x)>0)
 * @param x The input sample
 */
void tfhe_bootstrap_FFT(LweSample_FPGA *result, LweBootstrappingKeyFFT_FPGA *bk, Torus32 mu, LweSample_FPGA *x) {

    //LweSample_FPGA u = new_LweSample(&bk->accum_params->extracted_lweparams);

    LweSample_FPGA u;
    u.n = bk->accum_params->extracted_lweparams->n; 
    u.alpha_min = bk->accum_params->extracted_lweparams->alpha_min; 
    u.alpha_max = bk->accum_params->extracted_lweparams->alpha_max; 

    //Seperate File
    tfhe_bootstrap_woKS_FFT(u, bk, mu, x);
    // Key switching
    
    //In lwe-keyswitch-functions.cpp
    lweKeySwitch(result, bk->ks, u);

    delete_LweSample(u);
}

void lweKeySwitch(LweSample* result, const LweKeySwitchKey* ks, const LweSample* sample){
    const LweParams* params=ks->out_params;
    const int32_t n=ks->n;
    const int32_t basebit=ks->basebit;
    const int32_t t=ks->t;

    lweNoiselessTrivial(result,sample->b,params);
    lweKeySwitchTranslate_fromArray(result,
	    (const LweSample***) ks->ks, params,
	    sample->a, n, t, basebit);
}