#include "ebpf_helper.h"

#define LABEL_NEXT

/*
skip to the next line

https://github.com/ARMmbed/mbedtls/blob/bbd2bfb666c134fe534104e66a7f34f66f555781/library/bignum.c

*/
uint64_t skip_invalid_free(stack_frame *ctx) {
    // int ret_addr = GET_BLOCK_ADDR(LABEL_NEXT);
    int ret_addr = ctx->pc + 2;
    return set_return(FILTER_REDIRECT, ret_addr);
}