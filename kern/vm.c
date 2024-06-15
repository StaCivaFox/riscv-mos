#include <string.h>
#include <printk.h>
#include <mmu.h>
#include <pmap.h>
#include <vm.h>
#include <registers.h>
#include <env.h>

Pte *kernel_pagetable;

extern char *etext;     //set by kernel.lds


//create a kernel page table.
Pte *kvmcreate() {
    Pte *kpgtable;
    kpgtable = (Pte *)kalloc();
    memset(kpgtable, 0, PAGE_SIZE);
    //map devices high in kernel address spaces
    kvmmap(kpgtable, (CLINT + 0x88000000), CLINT, 0x10000, PTE_R | PTE_W);
    kvmmap(kpgtable, (UART0 + 0x88000000), UART0, PAGE_SIZE, PTE_R | PTE_W);

    // virtio mmio disk interface
    kvmmap(kpgtable, (VIRTIO0 + 0x88000000), VIRTIO0, PAGE_SIZE, PTE_R | PTE_W);

    // PLIC
    kvmmap(kpgtable, (PLIC + 0x88000000), PLIC, 0x400000, PTE_R | PTE_W);

    //map kernel text executable and read-only
    kvmmap(kpgtable, 0x80000000ULL, 0x80000000ULL, ((u_long)etext - 0x80000000ULL), PTE_R | PTE_X);
    
    //map kernel data and physical memory
    kvmmap(kpgtable, ROUND((u_long)etext, PAGE_SIZE), ROUND((u_long)etext, PAGE_SIZE), (PHYEND - (u_long)etext), PTE_R | PTE_W);
    
    //create and map envs table
    env_create_envs(kpgtable);
    return kpgtable;
}

void kpgtable_init() {
    kernel_pagetable = kvmcreate();
}

void kvm_init() {
    sfence_vma();
    w_satp(MAKE_SATP(kernel_pagetable));
    //printk("satp: %x\n", r_satp());
    sfence_vma();
    //printk("satp: %x\n", r_satp());
}

// Return the address of the PTE in page table pagetable
// that corresponds to virtual address va.  If alloc!=0,
// create any required page-table pages.
Pte *walk(Pte *pagetable, u_long va, int alloc) {
    if (va >= MAXVA) {
        printk("va :%x\n", va);
        printk("MAXVA: %x\n", MAXVA);
        panic("walk\n");
    }
    //Pte *pgtable = pagetable;
    for (int level = 2; level > 0; level--) {
        Pte *pte = &pagetable[VPN_L(level, va)];
        if (*pte & PTE_V) {
            pagetable = (Pte *)PTE2PA(*pte);
        }
        else {
            if (alloc) {
                pagetable = (Pte *)kalloc();
                if (pagetable == 0) {
                    return 0;
                }
                memset(pagetable, 0, PAGE_SIZE);
                *pte = PA2PTE(pagetable) | PTE_V;
            }
            else {
                return 0;
            }
        }
    }
    return &pagetable[VPN_L(0, va)];
}

//Look up the pa that corresponds to va.
//if ppte is set, store its pte.
//return pa on success or 0 otherwise.
//used in user space.
u_long lookup(Pte *pagetable, u_long va, Pte **ppte) {
    Pte *pte = walk(pagetable, va, 0);
    u_long pa;
    if (pte == NULL || (*pte & PTE_V) == 0) {
        return 0;
    }
    if ((*pte & PTE_U) == 0) {
        return 0;
    }
    pa = PTE2PA(*pte);
    if (ppte) {
        *ppte = pte;
    }
    return pa;
}

//add a mapping to kernel page table.
//only used when booting.
void kvmmap(Pte *pagetable, u_long va, u_long pa, u_long size, int perm) {
    int r = mappages(pagetable, va, size, pa, perm);
    if (r != 0) {
        printk("%d\n", r);
        panic("kvmmap");
    }
}

//map size of vm starting from va to pa, 
//creating PTE with permission perm.
//return 0 on success, -1 on walk() failure.
int mappages(Pte *pagetable, u_long va, u_long size, u_long pa, int perm) {
    u_long start, last;
    Pte *pte;
    if (size == 0) {
        panic("mappages: size");
    }
    start = ROUNDDOWN(va, PAGE_SIZE);
    last = ROUNDDOWN((va + size - 1), PAGE_SIZE);   //STARTING va of the last page
    sfence_vma();
    for (;;) {
        if ((pte = walk(pagetable, start, 1)) == 0) {
            return -1;
        }
            //printk("start: %d, %x\n", start, start);
            //printk("last: %d, %x\n", last, last);
            //printk("%d\n", (start - last == 0));
            //w_satp(MAKE_SATP(pagetable));
            /*printk("start: %d, %x\n", start, start);
            printk("last: %d, %x\n", last, last);*/
            /*printk("etext: %x\n", etext);
            
            printk("pte: %x\n", *pte);
            printk("va: %x\n", va);
            printk("pa: %x\n", pa);*/
        if (*pte & PTE_V) {
            //since mappages is also used for creating kernel pagetable,
            //we can't always use uvmunmap.
            //printk("pte: %x\n", *pte);
            //printk("va: %x\n", va);
            panic("mappages: remap; use 'uvmunmap' first");
        }
        *pte = PA2PTE(pa) | perm | PTE_V;
        if (start == last) {
            //printk("should break here\n");
            break;
        }
        start += PAGE_SIZE;
        pa += PAGE_SIZE;
    }
    return 0;
}

