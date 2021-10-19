// #include "ebpf_helper.h"
// uint64_t filter() {
//     Bpkt = LOC#{file: . line: addr = n * BLOCK_SIZE};
//     uint32_t memory_size = GLOBAL#{memory_size};
//     uint32_t cbw_ptr = GLOBAL#{cbw};
//     uint8_t *cbw_CB = (uint8_t *) (cbw_ptr + 16);
//     uint32_t op = FILTER_PASS;
//     uint32_t ret_code = 0;
//     uint32_t n = (cbw_CB[2] << 24) | (cbw_CB[3] << 16) | (cbw_CB[4] <<  8) | (cbw_CB[5] <<  0);
//     if (n * 16 > memory_size) {
//         op = FILTER_DROP;
//         ret_code = false;
//     }
//     return set_return(op, ret_code);
// }