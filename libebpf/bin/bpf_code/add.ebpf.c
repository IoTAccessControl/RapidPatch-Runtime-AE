#include <stdint.h>

struct data {
    int a;
    int b
};

// int func(struct data *d) {
//     return d->a + d->b;
// }

int func(void *p) {
    int a = *(int *)p;
    int b = *(int *)(p + 4);
    return a + b;
}