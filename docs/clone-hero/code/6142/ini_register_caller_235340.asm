00235340: mov    rsi, qword ptr [r12 + rax*8 + 0x20]
00235345: mov    rcx, qword ptr [rip + 0x3a81e04]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
0023534c: cmp    dword ptr [rcx + 0xe0], 0
00235353: jne    0x18023535a
00235355: call   0x182f60cf0
0023535a: xor    r8d, r8d
0023535d: mov    rdx, qword ptr [rip + 0x3a70d74]         ; [0x3ca60d8] str:'highway.webm'
00235364: mov    rcx, rsi
00235367: call   0x181b1eee0                              ; System.IO.Path$$Combine
0023536c: mov    rdi, rax
0023536f: xor    edx, edx
00235371: mov    rcx, rax
00235374: call   0x1803a1fb0                              ; ʴʸʼʻʺʺˁʻʴʼʷ$$ʹʵʽʺʵʹʿʸʽʷʼ
00235379: test   al, al
0023537b: je     0x1802354c3
00235381: mov    rcx, qword ptr [r13 + 0x188]
00235388: test   rcx, rcx
0023538b: je     0x180235697
00235391: mov    r8, qword ptr [rip + 0x3a82ce8]          ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
00235398: mov    rdx, rdi
0023539b: call   0x182c425f0
002353a0: mov    rcx, qword ptr [rip + 0x3a62531]         ; [0x3c978d8] meta:ʸˁʽʳˁˀʶˁʽʴˀ_TypeInfo
002353a7: call   0x182f60c00
002353ac: mov    rdi, rax
002353af: xor    edx, edx
002353b1: mov    rcx, rax
002353b4: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
002353b9: test   rdi, rdi
002353bc: je     0x180235692
002353c2: mov    byte ptr [rdi + 0x10], 0
002353c6: mov    qword ptr [rdi + 0x20], rsi
002353ca: lea    rcx, [rdi + 0x20]
002353ce: mov    rdx, rsi
002353d1: call   0x182f5fc00
002353d6: mov    byte ptr [rdi + 0x28], 1
002353da: mov    rcx, qword ptr [rip + 0x3a81d6f]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
002353e1: cmp    dword ptr [rcx + 0xe0], 0
002353e8: jne    0x1802353ef
002353ea: call   0x182f60cf0
002353ef: xor    edx, edx
002353f1: mov    rcx, rsi
002353f4: call   0x181b20010                              ; System.IO.Path$$GetFileNameWithoutExtension
002353f9: mov    qword ptr [rdi + 0x18], rax
002353fd: lea    rcx, [rdi + 0x18]
00235401: mov    rdx, rax
00235404: call   0x182f5fc00
00235409: xor    r8d, r8d
0023540c: mov    rdx, qword ptr [rdi + 0x18]
00235410: mov    rcx, qword ptr [rip + 0x3a70e31]         ; [0x3ca6248] str:'highway_'
00235417: call   0x18197f200                              ; System.String$$Concat
0023541c: mov    r15, rax
0023541f: xor    r8d, r8d
00235422: mov    rdx, qword ptr [rip + 0x3a97087]         ; [0x3ccc4b0] str:'config.ini'
00235429: mov    rcx, rsi
0023542c: call   0x181b1eee0                              ; System.IO.Path$$Combine
00235431: mov    rsi, rax
00235434: mov    rcx, qword ptr [rip + 0x3ad8b2d]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
0023543b: cmp    dword ptr [rcx + 0xe0], 0
00235442: jne    0x180235449
00235444: call   0x182f60cf0
00235449: xor    r8d, r8d
0023544c: mov    rdx, rsi
0023544f: mov    rcx, r15
00235452: call   0x182112bf0                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʴˀʺʳʲʺʴʴʽʷʳ
00235457: mov    r15, rax
0023545a: mov    rcx, qword ptr [rip + 0x3a61ba7]         ; [0x3c97008] meta:ʸʻʿʴʴʼʽʴʷˁʶ_TypeInfo
00235461: call   0x182f60c00
00235466: mov    rsi, rax
00235469: xor    edx, edx
0023546b: mov    rcx, rax
0023546e: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
00235473: test   rsi, rsi
00235476: je     0x18023568d
0023547c: xor    r8d, r8d
