//
// Created by Shangqi on 12/8/21.
//

#ifndef PHM_DPF_H
#define PHM_DPF_H

#include <cstring>

#include "AES.h"

uint8_t get_bit(int x, int length, int b);

bool test_bit_n(const block &value, int n);

bool test_bit(const block* input, int n);

void PRG(const AES_KEY* key, block input, block* output1, block* output2, uint8_t* bit1, uint8_t* bit2);

void dpf_gen(int alpha, int n,
             const AES_KEY* key,
             uint8_t* &k0, uint8_t* &k1);

block* dpf_eval_full(const AES_KEY* key,
                      const uint8_t* k);

#endif //PHM_DPF_H
