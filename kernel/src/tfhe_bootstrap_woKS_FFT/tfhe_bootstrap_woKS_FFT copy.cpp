#include <fpga_constants.h>

/**
 * result = LWE(mu) iff phase(x)>0, LWE(-mu) iff phase(x)<0
 * @param result The resulting LweSample
 * @param bk The bootstrapping + keyswitch key
 * @param mu The output message (if phase(x)>0)
 * @param x The input sample
 */
EXPORT void tfhe_bootstrap_woKS_FFT(hls::stream<LweSample_FPGA> &result, hls::stream<LweBootstrappingKeyFFT_FPGA> &bk, hls::stream<Torus32>& mu, hls::stream<LweSample_FPGA>& x) {

    LweBootstrappingKeyFFT_FPGA bk_temp; 
    TGswParams_FPGA bk_params;
    TLweParams_FPGA accum_params;
    LweParams in_params;
    int N, Nx2, n; 
    LweSample_FPGA x_temp; 
    Torus32 mu_temp; 
    int32_t bara[param_N];

    mu.read(mu_temp);
    x.read(x_temp);
    bk.read(bk_temp);
    bk_params = bk_temp.bk_params; 
    // accum_params = bk_temp.accum_params; 
    // in_params = bk_temp.in_out_params; 
    // N = accum_params.N; 
    // Nx2 = 2 * N; 
    // n = in_params.n; 
    

    // Modulus switching
    int32_t barb = modSwitchFromTorus32(x_temp.b, Nx2);
    for (int32_t i = 0; i < n; i++) {
        bara[i] = modSwitchFromTorus32(x_temp.a[i], Nx2);
    }

    // the initial testvec = [mu,mu,mu,...,mu]
    for (int32_t i = 0; i < N; i++) testvect.coefsT[i] = mu_temp;

    // Bootstrapping rotation and extraction
    tfhe_blindRotateAndExtract_FFT(result, testvect, bk->bkFFT, barb, bara, n, bk_params);


}

EXPORT int32_t modSwitchFromTorus32(Torus32 phase, int32_t Msize){
    uint64_t interv = ((UINT64_C(1)<<63)/Msize)*2; // width of each intervall
    uint64_t half_interval = interv/2; // begin of the first intervall
    uint64_t phase64 = (uint64_t(phase)<<32) + half_interval;
    //floor to the nearest multiples of interv
    return phase64/interv;
}