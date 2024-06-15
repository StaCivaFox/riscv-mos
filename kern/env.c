#include <elf.h>
#include <env.h>
#include <mmu.h>
#include <pmap.h>
#include <printk.h>
#include <vm.h>
#include <error.h>
#include <registers.h>

//extern Pte *kernel_pagetable;

struct Env envs[NENV];

struct Env *curenv = NULL;
static struct Env_list env_free_list;

// Invariant: 'env' in 'env_sched_list' iff. 'env->env_status' is 'RUNNABLE'.
struct Env_sched_list env_sched_list; // Runnable list

static Pte *base_pgtable;

static u_int asid_bitmap[NASID / 32] = {0};

//create envs[] and map them to kernel pagetable.
//each env struct holds a page,
//taking up a total of NENV * PAGE_SIZE = 0x400000 size of physical space.
void env_create_envs(Pte *kpagetable) {
    struct Env *e;
    for (e = envs; e < &envs[NENV]; e++) {
        char *pa = kalloc();
        if (pa == 0) {
            panic("env_create_envs: out of memory");
        }
        //map to kernel high virtual address
        u_long va = ENVS((int) (e - envs));
        kvmmap(kpagetable, va, (u_long)pa, PAGE_SIZE, PTE_R | PTE_W);
    }
}

/* Overview:
 *  Allocate an unused ASID.
 *
 * Post-Condition:
 *   return 0 and set '*asid' to the allocated ASID on success.
 *   return -E_NO_FREE_ENV if no ASID is available.
 */
static int asid_alloc(u_int *asid) {
	for (u_int i = 0; i < NASID; ++i) {
		int index = i >> 5;
		int inner = i & 31;
		if ((asid_bitmap[index] & (1 << inner)) == 0) {
			asid_bitmap[index] |= 1 << inner;
			*asid = i;
			return 0;
		}
	}
	return -E_NO_FREE_ENV;
}

/* Overview:
 *  Free an ASID.
 *
 * Pre-Condition:
 *  The ASID is allocated by 'asid_alloc'.
 *
 * Post-Condition:
 *  The ASID is freed and may be allocated again later.
 */
static void asid_free(u_int i) {
	int index = i >> 5;
	int inner = i & 31;
	asid_bitmap[index] &= ~(1 << inner);
}

/* Overview:
 *  This function is to make a unique ID for every env
 *
 * Pre-Condition:
 *  e should be valid
 *
 * Post-Condition:
 *  return e's envid on success
 */
u_int mkenvid(struct Env *e) {
	static u_int i = 0;
	return ((++i) << (1 + LOG2NENV)) | (e - envs);
}

/* Overview:
 *   Convert an existing 'envid' to an 'struct Env *'.
 *   If 'envid' is 0, set '*penv = curenv', otherwise set '*penv = &envs[ENVX(envid)]'.
 *   In addition, if 'checkperm' is non-zero, the requested env must be either 'curenv' or its
 *   immediate child.
 *
 * Pre-Condition:
 *   'penv' points to a valid 'struct Env *'.
 *
 * Post-Condition:
 *   return 0 on success, and set '*penv' to the env.
 *   return -E_BAD_ENV on error (invalid 'envid' or 'checkperm' violated).
 */
int envid2env(u_int envid, struct Env **penv, int checkperm) {
	struct Env *e;

	/* Step 1: Assign value to 'e' using 'envid'. */
	/* Hint:
	 *   If envid is zero, set 'penv' to 'curenv' and return 0.
	 *   You may want to use 'ENVX'.
	 */
	/* Exercise 4.3: Your code here. (1/2) */
	if (envid == 0) {
		*penv = curenv;
		return 0;
	}
	e = &envs[ENVX(envid)];

	if (e->env_status == ENV_FREE || e->env_id != envid) {
		return -E_BAD_ENV;
	}

	/* Step 2: Check when 'checkperm' is non-zero. */
	/* Hints:
	 *   Check whether the calling env has sufficient permissions to manipulate the
	 *   specified env, i.e. 'e' is either 'curenv' or its immediate child.
	 *   If violated, return '-E_BAD_ENV'.
	 */
	/* Exercise 4.3: Your code here. (2/2) */
	if (checkperm) {
		if (!(e == curenv || e->env_parent_id == curenv->env_id)) {
			return -E_BAD_ENV;
		}
	}
	/* Step 3: Assign 'e' to '*penv'. */
	*penv = e;
	return 0;
}

/* Overview:
 *   Mark all environments in 'envs' as free and insert them into the 'env_free_list'.
 *   Insert in reverse order, so that the first call to 'env_alloc' returns 'envs[0]'.
 *
 * Hints:
 *   You may use these macro definitions below: 'LIST_INIT', 'TAILQ_INIT', 'LIST_INSERT_HEAD'
 */
