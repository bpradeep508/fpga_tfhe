#include <fpga_constants.h>

/*

    Splits the x input into the a and b, and switches them from Torus32. Copies the mu value across a vector.

    Config File Specs:

        in_bk       <- tfhe_bootstrap_woKS_FFT 
        in_mu       <- tfhe_bootstrap_woKS_FFT 
        in_x        <- tfhe_bootstrap_woKS_FFT 
        out_bk      -> tfhe_blindRotateAndExtract_FFT 
        out_bara    -> tfhe_blindRotateAndExtract_FFT
        out_barb    -> tfhe_blindRotateAndExtract_FFT
        out_testvect-> tfhe_blindRotateAndExtract_FFT 

*/

void tfhe_bootstrap_woKS_FFT(
    hls::stream<LweBootstrappingKeyFFT_FPGA>& in_bk, 
    hls::stream<Torus32>& in_mu, 
    hls::stream<LweSample_FPGA>& in_x,
    hls::stream<LweBootstrappingKeyFFT_FPGA>& out_bk,
    hls::stream<TorusPolynomial_FPGA>& out_testvect,
    hls::stream<ARRAY>& out_bara,
    hls::stream<int32_t>& out_barb) {

    #pragma HLS STREAM variable=in_bk
    #pragma HLS STREAM variable=in_mu
    #pragma HLS STREAM variable=in_x
    #pragma HLS STREAM variable=out_testvect 
    #pragma HLS STREAM variable=out_bara
    #pragma HLS STREAM variable=out_barb


    LweSample_FPGA temp_x;
    ARRAY temp_bara; 
    int32_t temp_barb;
    TorusPolynomial_FPGA temp_testvect; 

    barb = modSwitchFromTorus32(temp_x.b, N/2)
    for (int i = 0; i < n; i++) {
        #pragma HLS UNROLL 
        temp_bara.coefs[i] = modSwitchFromTorus32(temp_x.a[i], N/2);
    }

    for (int i = 0; i < N; i++) {
        #pragma HLS UNROLL 
        temp_testvect.coefs[i] = mu_temp;
    }

    out_bk << in_bk;
    out_testvect << temp_testvect;
    out_bara << temp_bara; 
    out_barb << temp_barb;
}