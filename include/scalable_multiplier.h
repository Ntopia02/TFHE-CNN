#ifndef SCALABLE_MULTIPLIER
#define SCALABLE_MULTIPLIER

#include "tfhe/tfhe.h"
#include "shift_adder.h"


class scalable_add: public carried_adder {
    protected:
    LweSample *C;
    LweSample *S;

    int scale_factor;

    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    scalable_add(int scale_factor);
    ~scalable_add();

    void _add_evaluation(int32_t *A, int32_t *B, int mode0, int mode1);
    void _add_evaluation(LweSample *A, LweSample *B, int mode0, int mode1);
    LweSample *_add_to_add();
};


class scalable_shift {
    private:
    LweSample *temp_input;
    LweSample ***result;
    int *mode;

    int scale_factor;
    int channel;

    protected:
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    scalable_shift(int scale_factor);
    ~scalable_shift();

    void _shift_evaluate(int32_t *A, int32_t *W);
    void _Booth_evaluate(int32_t *A, int32_t *W);

    LweSample ***_shift_to_add();
    int* _shift_to_add_mode();
    void _show_mode();
};

class scalable_multiply {
    private:
    LweSample* result;
    int scale_factor;

    scalable_add **add_unit;
    scalable_shift shift_unit;

    protected:
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;
    
    public:
    scalable_multiply(int scale_factor);
    ~scalable_multiply();

    void _multiply_evaluate(int32_t *A, int32_t *W);

    void _show_result();
};

#endif