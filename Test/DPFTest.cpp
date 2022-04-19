//
// Created by Shangqi on 16/8/21.
//

#include <iostream>

#include "DPF.h"

using namespace std;

int getsize(const uint8_t *k){
    uint8_t n = k[0];
    // the key length = n - log (lambda/log |G|) (lambda = 128, |G| = 2)
    int length = n - 7;
    return 18 * (length + 1) + 16;
}

void output_bit_to_bit(uint64_t input){
    for(int i = 0; i < 64; i++)
    {
        if( (1ll << i) & input)
            printf("1");
        else
            printf("0");
    }
}

void print_block(block input) {
    auto *val = (uint64_t *) &input;

    output_bit_to_bit(val[0]);
    output_bit_to_bit(val[1]);
}

int main() {
    // generate a key
    AES_KEY user_key;

    long long key_value1 = 12354, key_value2 = 54321;

    block key_block = make_block(key_value1, key_value2);

    AES_set_encrypt_key(key_block, &user_key);

    uint8_t* k0;
    uint8_t* k1;

    dpf_gen(7, 8, &user_key, k0, k1);
    int key_size = getsize(k0);
    cout << "DPF Key Size:" << key_size << endl;
    // full domain evaluation on key
    block* res_list0 = dpf_eval_full(&user_key, k0);
    block* res_list1 = dpf_eval_full(&user_key, k1);

    cout << "Result of Server 0" << endl;
    // compute the number of block to be outputted
    int total_block_num = (1 << k0[0]) / 128;
    auto* res = (block*) malloc(total_block_num * sizeof(block));
    for(int j = 0; j < total_block_num; j++){
        print_block(res_list0[j]);
    }
    cout << endl << endl;
    cout << "Result of Server 1" << endl;
    for(int j = 0; j < total_block_num; j++){
        print_block(res_list1[j]);
    }

    for(int j = 0; j < total_block_num; j++){
        res[j] = block_xor(res_list0[j], res_list1[j]);
    }
    cout << endl << endl;
    cout << test_bit_n(res[0], 1) << endl;
    cout << test_bit_n(res[0], 7) << endl;
}