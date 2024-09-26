#include <common/defines.h>

void timer_init(u64 interval_ms);
void timer_init_percpu(void);
extern u64 ticks;
