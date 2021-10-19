#ifndef RT_APP_H_
#define RT_APP_H_

/*
Use two real-time app to measure the delay of the hotpatching.
*/

int init_rt_task_shrump();

int init_rt_task_servo_motor();

// servo motor PID
int init_rt_task_pid();

#endif
