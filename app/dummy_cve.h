#ifndef DUMMY_CVE_H_
#define DUMMY_CVE_H_

void show_cve_info(void);
void test_dummy_cve(void);


#include <stdint.h>

struct dummy_MQTT_buf_ctx {
	unsigned char *cur;
    unsigned char *end;
};

// test for MQTT length decode 
int dynamic_patch_dummy_cve1(struct dummy_MQTT_buf_ctx *buf, uint32_t *length);


// test for unbounded loop
// CVE_2020_17445_pico_ipv6_process_destopt
int dynamic_patch_dummy_cve2(uint8_t *destopt, uint8_t *f, uint32_t opt_ptr);




#endif
