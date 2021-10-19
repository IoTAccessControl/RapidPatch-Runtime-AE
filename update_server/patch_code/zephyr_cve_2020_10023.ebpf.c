#include "ebpf_helper.h"

#define FUNC_MEMMOV 0x00

inline int shell_strlen(char *str) {
	char *c = str;
	while (*c != 0) {
		c++;
	}
	return c - str;
}

inline int isspace(char v) {
	return v == ' ';
}

inline void memmove(char *dst, char *src, int len) {
	for (int i = 0; i < len; i++) {
		dst[i] = src[i];
	}
}

//int shell_spaces_trim(char *str);
int shell_spaces_trim(stack_frame *frame)
{
    char *str = frame->r0;
    uint32_t memmove_addr = frame->r1;
	uint16_t len = shell_strlen(str);
	uint16_t shift = 0U;

	if (!str) {
		return -1;
	}

	for (uint16_t i = 0; i < len - 1; i++) {
		if (isspace((int)str[i])) {
			for (uint16_t j = i + 1; j < len; j++) {
				if (isspace((int)str[j])) {
					shift++;
					continue;
				}

				if (shift > 0) {
					/* +1 for EOS */
					//memmove(&str[i + 1], &str[j], len - j + 1);
					//memmove(&str[0xb], &str[0xd], 8);
					// push_int()
					// ffi
                    c_call(memmove_addr, &str[i + 1], &str[j], len - j + 1, 0);
					len -= shift;
					shift = 0U;
				}

				break;
			}
		}
	}
	return len;
}