00294070: push   rbx
00294072: sub    rsp, 0x40
00294076: cmp    byte ptr [rip + 0x3c7977e], 0            ; [0x3f0d7fb] (bss)
0029407d: mov    rbx, rcx
00294080: jne    0x1802940ad
00294082: lea    rcx, [rip + 0x3a5b807]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00294089: call   0x182f609b0
0029408e: lea    rcx, [rip + 0x3a2d18b]                   ; [0x3cc1220] meta:UnityEngine.RectTransform_TypeInfo
00294095: call   0x182f609b0
0029409a: lea    rcx, [rip + 0x3a4be4f]                   ; [0x3cdfef0] str:'Solo Move'
002940a1: call   0x182f609b0
002940a6: mov    byte ptr [rip + 0x3c7974e], 1            ; [0x3f0d7fb] (bss)
002940ad: movaps xmmword ptr [rsp + 0x30], xmm6
002940b2: xor    edx, edx
002940b4: mov    rcx, rbx
002940b7: movaps xmmword ptr [rsp + 0x20], xmm7
002940bc: call   0x182878410                              ; UnityEngine.Component$$get_transform
002940c1: test   rax, rax
002940c4: je     0x180294208
002940ca: xor    edx, edx
002940cc: mov    rcx, rax
002940cf: call   0x182888b30                              ; UnityEngine.Transform$$GetParent
002940d4: test   rax, rax
002940d7: je     0x180294208
002940dd: mov    rdx, qword ptr [rip + 0x3a4be0c]         ; [0x3cdfef0] str:'Solo Move'
002940e4: xor    r8d, r8d
002940e7: mov    rcx, rax
002940ea: call   0x1828889e0                              ; UnityEngine.Transform$$Find
002940ef: test   rax, rax
002940f2: je     0x180294208
002940f8: mov    rdx, qword ptr [rip + 0x3a2d121]         ; [0x3cc1220] meta:UnityEngine.RectTransform_TypeInfo
002940ff: xor    ecx, ecx
00294101: cmp    qword ptr [rax], rdx
00294104: cmove  rcx, rax
00294108: test   rcx, rcx
0029410b: je     0x180294208
00294111: xor    ecx, ecx
00294113: cmp    qword ptr [rax], rdx
00294116: cmove  rcx, rax
0029411a: xor    edx, edx
0029411c: call   0x182883270                              ; UnityEngine.RectTransform$$get_anchoredPosition
00294121: movq   xmm6, rax
00294126: mov    rax, qword ptr [rip + 0x3a5b763]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0029412d: movsd  qword ptr [rsp + 0x68], xmm6
00294133: cmp    dword ptr [rax + 0xe0], 0
0029413a: jne    0x18029414b
0029413c: mov    rcx, rax
0029413f: call   0x182f60cf0
00294144: mov    rax, qword ptr [rip + 0x3a5b745]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0029414b: mov    rax, qword ptr [rax + 0xb8]
00294152: mov    rcx, qword ptr [rax + 8]
00294156: test   rcx, rcx
00294159: je     0x180294208
0029415f: cmp    dword ptr [rcx + 0x54], 1
00294163: jle    0x18029416f
00294165: movss  xmm7, dword ptr [rip + 0x2dd1813]        ; [0x3065980] flt=1.899999976158142 i=1072902963
0029416d: jmp    0x180294177
0029416f: movss  xmm7, dword ptr [rip + 0x2dcfc45]        ; [0x3063dbc] flt=1.0 i=1065353216
00294177: xor    edx, edx
00294179: mov    rcx, rbx
0029417c: call   0x182878410                              ; UnityEngine.Component$$get_transform
00294181: mov    rcx, qword ptr [rip + 0x3a5b708]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00294188: mov    rbx, rax
0029418b: cmp    dword ptr [rcx + 0xe0], 0
00294192: jne    0x1802941a0
00294194: call   0x182f60cf0
00294199: mov    rcx, qword ptr [rip + 0x3a5b6f0]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002941a0: mov    rcx, qword ptr [rcx + 0xb8]
002941a7: mov    rax, qword ptr [rcx + 8]
002941ab: test   rax, rax
002941ae: je     0x180294208
002941b0: movd   xmm0, dword ptr [rax + 0x54]
002941b5: cvtdq2ps xmm0, xmm0
002941b8: divss  xmm6, xmm0
002941bc: mulss  xmm6, xmm7
002941c0: test   rbx, rbx
002941c3: je     0x180294208
002941c5: mov    rdx, qword ptr [rip + 0x3a2d054]         ; [0x3cc1220] meta:UnityEngine.RectTransform_TypeInfo
002941cc: xor    eax, eax
002941ce: cmp    qword ptr [rbx], rdx
002941d1: cmove  rax, rbx
002941d5: test   rax, rax
002941d8: je     0x180294208
002941da: movss  xmm0, dword ptr [rsp + 0x6c]
002941e0: xor    ecx, ecx
002941e2: cmp    qword ptr [rbx], rdx
002941e5: unpcklps xmm6, xmm0
002941e8: cmove  rcx, rbx
002941ec: movq   rdx, xmm6
002941f1: xor    r8d, r8d
002941f4: movaps xmm7, xmmword ptr [rsp + 0x20]
002941f9: movaps xmm6, xmmword ptr [rsp + 0x30]
002941fe: add    rsp, 0x40
00294202: pop    rbx
00294203: jmp    0x1828839c0                              ; UnityEngine.RectTransform$$set_anchoredPosition
00294208: call   0x182f60c50
0029420d: int3   
0029420e: int3   
