tools_dir   := ../../tools
INCLUDES    := -I../../include

.PRECIOUS: %.b %.b.c

%.x: %.b.c
	riscv64-unknown-elf-gcc  --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany -c -o $@ $<

%.b.c: %.b
	$(tools_dir)/bintoc -f $< -o $@ -p user_bare

%.b: %.o
	riscv64-unknown-elf-ld -o $@ -static -n -nostdlib --fatal-warnings -T ../user.lds $^

%.o: %.c
	riscv64-unknown-elf-gcc  --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany $(INCLUDES) -c $<

%.o: %.S
	riscv64-unknown-elf-gcc  --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany $(INCLUDES) -c $<

.PHONY: all clean

all: loop.x #overflow.x put_a.x put_b.x

clean:
	rm -rf *.o *~ *.x *.b *.b.c
