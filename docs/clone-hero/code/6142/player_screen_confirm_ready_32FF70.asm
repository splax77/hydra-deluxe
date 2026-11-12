0032ff70: push   rbx
0032ff72: sub    rsp, 0x20
0032ff76: cmp    byte ptr [rip + 0x3bdddb7], 0            ; [0x3f0dd34] (bss)
0032ff7d: mov    rbx, rcx
0032ff80: jne    0x18032ff95
0032ff82: lea    rcx, [rip + 0x39df0bf]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032ff89: call   0x182f609b0
0032ff8e: mov    byte ptr [rip + 0x3bddd9f], 1            ; [0x3f0dd34] (bss)
0032ff95: cmp    byte ptr [rbx + 0x245], 0
0032ff9c: je     0x18032ffd1
0032ff9e: mov    rdx, qword ptr [rbx + 0x20]
0032ffa2: test   rdx, rdx
0032ffa5: je     0x180330092
0032ffab: mov    rax, qword ptr [rip + 0x39df096]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032ffb2: mov    rcx, qword ptr [rax + 0xb8]
0032ffb9: mov    rcx, qword ptr [rcx]
0032ffbc: test   rcx, rcx
0032ffbf: je     0x180330092
0032ffc5: mov    rdx, qword ptr [rdx + 0x10]
0032ffc9: xor    r8d, r8d
0032ffcc: call   0x180090d70                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʸʸʲʳʺʲʻˁʵˀʻ
0032ffd1: cmp    byte ptr [rbx + 0x244], 0
0032ffd8: jne    0x180330051
0032ffda: mov    rcx, qword ptr [rbx + 0x158]
0032ffe1: mov    dword ptr [rbx + 0x120], 4
0032ffeb: test   rcx, rcx
0032ffee: je     0x180330092
0032fff4: xor    r8d, r8d
0032fff7: xor    edx, edx
0032fff9: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0032fffe: mov    rcx, qword ptr [rbx + 0x160]
00330005: test   rcx, rcx
00330008: je     0x180330092
0033000e: cmp    dword ptr [rbx + 0x120], 4
00330015: sete   dl
00330018: xor    r8d, r8d
0033001b: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
00330020: cmp    byte ptr [rbx + 0x245], 0
00330027: je     0x18033004b
00330029: mov    rax, qword ptr [rip + 0x39df018]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
00330030: mov    rcx, qword ptr [rax + 0xb8]
00330037: mov    rcx, qword ptr [rcx]
0033003a: test   rcx, rcx
0033003d: je     0x180330092
0033003f: xor    edx, edx
00330041: add    rsp, 0x20
00330045: pop    rbx
00330046: jmp    0x180092f80                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ˁʻʴʽʸʺʷʼʹʹʼ
0033004b: add    rsp, 0x20
0033004f: pop    rbx
00330050: ret    
00330051: mov    rax, qword ptr [rbx + 0x20]
00330055: test   rax, rax
00330058: je     0x180330092
0033005a: mov    rcx, qword ptr [rax + 0x10]
0033005e: test   rcx, rcx
00330061: je     0x180330092
00330063: movzx  r8d, byte ptr [rbx + 0x241]
0033006b: xor    r9d, r9d
0033006e: movzx  edx, byte ptr [rbx + 0x240]
00330075: call   0x1800d86d0                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʾʵʽʳʳˁʼʼʾʶʷ
0033007a: mov    rcx, qword ptr [rbx + 0x1d0]
00330081: test   rcx, rcx
00330084: je     0x180330092
00330086: xor    edx, edx
00330088: add    rsp, 0x20
0033008c: pop    rbx
0033008d: jmp    0x180123240                              ; PlayerStatsBox$$ʵʿʴʲˀʾʿˁˀʽʶ
00330092: call   0x182f60c50
00330097: int3   
00330098: int3   
