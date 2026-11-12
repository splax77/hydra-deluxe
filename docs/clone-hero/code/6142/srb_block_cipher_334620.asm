00334620: mov    qword ptr [rsp + 0x10], rbx
00334625: mov    qword ptr [rsp + 0x18], rbp
0033462a: push   rsi
0033462b: sub    rsp, 0x30
0033462f: cmp    byte ptr [rip + 0x3bd977c], 0            ; [0x3f0ddb2] (bss)
00334636: mov    rsi, r8
00334639: mov    rbp, rdx
0033463c: mov    rbx, rcx
0033463f: jne    0x180334654
00334641: lea    rcx, [rip + 0x39616a8]                   ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334648: call   0x182f609b0
0033464d: mov    byte ptr [rip + 0x3bd975e], 1            ; [0x3f0ddb2] (bss)
00334654: cmp    qword ptr [rbx + 0x10], 0
00334659: je     0x180334730
0033465f: mov    qword ptr [rsp + 0x40], rdi
00334664: mov    rax, qword ptr [rbx + 0x10]
00334668: cmp    dword ptr [rax + 0x18], 0
0033466c: je     0x18033467e
0033466e: cmp    dword ptr [rax + 0x18], 1
00334672: jbe    0x180334744
00334678: lea    rdi, [rax + 0x24]
0033467c: jmp    0x180334680
0033467e: xor    edi, edi
00334680: mov    rcx, qword ptr [rip + 0x3961669]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334687: cmp    dword ptr [rcx + 0xe0], 0
0033468e: jne    0x18033469c
00334690: call   0x182f60cf0
00334695: mov    rcx, qword ptr [rip + 0x3961654]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
0033469c: mov    rax, qword ptr [rcx + 0xb8]
003346a3: mov    rdx, qword ptr [rax + 0x10]
003346a7: test   rdx, rdx
003346aa: je     0x1803346be
003346ac: cmp    dword ptr [rdx + 0x18], 0
003346b0: je     0x1803346be
003346b2: jbe    0x180334744
003346b8: lea    rbx, [rdx + 0x20]
003346bc: jmp    0x1803346c0
003346be: xor    ebx, ebx
003346c0: cmp    dword ptr [rcx + 0xe0], 0
003346c7: jne    0x1803346d5
003346c9: call   0x182f60cf0
003346ce: mov    rcx, qword ptr [rip + 0x396161b]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003346d5: mov    rax, qword ptr [rcx + 0xb8]
003346dc: mov    rcx, qword ptr [rax]
003346df: test   rcx, rcx
003346e2: je     0x1803346f2
003346e4: cmp    dword ptr [rcx + 0x18], 0
003346e8: je     0x1803346f2
003346ea: jbe    0x180334744
003346ec: add    rcx, 0x20
003346f0: jmp    0x1803346f7
003346f2: mov    ecx, 1
003346f7: mov    qword ptr [rsp + 0x28], 0
00334700: mov    r9, rbx
00334703: mov    qword ptr [rsp + 0x20], rcx
00334708: mov    r8, rdi
0033470b: mov    rcx, rbp
0033470e: mov    rdx, rsi
00334711: call   0x180336e50                              ; ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ$$ʴʳʽʵˀʶˁʶˁʵˀ
00334716: mov    rdi, qword ptr [rsp + 0x40]
0033471b: mov    eax, 0x7e
00334720: mov    rbx, qword ptr [rsp + 0x48]
00334725: mov    rbp, qword ptr [rsp + 0x50]
0033472a: add    rsp, 0x30
0033472e: pop    rsi
0033472f: ret    
00334730: mov    rcx, qword ptr [rip + 0x39615b9]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334737: call   0x182c41230
0033473c: xor    ecx, ecx
0033473e: call   0x180334bd0                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ˀʽʶʶʽʳʵʺʶʵʶ
00334743: int3   
00334744: call   0x182f60c40
00334749: int3   
0033474a: int3   
