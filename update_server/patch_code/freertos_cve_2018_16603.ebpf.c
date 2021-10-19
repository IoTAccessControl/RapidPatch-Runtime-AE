#include "ebpf_helper.h"

#define ipSIZE_OF_ETH_HEADER			14u
#define ipSIZE_OF_IPv4_HEADER			20u
#define ipSIZE_OF_TCP_HEADER			20u

uint64_t filter(stack_frame *frame) {
    uint64_t xDataLength = *(uint64_t *)((uint8_t *)(frame->r0) + 28);
    uint64_t tcpLength = xDataLength - ipSIZE_OF_ETH_HEADER - ipSIZE_OF_IPv4_HEADER;
    uint32_t op = 0;
    uint32_t ret_code = 0;
    if (tcpLength < ipSIZE_OF_TCP_HEADER) {
        op = 1;
        // ret_code = pdFAIL = 0
    }
    return set_return(op, ret_code);
}