0015df80: test   ecx, ecx
0015df82: je     0x18015f861
0015df88: xor    r8d, r8d
0015df8b: mov    dl, 1
0015df8d: call   0x180236d10                              ; GlobalVariables$$ʽʵʴʲʺʵʽʿʾʼʻ
0015df92: mov    rax, qword ptr [rip + 0x3b918f7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0015df99: mov    rcx, qword ptr [rax + 0xb8]
0015dfa0: mov    rbx, qword ptr [rcx + 8]
0015dfa4: test   rbx, rbx
0015dfa7: je     0x18015f861
0015dfad: mov    ebx, dword ptr [rbx + 0x54]
0015dfb0: mov    rcx, qword ptr [rip + 0x3b3dbc9]         ; [0x3c9bb80] meta:ʽʳʿˀʿˁʵʳʾʽʶ_TypeInfo
0015dfb7: call   0x182f60c00
0015dfbc: mov    rdi, rax
0015dfbf: xor    r8d, r8d
0015dfc2: mov    edx, ebx
0015dfc4: mov    rcx, rax
0015dfc7: call   0x180254830                              ; ʽʳʿˀʿˁʵʳʾʽʶ$$.ctor
0015dfcc: mov    qword ptr [rsi + 0xe8], rdi
0015dfd3: lea    rcx, [rsi + 0xe8]
0015dfda: mov    rdx, rdi
0015dfdd: call   0x182f5fc00
0015dfe2: mov    rax, qword ptr [rip + 0x3b918a7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0015dfe9: mov    rcx, qword ptr [rax + 0xb8]
0015dff0: mov    rdx, qword ptr [rcx + 8]
0015dff4: test   rdx, rdx
0015dff7: je     0x18015f861
0015dffd: mov    rax, qword ptr [rip + 0x3b3a31c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0015e004: mov    rcx, qword ptr [rax + 0xb8]
0015e00b: mov    r8, qword ptr [rcx + 0x10]
0015e00f: test   r8, r8
0015e012: je     0x18015f861
0015e018: mov    r10, qword ptr [rip + 0x3b84fa1]         ; [0x3ce2fc0] meta:EndOfSong_TypeInfo
0015e01f: mov    rax, qword ptr [r10 + 0xb8]
0015e026: mov    r9, qword ptr [rax]
0015e029: test   r9, r9
0015e02c: je     0x18015f861
0015e032: mov    rcx, qword ptr [rsi + 0xe8]
0015e039: test   rcx, rcx
0015e03c: je     0x18015f861
0015e042: xor    edi, edi
0015e044: mov    qword ptr [rsp + 0x28], rdi
0015e049: mov    eax, dword ptr [r9 + 0x20]
0015e04d: mov    dword ptr [rsp + 0x20], eax
0015e051: mov    r9d, dword ptr [r9 + 0x18]
0015e055: mov    r8d, dword ptr [r8 + 0x10]
0015e059: mov    rdx, qword ptr [rdx + 0x28]
0015e05d: call   0x180252200                              ; ʽʳʿˀʿˁʵʳʾʽʶ$$ʼʻʺʾʻʹʸʸʺˁʸ
0015e062: mov    rax, qword ptr [rip + 0x3b91827]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0015e069: mov    rcx, qword ptr [rax + 0xb8]
