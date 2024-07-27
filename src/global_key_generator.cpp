#include "tfhe/tfhe.h"
#include "global_key_generator.h"
#include <iostream>

using namespace std;

struct Exception42 {
};

TFheGateBootstrappingParameterSet *global_params = nullptr;
TFheGateBootstrappingSecretKeySet* global_key = nullptr;