//
// Created by shangqi on 11/4/22.
//

#include "ComputingEngineHandler.h"

ComputingEngineHandler::ComputingEngineHandler() {
    this->edge_con = new EdgeConnector();
}

void ComputingEngineHandler::obliv_range_ret_1(const std::vector<services::tBlock_pos>& request_block_list,
                                               const vector<services::tMetaRangeRet_p1>& token_list, meta_enc_block* retrieved_block_list,
                                               int32_t range_index, int batch_size) {
    // read blocks from the edge server (rORAM tree)
    for (int i = 0; i < batch_size; i++) {
        // copy path, bu, offset to the target meta_enc_block
        retrieved_block_list[i].path = request_block_list[i].path;
        retrieved_block_list[i].bu = request_block_list[i].bu;
        retrieved_block_list[i].offset = request_block_list[i].offset;
    }
    edge_con->edgeRetrieveByLocations(range_index, retrieved_block_list, batch_size);
    // use k_1 to unmask block i
    for(int i = 0; i < batch_size; i++) {
        for(int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            retrieved_block_list[i].enc_content[j] ^= token_list[i].k_part1[j];
        }
    }
}

void ComputingEngineHandler::obliv_range_ret_2(const vector<services::tMetaRangeRet_p2>& token_list, meta_enc_block* retrieved_block_list,
                                               int32_t range_index, int batch_size) {
    // use k_2 to unmask block i
    for(int i = 0; i < batch_size; i++) {
        for(int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            retrieved_block_list[i].enc_content[j] ^= token_list[i].k_part2[j];
        }
    }
    // route the batch to the Stash_r
    edge_con->stashUpload(range_index, retrieved_block_list, batch_size);
}

void ComputingEngineHandler::obliv_range_evict(const vector<services::tMetaOblivEvict>& evict_list, meta_enc_block* stash_list, meta_enc_block* retrieved_block_list,
                                               int role, int32_t range_index, int batch_size) {
    // read blocks from the edge server (stash)
    for (int i = 0; i < batch_size; i++) {
        // copy path, bu, offset to the target meta_enc_block
        retrieved_block_list[i].path = evict_list[i].path;
        retrieved_block_list[i].bu = evict_list[i].bu;
        retrieved_block_list[i].offset = evict_list[i].offset;

    }
    // CE_0 should fill the stash list
    if(role == 0) {
        for (int i = 0; i < batch_size; i++) {
            // copy path, bu, offset to the internal meta_enc_block
            stash_list[i].path = evict_list[i].path;
            stash_list[i].bu = evict_list[i].bu;
            stash_list[i].offset = evict_list[i].offset;
        }
        edge_con->stashRetrieveByLocations(range_index, stash_list, batch_size);
    }


    for (int i = 0; i < batch_size; i++) {
        // evaluate the dpf key
        block *dpf_res = dpf_eval_index(evict_list[i].key_arr.data(), evict_list[i].round, (const uint8_t*) evict_list[i].ks.data());
        // reset the retrieve block info
        memset(retrieved_block_list[i].enc_content, 0, VIDEO_BLOCK_SIZE);
        for (int j = 0; j < batch_size; j++) {
            if(test_bit(dpf_res, j)) {
                // if this bit == 1, then xor the share with each value in the current bucket
                for (int k = 0; k < VIDEO_BLOCK_SIZE; k++) {
                    retrieved_block_list[i].enc_content[k] ^= stash_list[j].enc_content[k];
                }
            }
        }
        // xor the k_part
        for (int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            retrieved_block_list[i].enc_content[j] ^= evict_list[i].k_part[j];
        }
    }
}

void ComputingEngineHandler::write_to_edge(meta_enc_block* retrieved_block_list_1, meta_enc_block* retrieved_block_list_2,
                                           int32_t range_index, int batch_size) {
    // xor two blocks together
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            retrieved_block_list_1[i].enc_content[j] ^= retrieved_block_list_2[i].enc_content[j];
        }
    }
    edge_con->edgeUpload(range_index, retrieved_block_list_1, batch_size);
}

