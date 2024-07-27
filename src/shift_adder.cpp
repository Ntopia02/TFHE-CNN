#include <tfhe/tfhe.h>
#include "global_key_generator.h"
#include "shift_adder.h"
#include <iostream>

using namespace std;

struct Exception42 {
};

carried_adder::carried_adder() {
    this->params = global_params;
    this->key = global_key;

    this->Enc_in = new_gate_bootstrapping_ciphertext_array(3, this->params);
    this->Enc_out = new_gate_bootstrapping_ciphertext_array(2, this->params);
}

carried_adder::~carried_adder() {
    delete_gate_bootstrapping_ciphertext_array(2, this->Enc_out);
    delete_gate_bootstrapping_ciphertext_array(3, this->Enc_in);
}

void carried_adder::_gate_evaluate(LweSample *S, LweSample *C)
{
    bootsXOR(&this->Enc_out[0], &this->Enc_in[0], &this->Enc_in[1], &this->key->cloud);
    bootsAND(&this->Enc_out[1], &this->Enc_in[0], &this->Enc_in[1], &this->key->cloud);
    bootsAND(&this->Enc_out[0], &this->Enc_out[0], &this->Enc_in[2], &this->key->cloud);
    bootsOR(&this->Enc_out[1], &this->Enc_out[0], &this->Enc_out[1], &this->key->cloud);

    bootsXOR(&this->Enc_out[0], &this->Enc_in[0], &this->Enc_in[1], &this->key->cloud);
    bootsXOR(&this->Enc_out[0], &this->Enc_out[0], &this->Enc_in[2], &this->key->cloud);
    this->_import_message(S, C);
}

void carried_adder::_load_message(int32_t *message, LweSample *C) {
    for (int i = 0; i < 2; i++) {
        bootsSymEncrypt(&this->Enc_in[i], message[i], this->key);
    }
    lweCopy(&this->Enc_in[2], C, this->params->in_out_params);
}

void carried_adder::_load_message(LweSample *A, LweSample *B, LweSample *C) {
    lweCopy(&this->Enc_in[0], A, this->params->in_out_params);
    lweCopy(&this->Enc_in[1], B, this->params->in_out_params);
    lweCopy(&this->Enc_in[2], C, this->params->in_out_params);
}

void carried_adder::_import_message(LweSample *S, LweSample *C) {
    lweCopy(S, &this->Enc_out[0], this->params->in_out_params);
    lweCopy(C, &this->Enc_out[1], this->params->in_out_params);
}

void carried_adder::_show_Lwe(uint32_t index) {
    if (index < 3) {
        int32_t _n = this->params->in_out_params->n;
        LweSample* _show_sample = &this->Enc_in[index];

        cout << "[param->n]: " << _n << endl;
        cout << "[Enc_in[" << index << "]->a]: ";
        for(int i = 0; i < _n; i++) {
           cout << _show_sample->a[i] << " ";
        }
        cout << endl;
        cout << "[Enc_in[" << index << "]->b]: " << _show_sample->b << endl;
        cout << "[Dec_in[" << index << "]]: " << bootsSymDecrypt(&this->Enc_in[index], this->key) << endl;
    }
    else {
        cerr << "[error]: Out of range!" << endl;
    }
}

bit8_carried_adder::bit8_carried_adder() {
    this->C = new_gate_bootstrapping_ciphertext(this->params);
    this->S = new_gate_bootstrapping_ciphertext_array(9, this->params);
}

bit8_carried_adder::~bit8_carried_adder() {
    delete_gate_bootstrapping_ciphertext_array(9, this->S);
    delete_gate_bootstrapping_ciphertext(this->C);
}

void bit8_carried_adder::_bit8_evaluate(int32_t *A, int32_t *B, int* mode) {
    int32_t message[] = {0, 0};

    /** straight-through B to result. */
    if (mode[0] == -1) {
        for (int i = 0; i < 9; i++) {
            bootsSymEncrypt(&this->S[i], B[i], this->key);
        }
        return;
    }

    /** straight-through A to result. */
    else if (mode[1] == -1) {
        for (int i = 0; i < 9; i++) {
            bootsSymEncrypt(&this->S[i], A[i], this->key);
        }
        return;
    }

    else if(mode[0] == 1 || mode[1] == 1) bootsSymEncrypt(this->C, 1, this->key);
    else bootsSymEncrypt(this->C, 0, this->key);

    for (int i = 0; i < 9; i++) {
        message[0] = A[i];
        message[1] = B[i];
        this->_load_message(message, this->C);
        this->_gate_evaluate(&this->S[i], this->C);
    }
}

