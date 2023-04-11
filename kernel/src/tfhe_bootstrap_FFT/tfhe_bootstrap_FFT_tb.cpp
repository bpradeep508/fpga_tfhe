#include "host.h"
#include "host_constants.h"
#include "host_to_fpga.h"

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }
    
    clock_t htod, dtoh, comp; 

    /*====================================================CL===============================================================*/

    std::string binaryFile = argv[1];
    cl_int err;
    cl::Context context;
    cl::Kernel krnl1, krnl2;
    cl::CommandQueue q;
    
    auto devices = get_xil_devices();
    auto fileBuf = read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, 0, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            std::cout << "Setting CUs up..." << std::endl; 
            OCL_CHECK(err, krnl1 = cl::Kernel(program, "tfhe_bootstrap_FFT", &err));
            OCL_CHECK(err, krnl2 = cl::Kernel(program, "tfhe_bootstrap_FFT_return", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    /*====================================================TFHE CLIENT===============================================================*/

    //generate a keyset
    printf("Generating a keyset.\n");
    const int minimum_lambda = 110;
    TFheGateBootstrappingParameterSet* params = new_default_gate_bootstrapping_parameters(minimum_lambda);

    //generate a random key
    printf("Generating a random key\n");
    uint32_t seed[] = { 314, 1592, 657 };
    tfhe_random_generator_setSeed(seed,3);
    TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);

    //export the secret key to file for later use
    printf("Exporting client secret key.\n");
    FILE* secret_key = fopen("./keys/secret.key","wb");
    export_tfheGateBootstrappingSecretKeySet_toFile(secret_key, key);
    fclose(secret_key);

    //export the cloud key to a file (for the cloud)
    printf("Exporting cloud key.\n");
    FILE* cloud_key = fopen("./keys/cloud.key","wb");
    export_tfheGateBootstrappingCloudKeySet_toFile(cloud_key, &key->cloud);
    fclose(cloud_key);
                                                          
    //you can put additional instructions here!!
    //...

    //generate encrypt the 16 bits of 499
    int16_t plaintext1 = 499;
    LweSample* ciphertext1 = new_gate_bootstrapping_ciphertext_array(16, params);
    for (int i=0; i<16; i++) {
        bootsSymEncrypt(&ciphertext1[i], (plaintext1>>i)&1, key);
    }

    //generate encrypt the 16 bits of 388
    int16_t plaintext2 = 388;
    LweSample* ciphertext2 = new_gate_bootstrapping_ciphertext_array(16, params);
    for (int i=0; i<16; i++) {
        bootsSymEncrypt(&ciphertext2[i], (plaintext2>>i)&1, key);
    }

    printf("Hi there! Today, I will ask the cloud what is the OR between %d and %d\n",plaintext1, plaintext2);
    
    //export the 2x16 ciphertexts to a file (for the cloud)
    FILE* cloud_data = fopen("./data/cloud.data","wb");
    for (int i=0; i<16; i++) 
        export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertext1[i], params);
    for (int i=0; i<16; i++) 
        export_gate_bootstrapping_ciphertext_toFile(cloud_data, &ciphertext2[i], params);
    fclose(cloud_data);

    //clean up all pointers
    delete_gate_bootstrapping_ciphertext_array(16, ciphertext2);
    delete_gate_bootstrapping_ciphertext_array(16, ciphertext1);


    //clean up all pointers
    delete_gate_bootstrapping_secret_keyset(key);
    delete_gate_bootstrapping_parameters(params);

    /*====================================================HOST BUFFERS===============================================================*/
    
    LweBootstrappingKeyFFT_FPGA *in_bk = (LweBootstrappingKeyFFT_FPGA*) malloc(sizeof(LweBootstrappingKeyFFT_FPGA)); 
    Torus32_FPGA *in_mu = (Torus32_FPGA*) malloc(sizeof(Torus32_FPGA)); 
    LweSample_FPGA *in_x = (LweSample_FPGA*) malloc(sizeof(LweSample_FPGA));

    LweBootstrappingKeyFFT_FPGA *out_bk = (LweBootstrappingKeyFFT_FPGA*) malloc(sizeof(LweBootstrappingKeyFFT_FPGA)); 
    Torus32_FPGA *out_mu = (Torus32_FPGA*) malloc(sizeof(Torus32_FPGA)); 
    LweSample_FPGA *out_x = (LweSample_FPGA*) malloc(sizeof(LweSample_FPGA));
    /*====================================================TFHE CLOUD===============================================================*/

    //reads the cloud key from file
    cloud_key = fopen("./keys/cloud.key","rb");
    TFheGateBootstrappingCloudKeySet* bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
    fclose(cloud_key);
 
    //if necessary, the params are inside the key
    const TFheGateBootstrappingParameterSet* cparams = bk->params;

    //read the 2x16 ciphertexts
    ciphertext1 = new_gate_bootstrapping_ciphertext_array(16, cparams);
    ciphertext2 = new_gate_bootstrapping_ciphertext_array(16, cparams);

    //reads the 2x16 ciphertexts from the cloud file
    cloud_data = fopen("./data/cloud.data","rb");
    for (int i=0; i<16; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ciphertext1[i], cparams);
    for (int i=0; i<16; i++) import_gate_bootstrapping_ciphertext_fromFile(cloud_data, &ciphertext2[i], cparams);
    fclose(cloud_data);

    //do some operations on the ciphertexts: here, we will compute the
    //minimum of the two
    //LweSample* result = new_gate_bootstrapping_ciphertext_array(16, params);
    //OR(result, ciphertext1, ciphertext2, 16, bk);

    // //export the 32 ciphertexts to a file (for the cloud)
    // FILE* answer_data = fopen("./data/answer.data","wb");
    // for (int i=0; i<16; i++) export_gate_bootstrapping_ciphertext_toFile(answer_data, &result[i], params);
    // fclose(answer_data);


    static const Torus32 MU = modSwitchToTorus32(1, 8);
    const LweParams *in_out_params = bk->params->in_out_params;

    LweSample *temp_result = new_LweSample(in_out_params);

    //compute: (0,1/8) + ca + cb
    static const Torus32 OrConst = modSwitchToTorus32(1, 8);
    lweNoiselessTrivial(temp_result, OrConst, in_out_params);
    lweAddTo(temp_result, ciphertext1, in_out_params);
    lweAddTo(temp_result, ciphertext2, in_out_params);

    //if the phase is positive, the result is 1/8
    //if the phase is positive, else the result is -1/8
    //tfhe_bootstrap_FFT(result, bk->bkFFT, MU, temp_result);

    std::cout << "Loading host buffers with cloud key, mu, and x" << std::endl; 
    
    *in_mu = MU; 
    LweSample_Host_to_FPGA(temp_result, in_x);
    LweBootstrappingKeyFFT_Host_to_FPGA(bk->bkFFT, in_bk);

    delete_LweSample(temp_result);

    //clean up all pointers
    //delete_gate_bootstrapping_ciphertext_array(16, result);
    delete_gate_bootstrapping_ciphertext_array(16, ciphertext2);
    delete_gate_bootstrapping_ciphertext_array(16, ciphertext1);
    
    
    delete_gate_bootstrapping_cloud_keyset(bk);

    /*====================================================Setting up kernel I/O===============================================================*/

    /* INPUT BUFFERS */
    OCL_CHECK(err, cl::Buffer buffer_in_bk(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(LweBootstrappingKeyFFT_FPGA), in_bk, &err)); 
    OCL_CHECK(err, cl::Buffer buffer_in_mu(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(Torus32_FPGA), in_mu, &err)); 
    OCL_CHECK(err, cl::Buffer buffer_in_x(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(LweSample_FPGA), in_x, &err)); 

    /* OUTPUT BUFFERS */
    OCL_CHECK(err, cl::Buffer buffer_out_bk(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(LweBootstrappingKeyFFT_FPGA), out_bk, &err)); 
    OCL_CHECK(err, cl::Buffer buffer_out_mu(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(Torus32_FPGA), out_mu, &err)); 
    OCL_CHECK(err, cl::Buffer buffer_out_x(context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, sizeof(LweSample_FPGA), out_x, &err)); 

    /* SETTING INPUT PARAMETERS */
    OCL_CHECK(err, err = krnl1.setArg(0, buffer_in_bk));
    OCL_CHECK(err, err = krnl1.setArg(1, buffer_in_mu));
    OCL_CHECK(err, err = krnl1.setArg(2, buffer_in_x));

    /* SETTING OUTPUT PARAMETERS */
    OCL_CHECK(err, err = krnl2.setArg(7, buffer_out_bk));
    OCL_CHECK(err, err = krnl2.setArg(8, buffer_out_mu));
    OCL_CHECK(err, err = krnl2.setArg(9, buffer_out_x));

    /*====================================================KERNEL===============================================================*/
    /* HOST -> DEVICE DATA TRANSFER*/
    std::cout << "HOST -> DEVICE" << std::endl; 
    htod = clock(); 
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in_bk}, 0 /* 0 means from host*/));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in_mu}, 0 /* 0 means from host*/));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_in_x}, 0 /* 0 means from host*/));
    q.finish();
    htod = clock() - htod; 
    
    /*STARTING KERNEL(S)*/
    std::cout << "STARTING KERNEL(S)" << std::endl; 
    comp = clock(); 
	OCL_CHECK(err, err = q.enqueueTask(krnl1));
    OCL_CHECK(err, err = q.enqueueTask(krnl2));
    q.finish(); 
    comp = clock() - comp;
    std::cout << "KERNEL(S) FINISHED" << std::endl; 

    /*DEVICE -> HOST DATA TRANSFER*/
    std::cout << "HOST <- DEVICE" << std::endl; 
    dtoh = clock();
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_out_bk}, CL_MIGRATE_MEM_OBJECT_HOST));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_out_mu}, CL_MIGRATE_MEM_OBJECT_HOST));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_out_x}, CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();
    dtoh = clock() - dtoh;

    /*====================================================VERIFICATION & TIMING===============================================================*/

    printf("Host -> Device : %lf ms\n", 1000.0 * htod/CLOCKS_PER_SEC);
    printf("Device -> Host : %lf ms\n", 1000.0 * dtoh/CLOCKS_PER_SEC);
    printf("Computation : %lf ms\n",  1000.0 * comp/CLOCKS_PER_SEC);
    
    bool match = true;

    //std::cout << "out_mu: " << *out_mu << " in_mu " << *in_mu << std::endl; 
    if (*out_mu != *in_mu) {
        match = false; 
    }
    //std::cout << "out_x_b: " << out_x->b << " in_x_b " << in_x->b << std::endl; 
    if (out_x->b != in_x->b) {
        match = false; 
    }
    for (int i = 0; i < N; i++) {
        //std::cout << "out_x_a: " << out_x->a[i] << " in_x_a " << in_x->a[i] << std::endl; 
        if (out_x->a[i] != in_x->a[i]) {
            match = false; 
        }
    }

    if (out_bk->in_out_params.n != in_bk->in_out_params.n) {
        match = false; 
    }
    if (out_bk->in_out_params.alpha_min != in_bk->in_out_params.alpha_min) {
        match = false; 
    }
    if (out_bk->in_out_params.alpha_max != in_bk->in_out_params.alpha_max) {
        match = false; 
    }
    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;

    free(in_bk);
    free(in_mu);
    free(in_x);

    free(out_bk);
    free(out_mu);
    free(out_x);

    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}