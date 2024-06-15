# ENDIAN is either EL (little endian) or EB (big endian)
ENDIAN         := EL

QEMU           := qemu-system-riscv64
CROSS_COMPILE  := riscv64-unknown-elf-
CC             := $(CROSS_COMPILE)gcc
CFLAGS         += --std=gnu99 -fno-pic -ffreestanding \
				  -fno-builtin \
                  -Wall -march=rv64imafdc -mabi=lp64d \
				  -mcmodel=medany
LD             := $(CROSS_COMPILE)ld
LDFLAGS        += -static -n -nostdlib --fatal-warnings

HOST_CC        := cc
HOST_CFLAGS    += --std=gnu99 -O2 -Wall
HOST_ENDIAN    := $(shell lscpu | grep -iq 'little endian' && echo EL || echo EB)

ifneq ($(HOST_ENDIAN), $(ENDIAN))
# CONFIG_REVERSE_ENDIAN is checked in tools/fsformat.c (lab5)
HOST_CFLAGS    += -DCONFIG_REVERSE_ENDIAN
endif
