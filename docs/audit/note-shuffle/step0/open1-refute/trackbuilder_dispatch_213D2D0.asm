0213d2d0: mov    qword ptr [rsp + 0x18], r8
0213d2d5: push   rbx
0213d2d6: push   rdi
0213d2d7: push   r14
0213d2d9: push   r15
0213d2db: sub    rsp, 0x68
0213d2df: cmp    byte ptr [rip + 0x1ddc379], 0            ; [0x3f1965f] (bss)
0213d2e6: mov    rdi, r9
0213d2e9: mov    rbx, r8
0213d2ec: mov    r14, rdx
0213d2ef: mov    r15, rcx
0213d2f2: jne    0x18213d31f
0213d2f4: lea    rcx, [rip + 0x1bd17e5]                   ; [0x3d0eae0] meta:long_TypeInfo
0213d2fb: call   0x182f609b0
0213d300: lea    rcx, [rip + 0x1b576d1]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213d307: call   0x182f609b0
0213d30c: lea    rcx, [rip + 0x1b5d64d]                   ; [0x3c9a960] str:'A track event occuring at offset {0} is out of order. It comes after the track event at {1} which is illegal.'
0213d313: call   0x182f609b0
0213d318: mov    byte ptr [rip + 0x1ddc340], 1            ; [0x3f1965f] (bss)
0213d31f: mov    rax, qword ptr [rdi]
0213d322: test   rax, rax
0213d325: je     0x18213d59b
0213d32b: mov    rcx, qword ptr [rip + 0x1b576a6]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213d332: mov    qword ptr [rsp + 0x90], rbp
0213d33a: mov    qword ptr [rsp + 0x60], rsi
0213d33f: mov    rsi, qword ptr [rax + 0x30]
0213d343: cmp    dword ptr [rcx + 0xe0], 0
0213d34a: mov    qword ptr [rsp + 0x58], r12
0213d34f: mov    qword ptr [rsp + 0x50], r13
0213d354: jne    0x18213d35b
0213d356: call   0x182f60cf0
0213d35b: movups xmm0, xmmword ptr [r15]
0213d35f: mov    r9, rsi
0213d362: mov    qword ptr [rsp + 0x20], 0
0213d36b: mov    r8, r14
0213d36e: lea    rdx, [rsp + 0x30]
0213d373: lea    rcx, [rsp + 0x40]
0213d378: movaps xmmword ptr [rsp + 0x30], xmm0
0213d37d: call   0x18213afc0
0213d382: mov    r12, qword ptr [rsp + 0xb0]
0213d38a: mov    rbp, qword ptr [rax + 8]
0213d38e: movzx  r13d, word ptr [rax]
0213d392: cmp    rbp, qword ptr [r12]
0213d396: jge    0x18213d42a
0213d39c: mov    rsi, qword ptr [rdi]
0213d39f: test   rsi, rsi
0213d3a2: je     0x18213d5a1
0213d3a8: mov    rsi, qword ptr [rsi + 0x30]
0213d3ac: lea    rdx, [rsp + 0xa8]
0213d3b4: mov    rcx, qword ptr [rip + 0x1bd1725]         ; [0x3d0eae0] meta:long_TypeInfo
0213d3bb: mov    edi, dword ptr [r14]
0213d3be: mov    qword ptr [rsp + 0xa8], rbp
0213d3c6: call   0x182f5fbe0
0213d3cb: mov    rcx, qword ptr [r12]
0213d3cf: lea    rdx, [rsp + 0x30]
0213d3d4: mov    qword ptr [rsp + 0x30], rcx
0213d3d9: mov    rbx, rax
0213d3dc: mov    rcx, qword ptr [rip + 0x1bd16fd]         ; [0x3d0eae0] meta:long_TypeInfo
0213d3e3: call   0x182f5fbe0
0213d3e8: mov    rcx, qword ptr [rip + 0x1b5d571]         ; [0x3c9a960] str:'A track event occuring at offset {0} is out of order. It comes after the track event at {1} which is illegal.'
0213d3ef: xor    r9d, r9d
0213d3f2: mov    r8, rax
0213d3f5: mov    rdx, rbx
0213d3f8: call   0x181982840                              ; System.String$$Format
0213d3fd: movups xmm0, xmmword ptr [r15]
0213d401: mov    r9, rax
0213d404: mov    qword ptr [rsp + 0x20], 0
0213d40d: mov    r8d, edi
0213d410: lea    rdx, [rsp + 0x40]
0213d415: mov    rcx, rsi
0213d418: movaps xmmword ptr [rsp + 0x40], xmm0
0213d41d: call   0x182163030                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʼʶʼʲʲʼʷʾʹʿʳ
0213d422: mov    rbx, qword ptr [rsp + 0xa0]
0213d42a: mov    qword ptr [r12], rbp
0213d42e: cmp    r13w, 0x45
0213d433: jne    0x18213d45c
0213d435: movups xmm0, xmmword ptr [r15]
0213d439: mov    r9, r12
0213d43c: mov    qword ptr [rsp + 0x20], 0
0213d445: mov    r8, rbp
0213d448: lea    rcx, [rsp + 0x40]
0213d44d: mov    rdx, rbx
0213d450: movaps xmmword ptr [rsp + 0x40], xmm0
0213d455: call   0x18213caf0                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʶʽʽʿʻʾʴˁˀʴʹ
0213d45a: jmp    0x18213d4b6
0213d45c: cmp    r13w, 0x4e
0213d461: jne    0x18213d48a
0213d463: movups xmm0, xmmword ptr [r15]
0213d467: mov    r9, r12
0213d46a: mov    qword ptr [rsp + 0x20], 0
0213d473: mov    r8, rbp
0213d476: lea    rcx, [rsp + 0x40]
0213d47b: mov    rdx, rbx
0213d47e: movaps xmmword ptr [rsp + 0x40], xmm0
0213d483: call   0x18213d620                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʷʾʵʽʻʿˁʶʼʿʿ
0213d488: jmp    0x18213d4b6
0213d48a: cmp    r13w, 0x53
0213d48f: jne    0x18213d506
0213d491: movups xmm0, xmmword ptr [r15]
0213d495: mov    r9, r12
0213d498: mov    qword ptr [rsp + 0x20], 0
0213d4a1: mov    r8, rbp
0213d4a4: lea    rcx, [rsp + 0x40]
0213d4a9: mov    rdx, rbx
0213d4ac: movaps xmmword ptr [rsp + 0x40], xmm0
0213d4b1: call   0x18213e0c0                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʽʾʳʶˁʶʼʼˀʷʻ
0213d4b6: mov    rcx, qword ptr [rip + 0x1b5751b]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213d4bd: mov    rdi, qword ptr [r15]
0213d4c0: mov    ebx, dword ptr [r15 + 8]
0213d4c4: cmp    dword ptr [rcx + 0xe0], 0
0213d4cb: jne    0x18213d4d2
0213d4cd: call   0x182f60cf0
0213d4d2: mov    eax, dword ptr [r14]
0213d4d5: cmp    eax, ebx
0213d4d7: jae    0x18213d5a7
0213d4dd: nop    dword ptr [rax]
0213d4e0: cdqe   
0213d4e2: movzx  ecx, word ptr [rdi + rax*2]
0213d4e6: cmp    cx, 9
0213d4ea: je     0x18213d4f6
0213d4ec: cmp    cx, 0x20
0213d4f0: jne    0x18213d579
0213d4f6: inc    dword ptr [r14]
0213d4f9: mov    eax, dword ptr [r14]
0213d4fc: cmp    eax, ebx
0213d4fe: jae    0x18213d5a7
0213d504: jmp    0x18213d4e0
0213d506: mov    rcx, qword ptr [rip + 0x1b574cb]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213d50d: mov    rsi, qword ptr [r15]
0213d510: mov    edi, dword ptr [r15 + 8]
0213d514: mov    ebx, dword ptr [r14]
0213d517: cmp    dword ptr [rcx + 0xe0], 0
0213d51e: jne    0x18213d525
0213d520: call   0x182f60cf0
0213d525: cmp    byte ptr [rip + 0x1dd0b36], 0            ; [0x3f0e062] (bss)
0213d52c: jne    0x18213d541
0213d52e: lea    rcx, [rip + 0x1b6482b]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
0213d535: call   0x182f609b0
0213d53a: mov    byte ptr [rip + 0x1dd0b21], 1            ; [0x3f0e062] (bss)
0213d541: cmp    ebx, edi
0213d543: jge    0x18213d574
0213d545: jae    0x18213d5a7
0213d547: movsxd rax, ebx
0213d54a: movzx  ecx, word ptr [rsi + rax*2]
0213d54e: cmp    cx, 0xa
0213d552: je     0x18213d574
0213d554: cmp    cx, 0xd
0213d558: jne    0x18213d56e
0213d55a: lea    eax, [rbx + 1]
0213d55d: cmp    eax, edi
0213d55f: jge    0x18213d56e
0213d561: jae    0x18213d5a7
0213d563: movsxd rax, ebx
0213d566: cmp    word ptr [rsi + rax*2 + 2], 0xa
0213d56c: je     0x18213d574
0213d56e: inc    ebx
0213d570: cmp    ebx, edi
0213d572: jl     0x18213d545
0213d574: inc    ebx
0213d576: mov    dword ptr [r14], ebx
0213d579: mov    r12, qword ptr [rsp + 0x58]
0213d57e: mov    rsi, qword ptr [rsp + 0x60]
0213d583: mov    rbp, qword ptr [rsp + 0x90]
0213d58b: mov    r13, qword ptr [rsp + 0x50]
0213d590: add    rsp, 0x68
0213d594: pop    r15
0213d596: pop    r14
0213d598: pop    rdi
0213d599: pop    rbx
0213d59a: ret    
0213d59b: call   0x182f60c50
0213d5a0: int3   
0213d5a1: call   0x182f60c50
0213d5a6: int3   
0213d5a7: call   0x182f60c40
0213d5ac: int3   
0213d5ad: int3   
