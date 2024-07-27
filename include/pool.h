#ifndef POOL
#define POOL

#include <tfhe/tfhe.h>
#include <shift_adder.h>

class bit4_pool {
    private:
    bit8_carried_adder subtract_unit;

    LweSample* max_sample;

    LweSample**** result;

    // input_layer
    int channel;
    int input_layer_size;

    // kernel
    int kernel_size;
    int stride;
    int padding;

    // output_layer
    int output_layer_size;

    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    /** initialize the structure of the pool unit.
     *  @param channel the channel nums
     *  @param input_layer_size the dimension of the input
     *  @param kernel_size the dimension of the pool kernel
     *  @param stride the stride of the pool kernel
     *  @param padding all zero padding for the input_layer
    */
    bit4_pool(int channel, int input_layer_size, int kernel_size, int stride, int padding);
    ~bit4_pool();

    void _max_pool(LweSample**** input);
    void _show_result();

    LweSample**** _pool_to_conv();
};

#endif