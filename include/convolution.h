#ifndef CONVOLUTION
#define CONVOLUTION

///@file
///@brief this file defines the class of homomorphic convolution operations with 4-bit multiplier.

#include <tfhe/tfhe.h>
#include <fstream>
#include <string>
#include "multiplier.h"
#include "shift_adder.h"
#include "activation_function.h"

#include <string.h>

class bit4_convolution {
    private:
    /** the unit handles the activation process of the convolution. */
    bit4_activation activate_unit;

    /** the unit handles the homomorphic multiply operation in the convolution. */
    bit4_multiplier multi_unit;

    /** the unit handles the homomorphic add operation in the convolution. */
    bit8_carried_adder add_unit;

    /** restore the result of the convolution operation. */
    LweSample ****result;

    /** restore the value of the accumulator. */
    LweSample *ACC;

    /** restore the temp value of input_layer */
    LweSample *X;

    protected:
    /** input layer */
    int input_size;
    int padding;

    /** convolution kernel */
    int out_channel;
    int in_channel;
    int kernel_size;
    int ****kernel;
    int *bias;
    int stride;

    /** output layer */
    int output_size;

    public:
    /** gate-bootstrapping params and key */
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;


    bit4_convolution(int input_size, int padding, int out_channel, int in_channel, int kernel_size, int stride);
    ~bit4_convolution();

    /** set ALL kernel elements with 1 value. */
    void _set_default_values();
    void _set_values(std::string filename);

    /** homomorphically evaluating the convolution operation. 
     *  @param input_layer the input LweSample Matrix.
     *  @param restore_filename the target destination of the restore file.
    */
    void _conv_evaluate(LweSample ****input_layer, std::string restore_filepath);

    /** convert a decimal weight to 4-bit and copy the lowest 4-bit of e_x to e_result for multiplying.
     *  @param result the 4-bit result.
     *  @param weight the decimal weight.
     *  @param e_result the lowest 4-bit of e_x.
     *  @param e_x the original LweSample
    */
    void _convert_DEC_to_BIN(int *result, int weight, LweSample* e_result, LweSample* e_x);
    void _convert_DEC_to_BIN(LweSample *result, int bias_value, int* mode);

    /** activate the element of the output with the ReLU function. */
    void _activate_evaluate();

    /** show the result of the output_layer. */
    void _show_result();
    void _show_values();

    /** export the result to pool layer. */
    LweSample**** _activate_to_pool();
};

#endif