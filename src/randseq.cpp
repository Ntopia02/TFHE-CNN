#include "randseq.h"
#include "transform_R2B.h"
#include "global_key_generator.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

randseq::randseq() {
    srand(time(NULL));
}

randseq::~randseq() {}

void randseq::_get_rand_seq(int32_t *message_A, int32_t *message_W , int32_t len) {

    for(int i = 0; i < len; i++) {
        message_A[i] = rand() % 2;
        if (message_A[i] == 1) {
            this->A_popcounter++;
        } 
    }

    for(int i = 0; i < len; i++) {
        message_W[i] = rand() % 2;
        if (message_W[i] == 1) {
            this->W_popcounter++;
        } 
    }

}

void randseq::_get_rand_seq(LweSample *message_A, LweSample *message_W, int32_t len) {
    for(int i = 0; i < len; i++) {
        bit4_bootsSymEncrypt(&message_A[i], rand() % 2, global_key);
        bit4_bootsSymEncrypt(&message_W[i], rand() % 2, global_key);
    }
}

void randseq::_export_time_to_excel(int i, int _iter,int32_t *messageA, int32_t *messageB,int bitwidth, double evaluation_time) {
    cout << "[" << i << "/" << _iter << "]" << endl;
    
    ofstream oFile;
    string str_A = to_string(this->A_popcounter);
    string str_W = to_string(this->W_popcounter);
    string str_bitwidth = to_string(bitwidth);
    string filePath = "./evaluation/";
    string fileName = str_A + "A" +str_W + "W"+ "scalable-"+str_bitwidth+"bit.csv";
    oFile.open(filePath+fileName, ios::out | ios::app);
    oFile << evaluation_time / CLOCKS_PER_SEC << endl;
    oFile.close();

    /** initailize the counter of A and W. */
    this->A_popcounter = 0;
    this->W_popcounter = 0;
}

void randseq::_export_time_to_excel(int i, int _iter, double evaluation_time, int verbose) {
    ofstream oFile;
    cout << "[evaluation_time]: " << evaluation_time / CLOCKS_PER_SEC << endl;
    cout << "[" << i + 1 << "/" << _iter << "] "<< endl << endl;

    string filePath = "./evaluation/";
    string fileName;
    if (verbose == 0) {
        fileName = "bit4Add.csv";
    }
    else if(verbose == 1){
        fileName = "GateAdd.csv";
    }
    else {
        fileName = "scalable-16bit.csv";
    }
    oFile.open(filePath+fileName, ios::out | ios::app);
    oFile << evaluation_time / CLOCKS_PER_SEC << endl;
    oFile.close();
}
