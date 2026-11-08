00208780: mov    qword ptr [rsp + 8], rbx
00208785: push   rdi
00208786: sub    rsp, 0x20
0020878a: mov    rbx, rcx
0020878d: mov    rdi, rdx
00208790: mov    rcx, qword ptr [rcx + 0xe8]
00208797: test   rcx, rcx
0020879a: je     0x1802087fe
0020879c: xor    edx, edx
0020879e: call   0x18026fb50                              ; BrutalFader$$ʶʴʺʼʹʾˀʼʾʺʷ
002087a3: mov    rdx, qword ptr [rbx + 0x120]
002087aa: test   rdx, rdx
002087ad: je     0x1802087fe
002087af: mov    rcx, qword ptr [rbx + 0x38]
002087b3: test   rcx, rcx
002087b6: je     0x1802087fe
002087b8: mov    edx, dword ptr [rdx + 0x88]
002087be: xor    r8d, r8d
002087c1: call   0x1801a88f0                              ; ComboColor$$ʳʽʲʵʹʻˀʷʸʸʶ
002087c6: test   rdi, rdi
002087c9: je     0x1802087fe
002087cb: mov    rax, qword ptr [rbx + 0x20]
002087cf: test   rax, rax
002087d2: je     0x1802087fe
002087d4: movsd  xmm0, qword ptr [rdi + 0x28]
002087d9: xor    edx, edx
002087db: subsd  xmm0, qword ptr [rax + 0x70]
002087e0: mov    rcx, rbx
002087e3: cvtpd2ps xmm0, xmm0
002087e7: movss  dword ptr [rbx + 0xc4], xmm0
002087ef: mov    rbx, qword ptr [rsp + 0x30]
002087f4: add    rsp, 0x20
002087f8: pop    rdi
002087f9: jmp    0x180208070                              ; BasePlayer$$ʳʲʷˁʿʶʾʺʺʿʲ
002087fe: call   0x182f60c50
00208803: int3   
00208804: int3   
