#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
    cpu.esp -= 4;
    MEM_W(cpu.esp, cpu.eip);
    cpu.eip = op_src->val;
    print_asm_template1();
}

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    cpu.eip += len + 1;
    op_src->val += cpu.eip;
    do_execute();

    cpu.eip -= len + 1;
    return len + 1;
}

#include "cpu/exec/template-end.h"