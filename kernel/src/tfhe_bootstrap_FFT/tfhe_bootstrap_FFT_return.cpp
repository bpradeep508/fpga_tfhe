#include<fpga_constants.h>

/*

    Used just to test fuctionality of tfhe_bootstrap_FFT.cpp

    Config File Specs:

        out_bk      -> HBM/DDR
        out_mu      -> HBM/DDR
        out_x       -> HBM/DDR
        out_bk     <- tfhe_bootstrap_FFT 
        out_mu     <- tfhe_bootstrap_FFT
        out_x      <- tfhe_bootstrap_FFT

*/

void tfhe_bootstrap_FFT_return(
    /*INPUT*/
    //Manually disaggregating struct LweBootstrappingKeyFFT_FPGA and sub structs:
    hls::stream<int32_t>& in_bk_in_out_params_n, 
    hls::stream<double>& in_bk_in_out_params_alpha_min,
    hls::stream<double>& in_bk_in_out_params_alpha_max,
    
    //Mu doesn't need disaggregating
    hls::stream<Torus32_FPGA>& in_mu, 
    //Manually disaggregating struct LweSample_FPGA:
    hls::stream<Torus32_FPGA>& in_x_a, 
    hls::stream<Torus32_FPGA>& in_x_b,
    hls::stream<Torus32_FPGA>& in_x_current_variance,
    
    /*OUTPUT*/
    LweBootstrappingKeyFFT_FPGA *out_bk, 
    Torus32_FPGA *out_mu, 
    LweSample_FPGA *out_x) {

    // #pragma HLS STREAM variable=in_bk_in_out_params_n
    // #pragma HLS STREAM variable=in_bk_in_out_params_alpha_max
    // #pragma HLS STREAM variable=in_bk_in_out_params_alpha_min
    #pragma HLS INTERFACE axis port=&in_bk_in_out_params_n
    #pragma HLS INTERFACE axis port=&in_bk_in_out_params_alpha_min
    #pragma HLS INTERFACE axis port=&in_bk_in_out_params_alpha_max
    #pragma HLS STREAM variable=in_mu
    #pragma HLS STREAM variable=in_x_a depth=N
    #pragma HLS STREAM variable=in_x_b
    #pragma HLS STREAM variable=in_x_current_variance


    #pragma HLS INTERFACE m_axi port=out_bk bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out_mu bundle=gmem0
    #pragma HLS INTERFACE m_axi port=out_x bundle=gmem0 

    in_bk_in_out_params_n >> out_bk->in_out_params.n; 
    in_bk_in_out_params_alpha_min >> out_bk->in_out_params.alpha_min;
    in_bk_in_out_params_alpha_max >> out_bk->in_out_params.alpha_max;

    in_mu >> *out_mu; 

    for (int i = 0; i < N; i++) {
        in_x_a >> out_x->a[i];
    }
    in_x_b >> out_x->b; 
}

