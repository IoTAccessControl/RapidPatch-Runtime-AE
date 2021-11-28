#include "ebpf_helper.h"
#include <stdint.h>

int fkt_filter(uint8_t *pkt, int len) {
	if (len < 3) {
		return 0;
	}
	if (pkt[0] != 'T' && pkt[1] != 'C' && pkt[2] != 'P') {
		return 0;
	}
	return 1;
}