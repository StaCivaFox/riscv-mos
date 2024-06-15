OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x80200000;
    .text ALIGN(4096) : {
        *(.text.boot)
        *(.text)
        PROVIDE(etext = .);
    }
    .data : {
        *(.data .data.* .sdata .sdata.*)
        PROVIDE(edata = .);
    }
    
    .bss : {
        *(.bss .bss.* .sbss .sbss.*)
        PROVIDE(ebss = .);
    }
    
    . = 0x80400000;
    PROVIDE(end = .);
    PROVIDE(_heap_size = 0x88000000 - end);
}