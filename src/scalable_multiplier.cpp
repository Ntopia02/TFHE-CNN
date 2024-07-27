#include "iostream"

#include "scalable_multiplier.h"
#include "tfhe/tfhe.h"
#include "math.h"
#include "global_key_generator.h"

using namespace std;

scalable_add::scalable_add(int scale_factor) {
    this->key = global_key;
    this->params = global_params;

    this->scale_factor = scale_factor;

    this->C = new_gate_bootstrapping_ciphertext_array(scale_factor+1, this->params);
    this->S = new_gate_bootstrapping_ciphertext_array(scale_factor+1, this->params);
}

void scalable_add::_add_evaluation(int32_t *A, int32_t *B, int mode0, int mode1) {
    int32_t message[] = {0, 0};
    int mode[2] = {mode0, mode1};

    /** straight-through B to result. */
    if (mode[0] == -1) {
        for (int i = 0; i < this->scale_factor+1; i++) {
            bootsSymEncrypt(&this->S[i], B[i], this->key);
        }
        return;
    }

    /** straight-through A to result. */
    else if (mode[1] == -1) {
        for (int i = 0; i < this->scale_factor+1; i++) {
            bootsSymEncrypt(&this->S[i], A[i], this->key);
        }
        return;
    }

    else if(mode[0] == 1 || mode[1] == 1) bootsSymEncrypt(this->C, 1, this->key);
    else bootsSymEncrypt(this->C, 0, this->key);

    for (int i = 0; i < this->scale_factor+1; i++) {
        message[0] = A[i];
        message[1] = B[i];
        this->_load_message(message, this->C);
        this->_gate_evaluate(&this->S[i], this->C);
    }
}

void scalable_add::_add_evaluation(LweSample *A, LweSample *B, int mode0, int mode1) {
    /** straight-through B to result. */
    int mode[2] = {mode0, mode1};
    if (mode[0] == -1) {
        for (int i = 0; i < this->scale_factor+1; i++) {
           lweCopy(&this->S[i], &B[i], this->params->in_out_params);
        }
        //cout << "passto B" <<endl;
        return;
    }

    /** straight-through A to result. */
    else if (mode[1] == -1) {
        for (int i = 0; i < this->scale_factor+1; i++) {
           lweCopy(&this->S[i], &A[i], this->params->in_out_params);
        }
        //cout << "passto A" <<endl;
        return;
    }

    if(mode[0] == 1 || mode[1] == 1) bootsSymEncrypt(this->C, 1, this->key);
    else bootsSymEncrypt(this->C, 0, this->key);

    for (int i = 0; i < this->scale_factor+1; i++) {
        this->_load_message(&A[i], &B[i], this->C);
        this->_gate_evaluate(&this->S[i], this->C);
    }
}

LweSample *scalable_add::_add_to_add() {
    return this->S;
}

scalable_shift::scalable_shift(int scale_factor) {
    this->key = global_key;
    this->params = global_params;

    this->scale_factor = scale_factor;
    this->channel = scale_factor / 2;

    this->temp_input = new_gate_bootstrapping_ciphertext_array(channel, this->params);

    this->mode = new int[scale_factor - 2];

    this->result = new LweSample**[channel];
    for (int i = 0; i < channel; i++) {
        this->result[i] = new LweSample*[2];
        for (int m = 0; m < 2; m++) {
            this->result[i][m] = new_gate_bootstrapping_ciphertext_array(scale_factor+1, this->params);
        }
        
    }
    
}

scalable_shift::~scalable_shift() {
    for (int i = 0; i < this->channel; i++) {
        for (int m = 0; m < 2; m++) {
            delete_gate_bootstrapping_ciphertext_array(scale_factor+1, this->result[i][m]);
        }
        delete[] this->result[i];
    }
    //delete[] this->result;

    //delete[] this->mode;
}

