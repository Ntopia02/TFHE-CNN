#ifndef GLOBAL_KEY_GENERATOR
#define GLOBAL_KEY_GENERATOR

#include <tfhe/tfhe.h>

/** restore the params for TLwe/TRLwe/TGsw*/
extern TFheGateBootstrappingParameterSet *global_params;

/** restore the key for TLwe/TRLwe*/
extern TFheGateBootstrappingSecretKeySet* global_key;

#endif