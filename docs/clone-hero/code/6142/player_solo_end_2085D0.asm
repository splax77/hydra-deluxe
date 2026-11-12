002085d0: push   rbx
002085d2: push   rbp
002085d3: push   rsi
002085d4: push   rdi
002085d5: sub    rsp, 0x58
002085d9: cmp    byte ptr [rip + 0x3d046f6], 0            ; [0x3f0ccd6] (bss)
002085e0: movzx  edi, r9b
002085e4: mov    esi, r8d
002085e7: mov    ebp, edx
002085e9: mov    rbx, rcx
002085ec: jne    0x180208601
002085ee: lea    rcx, [rip + 0x3abb223]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
002085f5: call   0x182f609b0
002085fa: mov    byte ptr [rip + 0x3d046d5], 1            ; [0x3f0ccd6] (bss)
00208601: mov    rcx, qword ptr [rbx + 0x58]
00208605: movaps xmmword ptr [rsp + 0x40], xmm6
0020860a: test   rcx, rcx
0020860d: je     0x1802086b3
00208613: movsd  xmm6, qword ptr [rsp + 0xa0]
0020861c: mov    r9d, ebp
0020861f: mov    qword ptr [rsp + 0x28], 0
00208628: movzx  r8d, dil
0020862c: mov    edx, esi
0020862e: movsd  qword ptr [rsp + 0x20], xmm6
00208634: call   0x1802960b0                              ; SoloCounter$$ˁʺʸʿʽʼʸʴʺʹˁ
00208639: mov    rax, qword ptr [rip + 0x3abb1d8]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
00208640: mov    rcx, qword ptr [rax + 0xb8]
00208647: mov    rcx, qword ptr [rcx]
0020864a: test   rcx, rcx
0020864d: je     0x1802086b3
0020864f: xor    edx, edx
00208651: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
00208656: test   al, al
00208658: je     0x1802086a5
0020865a: mov    rax, qword ptr [rbx + 0x88]
00208661: test   rax, rax
00208664: je     0x1802086b3
00208666: mov    rcx, qword ptr [rax + 0x10]
0020866a: test   rcx, rcx
0020866d: je     0x1802086b3
0020866f: cmp    byte ptr [rcx + 0xb1], 0
00208676: jne    0x1802086a5
00208678: mov    rcx, qword ptr [rbx + 0x118]
0020867f: test   rcx, rcx
00208682: je     0x1802086b3
00208684: mov    qword ptr [rsp + 0x30], 0
0020868d: movzx  r9d, dil
00208691: movsd  qword ptr [rsp + 0x28], xmm6
00208697: mov    r8b, 1
0020869a: mov    edx, esi
0020869c: mov    dword ptr [rsp + 0x20], ebp
002086a0: call   0x18026aae0                              ; ʼʸʺʳʽʿʽʾʽˀʼ$$ˁˀʵʹʳʴʽˁʹʶʹ
002086a5: movaps xmm6, xmmword ptr [rsp + 0x40]
002086aa: add    rsp, 0x58
002086ae: pop    rdi
002086af: pop    rsi
002086b0: pop    rbp
002086b1: pop    rbx
002086b2: ret    
002086b3: call   0x182f60c50
002086b8: int3   
002086b9: int3   
