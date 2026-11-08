003818e0: push   rbx
003818e2: sub    rsp, 0x30
003818e6: cmp    byte ptr [rip + 0x3b8c8bd], 0            ; [0x3f0e1aa] (bss)
003818ed: mov    rbx, rcx
003818f0: jne    0x180381905
003818f2: lea    rcx, [rip + 0x3932d3f]                   ; [0x3cb4638] meta:UnityEngine.Rendering.OnDemandRendering_TypeInfo
003818f9: call   0x182f609b0
003818fe: mov    byte ptr [rip + 0x3b8c8a5], 1            ; [0x3f0e1aa] (bss)
00381905: mov    rcx, qword ptr [rip + 0x3932d2c]         ; [0x3cb4638] meta:UnityEngine.Rendering.OnDemandRendering_TypeInfo
0038190c: cmp    dword ptr [rcx + 0xe0], 0
00381913: jne    0x18038191a
00381915: call   0x182f60cf0
0038191a: xor    ecx, ecx
0038191c: call   0x182895320                              ; UnityEngine.Rendering.OnDemandRendering$$get_willCurrentFrameRender
00381921: test   al, al
00381923: je     0x180381abd
00381929: cmp    byte ptr [rbx + 0x94], 0
00381930: je     0x1803819fd
00381936: mov    rax, qword ptr [rbx + 0x48]
0038193a: test   rax, rax
0038193d: je     0x180381aee
00381943: movsd  xmm1, qword ptr [rax + 0x78]
00381948: subsd  xmm1, qword ptr [rbx + 0x38]
0038194d: movss  xmm0, dword ptr [rbx + 0x50]
00381952: cvtps2pd xmm0, xmm0
00381955: mulsd  xmm1, xmm0
00381959: movsd  xmm0, qword ptr [rip + 0x2ce2b97]        ; [0x30644f8] dbl=100.0 q=0x4059000000000000
00381961: divsd  xmm1, qword ptr [rip + 0x2ce4927]        ; [0x3066290] dbl=6.0 q=0x4018000000000000
00381969: addsd  xmm1, qword ptr [rbx + 0x30]
0038196e: comisd xmm1, xmm0
00381972: movsd  qword ptr [rbx + 0x30], xmm1
00381977: jbe    0x180381982
00381979: subsd  xmm1, xmm0
0038197d: movsd  qword ptr [rbx + 0x30], xmm1
00381982: movsd  xmm0, qword ptr [rbx + 0x30]
00381987: cvtpd2ps xmm0, xmm0
0038198b: movss  dword ptr [rbx + 0x78], xmm0
00381990: mov    rcx, qword ptr [rbx + 0x28]
00381994: test   rcx, rcx
00381997: je     0x180381aee
0038199d: mov    rdx, qword ptr [rbx + 0x60]
003819a1: xor    r8d, r8d
003819a4: call   0x182870d50                              ; UnityEngine.Renderer$$GetPropertyBlock
003819a9: mov    rcx, qword ptr [rbx + 0x60]
003819ad: test   rcx, rcx
003819b0: je     0x180381aee
003819b6: movups xmm0, xmmword ptr [rbx + 0x6c]
003819ba: mov    edx, dword ptr [rbx + 0x68]
003819bd: lea    r8, [rsp + 0x20]
003819c2: xor    r9d, r9d
003819c5: movaps xmmword ptr [rsp + 0x20], xmm0
003819ca: call   0x182865fe0                              ; UnityEngine.MaterialPropertyBlock$$SetVector
003819cf: mov    rcx, qword ptr [rbx + 0x28]
003819d3: test   rcx, rcx
003819d6: je     0x180381aee
003819dc: mov    rdx, qword ptr [rbx + 0x60]
003819e0: xor    r8d, r8d
003819e3: call   0x182870de0                              ; UnityEngine.Renderer$$SetPropertyBlock
003819e8: mov    rax, qword ptr [rbx + 0x48]
003819ec: test   rax, rax
003819ef: je     0x180381aee
003819f5: mov    rax, qword ptr [rax + 0x78]
003819f9: mov    qword ptr [rbx + 0x38], rax
003819fd: mov    rax, qword ptr [rbx + 0x88]
00381a04: test   rax, rax
00381a07: je     0x180381aee
00381a0d: cmp    byte ptr [rax + 0x28], 0
00381a11: je     0x180381abd
00381a17: mov    rax, qword ptr [rbx + 0x20]
00381a1b: test   rax, rax
00381a1e: je     0x180381aee
00381a24: mov    rdx, qword ptr [rax + 0x20]
00381a28: test   rdx, rdx
00381a2b: je     0x180381aee
00381a31: mov    rcx, qword ptr [rbx + 0x80]
00381a38: test   rcx, rcx
00381a3b: je     0x180381aee
00381a41: cmp    byte ptr [rdx + 0x90], 0
00381a48: jne    0x180381ac3
00381a4a: xor    edx, edx
00381a4c: call   0x182ae6610                              ; UnityEngine.Video.VideoPlayer$$get_isPaused
00381a51: test   al, al
00381a53: je     0x180381abd
00381a55: cmp    byte ptr [rbx + 0xac], 0
00381a5c: jne    0x180381abd
00381a5e: mov    rcx, qword ptr [rbx + 0x80]
00381a65: mov    byte ptr [rbx + 0xac], 1
00381a6c: test   rcx, rcx
00381a6f: je     0x180381aee
00381a71: xor    edx, edx
00381a73: call   0x182ae62b0                              ; UnityEngine.Video.VideoPlayer$$Play
00381a78: mov    rax, qword ptr [rbx + 0x20]
00381a7c: test   rax, rax
00381a7f: je     0x180381aee
00381a81: mov    rax, qword ptr [rax + 0x20]
00381a85: test   rax, rax
00381a88: je     0x180381aee
00381a8a: mov    rcx, qword ptr [rax + 0xb0]
00381a91: test   rcx, rcx
00381a94: je     0x180381aee
00381a96: xor    edx, edx
00381a98: call   0x18016c910                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ˀʽˁˁʸʵʲʻʺʷʻ
00381a9d: test   al, al
00381a9f: je     0x180381abd
00381aa1: mov    rcx, qword ptr [rbx + 0x80]
00381aa8: test   rcx, rcx
00381aab: je     0x180381aee
00381aad: xor    r8d, r8d
00381ab0: xorps  xmm1, xmm1
00381ab3: add    rsp, 0x30
00381ab7: pop    rbx
00381ab8: jmp    0x182ae6b90                              ; UnityEngine.Video.VideoPlayer$$set_time
00381abd: add    rsp, 0x30
00381ac1: pop    rbx
00381ac2: ret    
00381ac3: xor    edx, edx
00381ac5: call   0x182ae6610                              ; UnityEngine.Video.VideoPlayer$$get_isPaused
00381aca: test   al, al
00381acc: jne    0x180381abd
00381ace: mov    rcx, qword ptr [rbx + 0x80]
00381ad5: test   rcx, rcx
00381ad8: je     0x180381aee
00381ada: xor    edx, edx
00381adc: call   0x182ae6270                              ; UnityEngine.Video.VideoPlayer$$Pause
00381ae1: mov    byte ptr [rbx + 0xac], 0
00381ae8: add    rsp, 0x30
00381aec: pop    rbx
00381aed: ret    
00381aee: call   0x182f60c50
00381af3: int3   
00381af4: int3   
