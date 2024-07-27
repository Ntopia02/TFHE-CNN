#include "tfhe/tfhe.h"
#include "global_key_generator.h"
#include "convolution.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <string>
#include <unistd.h>
#include <limits.h>

using namespace std;

struct Exception42 {
};

bit4_convolution::bit4_convolution(int input_size, int padding, int out_channel, int in_channel, int kernel_size, int stride) 
: multi_unit(), add_unit() {
    this->params = global_params;
    this->key = global_key;

    this->ACC = new_gate_bootstrapping_ciphertext_array(9, this->params);
    
    this->X = new_gate_bootstrapping_ciphertext_array(4, this->params);

    /** input layer */
    this->input_size = input_size;
    this->padding = padding;

    /** convolution kernel */
    this->out_channel = out_channel;
    this->in_channel = in_channel;
    this->kernel_size = kernel_size;
    this->stride = stride;

    /** output layer */
    this->output_size = (this->input_size + 2 * this->padding - this->kernel_size) / this->stride + 1;

    this->kernel = new int***[this->out_channel];
    for(int i=0; i < this->out_channel; i++) {
        this->kernel[i] = new int **[this->in_channel];
        for(int j=0; j < this->in_channel; j++) {
            this->kernel[i][j] = new int *[this->kernel_size];
            for(int k=0; k < this->kernel_size; k++) {
                this->kernel[i][j][k] = new int[this->kernel_size];
                for(int l=0; l < this->kernel_size; l++) {
                    this->kernel[i][j][k][l] = 0;
                }
            }
        }
    }

    this->result = new LweSample***[this->out_channel];
    for(int i=0; i < this->out_channel; i++) {
        this->result[i] = new LweSample**[this->output_size];
        for(int j=0; j< this->output_size; j++){
            this->result[i][j] = new LweSample*[this->output_size];
            for(int k=0; k< this->output_size; k++) {
                this->result[i][j][k] = new_gate_bootstrapping_ciphertext_array(9, this->params);
            }
        }
    }

    this->bias = new int[this->out_channel];
}

bit4_convolution::~bit4_convolution() {
    for(int i=0; i < this->out_channel; i++) {
        for(int j=0; j < this->kernel_size; j++) {
            for(int k=0; k < this->kernel_size; k++) {
                delete_gate_bootstrapping_ciphertext_array(9, this->result[i][j][k]);
            }
            delete[] this->result[i][j];
        }
        delete[] this->result[i];
    }
    delete[] this->result;

    for(int i=0; i < this->out_channel; i++) {
        for(int j=0; j < this->in_channel; j++) {
            for(int k=0; k < this->kernel_size; k++) {
                delete[] this->kernel[i][j][k];
            }
            delete[] this->kernel[i][j];
        }
        delete[] this->kernel[i];
    }

    delete[] this->bias;
    
    delete_gate_bootstrapping_ciphertext_array(4, this->X);
    delete_gate_bootstrapping_ciphertext_array(9, this->ACC);
}

void bit4_convolution::_set_default_values() {
    for(int i=0; i < this->out_channel; i++) {
        for(int j=0; j< this->in_channel; j++){
            for(int k=0; k< this->kernel_size; k++) {
                for(int l=0; l< this->kernel_size; l++) {
                    this->kernel[i][j][k][l] = 1;
                }
            }
        }
    }
}

void bit4_convolution::_set_values(string filename) {
    string weight_name = filename + "-weight.txt";
    string bias_name = filename + "-bias.txt";

    ifstream iFile_weight(weight_name);
    ifstream iFile_bias(bias_name);

    string line, element;
    //stringstream linestream;

    vector<int> weight_storage;
    vector<int> bias_storage;

    while (getline(iFile_weight, line)) {
        stringstream linestream(line);
        while(getline(linestream, element, ',')) {
            weight_storage.push_back(stoi(element));
        }
    }
    
    iFile_weight.close();

    while (getline(iFile_bias, line)) {
        stringstream linestream(line);
        while(getline(linestream, element, ',')) {
            bias_storage.push_back(stoi(element));
        }
    }

    iFile_bias.close();

    for (int oc = 0; oc < this->out_channel; oc++) {
        for (int ic = 0; ic < this->in_channel; ic++) {
            for (int sizex = 0; sizex < this->kernel_size; sizex++) {
                for (int sizey = 0; sizey < this->kernel_size; sizey++) {
                    this->kernel[oc][ic][sizex][sizey] = weight_storage[oc * this->in_channel * this->kernel_size * kernel_size + ic * this->kernel_size * this->kernel_size + sizex * this->kernel_size + sizey];
                }       
            }
        }
    }

    for (int oc = 0; oc < this->out_channel; oc++) {
        this->bias[oc] = bias_storage[oc];
    }
}

