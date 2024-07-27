#ifndef RANDSEQ
#define RANDSEQ

///@file
///@brief this file generates random bi-sequence and restore the evaluation time.

#include <iostream>
#include "tfhe/tfhe.h"
#include <chrono>

using namespace std;

class randseq {
    protected:
    int32_t A_popcounter = 0;
    int32_t W_popcounter = 0;

    public:
    randseq();
    ~randseq();

    /** generates random 2-bit sequence for A and W.
     * @param message_A data
     * @param message_W weight
     * @param len the bit width
    */
    void _get_rand_seq(int32_t *message_A, int32_t *message_W, int32_t len);
    void _get_rand_seq(LweSample *message_A, LweSample *message_W, int32_t len);

    /** export the evaluation time to excel and print it to console. 
     * @param i the iteration array
     * @param _iter the iteration times overall
     * @param messageA data
     * @param messageB weight
     * @param evaluation_time FE-evaluation time for A x W.
    */
    void _export_time_to_excel(int i, int _iter, int32_t *messageA, int32_t *messageB,int bitwidth, double evaluation_time);
    void _export_time_to_excel(int i, int _iter, double evaluation_time, int verbose);
};

#endif