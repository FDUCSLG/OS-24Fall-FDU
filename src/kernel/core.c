#include <aarch64/intrinsic.h>
#include <test/test.h>

NO_RETURN void idle_entry()
{
    arch_stop_cpu();
}