void bit4_convolution::_conv_evaluate(LweSample ****input_layer, string restore_filename) {
    // initialize temp variable.
    int weight[4];
    int mode[2]={0,0};
    int oh, ow;
    ofstream filestream(restore_filename, ios::out|ios::app);

    cout << "[input-info]: " << endl;
    for (int ic = 0; ic < this->in_channel; ic++) {
        for (int x = 0; x < this->input_size; x++) {
            for (int y = 0; y < this->input_size; y++) {
                for (int bit = 0; bit < 9; bit++) {
                    cout << bootsSymDecrypt(&input_layer[ic][x][y][bit], this->key);
                }
                cout << "   ";
            }
            cout << endl;
        }
        cout << endl;
    }
    
    LweSample* temp_bias = new_gate_bootstrapping_ciphertext_array(9, this->params);

    for(int oc = 0; oc < this->out_channel; oc++) {

        /** The loop is to simulate the convolution computation with tfhe.
         * @param h input_layer's height.
         * @param w: input_layer's weight.
         * @param ic: input_layer's channels.
         * @param x: kernel's height.
         * @param y: kernel's weight.
        */
        filestream << "[ oc = "<< oc << " ]"<< endl;
        filestream.flush();
        oh = 0;
        for(int h = 0; h < this->input_size + 2*this->padding - this->kernel_size + 1; h+=this->stride) {            
            ow = 0;
            for(int w = 0; w < this->input_size + 2*this->padding - this->kernel_size + 1; w+=this->stride) {
                // initialize ACC.
                for(int i=0;i<9;i++) {
                    bootsSymEncrypt(&this->ACC[i], 0, this->key);
                }

                // initialize mode.
                mode[0] = 0;
                mode[1] = 0;

                // homomorphic evaluation.
                for(int ic = 0; ic< this->in_channel; ic++) {
                    for(int x = 0; x< this->kernel_size; x++) {
                        for(int y = 0; y< this->kernel_size; y++) {
                            if (this->padding!=0 && ((h+x < this->padding) || (w+y < this->padding)
                            || (this->input_size - this->padding < h+x) || (this->input_size - this->padding < w+y))) {
                                // in this case, it means pointing to a padding area(0) in input_layer, so just continue.
                                continue;
                            }
                            cout << "[Location]: oc-" << oc << endl;
                            cout << "[input]: oh-" << oh << " ow-" << ow << endl;
                            cout << "[kernel]: in_channel-" << ic <<" x-"<<x<<" y-"<<y << endl;
                            this->_convert_DEC_to_BIN(weight, this->kernel[oc][ic][x][y], this->X, input_layer[ic][h+x][w+y]);

                            cout << "cipher-factor: " << bootsSymDecrypt(&this->X[0],this->key) << bootsSymDecrypt(&this->X[1],this->key)<< bootsSymDecrypt(&this->X[2],this->key) << bootsSymDecrypt(&this->X[3],this->key) << endl;
                            cout << "plain-factor: " << weight[0] << weight[1] << weight[2] << weight[3] << endl;

                            this->multi_unit._multiply_evaluate(this->X, weight);

                            this->add_unit._bit8_evaluate(this->multi_unit._multi_to_conv(), this->ACC, mode);

                            for(int i =0;i<9;i++) {
                                // compute multisum(ACC += X(LweSample).W(plaintext))
                                lweCopy(&this->ACC[i], &this->add_unit._add_to_add()[i], this->params->in_out_params);
                                cout << bootsSymDecrypt(&this->ACC[i], this->key);
                            }
                            cout << endl;
                        }
                    }
                }

                this->_convert_DEC_to_BIN(temp_bias, this->bias[oc], mode);

                this->add_unit._bit8_evaluate(this->ACC, temp_bias, mode);

                // export ACC to output_layer.
                for(int i =0;i<9;i++) {
                    lweCopy(&this->result[oc][oh][ow][i], &this->add_unit._add_to_add()[i], this->params->in_out_params);
                    cout << bootsSymDecrypt(&this->result[oc][oh][ow][i], this->key);
                    
                    int temp_int;
                    temp_int = bootsSymDecrypt(&this->result[oc][oh][ow][i], this->key);
                    filestream << temp_int;
                }
                cout << endl;
                filestream << "    ";
                filestream.flush();
                ow += 1;
            }
            filestream << endl;
            filestream.flush();
            oh += 1;
        }
    }

    filestream.close();
    delete_gate_bootstrapping_ciphertext_array(9, temp_bias);
}

