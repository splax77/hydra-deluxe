001f2060: push   rbp
001f2062: push   rbx
001f2063: push   rsi
001f2064: push   rdi
001f2065: sub    rsp, 0x28
001f2069: lea    rbp, [rsp + 0x20]
001f206e: mov    rax, qword ptr [rcx + 0x40]
001f2072: mov    rsi, rdx
001f2075: and    rax, 0xfffffffffffffff0
001f2079: mov    rbx, rcx
001f207c: lea    r8, [rax*2 - 0x20]
001f2084: test   r8, r8
001f2087: js     0x1801f20ad
001f2089: mov    rcx, qword ptr [rcx + 0x50]
001f208d: test   rcx, rcx
001f2090: je     0x1801f2100
001f2092: mov    rdx, qword ptr [rbx + 0x20]
001f2096: xor    r9d, r9d
001f2099: add    rdx, r8
001f209c: mov    r8, rsi
001f209f: call   0x180334620                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ʼʾʵʳʼʶʺʿʹˁʵ
001f20a4: lea    rsp, [rbp + 8]
001f20a8: pop    rdi
001f20a9: pop    rsi
001f20aa: pop    rbx
001f20ab: pop    rbp
001f20ac: ret    
001f20ad: mov    rax, 0xffffffffffffff90
001f20b4: call   0x18302b960
001f20b9: sub    rsp, rax
001f20bc: xor    edx, edx
001f20be: mov    r8, 0xffffffffffffff8f
001f20c5: lea    rdi, [rsp + 0x20]
001f20ca: mov    rcx, rdi
001f20cd: call   0x18305d9d0
001f20d2: mov    rax, qword ptr [rbx + 0x60]
001f20d6: mov    qword ptr [rdi], rax
001f20d9: mov    rax, qword ptr [rbx + 0x58]
001f20dd: mov    qword ptr [rdi], rax
001f20e0: mov    rcx, qword ptr [rbx + 0x50]
001f20e4: test   rcx, rcx
001f20e7: je     0x1801f2100
001f20e9: xor    r9d, r9d
001f20ec: mov    r8, rsi
001f20ef: mov    rdx, rdi
001f20f2: call   0x180334620                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ʼʾʵʳʼʶʺʿʹˁʵ
001f20f7: lea    rsp, [rbp + 8]
001f20fb: pop    rdi
001f20fc: pop    rsi
001f20fd: pop    rbx
001f20fe: pop    rbp
001f20ff: ret    
001f2100: call   0x182f60c50
001f2105: int3   
001f2106: int3   
