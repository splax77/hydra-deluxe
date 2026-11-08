002096f0: mov    qword ptr [rsp + 8], rbx
002096f5: push   rdi
002096f6: sub    rsp, 0x30
002096fa: cmp    byte ptr [rip + 0x3d035d4], 0            ; [0x3f0ccd5] (bss)
00209701: mov    rbx, rdx
00209704: mov    rdi, rcx
00209707: jne    0x180209728
00209709: lea    rcx, [rip + 0x3aba108]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
00209710: call   0x182f609b0
00209715: lea    rcx, [rip + 0x3a8ec04]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0020971c: call   0x182f609b0
00209721: mov    byte ptr [rip + 0x3d035ad], 1            ; [0x3f0ccd5] (bss)
00209728: mov    rax, qword ptr [rip + 0x3aba0e9]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0020972f: mov    r8, qword ptr [rax + 0xb8]
00209736: mov    rcx, qword ptr [r8]
00209739: test   rcx, rcx
0020973c: je     0x180209828
00209742: xor    edx, edx
00209744: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
00209749: test   al, al
0020974b: je     0x180209788
0020974d: mov    rcx, qword ptr [rdi + 0x88]
00209754: test   rcx, rcx
00209757: je     0x180209828
0020975d: xor    edx, edx
0020975f: call   0x180337720                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʿʹʲʸʻʻʼʵʸʵˀ
00209764: test   al, al
00209766: je     0x180209788
00209768: mov    rax, qword ptr [rip + 0x3a8ebb1]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0020976f: xor    edx, edx
00209771: mov    rcx, qword ptr [rax + 0xb8]
00209778: mov    rcx, qword ptr [rcx + 0x270]
0020977f: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
00209784: test   al, al
00209786: jne    0x1802097d5
00209788: mov    rcx, qword ptr [rdi + 0x58]
0020978c: test   rcx, rcx
0020978f: je     0x180209828
00209795: xor    edx, edx
00209797: call   0x180294a70                              ; SoloCounter$$ʺʸʳʷʲʾʹʵʽʿʼ
0020979c: test   rbx, rbx
0020979f: je     0x180209828
002097a5: mov    rcx, qword ptr [rdi + 0x58]
002097a9: test   rcx, rcx
002097ac: je     0x180209828
002097ae: movzx  edx, byte ptr [rbx + 0x11]
002097b2: xor    r8d, r8d
002097b5: call   0x180294c60                              ; SoloCounter$$ʺʹʴˁʵʲʿʳʼʿʶ
002097ba: mov    rcx, qword ptr [rdi + 0x58]
002097be: test   rcx, rcx
002097c1: je     0x180209828
002097c3: cmp    byte ptr [rbx + 0x12], 0
002097c7: movzx  edx, byte ptr [rbx + 0x11]
002097cb: jne    0x1802097e0
002097cd: xor    r8d, r8d
002097d0: call   0x180294c60                              ; SoloCounter$$ʺʹʴˁʵʲʿʳʼʿʶ
002097d5: mov    rbx, qword ptr [rsp + 0x40]
002097da: add    rsp, 0x30
002097de: pop    rdi
002097df: ret    
002097e0: movsd  xmm0, qword ptr [rbx + 0x18]
002097e5: mov    r9d, dword ptr [rbx + 0x14]
002097e9: movzx  r8d, byte ptr [rbx + 0x13]
002097ee: mov    qword ptr [rsp + 0x28], 0
002097f7: movsd  qword ptr [rsp + 0x20], xmm0
002097fd: call   0x1802960b0                              ; SoloCounter$$ˁʺʸʿʽʼʸʴʺʹˁ
00209802: mov    rcx, qword ptr [rdi + 0x120]
00209809: test   rcx, rcx
0020980c: je     0x180209828
0020980e: mov    eax, dword ptr [rcx + 0xa0]
00209814: add    eax, dword ptr [rbx + 0x14]
00209817: mov    rbx, qword ptr [rsp + 0x40]
0020981c: mov    dword ptr [rcx + 0xa0], eax
00209822: add    rsp, 0x30
00209826: pop    rdi
00209827: ret    
00209828: call   0x182f60c50
0020982d: int3   
0020982e: int3   