void scalable_shift::_shift_evaluate(int32_t *A, int32_t *W) {
    for (int m = 0; m < this->scale_factor / 2; m++) {
        bootsSymEncrypt(&this->temp_input[m], A[m], this->key);
    }

    for (int i = 0; i < this->scale_factor / 4; i++) {
        for (int j = 0; j < 2; j++) {
            if(W[2*i + j] == 0) {
                for (int n = 0; n < this->scale_factor + 1; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                    this->mode[2*i + j] = -1;
                }
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                }
                for (int n = delta; n < this->scale_factor / 2 +delta; n++) {
                    lweCopy(&this->result[i][j][n], &this->temp_input[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < this->scale_factor + 1; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                }
                this->mode[delta] = 0;
            }
            
        }
    }
    int number_array = 0;
    int counter;
    for (int level = int(log2(this->scale_factor) - 1); level > 0; level--) {
        int field = pow(2,level);
        if (field / 2 < 2) {
            break;
        }

        counter = 0;

        for (int loc = number_array; loc < number_array + field; loc+=2) {
            if (this->mode[loc] == -1 && this->mode[loc+1] == -1)
                this->mode[number_array+field+counter] = -1;
            else 
                this->mode[number_array+field+counter] = 0;
            counter++;
        }
        number_array += field;
    }
}

void scalable_shift::_Booth_evaluate(int32_t *A, int32_t *W) {
    for (int m = 0; m < this->scale_factor / 2; m++) {
        bootsSymEncrypt(&this->temp_input[m], A[m], this->key);
    }

    int lenth = this->scale_factor/2;
    int32_t *W_Booth = new int[this->scale_factor];

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

    for (int i = 0; i < this->scale_factor / 4; i++) {
        for (int j = 0; j < 2; j++) {
            if(W[2*i + j] == 0) {
                for (int n = 0; n < 9; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                    this->mode[2*i + j] = -1;
                }
            }
            else {
                uint32_t delta = 2*i + j;
                for (int n = 0; n < delta; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                }
                for (int n = delta; n < this->scale_factor / 2 +delta; n++) {
                    lweCopy(&this->result[i][j][n], &this->temp_input[n - delta], this->params->in_out_params);
                }
                for (int n = 4+delta; n < this->scale_factor + 1; n++) {
                    bootsSymEncrypt(&this->result[i][j][n], 0 ,this->key);
                }
                this->mode[delta] = 0;
            }
        }
    }
    int number_array = 0;
    int counter;
    for (int level = int(log2(this->scale_factor) - 1); level > 0; level--) {
        int field = pow(2,level);
        if (field / 2 < 2) {
            break;
        }

        counter = 0;

        for (int loc = number_array; loc < number_array + field; loc+=2) {
            for (int i = 0; i < 2; i++) {
                if (this->mode[loc+i] == -1 && this->mode[loc+i] == -1)
                    this->mode[number_array+field+counter] = -1;
                else 
                    this->mode[number_array+field+counter] = 0;
                counter++;
            }
        }
        number_array += field;
    }
}

LweSample ***scalable_shift::_shift_to_add() {
    return this->result;
}

int *scalable_shift::_shift_to_add_mode()
{
    return this->mode;
}

void scalable_shift::_show_mode() {
    cout << "[mode:]" << endl;
    for (int i = 0; i < this->scale_factor-2; i++) {
        cout << this->mode[i];
        if ((i+1)%2 == 0) {
            cout << "   ";
        }
    }
    cout << endl;
}

scalable_multiply::scalable_multiply(int scale_factor) :
shift_unit(scale_factor * 2)
{
    this->key = global_key;
    this->params = global_params;

    this->scale_factor = scale_factor;
    this->add_unit = new scalable_add*[scale_factor - 1];
    for (int i = 0; i < scale_factor - 1; i++)
    {
        this->add_unit[i] = new scalable_add(scale_factor * 2);
    }
    
    this->result = new_gate_bootstrapping_ciphertext_array(scale_factor*2 + 1, this->params);
}

scalable_multiply::~scalable_multiply() {

}

void scalable_multiply::_multiply_evaluate(int32_t *A, int32_t *W) {
    this->shift_unit._shift_evaluate(A, W);

    int level = int(log2(this->scale_factor));

    int shift_counter = 0;

    for (int i = 0; i < pow(2, level); i+=2) {
        this->add_unit[shift_counter]->_add_evaluation(this->shift_unit._shift_to_add()[shift_counter][0],
         this->shift_unit._shift_to_add()[shift_counter][1],
         this->shift_unit._shift_to_add_mode()[shift_counter*2],
         this->shift_unit._shift_to_add_mode()[shift_counter*2 + 1]);
        shift_counter++;
    }

    int array = 0;
    int add_counter;
    for (int l = level; l >= 1; l--) {
        int num = pow(2,l-1);
        if (num == 1) {
            break;
        }

        add_counter = 0;
        for (int i = 0; i < num; i+=2) {
            this->add_unit[array+num+add_counter]->_add_evaluation(this->add_unit[array+i]->_add_to_add(),
             this->add_unit[array+i+1]->_add_to_add(),
             this->shift_unit._shift_to_add_mode()[(array+num+add_counter)*2],
             this->shift_unit._shift_to_add_mode()[(array+num+add_counter)*2+1]);
            add_counter++;
        }

        array += num;
    }
}

void scalable_multiply::_show_result() {
    this->shift_unit._show_mode();

    cout << "[Scalable multiply evaluation result:]" <<endl;
    for (int c = 0; c < scale_factor-1; c++) {
        cout << "c:" << c << endl;
        for (int i = 0; i < this->scale_factor*2 + 1; i++) {
            cout << bootsSymDecrypt(&this->add_unit[c]->_add_to_add()[i], this->key);
        }
        cout<<endl;
    }
}
