#include <registers.h>
#include <sbi.h>
#include <printk.h>
#include <timer.h>

u_int ticks = 0;
static u_long TIMEBASE = 1000000;

void timerinit() {
    //printk("enter timerinit\n");
    ticks = 0;
    //printk("sie: %x\n", r_sie());
    w_sie(r_sie() | SIE_STIE);
    //printk("sie: %x\n", r_sie());
    timer_set_next_event();
}

void timer_set_next_event() {
    /*printk("enter timer_set\n");
    printk("current time: %x\n", r_time());*/
    sbi_set_timer(r_time() + TIMEBASE);
}

void do_timer() {
    //printk("do timer\n");
    timer_set_next_event();
    ticks += 1;
    if (ticks == 100) {
        ticks = 0;
        printk("--100 ticks!--\n");
    }
}