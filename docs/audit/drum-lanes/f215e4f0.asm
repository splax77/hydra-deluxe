0215e4f0: push   rbx
0215e4f2: push   rbp
0215e4f3: push   rsi
0215e4f4: push   rdi
0215e4f5: sub    rsp, 0x38
0215e4f9: cmp    byte ptr [rip + 0x1dbb258], 0            ; [0x3f19758] (bss)
0215e500: movzx  ebx, r9b
0215e504: mov    rdi, r8
0215e507: mov    rbp, rdx
0215e50a: mov    rsi, rcx
0215e50d: jne    0x18215e552
0215e50f: lea    rcx, [rip + 0x1b6fcea]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
0215e516: call   0x182f609b0
0215e51b: lea    rcx, [rip + 0x1b6ff1e]                   ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
0215e522: call   0x182f609b0
0215e527: lea    rcx, [rip + 0x1b6ffd2]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
0215e52e: call   0x182f609b0
0215e533: lea    rcx, [rip + 0x1b70086]                   ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215e53a: call   0x182f609b0
0215e53f: lea    rcx, [rip + 0x1b9a0fa]                   ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215e546: call   0x182f609b0
0215e54b: mov    byte ptr [rip + 0x1dbb206], 1            ; [0x3f19758] (bss)
0215e552: mov    rcx, qword ptr [rsi + 0x38]
0215e556: test   rcx, rcx
0215e559: je     0x18215e6d2
0215e55f: mov    rdx, qword ptr [rip + 0x1b9a0da]         ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215e566: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0215e56b: test   rax, rax
0215e56e: je     0x18215e6d2
0215e574: movzx  ecx, byte ptr [rsp + 0x80]
0215e57c: movzx  r9d, bl
0215e580: mov    qword ptr [rsp + 0x28], 0
0215e589: mov    r8, rdi
0215e58c: mov    byte ptr [rsp + 0x20], cl
0215e590: mov    rdx, rbp
0215e593: mov    rcx, rax
0215e596: call   0x182116390                              ; ʳʷʸʽʳʹʶˁʺʳʿ$$ʾʵʺʼʳʾʴˀʷʷʳ
0215e59b: mov    rdi, rax
0215e59e: test   rax, rax
0215e5a1: je     0x18215e6d2
0215e5a7: cmp    byte ptr [rax + 0x21], 0
0215e5ab: jne    0x18215e660
0215e5b1: mov    rbx, qword ptr [rsi + 0x80]
0215e5b8: test   rbx, rbx
0215e5bb: je     0x18215e6d2
0215e5c1: mov    ebx, dword ptr [rbx + 0x18]
0215e5c4: dec    ebx
0215e5c6: cmp    ebx, 1
0215e5c9: jl     0x18215e660
0215e5cf: mov    r8, qword ptr [rip + 0x1b6ffea]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215e5d6: mov    edx, ebx
0215e5d8: mov    rcx, qword ptr [rsi + 0x80]
0215e5df: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215e5e4: test   rax, rax
0215e5e7: je     0x18215e6d2
0215e5ed: mov    rcx, qword ptr [rdi + 0x10]
0215e5f1: cmp    qword ptr [rax + 0x10], rcx
0215e5f5: jne    0x18215e660
0215e5f7: cmp    byte ptr [rax + 0x21], 0
0215e5fb: je     0x18215e660
0215e5fd: nop    dword ptr [rax]
0215e600: mov    rcx, qword ptr [rsi + 0x80]
0215e607: test   rcx, rcx
0215e60a: je     0x18215e6d2
0215e610: mov    r8, qword ptr [rip + 0x1b6ffa9]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215e617: lea    edx, [rbx - 1]
0215e61a: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215e61f: test   rax, rax
0215e622: je     0x18215e6d2
0215e628: mov    rcx, qword ptr [rdi + 0x10]
0215e62c: cmp    qword ptr [rax + 0x10], rcx
0215e630: jne    0x18215e637
0215e632: sub    ebx, 1
0215e635: jne    0x18215e600
0215e637: mov    rcx, qword ptr [rsi + 0x80]
0215e63e: test   rcx, rcx
0215e641: je     0x18215e6d2
0215e647: mov    r9, qword ptr [rip + 0x1b6fdf2]          ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
0215e64e: mov    r8, rdi
0215e651: mov    edx, ebx
0215e653: add    rsp, 0x38
0215e657: pop    rdi
0215e658: pop    rsi
0215e659: pop    rbp
0215e65a: pop    rbx
0215e65b: jmp    0x1807746c0                              ; System.Collections.Generic.List<object>$$Insert
0215e660: mov    rcx, qword ptr [rsi + 0x80]
0215e667: test   rcx, rcx
0215e66a: je     0x18215e6d2
0215e66c: mov    r9, qword ptr [rip + 0x1b6fb8d]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
0215e673: inc    dword ptr [rcx + 0x1c]
0215e676: mov    rdx, qword ptr [rcx + 0x10]
0215e67a: test   rdx, rdx
0215e67d: je     0x18215e6d2
0215e67f: movsxd r8, dword ptr [rcx + 0x18]
0215e683: cmp    r8d, dword ptr [rdx + 0x18]
0215e687: jb     0x18215e6a8
0215e689: mov    rax, qword ptr [r9 + 0x20]
0215e68d: mov    rdx, rdi
0215e690: mov    r8, qword ptr [rax + 0xc0]
0215e697: mov    r8, qword ptr [r8 + 0x70]
0215e69b: add    rsp, 0x38
0215e69f: pop    rdi
0215e6a0: pop    rsi
0215e6a1: pop    rbp
0215e6a2: pop    rbx
0215e6a3: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0215e6a8: lea    eax, [r8 + 1]
0215e6ac: mov    dword ptr [rcx + 0x18], eax
0215e6af: cmp    r8d, dword ptr [rdx + 0x18]
0215e6b3: jae    0x18215e6d8
0215e6b5: mov    qword ptr [rdx + r8*8 + 0x20], rdi
0215e6ba: add    rdx, 0x20
0215e6be: lea    rcx, [rdx + r8*8]
0215e6c2: mov    rdx, rdi
0215e6c5: add    rsp, 0x38
0215e6c9: pop    rdi
0215e6ca: pop    rsi
0215e6cb: pop    rbp
0215e6cc: pop    rbx
0215e6cd: jmp    0x182f5fc00
0215e6d2: call   0x182f60c50
0215e6d7: int3   
0215e6d8: call   0x182f60c40
0215e6dd: int3   
0215e6de: int3   
