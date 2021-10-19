#include "ebpf_helper.h"

#define MARCO_VAR_MEM [REPLACE]
#define MARCO_VAR_N [REPLACE]
#define MARCO_VAR_BLS [REPLACE]
#define MARCO_BLOCK_RET [REPLACE]

uint64_t filter(stack_frame *frame) {
    uint32_t target = frame->r0;
    uint32_t mem_size = MARCO_VAR_MEM;
    uint32_t target = MARCO_VAR_N * MARCO_VAR_BLS;
    uint32_t op = FILTER_PASS;
    uint32_t ret_code = 0;
    if (target >= mem_size) {
        op = FILTER_REDIRECT;
        ret_code = MARCO_BLOCK_RET;
    }
    return set_return(op, ret_code);
}