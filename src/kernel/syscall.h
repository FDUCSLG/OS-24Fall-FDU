#include <kernel/proc.h>
#include <kernel/syscallno.h>
#include <kernel/proc.h>

#define NR_SYSCALL 512

extern void *syscall_table[NR_SYSCALL];

void syscall_entry(UserContext *context);

extern char early_init[], rest_init[];
void init_syscall();

#define define_early_init(name)                                           \
    static void init_##name();                                            \
    static __attribute__((section(".init.early"),                         \
                          used)) volatile const void *__initcall_##name = \
            &init_##name;                                                 \
    static void init_##name()

#define define_syscall(name, ...)                \
    static u64 sys_##name(__VA_ARGS__);          \
    define_early_init(__syscall_##name)          \
    {                                            \
        syscall_table[SYS_##name] = &sys_##name; \
    }                                            \
    static u64 sys_##name(__VA_ARGS__)

bool user_readable(const void *start, usize size);
bool user_writeable(const void *start, usize size);
usize user_strlen(const char *str, usize maxlen);