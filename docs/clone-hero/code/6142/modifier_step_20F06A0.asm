020f06a0: mov    qword ptr [rsp + 0x18], rbp
020f06a5: push   rsi
020f06a6: push   rdi
020f06a7: push   r14
020f06a9: sub    rsp, 0x20
020f06ad: cmp    byte ptr [rip + 0x1e28cc5], 0            ; [0x3f19379] (bss)
020f06b4: mov    rsi, rcx
020f06b7: jne    0x1820f0714
020f06b9: lea    rcx, [rip + 0x1be69a0]                   ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020f06c0: call   0x182f609b0
020f06c5: lea    rcx, [rip + 0x1be6b14]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f06cc: call   0x182f609b0
020f06d1: lea    rcx, [rip + 0x1be6bc8]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f06d8: call   0x182f609b0
020f06dd: lea    rcx, [rip + 0x1be6c7c]                   ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020f06e4: call   0x182f609b0
020f06e9: lea    rcx, [rip + 0x1c0fc68]                   ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020f06f0: call   0x182f609b0
020f06f5: lea    rcx, [rip + 0x1be90b4]                   ; [0x3cd97b0] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ˁʾʴʹʿʹʿʻʵʽʴ()
020f06fc: call   0x182f609b0
020f0701: lea    rcx, [rip + 0x1c14de0]                   ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0708: call   0x182f609b0
020f070d: mov    byte ptr [rip + 0x1e28c65], 1            ; [0x3f19379] (bss)
020f0714: xor    r14d, r14d
020f0717: mov    qword ptr [rsp + 0x40], rbx
020f071c: mov    qword ptr [rsp + 0x48], r15
020f0721: mov    ebp, r14d
020f0724: mov    edi, r14d
020f0727: mov    eax, r14d
020f072a: test   rsi, rsi
020f072d: je     0x1820f088b
020f0733: cmp    eax, dword ptr [rsi + 0x18]
020f0736: jge    0x1820f07c7
020f073c: mov    r8, qword ptr [rip + 0x1be6b5d]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0743: mov    edx, edi
020f0745: mov    rcx, rsi
020f0748: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f074d: mov    rbx, rax
020f0750: test   rax, rax
020f0753: je     0x1820f088b
020f0759: cmp    rbp, qword ptr [rax + 0x40]
020f075d: jne    0x1820f077d
020f075f: test   edi, edi
020f0761: je     0x1820f077d
020f0763: mov    r9, qword ptr [rip + 0x1be6bf6]          ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020f076a: xor    r8d, r8d
020f076d: mov    edx, edi
020f076f: mov    rcx, rsi
020f0772: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
020f0777: inc    edi
020f0779: mov    eax, edi
020f077b: jmp    0x1820f0733
020f077d: cmp    word ptr [rax + 0x80], 1
020f0785: je     0x1820f0790
020f0787: mov    word ptr [rax + 0x80], 1
020f0790: mov    rbp, qword ptr [rax + 0x40]
020f0794: cmp    qword ptr [rax + 0x18], r14
020f0798: je     0x1820f07a9
020f079a: lea    rcx, [rax + 0x18]
020f079e: mov    qword ptr [rax + 0x18], r14
020f07a2: xor    edx, edx
020f07a4: call   0x182f5fc00
020f07a9: cmp    qword ptr [rbx + 0x10], r14
020f07ad: je     0x1820f07be
020f07af: lea    rcx, [rbx + 0x10]
020f07b3: mov    qword ptr [rbx + 0x10], r14
020f07b7: xor    edx, edx
020f07b9: call   0x182f5fc00
020f07be: inc    edi
020f07c0: mov    eax, edi
020f07c2: jmp    0x1820f0733
020f07c7: mov    rcx, qword ptr [rip + 0x1c14d1a]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f07ce: cmp    dword ptr [rcx + 0xe0], r14d
020f07d5: jne    0x1820f07e3
020f07d7: call   0x182f60cf0
020f07dc: mov    rcx, qword ptr [rip + 0x1c14d05]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f07e3: mov    rax, qword ptr [rcx + 0xb8]
020f07ea: mov    rdi, qword ptr [rax + 0x18]
020f07ee: test   rdi, rdi
020f07f1: jne    0x1820f0862
020f07f3: cmp    dword ptr [rcx + 0xe0], r14d
020f07fa: jne    0x1820f0808
020f07fc: call   0x182f60cf0
020f0801: mov    rcx, qword ptr [rip + 0x1c14ce0]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0808: mov    rax, qword ptr [rcx + 0xb8]
020f080f: mov    rcx, qword ptr [rip + 0x1c0fb42]         ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020f0816: mov    rbx, qword ptr [rax]
020f0819: call   0x182f60c00
020f081e: mov    r8, qword ptr [rip + 0x1be8f8b]          ; [0x3cd97b0] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ˁʾʴʹʿʹʿʻʵʽʴ()
020f0825: xor    r9d, r9d
020f0828: mov    rdx, rbx
020f082b: mov    rcx, rax
020f082e: mov    rdi, rax
020f0831: call   0x1808ae670                              ; Grpc.Core.VerifyPeerCallback$$.ctor
020f0836: mov    rax, qword ptr [rip + 0x1c14cab]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f083d: mov    rdx, rdi
020f0840: mov    rcx, qword ptr [rax + 0xb8]
020f0847: mov    qword ptr [rcx + 0x18], rdi
020f084b: mov    rax, qword ptr [rip + 0x1c14c96]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0852: mov    rcx, qword ptr [rax + 0xb8]
020f0859: add    rcx, 0x18
020f085d: call   0x182f5fc00
020f0862: mov    r8, qword ptr [rip + 0x1be67f7]          ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020f0869: mov    rdx, rdi
020f086c: mov    rcx, rsi
020f086f: mov    r15, qword ptr [rsp + 0x48]
020f0874: mov    rbx, qword ptr [rsp + 0x40]
020f0879: mov    rbp, qword ptr [rsp + 0x50]
020f087e: add    rsp, 0x20
020f0882: pop    r14
020f0884: pop    rdi
020f0885: pop    rsi
020f0886: jmp    0x1807747e0                              ; System.Collections.Generic.List<object>$$RemoveAll
020f088b: call   0x182f60c50
020f0890: int3   
020f0891: int3   
