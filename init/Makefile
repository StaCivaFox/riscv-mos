INCLUDES    := -I../include/

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

all: start.o init.o mem.o

clean:
	rm -rf *.o *~
