//
// Created by Shangqi on 22/12/21.
//

#ifndef CDN_CE_COMPUTINGENGINE_H
#define CDN_CE_COMPUTINGENGINE_H

#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>

#include "../gen-cpp/ComputingEngine.h"
#include "ComputingEngineHandler.h"

class ComputingEngine : public ComputingEngineIf {
private:
    ComputingEngineHandler *handler;

public:
    explicit ComputingEngine();
    void obliv_range_ret(const std::vector<tBlock_pos> & block_list,
                         const std::vector<tMetaRangeRet_p1> & token_list_1, const std::vector<tMetaRangeRet_p2> & token_list_2,
                         int32_t range_index, int32_t batch_size) override;
    void obliv_range_evict(const std::vector<tMetaOblivEvict> & evict_list_1, const std::vector<tMetaOblivEvict> & evict_list_2,
                           int32_t range_index, int32_t batch_size) override;
    void obliv_permute_re(const std::vector<tMetaPerRe> & per_re_list_1, const std::vector<tMetaPerRe> & per_re_list_2,
                          int32_t range_index, int32_t stash_size) override;
    void pri_range_retrieve(const std::vector<tMetaPriRangeRet> & retrieve_list_1, const std::vector<tMetaPriRangeRet> & retrieve_list_2,
                            int32_t range_index, int32_t starting_index, int32_t batch_size) override;
};


#endif //CDN_CE_COMPUTINGENGINE_H