void env_init(void) {
	int i;
	/* Step 1: Initialize 'env_free_list' with 'LIST_INIT' and 'env_sched_list' with
	 * 'TAILQ_INIT'. */
	LIST_INIT(&env_free_list);
	TAILQ_INIT(&env_sched_list);

	/* Step 2: Traverse the elements of 'envs' array, set their status to 'ENV_FREE' and insert
	 * them into the 'env_free_list'. Make sure, after the insertion, the order of envs in the
	 * list should be the same as they are in the 'envs' array. */

	for (i = NENV; i >= 0; i--) {
		LIST_INSERT_HEAD(&env_free_list, &envs[i], env_link);
	}
}

//create user pagetable for Env e.
//since every user process needs access to kernel code and data (through ecall),
//we copy a kernel_pagetable for each user page table, 
//saving the trouble of switching satp every time a trap happens in user space.
static int env_setup_vm(struct Env *e) {
    Pte *pagetable = uvmcreate();
    if (pagetable == 0) {
        return -E_NO_MEM;
    }
    //we are now in kernel, where pa is mapped directly to equivalent va.
    //although devices addresses are not directly mapped, 
    //no need to worry since the physical pages we alloc will never reach there.
    if ((kvm_copy_to_user_pagetable(pagetable) != 0)) {
        panic("env_setup_vm: copy kernel pagetable error\n");
    }
    e->env_pgtable = pagetable;
    //pagetable self-mapping to user virtual space.
    e->env_pgtable[VPN_L(2, UVPT)] = PA2PTE((e->env_pgtable)) | PTE_V | PTE_R | PTE_U;
    return 0;
}

/* Overview:
 *   Allocate and initialize a new env.
 *   On success, the new env is stored at '*new'.
 *
 * Pre-Condition:
 *   If the new env doesn't have parent, 'parent_id' should be zero.
 *   'env_init' has been called before this function.
 *
 * Post-Condition:
 *   return 0 on success, and basic fields of the new Env are set up.
 *   return < 0 on error, if no free env, no free asid, or 'env_setup_vm' failed.
 *
 */
int env_alloc(struct Env **new, u_int parent_id) {
	int r;
	struct Env *e;

	/* Step 1: Get a free Env from 'env_free_list' */
	if (LIST_EMPTY(&env_free_list)) {
		return -E_NO_FREE_ENV;
	}
	e = LIST_FIRST(&env_free_list);

	/* Step 2: Call a 'env_setup_vm' to initialize the user address space for this new Env. */
	try(env_setup_vm(e));

	/* Step 3: Initialize these fields for the new Env with appropriate values:
	 *   'env_user_tlb_mod_entry' (lab4), 'env_runs' (lab6), 'env_id' (lab3), 'env_asid' (lab3),
	 *   'env_parent_id' (lab3)
	 *
	 * Hint:
	 *   Use 'asid_alloc' to allocate a free asid.
	 *   Use 'mkenvid' to allocate a free envid.
	 */
	e->env_runs = 0;	       // for lab6
	e->env_id = mkenvid(e);
	try(asid_alloc(&(e->env_asid)));
	e->env_parent_id = parent_id;

	/* Step 4: Initialize the sp and 'sstatus' in 'e->env_tf'.
	 */
	e->env_tf.sstatus = SSTATUS_SPIE;
	// Reserve space for 'argc' and 'argv'.
	e->env_tf.regs[29] = USTACKTOP - sizeof(int) - sizeof(char **);

	/* Step 5: Remove the new Env from env_free_list. */
	LIST_REMOVE(e, env_link);

	*new = e;
	return 0;
}

/* Overview:
 *   Load a page into the user address space of an env with permission 'perm'.
 *   If 'src' is not NULL, copy the 'len' bytes from 'src' into 'offset' at this page.
 *
 * Pre-Condition:
 *   'offset + len' is not larger than 'PAGE_SIZE'.
 *
 * Hint:
 *   The address of env structure is passed through 'data' from 'elf_load_seg', where this function
 *   works as a callback.
 *
 * Note:
 *   This function involves loading executable code to memory. After the completion of load
 *   procedures, D-cache and I-cache writeback/invalidation MUST be performed to maintain cache
 *   coherence, which MOS has NOT implemented. This may result in unexpected behaviours on real
 *   CPUs! QEMU doesn't simulate caching, allowing the OS to function correctly.
 */
static int load_icode_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
			     size_t len) {
	struct Env *env = (struct Env *)data;
	u_long pa;
	int r;

	/* Step 1: Allocate a page. */
	pa = kalloc();

	/* Step 2: If 'src' is not NULL, copy the 'len' bytes started at 'src' into 'offset' at this
	 * page. */
	if (src != NULL) {
		memcpy((void *)pa + offset, src, len);
	}

	/* Step 3: Insert 'p' into 'env->env_pgdir' at 'va' with 'perm'. */
    return mappages(env->env_pgtable, va, PAGE_SIZE, pa, perm);
}

/* Overview:
 *   Load program segments from 'binary' into user space of the env 'e'.
 *   'binary' points to an ELF executable image of 'size' bytes, which contains both text and data
 *   segments.
 */
