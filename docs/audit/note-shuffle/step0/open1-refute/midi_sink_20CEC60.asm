020cec60: push   rbx
020cec62: push   rbp
020cec63: push   rsi
020cec64: push   r14
020cec66: sub    rsp, 0x38
020cec6a: cmp    byte ptr [rip + 0x1e4a6ad], 0            ; [0x3f1931e] (bss)
020cec71: mov    rsi, r9
020cec74: mov    ebp, r8d
020cec77: mov    r14, rdx
020cec7a: mov    rbx, rcx
020cec7d: jne    0x1820cecaa
020cec7f: lea    rcx, [rip + 0x1c0885a]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020cec86: call   0x182f609b0
020cec8b: lea    rcx, [rip + 0x1c08a8e]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020cec92: call   0x182f609b0
020cec97: lea    rcx, [rip + 0x1c2bf12]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cec9e: call   0x182f609b0
020ceca3: mov    byte ptr [rip + 0x1e4a674], 1            ; [0x3f1931e] (bss)
020cecaa: mov    rax, qword ptr [rbx + 0x50]
020cecae: mov    qword ptr [rsp + 0x60], rdi
020cecb3: test   rax, rax
020cecb6: je     0x1820cedad
020cecbc: movsxd rax, dword ptr [rax + 0x114]
020cecc3: xor    edi, edi
020cecc5: mov    rcx, qword ptr [rbx + 0x10]
020cecc9: cmp    rsi, rax
020ceccc: cmovg  rdi, rsi
020cecd0: test   rcx, rcx
020cecd3: je     0x1820cedad
020cecd9: mov    rdx, qword ptr [rip + 0x1c2bed0]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cece0: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cece5: test   rax, rax
020cece8: je     0x1820cedad
020cecee: mov    ecx, dword ptr [rsp + 0x80]
020cecf5: mov    r9, rdi
020cecf8: mov    qword ptr [rsp + 0x28], 0
020ced01: mov    r8d, ebp
020ced04: mov    dword ptr [rsp + 0x20], ecx
020ced08: mov    rdx, r14
020ced0b: mov    rcx, rax
020ced0e: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020ced13: mov    rcx, qword ptr [rbx + 0x60]
020ced17: mov    r9, rax
020ced1a: test   rcx, rcx
020ced1d: je     0x1820cedad
020ced23: test   rax, rax
020ced26: je     0x1820cedad
020ced2c: mov    ecx, dword ptr [rcx + 0x18]
020ced2f: mov    dword ptr [rax + 0x28], ecx
020ced32: mov    rcx, qword ptr [rbx + 0x60]
020ced36: test   rcx, rcx
020ced39: je     0x1820cedad
020ced3b: mov    r10, qword ptr [rip + 0x1c0879e]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020ced42: inc    dword ptr [rcx + 0x1c]
020ced45: mov    rdx, qword ptr [rcx + 0x10]
020ced49: test   rdx, rdx
020ced4c: je     0x1820cedad
020ced4e: movsxd r8, dword ptr [rcx + 0x18]
020ced52: cmp    r8d, dword ptr [rdx + 0x18]
020ced56: jb     0x1820ced7d
020ced58: mov    rax, qword ptr [r10 + 0x20]
020ced5c: mov    rdx, r9
020ced5f: mov    r8, qword ptr [rax + 0xc0]
020ced66: mov    r8, qword ptr [r8 + 0x70]
020ced6a: mov    rdi, qword ptr [rsp + 0x60]
020ced6f: add    rsp, 0x38
020ced73: pop    r14
020ced75: pop    rsi
020ced76: pop    rbp
020ced77: pop    rbx
020ced78: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020ced7d: lea    eax, [r8 + 1]
020ced81: mov    dword ptr [rcx + 0x18], eax
020ced84: cmp    r8d, dword ptr [rdx + 0x18]
020ced88: jae    0x1820cedb3
020ced8a: mov    qword ptr [rdx + r8*8 + 0x20], r9
020ced8f: add    rdx, 0x20
020ced93: lea    rcx, [rdx + r8*8]
020ced97: mov    rdx, r9
020ced9a: mov    rdi, qword ptr [rsp + 0x60]
020ced9f: add    rsp, 0x38
020ceda3: pop    r14
020ceda5: pop    rsi
020ceda6: pop    rbp
020ceda7: pop    rbx
020ceda8: jmp    0x182f5fc00
020cedad: call   0x182f60c50
020cedb2: int3   
020cedb3: call   0x182f60c40
020cedb8: int3   
020cedb9: int3   
