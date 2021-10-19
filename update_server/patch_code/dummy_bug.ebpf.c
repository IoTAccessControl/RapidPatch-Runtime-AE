#include "ebpf_helper.h"

#define FIXED_PATCH 1

inline uint32_t get_args(int nArgs, int argIdx) {
#if defined(ARM_CORTEX_M)

#endif
    return 0;
}

uint64_t filter(stack_frame *ctx) {
    uint32_t op = FILTER_PASS;
    uint32_t ret = 0;

    return set_return(FILTER_PASS, ret);
}