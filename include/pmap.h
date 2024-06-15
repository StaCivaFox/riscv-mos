#ifndef _PMAP_H_
#define _PMAP_H_

#include <mmu.h>
#include <printk.h>
#include <types.h>


void kinit();
void freerange(void *pa_start, void *pa_end);
void kfree(void *pa);
void *kalloc();

void kalloc_check();
#endif
