#ifndef SHIFT_ADDER
#define SHIFT_ADDER

///@file
///@brief this file defines the class of carried adder with tfhe.

#include <tfhe/tfhe.h>

class carried_adder {
    private:
    /** Enc_in[0] refers to Lwe(A), Enc_in[1] refers to Lwe(B) and Enc_in[2] refers to Lwe(Ci-1). */
    LweSample *Enc_in;
    /** Enc_out[0] refers to Lwe(Si), and Enc_out[1] refers to Lwe(Ci). */
    LweSample *Enc_out;

    protected:
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    carried_adder();
    ~carried_adder();

    /** Evaluate 1-bit addition which consists of gate-bootstrapping operations. 
     * @param S restore Enc_out[0].
     * @param C restore Enc_out[1].
     */
    void _gate_evaluate(LweSample *S, LweSample *C);

    /** Encrypt 3-bits input [Ai, Bi] to LweSamples Enc_in[0:2] and C to Enc_in[2].
     * @param message 2-bits to be encrypted.
     * @param C LweSample of carry-bit
     */
    void _load_message(int32_t *message, LweSample *C);
    void _load_message(LweSample *A, LweSample *B, LweSample *C);

    /** Import Enc_out[0:2] to S and C with lwecopy.
     * @param S the destination of Enc_out[0].
     * @param C the destination of Enc_out[1].
     */
    void _import_message(LweSample *S, LweSample *C);

    /** Show param->n, Enc_in[index]->a, Enc_in[index]->b, [Dec_in[index]]
     * @param index the position to be shown.
     */
    void _show_Lwe(uint32_t index);
};

class bit8_carried_adder : public carried_adder {
    protected:
    LweSample *C;
    LweSample *S;

    public:
    bit8_carried_adder();
    ~bit8_carried_adder();

    /** evaluate add-operation of 8-bit A and B.
     * @param A array of encoded message(A).
     * @param B array of encoded message(B).
     * @param mode C0 value for subtracting in two's complement.
     */
    void _bit8_evaluate(int32_t *A, int32_t *B, int* mode);
    void _bit8_evaluate(LweSample *A, LweSample *B, int* mode);

    /** export S to result.
     *  @param result the output to restore.
    */
    void _export_result(LweSample *result);

    /** visit Enc(Si) and decrypt it to message. 
     * @param index the position to be shown.
    */
    void _bit8_show_Lwe(uint32_t index);
    void _bit8_show_Lwe();

    /** return the member S directly for level1_add_unit's input.*/
    LweSample *_add_to_add();
};

class bit8_shift {
    private:
    LweSample *temp_input;
    LweSample *shifted_S[2][2];

    /** mode and mode_2 stands for the input state of 'shift to add_0' and 'add_0 to add_1' seperatively
     *  and the value -1 means there'll be a straight-through process.
    */
    int mode[2][2] = {{-1,-1},{-1,-1}};
    int mode_2[2] = {0, 0};

    protected:
    TFheGateBootstrappingParameterSet *params;
    TFheGateBootstrappingSecretKeySet *key;

    public:
    bit8_shift();
    ~bit8_shift();

    /** shift A seperatively in W[0:4] with result restored in shifted_S[2][2].
     * @param A the input 4-bit LweSample for shifting.
     * @param W the input weight for shifting.
    */
    void _shift_evaluate(LweSample *A, int32_t *W);
    void _shift_evaluate(int32_t *A, int32_t *W);

    /** shift with the Booth encoding.
     * @param A the input 4-bit LweSample for shifting.
     * @param W the input weight for shifting.
    */
    void _Booth_shift_evaluate(LweSample *A, int32_t *W);
    void _Booth_shift_evaluate(int32_t *A, int32_t *W);

    /** export shifted_S[0][0:2] to result_0 and shifted_S[1][0:2] to result_1.
     *  @param result_0 the little endian output.
     *  @param result_1 the big endian output.
    */
    void _export_result(LweSample **result_0, LweSample ** result_1);

    /** show the 4 results of shifted_S */
    void _show_result();

    /** return the member shifted_S directly for level0_add_unit's input. 
     *  @return bit8_shift::shifted_S
    */
    LweSample *(*_shift_to_add())[2];

    /** return the mode of litte endian output or big endian output. 
     *  @return bit8_shift::mode
    */
    int (*_shift_mode_to_add())[2];

    /** return the mode of the two input of the level_2 carried_adder. 
     *  @return bit8_shift::mode_2
    */
    int *_shift_mode_to_add_2();
};

#endif