
/*
 * LEAF - declare leaf routine
 */
#define LEAF(symbol)                                                                               \
	.globl symbol;                                                                             \
	.align 2;                                                                                  \
	.type symbol, @function;                                                                   \
	

/*
 * NESTED - declare nested routine entry point
 */
#define NESTED(symbol, framesize, rpc)                                                             \
	.globl symbol;                                                                             \
	.align 2;                                                                                  \
	.type symbol, @function;                                                                   \


/*
 * END - mark end of function
 */
#define END(function)                                                                              \
	.end function;                                                                             \
	.size function, .- function

#define EXPORT(symbol)                                                                             \
	.globl symbol;                                                                             \
	symbol:

#define FEXPORT(symbol)                                                                            \
	.globl symbol;                                                                             \
	.type symbol, @function;                                                                   \
	symbol:
