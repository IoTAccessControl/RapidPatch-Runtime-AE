#include "ebpf_helper.h"

#define EINVAL 22

uint64_t filter(stack_frame *frame) {
    uint32_t *gpio_driver_api_ptr_addr = (uint32_t *)(frame->r1) + 1;
    uint32_t *gpio_driver_api_ptr = (uint32_t *)(*gpio_driver_api_ptr_addr);
    uint32_t *gpio_enable_callback_ptr_addr = gpio_driver_api_ptr + 4;
    uint32_t *gpio_enable_callback_ptr = (uint32_t *)(*gpio_driver_api_ptr);
    uint32_t op = 0;
    uint32_t ret_code = 0;  
    if (gpio_enable_callback_ptr == 0) {
        op = 1;
        ret_code = -EINVAL;
    }
    return set_return(op, ret_code);
}