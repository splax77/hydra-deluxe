0214bf10: mov    qword ptr [rsp + 8], rbx
0214bf15: push   rdi
0214bf16: sub    rsp, 0x40
0214bf1a: movaps xmmword ptr [rsp + 0x30], xmm6
0214bf1f: xor    edx, edx
0214bf21: movaps xmmword ptr [rsp + 0x20], xmm7
0214bf26: movzx  ebx, r9w
0214bf2a: movaps xmm7, xmm2
0214bf2d: mov    dword ptr [rcx + 0x50], 0xffffffff
0214bf34: movaps xmm6, xmm1
0214bf37: mov    rdi, rcx
0214bf3a: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0214bf3f: mov    eax, dword ptr [rsp + 0x70]
0214bf43: xorps  xmm0, xmm0
0214bf46: movsd  qword ptr [rdi + 0x30], xmm7
0214bf4b: subsd  xmm7, xmm6
0214bf4f: mov    dword ptr [rdi + 0x7c], eax
0214bf52: mov    rax, qword ptr [rsp + 0x78]
0214bf57: mov    qword ptr [rdi + 0x40], rax
0214bf5b: mov    rax, qword ptr [rsp + 0x80]
0214bf63: cvtsd2ss xmm0, xmm7
0214bf67: mov    word ptr [rdi + 0x80], bx
0214bf6e: mov    rbx, qword ptr [rsp + 0x50]
0214bf73: mov    qword ptr [rdi + 0x48], rax
0214bf77: mov    qword ptr [rdi + 0x60], rax
0214bf7b: movaps xmm7, xmmword ptr [rsp + 0x20]
0214bf80: movsd  qword ptr [rdi + 0x28], xmm6
0214bf85: movaps xmm6, xmmword ptr [rsp + 0x30]
0214bf8a: movss  dword ptr [rdi + 0x38], xmm0
0214bf8f: add    rsp, 0x40
0214bf93: pop    rdi
0214bf94: ret    
0214bf95: int3   
