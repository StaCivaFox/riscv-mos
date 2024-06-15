#include <printk.h>
#include <types.h>
#include <pmap.h>
#include <vm.h>
#include <registers.h>
#include <trap.h>
#include <timer.h>
#include <env.h>
#include <sched.h>

extern u_long HEAP_START;
extern u_long HEAP_SIZE;
extern u_long etext;

void rv_main(void) {
    printk("######  #######  #####  ####### \n");
    printk("#     # #     # #     # #       \n");
    printk("#     # #     # #       #       \n");
    printk("######  #     #  #####  #####   \n");
    printk("#   #   #     #       # #       \n");
    printk("#    #  #     # #     # #       \n");
    printk("#     # #######  #####  ####### \n");
    unsigned long long fdt_addr;
    asm volatile (
        "mv %0, a1"
        :"=r"(fdt_addr)
    );
    printk("kernel sp: %x\n", r_sp());
    printk("etext: %x\n", etext);
    //printk("%x\n", fdt_addr);
    printk("%x\n", HEAP_START);
    printk("%x\n", HEAP_SIZE);
    kinit();
    //kalloc_check();
    kpgtable_init();
    kvm_init();
    printk("turn on paging\n");
    //printk("turn on paging\n");
    kvm_init();
    printk("turn on paging\n");
    trapinit();
    printk("init trap\n");
    /*timerinit();
    printk("init timer\n");*/
    /*asm volatile (
        "ebreak"
    );
    printk("return from trap\n");*/

    env_init();
	ENV_CREATE_PRIORITY(user_bare_loop, 1);
	ENV_CREATE_PRIORITY(user_bare_loop, 2);
    timerinit();
    printk("init timer\n");
    //schedule(0);
   // printk("init trap\n");
    //while (1);
}