//
// Created by Shangqi on 29/8/21.
//

#include "DPF.h"

uint8_t get_bit(int x, int length, int b){
    return ((unsigned int)(x) >> (length - b)) & 1;
}

bool test_bit_n(const block &value, int n) {
    block positioned = _mm_slli_epi64(value, 7 - (n & 7));
    return (_mm_movemask_epi8(positioned) & (1 << (n / 8))) != 0;
}

bool test_bit(const block* input, int n) {
    int block_pos = n / 128;
    int test_pos = n % 128;
    return test_bit_n(input[block_pos], test_pos);
}

void PRG(const AES_KEY* key, block input, block* output1, block* output2, uint8_t* bit1, uint8_t* bit2){
    block stash[2];
    stash[0] = input;
    stash[1] = reverse_lsb(input);

    AES_ecb_encrypt_blks(stash, 2, key);

    stash[0] = block_xor(stash[0], input);
    stash[1] = block_xor(stash[1], input);

    *bit1 = block_lsb(stash[0]);
    *bit2 = block_lsb(stash[1]);

    *output1 = stash[0];
    *output2 = stash[1];
}

void dpf_gen(int alpha, int n,
             const AES_KEY* key,
             uint8_t* &k0, uint8_t* &k1) {
    // the key length = n - log (lambda/log |G|) (lambda = 128, |G| = 2)
    int length = n - 7;

    block s[length + 1][2];
    uint8_t t[length + 1][2];
    // correction words
    block sCW[length];
    uint8_t tCW[length][2];
    // randomly select the initialisation blocks of two keys
    random_block(&s[0][0]);
    random_block(&s[0][1]);
    t[0][0] = block_lsb(s[0][0]);
    t[0][1] = t[0][0] ^ 1;
    // initialise the key
    // structure:
    // input_length (n) | s_b^0 | t_b^0 |CWs | CW(v + 1)
    k0 = (uint8_t*) malloc(1 + sizeof(block) + 1 + 18 * length + sizeof(block));
    k1 = (uint8_t*) malloc(1 + sizeof(block) + 1 + 18 * length + sizeof(block));
    // k_b = s_b^0||CW_1||CW_2||...||CW_n
    k0[0] = n;
    memcpy(k0 + 1, &s[0][0], sizeof(block));
    k0[17] = t[0][0];
    k1[0] = n;
    memcpy(k1 + 1, &s[0][1], sizeof(block));
    k1[17] = t[0][1];

    #define LEFT 0
	#define RIGHT 1
    block s0[2], s1[2];
    uint8_t t0[2], t1[2];

    for (int i = 1; i <= length; i++) {
        // use PRG to derive the label of sL, sR, tL, tR from the s at the current level
        PRG(key, s[i - 1][0], &s0[LEFT], &s0[RIGHT], &t0[LEFT], &t0[RIGHT]);
        PRG(key, s[i - 1][1], &s1[LEFT], &s1[RIGHT], &t1[LEFT], &t1[RIGHT]);
        // determine keep and lose paths
        int keep, lose;
        int alpha_bit = get_bit(alpha, n, i);
        if(alpha_bit == 0) {
            keep = LEFT;
            lose = RIGHT;
        } else {
            keep = RIGHT;
            lose = LEFT;
        }
        // compute sCW and tCW
        sCW[i - 1] = block_xor(s0[lose], s1[lose]);
        tCW[i - 1][LEFT] = t0[LEFT] ^ t1[LEFT] ^ alpha_bit ^ 1;
        tCW[i - 1][RIGHT] = t0[RIGHT] ^ t1[RIGHT] ^ alpha_bit;
        // concatenate sCW, tCW (L/R) as the key part (i.e., CW^i)
        memcpy(k0 + 18 * i, sCW + i - 1, sizeof(block));
        k0[18 * i + 16] = tCW[i - 1][LEFT];
        k0[18 * i + 17] = tCW[i - 1][RIGHT];
        // get the next level s0, s1, t0, t1
        // s_b^i=s_b[keep] ^ t_b^(i-1) *sCW[i - 1]
        // t_b^i=t_b[keep] ^ t_b^(i-1) *tCW[i - 1][keep]
        if(t[i - 1][0] == 1) {
            s[i][0] = block_xor(s0[keep], sCW[i - 1]);
            t[i][0] = t0[keep] ^ tCW[i - 1][keep];
        } else {
            s[i][0] = s0[keep];
            t[i][0] = t0[keep];
        }

        if(t[i - 1][1] == 1) {
            s[i][1] = block_xor(s1[keep], sCW[i - 1]);
            t[i][1] = t1[keep] ^ tCW[i - 1][keep];
        } else {
            s[i][1] = s1[keep];
            t[i][1] = t1[keep];
        }
    }
    // generate the final block
    block final_block = zero_block();
    // set the final block to 1 first
    final_block = reverse_lsb(final_block);
    // extra the last 2^7 bits from alpha
    uint8_t shift = alpha & 127;
    // set the block with corresponding bits in alpha
    if(shift & 64){
        final_block = block_left_shift(final_block, 64);
    }
    if(shift & 32){
        final_block = block_left_shift(final_block, 32);
    }
    if(shift & 16){
        final_block = block_left_shift(final_block, 16);
    }
    if(shift & 8){
        final_block = block_left_shift(final_block, 8);
    }
    if(shift & 4){
        final_block = block_left_shift(final_block, 4);
    }
    if(shift & 2){
        final_block = block_left_shift(final_block, 2);
    }
    if(shift & 1){
        final_block = block_left_shift(final_block, 1);
    }

    final_block = block_xor(final_block, s[length][0]);
    final_block = block_xor(final_block, s[length][1]);
    // attach final block on k0
    memcpy(k0 + 18 * (length + 1), &final_block, sizeof(block));
    // copy the identical part from k0 to k1
    memcpy(k1 + 18, k0 + 18, 18 * length + sizeof(block));
}

