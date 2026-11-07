020e49d0: push   rdi
020e49d2: push   r12
020e49d4: push   r14
020e49d6: push   r15
020e49d8: sub    rsp, 0x68
020e49dc: mov    r15, rdx
020e49df: mov    rdi, rcx
020e49e2: mov    rcx, qword ptr [rcx + 0x10]
020e49e6: xor    edx, edx
020e49e8: mov    r14d, 1
020e49ee: call   0x1820cf940
020e49f3: mov    r12d, eax
020e49f6: cmp    r12, r14
020e49f9: jbe    0x1820e4baa
020e49ff: mov    qword ptr [rsp + 0x90], rbx
020e4a07: mov    qword ptr [rsp + 0xa0], rsi
020e4a0f: mov    qword ptr [rsp + 0x60], r13
020e4a14: xor    r13d, r13d
020e4a17: mov    qword ptr [rsp + 0x98], rbp
020e4a1f: nop    
020e4a20: cmp    byte ptr [rip + 0x1e348f4], r13b         ; [0x3f1931b] (bss)
020e4a27: mov    rsi, qword ptr [rdi + 0x10]
020e4a2b: jne    0x1820e4a40
020e4a2d: lea    rcx, [rip + 0x1c244cc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4a34: call   0x182f609b0
020e4a39: mov    byte ptr [rip + 0x1e348db], 1            ; [0x3f1931b] (bss)
020e4a40: xor    ecx, ecx
020e4a42: call   0x1801bd810
020e4a47: mov    rcx, qword ptr [rip + 0x1c244b2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4a4e: mov    rbx, rax
020e4a51: cmp    qword ptr [rcx + 0x38], r13
020e4a55: jne    0x1820e4a5c
020e4a57: call   0x182f657d0
020e4a5c: mov    qword ptr [rsp + 0x38], r13
020e4a61: test   rbx, rbx
020e4a64: je     0x1820e4bb6
020e4a6a: cmp    dword ptr [rbx + 0x18], 8
020e4a6e: jae    0x1820e4a77
020e4a70: xor    ecx, ecx
020e4a72: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4a77: lea    rax, [rbx + 0x20]
020e4a7b: mov    dword ptr [rsp + 0x38], 8
020e4a83: mov    qword ptr [rsp + 0x30], rax
020e4a88: lea    rdx, [rsp + 0x50]
020e4a8d: movaps xmm0, xmmword ptr [rsp + 0x30]
020e4a92: xor    r8d, r8d
020e4a95: mov    rcx, rsi
020e4a98: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4a9e: call   0x1805f18b0
020e4aa3: cmp    dword ptr [rbx + 0x18], r13d
020e4aa7: jbe    0x1820e4bdc
020e4aad: cmp    byte ptr [rip + 0x1e34867], r13b         ; [0x3f1931b] (bss)
020e4ab4: mov    rbp, qword ptr [rbx + 0x20]
020e4ab8: mov    rsi, qword ptr [rdi + 0x10]
020e4abc: jne    0x1820e4ad1
020e4abe: lea    rcx, [rip + 0x1c2443b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4ac5: call   0x182f609b0
020e4aca: mov    byte ptr [rip + 0x1e3484a], 1            ; [0x3f1931b] (bss)
020e4ad1: xor    ecx, ecx
020e4ad3: call   0x1801bd810
020e4ad8: mov    rcx, qword ptr [rip + 0x1c24421]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4adf: mov    rbx, rax
020e4ae2: cmp    qword ptr [rcx + 0x38], r13
020e4ae6: jne    0x1820e4aed
020e4ae8: call   0x182f657d0
020e4aed: mov    qword ptr [rsp + 0x48], r13
020e4af2: test   rbx, rbx
020e4af5: je     0x1820e4bb6
020e4afb: cmp    dword ptr [rbx + 0x18], 8
020e4aff: jae    0x1820e4b08
020e4b01: xor    ecx, ecx
020e4b03: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4b08: lea    rax, [rbx + 0x20]
020e4b0c: mov    dword ptr [rsp + 0x48], 8
020e4b14: mov    qword ptr [rsp + 0x40], rax
020e4b19: lea    rdx, [rsp + 0x50]
020e4b1e: movaps xmm0, xmmword ptr [rsp + 0x40]
020e4b23: xor    r8d, r8d
020e4b26: mov    rcx, rsi
020e4b29: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4b2f: call   0x1805f18b0
020e4b34: cmp    dword ptr [rbx + 0x18], r13d
020e4b38: jbe    0x1820e4bdc
020e4b3e: mov    rcx, qword ptr [rdi + 0x10]
020e4b42: test   rcx, rcx
020e4b45: je     0x1820e4bd6
020e4b4b: mov    rax, qword ptr [rcx]
020e4b4e: mov    rbx, qword ptr [rbx + 0x20]
020e4b52: mov    rdx, qword ptr [rax + 0x390]
020e4b59: call   qword ptr [rax + 0x388]
020e4b5f: cmp    eax, 1
020e4b62: sete   r9b
020e4b66: test   r15, r15
020e4b69: je     0x1820e4bd6
020e4b6b: mov    r8, rbx
020e4b6e: mov    qword ptr [rsp + 0x20], r13
020e4b73: mov    rdx, rbp
020e4b76: mov    rcx, r15
020e4b79: call   0x18215b7c0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʲʲʲʿʴʴʴʷʲʽʳ
020e4b7e: inc    r14d
020e4b81: movsxd rax, r14d
020e4b84: cmp    rax, r12
020e4b87: jl     0x1820e4a20
020e4b8d: mov    rbp, qword ptr [rsp + 0x98]
020e4b95: mov    rsi, qword ptr [rsp + 0xa0]
020e4b9d: mov    rbx, qword ptr [rsp + 0x90]
020e4ba5: mov    r13, qword ptr [rsp + 0x60]
020e4baa: add    rsp, 0x68
020e4bae: pop    r15
020e4bb0: pop    r14
020e4bb2: pop    r12
020e4bb4: pop    rdi
020e4bb5: ret    
020e4bb6: xor    ecx, ecx
020e4bb8: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4bbd: xorps  xmm0, xmm0
020e4bc0: lea    rdx, [rsp + 0x50]
020e4bc5: xor    r8d, r8d
020e4bc8: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4bce: mov    rcx, rsi
020e4bd1: call   0x1805f18b0
020e4bd6: call   0x182f60c50
020e4bdb: int3   
020e4bdc: call   0x182f60c40
020e4be1: int3   
020e4be2: int3   
