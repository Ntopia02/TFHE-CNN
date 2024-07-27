#ifndef ACTIVATION_FUNCTION
#define ACTIVATION_FUNCTION

#include <tfhe/tfhe.h>
#include "multiplier.h"
#include "shift_adder.h"

///@file
///@brief this file defines the activation ReLU layer of the convolution.

class bit4_activation {
    private:
    LweSample* not_sign;    

    public:
    /** gate-bootstrapping params and key */
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    bit4_activation();
    ~bit4_activation();
    void _relu(LweSample* result,LweSample* input_element);
};

#endif