block* dpf_eval_full(const AES_KEY* key,
                     const uint8_t* k) {
    // get the key size
    uint8_t n = k[0];
    // the key length = n - log (lambda/log |G|) (lambda = 128, |G| = 2)
    int length = n - 7;
    // max item in the tree (2^(n-v))
    int max_item_num = 1 << (n - 7);

    block s[2][max_item_num];
    uint8_t t[2][max_item_num];
    // the current layer of key search (0, 1 will be used alternatively until it repeats length time)
    int cur_layer = 1;
    // correction words
    block sCW[length];
    int tCW[length][2];
    // final block
    block final_block;
    memcpy(&final_block, k + 18 * (length + 1), sizeof(block));
    // set initial s, t
    memcpy(&s[0][0], k + 1, sizeof(block));
    t[0][0] = k[17];
    // parse CWs in the key to sCW, tCW (L/R)
    for(int i = 1; i <= length; i++){
        memcpy(&sCW[i - 1], &k[18 * i], sizeof(block));
        tCW[i - 1][0] = k[18 * i + 16];
        tCW[i - 1][1] = k[18 * i + 17];
    }

    block sL, sR;
    uint8_t tL, tR;
    // try to generate the share
    for(int i = 1; i <= length; i++){
        // derive the label for all children of the current nodes
        int cur_item_num = 1 << (i - 1);
        for(int j = 0; j < cur_item_num; j++){
            // use PRG to derive the label of sL, sR, tL, tR from s at the current level
            PRG(key, s[1 - cur_layer][j], &sL, &sR, &tL, &tR);
            // get the next level labels
            // label^i = label^(i - 1) ^ t[i - 1] * CW^(i - 1)
            if(t[1 - cur_layer][j] == 1) {
                sL = block_xor(sL, sCW[i - 1]);
                sR = block_xor(sR, sCW[i - 1]);
                tL = tL ^ tCW[i - 1][0];
                tR = tR ^ tCW[i - 1][1];
            }
            // find the sub-branch for current level
            s[cur_layer][2 * j] = sL;
            t[cur_layer][2 * j] = tL;
            s[cur_layer][2 * j + 1] = sR;
            t[cur_layer][2 * j + 1] = tR;
        }
        cur_layer = 1 - cur_layer;
    }
    // the size of final output
    int item_number = 1 << length;
    auto *res = (block *) malloc(item_number * sizeof(block));
    // use s to unmask the final block
    for(int j = 0; j < item_number; j ++){
        res[j] = s[1 - cur_layer][j];

        if(t[1 - cur_layer][j] == 1){
            res[j] = block_xor(res[j], final_block);
        }
    }
    return res;
}