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
.set noat,
.set noreorder,
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

.set at,
.set reorder,
.endm

.macro RESTORE_ALL
.set noat,
.set noreorder,
    LOAD s2, 33
    LOAD s1, 32
    andi s0, s1, 1<<8
    bnez s0, _to_kernel
_to_user:
    addi s0, sp, 36*REG_SIZE
    csrw sscratch, s0
_to_kernel:
    csrw sepc, s2
    csrw sstatus, s1

    LOAD x31, 31
    LOAD x30, 30
    LOAD x29, 29
    LOAD x28, 28
    LOAD x27, 27
    LOAD x26, 26
    LOAD x25, 25
    LOAD x24, 24
    LOAD x23, 23
    LOAD x22, 22
    LOAD x21, 21
    LOAD x20, 20
    LOAD x19, 19
    LOAD x18, 18
    LOAD x17, 17
    LOAD x16, 16
    LOAD x15, 15
    LOAD x14, 14
    LOAD x13, 13
    LOAD x12, 12
    LOAD x11, 11
    LOAD x10, 10
    LOAD x9, 9
    LOAD x8, 8
    LOAD x7, 7
    LOAD x6, 6
    LOAD x5, 5
    LOAD x4, 4
    LOAD x3, 3
    LOAD x1, 1
    LOAD x2, 2
.set at,
.set reorder,
.endm