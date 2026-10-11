020f5810: push   rbp
020f5812: push   rdi
020f5813: lea    rbp, [rsp - 0x4f]
020f5818: sub    rsp, 0xc8
020f581f: cmp    byte ptr [rip + 0x1e23b8c], 0            ; [0x3f193b2] (bss)
020f5826: mov    rdi, rcx
020f5829: jne    0x1820f586e
020f582b: lea    rcx, [rip + 0x1b9c77e]                   ; [0x3c91fb0] metamethod:Method$System.Array.Resize<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f5832: call   0x182f609b0
020f5837: lea    rcx, [rip + 0x1be19a2]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f583e: call   0x182f609b0
020f5843: lea    rcx, [rip + 0x1be1a56]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f584a: call   0x182f609b0
020f584f: lea    rcx, [rip + 0x1bb5e0a]                   ; [0x3cab660] metamethod:Method$System.Span<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Length()
020f5856: call   0x182f609b0
020f585b: lea    rcx, [rip + 0x1bfc24e]                   ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f5862: call   0x182f609b0
020f5867: mov    byte ptr [rip + 0x1e23b44], 1            ; [0x3f193b2] (bss)
020f586e: xor    eax, eax
020f5870: xorps  xmm0, xmm0
020f5873: xorps  xmm1, xmm1
020f5876: mov    qword ptr [rbp - 0x31], rax
020f587a: movups xmmword ptr [rbp - 0x41], xmm0
020f587e: mov    qword ptr [rbp - 0x19], rax
020f5882: movups xmmword ptr [rbp - 0x29], xmm1
020f5886: cmp    qword ptr [rdi + 0x1e0], rax
020f588d: je     0x1820f5d62
020f5893: mov    rax, qword ptr [rdi + 0x1e0]
020f589a: cmp    dword ptr [rax + 0x18], 0
020f589e: je     0x1820f5d62
020f58a4: mov    ecx, dword ptr [rax + 0x18]
020f58a7: cmp    dword ptr [rdi + 0x50], ecx
020f58aa: jge    0x1820f5d62
020f58b0: mov    qword ptr [rsp + 0xe0], rbx
020f58b8: mov    qword ptr [rsp + 0xe8], rsi
020f58c0: mov    qword ptr [rsp + 0xf0], r12
020f58c8: mov    qword ptr [rsp + 0xc0], r13
020f58d0: mov    qword ptr [rsp + 0xb0], r15
020f58d8: movaps xmmword ptr [rsp + 0x70], xmm9
020f58de: mov    rbx, qword ptr [rip + 0x1bfc1cb]         ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f58e5: mov    rsi, rax
020f58e8: cmp    qword ptr [rbx + 0x38], 0
020f58ed: jne    0x1820f58f7
020f58ef: mov    rcx, rbx
020f58f2: call   0x182f657d0
020f58f7: mov    r8, qword ptr [rbx + 0x38]
020f58fb: lea    rcx, [rbp - 0x59]
020f58ff: mov    qword ptr [rsp + 0xb8], r14
020f5907: mov    rdx, rsi
020f590a: movaps xmmword ptr [rsp + 0xa0], xmm6
020f5912: movaps xmmword ptr [rsp + 0x90], xmm7
020f591a: mov    r8, qword ptr [r8 + 8]
020f591e: movaps xmmword ptr [rsp + 0x80], xmm8
020f5927: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
020f592c: movsxd r13, dword ptr [rbp - 0x51]
020f5930: movabs rsi, 0x7fffffffffffffff
020f593a: mov    r15d, dword ptr [rdi + 0x50]
020f593e: mov    r12, qword ptr [rbp - 0x59]
020f5942: movsd  xmm9, qword ptr [rdi + 0x100]
020f594b: cmp    r15d, r13d
020f594e: jge    0x1820f5acf
020f5954: movsd  xmm7, qword ptr [rip + 0xf6efd4]         ; [0x3064930] dbl=-0.0 q=0x8000000000000000
020f595c: movsd  xmm8, qword ptr [rip + 0xf6f093]         ; [0x30649f8] dbl=3.0 q=0x4008000000000000
020f5965: jae    0x1820f5d70
020f596b: movsxd rax, r15d
020f596e: mov    rbx, qword ptr [r12 + rax*8]
020f5972: test   rbx, rbx
020f5975: je     0x1820f5d86
020f597b: movsd  xmm6, qword ptr [rbx + 0x28]
020f5980: xor    r8d, r8d
020f5983: mov    rdx, rbx
020f5986: subsd  xmm6, xmm9
020f598b: mov    rcx, rdi
020f598e: call   0x1820f3a90                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʴʼʶʿʴʲʶʵʼʳˁ
020f5993: cmp    byte ptr [rdi + 0x195], 0
020f599a: movaps xmm1, xmm0
020f599d: addsd  xmm1, xmm0
020f59a1: movsd  qword ptr [rdi + 0x10], xmm0
020f59a6: xorps  xmm0, xmm7
020f59a9: movsd  qword ptr [rdi + 0x18], xmm0
020f59ae: movsd  qword ptr [rdi + 0x20], xmm1
020f59b3: je     0x1820f59c5
020f59b5: xorps  xmm1, xmm7
020f59b8: mov    qword ptr [rdi + 0x10], 0
020f59c0: movsd  qword ptr [rdi + 0x18], xmm1
020f59c5: xor    edx, edx
020f59c7: mov    rcx, rbx
020f59ca: comisd xmm6, xmmword ptr [rdi + 0x10]
020f59cf: ja     0x1820f5c33
020f59d5: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020f59da: test   al, al
020f59dc: jne    0x1820f5c84
020f59e2: movsd  xmm0, qword ptr [rdi + 0x18]
020f59e7: comisd xmm0, xmm6
020f59eb: ja     0x1820f5b9d
020f59f1: cmp    qword ptr [rbx + 0x40], rsi
020f59f5: je     0x1820f5c80
020f59fb: movsd  xmm0, qword ptr [rdi + 0x10]
020f5a00: comisd xmm0, xmm6
020f5a04: jb     0x1820f5c80
020f5a0a: comisd xmm6, xmmword ptr [rdi + 0x18]
020f5a0f: jb     0x1820f5c80
020f5a15: mov    rcx, qword ptr [rdi + 0x58]
020f5a19: test   rcx, rcx
020f5a1c: je     0x1820f5d86
020f5a22: mov    eax, dword ptr [rdi + 0x8c]
020f5a28: cmp    dword ptr [rcx + 0x18], eax
020f5a2b: jne    0x1820f5a42
020f5a2d: mov    edx, dword ptr [rcx + 0x18]
020f5a30: lea    rcx, [rdi + 0x58]
020f5a34: mov    r8, qword ptr [rip + 0x1b9c575]          ; [0x3c91fb0] metamethod:Method$System.Array.Resize<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f5a3b: add    edx, edx
020f5a3d: call   0x1804129f0                              ; System.Array$$Resize<BindingRestrictions.TestBuilder.AndNode>
020f5a42: mov    rsi, qword ptr [rdi + 0x58]
020f5a46: test   rsi, rsi
020f5a49: je     0x1820f5d86
020f5a4f: mov    rdx, qword ptr [rsi]
020f5a52: mov    rcx, rbx
020f5a55: movsxd r14, dword ptr [rdi + 0x8c]
020f5a5c: mov    rdx, qword ptr [rdx + 0x40]
020f5a60: call   0x182f5fc20
020f5a65: test   rax, rax
020f5a68: je     0x1820f5d76
020f5a6e: cmp    r14d, dword ptr [rsi + 0x18]
020f5a72: jae    0x1820f5d70
020f5a78: lea    rax, [r14 + 4]
020f5a7c: mov    qword ptr [rsi + r14*8 + 0x20], rbx
020f5a81: lea    rcx, [rsi + rax*8]
020f5a85: mov    rdx, rbx
020f5a88: call   0x182f5fc00
020f5a8d: inc    dword ptr [rdi + 0x8c]
020f5a93: xor    edx, edx
020f5a95: mov    rcx, rbx
020f5a98: call   0x18214be50                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʼʶˀʵʳˁʴˀʼʽ
020f5a9d: test   al, al
020f5a9f: jne    0x1820f5aaf
020f5aa1: xor    edx, edx
020f5aa3: mov    rcx, rbx
020f5aa6: call   0x18214ba10                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʺʲʾˁʷʵʹʻʵʹ
020f5aab: test   al, al
020f5aad: je     0x1820f5ab5
020f5aaf: inc    dword ptr [rdi + 0x90]
020f5ab5: cmp    byte ptr [rdi + 0x19c], 0
020f5abc: je     0x1820f5c80
020f5ac2: cmp    dword ptr [rdi + 0x88], 0
020f5ac9: jle    0x1820f5c80
020f5acf: mov    rax, qword ptr [rdi + 0x1e0]
020f5ad6: mov    ecx, dword ptr [rdi + 0x50]
020f5ad9: test   rax, rax
020f5adc: je     0x1820f5d86
020f5ae2: cmp    ecx, dword ptr [rax + 0x18]
020f5ae5: jge    0x1820f5b8b
020f5aeb: mov    rcx, qword ptr [rdi + 0x1e0]
020f5af2: test   rcx, rcx
020f5af5: je     0x1820f5d86
020f5afb: mov    r8, qword ptr [rip + 0x1be179e]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f5b02: mov    edx, dword ptr [rdi + 0x50]
020f5b05: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f5b0a: mov    rsi, rax
020f5b0d: test   rax, rax
020f5b10: je     0x1820f5d86
020f5b16: xor    r8d, r8d
020f5b19: lea    rcx, [rbp - 0x59]
020f5b1d: mov    rdx, rax
020f5b20: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020f5b25: xor    r8d, r8d
020f5b28: lea    rdx, [rbp - 0x29]
020f5b2c: lea    rcx, [rbp - 0x59]
020f5b30: movups xmm0, xmmword ptr [rax]
020f5b33: movups xmmword ptr [rbp - 0x29], xmm0
020f5b37: movsd  xmm1, qword ptr [rax + 0x10]
020f5b3c: movsd  qword ptr [rbp - 0x19], xmm1
020f5b41: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020f5b46: movups xmm0, xmmword ptr [rax]
020f5b49: movups xmmword ptr [rbp - 0x41], xmm0
020f5b4d: movsd  xmm1, qword ptr [rax + 0x10]
020f5b52: movsd  qword ptr [rbp - 0x31], xmm1
020f5b57: nop    word ptr [rax + rax]
020f5b60: xor    edx, edx
020f5b62: lea    rcx, [rbp - 0x41]
020f5b66: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f5b6b: test   al, al
020f5b6d: je     0x1820f5c95
020f5b73: mov    rcx, qword ptr [rbp - 0x39]
020f5b77: test   rcx, rcx
020f5b7a: je     0x1820f5d86
020f5b80: xor    edx, edx
020f5b82: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020f5b87: test   al, al
020f5b89: jne    0x1820f5b60
020f5b8b: lea    eax, [r13 - 1]
020f5b8f: cmp    eax, r13d
020f5b92: jae    0x1820f5d70
020f5b98: jmp    0x1820f5cc2
020f5b9d: xor    r8d, r8d
020f5ba0: lea    rcx, [rbp - 0x59]
020f5ba4: mov    rdx, rbx
020f5ba7: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020f5bac: xor    r8d, r8d
020f5baf: lea    rdx, [rbp - 0x29]
020f5bb3: lea    rcx, [rbp - 0x59]
020f5bb7: movups xmm0, xmmword ptr [rax]
020f5bba: movups xmmword ptr [rbp - 0x29], xmm0
020f5bbe: movsd  xmm1, qword ptr [rax + 0x10]
020f5bc3: movsd  qword ptr [rbp - 0x19], xmm1
020f5bc8: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020f5bcd: xor    edx, edx
020f5bcf: lea    rcx, [rbp - 0x41]
020f5bd3: movups xmm0, xmmword ptr [rax]
020f5bd6: movups xmmword ptr [rbp - 0x41], xmm0
020f5bda: movsd  xmm1, qword ptr [rax + 0x10]
020f5bdf: movsd  qword ptr [rbp - 0x31], xmm1
020f5be4: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f5be9: test   al, al
020f5beb: je     0x1820f5c84
020f5bf1: mov    rbx, qword ptr [rbp - 0x39]
020f5bf5: test   rbx, rbx
020f5bf8: je     0x1820f5d86
020f5bfe: xor    edx, edx
020f5c00: mov    rcx, rbx
020f5c03: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020f5c08: test   al, al
020f5c0a: jne    0x1820f5c22
020f5c0c: mov    rax, qword ptr [rdi]
020f5c0f: mov    rdx, rbx
020f5c12: mov    rcx, rdi
020f5c15: mov    r8, qword ptr [rax + 0x180]
020f5c1c: call   qword ptr [rax + 0x178]
020f5c22: xor    edx, edx
020f5c24: lea    rcx, [rbp - 0x41]
020f5c28: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f5c2d: test   al, al
020f5c2f: jne    0x1820f5bf1
020f5c31: jmp    0x1820f5c84
020f5c33: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020f5c38: test   al, al
020f5c3a: jne    0x1820f5acf
020f5c40: xor    edx, edx
020f5c42: mov    rcx, rbx
020f5c45: call   0x18214be50                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʼʶˀʵʳˁʴˀʼʽ
020f5c4a: test   al, al
020f5c4c: jne    0x1820f5c60
020f5c4e: xor    edx, edx
020f5c50: mov    rcx, rbx
020f5c53: call   0x18214ba10                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʺʲʾˁʷʵʹʻʵʹ
020f5c58: test   al, al
020f5c5a: je     0x1820f5acf
020f5c60: movsd  xmm0, qword ptr [rdi + 0x10]
020f5c65: mulsd  xmm0, xmm8
020f5c6a: comisd xmm0, xmm6
020f5c6e: jb     0x1820f5acf
020f5c74: cmp    qword ptr [rbx + 0x40], rsi
020f5c78: je     0x1820f5c80
020f5c7a: inc    dword ptr [rdi + 0x90]
020f5c80: mov    rsi, qword ptr [rbx + 0x40]
020f5c84: inc    r15d
020f5c87: cmp    r15d, r13d
020f5c8a: jl     0x1820f5965
020f5c90: jmp    0x1820f5acf
020f5c95: mov    eax, dword ptr [rdi + 0x50]
020f5c98: xor    edx, edx
020f5c9a: mov    rcx, rsi
020f5c9d: mov    dword ptr [rdi + 0x54], eax
020f5ca0: mov    ebx, eax
020f5ca2: call   0x18214ba20                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽˀʺˁʵʸʲʴʺʽʼ
020f5ca7: lea    ecx, [rax + rbx]
020f5caa: mov    dword ptr [rdi + 0x50], ecx
020f5cad: mov    rax, qword ptr [rdi + 0x1e0]
020f5cb4: test   rax, rax
020f5cb7: je     0x1820f5d86
020f5cbd: jmp    0x1820f5ae2
020f5cc2: mov    rax, qword ptr [rdi + 0x1e0]
020f5cc9: test   rax, rax
020f5ccc: je     0x1820f5d86
020f5cd2: cmp    dword ptr [rax + 0x18], 0
020f5cd6: jle    0x1820f5d04
020f5cd8: cmp    dword ptr [rdi + 0x50], 0
020f5cdc: jle    0x1820f5d04
020f5cde: mov    rcx, qword ptr [r12 + r13*8 - 8]
020f5ce3: test   rcx, rcx
020f5ce6: je     0x1820f5d86
020f5cec: xor    edx, edx
020f5cee: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020f5cf3: test   al, al
020f5cf5: jne    0x1820f5d04
020f5cf7: xor    eax, eax
020f5cf9: cmp    byte ptr [rdi + 0x154], al
020f5cff: sete   al
020f5d02: jmp    0x1820f5d06
020f5d04: xor    eax, eax
020f5d06: movaps xmm8, xmmword ptr [rsp + 0x80]
020f5d0f: movaps xmm7, xmmword ptr [rsp + 0x90]
020f5d17: movaps xmm6, xmmword ptr [rsp + 0xa0]
020f5d1f: mov    r14, qword ptr [rsp + 0xb8]
020f5d27: mov    r15, qword ptr [rsp + 0xb0]
020f5d2f: mov    r13, qword ptr [rsp + 0xc0]
020f5d37: mov    r12, qword ptr [rsp + 0xf0]
020f5d3f: mov    rsi, qword ptr [rsp + 0xe8]
020f5d47: mov    rbx, qword ptr [rsp + 0xe0]
020f5d4f: movaps xmm9, xmmword ptr [rsp + 0x70]
020f5d55: mov    byte ptr [rdi + 0x40], al
020f5d58: add    rsp, 0xc8
020f5d5f: pop    rdi
020f5d60: pop    rbp
020f5d61: ret    
020f5d62: mov    byte ptr [rdi + 0x40], 0
020f5d66: add    rsp, 0xc8
020f5d6d: pop    rdi
020f5d6e: pop    rbp
020f5d6f: ret    
020f5d70: call   0x182f60c40
020f5d75: int3   
020f5d76: call   0x182f604f0
020f5d7b: mov    rcx, rax
020f5d7e: xor    edx, edx
020f5d80: call   0x182f60c10
020f5d85: int3   
020f5d86: call   0x182f60c50
020f5d8b: int3   
020f5d8c: int3   
