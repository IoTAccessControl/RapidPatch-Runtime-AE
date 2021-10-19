#ifndef IHP_CONFIG_H_
#define IHP_CONFIG_H_


#ifndef LINUX_TEST
/*
SYS_CORTEX_M4
SYS_RISCV_32
*/
// #define SYS_CORTEX_M4

#else

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8000

#endif // end LINUX_TEST

#endif // end header
