#include "dummy_cve.h"
#include <stdint.h>
#include <stddef.h>
#include "hotpatch/include/utils.h"
#include "hotpatch/include/patch_point.h"
#include "hotpatch/include/iotpatch.h"
#include "hotpatch/include/patch_service.h"
#include "hotpatch/include/profiling.h"

#ifdef LINUX_TEST
struct cve_info {
	void *func;
	char *msg;
};

static void dummy_buf_len_cve(uint8_t *buf, int len) {
	DEBUG_LOG("test cve dummy_buf_len_cve\n");
}

static struct cve_info cve_list[] = {
	{dummy_buf_len_cve, "+ Dummy Packet Len CVE Demo\n"},
	{NULL, NULL},
};

void show_cve_info() {
	DEBUG_LOG("::CVE List\n");
	for (int i = 0; i < 100; i++) {
		struct cve_info *item = &cve_list[i];
		if (item->func == NULL) {
			break;
		}
		DEBUG_LOG(item->msg);
	}
}
#else
void show_cve_info() {}
#endif

#ifdef EVA_TEST
#include "hotpatch/include/ebpf_test.h"

static void dummy_cve_fun1(uint8_t *pkt, int len) {
	profile_start(1);
	fixed_patch_point_hanlder();
	profile_end(1);
	DEBUG_LOG("fixed_patch_point_hanlder\n");
	profile_dump(1);
	
	profile_start(1);
	int ret = 0;
	// dummy_cv1_patch_point(pkt, len);
	profile_end(1);
	if (ret == FILTER_UNUSED) {
		DEBUG_LOG("Filter_UNUSED\n");
		profile_dump(1);
	}
	
	if (ret == FILTER_DROP) {
		DEBUG_LOG("Ebpf filter block Dummy-CVE1! ret=%d\n", ret);
		profile_dump(1);
		return;
	}

	if (ret == FILTER_PASS) {
		DEBUG_LOG("FILTER_PASS\n");
		profile_dump(1);
	}

	if (len > 20) {
		DEBUG_LOG("Dummy-CVE1 triggered: %d\n", len);
	} 
	for (int i = 0; i < len; i++) {
		pkt[i] = i + '0';
	}
	pkt[len] = 0;
	DEBUG_LOG("Dummy-CVE1 get str: %s pos: %d\n", pkt, len);
}

static void test_buf_len_cve() {
	DEBUG_LOG("buf len cve\n");
	uint8_t buf[20] = {0};
	int len = 12;
	// dummy_cve_fun1(buf, len);
	dummy_cve_fun1(buf, 16);
}

static uint8_t fixed_patch_0[64] = ""
"\x01\x00\x38\x00\x00\x00\x00\x00\x61\x11\x04\x00\x00\x00\x00\x00\x67\x01\x00\x00\x20\x00\x00\x00\xc7"
"\x01\x00\x00\x20\x00\x00\x00\xb7\x00\x00\x00\x01\x00\x00\x00\x65\x01\x01\x00\x13\x00\x00\x00\xb7\x00"
"\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00"
"";

static uint8_t sign_0[16] = "\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73\x73";

static void load_fixed_patch_0(void) {
	init_patch_sys();
	patch_payload paylod;
	int pkt_len = 64;
	paylod.pkt = ebpf_malloc(pkt_len);
	memcpy(paylod.sign, sign_0, sizeof(sign_0));
	memcpy(paylod.pkt, fixed_patch_0, pkt_len);
	patch_desc *patch = (patch_desc*) (paylod.pkt);
	DEBUG_LOG("packet size: %d patch type:%d code_len:%d addr:0x%08x\n", pkt_len, patch->type, patch->code_len, patch->inst_addr);
	notify_new_patch(patch);
}

void test_dummy_cve() {
	profile_add_event("fixed_patch_point\n");
	profile_start(EV0);
	profile_end(EV0);
	profile_dump(EV0);
	// ebpf_patch_setup();
	show_cve_info();
	DEBUG_LOG("test cve in Linux\n");
	//run_ebpf_test();
	load_fixed_patch_0();
	test_buf_len_cve();
}
#else
void test_dummy_cve() {
	DEBUG_LOG("Not IPML!\n");
}
#endif
