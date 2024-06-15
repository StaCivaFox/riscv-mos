#include <trap.h>
#include <printk.h>
#include <registers.h>

extern void handle_s_int(void);
extern void handle_sys(void);
extern void handle_reserved(void);
extern void handle_ebreak(void);

void kexcentry();

void trapinit() {
    w_sstatus(SSTATUS_SIE);         //enable kernel global interrupt
    w_stvec((u_long)kexcentry);
}

void (*exception_handlers[64])(void) = {
    [0 ... 63] = handle_reserved,
    [3] = handle_ebreak,
    [21] = handle_s_int,
};

void do_reserved(struct Trapframe *tf) {
    printk("do reserved\n");
    print_tf(tf);
    printk("the fault intr: %x\n", (*(u_long *)(tf->sepc)));
    panic("reserve_exception");
}

void do_ebreak(struct Trapframe *tf) {
    printk("do ebreak\n");
    print_tf(tf);
    tf->sepc += 2;
}
