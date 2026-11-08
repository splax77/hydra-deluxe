00334340: mov    qword ptr [rsp + 0x10], rbx
00334345: mov    qword ptr [rsp + 0x18], rbp
0033434a: push   rsi
0033434b: sub    rsp, 0x30
0033434f: cmp    byte ptr [rip + 0x3bd9a5e], 0            ; [0x3f0ddb4] (bss)
00334356: mov    rsi, r8
00334359: mov    rbp, rdx
0033435c: mov    rbx, rcx
0033435f: jne    0x180334374
00334361: lea    rcx, [rip + 0x3961988]                   ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334368: call   0x182f609b0
0033436d: mov    byte ptr [rip + 0x3bd9a40], 1            ; [0x3f0ddb4] (bss)
00334374: cmp    qword ptr [rbx + 0x10], 0
00334379: je     0x18033444c
0033437f: mov    qword ptr [rsp + 0x40], rdi
00334384: mov    rax, qword ptr [rbx + 0x10]
00334388: cmp    dword ptr [rax + 0x18], 0
0033438c: je     0x18033439a
0033438e: jbe    0x180334460
00334394: lea    rdi, [rax + 0x20]
00334398: jmp    0x18033439c
0033439a: xor    edi, edi
0033439c: mov    rcx, qword ptr [rip + 0x396194d]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003343a3: cmp    dword ptr [rcx + 0xe0], 0
003343aa: jne    0x1803343b8
003343ac: call   0x182f60cf0
003343b1: mov    rcx, qword ptr [rip + 0x3961938]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003343b8: mov    rax, qword ptr [rcx + 0xb8]
003343bf: mov    rdx, qword ptr [rax + 0x10]
003343c3: test   rdx, rdx
003343c6: je     0x1803343da
003343c8: cmp    dword ptr [rdx + 0x18], 0
003343cc: je     0x1803343da
003343ce: jbe    0x180334460
003343d4: lea    rbx, [rdx + 0x20]
003343d8: jmp    0x1803343dc
003343da: xor    ebx, ebx
003343dc: cmp    dword ptr [rcx + 0xe0], 0
003343e3: jne    0x1803343f1
003343e5: call   0x182f60cf0
003343ea: mov    rcx, qword ptr [rip + 0x39618ff]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003343f1: mov    rax, qword ptr [rcx + 0xb8]
003343f8: mov    rcx, qword ptr [rax]
003343fb: test   rcx, rcx
003343fe: je     0x18033440e
00334400: cmp    dword ptr [rcx + 0x18], 0
00334404: je     0x18033440e
00334406: jbe    0x180334460
00334408: add    rcx, 0x20
0033440c: jmp    0x180334413
0033440e: mov    ecx, 1
00334413: mov    qword ptr [rsp + 0x28], 0
0033441c: mov    r9, rbx
0033441f: mov    qword ptr [rsp + 0x20], rcx
00334424: mov    r8, rdi
00334427: mov    rcx, rbp
0033442a: mov    rdx, rsi
0033442d: call   0x180336e50                              ; ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ$$ʴʳʽʵˀʶˁʶˁʵˀ
00334432: mov    rdi, qword ptr [rsp + 0x40]
00334437: mov    eax, 0xffffffb3
0033443c: mov    rbx, qword ptr [rsp + 0x48]
00334441: mov    rbp, qword ptr [rsp + 0x50]
00334446: add    rsp, 0x30
0033444a: pop    rsi
0033444b: ret    
0033444c: mov    rcx, qword ptr [rip + 0x396189d]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334453: call   0x182c41230
00334458: xor    ecx, ecx
0033445a: call   0x180334bd0                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ˀʽʶʶʽʳʵʺʶʵʶ
0033445f: int3   
00334460: call   0x182f60c40
00334465: int3   
00334466: int3   
