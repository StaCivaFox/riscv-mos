.altmacro
.set REG_SIZE, 8
.set TRAPFRAME_SIZE, 36 

.macro STORE reg, offset
    sd \reg, \offset*8(sp)
.endm

.macro LOAD reg, offset
    ld \reg, \offset*8(sp)
.endm

.macro SAVE_ALL
    csrrw sp, sscratch, sp
    bnez sp, trap_from_user
trap_from_kernel:
    csrr sp, sscratch
trap_from_user:
    addi sp, sp, -36*REG_SIZE
    STORE x1, 1
    STORE x3, 3
    STORE x4, 4
    STORE x5, 5
    STORE x6, 6
    STORE x7, 7
    STORE x8, 8
    STORE x9, 9
    STORE x10, 10
    STORE x11, 11
    STORE x12, 12
    STORE x13, 13
    STORE x14, 14
    STORE x15, 15
    STORE x16, 16
    STORE x17, 17
    STORE x18, 18
    STORE x19, 19
    STORE x20, 20
    STORE x21, 21
    STORE x22, 22
    STORE x23, 23
    STORE x24, 24
    STORE x25, 25
    STORE x26, 26
    STORE x27, 27
    STORE x28, 28
    STORE x29, 29
    STORE x30, 30
    STORE x31, 31
    csrrw s0, sscratch, x0
    csrr s1, sstatus
    csrr s2, sepc
    csrr s3, stval
    csrr s4, scause
    STORE s0, 2
    STORE s1, 32
    STORE s2, 33
    STORE s3, 34
    STORE s4, 35
.endm

.section .text
.globl kexcentry
.align 2
kexcentry:
    SAVE_ALL
    csrr s0, scause

    #check if is interrupt
    li s2, 1
    slli s2, s2, 63
    and s2, s0, s2          
    srli s2, s2, 63         #if s2 == 1, then is interrupt
    
    andi s0, s0, 0x3f
    la s1, exception_handlers

    #calculate offset
    slli s2, s2, 4
    add s0, s0, s2
    slli s0, s0, 3
    
    add s1, s1, s0
    ld s0, 0(s1)        
    jr s0
    
