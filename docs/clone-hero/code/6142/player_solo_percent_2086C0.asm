002086c0: mov    qword ptr [rsp + 8], rbx
002086c5: push   rdi
002086c6: sub    rsp, 0x40
002086ca: cmp    byte ptr [rip + 0x3d04619], 0            ; [0x3f0ccea] (bss)
002086d1: mov    edi, edx
002086d3: mov    rbx, rcx
002086d6: jne    0x1802086eb
002086d8: lea    rcx, [rip + 0x3abb139]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
002086df: call   0x182f609b0
002086e4: mov    byte ptr [rip + 0x3d045ff], 1            ; [0x3f0ccea] (bss)
002086eb: mov    rcx, qword ptr [rbx + 0x58]
002086ef: test   rcx, rcx
002086f2: je     0x180208779
002086f8: xor    r8d, r8d
002086fb: mov    edx, edi
002086fd: call   0x180294c60                              ; SoloCounter$$ʺʹʴˁʵʲʿʳʼʿʶ
00208702: mov    rax, qword ptr [rip + 0x3abb10f]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
00208709: mov    rcx, qword ptr [rax + 0xb8]
00208710: mov    rcx, qword ptr [rcx]
00208713: test   rcx, rcx
00208716: je     0x180208779
00208718: xor    edx, edx
0020871a: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0020871f: test   al, al
00208721: je     0x18020876e
00208723: mov    rax, qword ptr [rbx + 0x88]
0020872a: test   rax, rax
0020872d: je     0x180208779
0020872f: mov    rcx, qword ptr [rax + 0x10]
00208733: test   rcx, rcx
00208736: je     0x180208779
00208738: cmp    byte ptr [rcx + 0xb1], 0
0020873f: jne    0x18020876e
00208741: mov    rcx, qword ptr [rbx + 0x118]
00208748: test   rcx, rcx
0020874b: je     0x180208779
0020874d: xor    eax, eax
0020874f: xorps  xmm0, xmm0
00208752: mov    qword ptr [rsp + 0x30], rax
00208757: xor    r9d, r9d
0020875a: movsd  qword ptr [rsp + 0x28], xmm0
00208760: xor    r8d, r8d
00208763: mov    edx, edi
00208765: mov    dword ptr [rsp + 0x20], eax
00208769: call   0x18026aae0                              ; ʼʸʺʳʽʿʽʾʽˀʼ$$ˁˀʵʹʳʴʽˁʹʶʹ
0020876e: mov    rbx, qword ptr [rsp + 0x50]
00208773: add    rsp, 0x40
00208777: pop    rdi
00208778: ret    
00208779: call   0x182f60c50
0020877e: int3   
0020877f: int3   
