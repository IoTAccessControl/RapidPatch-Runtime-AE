#ifndef FIXED_PATCH_POINT_DEFINE_H_
#define FIXED_PATCH_POINT_DEFINE_H_


// #ifdef USE_FIXED_PATCH_INSTRUMENT

// #define FUNCTION_PATCH fixed_patch_point_hanlder();

// REPLACE it!!!
#define ERR_CODE -1
#ifndef FIXED_OP_PASS
  #define FIXED_OP_PASS 0x00010000 // set a unusual value
#endif

#define __NAKE __attribute__((naked))
#define __USED  __attribute__((used))
#define __INLINE  __attribute__((always_inline))

// inline int fixed_patch_point_hanlder() __NAKE __USED __INLINE;
extern int fixed_patch_point_hanlder();


/* Our approach can only patch the functions that return void or return an error code.
*/
#define PATCH_FUNCTION_ERR_CODE \
  int ret_code = fixed_patch_point_hanlder(); 	\
  if (ret_code != FIXED_OP_PASS) {				\
	return ret_code;							\
  }

// #define PATCH_FUNCTION_VOID \
// 	int ret_code = fixed_patch_point_hanlder(); 	\
//   	if (ret_code != FIXED_OP_PASS) {				\
// 		return;							\
//   	}

// #define PATCH_FUNCTION_ERR_CODE \
// 	__asm__ volatile("blx fixed_patch_point_hanlder"); 
// 	int ret; \
// 	__asm__ volatile("", )

#define PATCH_FUNCTION_VOID do {} while(0);

// int fixed_patch_point_hanlder(void *sp) {
// 	int loc = *(int *)(sp - 4);
// 	if ((loc & pctx.fbits_filter) == loc) {
// 		return FIXED_OP_PASS;
// 	}
	
// 	return 0;
// }

#endif // FIXED_PATCH_POINT_DEFINE_H_