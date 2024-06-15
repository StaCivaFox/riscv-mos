#ifndef _TRAP_H_
#define _TRAP_H_

#ifndef __ASSEMBLER__

#include <types.h>

struct Trapframe {
	/* Saved main processor registers. */
	u_long regs[32];

	/* Saved special registers. */
	u_long sstatus;
	u_long sepc;
	u_long stval;
	u_long scause;
};

void print_tf(struct Trapframe *tf);
void trapinit();


#endif /* !__ASSEMBLER__ */

#endif