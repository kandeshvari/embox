/**
 * @file
 *
 * @date  14.05.2019
 * @author Dmitry Kurbatov
 */

//#include <ipl_impl.h>
#include <hal/ipl.h>
#include <asm/regs.h>
#include <asm/interrupts.h>

void ipl_init(void) {
	enable_interrupts();
	// set_csr_bit(mstatus, MSTATUS_MIE);
	// write_csr(mstatus, read_csr(mstatus) | MSTATUS_MIE );
}

ipl_t ipl_save(void) {
	ipl_t csr;
	csr = read_csr(mstatus);
	write_csr(mstatus, csr & ~(MSTATUS_MIE) );
	return csr;
}

void ipl_restore(ipl_t ipl) {
	write_csr(mstatus, ipl);
}
