#include "include/profiling.h"
#include "include/utils.h"
#include <string.h>
#include <stdbool.h>

#define PROFILER_EVENTS_MAX 12
static char *EV0_TAG = "EV0";
static char *EV1_TAG = "EV1";
static bool profiler_is_init = false;
static uint32_t ticks_per_us = 1000;

static inline float cycles_to_us(uint32_t cycles) {
	return cycles / (float) ticks_per_us;
}

static struct {
	uint32_t start_tick[PROFILER_EVENTS_MAX];
	uint32_t end_tick[PROFILER_EVENTS_MAX];
	const char* event_name[PROFILER_EVENTS_MAX];
	int event_count;
}	profiler;

static struct {
	uint32_t ticks[PROFILER_EVENTS_MAX];
	const char* event_name[PROFILER_EVENTS_MAX];
	int cid;
} step_profiler;

#ifdef DEV_NRF52840
#define SYSCoreClock 64000000 // 64mhz

#elif defined(DEV_STM32L475)
#define SYSCoreClock 80000000 // 80mhz

#elif defined(DEV_STM32F429)
#define SYSCoreClock 180000000 // 180mhz
 
#else
#define SYSCoreClock 100000000 // 100mhz
#endif // DEV_NRF52840 

#ifdef SYS_CORTEX_M4

#define COREDEBUG_DEMCR  (volatile uint32_t *) (0xE000EDFC)
#define DWT_CTRL (volatile uint32_t *) (0xE0001000) // page e156
#define DWT_CYCCNT (volatile uint32_t *) (0xE0001004) // page e156
#define MSK_DEMCR_TRCENA (1 << 24) // page 464
#define MSK_DWT_CYCCNTENA (1 << 0) // page e159

static inline void dwt_init() {
	ticks_per_us = SYSCoreClock / 1000000;
	*COREDEBUG_DEMCR |= MSK_DEMCR_TRCENA;
	*DWT_CYCCNT = 0;
	*DWT_CTRL |= MSK_DWT_CYCCNTENA;
}

static inline void dwt_deinit() {
	*DWT_CTRL &= ~MSK_DWT_CYCCNTENA;
	*DWT_CYCCNT = 0;
	*COREDEBUG_DEMCR &= ~MSK_DEMCR_TRCENA;
}

static inline uint32_t dwt_get_counter() {
	return *DWT_CYCCNT;
}

#else
static inline void dwt_init() {
}

static inline void dwt_deinit() {
}

static inline uint32_t dwt_get_counter() {
	return 0;
}
#endif

int get_start_tick() {
	dwt_init();
	return dwt_get_counter();
}


int get_cur_tick() {
	return dwt_get_counter();
}

static inline void setup_profiler() {
	profiler.event_count = 0;
	dwt_init();
	profiler.event_name[EV0] = EV0_TAG;
	profiler.event_name[EV1] = EV1_TAG;
}

int profile_add_event(const char *event) {
	if (!profiler_is_init) {
		setup_profiler();
	}
	
	if (profiler.event_count >= PROFILER_EVENTS_MAX) {
		profiler.event_count = 0;
	} 
	int event_count = profiler.event_count;
	profiler.event_name[event_count] = event;
	profiler.start_tick[event_count] = profiler.end_tick[event_count] = 0;
	profiler.event_count++;
	return event_count;
}

void profile_start(int eid) {
	// if (!profiler_is_init) {
	// 	setup_profiler();
	// }
	profiler.start_tick[eid] = dwt_get_counter();
}

uint32_t profile_end(int eid) {
	profiler.end_tick[eid] = dwt_get_counter();
	return profiler.end_tick[eid] - profiler.start_tick[eid];
}

float cycles2us(int cycles) {
	return cycles_to_us(cycles);
}

void profile_dump(int eid) {
	uint32_t ticks = profiler.end_tick[eid] - profiler.start_tick[eid];
	
#if defined(NRF52_NO_OS) || defined(STM32L475_NO_OS)
	/*
	us = (float) ticks_per_us  // <------ will crash in qemu(cortex-m4)
	*/
	float us = cycles_to_us(ticks);
	DEBUG_LOG("Event %d -> cycle: %d time(us): %f\n", eid, ticks, us);
#elif defined(ZEPHYR_OS)
	// zephyr do not support printf float
	int us = cycles_to_us(ticks);
	//us = ticks * 10 / 189;
	DEBUG_LOG("Event %d -> cycle: %d time(us): %d\n", eid, ticks, us);
#else
	DEBUG_LOG("QEMU Event %d -> cycle: %d %d\n", ticks);
#endif
}

void profile_exit(void) {
	profiler.event_count = 0;
	dwt_deinit();
	profiler_is_init = false;
}


void init_multiple_step_profiling() {
	memset(&step_profiler, 0, sizeof(step_profiler));
	dwt_init();
	step_profiler.ticks[0] = dwt_get_counter();
	step_profiler.event_name[0] = "init";
}

void add_mulprf_step(const char *step) {
	step_profiler.cid++;
	if (step_profiler.cid >= PROFILER_EVENTS_MAX) {
		DEBUG_LOG("Can only set %d events!\n", PROFILER_EVENTS_MAX);
		return;
	}
	step_profiler.ticks[step_profiler.cid] = dwt_get_counter();
	step_profiler.event_name[step_profiler.cid] = step;
}

void show_mulprf_steps_result() {
	dwt_deinit();
	for (int i = 0; i <= step_profiler.cid; i++) {
		uint32_t ticks = step_profiler.ticks[i];
		float us = cycles_to_us(ticks);
		DEBUG_LOG("Event %s -> cycle: %u time(us): %f\n", step_profiler.event_name[i], ticks, us);
	}
}

void clear_mulprf_step_profiling() {
	memset(&step_profiler, 0, sizeof(step_profiler));
	dwt_deinit();
}