void bit8_carried_adder::_bit8_evaluate(LweSample *A, LweSample *B, int* mode) {
    /** straight-through B to result. */
    if (mode[0] == -1) {
        for (int i = 0; i < 9; i++) {
           lweCopy(&this->S[i], &B[i], this->params->in_out_params);
        }
        //cout << "passto B" <<endl;
        return;
    }

    /** straight-through A to result. */
    else if (mode[1] == -1) {
        for (int i = 0; i < 9; i++) {
           lweCopy(&this->S[i], &A[i], this->params->in_out_params);
        }
        //cout << "passto A" <<endl;
        return;
    }

    if(mode[0] == 1 || mode[1] == 1) bootsSymEncrypt(this->C, 1, this->key);
    else bootsSymEncrypt(this->C, 0, this->key);

    for (int i = 0; i < 9; i++) {
        this->_load_message(&A[i], &B[i], this->C);
        this->_gate_evaluate(&this->S[i], this->C);
    }
}

void bit8_carried_adder::_export_result(LweSample *result) {
    for(int i= 0; i<9; i++) {
        lweCopy(&result[i], &this->S[i], this->params->in_out_params);
    }
}

void bit8_carried_adder::_bit8_show_Lwe(uint32_t index)
{
    if (index < 9) {
        LweSample* _show_sample = &this->S[index];
        cout << "[S[" << index << "]]: " << bootsSymDecrypt(&this->S[index], this->key) << endl;
    }
    else {
        cerr << "[error]: Out of range!" << endl;
    }
}

void bit8_carried_adder::_bit8_show_Lwe() {
    cout << "[result]:" << endl;
    for (int i =0; i < 9; i++) {
        cout << bootsSymDecrypt(&this->S[i], this->key);
    }
    cout << endl;
}

LweSample *bit8_carried_adder::_add_to_add()
{
    return this->S;
}

bit8_shift::bit8_shift() {
    this->params = global_params;
    this->key = global_key;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            this->shifted_S[i][j] = new_gate_bootstrapping_ciphertext_array(9, this->params);
        }
    }
    this->temp_input = new_gate_bootstrapping_ciphertext_array(4, this->params);
}

bit8_shift::~bit8_shift() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            delete_gate_bootstrapping_ciphertext_array(9, this->shifted_S[i][j]);
        }
    }

    delete_gate_bootstrapping_ciphertext_array(4, this->temp_input);
}

void bit8_shift::_shift_evaluate(LweSample *A, int32_t *W) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if(W[2*i + j] == 0) {
                for (int n = 0; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0, this->key);
                    this->mode[i][j] = -1;
                }
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 , this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    lweCopy(&this->shifted_S[i][j][n], &A[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 , this->key);
                }
                this->mode[i][j] = 0;
            }
        }
    }

    if (this->mode[0][0] == -1 && this->mode[0][1] == -1) this->mode_2[0] = -1;
    else this->mode_2[0] = 0;

    if (this->mode[1][0] == -1 && this->mode[1][1] == -1) this->mode_2[1] = -1;
    else this->mode_2[1] = 0;
}

