#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    DATA_TYPE result = op_dest->val & op_src->val;
    cpu.eflags.CF = 0;
    cpu.eflags.ZF = result ? 0 : 1;
    cpu.eflags.SF = result >> (8 * DATA_BYTE - 1);
    print_asm_template2();
}

make_instr_helper(rm2r);

#include "cpu/exec/template-end.h"