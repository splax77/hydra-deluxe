001286b0: push   rbx
001286b2: sub    rsp, 0x70
001286b6: cmp    byte ptr [rip + 0x3de3d9f], 0            ; [0x3f0c45c] (bss)
001286bd: mov    rbx, rcx
001286c0: movaps xmmword ptr [rsp + 0x60], xmm6
001286c5: movaps xmmword ptr [rsp + 0x50], xmm7
001286ca: movaps xmmword ptr [rsp + 0x40], xmm8
001286d0: jne    0x1801286e5
001286d2: lea    rcx, [rip + 0x3bc71b7]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
001286d9: call   0x182f609b0
001286de: mov    byte ptr [rip + 0x3de3d77], 1            ; [0x3f0c45c] (bss)
001286e5: xor    edx, edx
001286e7: mov    qword ptr [rsp + 0x80], rdi
001286ef: mov    rcx, rbx
001286f2: call   0x1801285a0                              ; ScoreManager$$ˀʻʹʼʽʳʲʾʷʹʲ
001286f7: cmp    dword ptr [rbx + 0x48], 0x1e
001286fb: mov    ecx, 0x5c
00128700: movss  xmm8, dword ptr [rbx + 0x58]
00128706: mov    eax, 0x60
0012870b: cmovl  eax, ecx
0012870e: xor    ecx, ecx
00128710: movss  xmm7, dword ptr [rax + rbx]
00128715: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
0012871a: mov    rdx, qword ptr [rbx + 0x20]
0012871e: movaps xmm6, xmm0
00128721: test   rdx, rdx
00128724: je     0x180128926
0012872a: xor    r8d, r8d
0012872d: lea    rcx, [rsp + 0x20]
00128732: mov    rdi, rdx
00128735: call   0x182889ef0                              ; UnityEngine.Transform$$get_localPosition
0012873a: mov    rdx, qword ptr [rbx + 0x20]
0012873e: test   rdx, rdx
00128741: je     0x180128926
00128747: mulss  xmm6, xmm8
0012874c: lea    rcx, [rsp + 0x20]
00128751: xor    r8d, r8d
00128754: movss  xmm8, dword ptr [rax]
00128759: call   0x182889ef0                              ; UnityEngine.Transform$$get_localPosition
0012875e: movaps xmm0, xmm7
00128761: movss  xmm1, dword ptr [rax + 4]
00128766: xor    eax, eax
00128768: subss  xmm0, xmm1
0012876c: andps  xmm0, xmmword ptr [rip + 0x2f3b6cd]      ; [0x3063e40] xmm dbl=(nan, nan) flt=(nan, nan, nan, nan)
00128773: comiss xmm6, xmm0
00128776: setb   al
00128779: test   eax, eax
0012877b: je     0x1801287ad
0012877d: subss  xmm7, xmm1
00128781: xorps  xmm0, xmm0
00128784: comiss xmm7, xmm0
00128787: jb     0x18012879b
00128789: movss  xmm0, dword ptr [rip + 0x2f3b62b]        ; [0x3063dbc] flt=1.0 i=1065353216
00128791: mulss  xmm6, xmm0
00128795: addss  xmm6, xmm1
00128799: jmp    0x1801287b0
0012879b: movss  xmm0, dword ptr [rip + 0x2f3b62d]        ; [0x3063dd0] flt=-1.0 i=3212836864
001287a3: mulss  xmm6, xmm0
001287a7: addss  xmm6, xmm1
001287ab: jmp    0x1801287b0
001287ad: movaps xmm6, xmm7
001287b0: mov    rdx, qword ptr [rbx + 0x20]
001287b4: test   rdx, rdx
001287b7: je     0x180128926
001287bd: xor    r8d, r8d
001287c0: movss  dword ptr [rsp + 0x20], xmm8
001287c7: lea    rcx, [rsp + 0x30]
001287cc: movss  dword ptr [rsp + 0x24], xmm6
001287d2: call   0x182889ef0                              ; UnityEngine.Transform$$get_localPosition
001287d7: xor    r8d, r8d
001287da: lea    rdx, [rsp + 0x20]
001287df: mov    rcx, rdi
001287e2: movss  xmm0, dword ptr [rax + 8]
001287e7: movss  dword ptr [rsp + 0x28], xmm0
001287ed: call   0x18288a510                              ; UnityEngine.Transform$$set_localPosition
001287f2: mov    eax, dword ptr [rbx + 0x48]
001287f5: cmp    dword ptr [rbx + 0x54], eax
001287f8: je     0x180128817
001287fa: mov    rcx, qword ptr [rbx + 0x80]
00128801: mov    dword ptr [rbx + 0x54], eax
00128804: test   rcx, rcx
00128807: je     0x180128926
0012880d: xor    r8d, r8d
00128810: mov    edx, eax
00128812: call   0x180224ee0                              ; SpriteFont$$ʸʲʲˁʷʳʾʻʶʾˁ
00128817: cmp    byte ptr [rbx + 0x71], 0
0012881b: jne    0x1801288ba
00128821: mov    rcx, qword ptr [rbx + 0x90]
00128828: test   rcx, rcx
0012882b: je     0x180128926
00128831: mov    edx, dword ptr [rbx + 0x40]
00128834: xor    r8d, r8d
00128837: call   0x1802df7f0                              ; StarProgress$$ʺʽʸʺʾˀʲʾʹʲʻ
0012883c: cmp    dword ptr [rbx + 0x4c], 0
00128840: je     0x1801288ba
00128842: cmp    byte ptr [rbx + 0x30], 0
00128846: jne    0x1801288ba
00128848: cmp    byte ptr [rbx + 0xa4], 0
0012884f: jne    0x1801288d8
00128855: mov    rax, qword ptr [rip + 0x3bc7034]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0012885c: cmp    dword ptr [rax + 0xe0], 0
00128863: jne    0x180128874
00128865: mov    rcx, rax
00128868: call   0x182f60cf0
0012886d: mov    rax, qword ptr [rip + 0x3bc701c]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00128874: mov    rax, qword ptr [rax + 0xb8]
0012887b: mov    rcx, qword ptr [rax + 8]
0012887f: test   rcx, rcx
00128882: je     0x180128926
00128888: cmp    dword ptr [rcx + 0x7c], 0
0012888c: jne    0x1801288ba
0012888e: mov    eax, dword ptr [rbx + 0x4c]
00128891: cmp    dword ptr [rbx + 0x50], eax
00128894: jle    0x1801288ba
00128896: cmp    byte ptr [rbx + 0x70], 0
0012889a: jne    0x1801288ba
0012889c: mov    rcx, qword ptr [rbx + 0x28]
001288a0: mov    byte ptr [rbx + 0x70], 1
001288a4: test   rcx, rcx
001288a7: je     0x180128926
001288a9: xor    r8d, r8d
001288ac: xor    edx, edx
001288ae: call   0x1802e5b50                              ; HighScoreDisplay$$ʳʼʴʵʽˀʸʽʷʵʿ
001288b3: mov    byte ptr [rbx + 0xa4], 1
001288ba: mov    rdi, qword ptr [rsp + 0x80]
001288c2: movaps xmm6, xmmword ptr [rsp + 0x60]
001288c7: movaps xmm7, xmmword ptr [rsp + 0x50]
001288cc: movaps xmm8, xmmword ptr [rsp + 0x40]
001288d2: add    rsp, 0x70
001288d6: pop    rbx
001288d7: ret    
001288d8: movss  xmm6, dword ptr [rbx + 0xa0]
001288e0: xor    ecx, ecx
001288e2: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
001288e7: movss  xmm1, dword ptr [rip + 0x2f3bae1]        ; [0x30643d0] flt=3.5 i=1080033280
001288ef: addss  xmm0, xmm6
001288f3: comiss xmm1, xmm0
001288f6: movss  dword ptr [rbx + 0xa0], xmm0
001288fe: ja     0x1801288ba
00128900: mov    rcx, qword ptr [rbx + 0x28]
00128904: mov    byte ptr [rbx + 0xa4], 0
0012890b: mov    dword ptr [rbx + 0xa0], 0
00128915: test   rcx, rcx
00128918: je     0x180128926
0012891a: xor    r8d, r8d
0012891d: mov    dl, 1
0012891f: call   0x1802e5b50                              ; HighScoreDisplay$$ʳʼʴʵʽˀʸʽʷʵʿ
00128924: jmp    0x1801288ba
00128926: call   0x182f60c50
0012892b: int3   
0012892c: int3   
