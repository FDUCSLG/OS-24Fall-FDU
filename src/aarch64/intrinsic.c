#include <aarch64/intrinsic.h>

void delay_us(u64 n)
{
	u64 freq = get_clock_frequency();
	u64 end = get_timestamp(), now;
	end += freq / 1000000 * n;

	do {
		now = get_timestamp();
	} while (now <= end);
}

int psci_cpu_on(u64 cpuid, u64 ep)
{
	return psci_fn(PSCI_SYSTEM_CPUON, cpuid, ep, 0);
}