static void load_icode(struct Env *e, const void *binary, size_t size) {
	/* Step 1: Use 'elf_from' to parse an ELF header from 'binary'. */
	const Elf32_Ehdr *ehdr = elf_from(binary, size);
	if (!ehdr) {
		panic("load icode: bad elf");
	}

	/* Step 2: Load the segments using 'ELF_FOREACH_PHDR_OFF' and 'elf_load_seg'.
	 * As a loader, we just care about loadable segments, so parse only program headers here.
	 */
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF (ph_off, ehdr) {
		Elf32_Phdr *ph = (Elf32_Phdr *)(binary + ph_off);
		if (ph->p_type == PT_LOAD) {
			// 'elf_load_seg' is defined in lib/elfloader.c
			// 'load_icode_mapper' defines the way in which a page in this segment
			// should be mapped.
			panic_on(elf_load_seg(ph, binary + ph->p_offset, load_icode_mapper, e));
		}
	}

	/* Step 3: Set 'e->env_tf.cp0_epc' to 'ehdr->e_entry'. */
	e->env_tf.sepc = ehdr->e_entry;

}

/* Overview:
 *   Create a new env with specified 'binary' and 'priority'.
 *   This is only used to create early envs from kernel during initialization, before the
 *   first created env is scheduled.
 *
 * Hint:
 *   'binary' is an ELF executable image in memory.
 */
struct Env *env_create(const void *binary, size_t size, int priority) {
	struct Env *e;
	/* Step 1: Use 'env_alloc' to alloc a new env, with 0 as 'parent_id'. */
	env_alloc(&e, 0);
	/* Step 2: Assign the 'priority' to 'e' and mark its 'env_status' as runnable. */
	e->env_pri = priority;
	e->env_status = ENV_RUNNABLE;
	/* Step 3: Use 'load_icode' to load the image from 'binary', and insert 'e' into
	 * 'env_sched_list' using 'TAILQ_INSERT_HEAD'. */
	load_icode(e, binary, size);
	TAILQ_INSERT_HEAD(&env_sched_list, e, env_sched_link);
	return e;
}

/* Overview:
 *  Free env e and all memory it uses.
 */
void env_free(struct Env *e) {
	Pte *pt;
	u_int pdeno, pteno, pa;

	/* Hint: Note the environment's demise.*/
	printk("[%08x] free env %08x\n", curenv ? curenv->env_id : 0, e->env_id);

	/* Hint: Flush all mapped pages in the user portion of the address space */
    uvmunmap(e->env_pgtable, 0x80000000, MAXVA / PAGE_SIZE, 0);
	uvmfree(e->env_pgtable, USTACKTOP);
	/* Hint: free the ASID */
	asid_free(e->env_asid);
	/* Hint: invalidate page directory in TLB */
	sfence_vma();
	/* Hint: return the environment to the free list. */
	e->env_status = ENV_FREE;
	LIST_INSERT_HEAD((&env_free_list), (e), env_link);
	TAILQ_REMOVE(&env_sched_list, (e), env_sched_link);
}

/* Overview:
 *  Free env e, and schedule to run a new env if e is the current env.
 */
void env_destroy(struct Env *e) {
	/* Hint: free e. */
	env_free(e);

	/* Hint: schedule to run a new environment. */
	if (curenv == e) {
		curenv = NULL;
		printk("i am killed ... \n");
		schedule(1);        //TODO
	}
}

// WARNING BEGIN: DO NOT MODIFY FOLLOWING LINES!
#ifdef MOS_PRE_ENV_RUN
#include <generated/pre_env_run.h>
#endif
// WARNING END

extern void env_pop_tf(struct Trapframe *tf) __attribute__((noreturn));

/* Overview:
 *   Switch CPU context to the specified env 'e'.
 *
 * Post-Condition:
 *   Set 'e' as the current running env 'curenv'.
 *
 * Hints:
 *   You may use these functions: 'env_pop_tf'.
 */
void env_run(struct Env *e) {
	assert(e->env_status == ENV_RUNNABLE);
	// WARNING BEGIN: DO NOT MODIFY FOLLOWING LINES!
#ifdef MOS_PRE_ENV_RUN
	MOS_PRE_ENV_RUN_STMT
#endif
	// WARNING END

	/* Step 1:
	 *   If 'curenv' is NULL, this is the first time through.
	 *   If not, we may be switching from a previous env, so save its context into
	 *   'curenv->env_tf' first.
	 */
	if (curenv) {
		curenv->env_tf = *((struct Trapframe *)KSTACKTOP - 1);
	}

	/* Step 2: Change 'curenv' to 'e'. */
	curenv = e;
	curenv->env_runs++; // lab6

	/* Step 3: Change 'cur_pgdir' to 'curenv->env_pgdir', switching to its address space. */
	sfence_vma();
    w_satp(MAKE_SATP(curenv->env_pgtable));
    sfence_vma();

	/* Step 4: Use 'env_pop_tf' to restore the curenv's saved context (registers) and return/go
	 * to user mode.
	 *
	 * Hint:
	 *  - You should use 'curenv->env_asid' here.
	 *  - 'env_pop_tf' is a 'noreturn' function: it restores PC from 'cp0_epc' thus not
	 *    returning to the kernel caller, making 'env_run' a 'noreturn' function as well.
	 */
	
	env_pop_tf(&(curenv->env_tf));

}
