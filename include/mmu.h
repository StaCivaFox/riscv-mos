#ifndef _MMU_H_
#define _MMU_H_

#define NASID 256

/*Memory layout*/
#define UART0 0x10000000L
#define UART0_IRQ 10

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

// core local interruptor (CLINT), which contains the timer.
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid))
#define CLINT_MTIME (CLINT + 0xBFF8) // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
#define PLIC 0x0c000000L
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

#define KERNBASE 0x80200000L
#define KERNEND 0x80400000L
#define HEAPSTART KERNEND
#define PHYEND 0x88000000L
#define HEAPSIZE (0x88000000L - KERNEND)     //for physical page alloc

//virtual addresses
//kernel stack mapping
#define KSTACKTOP 0x80400000
//user page table base; a whole page table in virtual space is 2^30 Bytes.
#define UVPT (MAXVA - PAGE_SIZE - 0x40000000)
//envs
#define ENVS(i) (UVPT - ((i) + 1) * PAGE_SIZE)
//user stack
#define USTACKTOP (0x80000000 - PAGE_SIZE)


/*Page Table defines*/
/*Using Sv39*/
#define SATP_SV39 (8L << 60L)

#define MAKE_SATP(Pte) (SATP_SV39 | (((u_long)(Pte)) >> 12))

#define PAGE_SIZE 4096

//virtual address fields
#define VPN_L0_SHIFT 12

#define VPN_SHIFT(level) (VPN_L0_SHIFT + (9 * (level)))
#define VPN_L(level, va) (((u_long)(va)) >> VPN_SHIFT(level) & 0x01ff)
#define VPN_L0(va) ((((uint64_t)(va)) >> VPN_L0_SHIFT) & 0x01FF)
#define VPN_L1(va) ((((uint64_t)(va)) >> VPN_L1_SHIFT) & 0x01FF)
#define VPN_L2(va) ((((uint64_t)(va)) >> VPN_L2_SHIFT) & 0x01FF)
#define VPN(va) ((((uint64_t)(va)) >> VPN_L0_SHIFT))

//physical address fields
#define PPN_L0_SHIFT 12
#define PPN_L1_SHIFT 21
#define PPN_L2_SHIFT 30

#define PPN_L0(pa) ((((uint64_t)(pa)) >> PPN_L0_SHIFT) & 0x01FF)
#define PPN_L1(pa) ((((uint64_t)(pa)) >> PPN_L1_SHIFT) & 0x01FF)
#define PPN_L2(pa) ((((uint64_t)(pa)) >> PPN_L2_SHIFT) & 0x3FFFFFF)
#define PPN(pa) ((((uint64_t)(pa)) >> PPN_L0_SHIFT))

//page table entry fields
#define PTE_PPN_L0_SHIFT 10
#define PTE_PPN_L1_SHIFT 19
#define PTE_PPN_L2_SHIFT 28

#define PTE_PPN_L0(pte) ((((uint64_t)(pte)) >> PPN_L0_SHIFT) & 0x01FF)
#define PTE_PPN_L1(pte) ((((uint64_t)(pte)) >> PPN_L1_SHIFT) & 0x01FF)
#define PTE_PPN_L2(pte) ((((uint64_t)(pte)) >> PPN_L2_SHIFT) & 0x3FFFFFF)

#define PTE_FLAGS(pte) (((uint64_t)(pte)) & 0x3FF)
#define PTE_V (1L << 0) // valid
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)
#define PTE_U (1L << 4) // user can access
#define PTE_G 0x0020
#define PTE_A 0x0040
#define PTE_D 0x0080

#define PA2PTE(pa) ((((u_long)pa) >> 12) << 10)

#define PTE2PA(pte) (((pte) >> 10) << 12)

#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

#ifndef __ASSEMBLER__
#include <types.h>

typedef u_long Pte;


#endif
#endif
