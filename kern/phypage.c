#include <types.h>
#include <pmap.h>
#include <printk.h>
#include <string.h>

//Delicate use of linkedlist.
//A poniter to struct run can be interpreted as "current starting pa".
//kmem.freelist can be interpreted as the "starting pa of the free page", 
//decreasing from max after init.
//r->next can be interpreted as the "starting pa of the next free page to r".
struct run {
    struct run *next;
};

struct {
    struct run *freelist;
} kmem;

void kinit() {
    freerange((void *)KERNEND, (void *)PHYEND);
}

void freerange(void *pa_start, void *pa_end) {
    char *p = (char *)ROUND(pa_start, PAGE_SIZE);
    printk("starting kmem.freelist: %x\n", kmem.freelist);
    for (; p + PAGE_SIZE <= (char *)pa_end; p += PAGE_SIZE) {
        kfree(p);
    }
}

//Allocate one 4096 bytes physical page.
//return the starting address on success.
//return 0 on failure.
void *kalloc() {
    struct run *r;
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
        memset((char *)r, 0, PAGE_SIZE);
    }
    return (void *)r;
}

//free a physical page starting from pa.
void kfree(void *pa) {
    struct run *r;
    if (((u_long)pa % PAGE_SIZE) != 0 || (u_long)pa < KERNEND || (u_long)pa >= PHYEND) {
        panic("kfree");
    }
    memset(pa, 0, PAGE_SIZE);
    r = (struct run *)pa;
    //printk(" before free: r = %x, r->next = %x, kmem.freelist = %x\n", r, r->next, kmem.freelist);
    r->next = kmem.freelist;
    kmem.freelist = r;
    //printk(" after free: r = %x, r->next = %x, kmem.freelist = %x\n", r, r->next, kmem.freelist);
}

void kalloc_check() {
    printk("enter kalloc_check\n");
    //kinit();
    printk("kmem.freelist = %x\n", kmem.freelist);
    void *pa;
    pa = kalloc();
    printk("1: %x\n", pa);
    pa = kalloc();
    printk("2: %x\n", pa);
    pa = kalloc();
    printk("3: %x\n", pa);
    pa = kalloc();
    printk("4: %x\n", pa);
    kfree(pa);
    printk("5: %x\n", pa);
    printk("kmem.freelist = %x\n", kmem.freelist);
    kfree(pa - PAGE_SIZE);
    printk("kmem.freelist = %x\n", kmem.freelist);
    kfree(pa - 2 * PAGE_SIZE);
    printk("kmem.freelist = %x\n", kmem.freelist);
    kfree(pa - 3 * PAGE_SIZE);
    printk("kmem.freelist = %x\n", kmem.freelist);
    kfree(pa - 4 * PAGE_SIZE);
    printk("kmem.freelist = %x\n", kmem.freelist);
}
