#include<fpga_constants.h>

/*

    Starting function of TFHE (possibly not needed). The key, mu, and x value are passed from the HOST to MEM and read by this
    and written into stream to start the computations. 

    Config File Specs:

        in_bk      <- HBM/DDR
        in_mu      <- HBM/DDR
        in_x       <- HBM/DDR
        out_bk     -> tfhe_bootstrap_woKS_FFT 
        out_mu     -> tfhe_bootstrap_woKS_FFT
        out_x      -> tfhe_bootstrap_woKS_FFT

*/

    void tfhe_bootstrap_FFT(
    /*INPUT*/
    LweBootstrappingKeyFFT_FPGA *in_bk, 
    Torus32_FPGA *in_mu, 
    LweSample_FPGA *in_x,
    
    /*OUTPUT*/
    //Manually disaggregating struct LweBootstrappingKeyFFT_FPGA and sub structs:
    hls::stream<int32_t>& out_bk_in_out_params_n, 
    hls::stream<double>& out_bk_in_out_params_alpha_min,
    hls::stream<double>& out_bk_in_out_params_alpha_max,
    
    //Mu doesn't need disaggregating
    hls::stream<Torus32_FPGA>& out_mu, 
    //Manually disaggregating struct LweSample_FPGA:
    hls::stream<Torus32_FPGA>& out_x_a, 
    hls::stream<Torus32_FPGA>& out_x_b,
    hls::stream<Torus32_FPGA>& out_x_current_variance) {

    /*INPUT INTERFACE*/
    #pragma HLS INTERFACE m_axi port=in_bk bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in_mu bundle=gmem0
    #pragma HLS INTERFACE m_axi port=in_x bundle=gmem0 

    /*OUTPUT INTERFACE*/
    //#pragma HLS STREAM variable=out_bk_in_out_params_n
    //#pragma HLS STREAM variable=out_bk_in_out_params_alpha_min
    //#pragma HLS STREAM variable=out_bk_in_out_params_alpha_max
    #pragma HLS INTERFACE axis port=&out_bk_in_out_params_n
    #pragma HLS INTERFACE axis port=&out_bk_in_out_params_alpha_min
    #pragma HLS INTERFACE axis port=&out_bk_in_out_params_alpha_max
    #pragma HLS STREAM variable=out_mu
    #pragma HLS STREAM variable=out_x_a depth=N
    #pragma HLS STREAM variable=out_x_b
    #pragma HLS STREAM variable=out_x_current_variance

    out_bk_in_out_params_n.write(in_bk->in_out_params.n); 
    out_bk_in_out_params_alpha_min.write(in_bk->in_out_params.alpha_min);
    out_bk_in_out_params_alpha_max.write(in_bk->in_out_params.alpha_max);

    out_mu.write(*in_mu); 
    for (int i = 0; i < N; i++) {
        out_x_a.write(in_x->a[i]);
    }
    out_x_b.write(in_x->b); 
    out_x_current_variance.write(in_x->current_variance); 
    
}
