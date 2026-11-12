001d6820: push   rbx
001d6822: push   rbp
001d6823: push   rsi
001d6824: push   rdi
001d6825: sub    rsp, 0x28
001d6829: cmp    byte ptr [rcx + 0x9a], 0
001d6830: movzx  ebp, dl
001d6833: mov    esi, dword ptr [rcx + 0x3c]
001d6836: mov    rbx, rcx
001d6839: mov    edi, dword ptr [rcx + 0x40]
001d683c: mov    eax, esi
001d683e: jne    0x1801d6883
001d6840: test   eax, eax
001d6842: jle    0x1801d685e
001d6844: cmp    dword ptr [rcx + 0xe0], 0
001d684b: jle    0x1801d6859
001d684d: test   edi, edi
001d684f: jle    0x1801d6859
001d6851: cmp    edi, dword ptr [rcx + 0xe0]
001d6857: jbe    0x1801d6887
001d6859: dec    dword ptr [rcx + 0x40]
001d685c: jmp    0x1801d688a
001d685e: test   edi, edi
001d6860: jg     0x1801d6859
001d6862: cmp    byte ptr [rcx + 0x2a], 0
001d6866: jne    0x1801d688a
001d6868: cmp    byte ptr [rcx + 0x98], 0
001d686f: je     0x1801d688a
001d6871: mov    rax, qword ptr [rcx]
001d6874: mov    rdx, qword ptr [rax + 0x4a0]
001d687b: call   qword ptr [rax + 0x498]
001d6881: jmp    0x1801d688a
001d6883: test   eax, eax
001d6885: jle    0x1801d688a
001d6887: dec    dword ptr [rcx + 0x3c]
001d688a: cmp    dword ptr [rbx + 0xe4], 0
001d6891: mov    byte ptr [rbx + 0x2e], 1
001d6895: jle    0x1801d68a2
001d6897: cmp    dword ptr [rbx + 0x40], edi
001d689a: je     0x1801d68a2
001d689c: dec    dword ptr [rbx + 0xe4]
001d68a2: test   bpl, bpl
001d68a5: je     0x1801d68ec
001d68a7: cmp    edi, dword ptr [rbx + 0x40]
001d68aa: jne    0x1801d68b1
001d68ac: cmp    esi, dword ptr [rbx + 0x3c]
001d68af: je     0x1801d68ec
001d68b1: cmp    byte ptr [rbx + 0x11c], 0
001d68b8: je     0x1801d68d7
001d68ba: cmp    byte ptr [rbx + 0x11d], 0
001d68c1: je     0x1801d68cd
001d68c3: movss  xmm1, dword ptr [rip + 0x2e8e3f5]        ; [0x3064cc0] flt=0.30000001192092896 i=1050253722
001d68cb: jmp    0x1801d68df
001d68cd: movss  xmm1, dword ptr [rip + 0x2e8dadf]        ; [0x30643b4] flt=0.5 i=1056964608
001d68d5: jmp    0x1801d68df
001d68d7: movss  xmm1, dword ptr [rip + 0x2e8d4dd]        ; [0x3063dbc] flt=1.0 i=1065353216
001d68df: xor    r8d, r8d
001d68e2: mov    ecx, 0xe
001d68e7: call   0x1801a6ef0                              ; ʹˁʾʼʼʹʸʹʿʺʽ$$ʾˁʶʵʺʸʽʾˀʸʸ
001d68ec: add    rsp, 0x28
001d68f0: pop    rdi
001d68f1: pop    rsi
001d68f2: pop    rbp
001d68f3: pop    rbx
001d68f4: ret    
001d68f5: int3   
