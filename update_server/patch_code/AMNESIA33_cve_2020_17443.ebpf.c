#include "ebpf_helper.h"

// struct pico_frame {

//     /* Connector for queues */
//     struct pico_frame *next;

//     /* Start of the whole buffer, total frame length. */
//     unsigned char *buffer;
//     uint32_t buffer_len;

//     /* For outgoing packets: this is the meaningful buffer. */
//     unsigned char *start;
//     uint32_t len;

//     /* Pointer to usage counter */
//     uint32_t *usage_count;

//     /* Pointer to protocol headers */
//     uint8_t *datalink_hdr;

//     uint8_t *net_hdr;
//     uint16_t net_len;
//     uint8_t *transport_hdr;
//     uint16_t transport_len;
//     uint8_t *app_hdr;
//     uint16_t app_len;

//     /* Pointer to the phisical device this packet belongs to.
//      * Should be valid in both routing directions
//      */
//     struct pico_device *dev;

//     pico_time timestamp;

//     /* Failures due to bad datalink addressing. */
//     uint16_t failure_count;

//     /* Protocol over IP */
//     uint8_t proto;

//     /* PICO_FRAME_FLAG_* */
//     uint8_t flags;

//     /* Pointer to payload */
//     unsigned char *payload;
//     uint16_t payload_len;

// #if defined(PICO_SUPPORT_IPV4FRAG) || defined(PICO_SUPPORT_IPV6FRAG)
//     /* Payload fragmentation info */
//     uint16_t frag;
// #endif

// #if defined(PICO_SUPPORT_6LOWPAN)
//     uint32_t hash;
//     union pico_ll_addr src;
//     union pico_ll_addr dst;
// #endif

//     /* Pointer to socket */
//     struct pico_socket *sock;

//     /* Pointer to transport info, used to store remote UDP endpoint (IP + port) */
//     void *info;

//     /*Priority. "best-effort" priority, the default value is 0. Priority can be in between -10 and +10*/
//     int8_t priority;
//     uint8_t transport_flags_saved;

//     /* Callback to notify listener when the buffer has been discarded */
//     void (*notify_free)(uint8_t *);

//     uint8_t send_ttl; /* Special TTL/HOPS value, 0 = auto assign */
//     uint8_t send_tos; /* Type of service */
// };

uint64_t filter(stack_frame *frame) {
    uint8_t *echo = (uint8_t *)(frame->r0);
    uint16_t *transport_len_ptr = (uint16_t *)(echo + 38);
    uint16_t transport_len = (uint16_t)(*transport_len_ptr);
    uint32_t op = 0;
    uint32_t ret_code = 0;

    if (transport_len < 8) {
        // intercept
        op = 1;
        ret_code = -1;
    }
    return set_return(op, ret_code);
}