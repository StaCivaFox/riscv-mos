#include <asm.h>

.altmacro
.set REG_SIZE, 8
.set TRAPFRAME_SIZE, 36 

.macro STORE reg, offset
    sd \reg, \offset*8(sp)
.endm

.macro LOAD reg, offset
    ld \reg, \offset*8(sp)
.endm

.macro RESTORE_ALL
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
.endm

.text

FEXPORT(ret_from_exception)
	RESTORE_ALL
	sret

.globl handle_reserved
.align 2
handle_reserved:
    mv a0, sp
    addi sp, sp, -16
    jal do_reserved
    addi sp, sp, 16
    j ret_from_exception

.globl handle_ebreak
.align 2
handle_ebreak:
    mv a0, sp
    addi sp, sp, -16
    jal do_ebreak
    addi sp, sp, 16
    j ret_from_exception

.globl handle_s_int
.align 2
handle_s_int:       #to be altered for scheduling
    #mv a0, sp
    #addi sp, sp, -16
    #jal do_timer
    #addi sp, sp, 16
    #j ret_from_exception
    li a0, 0
    j schedule