void bit8_shift::_shift_evaluate(int32_t *A, int32_t *W) {
    for (int m = 0; m < 4; m++) {
        bootsSymEncrypt(&this->temp_input[m], A[m], this->key);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if(W[2*i + j] == 0) {
                for (int n = 0; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                    this->mode[i][j] = -1;
                }
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    lweCopy(&this->shifted_S[i][j][n], &this->temp_input[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                this->mode[i][j] = 0;
            }
        }
    }

    if (this->mode[0][0] == -1 && this->mode[0][1] == -1) this->mode_2[0] = -1;
    else this->mode_2[0] = 0;

    if (this->mode[1][0] == -1 && this->mode[1][1] == -1) this->mode_2[1] = -1;
    else this->mode_2[1] = 0;
}

void bit8_shift::_Booth_shift_evaluate(LweSample *A, int32_t *W) {
    int lenth = 4;
    int32_t W_Booth[8] = {0};

    for(int i = lenth - 1; i > -1 ; i--) {
        if(W[i] == 0) {
            int count = 0;
            for (int j = i - 1; j > -1; j--) {
                if(W[j] == 1) count++;
                else break;
            }
            if(count >= 3) {
                W_Booth[i] = 1;
                i -= count;
                W_Booth[i] = -1;
            }
        }
        else W_Booth[i] = 1;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if(W_Booth[2*i + j] == 0) {
                for (int n = 0; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                    this->mode[i][j] = -1;
                }
            }
            else if(W_Booth[2*i + j] == 1) {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    lweCopy(&this->shifted_S[i][j][n], &A[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                this->mode[i][j] = 0;
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 1 ,this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    bootsNOT(&this->shifted_S[i][j][n], &A[n - delta], &this->key->cloud);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 1 ,this->key);
                }
                this->mode[i][j] = 1;
            }
        }
    }

    if (this->mode[0][0] == -1 && this->mode[0][1] == -1) this->mode_2[0] = -1;
    else this->mode_2[0] = 0;

    if (this->mode[1][0] == -1 && this->mode[1][1] == -1) this->mode_2[1] = -1;
    else this->mode_2[1] = 0;
}

void bit8_shift::_Booth_shift_evaluate(int32_t *A, int32_t *W) {
    for (int m = 0; m < 4; m++) {
        bootsSymEncrypt(&this->temp_input[m], A[m], this->key);
    }

    int lenth = 4;
    int32_t W_Booth[8] = {0};

    for(int i = lenth - 1; i > -1 ; i--) {
        if(W[i] == 0) {
            int count = 0;
            for (int j = i - 1; j > -1; j--) {
                if(W[j] == 1) count++;
                else break;
            }
            if(count >= 3) {
                W_Booth[i] = 1;
                i -= count;
                W_Booth[i] = -1;
            }
        }
        else W_Booth[i] = 1;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if(W_Booth[2*i + j] == 0) {
                for (int n = 0; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                    this->mode[i][j] = -1;
                }
            }
            else if(W_Booth[2*i + j] == 1) {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    lweCopy(&this->shifted_S[i][j][n], &this->temp_input[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 0 ,this->key);
                }
                this->mode[i][j] = 0;
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 1 ,this->key);
                }
                for (int n = delta; n < 4+delta; n++) {
                    bootsNOT(&this->shifted_S[i][j][n], &this->temp_input[n - delta], &this->key->cloud);
                }
                for (int n = 4+delta; n < 9; n++) {
                    bootsSymEncrypt(&this->shifted_S[i][j][n], 1 ,this->key);
                }
                this->mode[i][j] = 1;
            }
        }
    }

    if (this->mode[0][0] == -1 && this->mode[0][1] == -1) this->mode_2[0] = -1;
    else this->mode_2[0] = 0;

    if (this->mode[1][0] == -1 && this->mode[1][1] == -1) this->mode_2[1] = -1;
    else this->mode_2[1] = 0;
}

void bit8_shift::_export_result(LweSample **result_0, LweSample **result_1) {
    result_0 = this->shifted_S[0];
    result_1 = this->shifted_S[1];
}

void bit8_shift::_show_result() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 2; j++) {
            cout << "shifted_S["<< i <<"]["<< j <<"]" << endl;
            for(int n = 0; n < 9; n++) {
                cout << bootsSymDecrypt(&shifted_S[i][j][n], this->key);
            }
            cout << endl;
        }
    }
}

LweSample *(*bit8_shift::_shift_to_add())[2] {
    return this->shifted_S;
}

int (*bit8_shift::_shift_mode_to_add())[2]
{
    return this->mode;
}

int *bit8_shift::_shift_mode_to_add_2()
{
    return this->mode_2;
}