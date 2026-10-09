0215bbf0: push   rbx
0215bbf2: push   rbp
0215bbf3: push   rsi
0215bbf4: push   r14
0215bbf6: sub    rsp, 0x38
0215bbfa: cmp    byte ptr [rip + 0x1dbdb39], 0            ; [0x3f1973a] (bss)
0215bc01: mov    rsi, r9
0215bc04: mov    ebp, r8d
0215bc07: mov    r14, rdx
0215bc0a: mov    rbx, rcx
0215bc0d: jne    0x18215bc3a
0215bc0f: lea    rcx, [rip + 0x1b7b8ca]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
0215bc16: call   0x182f609b0
0215bc1b: lea    rcx, [rip + 0x1b7bafe]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215bc22: call   0x182f609b0
0215bc27: lea    rcx, [rip + 0x1b9ef82]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215bc2e: call   0x182f609b0
0215bc33: mov    byte ptr [rip + 0x1dbdb00], 1            ; [0x3f1973a] (bss)
0215bc3a: mov    rax, qword ptr [rbx + 0x50]
0215bc3e: mov    qword ptr [rsp + 0x60], rdi
0215bc43: test   rax, rax
0215bc46: je     0x18215bd3d
0215bc4c: movsxd rax, dword ptr [rax + 0x114]
0215bc53: xor    edi, edi
0215bc55: mov    rcx, qword ptr [rbx + 0x10]
0215bc59: cmp    rsi, rax
0215bc5c: cmovg  rdi, rsi
0215bc60: test   rcx, rcx
0215bc63: je     0x18215bd3d
0215bc69: mov    rdx, qword ptr [rip + 0x1b9ef40]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215bc70: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0215bc75: test   rax, rax
0215bc78: je     0x18215bd3d
0215bc7e: mov    ecx, dword ptr [rsp + 0x80]
0215bc85: mov    r9, rdi
0215bc88: mov    qword ptr [rsp + 0x28], 0
0215bc91: mov    r8d, ebp
0215bc94: mov    dword ptr [rsp + 0x20], ecx
0215bc98: mov    rdx, r14
0215bc9b: mov    rcx, rax
0215bc9e: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
0215bca3: mov    rcx, qword ptr [rbx + 0x60]
0215bca7: mov    r9, rax
0215bcaa: test   rcx, rcx
0215bcad: je     0x18215bd3d
0215bcb3: test   rax, rax
0215bcb6: je     0x18215bd3d
0215bcbc: mov    ecx, dword ptr [rcx + 0x18]
0215bcbf: mov    dword ptr [rax + 0x28], ecx
0215bcc2: mov    rcx, qword ptr [rbx + 0x60]
0215bcc6: test   rcx, rcx
0215bcc9: je     0x18215bd3d
0215bccb: mov    r10, qword ptr [rip + 0x1b7b80e]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
0215bcd2: inc    dword ptr [rcx + 0x1c]
0215bcd5: mov    rdx, qword ptr [rcx + 0x10]
0215bcd9: test   rdx, rdx
0215bcdc: je     0x18215bd3d
0215bcde: movsxd r8, dword ptr [rcx + 0x18]
0215bce2: cmp    r8d, dword ptr [rdx + 0x18]
0215bce6: jb     0x18215bd0d
0215bce8: mov    rax, qword ptr [r10 + 0x20]
0215bcec: mov    rdx, r9
0215bcef: mov    r8, qword ptr [rax + 0xc0]
0215bcf6: mov    r8, qword ptr [r8 + 0x70]
0215bcfa: mov    rdi, qword ptr [rsp + 0x60]
0215bcff: add    rsp, 0x38
0215bd03: pop    r14
0215bd05: pop    rsi
0215bd06: pop    rbp
0215bd07: pop    rbx
0215bd08: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0215bd0d: lea    eax, [r8 + 1]
0215bd11: mov    dword ptr [rcx + 0x18], eax
0215bd14: cmp    r8d, dword ptr [rdx + 0x18]
0215bd18: jae    0x18215bd43
0215bd1a: mov    qword ptr [rdx + r8*8 + 0x20], r9
0215bd1f: add    rdx, 0x20
0215bd23: lea    rcx, [rdx + r8*8]
0215bd27: mov    rdx, r9
0215bd2a: mov    rdi, qword ptr [rsp + 0x60]
0215bd2f: add    rsp, 0x38
0215bd33: pop    r14
0215bd35: pop    rsi
0215bd36: pop    rbp
0215bd37: pop    rbx
0215bd38: jmp    0x182f5fc00
0215bd3d: call   0x182f60c50
0215bd42: int3   
0215bd43: call   0x182f60c40
0215bd48: int3   
0215bd49: int3   
