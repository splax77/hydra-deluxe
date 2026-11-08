0212bcc0: push   rsi
0212bcc2: sub    rsp, 0x70
0212bcc6: cmp    byte ptr [rip + 0x1ded8bc], 0            ; [0x3f19589] (bss)
0212bccd: mov    rsi, rcx
0212bcd0: movaps xmmword ptr [rsp + 0x60], xmm6
0212bcd5: movaps xmm6, xmm1
0212bcd8: movaps xmmword ptr [rsp + 0x50], xmm7
0212bcdd: movaps xmmword ptr [rsp + 0x40], xmm8
0212bce3: jne    0x18212bcf8
0212bce5: lea    rcx, [rip + 0x1b75384]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212bcec: call   0x182f609b0
0212bcf1: mov    byte ptr [rip + 0x1ded891], 1            ; [0x3f19589] (bss)
0212bcf8: xorps  xmm7, xmm7
0212bcfb: comisd xmm7, xmm6
0212bcff: jbe    0x18212bd04
0212bd01: xorps  xmm6, xmm6
0212bd04: cmp    byte ptr [rip + 0x1ded888], 0            ; [0x3f19593] (bss)
0212bd0b: movaps xmm8, xmm6
0212bd0f: jne    0x18212bd3c
0212bd11: lea    rcx, [rip + 0x1ba8c68]                   ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
0212bd18: call   0x182f609b0
0212bd1d: lea    rcx, [rip + 0x1b7f65c]                   ; [0x3cab380] metamethod:Method$System.Span<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Length()
0212bd24: call   0x182f609b0
0212bd29: lea    rcx, [rip + 0x1bc5c00]                   ; [0x3cf1930] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212bd30: call   0x182f609b0
0212bd35: mov    byte ptr [rip + 0x1ded857], 1            ; [0x3f19593] (bss)
0212bd3c: comisd xmm7, xmm6
0212bd40: jbe    0x18212bd46
0212bd42: xorps  xmm8, xmm8
0212bd46: mov    rcx, qword ptr [rsi + 0xe0]
0212bd4d: mov    qword ptr [rsp + 0x80], rbx
0212bd55: mov    qword ptr [rsp + 0x90], rdi
0212bd5d: test   rcx, rcx
0212bd60: je     0x18212be80
0212bd66: mov    r8, qword ptr [rip + 0x1ba8c13]          ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
0212bd6d: xor    edx, edx
0212bd6f: mov    qword ptr [rsp + 0x88], rbp
0212bd77: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0212bd7c: mov    rdi, qword ptr [rip + 0x1bc5bad]         ; [0x3cf1930] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʳʿʼʼʾʺʿʺʸʸ>()
0212bd83: mov    rbx, rax
0212bd86: mov    rbp, qword ptr [rsi + 0xe0]
0212bd8d: cmp    qword ptr [rdi + 0x38], 0
0212bd92: jne    0x18212bd9c
0212bd94: mov    rcx, rdi
0212bd97: call   0x182f657d0
0212bd9c: mov    r8, qword ptr [rdi + 0x38]
0212bda0: lea    rcx, [rsp + 0x20]
0212bda5: mov    rdx, rbp
0212bda8: mov    r8, qword ptr [r8 + 8]
0212bdac: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0212bdb1: mov    r8d, dword ptr [rsp + 0x28]
0212bdb6: xor    ecx, ecx
0212bdb8: mov    r9, qword ptr [rsp + 0x20]
0212bdbd: mov    rbp, qword ptr [rsp + 0x88]
0212bdc5: test   r8d, r8d
0212bdc8: jle    0x18212bdfe
0212bdca: nop    word ptr [rax + rax]
0212bdd0: cmp    ecx, r8d
0212bdd3: jae    0x18212be86
0212bdd9: mov    rdx, qword ptr [r9 + rcx*8]
0212bddd: test   rdx, rdx
0212bde0: je     0x18212be80
0212bde6: movsd  xmm0, qword ptr [rdx + 0x20]
0212bdeb: comisd xmm0, xmm8
0212bdf0: jae    0x18212bdfe
0212bdf2: inc    ecx
0212bdf4: mov    rbx, rdx
0212bdf7: cmp    ecx, r8d
0212bdfa: jl     0x18212bdd3
0212bdfc: jmp    0x18212be03
0212bdfe: test   rbx, rbx
0212be01: je     0x18212be80
0212be03: mov    rcx, qword ptr [rip + 0x1b75266]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212be0a: mov    rdi, qword ptr [rbx + 0x10]
0212be0e: movsd  xmm7, qword ptr [rbx + 0x20]
0212be13: movsd  xmm8, qword ptr [rsi + 0xb0]
0212be1c: cmp    dword ptr [rcx + 0xe0], 0
0212be23: movaps xmmword ptr [rsp + 0x30], xmm9
0212be29: movsd  xmm9, qword ptr [rbx + 0x18]
0212be2f: jne    0x18212be36
0212be31: call   0x182f60cf0
0212be36: mov    rbx, qword ptr [rsp + 0x80]
0212be3e: subsd  xmm6, xmm7
0212be42: movaps xmm7, xmmword ptr [rsp + 0x50]
0212be47: mulsd  xmm6, xmm8
0212be4c: movaps xmm8, xmmword ptr [rsp + 0x40]
0212be52: mulsd  xmm6, xmm9
0212be57: movaps xmm9, xmmword ptr [rsp + 0x30]
0212be5d: divsd  xmm6, qword ptr [rip + 0xf39d6b]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
0212be65: cvttsd2si rax, xmm6
0212be6a: movaps xmm6, xmmword ptr [rsp + 0x60]
0212be6f: add    rax, rdi
0212be72: mov    rdi, qword ptr [rsp + 0x90]
0212be7a: add    rsp, 0x70
0212be7e: pop    rsi
0212be7f: ret    
0212be80: call   0x182f60c50
0212be85: int3   
0212be86: call   0x182f60c40
0212be8b: int3   
0212be8c: int3   
