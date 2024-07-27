#include <iostream>
#include <string.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>

#include <tfhe/tfhe.h>

#include "global_key_generator.h"
#include "shift_adder.h"
#include "multiplier.h"
#include "convolution.h"
#include "activation_function.h"
#include "pool.h"
#include "linear.h"
#include "randseq.h"
#include "scalable_multiplier.h"
#include "transform_R2B.h"

using namespace std;

void initialize_global_key(const int minimum_lambda, uint32_t* seed_values) {
    global_params = new_default_gate_bootstrapping_parameters(minimum_lambda);
    tfhe_random_generator_setSeed(seed_values, sizeof(seed_values));
    global_key = new_random_gate_bootstrapping_secret_keyset(global_params);
}

void _load_img(LweSample ****img, string imgname) {
    ifstream iFile(imgname);

    vector<int> storage;

    string line, element;
    stringstream linestream;

    while(getline(iFile, line)) {
        linestream << line;
        while(getline(linestream, element, ',')) {
            storage.push_back(stoi(element));
        }
    }

    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 28; j++) {
            for (int k = 0; k < 28; k++) {
                int temp_dec = storage[i*28*28+j*28+k];
                for (int bit = 0; bit < 8; bit++) {
                    if (temp_dec == 0) {
                        bootsSymEncrypt(&img[i][j][k][bit], 0, global_key);
                    }
                    else {
                        bool remainder = temp_dec % 2;
                        bootsSymEncrypt(&img[i][j][k][bit], remainder, global_key);
                        temp_dec /= 2;
                    }
                }
                bootsSymEncrypt(&img[i][j][k][8], 0, global_key);
            }
        }
    }

    iFile.close();
}

void _show_img(LweSample ****img) {
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 28; j++) {
            for (int k = 0; k < 28; k++) {
                for (int bit = 0; bit < 9; bit++) {
                    cout << bootsSymDecrypt(&img[i][j][k][bit], global_key);
                }
                cout << "   ";
            }
            cout << endl;
        }
    }
    
}

void tfhe_MNIST_2levelCNN_inference(LweSample**** img_input, vector<string> filename, string timename, bool verbose) {
    double start_time, end_time;
    fstream timestream(timename, ios::out);

    if (verbose) {
        cout << "[TFHE-CNN]: begin convolution process." << endl;
        cout << "[TFHE-CNN]: begin conv1."<<  endl;
    }

    //conv1
    start_time = clock();

    bit4_convolution conv1(28, 0, 1, 1, 3, 1);
    conv1._set_values(filename[0]);
    conv1._show_values();

    conv1._conv_evaluate(img_input, "./result/conv1-1x1x3x3.txt");
    conv1._activate_evaluate();
    bit4_pool pool1(1, 26, 2, 2, 0);
    pool1._max_pool(conv1._activate_to_pool());

    end_time = clock();

    if (verbose) {
        cout << "[TFHE-CNN]: begin conv2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;
    }
    timestream << "[TFHE-CNN]: begin conv2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;


    //conv2
    start_time = clock();

    bit4_convolution conv2(13, 0, 2, 1, 3, 1);
    conv2._set_values(filename[1]);
    conv2._conv_evaluate(pool1._pool_to_conv(), "./result/conv2-2x1x3x3.txt");
    conv2._activate_evaluate();
    bit4_pool pool2(2, 11, 2, 2, 0);
    pool2._max_pool(conv2._activate_to_pool());

    end_time = clock();

    if (verbose) {
        cout << "[TFHE-CNN]: convolution end, linear1 begin, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;
    }
    timestream << "[TFHE-CNN]: convolution end, linear1 begin, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;

    //linear1
    start_time = clock();

    bit8_conv_linear conv_linear(32, 2, 5, 5);
    conv_linear._set_values(filename[2]);
    conv_linear._conv_linear_evaluation(pool2._pool_to_conv(), "./result/linear1-32x2x5x5.txt");
    end_time = clock();

    if (verbose) {
        cout << "[TFHE-CNN]: begin linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;
    }
    timestream << "[TFHE-CNN]: begin linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl; 

    //linear2
    start_time = clock();

    bit8_linear linear(10, 32);
    linear._set_values(filename[3]);
    linear._linear_evaluation(conv_linear._linear_to_linear(), "./result/linear2-10x32.txt");
    end_time = clock();

    if (verbose) {
        cout << "[TFHE-CNN]: end linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl;
    }
    timestream << "[TFHE-CNN]: begin linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl; 

    linear._show_result();
}

void test_linear2(LweSample** input) {
    fstream timestream("./result/time.txt", ios::out|ios::app);

    double start_time, end_time;

    start_time = clock();

    bit8_linear linear(10, 32);
    linear._set_values("./quantized-information/simple/linear2-10x32");
    linear._linear_evaluation(input, "./result/linear2-10x32.txt");

    end_time = clock();

    timestream.flush();
    cout << "[TFHE-CNN]: end linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl; 
    timestream << "[TFHE-CNN]: end linear2, latency is " << (end_time - start_time) /CLOCKS_PER_SEC << "s"<< endl; 

    timestream.close();
    linear._show_result();
}


int main() {
    int minimum_lambda = 110;
    uint32_t seed[] = {314, 1592, 657};

    initialize_global_key(minimum_lambda, seed);

    /////////////////////////
    /* LweSample ** input_test;

    vector<string> input_s = {
        "000001000",
        "110101100",
        "111100010",
        "011101100",
        "110101000",
        "110000100",
        "001010110",
        "001010010",
        "001111100",
        "011101110",
        "100001000",
        "110001010",
        "001001100",
        "110011100",
        "011100110",
        "101111000",
        "001101000",
        "010111011",
        "011100010",
        "100000100",
        "011111010",
        "000110000",
        "000010000",
        "001011011",
        "001101111",
        "010000010",
        "100011100",
        "011100010",
        "000111000",
        "010111100",
        "110110010",
        "000100100"};

    input_test = new LweSample*[32];
    for (int i = 0; i < 32; i++) {
        input_test[i] = new_gate_bootstrapping_ciphertext_array(9, global_params);
    }

    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            char element = input_s[i][j];
            bootsSymEncrypt(&input_test[i][j], element - '0', global_key);
        }
    }

    test_linear2(input_test); */
    /////////////////////////

    string folder = "./quantized-information/binaried[uint1]/";

    string imgname = "./quantized-information/MNIST-number7.txt";
    vector<string> weightANDbias = {folder+"conv1-1x1x3x3", folder+"conv2-2x1x3x3", folder+"linear1-32x2x5x5", folder+"linear2-10x32"};
    string timename = "./result/binaried[uint1]/time.txt";

    bool verbose = 1;

    LweSample ****img;
    img = new LweSample***[1];
    for (int i = 0; i < 1; i++) {
        img[i] = new LweSample**[28];
        for (int j = 0; j < 28; j++) {
            img[i][j] = new LweSample*[28];
            for (int k = 0; k < 28; k++) {
                img[i][j][k] = new_gate_bootstrapping_ciphertext_array(9, global_params);
            }
        }
    }
    
    _load_img(img, imgname);

    //_show_img(img);
    tfhe_MNIST_2levelCNN_inference(img, weightANDbias, timename, verbose);

    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 28; j++) {
            for (int k = 0; k < 28; k++) {
                delete_gate_bootstrapping_ciphertext_array(9, img[i][j][k]);
            }
            delete[] img[i][j];
        }
        delete[] img[i];
    }
    delete[] img;

    delete_gate_bootstrapping_secret_keyset(global_key);
    delete_gate_bootstrapping_parameters(global_params);
    return 0;
}