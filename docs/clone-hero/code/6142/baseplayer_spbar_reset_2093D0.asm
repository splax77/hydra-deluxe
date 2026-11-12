002093d0: push   rbx
002093d2: sub    rsp, 0x20
002093d6: mov    rbx, rcx
002093d9: mov    rcx, qword ptr [rcx + 0x38]
002093dd: test   rcx, rcx
002093e0: je     0x18020944a
002093e2: xor    edx, edx
002093e4: call   0x1801a9af0                              ; ComboColor$$ʿˀʾʳʸʶʸʸʹʴˀ
002093e9: mov    rcx, qword ptr [rbx + 0x28]
002093ed: test   rcx, rcx
002093f0: je     0x18020944a
002093f2: mov    rax, qword ptr [rcx]
002093f5: xor    edx, edx
002093f7: mov    r8, qword ptr [rax + 0x1c0]
002093fe: call   qword ptr [rax + 0x1b8]
00209404: xor    edx, edx
00209406: mov    rcx, rbx
00209409: call   0x182877cc0                              ; UnityEngine.Behaviour$$get_isActiveAndEnabled
0020940e: test   al, al
00209410: je     0x180209444
00209412: mov    rcx, qword ptr [rbx + 0x90]
00209419: test   rcx, rcx
0020941c: je     0x18020944a
0020941e: xor    r9d, r9d
00209421: xor    r8d, r8d
00209424: xorps  xmm1, xmm1
00209427: call   0x18014d0d0                              ; SPBar$$ʸʿʲˁʴʾˁʴʻʳʼ
0020942c: mov    rcx, qword ptr [rbx + 0xd8]
00209433: test   rcx, rcx
00209436: je     0x18020944a
00209438: xor    edx, edx
0020943a: add    rsp, 0x20
0020943e: pop    rbx
0020943f: jmp    0x18014e3b0                              ; SpNeckRenderer$$ʾʺʵʿʿʷʴʼʻʲʴ
00209444: add    rsp, 0x20
00209448: pop    rbx
00209449: ret    
0020944a: call   0x182f60c50
0020944f: int3   
