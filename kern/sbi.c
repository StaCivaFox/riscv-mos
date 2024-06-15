#include <sbi.h>

struct sbiret sbi_set_timer(u_long stime_value) {
    asm volatile (
        "mv a0, %0;\
        li a6, 0;\
        li a7, 0x54494d45;\
        ecall"
        :
        :"r"(stime_value)
        :"a0", "a6", "a7"
    );
}