#include "cpu/exec/helper.h"

make_helper(ret) {
    cpu.eip = swaddr_read(cpu.esp, 4);
    cpu.esp += 4;
    print_asm("ret: 0x%x", cpu.eip);
    
    cpu.eip -= 1;
    return 1;
}