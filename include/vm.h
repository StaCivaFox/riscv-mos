#ifndef _VM_H_
#define _VM_H_

#include <mmu.h>
#include <types.h>
#include <string.h>
#include <printk.h>
#include <pmap.h>


Pte *kvmcreate();
  
void kpgtable_init();

void kvm_init();

Pte *walk(Pte *pagetable, u_long va, int alloc);

u_long lookup(Pte *pagetable, u_long va, Pte **ppte);

void kvmmap(Pte *pagetable, u_long va, u_long pa, u_long size, int perm);

int mappages(Pte *pagetable, u_long va, u_long size, u_long pa, int perm);

int uvmunmap(Pte *pagetable, u_long va, u_long npages, int do_free);

u_long uvmalloc(Pte *pagetable, u_long old_size, u_long new_size, int perm);

u_long uvmdealloc(Pte *pagetable, u_long old_size, u_long new_size);

Pte *uvmcreate();

void freewalk(Pte *pagetable);

void uvmfree(Pte *pagetable, u_long size);
int kvm_copy_to_user_pagetable(Pte *upgtable);
#endif