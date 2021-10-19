#ifndef EBPF_LOCK_H_
#define EBPF_LOCK_H_

#define SPIN_INIT(q) ebpf_lock_init(&(q)->lock);
#define SPIN_LOCK(q) ebpf_lock_lock(&(q)->lock);
#define SPIN_UNLOCK(q) ebpf_lock_unlock(&(q)->lock);
#define SPIN_DESTROY(q) ebpf_lock_destroy(&(q)->lock);

typedef struct ebpf_lock {
	int lock;
} ebpf_lock;

static inline void
ebpf_lock_init(ebpf_lock *lock) {
	lock->lock = 0;
}



#if defined(SYS_CORTEX_M4)
/*
https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html

*/

//inline void enter_critical_section(void) {
//	asm("CPSID i"); // disable all maskable IRQs
//	asm("bx lr");
//}

//inline void leave_critical_section(void) {
//	asm("CPSIE i");
//	asm("bx lr");
//}


static void arch_irq_lock(void) {
	unsigned int tmp;
	
	// __asm__ volatile(
	// 	"mov %1, %2;"
	// 	"mrs %0, BASEPRI;"
	// 	"msr BASEPRI, %1;"
	// 	"isb;"
	// 	: "=r"(key), "=r"(tmp)
	// 	: "i"(0)
	// 	: "memory");
}
#ifndef __ATOMIC_ACQUIRE
#define __ATOMIC_ACQUIRE 1
#define __ATOMIC_RELEASE 3 
#endif

static inline void
ebpf_lock_lock(ebpf_lock *lock) {
	//while (__atomic_test_and_set(lock, __ATOMIC_ACQUIRE)) ;
}

static inline void
ebpf_lock_unlock(ebpf_lock *lock) {
	//__atomic_clear(lock, __ATOMIC_RELEASE);
}

static inline void
ebpf_lock_destroy(ebpf_lock *lock) {
	(void)lock;
}

#endif

#ifdef Win32

static inline void
ebpf_lock_lock(ebpf_lock *lock) {
	while (InterlockedExchange(&lock->lock, 1)) {}
}

static inline void
ebpf_lock_unlock(ebpf_lock *lock) {
	InterlockedExchange(&lock->lock, 0);
	//__sync_lock_release(&lock->lock);
	// __atomic_test_and_set
}

static inline void
ebpf_lock_destroy(ebpf_lock *lock) {
	(void)lock;
}

#endif

#endif // !EBPF_LOCK_H_
