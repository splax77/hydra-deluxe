001285a0: push   rbx
001285a2: sub    rsp, 0x20
001285a6: cmp    byte ptr [rip + 0x3de3eac], 0            ; [0x3f0c459] (bss)
001285ad: mov    rbx, rcx
001285b0: jne    0x1801285d1
001285b2: lea    rcx, [rip + 0x3b6b6e7]                   ; [0x3c93ca0] meta:ʵʽʸʽʾʳʳʿʼʸʻ_TypeInfo
001285b9: call   0x182f609b0
001285be: lea    rcx, [rip + 0x3b76b2b]                   ; [0x3c9f0f0] meta:ˀʴʴʾʾʼʺʵʿʸˀ_TypeInfo
001285c5: call   0x182f609b0
001285ca: mov    byte ptr [rip + 0x3de3e88], 1            ; [0x3f0c459] (bss)
001285d1: mov    r9, qword ptr [rbx + 0x38]
001285d5: xor    r10d, r10d
001285d8: mov    qword ptr [rbx + 0x44], r10
001285dc: mov    dword ptr [rbx + 0x40], r10d
001285e0: mov    dword ptr [rbx + 0x68], r10d
001285e4: test   r9, r9
001285e7: je     0x18012869d
001285ed: mov    edx, r10d
001285f0: mov    eax, r10d
001285f3: cmp    eax, dword ptr [r9 + 0x18]
001285f7: jge    0x180128678
001285f9: test   r9, r9
001285fc: je     0x18012869d
00128602: cmp    edx, dword ptr [r9 + 0x18]
00128606: jae    0x1801286a3
0012860c: movsxd rax, edx
0012860f: mov    rcx, qword ptr [r9 + rax*8 + 0x20]
00128614: test   rcx, rcx
00128617: je     0x18012866a
00128619: mov    eax, dword ptr [rcx + 0x94]
0012861f: mov    r8, r10
00128622: add    dword ptr [rbx + 0x40], eax
00128625: mov    eax, dword ptr [rcx + 0xa0]
0012862b: add    dword ptr [rbx + 0x44], eax
0012862e: mov    eax, dword ptr [rcx + 0x88]
00128634: add    dword ptr [rbx + 0x48], eax
00128637: mov    rax, qword ptr [rip + 0x3b6b662]         ; [0x3c93ca0] meta:ʵʽʸʽʾʳʳʿʼʸʻ_TypeInfo
0012863e: cmp    qword ptr [rcx], rax
00128641: cmove  r8, rcx
00128645: test   r8, r8
00128648: jne    0x180128660
0012864a: mov    rax, qword ptr [rip + 0x3b76a9f]         ; [0x3c9f0f0] meta:ˀʴʴʾʾʼʺʵʿʸˀ_TypeInfo
00128651: mov    r8, r10
00128654: cmp    qword ptr [rcx], rax
00128657: cmove  r8, rcx
0012865b: test   r8, r8
0012865e: je     0x18012866a
00128660: mov    eax, dword ptr [r8 + 0xa4]
00128667: add    dword ptr [rbx + 0x68], eax
0012866a: inc    edx
0012866c: mov    eax, edx
0012866e: test   r9, r9
00128671: je     0x18012869d
00128673: jmp    0x1801285f3
00128678: mov    edx, dword ptr [rbx + 0x68]
0012867b: add    edx, dword ptr [rbx + 0x44]
0012867e: add    edx, dword ptr [rbx + 0x40]
00128681: mov    rcx, qword ptr [rbx + 0x88]
00128688: mov    dword ptr [rbx + 0x50], edx
0012868b: test   rcx, rcx
0012868e: je     0x18012869d
00128690: xor    r8d, r8d
00128693: add    rsp, 0x20
00128697: pop    rbx
00128698: jmp    0x180224ee0                              ; SpriteFont$$ʸʲʲˁʷʳʾʻʶʾˁ
0012869d: call   0x182f60c50
001286a2: int3   
001286a3: call   0x182f60c40
001286a8: int3   
001286a9: int3   
