#include "ebpf_helper.h"

void ebpf_main(stack_frame *frame) {
    // HELPER_FUNC_INIT;

    uint32_t func_addr = frame->r1;
    // print_log("")
    // c_call(func_addr, 0, 0, 0, 0);
    print_log("xxxx");
}