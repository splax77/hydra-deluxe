02150c00: push   rbp
02150c02: push   rbx
02150c03: push   rsi
02150c04: push   rdi
02150c05: push   r12
02150c07: push   r14
02150c09: push   r15
02150c0b: mov    rbp, rsp
02150c0e: sub    rsp, 0x50
02150c12: cmp    byte ptr [rip + 0x1dc8b47], 0            ; [0x3f19760] (bss)
02150c19: mov    rdi, r9
02150c1c: mov    esi, r8d
02150c1f: mov    r14, rdx
02150c22: mov    r15, rcx
02150c25: jne    0x182150c5e
02150c27: lea    rcx, [rip + 0x1bb8f4a]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02150c2e: call   0x182f609b0
02150c33: lea    rcx, [rip + 0x1ba894e]                   ; [0x3cf9588] str:'SyncTrack'
02150c3a: call   0x182f609b0
02150c3f: lea    rcx, [rip + 0x1b65972]                   ; [0x3cb65b8] str:'Events'
02150c46: call   0x182f609b0
02150c4b: lea    rcx, [rip + 0x1b903de]                   ; [0x3ce1030] str:'Song'
02150c52: call   0x182f609b0
02150c57: mov    byte ptr [rip + 0x1dc8b02], 1            ; [0x3f19760] (bss)
02150c5e: cmp    byte ptr [rip + 0x1dbb4f6], 0            ; [0x3f0c15b] (bss)
02150c65: mov    rbx, qword ptr [rip + 0x1b903c4]         ; [0x3ce1030] str:'Song'
02150c6c: jne    0x182150c81
02150c6e: lea    rcx, [rip + 0x1b50933]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02150c75: call   0x182f609b0
02150c7a: mov    byte ptr [rip + 0x1dbb4da], 1            ; [0x3f0c15b] (bss)
02150c81: xor    r12d, r12d
02150c84: test   rbx, rbx
02150c87: je     0x182150ca7
02150c89: xor    edx, edx
02150c8b: mov    dword ptr [rbp - 0x14], r12d
02150c8f: mov    rcx, rbx
02150c92: call   0x1819829d0                              ; System.String$$GetRawStringData
02150c97: mov    qword ptr [rbp - 0x20], rax
02150c9b: mov    eax, dword ptr [rbx + 0x10]
02150c9e: mov    dword ptr [rbp - 0x18], eax
02150ca1: movaps xmm0, xmmword ptr [rbp - 0x20]
02150ca5: jmp    0x182150caa
02150ca7: xorps  xmm0, xmm0
02150caa: mov    r8, qword ptr [rip + 0x1bb8ec7]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02150cb1: lea    rdx, [rbp - 0x20]
02150cb5: movdqa xmmword ptr [rbp - 0x20], xmm0
02150cba: lea    rcx, [rbp - 0x10]
02150cbe: movups xmm0, xmmword ptr [r15]
02150cc2: movaps xmmword ptr [rbp - 0x10], xmm0
02150cc6: call   0x182c61850
02150ccb: test   al, al
02150ccd: jne    0x182150e37
02150cd3: cmp    byte ptr [rip + 0x1dbb481], r12b         ; [0x3f0c15b] (bss)
02150cda: mov    rbx, qword ptr [rip + 0x1ba88a7]         ; [0x3cf9588] str:'SyncTrack'
02150ce1: jne    0x182150cf6
02150ce3: lea    rcx, [rip + 0x1b508be]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02150cea: call   0x182f609b0
02150cef: mov    byte ptr [rip + 0x1dbb465], 1            ; [0x3f0c15b] (bss)
02150cf6: test   rbx, rbx
02150cf9: je     0x182150d19
02150cfb: xor    edx, edx
02150cfd: mov    dword ptr [rbp - 0x14], r12d
02150d01: mov    rcx, rbx
02150d04: call   0x1819829d0                              ; System.String$$GetRawStringData
02150d09: mov    qword ptr [rbp - 0x20], rax
02150d0d: mov    eax, dword ptr [rbx + 0x10]
02150d10: mov    dword ptr [rbp - 0x18], eax
02150d13: movaps xmm0, xmmword ptr [rbp - 0x20]
02150d17: jmp    0x182150d1c
02150d19: xorps  xmm0, xmm0
02150d1c: mov    r8, qword ptr [rip + 0x1bb8e55]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02150d23: lea    rdx, [rbp - 0x10]
02150d27: movdqa xmmword ptr [rbp - 0x10], xmm0
02150d2c: lea    rcx, [rbp - 0x20]
02150d30: movups xmm0, xmmword ptr [r15]
02150d34: movaps xmmword ptr [rbp - 0x20], xmm0
02150d38: call   0x182c61850
02150d3d: test   al, al
02150d3f: jne    0x182150e0f
02150d45: cmp    byte ptr [rip + 0x1dbb40f], r12b         ; [0x3f0c15b] (bss)
02150d4c: mov    rbx, qword ptr [rip + 0x1b65865]         ; [0x3cb65b8] str:'Events'
02150d53: jne    0x182150d68
02150d55: lea    rcx, [rip + 0x1b5084c]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02150d5c: call   0x182f609b0
02150d61: mov    byte ptr [rip + 0x1dbb3f3], 1            ; [0x3f0c15b] (bss)
02150d68: test   rbx, rbx
02150d6b: je     0x182150d8b
02150d6d: xor    edx, edx
02150d6f: mov    dword ptr [rbp - 0x14], r12d
02150d73: mov    rcx, rbx
02150d76: call   0x1819829d0                              ; System.String$$GetRawStringData
02150d7b: mov    qword ptr [rbp - 0x20], rax
02150d7f: mov    eax, dword ptr [rbx + 0x10]
02150d82: mov    dword ptr [rbp - 0x18], eax
02150d85: movaps xmm0, xmmword ptr [rbp - 0x20]
02150d89: jmp    0x182150d8e
02150d8b: xorps  xmm0, xmm0
02150d8e: mov    r8, qword ptr [rip + 0x1bb8de3]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02150d95: lea    rdx, [rbp - 0x10]
02150d99: movdqa xmmword ptr [rbp - 0x10], xmm0
02150d9e: lea    rcx, [rbp - 0x20]
02150da2: movups xmm0, xmmword ptr [r15]
02150da6: movaps xmmword ptr [rbp - 0x20], xmm0
02150daa: call   0x182c61850
02150daf: mov    edx, esi
02150db1: test   al, al
02150db3: jne    0x182150de9
02150db5: movups xmm0, xmmword ptr [r15]
02150db9: mov    r9, rdi
02150dbc: lea    r8, [rbp - 0x10]
02150dc0: movups xmm1, xmmword ptr [r14]
02150dc4: lea    rcx, [rbp - 0x20]
02150dc8: mov    qword ptr [rsp + 0x20], r12
02150dcd: movaps xmmword ptr [rbp - 0x10], xmm0
02150dd1: movaps xmmword ptr [rbp - 0x20], xmm1
02150dd5: call   0x182151650                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʻʸˀʴʾˀʲʵʸʿʾ
02150dda: add    rsp, 0x50
02150dde: pop    r15
02150de0: pop    r14
02150de2: pop    r12
02150de4: pop    rdi
02150de5: pop    rsi
02150de6: pop    rbx
02150de7: pop    rbp
02150de8: ret    
02150de9: movups xmm0, xmmword ptr [r14]
02150ded: xor    r9d, r9d
02150df0: lea    rcx, [rbp - 0x10]
02150df4: mov    r8, rdi
02150df7: movaps xmmword ptr [rbp - 0x10], xmm0
02150dfb: call   0x182151830
02150e00: add    rsp, 0x50
02150e04: pop    r15
02150e06: pop    r14
02150e08: pop    r12
02150e0a: pop    rdi
02150e0b: pop    rsi
02150e0c: pop    rbx
02150e0d: pop    rbp
02150e0e: ret    
02150e0f: movups xmm0, xmmword ptr [r14]
02150e13: xor    r9d, r9d
02150e16: lea    rcx, [rbp - 0x10]
02150e1a: mov    r8, rdi
02150e1d: mov    edx, esi
02150e1f: movaps xmmword ptr [rbp - 0x10], xmm0
02150e23: call   0x182150e60
02150e28: add    rsp, 0x50
02150e2c: pop    r15
02150e2e: pop    r14
02150e30: pop    r12
02150e32: pop    rdi
02150e33: pop    rsi
02150e34: pop    rbx
02150e35: pop    rbp
02150e36: ret    
02150e37: movups xmm0, xmmword ptr [r14]
02150e3b: mov    r8, qword ptr [rdi]
02150e3e: lea    rcx, [rbp - 0x10]
02150e42: xor    r9d, r9d
02150e45: mov    edx, esi
02150e47: movaps xmmword ptr [rbp - 0x10], xmm0
02150e4b: call   0x182164100                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ˁʸʷʶʳˀʼʹʿʺʳ
02150e50: add    rsp, 0x50
02150e54: pop    r15
02150e56: pop    r14
02150e58: pop    r12
02150e5a: pop    rdi
02150e5b: pop    rsi
02150e5c: pop    rbx
02150e5d: pop    rbp
02150e5e: ret    
02150e5f: int3   
