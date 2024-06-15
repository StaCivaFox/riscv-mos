include ./include.mk

INCLUDES    := -I../include/

%.o: %.c
	riscv64-unknown-elf-gcc  --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany $(INCLUDES) -c -o $@ $<

%.o: %.S
	riscv64-unknown-elf-gcc  --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany $(INCLUDES) -c -o $@ $<

.PHONY: all clean

all: $(targets)

clean:
	rm -rf *~ *.o
