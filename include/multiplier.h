#ifndef MULTIPLIER
#define MULTIPLIER

///@file
///@brief this file defines the 4-bit multiplier with the shift_adder.

/** What's important is that all the operations are evaluated with 'COPY',
 *  so it dont't care about where the input is from.
*/

#include "tfhe/tfhe.h"
#include "shift_adder.h"

class bit4_multiplier {
    private:
    /** the unit shifts A with W to generate four inputs for the level-0 add unit. */
    bit8_shift shift_unit;

    /** the level-0 add unit[0] evaluates the sum of A<<1 and A<<2.
     *  the level-0 add unit[1] evaluates the sum of A<<3 and A<<4.
    */
    bit8_carried_adder *level0_add_unit[2];

    /** the level-1 add unit evaluates the sum of level-0 add unit's ouput. */
    bit8_carried_adder level1_add_unit;

    /** restore the result of bit4_multiplier */
    LweSample *result;

    protected:
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    bit4_multiplier();
    ~bit4_multiplier();

    /** evaluate 4-bit data A multiply by weight W.
     *  @param A the (encrypted) message A.
     *  @param W the weight.
    */
    void _multiply_evaluate(LweSample *A, int32_t *W);
    void _multiply_evaluate(int32_t *A, int32_t *W);

    /** evaluate 4-bit with Booth encoding.
     *  @param A the (encrypted) message A.
     *  @param W the weight.
    */
    void _Booth_multiply_evaluate(LweSample *A, int32_t *W);
    void _Booth_multiply_evaluate(int32_t *A, int32_t *W);

    /** export result to output(after the output is initialized).
     *  @param result the product of A x W.
    */
    void _export_result(LweSample *result);

    /** show the decrypted result of multiply operation. */
    void _show_result();

    /** show the decrypted result of multiply operation. 
     *  @return bit4_multiplier::result
    */
    LweSample* _multi_to_conv();
};

#endif