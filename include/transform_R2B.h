#ifndef TRANSFORM_R2B
#define TRANSFORM_R2B

#include "tfhe/tfhe.h"

// bit4 encryption/decryption
void bit4_bootsSymEncrypt(LweSample *result, int32_t message, const TFheGateBootstrappingSecretKeySet *key);

int32_t bit4_bootsSymDecrypt(const LweSample *sample, const TFheGateBootstrappingSecretKeySet *key);

// level-0/ gate-application
void bit4_bootsADD(LweSample* result, int bitwidth, LweSample* A, LweSample* B, const TFheGateBootstrappingCloudKeySet *bk);

// level-1
void R2B_tfhe_bootstrap_FFT(int32_t bitwidth, LweSample *result, const LweBootstrappingKeyFFT *bk, Torus32 mu_0, Torus32 mu_1, LweSample *x);

// level-2
void R2B_tfhe_bootstrap_woKS_FFT(int32_t bitwidth, LweSample *result, const LweBootstrappingKeyFFT *bk, Torus32 mu_0, Torus32 mu_1, LweSample *x);

// level-3
void R2B_tfhe_blindRotateAndExtract_FFT(int32_t bitwidth, LweSample *result, const TorusPolynomial *v, const TGswSampleFFT *bk, const int32_t barb, const int32_t *bara, const int32_t n, const TGswParams *bk_params);

#endif