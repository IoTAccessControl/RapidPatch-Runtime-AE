#include "ebpf_helper.h"

typedef struct xNETWORK_BUFFER
{
	uint32_t xBufferListItemPtr; 	/* Used to reference the buffer form the free buffer list or a socket. */
	uint32_t ulIPAddress;			/* Source or destination IP address, depending on usage scenario. */
	uint32_t pucEthernetBufferPtr; 	/* Pointer to the start of the Ethernet frame. */
	uint32_t xDataLength; 			/* Starts by holding the total Ethernet frame length, then the UDP/TCP payload length. */
	uint16_t usPort;				/* Source or destination port, depending on usage scenario. */
	uint16_t usBoundPort;			/* The port to which a transmitting socket is bound. */
} NetworkBufferDescriptor_t;


// prvProcessEthernetPacket
uint64_t ip_packet_filter(stack_frame *frame) {
    NetworkBufferDescriptor_t *pxNetworkBuffer = (NetworkBufferDescriptor_t *) frame->r0;
    
}