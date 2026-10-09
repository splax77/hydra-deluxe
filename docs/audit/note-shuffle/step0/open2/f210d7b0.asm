0210d7b0: movzx  eax, cl
0210d7b3: sub    eax, 0x3b
0210d7b6: cmp    eax, 0x2a
0210d7b9: ja     0x18210d803
0210d7bb: movzx  eax, cl
0210d7be: lea    rdx, [rip - 0x210d7c5]                   ; [0x0] (bss)
0210d7c5: sub    eax, 0x3b
0210d7c8: movsxd rcx, eax
0210d7cb: movzx  eax, byte ptr [rdx + rcx + 0x210d824]
0210d7d3: mov    ecx, dword ptr [rdx + rax*4 + 0x210d808]
0210d7da: add    rcx, rdx
0210d7dd: jmp    rcx
0210d7df: mov    eax, 0xd
0210d7e4: ret    
0210d7e5: mov    eax, 0xe
0210d7ea: ret    
0210d7eb: mov    eax, 0xf
0210d7f0: ret    
0210d7f1: mov    eax, 0x10
0210d7f6: ret    
0210d7f7: mov    eax, 0x12
0210d7fc: ret    
0210d7fd: mov    eax, 0x11
0210d802: ret    
0210d803: xor    eax, eax
0210d805: ret    
0210d806: nop    
