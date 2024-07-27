#ifndef LINEAR
#define LINEAR

#include <string>
#include "tfhe/tfhe.h"
#include "multiplier.h"
#include "shift_adder.h"

///@file
///@brief this file defines the activation ReLU layer of the convolution.


class bit8_conv_linear {
    private:
    bit8_carried_adder add_unit;
    bit4_multiplier multi_unit;

    protected:
    LweSample** result;
    LweSample* ACC;
    LweSample* X;

    int**** weight;
    int* bias;

    int input_size[3];
    int output_size;

    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:

    bit8_conv_linear(int out, int c, int x, int y);
    ~bit8_conv_linear();

    void _conv_linear_evaluation(LweSample**** input_layer, std::string restore_filepath);
    void _show_result();

    void _convert_DEC_to_BIN(int *result, int weight, LweSample* e_result, LweSample* e_x);
    void _convert_DEC_to_BIN(LweSample* result, int bias, int* mode);

    void _set_weight(int ****input_weight);
    void _set_values(std::string filename);

    LweSample** _linear_to_linear();
};

class bit8_linear {
    private:
    bit8_carried_adder add_unit;
    bit4_multiplier multi_unit;

    protected:
    LweSample** result;
    LweSample* ACC;
    LweSample* X;

    int** weight;
    int* bias;

    int input_size;
    int output_size;

    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    bit8_linear(int out, int in);
    ~bit8_linear();

    void _linear_evaluation(LweSample** input_layer, std::string restore_filepath);
    void _show_result();

    void _convert_DEC_to_BIN(int *result, int weight, LweSample* e_result, LweSample* e_x);
    void _convert_DEC_to_BIN(LweSample* result, int bias, int* mode);
    
    void _set_weight(int **input_weight);
    void _set_values(std::string filename);

    LweSample** _linear_to_linear();
};

#endif
