//
// Created by Shangqi on 22/12/21.
//

#include "ComputingEngine.h"

ComputingEngine::ComputingEngine() {
    this->handler = new ComputingEngineHandler();
}

void ComputingEngine::obliv_range_ret(const std::vector<tBlock_pos>& block_list,
                                      const std::vector<tMetaRangeRet_p1> & token_list_1, const std::vector<tMetaRangeRet_p2> & token_list_2,
                                      int32_t range_index, int32_t batch_size)  {
    meta_enc_block retrieved_blocks[batch_size];
    // process the retrieved blocks
    this->handler->obliv_range_ret_1(block_list, token_list_1, retrieved_blocks, range_index, batch_size);
    this->handler->obliv_range_ret_2(token_list_2, retrieved_blocks, range_index, batch_size);
}

void ComputingEngine::obliv_range_evict(const std::vector<tMetaOblivEvict>& evict_list_1, const std::vector<tMetaOblivEvict>& evict_list_2,
                                        int32_t range_index, int32_t batch_size) {
    meta_enc_block stash_list[batch_size];
    meta_enc_block retrieved_blocks_1[batch_size];
    meta_enc_block retrieved_blocks_2[batch_size];
    // perform the eviction
    this->handler->obliv_range_evict(evict_list_1, stash_list, retrieved_blocks_1, 0, range_index, batch_size);
    this->handler->obliv_range_evict(evict_list_2, stash_list, retrieved_blocks_2, 1, range_index, batch_size);
    // upload blocks to the edge
    this->handler->write_to_edge(retrieved_blocks_1, retrieved_blocks_2, range_index, batch_size);
}

void ComputingEngine::obliv_permute_re(const std::vector<tMetaPerRe>& per_re_list_1, const std::vector<tMetaPerRe>& per_re_list_2,
                      int32_t range_index, int32_t stash_size) {
    meta_enc_block permuted_blocks[stash_size];
    for (int i = 0; i < stash_size; i++) {
        permuted_blocks[i].path = 0;
        permuted_blocks[i].bu = 0;
        permuted_blocks[i].offset = i;
    }
    this->handler->obliv_permute_re_1(permuted_blocks, per_re_list_1, range_index, stash_size);
    this->handler->obliv_permute_re_2(permuted_blocks, per_re_list_2, range_index, stash_size);
}

void ComputingEngine::pri_range_retrieve(const std::vector<tMetaPriRangeRet> & retrieve_list_1, const std::vector<tMetaPriRangeRet> & retrieve_list_2,
                                         int32_t range_index, int32_t starting_index, int32_t range_size) {
    meta_enc_block retrieved_blocks_1[range_size];
    meta_enc_block retrieved_blocks_2[range_size];
    this->handler->pri_range_retrieve(retrieved_blocks_1, retrieve_list_1, range_index, starting_index, range_size);
    this->handler->pri_range_retrieve(retrieved_blocks_2, retrieve_list_2, range_index, starting_index, range_size);
    // upload blocks to the stash
    this->handler->write_to_stash(retrieved_blocks_1, retrieved_blocks_2, range_index, range_size);
}


int main() {
    TSimpleServer server(
            make_shared<ComputingEngineProcessor>(make_shared<ComputingEngine>()),
            make_shared<TServerSocket>("localhost", 9091),
            make_shared<TFramedTransportFactory>(),
            make_shared<protocol::TBinaryProtocolFactory>()
    );
    server.serve();
}