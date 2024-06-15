
.text
.globl env_pop_tf
.align 2
env_pop_tf:
	move    sp, a0
	j       ret_from_exception
