#include <tfhe/tfhe.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "global_key_generator.h"
#include "linear.h"

using namespace std;

bit8_conv_linear::bit8_conv_linear(int out, int c, int x, int y) {
    this->params = global_params;
    this->key = global_key;

    this->input_size[0] = c;
    this->input_size[1] = x;
    this->input_size[2] = y;
    this->output_size = out;

    this->weight = new int***[out];
    for(int o = 0; o<out; o++){
        this->weight[o] = new int**[c];
        for (int i = 0; i<c; i++) {
            this->weight[o][i] = new int*[x];
            for (int m = 0; m<x; m++) {
                this->weight[o][i][m] = new int[y];
            }
        }
    }
    this->bias = new int[out];

    this->result = new LweSample*[out];
    for(int i = 0; i< out; i++) {
        this->result[i] = new_gate_bootstrapping_ciphertext_array(9, this->params);
    }

    this->ACC = new_gate_bootstrapping_ciphertext_array(9, this->params);
    
    this->X = new_gate_bootstrapping_ciphertext_array(4, this->params);
}

bit8_conv_linear::~bit8_conv_linear() {
    for(int o = 0; o<this->output_size; o++) {
        for (int i = 0; i<this->input_size[0]; i++) {
            for (int m = 0; m<this->input_size[1]; m++) {
                delete[] this->weight[o][i][m];
            }
            delete[] this->weight[o][i];
        }
        delete[] this->weight[o];
    }
    delete this->weight;

    delete[] this->bias;

    for(int i = 0; i< this->output_size; i++) {
        delete_gate_bootstrapping_ciphertext_array(9, this->result[i]);    
    }
    delete[] this->result;

    delete_gate_bootstrapping_ciphertext_array(4, this->X);
    delete_gate_bootstrapping_ciphertext_array(9, this->ACC);
}

void bit8_conv_linear::_conv_linear_evaluation(LweSample ****input_layer, string filepath) {
    int temp_weight[4];
    int mode[2] = {0,0};

    LweSample* temp_bias = new_gate_bootstrapping_ciphertext_array(9, this->params);
    fstream filestream(filepath, ios::out|ios::app);

    for(int o=0; o<this->output_size;o++) {

        filestream << "[ o = "<< o << " ]"<< endl;
        for(int i=0;i<9;i++) {
            bootsSymEncrypt(&this->ACC[i], 0, this->key);
        }

        for(int i = 0; i<this->input_size[0]; i++) {
            for(int x = 0; x<this->input_size[1]; x++) {
                for(int y = 0; y<this->input_size[2]; y++) {
                    this->_convert_DEC_to_BIN(temp_weight, this->weight[o][i][x][y], this->X, input_layer[i][x][y]);
                    this->multi_unit._multiply_evaluate(this->X, temp_weight);
                    this->add_unit._bit8_evaluate(this->multi_unit._multi_to_conv(), this->ACC, mode);

                    for(int i =0;i<9;i++) {
                        // compute multisum(ACC += X(LweSample).W(plaintext))
                        lweCopy(&this->ACC[i], &this->add_unit._add_to_add()[i], this->params->in_out_params);
                        //cout << bootsSymDecrypt(&this->ACC[i], this->key);
                    }
                    //cout << endl;
                }
            }
        }

        this->_convert_DEC_to_BIN(temp_bias, this->bias[o], mode);
        this->add_unit._bit8_evaluate(this->ACC, temp_bias, mode);

        for(int i=0; i<9;i++) {
            lweCopy(&this->result[o][i], &this->add_unit._add_to_add()[i], this->params->in_out_params);
        }
        cout << "[result]: o-"<< o << endl;
        for (int i = 0; i < 9; i++)
        {
            cout << bootsSymDecrypt(&this->result[o][i], this->key);
            filestream << bootsSymDecrypt(&this->result[o][i], this->key);
        }
        cout << endl;
        filestream << endl;
    }

    filestream.close();
    delete_gate_bootstrapping_ciphertext_array(9, temp_bias);
}

void bit8_conv_linear::_show_result() {
    cout << "[Conv-Linear-result]: All element showed are LweSample." << endl;
    for(int c = 0; c < this->output_size; c++) {
        for(int i =0; i < 9; i++) {
            cout << bootsSymDecrypt(&this->result[c][i], this->key);
        }
        cout << "   ";
    }
    cout << endl;
}

