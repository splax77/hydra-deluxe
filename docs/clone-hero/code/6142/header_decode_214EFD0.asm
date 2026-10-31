0214efd0: xor    rcx, rdx
0214efd3: movabs rax, 0x1fffffffff
0214efdd: imul   rdx, rcx, 0x308dbc3
0214efe4: and    rdx, rax
0214efe7: mov    rcx, rdx
0214efea: shr    rcx, 7
0214efee: xor    rcx, rdx
0214eff1: and    ecx, 0x550055
0214eff7: mov    eax, ecx
0214eff9: shl    rax, 7
0214effd: xor    rax, rcx
0214f000: xor    rax, rdx
0214f003: dec    rax
0214f006: ret    
0214f007: int3   