void bit4_convolution::_convert_DEC_to_BIN(int *result, int weight, LweSample* e_result, LweSample* e_x) {
    if(!(weight>=0 && weight < 16)) {
        cerr << "[Error]: weight is beyond limit." << endl;
    }
    
    for(int i =0; i < 4; i++) {
        if (weight > 0) {
            result[i] = weight % 2;
            weight = weight / 2;
        }
        else {
            result[i] = 0;
        }
        // fetch and copy the lowest 4-bits to X.
        lweCopy(&e_result[i], &e_x[i], this->params->in_out_params);
    }
}

void bit4_convolution::_convert_DEC_to_BIN(LweSample *result, int bias_value, int* mode) {
    if(!(bias_value < 512 && bias_value > -512)) {
        cerr << "[Error]: weight is beyond limit." << endl;
    }

    if (bias_value < 0) {
        bootsSymEncrypt(&result[8], 1, this->key);

        bias_value = abs(bias_value);
        for (int i = 0; i < 8; i++) {
            if (bias_value > 0) {
                bootsSymEncrypt(&result[i], !(bool(bias_value % 2)), this->key);
                bias_value /= 2;
            }

            else {
                bootsSymEncrypt(&result[i], 1, this->key);
            }
        }
        mode[1] = 1;
    }
    else if (bias_value > 0){
        bootsSymEncrypt(&result[8], 0, this->key);

        bias_value = abs(bias_value);
        for (int i = 0; i < 8; i++) {
            if (bias_value > 0) {
                bootsSymEncrypt(&result[i], bias_value % 2, this->key);
                bias_value /= 2;
            }

            else {
                bootsSymEncrypt(&result[i], 0, this->key);
            }
        }
        mode[1] = 0;
    }
    else {
        for (int i = 0; i < 9; i++) {
            bootsSymEncrypt(&result[i], 0, this->key);
        }
        mode[1] = -1;
    }

    cout << "[Bias]: ";
    for (int i = 0; i < 9; i++)
    {
        cout << bootsSymDecrypt(&result[i], this->key);
    }
    cout << endl;
    
}

void bit4_convolution::_activate_evaluate() {
    for (int oc = 0; oc < this->out_channel; oc++) {
        for (int sizex = 0; sizex < this->output_size; sizex++) {
            for (int sizey = 0; sizey < this->output_size; sizey++) {
                this->activate_unit._relu(this->result[oc][sizex][sizey], this->result[oc][sizex][sizey]);
            }
        }
    }   
    cout << "[Conv-activation]: All element in the conv layer are activated." << endl;
}

void bit4_convolution::_show_result() {
    cout << "[Conv-result]: All element showed are LweSample." << endl;
    for(int oc = 0; oc < this->out_channel; oc++) {
        cout << "channel["<< oc <<"]:" << endl;
        for (int r = 0; r < this->output_size; r++) {
            for (int c = 0; c < this->output_size; c++) {
                for(int i =0; i < 9; i++) {
                    cout << bootsSymDecrypt(&this->result[oc][r][c][i], this->key);
                }
                cout << "       ";
            }
            cout << endl;
        }
    }
}

void bit4_convolution::_show_values() {
    cout << "[Kernel]: " << endl;

    for (int oc = 0; oc < this->out_channel; oc++) {
        for (int ic = 0; ic < this->in_channel; ic++) {
            for (int x = 0; x < this->kernel_size; x++) {
                for (int y = 0; y < this->kernel_size; y++)
                {
                    cout << kernel[oc][ic][x][y] << "   ";
                }
                cout << endl;
            }
            cout << endl;
        }
        cout << endl;
    }

    cout << "[Bias]: " << endl;
    for (int oc = 0; oc < this->out_channel; oc++) {
        cout << this->bias[oc] << " ";
    }
    cout << endl;
}

LweSample ****bit4_convolution::_activate_to_pool() {
    return this->result;
}
