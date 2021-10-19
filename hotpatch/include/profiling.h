#ifndef PROFILING_H_
#define PROFILING_H_
#include <stdint.h>

#define EV0 0
#define EV1 1

// by tick
int get_start_tick();
int get_cur_tick();

// for single step task profiling
int profile_add_event(const char *event);
void profile_start(int eid);
uint32_t profile_end(int eid);
void profile_dump(int eid);
void profile_exit(void);

float cycles2us(int cycles);

// for multiple steps task profiling
void init_multiple_step_profiling();
void add_mulprf_step(const char *step);
void show_mulprf_steps_result();
void clear_mulprf_step_profiling();

#endif
