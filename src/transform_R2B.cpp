#include "iostream"
#include "cassert"
#include "tfhe/tfhe.h"
#include "transform_R2B.h"
#include "global_key_generator.h"

using namespace std;

void bit4_bootsSymEncrypt(LweSample *result, int32_t message, const TFheGateBootstrappingSecretKeySet *key) {
    Torus32 _1s16_0 = modSwitchToTorus32(0, 16);
    Torus32 _1s16_1 = modSwitchToTorus32(1, 16);
    Torus32 mu = message ? _1s16_1 : _1s16_0;
    double alpha = key->params->in_out_params->alpha_min; //TODO: specify noise
    lweSymEncrypt(result, mu, alpha, key->lwe_key);
}

int32_t bit4_bootsSymDecrypt(const LweSample *sample, const TFheGateBootstrappingSecretKeySet *key) {
    Torus32 mu = lwePhase(sample, key->lwe_key);
    Torus32 boundary = modSwitchToTorus32(1, 32);
    return (mu > boundary ? 1 : 0); //we have to do that because of the C binding
}


void bit4_bootsADD(LweSample* result, int bitwidth, LweSample* A, LweSample* B, const TFheGateBootstrappingCloudKeySet *bk) {
    static const Torus32 MU_0 = modSwitchToTorus32(0, 16);
    static const Torus32 MU_1 = modSwitchToTorus32(1, 16);

    const LweParams *in_out_params = bk->params->in_out_params;

    // temp_result = bit4 [int cA + int cB]
    LweSample *temp_result = new_LweSample(in_out_params);

    static const Torus32 MU_bias = modSwitchToTorus32(1, 32);
    lweNoiselessTrivial(temp_result, MU_bias, in_out_params);

    for (int i = 0; i < 4; i++) {
        lweAddMulTo(temp_result, 1<<i, &A[i], in_out_params);
        lweAddMulTo(temp_result, 1<<i, &B[i], in_out_params);
    }

    R2B_tfhe_bootstrap_FFT(4, result, bk->bkFFT, MU_0, MU_1, temp_result);

    delete_LweSample(temp_result);
}

void R2B_tfhe_bootstrap_FFT(int32_t bitwidth, LweSample *result, const LweBootstrappingKeyFFT *bk, Torus32 mu_0, Torus32 mu_1, LweSample *x)
{
    LweSample *u = new_LweSample_array(bitwidth, &bk->accum_params->extracted_lweparams);

    R2B_tfhe_bootstrap_woKS_FFT(bitwidth, u, bk, mu_0, mu_1, x);

    // Key switching
    for(int i = 0; i< bitwidth; i++) {
        lweKeySwitch(&result[i], bk->ks, &u[i]);
    }
    
    delete_LweSample_array(bitwidth, u);
}