void bit8_conv_linear::_convert_DEC_to_BIN(int *result, int weight, LweSample *e_result, LweSample *e_x) {
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

void bit8_conv_linear::_convert_DEC_to_BIN(LweSample *result, int bias_value, int *mode) {
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

void bit8_conv_linear::_set_weight(int ****input_weight) {
    for(int o =0; o<this->output_size; o++) {
        for(int c = 0; c< this->input_size[0]; c++) {
            for(int x = 0; x< this->input_size[1]; x++) {
                for(int y = 0; y< this->input_size[2]; y++) {
                    this->weight[o][c][x][y] = input_weight[o][c][x][y];
                }
            }
        }
    }

    cout << "[Info]: weights are loaded." << endl;
    
}

void bit8_conv_linear::_set_values(string filename) {
    string weight_name = filename + "-weight.txt";
    string bias_name = filename + "-bias.txt";

    ifstream iFile_weight(weight_name);
    ifstream iFile_bias(bias_name);
    
    string line, element;
    //stringstream linestream;

    vector<int> weight_storage;
    vector<int> bias_storage;

    while (getline(iFile_weight, line)) {
        stringstream linestream (line);
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

    for (int oc = 0; oc < this->output_size; oc++) {
        for (int ic = 0; ic < this->input_size[0]; ic++) {
            for (int sizex = 0; sizex < this->input_size[1]; sizex++) {
                for (int sizey = 0; sizey < this->input_size[2]; sizey++) {
                    this->weight[oc][ic][sizex][sizey] = weight_storage[oc * this->input_size[0] * this->input_size[1] * this->input_size[2] + ic * this->input_size[1] * this->input_size[2] + sizex * this->input_size[2] + sizey];
                }       
            }
        }
    }

    for (int oc = 0; oc < this->output_size; oc++) {
        this->bias[oc] = bias_storage[oc];
    }
}

LweSample **bit8_conv_linear::_linear_to_linear() {
    return this->result;
}



bit8_linear::bit8_linear(int out, int in) {
    this->params = global_params;
    this->key = global_key;

    this->output_size = out;
    this->input_size = in;

    this->weight = new int*[out];
    for (int i = 0; i<out; i++) {
        this->weight[i] = new int[in];
    }
    this->bias = new int[out];

    this->result = new LweSample*[out];
    for(int i = 0; i< out; i++) {
        this->result[i] = new_gate_bootstrapping_ciphertext_array(9, this->params);
    }

    this->ACC = new_gate_bootstrapping_ciphertext_array(9, this->params);
    
    this->X = new_gate_bootstrapping_ciphertext_array(4, this->params);
}

bit8_linear::~bit8_linear() {
    for (int i = 0; i<this->output_size; i++) {
        delete[] this->weight[i];
    }
    delete this->weight;
    delete[] this->bias;

    for(int i = 0; i< this->output_size; i++) {
        delete_gate_bootstrapping_ciphertext_array(9, this->result[i]);    
    }
    delete[] this->result;

    delete_gate_bootstrapping_ciphertext_array(4, this->X);
    delete_gate_bootstrapping_ciphertext_array(9, this->ACC);
}

void bit8_linear::_linear_evaluation(LweSample **input_layer, string filepath) {
    int temp_weight[4];
    int mode[2] = {0,0};

    LweSample* temp_bias = new_gate_bootstrapping_ciphertext_array(9, this->params);
    fstream filestream(filepath, ios::out|ios::app);

    for(int o=0; o < this->output_size; o++) {
        filestream << "[ o = "<< o << " ]"<< endl;
        for(int i=0;i<9;i++) {
            bootsSymEncrypt(&this->ACC[i], 0, this->key);
        }

        for(int i = 0; i < this->input_size; i++) {
            this->_convert_DEC_to_BIN(temp_weight, this->weight[o][i], this->X, input_layer[i]);
            this->multi_unit._multiply_evaluate(this->X, temp_weight);
            this->add_unit._bit8_evaluate(this->multi_unit._multi_to_conv(), this->ACC, mode);

            for(int m =0;m<9;m++) {
                // compute multisum(ACC += X(LweSample).W(plaintext))
                lweCopy(&this->ACC[m], &this->add_unit._add_to_add()[m], this->params->in_out_params);
                //cout << bootsSymDecrypt(&this->ACC[m], this->key);
            }
            //cout << endl;
        }

        this->_convert_DEC_to_BIN(temp_bias, this->bias[o], mode);
        this->add_unit._bit8_evaluate(this->ACC, temp_bias, mode);

        for(int i=0; i<9;i++) {
            lweCopy(&this->result[o][i], &this->ACC[i], this->params->in_out_params);
        }

        cout << "[result]: o-"<< o << endl;
        for (int i = 0; i < 9; i++)
        {
            cout << bootsSymDecrypt(&this->result[o][i], global_key);
            filestream << bootsSymDecrypt(&this->result[o][i], global_key);
        }
        cout << endl;
        filestream << endl;
    }

    filestream.close();

    delete_gate_bootstrapping_ciphertext_array(9, temp_bias);
}

void bit8_linear::_show_result()
{
    cout << "[Linear-result]: All element showed are LweSample." << endl;
    for(int c = 0; c < this->output_size; c++) {
        for(int i =0; i < 9; i++) {
            cout << bootsSymDecrypt(&this->result[c][i], this->key);
        }
        cout << "   ";
    }
    cout << endl;
}

void bit8_linear::_convert_DEC_to_BIN(int *result, int weight, LweSample *e_result, LweSample *e_x) {
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

void bit8_linear::_convert_DEC_to_BIN(LweSample *result, int bias_value, int *mode) {
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

void bit8_linear::_set_weight(int **intput_weight) {
    for(int i = 0; i< this->input_size; i++) {
        for(int m = 0; m< this->output_size; m++) {
            this->weight[i][m] = intput_weight[i][m];
        }
    }

    cout << "[Info]: weights are loaded." << endl;
}

void bit8_linear::_set_values(string filename) {
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

    for (int oc = 0; oc < this->output_size; oc++) {
        for (int ic = 0; ic < this->input_size; ic++) {
            this->weight[oc][ic] = weight_storage[oc* this->input_size + ic];
        }
    }

    for (int oc = 0; oc < this->output_size; oc++) {
        this->bias[oc] = bias_storage[oc];
    }
}

LweSample **bit8_linear::_linear_to_linear() {
    return this->result;
}
