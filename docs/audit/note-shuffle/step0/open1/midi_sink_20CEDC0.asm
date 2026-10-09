020cedc0: push   rbx
020cedc2: push   rbp
020cedc3: push   rsi
020cedc4: push   rdi
020cedc5: sub    rsp, 0x38
020cedc9: cmp    byte ptr [rip + 0x1e4a552], 0            ; [0x3f19322] (bss)
020cedd0: movzx  edi, r9b
020cedd4: mov    rsi, r8
020cedd7: mov    rbp, rdx
020cedda: mov    rbx, rcx
020ceddd: jne    0x1820cedfe
020ceddf: lea    rcx, [rip + 0x1bfe3a2]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020cede6: call   0x182f609b0
020cedeb: lea    rcx, [rip + 0x1c2926e]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cedf2: call   0x182f609b0
020cedf7: mov    byte ptr [rip + 0x1e4a524], 1            ; [0x3f19322] (bss)
020cedfe: mov    rcx, qword ptr [rbx + 0x30]
020cee02: test   rcx, rcx
020cee05: je     0x1820ceeb0
020cee0b: mov    rdx, qword ptr [rip + 0x1c2924e]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cee12: mov    rbx, qword ptr [rbx + 0x88]
020cee19: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cee1e: test   rax, rax
020cee21: je     0x1820ceeb0
020cee27: movzx  r9d, dil
020cee2b: mov    qword ptr [rsp + 0x20], 0
020cee34: mov    r8, rsi
020cee37: mov    rdx, rbp
020cee3a: mov    rcx, rax
020cee3d: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
020cee42: mov    r9, rax
020cee45: test   rbx, rbx
020cee48: je     0x1820ceeb0
020cee4a: mov    rax, qword ptr [rip + 0x1bfe337]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020cee51: inc    dword ptr [rbx + 0x1c]
020cee54: mov    rcx, qword ptr [rbx + 0x10]
020cee58: test   rcx, rcx
020cee5b: je     0x1820ceeb0
020cee5d: movsxd rdx, dword ptr [rbx + 0x18]
020cee61: cmp    edx, dword ptr [rcx + 0x18]
020cee64: jb     0x1820cee88
020cee66: mov    rax, qword ptr [rax + 0x20]
020cee6a: mov    rdx, r9
020cee6d: mov    rcx, rbx
020cee70: mov    r8, qword ptr [rax + 0xc0]
020cee77: mov    r8, qword ptr [r8 + 0x70]
020cee7b: add    rsp, 0x38
020cee7f: pop    rdi
020cee80: pop    rsi
020cee81: pop    rbp
020cee82: pop    rbx
020cee83: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020cee88: lea    eax, [rdx + 1]
020cee8b: mov    dword ptr [rbx + 0x18], eax
020cee8e: cmp    edx, dword ptr [rcx + 0x18]
020cee91: jae    0x1820ceeb6
020cee93: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020cee98: lea    rcx, [rcx + rdx*8]
020cee9c: add    rcx, 0x20
020ceea0: mov    rdx, r9
020ceea3: add    rsp, 0x38
020ceea7: pop    rdi
020ceea8: pop    rsi
020ceea9: pop    rbp
020ceeaa: pop    rbx
020ceeab: jmp    0x182f5fc00
020ceeb0: call   0x182f60c50
020ceeb5: int3   
020ceeb6: call   0x182f60c40
020ceebb: int3   
020ceebc: int3   
