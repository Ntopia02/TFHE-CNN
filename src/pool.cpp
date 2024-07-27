#include <tfhe/tfhe.h>
#include <iostream>
#include "pool.h"
#include "global_key_generator.h"

using namespace std;

bit4_pool::bit4_pool(int channel, int input_layer_size, int kernel_size, int stride, int padding) {
    this->channel = channel;
    this->params = global_params;
    this->key = global_key;

    this->input_layer_size = input_layer_size;
    this->kernel_size = kernel_size;
    this->stride = stride;

    this->output_layer_size = (input_layer_size - kernel_size) / stride + 1;

    this->result = new LweSample***[this->channel];
    for (int c = 0; c < this->channel; c++) {
        this->result[c] = new LweSample**[this->output_layer_size];
        for (int x = 0; x < this->output_layer_size; x++) {
            this->result[c][x] = new LweSample*[this->output_layer_size];
            for (int y = 0; y < this->output_layer_size; y++) {
                this->result[c][x][y] = new_gate_bootstrapping_ciphertext_array(9, this->params);
            }
        }
    }

    this->max_sample = new_gate_bootstrapping_ciphertext_array(9, this->params);
}

bit4_pool::~bit4_pool() {
    for (int c = 0; c < this->channel; c++) {
        for (int x = 0; x < this->output_layer_size; x++) {
            for (int y = 0; y < this->output_layer_size; y++) {
                delete_gate_bootstrapping_ciphertext_array(9, this->result[c][x][y]);
            }
            delete[] result[c][x];
        }
        delete[] result[c];
    }
    delete[] result;

    delete_gate_bootstrapping_ciphertext_array(9, max_sample);
}

void bit4_pool::_max_pool(LweSample ****input) {
    int oh, ow;
    int mode[2] = {0, 1};
    LweSample* temp_max = new_gate_bootstrapping_ciphertext_array(9, this->params);
    bootsSymEncrypt(&temp_max[8], 1, this->key);
    for (int c = 0; c < this->channel; c++) {
        cout << "begin evaluation of c " << c << "." << endl;
        oh = 0;
        for (int h = 0; h < this->input_layer_size - this->kernel_size + 1; h += this->stride ) {
            ow = 0;
            for (int w = 0; w < this->input_layer_size - this->kernel_size + 1; w += this->stride ) {   
                for(int i = 0; i< 8; i++) {
                    bootsSymEncrypt(&this->max_sample[i], 0, this->key);}
                bootsSymEncrypt(&this->max_sample[8], 1, this->key);
                cout << "begin compare ["<<h<<"]" << "[" <<w<< "]." << endl;
                for(int x = 0 ; x < this->kernel_size; x++) {
                    for(int y = 0 ; y < this->kernel_size; y++) {
                        for(int i = 0; i < 8; i++) {
                            bootsCOPY(&temp_max[i], &this->max_sample[i], &this->key->cloud);}
                        for(int i = 0; i < 8; i++) {
                            bootsNOT(&temp_max[i], &temp_max[i], &this->key->cloud);}
                        // evaluate input - max_sample 
                        this->subtract_unit._bit8_evaluate(input[c][h+x][w+y], temp_max, mode);
                        for(int i = 0; i < 8; i++) {
                            bootsMUX(&this->max_sample[i], &this->subtract_unit._add_to_add()[8],
                             &this->max_sample[i], &input[c][h+x][w+y][i], &this->key->cloud);}
                    }
                }
                //deliver the value to result.
                for(int i = 0; i< 8; i++) {
                    lweCopy(&this->result[c][oh][ow][i], &this->max_sample[i], this->params->in_out_params);}
                bootsSymEncrypt(&this->result[c][oh][ow][8], 0, this->key);
                ow++;
            }
            oh++;
        }
    }
    delete_gate_bootstrapping_ciphertext_array(9, temp_max);
}

void bit4_pool::_show_result()
{
    cout << "[Pool-result]: All element showed are LweSample." << endl;
    for(int c= 0; c < this->channel; c++) {
        cout << "channel["<< c <<"]:" << endl;
        for(int x= 0; x < this->output_layer_size; x++) {
            for(int y= 0; y < this->output_layer_size; y++) {
                for(int i = 0; i < 9 ;i++) {
                    cout << bootsSymDecrypt(&this->result[c][x][y][i], this->key);
                }
                cout << "   ";
            }
            cout << endl;
        }
    }
}

LweSample ****bit4_pool::_pool_to_conv() {
    return this->result;
}
