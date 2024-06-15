.section .text.boot ,"ax",%progbits

.globl _start
_start:

  #lui t0, %hi(boot_page_table)
  #li t1, 0xffffffff00000000
  #sub t0, t0, t1
  #srli t0, t0, 12
  #li t1, 8 << 60
  #or t0, t0, t1
  #csrw satp, t0
  #sfence.vma
  #lui sp, %hi(0xffffffff80400000)
  #addi sp, sp, %lo(0xffffffff80400000)
  #lui t0, %hi(rv_main)
  #addi t0, t0, %lo(rv_main)
  #jr  t0
  li sp, 0x80400000
  call rv_main
spin:
  j spin
