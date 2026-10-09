0213d620: mov    qword ptr [rsp + 0x18], rbx
0213d625: push   rbp
0213d626: push   rsi
0213d627: push   rdi
0213d628: push   r13
0213d62a: push   r14
0213d62c: mov    rbp, rsp
0213d62f: sub    rsp, 0x60
0213d633: cmp    byte ptr [rip + 0x1ddc023], 0            ; [0x3f1965d] (bss)
0213d63a: mov    rbx, r9
0213d63d: mov    r13, r8
0213d640: mov    r14, rdx
0213d643: mov    rdi, rcx
0213d646: jne    0x18213d65b
0213d648: lea    rcx, [rip + 0x1b9a0d1]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0213d64f: call   0x182f609b0
0213d654: mov    byte ptr [rip + 0x1ddc002], 1            ; [0x3f1965d] (bss)
0213d65b: movups xmm0, xmmword ptr [rdi]
0213d65e: mov    qword ptr [rsp + 0x90], r12
0213d666: lea    r8, [rbx + 8]
0213d66a: xor    r9d, r9d
0213d66d: mov    qword ptr [rsp + 0x98], r15
0213d675: lea    rdx, [rbp - 0x40]
0213d679: movaps xmmword ptr [rbp - 0x40], xmm0
0213d67d: lea    rcx, [rbp - 0x20]
0213d681: call   0x18213db70                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʸʾʺʵˁʳʵʵʺʸʹ
0213d686: cmp    byte ptr [rbx + 0xc], 0
0213d68a: mov    esi, dword ptr [rax]
0213d68c: je     0x18213d6fb
0213d68e: test   r14, r14
0213d691: je     0x18213dad5
0213d697: mov    rcx, qword ptr [r14 + 0x60]
0213d69b: test   rcx, rcx
0213d69e: je     0x18213dad5
0213d6a4: cmp    dword ptr [rcx + 0x18], 1
0213d6a8: jle    0x18213d6fb
0213d6aa: xor    edx, edx
0213d6ac: mov    rcx, r14
0213d6af: call   0x18215fe80                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʿʷˁʹʹʴʵˁʴʸʼ
0213d6b4: test   al, al
0213d6b6: je     0x18213d6c5
0213d6b8: cmp    esi, 5
0213d6bb: jne    0x18213d6e9
0213d6bd: mov    byte ptr [r14 + 0xa1], 1
0213d6c5: mov    r15, qword ptr [rsp + 0x98]
0213d6cd: mov    r12, qword ptr [rsp + 0x90]
0213d6d5: mov    rbx, qword ptr [rsp + 0xa0]
0213d6dd: add    rsp, 0x60
0213d6e1: pop    r14
0213d6e3: pop    r13
0213d6e5: pop    rdi
0213d6e6: pop    rsi
0213d6e7: pop    rbp
0213d6e8: ret    
0213d6e9: lea    eax, [rsi - 0x42]
0213d6ec: cmp    eax, 2
0213d6ef: ja     0x18213d6c5
0213d6f1: mov    byte ptr [r14 + 0xa2], 1
0213d6f9: jmp    0x18213d6c5
0213d6fb: mov    r12, qword ptr [rax + 8]
0213d6ff: lea    rdx, [rip - 0x213d706]                   ; [0x0] (bss)
0213d706: movsx  eax, byte ptr [rbx + 0xd]
0213d70a: xor    edi, edi
0213d70c: add    eax, -4
0213d70f: cmp    eax, 0xa
0213d712: ja     0x18213d90f
0213d718: cdqe   
0213d71a: mov    ecx, dword ptr [rdx + rax*4 + 0x213dae4]
0213d721: add    rcx, rdx
0213d724: jmp    rcx
0213d726: cmp    esi, 5
0213d729: ja     0x18213d76f
0213d72b: mov    eax, dword ptr [rdx + rsi*4 + 0x213db10]
0213d732: add    rax, rdx
0213d735: jmp    rax
0213d737: mov    eax, 0xd
0213d73c: mov    ebx, eax
0213d73e: jmp    0x18213d77d
0213d740: mov    eax, 0xe
0213d745: mov    ebx, eax
0213d747: jmp    0x18213d778
0213d749: mov    eax, 0xf
0213d74e: mov    ebx, eax
0213d750: jmp    0x18213d778
0213d752: mov    edi, 0x10
0213d757: mov    eax, edi
0213d759: mov    ebx, edi
0213d75b: jmp    0x18213d77d
0213d75d: mov    eax, 0x12
0213d762: mov    ebx, eax
0213d764: jmp    0x18213d778
0213d766: mov    eax, 0x11
0213d76b: mov    ebx, eax
0213d76d: jmp    0x18213d778
0213d76f: cmp    esi, 0x20
0213d772: je     0x18213d799
0213d774: xor    eax, eax
0213d776: xor    ebx, ebx
0213d778: mov    edi, 0x10
0213d77d: cmp    eax, 0x11
0213d780: jne    0x18213d94c
0213d786: test   r14, r14
0213d789: je     0x18213dad5
0213d78f: mov    byte ptr [r14 + 0xa1], 1
0213d797: jmp    0x18213d7ac
0213d799: mov    edi, 8
0213d79e: mov    ebx, 0xd
0213d7a3: test   r14, r14
0213d7a6: je     0x18213dad5
0213d7ac: cmp    byte ptr [rip + 0x1ddbb6b], 0            ; [0x3f1931e] (bss)
0213d7b3: jne    0x18213d7e0
0213d7b5: lea    rcx, [rip + 0x1b99d24]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
0213d7bc: call   0x182f609b0
0213d7c1: lea    rcx, [rip + 0x1b99f58]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0213d7c8: call   0x182f609b0
0213d7cd: lea    rcx, [rip + 0x1bbd3dc]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213d7d4: call   0x182f609b0
0213d7d9: mov    byte ptr [rip + 0x1ddbb3e], 1            ; [0x3f1931e] (bss)
0213d7e0: mov    rax, qword ptr [r14 + 0x50]
0213d7e4: test   rax, rax
0213d7e7: je     0x18213dad5
0213d7ed: movsxd rax, dword ptr [rax + 0x114]
0213d7f4: xor    r15d, r15d
0213d7f7: mov    rcx, qword ptr [r14 + 0x10]
0213d7fb: cmp    r12, rax
0213d7fe: cmovg  r15, r12
0213d802: test   rcx, rcx
0213d805: je     0x18213dad5
0213d80b: mov    rdx, qword ptr [rip + 0x1bbd39e]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213d812: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0213d817: mov    rsi, rax
0213d81a: test   rax, rax
0213d81d: je     0x18213dad5
0213d823: cmp    byte ptr [rip + 0x1ddbde5], 0            ; [0x3f1960f] (bss)
0213d82a: jne    0x18213d83f
0213d82c: lea    rcx, [rip + 0x1b94ecd]                   ; [0x3cd2700] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Clear()
0213d833: call   0x182f609b0
0213d838: mov    byte ptr [rip + 0x1ddbdd0], 1            ; [0x3f1960f] (bss)
0213d83f: mov    rcx, qword ptr [rsi + 0x38]
0213d843: xor    edx, edx
0213d845: mov    qword ptr [rsi + 0x10], r13
0213d849: mov    qword ptr [rsi + 0x18], r15
0213d84d: mov    dword ptr [rsi + 0x24], edi
0213d850: mov    dword ptr [rsi + 0x20], ebx
0213d853: mov    word ptr [rsi + 0x34], 0
0213d859: mov    byte ptr [rsi + 0x36], 0
0213d85d: mov    qword ptr [rsi + 0x28], rdx
0213d861: mov    dword ptr [rsi + 0x30], edx
0213d864: test   rcx, rcx
0213d867: je     0x18213dad5
0213d86d: inc    dword ptr [rcx + 0x1c]
0213d870: mov    dword ptr [rcx + 0x18], edx
0213d873: mov    rax, qword ptr [r14 + 0x60]
0213d877: test   rax, rax
0213d87a: je     0x18213dad5
0213d880: mov    eax, dword ptr [rax + 0x18]
0213d883: mov    dword ptr [rsi + 0x28], eax
0213d886: mov    rcx, qword ptr [r14 + 0x60]
0213d88a: test   rcx, rcx
0213d88d: je     0x18213dad5
0213d893: mov    r9, qword ptr [rip + 0x1b99c46]          ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
0213d89a: inc    dword ptr [rcx + 0x1c]
0213d89d: mov    rdx, qword ptr [rcx + 0x10]
0213d8a1: test   rdx, rdx
0213d8a4: je     0x18213dad5
0213d8aa: movsxd r8, dword ptr [rcx + 0x18]
0213d8ae: cmp    r8d, dword ptr [rdx + 0x18]
0213d8b2: jb     0x18213daae
0213d8b8: mov    rax, qword ptr [r9 + 0x20]
0213d8bc: mov    rdx, rsi
0213d8bf: mov    r8, qword ptr [rax + 0xc0]
0213d8c6: mov    r8, qword ptr [r8 + 0x70]
0213d8ca: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0213d8cf: jmp    0x18213d6c5
0213d8d4: cmp    esi, 8
0213d8d7: ja     0x18213d94a
0213d8d9: mov    eax, dword ptr [rdx + rsi*4 + 0x213db28]
0213d8e0: add    rax, rdx
0213d8e3: jmp    rax
0213d8e5: mov    ebx, 0xa
0213d8ea: jmp    0x18213d94c
0213d8ec: mov    ebx, 0xb
0213d8f1: jmp    0x18213d94c
0213d8f3: mov    ebx, 0xc
0213d8f8: jmp    0x18213d94c
0213d8fa: mov    ebx, 7
0213d8ff: jmp    0x18213d94c
0213d901: mov    ebx, 8
0213d906: jmp    0x18213d94c
0213d908: mov    ebx, 9
0213d90d: jmp    0x18213d94c
0213d90f: cmp    esi, 7
0213d912: ja     0x18213d94a
0213d914: mov    eax, dword ptr [rdx + rsi*4 + 0x213db4c]
0213d91b: add    rax, rdx
0213d91e: jmp    rax
0213d920: mov    ebx, 2
0213d925: jmp    0x18213d94c
0213d927: mov    ebx, 3
0213d92c: jmp    0x18213d94c
0213d92e: mov    ebx, 4
0213d933: jmp    0x18213d94c
0213d935: mov    ebx, 5
0213d93a: jmp    0x18213d94c
0213d93c: mov    ebx, 6
0213d941: jmp    0x18213d94c
0213d943: mov    ebx, 1
0213d948: jmp    0x18213d94c
0213d94a: xor    ebx, ebx
0213d94c: test   ebx, ebx
0213d94e: jne    0x18213d7a3
0213d954: xor    edx, edx
0213d956: mov    ecx, esi
0213d958: call   0x18213d160                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʷʷʴʵʲˀʲˀʶʻʳ
0213d95d: mov    rbx, rax
0213d960: test   al, al
0213d962: je     0x18213d6c5
0213d968: cmp    bl, 7
0213d96b: jne    0x18213d97e
0213d96d: test   r14, r14
0213d970: je     0x18213dad5
0213d976: mov    byte ptr [r14 + 0xa2], 1
0213d97e: test   r14, r14
0213d981: je     0x18213dad5
0213d987: movzx  esi, byte ptr [r14 + 0x59]
0213d98c: xorps  xmm0, xmm0
0213d98f: mov    rdi, rbx
0213d992: shr    rdi, 0x20
0213d996: cmp    byte ptr [rip + 0x1ddb986], 0            ; [0x3f19323] (bss)
0213d99d: movups xmmword ptr [rbp - 0x40], xmm0
0213d9a1: movups xmmword ptr [rbp - 0x30], xmm0
0213d9a5: jne    0x18213d9ba
0213d9a7: lea    rcx, [rip + 0x1b94bd2]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0213d9ae: call   0x182f609b0
0213d9b3: mov    byte ptr [rip + 0x1ddb969], 1            ; [0x3f19323] (bss)
0213d9ba: xor    r8d, r8d
0213d9bd: movzx  edx, bl
0213d9c0: mov    rcx, r14
0213d9c3: call   0x182160210                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʽʿʳʳʲʾʷʹʲʴ
0213d9c8: mov    r9, rax
0213d9cb: test   rax, rax
0213d9ce: je     0x18213dad5
0213d9d4: mov    r8, qword ptr [rip + 0x1b94ba5]          ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0213d9db: inc    dword ptr [rax + 0x1c]
0213d9de: mov    rcx, qword ptr [rax + 0x10]
0213d9e2: test   rcx, rcx
0213d9e5: je     0x18213dad5
0213d9eb: movzx  eax, word ptr [rbp - 0x2f]
0213d9ef: movsxd rdx, dword ptr [r9 + 0x18]
0213d9f3: mov    word ptr [rbp - 0xf], ax
0213d9f7: movzx  eax, byte ptr [rbp - 0x2d]
0213d9fb: mov    byte ptr [rbp - 0xd], al
0213d9fe: mov    qword ptr [rbp - 0x20], r13
0213da02: mov    qword ptr [rbp - 0x18], r13
0213da06: mov    byte ptr [rbp - 0x10], sil
0213da0a: mov    dword ptr [rbp - 0xc], edi
0213da0d: cmp    edx, dword ptr [rcx + 0x18]
0213da10: jb     0x18213da63
0213da12: mov    eax, dword ptr [rbp - 0x27]
0213da15: lea    rdx, [rbp - 0x40]
0213da19: movaps xmm0, xmmword ptr [rbp - 0x20]
0213da1d: mov    rcx, r9
