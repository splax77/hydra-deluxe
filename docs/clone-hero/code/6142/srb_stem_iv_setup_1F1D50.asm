001f1d50: push   rbp
001f1d52: sub    rsp, 0x20
001f1d56: lea    rbp, [rsp + 0x20]
001f1d5b: mov    rax, qword ptr [rcx + 0x40]
001f1d5f: mov    r11, rdx
001f1d62: and    rax, 0xffffffffffffffe0
001f1d66: lea    r8, [rax*4]
001f1d6e: cmp    r8, 1
001f1d72: jl     0x1801f1d97
001f1d74: mov    r10, qword ptr [rcx + 0x50]
001f1d78: test   r10, r10
001f1d7b: je     0x1801f1dd5
001f1d7d: mov    rdx, qword ptr [rcx + 0x20]
001f1d81: xor    r9d, r9d
001f1d84: add    rdx, r8
001f1d87: mov    rcx, r10
001f1d8a: mov    r8, r11
001f1d8d: call   0x180334a30                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ʾʳʾʶʽʺʵʴʴʳʽ
001f1d92: mov    rsp, rbp
001f1d95: pop    rbp
001f1d96: ret    
001f1d97: mov    eax, dword ptr [rsp]
001f1d9a: sub    rsp, 0x10
001f1d9e: lea    rdx, [rsp + 0x20]
001f1da3: mov    eax, dword ptr [rdx]
001f1da5: xor    eax, eax
001f1da7: mov    word ptr [rdx], ax
001f1daa: mov    byte ptr [rdx + 2], al
001f1dad: mov    rax, qword ptr [rcx + 0x60]
001f1db1: mov    qword ptr [rdx], rax
001f1db4: mov    rax, qword ptr [rcx + 0x58]
001f1db8: mov    qword ptr [rdx + 2], rax
001f1dbc: mov    rcx, qword ptr [rcx + 0x50]
001f1dc0: test   rcx, rcx
001f1dc3: je     0x1801f1dd5
001f1dc5: xor    r9d, r9d
001f1dc8: mov    r8, r11
001f1dcb: call   0x180334340                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ʹʲˀʹʸʵʽʴʺʵʺ
001f1dd0: mov    rsp, rbp
001f1dd3: pop    rbp
001f1dd4: ret    
001f1dd5: call   0x182f60c50
001f1dda: int3   
001f1ddb: int3   
