#include "ebpf_helper.h"

typedef struct {
		unsigned int bptr;
		int len;
} Args;

int filter(Args *args) {
    if (args->len >= 20) {
        return FILTER_DROP;
    }
    //set_err_code(0);
    return FILTER_PASS;
    // return args->len;
}