//remove npages of mapping starting from va,
//optionally free physical memory.
//the mappings must exsit beforehand.
//return 0 on success.
int uvmunmap(Pte *pagetable, u_long va, u_long npages, int do_free) {
    if ((va % PAGE_SIZE) != 0) {
        panic("uvmunmap: va not aligned");
    }
    for (u_long a = va; a < va + npages * PAGE_SIZE; a += PAGE_SIZE) {
        Pte *pte = walk(pagetable, va, 0);
        if (pte == 0 || (*pte & PTE_V) == 0) {
            panic("uvmunmap: not mapped");
        }
        if (PTE_FLAGS(*pte) == PTE_V) {
            panic("uvmunmap: not leaf");
        }
        if (do_free) {
            kfree((void *)(PTE2PA(*pte)));
        }
        *pte = 0;
        sfence_vma();
    }
    return 0;
}

//allocate PTEs and physical memory for a process
//to grow its space from old_size to new_size.
//return new_size on success or 0 on error.
u_long uvmalloc(Pte *pagetable, u_long old_size, u_long new_size, int perm) {
    void *mem;
    if (new_size < old_size) {
        return old_size;
    }
    old_size = ROUND(old_size, PAGE_SIZE);
    for (u_long a = old_size; a < new_size; a += PAGE_SIZE) {
        mem = kalloc();
        if (mem == 0) {
            uvmdealloc(pagetable, a, old_size);
            return 0;
        }
        memset(mem, 0, PAGE_SIZE);
        if (mappages(pagetable, a, PAGE_SIZE, (u_long)mem, PTE_R|PTE_U|perm) != 0) {
            kfree(mem);
            uvmdealloc(pagetable, a, old_size);
            return 0;
        }
    }
    return new_size;
}

//deallocate PTEs and physical memory for a process
//to bring its space back to new_size.
u_long uvmdealloc(Pte *pagetable, u_long old_size, u_long new_size) {
    if (new_size >= old_size) {
        return old_size;
    }
    if (ROUND(new_size, PAGE_SIZE) < ROUND(old_size, PAGE_SIZE)) {
        int npages = (ROUND(old_size, PAGE_SIZE) - ROUND(new_size, PAGE_SIZE)) / PAGE_SIZE;
        uvmunmap(pagetable, ROUND(new_size, PAGE_SIZE), npages, 1);
    }
    return new_size;
}

//create an empty page table for user process.
Pte *uvmcreate() {
    Pte *pagetable = (Pte *)kalloc();
    if (pagetable == 0) {
        return 0;
    }
    memset(pagetable, 0, PAGE_SIZE);
    return pagetable;
}

//recursively free a page table.
//all the leaf PTEs must already have been unmapped,
//or else we'll never find those corresponding physical pages.
void freewalk(Pte *pagetable) {
    for (int i = 0; i < 512; i++) {
        Pte pte = pagetable[i];
        if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0) {
            freewalk((Pte *)(PTE2PA(pte)));
            pagetable[i] = 0;
        }
        else if (pte & PTE_V) {
            panic("freewalk: leaf still mapped");
        }
    }
    kfree((void *)pagetable);
}

void uvmfree(Pte *pagetable, u_long size) {
    if (size > 0) {
        uvmunmap(pagetable, 0, (ROUND(size, PAGE_SIZE) / PAGE_SIZE), 1);
    }
    freewalk(pagetable);
}

//copy kernel pagetable to a user pagetable.
//doesn't copy physical memory.
//return 0 on success, -1 on failure.
int kvm_copy_to_user_pagetable(Pte *upgtable) {
    Pte *pte;
    u_long pa, i;
    u_int flags;

    //traverse until MAXVA might be ineffective.
    for (i = 0x80000000; i < MAXVA; i += PAGE_SIZE) {
        if ((pte = walk(kernel_pagetable, i, 0)) == 0) {
            continue;
        }
        if ((*pte & PTE_V) == 0) {
            continue;
        }
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);
        if (mappages(upgtable, i, PAGE_SIZE, pa, flags) != 0) {
            //error
            uvmunmap(upgtable, 0x80000000, i / PAGE_SIZE, 0);
            return -1;
        }
    }
    return 0;
}





