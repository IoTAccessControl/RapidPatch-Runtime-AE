#include "ebpf_helper.h"

uint64_t fix_bug(stack_frame *ctx) {
    int val = ctx->r0;
	uint64_t ret_code = 0;
	uint64_t op = FILTER_PASS;
    if (val <= 5000 && val > 2000) {
        ret_code = 0;
		op = FILTER_DROP;
    }
	return set_return(op, ret_code);
}