//
// Created by shangqi on 11/4/22.
//

#ifndef CDN_CE_COMPUTINGENGINEHANDLER_H
#define CDN_CE_COMPUTINGENGINEHANDLER_H

#include <cstring>
#include <vector>
#include <unordered_map>

#include "CommonUtil.h"
#include "data_type.h"
#include "EdgeConnector.h"

using namespace services;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;

using namespace std;

class ComputingEngineHandler {
public:
    ComputingEngineHandler();
    void obliv_range_ret_1(const std::vector<services::tBlock_pos>& request_block_list,
                           const vector<services::tMetaRangeRet_p1>& token_list, meta_enc_block* retrieved_block_list,
                           int32_t range_index, int batch_size);
    void obliv_range_ret_2(const vector<services::tMetaRangeRet_p2>& token_list, meta_enc_block* retrieved_block_list,
                           int32_t range_index, int batch_size);

    void obliv_range_evict(const vector<services::tMetaOblivEvict>& evict_list, meta_enc_block* stash_list, meta_enc_block* retrieved_block_list,
                                             int role, int32_t range_index, int batch_size);
    void write_to_edge(meta_enc_block* retrieved_block_list_1, meta_enc_block* retrieved_block_list_2,
                       int32_t range_index, int batch_size);

    void obliv_permute_re_1(meta_enc_block* stash_list, const vector<services::tMetaPerRe>& permute_re_list,
                            int32_t range_index, int stash_size);
    void obliv_permute_re_2(meta_enc_block* stash_list, const vector<services::tMetaPerRe>& permute_re_list,
                            int32_t range_index, int stash_size);

    void pri_range_retrieve(meta_enc_block* block_list, const vector<services::tMetaPriRangeRet>& range_retrieve_list,
                              int32_t range_index, int32_t starting_index, int32_t range_size);
    void write_to_stash(meta_enc_block* retrieved_block_list_1, meta_enc_block* retrieved_block_list_2,
                       int32_t range_index, int batch_size);

private:
    EdgeConnector *edge_con;
};


#endif //CDN_CE_COMPUTINGENGINEHANDLER_H
