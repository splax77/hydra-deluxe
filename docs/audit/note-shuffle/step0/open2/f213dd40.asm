0213dd40: mov    qword ptr [rsp + 0x18], rbx
0213dd45: push   rbp
0213dd46: push   rsi
0213dd47: push   rdi
0213dd48: push   r13
0213dd4a: push   r15
0213dd4c: mov    rbp, rsp
0213dd4f: sub    rsp, 0x80
0213dd56: cmp    byte ptr [rip + 0x1ddb904], 0            ; [0x3f19661] (bss)
0213dd5d: movzx  esi, r9b
0213dd61: mov    ebx, r8d
0213dd64: mov    r15, rdx
0213dd67: mov    rdi, rcx
0213dd6a: jne    0x18213ddc7
0213dd6c: lea    rcx, [rip + 0x1b93d8d]                   ; [0x3cd1b00] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.RemoveAll()
0213dd73: call   0x182f609b0
0213dd78: lea    rcx, [rip + 0x1b93fc1]                   ; [0x3cd1d40] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.set_Item()
0213dd7f: call   0x182f609b0
0213dd84: lea    rcx, [rip + 0x1bc2455]                   ; [0x3d001e0] meta:System.Predicate<ʸʵʵʾʿˀʺʽʲʾˁ>_TypeInfo
0213dd8b: call   0x182f609b0
0213dd90: lea    rcx, [rip + 0x1bb5349]                   ; [0x3cf30e0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.Dispose()
0213dd97: call   0x182f609b0
0213dd9c: lea    rcx, [rip + 0x1bb54bd]                   ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213dda3: call   0x182f609b0
0213dda8: lea    rcx, [rip + 0x1b9a201]                   ; [0x3cd7fb0] metamethod:Method$ʷʹʸʲʶʳʾˁʾʷʶ.<>c.ʻʿʿʲʽʻʵʼʹʲʺ()
0213ddaf: call   0x182f609b0
0213ddb4: lea    rcx, [rip + 0x1bc4d55]                   ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213ddbb: call   0x182f609b0
0213ddc0: mov    byte ptr [rip + 0x1ddb89a], 1            ; [0x3f19661] (bss)
0213ddc7: xorps  xmm0, xmm0
0213ddca: mov    qword ptr [rsp + 0xb0], r12
0213ddd2: xor    eax, eax
0213ddd4: mov    qword ptr [rsp + 0xb8], r14
0213dddc: movups xmmword ptr [rbp - 0x40], xmm0
0213dde0: xor    r13d, r13d
0213dde3: mov    byte ptr [rbp - 0x33], sil
0213dde7: mov    rsi, qword ptr [rbp + 0x50]
0213ddeb: mov    qword ptr [rbp - 0x10], rax
0213ddef: movups xmmword ptr [rbp - 0x30], xmm0
0213ddf3: mov    dword ptr [rbp + 0x48], r13d
0213ddf7: mov    rax, qword ptr [rsi]
0213ddfa: mov    dword ptr [rbp - 0x38], ebx
0213ddfd: movups xmmword ptr [rbp - 0x20], xmm0
0213de01: test   rax, rax
0213de04: je     0x18213e02c
0213de0a: movzx  eax, byte ptr [rax + 0x2c]
0213de0e: mov    byte ptr [rbp - 0x34], al
0213de11: mov    rax, qword ptr [rsi]
0213de14: test   rax, rax
0213de17: je     0x18213e02c
0213de1d: test   rdi, rdi
0213de20: je     0x18213e02c
0213de26: movzx  eax, byte ptr [rax + 0x10]
0213de2a: mov    byte ptr [rdi + 0xa1], al
0213de30: mov    rax, qword ptr [rsi]
0213de33: test   rax, rax
0213de36: je     0x18213e02c
0213de3c: movzx  eax, byte ptr [rax + 0x11]
0213de40: mov    r14d, dword ptr [r15 + 8]
0213de44: mov    r12, qword ptr [r15]
0213de47: mov    byte ptr [rdi + 0xa2], al
0213de4d: xor    eax, eax
0213de4f: mov    dword ptr [rbp - 0x12], eax
0213de52: mov    eax, ebx
0213de54: mov    qword ptr [rbp - 0x40], r13
0213de58: movups xmmword ptr [rbp - 0x32], xmm0
0213de5c: movups xmmword ptr [rbp - 0x22], xmm0
0213de60: cmp    ebx, r14d
0213de63: jae    0x18213e032
0213de69: nop    dword ptr [rax]
0213de70: cdqe   
0213de72: movzx  edx, word ptr [r12 + rax*2]
0213de77: mov    eax, edx
0213de79: sub    eax, 9
0213de7c: je     0x18213deca
0213de7e: sub    eax, 1
0213de81: je     0x18213deca
0213de83: sub    eax, 1
0213de86: je     0x18213de9c
0213de88: sub    eax, 1
0213de8b: je     0x18213de9c
0213de8d: cmp    eax, 1
0213de90: je     0x18213deca
0213de92: cmp    edx, 0x20
0213de95: je     0x18213deca
0213de97: cmp    edx, 0x7d
0213de9a: je     0x18213dedc
0213de9c: movups xmm0, xmmword ptr [r15]
0213dea0: lea    rax, [rbp - 0x40]
0213dea4: mov    qword ptr [rsp + 0x28], r13
0213dea9: mov    r9, rsi
0213deac: mov    qword ptr [rsp + 0x20], rax
0213deb1: mov    r8, rdi
0213deb4: movaps xmmword ptr [rbp - 0x50], xmm0
0213deb8: lea    rdx, [rbp - 0x38]
0213debc: lea    rcx, [rbp - 0x50]
0213dec0: call   0x18213d2d0                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʷʷʴʿʿʷʸʺˀʲʾ
0213dec5: mov    ebx, dword ptr [rbp - 0x38]
0213dec8: jmp    0x18213decf
0213deca: inc    ebx
0213decc: mov    dword ptr [rbp - 0x38], ebx
0213decf: mov    eax, ebx
0213ded1: cmp    ebx, r14d
0213ded4: jae    0x18213e032
0213deda: jmp    0x18213de70
0213dedc: inc    ebx
0213dede: mov    dword ptr [rbp - 0x38], ebx
0213dee1: cmp    byte ptr [rbp - 0x34], r13b
0213dee5: jne    0x18213e003
0213deeb: mov    r8, qword ptr [rip + 0x1bb536e]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213def2: lea    rdx, [rbp + 0x48]
0213def6: lea    rcx, [rbp - 0x32]
0213defa: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213deff: test   al, al
0213df01: je     0x18213df3a
0213df03: mov    rcx, qword ptr [rdi + 0x78]
0213df07: test   rcx, rcx
0213df0a: je     0x18213e02c
0213df10: mov    r9, qword ptr [rip + 0x1b93e29]          ; [0x3cd1d40] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.set_Item()
0213df17: xor    r8d, r8d
0213df1a: mov    edx, dword ptr [rbp + 0x48]
0213df1d: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
0213df22: mov    r8, qword ptr [rip + 0x1bb5337]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213df29: lea    rdx, [rbp + 0x48]
0213df2d: lea    rcx, [rbp - 0x32]
0213df31: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213df36: test   al, al
0213df38: jne    0x18213df03
0213df3a: mov    rcx, qword ptr [rip + 0x1bc4bcf]         ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213df41: mov    rsi, qword ptr [rdi + 0x78]
0213df45: cmp    dword ptr [rcx + 0xe0], r13d
0213df4c: jne    0x18213df5a
0213df4e: call   0x182f60cf0
0213df53: mov    rcx, qword ptr [rip + 0x1bc4bb6]         ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213df5a: mov    rax, qword ptr [rcx + 0xb8]
0213df61: mov    rdi, qword ptr [rax + 8]
0213df65: test   rdi, rdi
0213df68: jne    0x18213dfd9
0213df6a: cmp    dword ptr [rcx + 0xe0], r13d
0213df71: jne    0x18213df7f
0213df73: call   0x182f60cf0
0213df78: mov    rcx, qword ptr [rip + 0x1bc4b91]         ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213df7f: mov    rax, qword ptr [rcx + 0xb8]
0213df86: mov    rcx, qword ptr [rip + 0x1bc2253]         ; [0x3d001e0] meta:System.Predicate<ʸʵʵʾʿˀʺʽʲʾˁ>_TypeInfo
0213df8d: mov    rbx, qword ptr [rax]
0213df90: call   0x182f60c00
0213df95: mov    r8, qword ptr [rip + 0x1b9a014]          ; [0x3cd7fb0] metamethod:Method$ʷʹʸʲʶʳʾˁʾʷʶ.<>c.ʻʿʿʲʽʻʵʼʹʲʺ()
0213df9c: xor    r9d, r9d
0213df9f: mov    rdx, rbx
0213dfa2: mov    rcx, rax
0213dfa5: mov    rdi, rax
0213dfa8: call   0x1808ae670                              ; Grpc.Core.VerifyPeerCallback$$.ctor
0213dfad: mov    rax, qword ptr [rip + 0x1bc4b5c]         ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213dfb4: mov    rdx, rdi
0213dfb7: mov    rcx, qword ptr [rax + 0xb8]
0213dfbe: mov    qword ptr [rcx + 8], rdi
0213dfc2: mov    rax, qword ptr [rip + 0x1bc4b47]         ; [0x3d02b10] meta:ʷʹʸʲʶʳʾˁʾʷʶ.<>c_TypeInfo
0213dfc9: mov    rcx, qword ptr [rax + 0xb8]
0213dfd0: add    rcx, 8
0213dfd4: call   0x182f5fc00
0213dfd9: test   rsi, rsi
0213dfdc: je     0x18213e02c
0213dfde: mov    r8, qword ptr [rip + 0x1b93b1b]          ; [0x3cd1b00] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.RemoveAll()
0213dfe5: mov    rdx, rdi
0213dfe8: mov    rcx, rsi
0213dfeb: call   0x1807747e0                              ; System.Collections.Generic.List<object>$$RemoveAll
0213dff0: mov    rdx, qword ptr [rip + 0x1bb50e9]         ; [0x3cf30e0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.Dispose()
0213dff7: lea    rcx, [rbp - 0x32]
0213dffb: call   0x180f9da60                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ˁˀʺʳʼˀʳʴˁʽʷ
0213e000: mov    ebx, dword ptr [rbp - 0x38]
0213e003: mov    r14, qword ptr [rsp + 0xb8]
0213e00b: mov    eax, ebx
0213e00d: mov    rbx, qword ptr [rsp + 0xc0]
0213e015: mov    r12, qword ptr [rsp + 0xb0]
0213e01d: add    rsp, 0x80
0213e024: pop    r15
0213e026: pop    r13
0213e028: pop    rdi
0213e029: pop    rsi
0213e02a: pop    rbp
0213e02b: ret    
0213e02c: call   0x182f60c50
0213e031: int3   
0213e032: call   0x182f60c40
0213e037: int3   
0213e038: int3   
