//
// Created by Shangqi on 12/8/21.
//

#include <iostream>

extern "C" {
    #include "AES.h"
}


using namespace std;

int main() {
    AES_KEY user_key;

    long long keyvalue1 = 12354, keyvalue2 = 54321;
    long long content1 = 0, content2 = 1;

    block key_block = make_block(keyvalue1, keyvalue2);

    AES_set_encrypt_key(key_block, &user_key);

    block content = make_block(content1, content2);

    AES_ecb_encrypt_blks(&content, 1, &user_key);

    AES_set_decrypt_key(key_block, &user_key);

    AES_ecb_decrypt_blks(&content, 1, &user_key);

    cout <<"Recovered block (0-63):" << ((int64_t *) &content)[0] << endl;
    cout <<"Recovered block (64-127):" << ((int64_t *) &content)[1] << endl;
}