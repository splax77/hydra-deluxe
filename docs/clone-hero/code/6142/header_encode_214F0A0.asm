0214f0a0: lea    r8, [rcx + 1]
0214f0a4: mov    rcx, r8
0214f0a7: shr    rcx, 7
0214f0ab: xor    rcx, r8
0214f0ae: and    ecx, 0x550055
0214f0b4: mov    eax, ecx
0214f0b6: shl    rax, 7
0214f0ba: xor    rax, rcx
0214f0bd: movabs rcx, 0x1fffffffff
0214f0c7: xor    rax, r8
0214f0ca: imul   rax, rax, 0x2b86ceb
0214f0d1: xor    rax, rdx
0214f0d4: and    rax, rcx
0214f0d7: ret    
0214f0d8: int3   
