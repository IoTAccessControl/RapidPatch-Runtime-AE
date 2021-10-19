#include "ebpf_helper.h"

#define LABEL_CLOSE 

/*

redirect to  while( SOCKETS_Recv(  ...

https://github.com/aws/amazon-freertos/blob/7d4ead242064e9ad2ddf11001deb6132d3444700/lib/mqtt/aws_mqtt_agent.c#L1013
*/
uint64_t filter_close_failed(stack_frame *ctx) {
    int key = ctx->r0;
    int val = map_get_val(DEFAULT_MAP, key);
    if (val == 0) { // pdFailed
        // int ret_addr = GET_BLOCK_ADDR(LABEL_CLOSE);
        int ret_addr = 2000;
        return set_return(FILTER_REDIRECT, ret_addr);
    }
    return set_return(FILTER_PASS, 0);
}