void R2B_tfhe_bootstrap_woKS_FFT(int32_t bitwidth, LweSample *result, const LweBootstrappingKeyFFT *bk, Torus32 mu_0, Torus32 mu_1, LweSample *x) {
    // bootstrapping
    const TGswParams *bk_params = bk->bk_params;

    // testvector
    const TLweParams *accum_params = bk->accum_params;
    
    // input cipertext
    const LweParams *in_params = bk->in_out_params;
    const int32_t N = accum_params->N;

    // cyclotomic field/ polynomial degrees
    const int32_t Nx2 = N;

    // origin params
    const int32_t n = in_params->n;

    //TorusPolynomial *testvect = new_TorusPolynomial(N);
    TorusPolynomial *testvect = new_TorusPolynomial_array(bitwidth, N);

    int32_t *bara = new int32_t[N];

    // Modulus switching
    int32_t barb = modSwitchFromTorus32(x->b, Nx2);
    for (int32_t i = 0; i < n; i++) {
        bara[i] = modSwitchFromTorus32(x->a[i], Nx2);
    }

    // [the most significant bit] [xxx*]
    for (int32_t i = 0; i < N/2; i++) {
        testvect[3].coefsT[i] = mu_0;
    }
    for (int32_t i = N/2; i < N; i++) {
        testvect[3].coefsT[i] = mu_1;
    }

    // [the second significant bit] [xx*x]
    for (int32_t i = 0; i < N/4; i++) {
        testvect[2].coefsT[i] = mu_0;
    }
    for (int32_t i = N/4; i < N/2; i++) {
        testvect[2].coefsT[i] = mu_1;
    }
    for (int32_t i = N/2; i < 3*N/4; i++) {
        testvect[2].coefsT[i] = mu_0;
    }
    for (int32_t i = 3*N/4; i < N; i++) {
        testvect[2].coefsT[i] = mu_1;
    }

    // [the third significant bit] [x*xx]
    for (int32_t i = 0; i < N/8; i++) {
        testvect[1].coefsT[i] = mu_0;
    }
    for (int32_t i = N/8; i < N/4; i++) {
        testvect[1].coefsT[i] = mu_1;
    }
    for (int32_t i = N/4; i < 3*N/8; i++) {
        testvect[1].coefsT[i] = mu_0;
    }
    for (int32_t i = 3*N/8; i < N/2; i++) {
        testvect[1].coefsT[i] = mu_1;
    }
    for (int32_t i = N/2; i < N*5/8; i++) {
        testvect[1].coefsT[i] = mu_0;
    }
    for (int32_t i = N*5/8; i < N*3/4; i++) {
        testvect[1].coefsT[i] = mu_1;
    }
    for (int32_t i = N*3/4; i < N*7/8; i++) {
        testvect[1].coefsT[i] = mu_0;
    }
    for (int32_t i = N*7/8; i < N; i++) {
        testvect[1].coefsT[i] = mu_1;
    }

    // [the least significant bit] [*xxx]
    for (int32_t i = 0; i < N/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N/16; i < N/8; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N/8; i < 3*N/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = 3*N/16; i < N/4; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N/4; i < N*5/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*5/16; i < N*3/8; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N*3/8; i < N*7/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*7/16; i < N/2; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N/2; i < N*9/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*9/16; i < N*5/8; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N*5/8; i < N*11/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*11/16; i < N*3/4; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = 3*N/4; i < N*13/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*13/16; i < N*7/8; i++) {
        testvect[0].coefsT[i] = mu_1;
    }
    for (int32_t i = N*7/8; i < N*15/16; i++) {
        testvect[0].coefsT[i] = mu_0;
    }
    for (int32_t i = N*15/16; i < N; i++) {
        testvect[0].coefsT[i] = mu_1;
    }

    // Bootstrapping rotation and extraction

    R2B_tfhe_blindRotateAndExtract_FFT(bitwidth, result, testvect, bk->bkFFT, barb, bara, n, bk_params);

    delete[] bara;
    delete_TorusPolynomial_array(bitwidth, testvect);
}

void R2B_tfhe_blindRotateAndExtract_FFT(int32_t bitwidth, LweSample *result, const TorusPolynomial *v, const TGswSampleFFT *bk, const int32_t barb, const int32_t *bara, const int32_t n, const TGswParams *bk_params) {
    const TLweParams *accum_params = bk_params->tlwe_params;
    const LweParams *extract_params = &accum_params->extracted_lweparams;
    const int32_t N = accum_params->N;
    const int32_t _2N = 2 * N;

    // Test polynomial
    TorusPolynomial *testvectbis = new_TorusPolynomial_array(bitwidth, N);
    // Accumulator
    TLweSample *acc = new_TLweSample_array(bitwidth, accum_params);

    for (int bit = 0; bit < bitwidth; bit++) {
        // testvector = X^{2N-barb}*v
        if (barb != 0) torusPolynomialMulByXai(&testvectbis[bit], _2N - barb, &v[bit]);
        else torusPolynomialCopy(&testvectbis[bit], &v[bit]);
    
        tLweNoiselessTrivial(&acc[bit], &testvectbis[bit], accum_params);
        // Blind rotation

        tfhe_blindRotate_FFT(&acc[bit], bk, bara, n, bk_params);

        // Extraction
        tLweExtractLweSample(&result[bit], &acc[bit], extract_params, accum_params);

    }

    // garbage collection
    delete_TLweSample_array(bitwidth, acc);
    delete_TorusPolynomial_array(bitwidth, testvectbis);
}