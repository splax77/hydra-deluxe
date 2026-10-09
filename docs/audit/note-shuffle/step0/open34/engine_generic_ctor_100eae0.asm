0100eae0: mov    rax, rsp
0100eae3: push   rbx
0100eae4: push   rbp
0100eae5: push   rsi
0100eae6: push   rdi
0100eae7: push   r14
0100eae9: sub    rsp, 0xc0
0100eaf0: movups xmm0, xmmword ptr [rdx]
0100eaf3: mov    rsi, r9
0100eaf6: mov    r14, r8
0100eaf9: movups xmm1, xmmword ptr [rdx + 0x10]
0100eafd: mov    r8, qword ptr [rsp + 0x110]
0100eb05: xor    r9d, r9d
0100eb08: movups xmmword ptr [rsp + 0x20], xmm0
0100eb0d: mov    rdi, rcx
0100eb10: movups xmm0, xmmword ptr [rdx + 0x20]
0100eb14: movups xmmword ptr [rsp + 0x30], xmm1
0100eb19: movups xmm1, xmmword ptr [rdx + 0x30]
0100eb1d: movups xmmword ptr [rsp + 0x40], xmm0
0100eb22: movups xmm0, xmmword ptr [rdx + 0x40]
0100eb26: movups xmmword ptr [rsp + 0x50], xmm1
0100eb2b: movups xmm1, xmmword ptr [rdx + 0x50]
0100eb2f: movups xmmword ptr [rsp + 0x60], xmm0
0100eb34: movups xmm0, xmmword ptr [rdx + 0x60]
0100eb38: movups xmmword ptr [rax - 0x78], xmm1
0100eb3c: movups xmm1, xmmword ptr [rdx + 0x70]
0100eb40: movups xmmword ptr [rax - 0x68], xmm0
0100eb44: movups xmm0, xmmword ptr [rdx + 0x80]
0100eb4b: movups xmmword ptr [rax - 0x58], xmm1
0100eb4f: movups xmm1, xmmword ptr [rdx + 0x90]
0100eb56: lea    rdx, [rsp + 0x20]
0100eb5b: movups xmmword ptr [rax - 0x48], xmm0
0100eb5f: movups xmmword ptr [rax - 0x38], xmm1
0100eb63: call   0x1820f79e0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$.ctor
0100eb68: mov    rbp, qword ptr [rsp + 0x118]
0100eb70: mov    rax, qword ptr [rbp + 0x20]
0100eb74: mov    rcx, qword ptr [rax + 0xc0]
0100eb7b: mov    rax, qword ptr [rcx]
0100eb7e: test   byte ptr [rax + 0x135], 1
0100eb85: jne    0x18100eb8f
0100eb87: mov    rcx, rax
0100eb8a: call   0x182f65750
0100eb8f: mov    rcx, rax
0100eb92: call   0x182f60c00
0100eb97: mov    rcx, qword ptr [rbp + 0x20]
0100eb9b: mov    rbx, rax
0100eb9e: mov    rdx, qword ptr [rcx + 0xc0]
0100eba5: mov    rcx, rax
0100eba8: mov    rdx, qword ptr [rdx + 8]
0100ebac: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
0100ebb1: test   rbx, rbx
0100ebb4: je     0x18100ec53
0100ebba: mov    rcx, qword ptr [rbp + 0x20]
0100ebbe: mov    rdx, qword ptr [rcx + 0xc0]
0100ebc5: mov    rax, qword ptr [rdx + 0x18]
0100ebc9: inc    dword ptr [rbx + 0x1c]
0100ebcc: mov    rdx, qword ptr [rbx + 0x10]
0100ebd0: test   rdx, rdx
0100ebd3: je     0x18100ec53
0100ebd5: movsxd rcx, dword ptr [rbx + 0x18]
0100ebd9: cmp    ecx, dword ptr [rdx + 0x18]
0100ebdc: jb     0x18100ebfa
0100ebde: mov    rax, qword ptr [rax + 0x20]
0100ebe2: mov    rdx, rsi
0100ebe5: mov    rcx, rbx
0100ebe8: mov    r8, qword ptr [rax + 0xc0]
0100ebef: mov    r8, qword ptr [r8 + 0x70]
0100ebf3: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0100ebf8: jmp    0x18100ec1a
0100ebfa: lea    eax, [rcx + 1]
0100ebfd: mov    dword ptr [rbx + 0x18], eax
0100ec00: cmp    ecx, dword ptr [rdx + 0x18]
0100ec03: jae    0x18100ec59
0100ec05: lea    rax, [rdx + 0x20]
0100ec09: mov    qword ptr [rdx + rcx*8 + 0x20], rsi
0100ec0e: lea    rcx, [rax + rcx*8]
0100ec12: mov    rdx, rsi
0100ec15: call   0x182f5fc00
0100ec1a: lea    rcx, [rdi + 0x2b8]
0100ec21: mov    qword ptr [rdi + 0x2b8], rbx
0100ec28: mov    rdx, rbx
0100ec2b: call   0x182f5fc00
0100ec30: lea    rcx, [rdi + 0x2b0]
0100ec37: mov    qword ptr [rdi + 0x2b0], r14
0100ec3e: mov    rdx, r14
0100ec41: add    rsp, 0xc0
0100ec48: pop    r14
0100ec4a: pop    rdi
0100ec4b: pop    rsi
0100ec4c: pop    rbp
0100ec4d: pop    rbx
0100ec4e: jmp    0x182f5fc00
0100ec53: call   0x182f60c50
0100ec58: int3   
0100ec59: call   0x182f60c40
0100ec5e: int3   
0100ec5f: int3   
