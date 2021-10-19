#include "ebpf_helper.h"

#define VAR_XSTATUS 2
#define VAR_CONNECTION 
/*
https://github.com/aws/amazon-freertos/blob/7d4ead242064e9ad2ddf11001deb6132d3444700/lib/mqtt/aws_mqtt_agent.c#L953
SOCKETS_Connect
save xStatus 
*/

uint64_t save_xstatus(stack_frame *ctx) {
    // int xStatus = GET_REG_VAR(VAR_XSTATUS);
    // int key = GET_REG_VAR(VAR_CONNECTION);
    int key = ctx->r0;
    int xStatus = ctx->r1;
    map_save_val(DEFAULT_MAP, key, xStatus);
    return set_return(FILTER_PASS, 0);
}