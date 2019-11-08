#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
    cpu.eflags.CF =(op_dest->val < op_src->val);
    DATA_TYPE result = op_dest->val - op_src->val;
    cpu.eflags.ZF = result ? 0 : 1;
    cpu.eflags.SF = result >> (8 * DATA_BYTE - 1);
    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm);
#endif

#include "cpu/exec/template-end.h"