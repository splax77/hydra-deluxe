0212b9c0: push   rbx
0212b9c2: push   rbp
0212b9c3: push   rsi
0212b9c4: push   rdi
0212b9c5: sub    rsp, 0xa8
0212b9cc: cmp    byte ptr [rip + 0x1dedbb5], 0            ; [0x3f19588] (bss)
0212b9d3: mov    rbp, rdx
0212b9d6: mov    rsi, rcx
0212b9d9: jne    0x18212ba12
0212b9db: lea    rcx, [rip + 0x1b67596]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212b9e2: call   0x182f609b0
0212b9e7: lea    rcx, [rip + 0x1b6ea7a]                   ; [0x3c9a468] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʶʻʸʳʼʾʸʽʴʶʸ<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212b9ee: call   0x182f609b0
0212b9f3: lea    rcx, [rip + 0x1b75676]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212b9fa: call   0x182f609b0
0212b9ff: lea    rcx, [rip + 0x1bc5f2a]                   ; [0x3cf1930] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212ba06: call   0x182f609b0
0212ba0b: mov    byte ptr [rip + 0x1dedb76], 1            ; [0x3f19588] (bss)
0212ba12: mov    rbx, qword ptr [rip + 0x1bc5f17]         ; [0x3cf1930] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212ba19: mov    rdi, qword ptr [rsi + 0xe0]
0212ba20: cmp    qword ptr [rbx + 0x38], 0
0212ba25: jne    0x18212ba2f
0212ba27: mov    rcx, rbx
0212ba2a: call   0x182f657d0
0212ba2f: mov    r8, qword ptr [rbx + 0x38]
0212ba33: lea    rcx, [rsp + 0x20]
0212ba38: mov    rdx, rdi
0212ba3b: mov    r8, qword ptr [r8 + 8]
0212ba3f: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0212ba44: movaps xmm0, xmmword ptr [rsp + 0x20]
0212ba49: lea    rcx, [rsp + 0x20]
0212ba4e: mov    r8, qword ptr [rip + 0x1b6ea13]          ; [0x3c9a468] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʶʻʸʳʼʾʸʽʴʶʸ<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212ba55: mov    rdx, rbp
0212ba58: movdqa xmmword ptr [rsp + 0x20], xmm0
0212ba5e: call   0x1805fac10                              ; ʽˁʷʳʸʶʳʵʸʾʿ$$ʶʻʸʳʼʾʸʽʴʶʸ<object>
0212ba63: mov    rdi, rax
0212ba66: test   rax, rax
0212ba69: je     0x18212bcb8
0212ba6f: mov    rcx, qword ptr [rip + 0x1b755fa]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212ba76: mov    rbx, qword ptr [rax + 0x10]
0212ba7a: mov    qword ptr [rsp + 0xd0], r12
0212ba82: mov    qword ptr [rsp + 0xd8], r13
0212ba8a: cmp    dword ptr [rcx + 0xe0], 0
0212ba91: mov    qword ptr [rsp + 0xe0], r14
0212ba99: mov    qword ptr [rsp + 0xa0], r15
0212baa1: movaps xmmword ptr [rsp + 0x90], xmm6
0212baa9: movaps xmmword ptr [rsp + 0x80], xmm7
0212bab1: movsd  xmm7, qword ptr [rax + 0x18]
0212bab6: movaps xmmword ptr [rsp + 0x70], xmm8
0212babc: movsd  xmm8, qword ptr [rax + 0x20]
0212bac2: movaps xmmword ptr [rsp + 0x60], xmm9
0212bac8: movsd  xmm9, qword ptr [rsi + 0xb0]
0212bad1: movaps xmmword ptr [rsp + 0x50], xmm10
0212bad7: movaps xmmword ptr [rsp + 0x40], xmm11
0212badd: movaps xmmword ptr [rsp + 0x30], xmm12
0212bae3: jne    0x18212baea
0212bae5: call   0x182f60cf0
0212baea: movsd  xmm12, qword ptr [rip + 0xf3a0dd]        ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
0212baf3: xorps  xmm6, xmm6
0212baf6: movsd  xmm10, qword ptr [rip + 0xf3b501]        ; [0x3067000] dbl=5e-324 q=0x1
0212baff: mov    rax, rbp
0212bb02: movsd  xmm11, qword ptr [rip + 0xf3b535]        ; [0x3067040] dbl=-1.7976931348623157e+308 q=0xffefffffffffffff
0212bb0b: sub    rax, rbx
0212bb0e: mulsd  xmm7, xmm9
0212bb13: movabs r12, 0x7fffffffffffffff
0212bb1d: movabs r13, 0x7ff0000000000000
0212bb27: movabs r14, 0x8000000000000000
0212bb31: movabs r15, 0xfff0000000000000
0212bb3b: cvtsi2sd xmm6, rax
0212bb40: mulsd  xmm6, xmm12
0212bb45: divsd  xmm6, xmm7
0212bb49: addsd  xmm6, xmm8
0212bb4e: nop    
0212bb50: mov    rcx, qword ptr [rip + 0x1b75519]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212bb57: mov    rbx, qword ptr [rdi + 0x10]
0212bb5b: movsd  xmm7, qword ptr [rdi + 0x20]
0212bb60: movsd  xmm8, qword ptr [rsi + 0xb0]
0212bb69: cmp    dword ptr [rcx + 0xe0], 0
0212bb70: movsd  xmm9, qword ptr [rdi + 0x18]
0212bb76: jne    0x18212bb7d
0212bb78: call   0x182f60cf0
0212bb7d: movaps xmm0, xmm6
0212bb80: subsd  xmm0, xmm7
0212bb84: mulsd  xmm0, xmm8
0212bb89: mulsd  xmm0, xmm9
0212bb8e: divsd  xmm0, xmm12
0212bb93: cvttsd2si rax, xmm0
0212bb98: add    rax, rbx
0212bb9b: cmp    rax, rbp
0212bb9e: je     0x18212bc5b
0212bba4: mov    rcx, qword ptr [rip + 0x1b673cd]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212bbab: cmp    dword ptr [rcx + 0xe0], 0
0212bbb2: jne    0x18212bbb9
0212bbb4: call   0x182f60cf0
0212bbb9: cmp    byte ptr [rip + 0x1de4fd4], 0            ; [0x3f10b94] (bss)
0212bbc0: jne    0x18212bbd5
0212bbc2: lea    rcx, [rip + 0x1b673af]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212bbc9: call   0x182f609b0
0212bbce: mov    byte ptr [rip + 0x1de4fbf], 1            ; [0x3f10b94] (bss)
0212bbd5: mov    rcx, qword ptr [rip + 0x1b6739c]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212bbdc: cmp    dword ptr [rcx + 0xe0], 0
0212bbe3: jne    0x18212bbea
0212bbe5: call   0x182f60cf0
0212bbea: movq   rbx, xmm6
0212bbef: mov    rax, rbx
0212bbf2: and    rax, r12
0212bbf5: cmp    rax, r13
0212bbf8: jae    0x18212bc49
0212bbfa: cmp    rbx, r14
0212bbfd: je     0x18212bc40
0212bbff: mov    rcx, qword ptr [rip + 0x1b67372]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212bc06: cmp    dword ptr [rcx + 0xe0], 0
0212bc0d: jne    0x18212bc14
0212bc0f: call   0x182f60cf0
0212bc14: test   rbx, rbx
0212bc17: jns    0x18212bc1e
0212bc19: dec    rbx
0212bc1c: jmp    0x18212bc21
0212bc1e: inc    rbx
0212bc21: mov    rcx, qword ptr [rip + 0x1b67350]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0212bc28: cmp    dword ptr [rcx + 0xe0], 0
0212bc2f: jne    0x18212bc36
0212bc31: call   0x182f60cf0
0212bc36: movq   xmm6, rbx
0212bc3b: jmp    0x18212bb50
0212bc40: movaps xmm6, xmm10
0212bc44: jmp    0x18212bb50
0212bc49: cmp    rbx, r15
0212bc4c: jne    0x18212bb50
0212bc52: movaps xmm6, xmm11
0212bc56: jmp    0x18212bb50
0212bc5b: movaps xmm12, xmmword ptr [rsp + 0x30]
0212bc61: movaps xmm0, xmm6
0212bc64: movaps xmm6, xmmword ptr [rsp + 0x90]
0212bc6c: movaps xmm11, xmmword ptr [rsp + 0x40]
0212bc72: movaps xmm10, xmmword ptr [rsp + 0x50]
0212bc78: movaps xmm9, xmmword ptr [rsp + 0x60]
0212bc7e: movaps xmm8, xmmword ptr [rsp + 0x70]
0212bc84: movaps xmm7, xmmword ptr [rsp + 0x80]
0212bc8c: mov    r15, qword ptr [rsp + 0xa0]
0212bc94: mov    r14, qword ptr [rsp + 0xe0]
0212bc9c: mov    r13, qword ptr [rsp + 0xd8]
0212bca4: mov    r12, qword ptr [rsp + 0xd0]
0212bcac: add    rsp, 0xa8
0212bcb3: pop    rdi
0212bcb4: pop    rsi
0212bcb5: pop    rbp
0212bcb6: pop    rbx
0212bcb7: ret    
0212bcb8: call   0x182f60c50
0212bcbd: int3   
0212bcbe: int3   
