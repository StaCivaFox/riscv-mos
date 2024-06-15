#ifndef _SBI_H_
#define _SBI_H_

#include <types.h>

struct sbiret
{
    long error;
    long value;
};

struct sbiret sbi_set_timer(u_long stime_value);
#endif