void ComputingEngineHandler::obliv_permute_re_1(meta_enc_block* stash_list, const vector<services::tMetaPerRe>& permute_re_list, int32_t range_index, int stash_size) {
    // read blocks from the edge server (stash)
    meta_enc_block retrieved_stash_list[stash_size];
    for (int i = 0; i < stash_size; i++) {
        // fetch the entire stash in order
        retrieved_stash_list[i].path = 0;
        retrieved_stash_list[i].bu = 0;
        retrieved_stash_list[i].offset = i;
    }
    edge_con->stashRetrieveByLocations(range_index, retrieved_stash_list, stash_size);
    // permute and re-encrypt the stash list
    for (int i = 0; i < stash_size; i++) {
        // xor the key part and assign to new position
        for (int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            stash_list[permute_re_list[i].newIndex].enc_content[j] = retrieved_stash_list[permute_re_list[i].originIndex].enc_content[j] ^ permute_re_list[i].k_part[j];
        }
    }
}

void ComputingEngineHandler::obliv_permute_re_2(meta_enc_block* stash_list, const vector<services::tMetaPerRe>& permute_re_list,
                                                int32_t range_index, int stash_size) {
    // save the current stash list
    meta_enc_block retrieved_stash_list[stash_size];
    memcpy(retrieved_stash_list, stash_list, stash_size * sizeof(meta_enc_block));
    // permute and re-encrypt the stash list
    for (int i = 0; i < stash_size; i++) {
        // xor the key part and assign to new position
        for (int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            stash_list[permute_re_list[i].newIndex].enc_content[j] = retrieved_stash_list[permute_re_list[i].originIndex].enc_content[j] ^ permute_re_list[i].k_part[j];
        }
    }
    // write the stash back
    edge_con->stashUpload(range_index, stash_list, stash_size);
}

void ComputingEngineHandler::pri_range_retrieve(meta_enc_block* block_list, const vector<services::tMetaPriRangeRet>& range_retrieve_list,
                                                int32_t range_index, int32_t starting_index, int32_t range_size) {
    for (int i = 0; i < range_size; i++) {
        // assign index to block list
        block_list[i].path = 0;
        block_list[i].bu = 0;
        block_list[i].offset = range_retrieve_list[i].stash_index;
        // read blocks from the edge server (edge)
        meta_enc_block retrieved_block_list[range_retrieve_list[i].bu_max_inclusive];
        // process each path separately
        for (int j = 0; j < range_retrieve_list[i].bu_max_inclusive; j++) {
            // copy path, bu, offset to the target meta_enc_block
            retrieved_block_list[j].path = range_retrieve_list[i].path;
            retrieved_block_list[j].bu = j + 1;
            retrieved_block_list[j].offset = range_retrieve_list[i].offset;
        }
        edge_con->edgeRetrieveByLocations(range_index, retrieved_block_list, range_retrieve_list[i].bu_max_inclusive);

        // evaluate the dpf key
        block *dpf_res = dpf_eval_index(range_retrieve_list[i].key_arr.data(), range_retrieve_list[i].round,
                                        (const uint8_t *) range_retrieve_list[i].ks.data());
        // reset the retrieve block info
        memset(block_list[i].enc_content, 0, VIDEO_BLOCK_SIZE);
        for (int j = 0; j < range_retrieve_list[i].bu_max_inclusive; j++) {
            if (test_bit(dpf_res, starting_index + (retrieved_block_list[j].bu - 1) * ORAM_BUCKET_SIZE +
                                  retrieved_block_list[j].offset)) {
                // if this bit == 1, then xor the share with each value in the current bucket
                for (int k = 0; k < VIDEO_BLOCK_SIZE; k++) {
                    block_list[i].enc_content[k] ^= retrieved_block_list[j].enc_content[k];
                }
            }
        }
    }
}

void ComputingEngineHandler::write_to_stash(meta_enc_block *retrieved_block_list_1,
                                            meta_enc_block *retrieved_block_list_2, int32_t range_index,
                                            int batch_size) {
    // xor two blocks together
    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < VIDEO_BLOCK_SIZE; j++) {
            retrieved_block_list_1[i].enc_content[j] ^= retrieved_block_list_2[i].enc_content[j];
        }
    }
    edge_con->stashUpload(range_index, retrieved_block_list_1, batch_size);
}