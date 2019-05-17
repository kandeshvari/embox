// #include <kernel/printk.h>
#include <kernel/panic.h>

#define METAL_MCAUSE_CAUSE         0x000003FFUL

void trap_handler(void) __attribute__((interrupt, aligned(128)));
void trap_handler() {
	uintptr_t mcause, mepc, mtval;
	printk("\nIT'S A TRAP!\n");

	asm volatile ("csrr %0, mcause" : "=r"(mcause));
	asm volatile ("csrr %0, mepc" : "=r"(mepc));
	asm volatile ("csrr %0, mtval" : "=r"(mtval));

	// int async = mcause < 0;
	// int cause = mcause & (((uintptr_t)-1) >> async);
	panic("    cause=%lu mtval=0x%lx addr=0x%lx\n",
		mcause & METAL_MCAUSE_CAUSE, mtval, mepc);
}
