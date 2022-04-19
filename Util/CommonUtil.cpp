//
// Created by Shangqi on 29/8/21.
//

#include "CommonUtil.h"

uint32_t positive_modulo(__int64_t i) {
    return (i % ((__int64_t) UINT32_MAX + 1) + ((__int64_t) UINT32_MAX + 1)) % ((__int64_t) UINT32_MAX + 1);
}