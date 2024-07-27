#include "tfhe/tfhe.h"
#include "global_key_generator.h"
#include "multiplier.h"
#include <iostream>

using namespace std;


bit4_multiplier::bit4_multiplier() 
: shift_unit(), level1_add_unit(){
    this->level0_add_unit[0] = new bit8_carried_adder();
    this->level0_add_unit[1] = new bit8_carried_adder();
    this->params = global_params;
    this->key = global_key;

    this->result = new_gate_bootstrapping_ciphertext_array(9, this->params);
}

bit4_multiplier::~bit4_multiplier() {
    delete_gate_bootstrapping_ciphertext_array(9, this->result);
}

void bit4_multiplier::_multiply_evaluate(LweSample *A, int32_t *W) {
    this->shift_unit._shift_evaluate(A, W);
    this->level0_add_unit[0]->_bit8_evaluate(this->shift_unit._shift_to_add()[0][0],this->shift_unit._shift_to_add()[0][1], this->shift_unit._shift_mode_to_add()[0]);
    this->level0_add_unit[1]->_bit8_evaluate(this->shift_unit._shift_to_add()[1][0],this->shift_unit._shift_to_add()[1][1], this->shift_unit._shift_mode_to_add()[1]);
    this->level1_add_unit._bit8_evaluate(this->level0_add_unit[0]->_add_to_add(), this->level0_add_unit[1]->_add_to_add(), this->shift_unit._shift_mode_to_add_2());
    this->level1_add_unit._export_result(this->result);
}

void bit4_multiplier::_multiply_evaluate(int32_t *A, int32_t *W) {
    this->shift_unit._shift_evaluate(A, W);
    this->level0_add_unit[0]->_bit8_evaluate(this->shift_unit._shift_to_add()[0][0],this->shift_unit._shift_to_add()[0][1], this->shift_unit._shift_mode_to_add()[0]);
    this->level0_add_unit[1]->_bit8_evaluate(this->shift_unit._shift_to_add()[1][0],this->shift_unit._shift_to_add()[1][1], this->shift_unit._shift_mode_to_add()[1]);
    this->level1_add_unit._bit8_evaluate(this->level0_add_unit[0]->_add_to_add(), this->level0_add_unit[1]->_add_to_add(), this->shift_unit._shift_mode_to_add_2());
    this->level1_add_unit._export_result(this->result);
}

void bit4_multiplier::_Booth_multiply_evaluate(LweSample *A, int32_t *W)
{
    this->shift_unit._Booth_shift_evaluate(A, W);
    this->level0_add_unit[0]->_bit8_evaluate(this->shift_unit._shift_to_add()[0][0],this->shift_unit._shift_to_add()[0][1], this->shift_unit._shift_mode_to_add()[0]);
    this->level0_add_unit[1]->_bit8_evaluate(this->shift_unit._shift_to_add()[1][0],this->shift_unit._shift_to_add()[1][1], this->shift_unit._shift_mode_to_add()[1]);
    this->level1_add_unit._bit8_evaluate(this->level0_add_unit[0]->_add_to_add(), this->level0_add_unit[1]->_add_to_add(), this->shift_unit._shift_mode_to_add_2());
    this->level1_add_unit._export_result(this->result);
}

void bit4_multiplier::_Booth_multiply_evaluate(int32_t *A, int32_t *W)
{
    this->shift_unit._Booth_shift_evaluate(A, W);
    this->level0_add_unit[0]->_bit8_evaluate(this->shift_unit._shift_to_add()[0][0],this->shift_unit._shift_to_add()[0][1], this->shift_unit._shift_mode_to_add()[0]);
    this->level0_add_unit[1]->_bit8_evaluate(this->shift_unit._shift_to_add()[1][0],this->shift_unit._shift_to_add()[1][1], this->shift_unit._shift_mode_to_add()[1]);
    this->level1_add_unit._bit8_evaluate(this->level0_add_unit[0]->_add_to_add(), this->level0_add_unit[1]->_add_to_add(), this->shift_unit._shift_mode_to_add_2());
    this->level1_add_unit._export_result(this->result);
}

void bit4_multiplier::_export_result(LweSample *result) {
    for(int i= 0; i<9; i++) {
        lweCopy(&result[i], &this->result[i], this->params->in_out_params);
    }
}

void bit4_multiplier::_show_result() {
    cout << "[result]:" << endl;
    for(int i= 0; i<9; i++) {
        cout << bootsSymDecrypt(&this->result[i], this->key);
    }
}

LweSample *bit4_multiplier::_multi_to_conv()
{
    return this->result;
}
