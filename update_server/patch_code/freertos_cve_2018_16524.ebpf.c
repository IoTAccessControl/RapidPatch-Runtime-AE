#include "ebpf_helper.h"

#define TCP_OPT_END				0u   /* End of TCP options list */
#define TCP_OPT_NOOP			1u   /* "No-operation" TCP option */
#define TCP_OPT_MSS				2u   /* Maximum segment size TCP option */
#define TCP_OPT_WSOPT			3u   /* TCP Window Scale Option (3-byte long) */
#define TCP_OPT_SACK_P			4u   /* Advertize that SACK is permitted */
#define TCP_OPT_SACK_A			5u   /* SACK option with first/last */
#define TCP_OPT_TIMESTAMP		8u   /* Time-stamp option */

#define TCP_OPT_MSS_LEN			4u   /* Length of TCP MSS option. */
#define TCP_OPT_WSOPT_LEN		3u   /* Length of TCP WSOPT option. */

#define TCP_OPT_TIMESTAMP_LEN	10	/* fixed length of the time-stamp option */

static inline uint16_t usChar2u16 (const uint8_t *apChr)
{
	return ( uint16_t )
			( ( ( ( uint32_t )apChr[0] ) << 8) |
			  ( ( ( uint32_t )apChr[1] ) ) );
}

uint64_t filter(stack_frame *frame) {
    uint8_t *pxTCPPacket = (uint8_t *)(*((uint8_t *)(frame->r1) + 24));
    uint8_t *pxTCPHeader = (uint8_t *)(pxTCPPacket + 34);
    uint8_t *pucPtr = (uint8_t *)(pxTCPHeader + 20);
    uint8_t *pucLast = (uint8_t *)(pucPtr + ((((*(pxTCPHeader + 12)) >> 4) - 5) << 2));
    uint32_t op = 0;
    uint32_t ret_code = 0;
	while( pucPtr < pucLast )
	{
		if( pucPtr[ 0 ] == TCP_OPT_END )
		{
			/* End of options. */
			break;
		}
		if( pucPtr[ 0 ] == TCP_OPT_NOOP)
		{
			pucPtr++;
			/* NOP option, inserted to make the length a multiple of 4. */
		}
		else if( ( pucPtr[ 0 ] == TCP_OPT_WSOPT ) && ( pucPtr[ 1 ] == TCP_OPT_WSOPT_LEN ) )
		{
			pucPtr += TCP_OPT_WSOPT_LEN;
		}
		else if( ( pucPtr[ 0 ] == TCP_OPT_MSS ) && ( pucPtr[ 1 ] == TCP_OPT_MSS_LEN ) )
		{
			uint16_t uxNewMSS = usChar2u16( pucPtr + 2 );
            if (uxNewMSS == 0) {
                op = 1;
            }

            break;
            // pucPtr += TCP_OPT_MSS_LEN;
		}
		else
		{
			/* All other options have a length field, so that we easily
			can skip past them. */
			int len = ( int )pucPtr[ 1 ];
			if( len == 0 )
			{
				/* If the length field is zero, the options are malformed
				and we don't process them further. */
				break;
			}
			pucPtr += len;
		}
	}
    
    return set_return(op, ret_code);
}