#include "fpga_constants.h"
#include <assert.h>

extern "C" {
  void tLweCopy(TLweSample_FPGA *input, TLweSample_FPGA *output) {

    #pragma HLS INTERFACE m_axi port=input
    #pragma HLS INTERFACE m_axi port=output 

    tLweCopy_loop_1: for (int32_t i = 0; i <= param_k; ++i)
      tLweCopy_loop_2: for (int32_t j = 0; j < param_N; ++j) {

        output->a[i].coefsT[j] = input->a[i].coefsT[j];
      }
    output->current_variance = input->current_variance;
  }
}

extern "C" {
  void tfhe_blindRotate_FFT(
      TLweSample_FPGA* input, 
      const TGswSampleFFT_FPGA bkFFT[param_n], 
      const int32_t bara[param_n],
      TLweSample_FPGA* output) {
    
    #pragma HLS INTERFACE m_axi port=input
    #pragma HLS INTERFACE m_axi port=bkFFT 
    #pragma HLS INTERFACE m_axi port=bara
    #pragma HLS INTERFACE m_axi port=input

    TLweSample_FPGA accum;

    // accum.current_variance = input->current_variance;
    // tfhe_blindRotate_FFT_load_1: for(int i=0; i<=param_k; i++) {
    //   tfhe_blindRotate_FFT_load_2: for(int j=0; j<param_N; j++) {
    //     accum.a[i].coefsT[j] = input->a[i].coefsT[j];
    //   }
    // }

    tLweCopy(input, &accum);

    TLweSample_FPGA temp;
    TLweSample_FPGA *temp2 = &temp;
    TLweSample_FPGA *temp3 = &accum;

    for (int32_t i = 0; i < param_n; i++) {
      const int32_t barai = bara[i];
      // if (barai == 0) continue; //indeed, this is an easy case!

      tfhe_MuxRotate_FFT(temp2, temp3, &bkFFT[i], barai);

      // swap(temp2, temp3);
      TLweSample_FPGA *t = temp2;
      temp2 = temp3;
      temp3 = t;
    }

    //assert(param_n % 2 == 0);
    
    if (temp3 != &accum) {
      tLweCopy(temp3, &accum);
    }

    tLweCopy(&accum, output);

    // output->current_variance = accum.current_variance;
    // tfhe_blindRotate_FFT_store_1: for(int i=0; i<=param_k; i++) {
    //   tfhe_blindRotate_FFT_store_2: for(int j=0; j<param_N; j++) {
    //     output->a[i].coefsT[j] = output.a[i].coefsT[j];
    //   }
    // }
  }
}