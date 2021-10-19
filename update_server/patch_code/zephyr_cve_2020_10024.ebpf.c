#include "ebpf_helper.h"

uint64_t filter(stack_frame *frame) {
    // uint32_t status = frame->xpsr;
    // get C flag
    uint32_t C_flag = (frame->xpsr) & 0x20000000;
    uint32_t op = FILTER_PASS;
    uint32_t ret_code = 0;    
    if (C_flag != 0) {
        op = FILTER_REDIRECT;
        // frame->lr = frame->pc + 4;
        ret_code = frame->pc + 4;
    }
    return set_return(op, ret_code);
}