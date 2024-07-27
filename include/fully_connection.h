#ifndef FULLY_CONNECTION
#define FULLY_CONNECTION

#include <tfhe/tfhe.h>
#include "multiplier.h"
#include "shift_adder.h"

///@file
///@brief this file defines the 4-bit fully connection layer with the multiplier and shift_adder.

class bit4_fc {
    private:
    bit4_multiplier multi_unit;
    bit8_carried_adder add_unit;

    /** restore the final result of the FC layer*/
    LweSample* result;
 
    protected:
    TFheGateBootstrappingParameterSet* params;
    TFheGateBootstrappingSecretKeySet* key;

    int **weight;

    int size;

    public:
    bit4_fc();
    ~bit4_fc();


    void _fc_evaluate();

    /** convert a decimal weight to 4-bit and copy the lowest 4-bit of e_x to e_result for multiplying.
     *  @param result the 4-bit result.
     *  @param weight the decimal weight.
     *  @param e_result the lowest 4-bit of e_x.
     *  @param e_x the original LweSample
    */
    void _convert_DEC_to_BIN(int *result, int weight, LweSample* e_result, LweSample* e_x);
};


#endif