void printcharc(char ch) {
    if (ch == '\n') {
        printcharc('\r');
    }
    asm volatile (
        "mv a0, %[c];\
         li a6, 0;\
         li a7, 1;\
         ecall"
        : 
        :[c]"r"(ch)
        :"a0", "a6", "a7"
    );
}
