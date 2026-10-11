0215f860: push   rbx
0215f862: push   rbp
0215f863: push   rsi
0215f864: push   rdi
0215f865: sub    rsp, 0x38
0215f869: cmp    byte ptr [rip + 0x1db9ecb], 0            ; [0x3f1973b] (bss)
0215f870: movzx  ebx, r9b
0215f874: mov    rdi, r8
0215f877: mov    rbp, rdx
0215f87a: mov    rsi, rcx
0215f87d: jne    0x18215f8c2
0215f87f: lea    rcx, [rip + 0x1b6e97a]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
0215f886: call   0x182f609b0
0215f88b: lea    rcx, [rip + 0x1b6ebae]                   ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
0215f892: call   0x182f609b0
0215f897: lea    rcx, [rip + 0x1b6ec62]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
0215f89e: call   0x182f609b0
0215f8a3: lea    rcx, [rip + 0x1b6ed16]                   ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215f8aa: call   0x182f609b0
0215f8af: lea    rcx, [rip + 0x1b98d8a]                   ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215f8b6: call   0x182f609b0
0215f8bb: mov    byte ptr [rip + 0x1db9e79], 1            ; [0x3f1973b] (bss)
0215f8c2: mov    rcx, qword ptr [rsi + 0x38]
0215f8c6: test   rcx, rcx
0215f8c9: je     0x18215fa42
0215f8cf: mov    rdx, qword ptr [rip + 0x1b98d6a]         ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215f8d6: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0215f8db: test   rax, rax
0215f8de: je     0x18215fa42
0215f8e4: movzx  ecx, byte ptr [rsp + 0x80]
0215f8ec: movzx  r9d, bl
0215f8f0: mov    qword ptr [rsp + 0x28], 0
0215f8f9: mov    r8, rdi
0215f8fc: mov    byte ptr [rsp + 0x20], cl
0215f900: mov    rdx, rbp
0215f903: mov    rcx, rax
0215f906: call   0x182116390                              ; ʳʷʸʽʳʹʶˁʺʳʿ$$ʾʵʺʼʳʾʴˀʷʷʳ
0215f90b: mov    rdi, rax
0215f90e: test   rax, rax
0215f911: je     0x18215fa42
0215f917: cmp    byte ptr [rax + 0x21], 0
0215f91b: jne    0x18215f9d0
0215f921: mov    rbx, qword ptr [rsi + 0x80]
0215f928: test   rbx, rbx
0215f92b: je     0x18215fa42
0215f931: mov    ebx, dword ptr [rbx + 0x18]
0215f934: sub    ebx, 1
0215f937: js     0x18215f9d0
0215f93d: mov    r8, qword ptr [rip + 0x1b6ec7c]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215f944: mov    edx, ebx
0215f946: mov    rcx, qword ptr [rsi + 0x80]
0215f94d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215f952: test   rax, rax
0215f955: je     0x18215fa42
0215f95b: mov    rcx, qword ptr [rdi + 0x10]
0215f95f: cmp    qword ptr [rax + 0x10], rcx
0215f963: jne    0x18215f9d0
0215f965: cmp    byte ptr [rax + 0x21], 0
0215f969: je     0x18215f9d0
0215f96b: test   ebx, ebx
0215f96d: je     0x18215f9a7
0215f96f: nop    
0215f970: mov    rcx, qword ptr [rsi + 0x80]
0215f977: test   rcx, rcx
0215f97a: je     0x18215fa42
0215f980: mov    r8, qword ptr [rip + 0x1b6ec39]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
0215f987: lea    edx, [rbx - 1]
0215f98a: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215f98f: test   rax, rax
0215f992: je     0x18215fa42
0215f998: mov    rcx, qword ptr [rdi + 0x10]
0215f99c: cmp    qword ptr [rax + 0x10], rcx
0215f9a0: jne    0x18215f9a7
0215f9a2: sub    ebx, 1
0215f9a5: jne    0x18215f970
0215f9a7: mov    rcx, qword ptr [rsi + 0x80]
0215f9ae: test   rcx, rcx
0215f9b1: je     0x18215fa42
0215f9b7: mov    r9, qword ptr [rip + 0x1b6ea82]          ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
0215f9be: mov    r8, rdi
0215f9c1: mov    edx, ebx
0215f9c3: add    rsp, 0x38
0215f9c7: pop    rdi
0215f9c8: pop    rsi
0215f9c9: pop    rbp
0215f9ca: pop    rbx
0215f9cb: jmp    0x1807746c0                              ; System.Collections.Generic.List<object>$$Insert
0215f9d0: mov    rcx, qword ptr [rsi + 0x80]
0215f9d7: test   rcx, rcx
0215f9da: je     0x18215fa42
0215f9dc: mov    r9, qword ptr [rip + 0x1b6e81d]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
0215f9e3: inc    dword ptr [rcx + 0x1c]
0215f9e6: mov    rdx, qword ptr [rcx + 0x10]
0215f9ea: test   rdx, rdx
0215f9ed: je     0x18215fa42
0215f9ef: movsxd r8, dword ptr [rcx + 0x18]
0215f9f3: cmp    r8d, dword ptr [rdx + 0x18]
0215f9f7: jb     0x18215fa18
0215f9f9: mov    rax, qword ptr [r9 + 0x20]
0215f9fd: mov    rdx, rdi
0215fa00: mov    r8, qword ptr [rax + 0xc0]
0215fa07: mov    r8, qword ptr [r8 + 0x70]
0215fa0b: add    rsp, 0x38
0215fa0f: pop    rdi
0215fa10: pop    rsi
0215fa11: pop    rbp
0215fa12: pop    rbx
0215fa13: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0215fa18: lea    eax, [r8 + 1]
0215fa1c: mov    dword ptr [rcx + 0x18], eax
0215fa1f: cmp    r8d, dword ptr [rdx + 0x18]
0215fa23: jae    0x18215fa48
0215fa25: mov    qword ptr [rdx + r8*8 + 0x20], rdi
0215fa2a: add    rdx, 0x20
0215fa2e: lea    rcx, [rdx + r8*8]
0215fa32: mov    rdx, rdi
0215fa35: add    rsp, 0x38
0215fa39: pop    rdi
0215fa3a: pop    rsi
0215fa3b: pop    rbp
0215fa3c: pop    rbx
0215fa3d: jmp    0x182f5fc00
0215fa42: call   0x182f60c50
0215fa47: int3   
0215fa48: call   0x182f60c40
0215fa4d: int3   
0215fa4e: int3   
