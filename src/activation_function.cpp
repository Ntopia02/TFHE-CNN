#include "activation_function.h"
#include "global_key_generator.h"
#include "iostream"

using namespace std;

bit4_activation::bit4_activation() {
    this->params = global_params;
    this->key = global_key;

    this->not_sign = new_gate_bootstrapping_ciphertext(this->params);
}

bit4_activation::~bit4_activation() {
    delete_gate_bootstrapping_ciphertext(this->not_sign);
}

void bit4_activation::_relu(LweSample* result, LweSample *input_element) {
    bootsNOT(this->not_sign,&input_element[8], &this->key->cloud);

    for(int i = 0; i<=8; i++) {
        bootsMUX(&result[i], &input_element[8], this->not_sign, &input_element[i], &this->key->cloud);
    }
}
