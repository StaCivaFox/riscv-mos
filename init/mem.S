#define SIZE_PTR .dword

.section .rodata
.global HEAP_START
HEAP_START: SIZE_PTR end

.global HEAP_SIZE
HEAP_SIZE: SIZE_PTR _heap_size

.global etext
etext: SIZE_PTR etext
