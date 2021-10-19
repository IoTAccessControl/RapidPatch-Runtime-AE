#include "ebpf_helper.h"

#define EINVAL 22

// uint64_t filter(stack_frame *frame) {
//     uint32_t data = frame->r1;
//     uint32_t len = *(uint16_t *) (data + 5);
//     uint32_t max_len = frame->r2;
//     uint32_t op = 0;
//     uint32_t ret_code = 0;
//     if (len > max_len || len + max_len >= 0xffff) {
//         // intercept
//         op = 1;
//         ret_code = -EINVAL;
//     }
//     return set_return(op, ret_code);
// }

// zephyr_cve_2020_10063
uint64_t filter(stack_frame *frame) {
    uint32_t data = frame->r1;
    uint32_t len = *(uint16_t *) (data + 5);
    // uint32_t len = data;
    uint32_t op = 0;
    uint32_t ret_code = 0;
    if (len + 0xfff >= 0xffff) {
        // intercept
        op = 1;
        ret_code = -EINVAL;
    }
    return set_return(op, ret_code);
}