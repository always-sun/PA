#include "cpu/exec.h"

//data-mov.c
make_EHelper(mov);
make_EHelper(push);
make_EHelper(pop);

//prefix.c
make_EHelper(operand_size);

//special.c
make_EHelper(inv);
make_EHelper(nemu_trap);

// logic.c
make_EHelper(xor);

// control.c
make_EHelper(jmp);
make_EHelper(jmp_rm);
make_EHelper(call);
make_EHelper(ret);


// arith.c
make_EHelper(sub);
