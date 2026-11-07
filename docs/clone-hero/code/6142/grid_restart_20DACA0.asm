020daca0: push   r13
020daca2: push   r15
020daca4: sub    rsp, 0x78
020daca8: cmp    byte ptr [rip + 0x1e3e62c], 0            ; [0x3f192db] (bss)
020dacaf: mov    r13, rdx
020dacb2: mov    r15, rcx
020dacb5: jne    0x1820dacee
020dacb7: lea    rcx, [rip + 0x1bf8582]                   ; [0x3cd3240] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>.get_Count()
020dacbe: call   0x182f609b0
020dacc3: lea    rcx, [rip + 0x1c33c2e]                   ; [0x3d0e8f8] metamethod:Method$System.Collections.Generic.List<double>.get_Count()
020dacca: call   0x182f609b0
020daccf: lea    rcx, [rip + 0x1bc5c9a]                   ; [0x3ca0970] metamethod:Method$System.Collections.Generic.List<long>.get_Count()
020dacd6: call   0x182f609b0
020dacdb: lea    rcx, [rip + 0x1bf86de]                   ; [0x3cd33c0] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>.set_Item()
020dace2: call   0x182f609b0
020dace7: mov    byte ptr [rip + 0x1e3e5ed], 1            ; [0x3f192db] (bss)
020dacee: mov    qword ptr [rsp + 0x90], rbx
020dacf6: mov    qword ptr [rsp + 0x70], rbp
020dacfb: mov    qword ptr [rsp + 0x68], rsi
020dad00: mov    qword ptr [rsp + 0x60], rdi
020dad05: mov    qword ptr [rsp + 0x58], r12
020dad0a: mov    qword ptr [rsp + 0x50], r14
020dad0f: movaps xmmword ptr [rsp + 0x40], xmm6
020dad14: test   r13, r13
020dad17: je     0x1820db006
020dad1d: xor    r8d, r8d
020dad20: mov    dl, 1
020dad22: mov    rcx, r13
020dad25: call   0x18212e130                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʿʼʿʿʵʵʲʿʷˁʻ
020dad2a: mov    qword ptr [rsp + 0xa8], rax
020dad32: mov    dword ptr [rsp + 0x98], 0
020dad3d: test   rax, rax
020dad40: js     0x1820dafb3
020dad46: movsd  xmm6, qword ptr [rip + 0xf8ae72]         ; [0x3065bc0] dbl=4.0 q=0x4010000000000000
020dad4e: xor    ebp, ebp
020dad50: xor    edi, edi
020dad52: xor    r14d, r14d
020dad55: xor    r12d, r12d
020dad58: nop    dword ptr [rax + rax]
020dad60: cmp    byte ptr [rip + 0x1e3e5be], 0            ; [0x3f19325] (bss)
020dad67: jne    0x1820dad88
020dad69: lea    rcx, [rip + 0x1bbf648]                   ; [0x3c9a3b8] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʶʻʸʳʼʾʸʽʴʶʸ<ʵʴʻʵʶʾʵʾʳʵʺ>()
020dad70: call   0x182f609b0
020dad75: lea    rcx, [rip + 0x1c16734]                   ; [0x3cf14b0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʵʴʻʵʶʾʵʾʳʵʺ>()
020dad7c: call   0x182f609b0
020dad81: mov    byte ptr [rip + 0x1e3e59d], 1            ; [0x3f19325] (bss)
020dad88: mov    rbx, qword ptr [rip + 0x1c16721]         ; [0x3cf14b0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʵʴʻʵʶʾʵʾʳʵʺ>()
020dad8f: mov    rsi, qword ptr [r13 + 0xe8]
020dad96: cmp    qword ptr [rbx + 0x38], 0
020dad9b: jne    0x1820dada5
020dad9d: mov    rcx, rbx
020dada0: call   0x182f657d0
020dada5: mov    r8, qword ptr [rbx + 0x38]
020dada9: lea    rcx, [rsp + 0x30]
020dadae: mov    rdx, rsi
020dadb1: mov    r8, qword ptr [r8 + 8]
020dadb5: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
020dadba: mov    rcx, qword ptr [rip + 0x1bbf5f7]         ; [0x3c9a3b8] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʶʻʸʳʼʾʸʽʴʶʸ<ʵʴʻʵʶʾʵʾʳʵʺ>()
020dadc1: mov    ebx, dword ptr [rsp + 0x38]
020dadc5: cmp    qword ptr [rcx + 0x38], 0
020dadca: jne    0x1820dadd1
020dadcc: call   0x182f657d0
020dadd1: test   ebx, ebx
020dadd3: je     0x1820db006
020dadd9: mov    r10, qword ptr [rsp + 0x30]
020dadde: test   rbp, rbp
020dade1: je     0x1820dae57
020dade3: xor    r8d, r8d
020dade6: lea    r9d, [rbx - 1]
020dadea: test   r9d, r9d
020daded: jle    0x1820dae2b
020dadef: nop    
020dadf0: lea    eax, [r8 + 1]
020dadf4: add    eax, r9d
020dadf7: cdq    
020dadf8: sub    eax, edx
020dadfa: sar    eax, 1
020dadfc: cmp    eax, ebx
020dadfe: jae    0x1820db00c
020dae04: movsxd rcx, eax
020dae07: mov    rdx, qword ptr [r10 + rcx*8]
020dae0b: test   rdx, rdx
020dae0e: je     0x1820db006
020dae14: cmp    qword ptr [rdx + 0x10], rbp
020dae18: lea    ecx, [rax - 1]
020dae1b: cmovle ecx, r9d
020dae1f: cmovle r8d, eax
020dae23: mov    r9d, ecx
020dae26: cmp    r8d, ecx
020dae29: jl     0x1820dadf0
020dae2b: cmp    r8d, ebx
020dae2e: jae    0x1820db00c
020dae34: movsxd rax, r8d
020dae37: mov    rcx, qword ptr [r10 + rax*8]
020dae3b: test   rcx, rcx
020dae3e: je     0x1820db006
020dae44: cmp    qword ptr [rcx + 0x10], rbp
020dae48: jg     0x1820db006
020dae4e: movsxd rax, r8d
020dae51: mov    rbx, qword ptr [r10 + rax*8]
020dae55: jmp    0x1820dae6b
020dae57: test   ebx, ebx
020dae59: je     0x1820db00c
020dae5f: mov    rbx, qword ptr [r10]
020dae62: test   rbx, rbx
020dae65: je     0x1820db006
020dae6b: test   rdi, rdi
020dae6e: jne    0x1820dae78
020dae70: mov    r8d, 1
020dae76: jmp    0x1820dae95
020dae78: mov    eax, dword ptr [rbx + 0x18]
020dae7b: cmp    dword ptr [rdi + 0x18], eax
020dae7e: je     0x1820dae88
020dae80: mov    r8d, 1
020dae86: jmp    0x1820dae95
020dae88: mov    eax, dword ptr [rbx + 0x1c]
020dae8b: xor    r8d, r8d
020dae8e: cmp    dword ptr [rdi + 0x1c], eax
020dae91: setne  r8b
020dae95: mov    r9d, dword ptr [rbx + 0x1c]
020dae99: xor    esi, esi
020dae9b: test   r8b, r8b
020dae9e: cmove  esi, r12d
020daea2: xor    edx, edx
020daea4: test   r8b, r8b
020daea7: cmove  edx, r14d
020daeab: shr    r9d, 2
020daeaf: mov    ecx, dword ptr [rbx + 0x18]
020daeb2: xor    r14d, r14d
020daeb5: movsxd rax, edx
020daeb8: mov    r12d, 1
020daebe: cmp    rax, rcx
020daec1: cmovne r14d, edx
020daec5: test   r14d, r14d
020daec8: cmove  r8d, r12d
020daecc: test   r8d, r8d
020daecf: jne    0x1820daef4
020daed1: cmp    dword ptr [rbx + 0x1c], 4
020daed5: jbe    0x1820daeef
020daed7: movsxd rax, r14d
020daeda: cqo    
020daedc: mov    ecx, r9d
020daedf: idiv   rcx
020daee2: test   rdx, rdx
020daee5: je     0x1820daeef
020daee7: mov    r9d, 2
020daeed: jmp    0x1820daf52
020daeef: mov    r9d, r12d
020daef2: jmp    0x1820daf52
020daef4: cmp    dword ptr [rbx + 0x18], r12d
020daef8: jne    0x1820daf1c
020daefa: test   esi, esi
020daefc: jle    0x1820daf1c
020daefe: cmp    dword ptr [rbx + 0x1c], 4
020daf02: jbe    0x1820daeef
020daf04: movsxd rax, esi
020daf07: cqo    
020daf09: mov    ecx, r9d
020daf0c: idiv   rcx
020daf0f: test   rdx, rdx
020daf12: je     0x1820daeef
020daf14: mov    r9d, 2
020daf1a: jmp    0x1820daf52
020daf1c: test   rdi, rdi
020daf1f: je     0x1820daf4f
020daf21: cmp    dword ptr [rdi + 0x1c], 4
020daf25: jbe    0x1820daf4f
020daf27: mov    rdx, qword ptr [r15 + 0x18]
020daf2b: test   rdx, rdx
020daf2e: je     0x1820db006
020daf34: mov    edx, dword ptr [rdx + 0x18]
020daf37: mov    r8d, 2
020daf3d: mov    r9, qword ptr [rip + 0x1bf847c]          ; [0x3cd33c0] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>.set_Item()
020daf44: dec    edx
020daf46: mov    rcx, qword ptr [r15 + 0x18]
020daf4a: call   0x180761160                              ; System.Collections.Generic.List<UInt32Enum>$$set_Item
020daf4f: xor    r9d, r9d
020daf52: lea    rax, [rsp + 0x98]
020daf5a: mov    qword ptr [rsp + 0x28], 0
020daf63: mov    r8, rbp
020daf66: mov    qword ptr [rsp + 0x20], rax
020daf6b: mov    rdx, r13
020daf6e: mov    rcx, r15
020daf71: call   0x1820cf2b0
020daf76: mov    eax, dword ptr [rbx + 0x1c]
020daf79: xorps  xmm0, xmm0
020daf7c: movsd  xmm1, qword ptr [r13 + 0xb8]
020daf85: mulsd  xmm1, xmm6
020daf89: cvtsi2sd xmm0, rax
020daf8e: divsd  xmm1, xmm0
020daf92: cvttsd2si rax, xmm1
020daf97: add    rbp, rax
020daf9a: cmp    rbp, qword ptr [rsp + 0xa8]
020dafa2: jg     0x1820dafb3
020dafa4: inc    r14d
020dafa7: lea    r12d, [rsi + 1]
020dafab: mov    rdi, rbx
020dafae: jmp    0x1820dad60
020dafb3: mov    rax, qword ptr [r15 + 0x20]
020dafb7: test   rax, rax
020dafba: je     0x1820db006
020dafbc: mov    eax, dword ptr [rax + 0x18]
020dafbf: dec    eax
020dafc1: mov    dword ptr [r15 + 0x30], eax
020dafc5: mov    rax, qword ptr [r15 + 0x10]
020dafc9: test   rax, rax
020dafcc: je     0x1820db006
020dafce: mov    eax, dword ptr [rax + 0x18]
020dafd1: movaps xmm6, xmmword ptr [rsp + 0x40]
020dafd6: dec    eax
020dafd8: mov    r14, qword ptr [rsp + 0x50]
020dafdd: mov    r12, qword ptr [rsp + 0x58]
020dafe2: mov    rdi, qword ptr [rsp + 0x60]
020dafe7: mov    rsi, qword ptr [rsp + 0x68]
020dafec: mov    rbp, qword ptr [rsp + 0x70]
020daff1: mov    rbx, qword ptr [rsp + 0x90]
020daff9: mov    dword ptr [r15 + 0x38], eax
020daffd: add    rsp, 0x78
020db001: pop    r15
020db003: pop    r13
020db005: ret    
020db006: call   0x182f60c50
020db00b: int3   
020db00c: call   0x182f60c40
020db011: int3   
020db012: int3   
