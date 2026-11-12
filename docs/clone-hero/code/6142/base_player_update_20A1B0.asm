0020a1b0: push   rbx
0020a1b2: sub    rsp, 0x30
0020a1b6: cmp    byte ptr [rip + 0x3d02b22], 0            ; [0x3f0ccdf] (bss)
0020a1bd: mov    rbx, rcx
0020a1c0: movaps xmmword ptr [rsp + 0x20], xmm6
0020a1c5: jne    0x18020a1e6
0020a1c7: lea    rcx, [rip + 0x3ab1c8a]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0020a1ce: call   0x182f609b0
0020a1d3: lea    rcx, [rip + 0x3ab963e]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
0020a1da: call   0x182f609b0
0020a1df: mov    byte ptr [rip + 0x3d02af9], 1            ; [0x3f0ccdf] (bss)
0020a1e6: mov    rax, qword ptr [rbx + 0x20]
0020a1ea: xorps  xmm6, xmm6
0020a1ed: movsd  qword ptr [rsp + 0x40], xmm6
0020a1f3: movsd  qword ptr [rsp + 0x50], xmm6
0020a1f9: mov    qword ptr [rsp + 0x48], rdi
0020a1fe: test   rax, rax
0020a201: je     0x18020a4f5
0020a207: cmp    byte ptr [rax + 0x90], 0
0020a20e: jne    0x18020a270
0020a210: mov    rax, qword ptr [rbx + 0x88]
0020a217: test   rax, rax
0020a21a: je     0x18020a4f5
0020a220: mov    rcx, qword ptr [rax + 0x10]
0020a224: test   rcx, rcx
0020a227: je     0x18020a4f5
0020a22d: cmp    byte ptr [rcx + 0xb1], 0
0020a234: jne    0x18020a270
0020a236: mov    rcx, qword ptr [rax + 0x18]
0020a23a: test   rcx, rcx
0020a23d: je     0x18020a4f5
0020a243: xor    r8d, r8d
0020a246: mov    edx, 0x10
0020a24b: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0020a250: test   al, al
0020a252: je     0x18020a270
0020a254: mov    rcx, qword ptr [rbx + 0x20]
0020a258: test   rcx, rcx
0020a25b: je     0x18020a4f5
0020a261: mov    rdx, qword ptr [rbx + 0x88]
0020a268: xor    r8d, r8d
0020a26b: call   0x1800b9680                              ; GameManager$$ʷʻʶˀʾʵʸˀʳˀʽ
0020a270: mov    rax, qword ptr [rbx + 0x20]
0020a274: test   rax, rax
0020a277: je     0x18020a4f5
0020a27d: cmp    byte ptr [rax + 0x90], 0
0020a284: je     0x18020a2af
0020a286: mov    rax, qword ptr [rip + 0x3ab958b]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0020a28d: mov    rcx, qword ptr [rax + 0xb8]
0020a294: mov    rcx, qword ptr [rcx]
0020a297: test   rcx, rcx
0020a29a: je     0x18020a4f5
0020a2a0: xor    edx, edx
0020a2a2: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0020a2a7: test   al, al
0020a2a9: je     0x18020a4db
0020a2af: mov    rax, qword ptr [rbx + 0x20]
0020a2b3: test   rax, rax
0020a2b6: je     0x18020a4f5
0020a2bc: cmp    byte ptr [rax + 0xc0], 0
0020a2c3: je     0x18020a4db
0020a2c9: cmp    byte ptr [rax + 0x90], 0
0020a2d0: je     0x18020a2e2
0020a2d2: cmp    byte ptr [rbx + 0xd0], 0
0020a2d9: je     0x18020a2e2
0020a2db: mov    byte ptr [rbx + 0xd0], 0
0020a2e2: cmp    byte ptr [rbx + 0x110], 0
0020a2e9: jne    0x18020a368
0020a2eb: test   rax, rax
0020a2ee: je     0x18020a4f5
0020a2f4: movsd  xmm0, qword ptr [rax + 0x70]
0020a2f9: comisd xmm0, xmm6
0020a2fd: jb     0x18020a368
0020a2ff: mov    rax, qword ptr [rip + 0x3ab1b52]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0020a306: cmp    dword ptr [rax + 0xe0], 0
0020a30d: jne    0x18020a31e
0020a30f: mov    rcx, rax
0020a312: call   0x182f60cf0
0020a317: mov    rax, qword ptr [rip + 0x3ab1b3a]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0020a31e: mov    rcx, qword ptr [rbx + 0x88]
0020a325: test   rcx, rcx
0020a328: je     0x18020a4f5
0020a32e: mov    rdx, qword ptr [rcx + 0x10]
0020a332: test   rdx, rdx
0020a335: je     0x18020a4f5
0020a33b: mov    rax, qword ptr [rax + 0xb8]
0020a342: mov    rcx, qword ptr [rax + 8]
0020a346: test   rcx, rcx
0020a349: je     0x18020a4f5
0020a34f: movzx  edx, byte ptr [rdx + 0x10]
0020a353: xor    r8d, r8d
0020a356: call   0x1803677c0                              ; BassAudioManager$$ʶʵʽˀʶʺʲʹˁʽʽ
0020a35b: mov    dword ptr [rbx + 0x10c], eax
0020a361: mov    byte ptr [rbx + 0x110], 1
0020a368: mov    rcx, qword ptr [rbx + 0x20]
0020a36c: xor    dil, dil
0020a36f: test   rcx, rcx
0020a372: je     0x18020a4f5
0020a378: nop    dword ptr [rax + rax]
0020a380: mov    rax, qword ptr [rbx]
0020a383: lea    r8, [rsp + 0x40]
0020a388: movsd  xmm1, qword ptr [rcx + 0x70]
0020a38d: mov    rcx, rbx
0020a390: mov    r9, qword ptr [rax + 0x380]
0020a397: call   qword ptr [rax + 0x378]
0020a39d: test   al, al
0020a39f: je     0x18020a3d4
0020a3a1: mov    rcx, qword ptr [rbx + 0x120]
0020a3a8: test   rcx, rcx
0020a3ab: je     0x18020a4f5
0020a3b1: movsd  xmm1, qword ptr [rsp + 0x40]
0020a3b7: xor    r9d, r9d
0020a3ba: mov    r8b, 1
0020a3bd: call   0x1820f4550                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʺʴʻˀʺˀˁʶʻʷ
0020a3c2: mov    rcx, qword ptr [rbx + 0x20]
0020a3c6: mov    dil, 1
0020a3c9: test   rcx, rcx
0020a3cc: je     0x18020a4f5
0020a3d2: jmp    0x18020a380
0020a3d4: mov    rax, qword ptr [rbx + 0x20]
0020a3d8: test   rax, rax
0020a3db: je     0x18020a4f5
0020a3e1: nop    dword ptr [rax]
0020a3e5: nop    word ptr [rax + rax]
0020a3f0: movsd  xmm1, qword ptr [rax + 0x70]
0020a3f5: lea    r8, [rsp + 0x50]
0020a3fa: xor    r9d, r9d
0020a3fd: mov    rcx, rbx
0020a400: call   0x180208f50                              ; BasePlayer$$ʸʾʶʹʲʽʵʿʹʺʽ
0020a405: test   al, al
0020a407: je     0x18020a43b
0020a409: mov    rcx, qword ptr [rbx + 0x120]
0020a410: test   rcx, rcx
0020a413: je     0x18020a4f5
0020a419: movsd  xmm1, qword ptr [rsp + 0x50]
0020a41f: xor    r9d, r9d
0020a422: mov    r8b, 1
0020a425: call   0x1820f4550                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʺʴʻˀʺˀˁʶʻʷ
0020a42a: mov    rax, qword ptr [rbx + 0x20]
0020a42e: mov    dil, 1
0020a431: test   rax, rax
0020a434: jne    0x18020a3f0
0020a436: jmp    0x18020a4f5
0020a43b: test   dil, dil
0020a43e: jne    0x18020a48f
0020a440: mov    rax, qword ptr [rbx + 0x20]
0020a444: test   rax, rax
0020a447: je     0x18020a4f5
0020a44d: mov    rcx, qword ptr [rbx + 0x120]
0020a454: test   rcx, rcx
0020a457: je     0x18020a4f5
0020a45d: movsd  xmm1, qword ptr [rax + 0x70]
0020a462: xor    r9d, r9d
0020a465: mov    r8b, 1
0020a468: call   0x1820f4550                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʺʴʻˀʺˀˁʶʻʷ
0020a46d: mov    rax, qword ptr [rbx + 0x20]
0020a471: test   rax, rax
0020a474: je     0x18020a4f5
0020a476: mov    rcx, qword ptr [rbx + 0x120]
0020a47d: test   rcx, rcx
0020a480: je     0x18020a4f5
0020a482: movsd  xmm1, qword ptr [rax + 0x70]
0020a487: xor    r8d, r8d
0020a48a: call   0x1820f3500                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʲʹʹʷʶʿʶʸˁʽʷ
0020a48f: mov    rcx, qword ptr [rbx + 0x120]
0020a496: test   rcx, rcx
0020a499: je     0x18020a4f5
0020a49b: mov    rax, qword ptr [rcx]
0020a49e: mov    rbx, qword ptr [rbx + 0x28]
0020a4a2: mov    rdx, qword ptr [rax + 0x2c0]
0020a4a9: call   qword ptr [rax + 0x2b8]
0020a4af: test   rbx, rbx
0020a4b2: je     0x18020a4f5
0020a4b4: mov    r9, qword ptr [rbx]
0020a4b7: movzx  edx, ax
0020a4ba: mov    rcx, rbx
0020a4bd: mov    r8, qword ptr [r9 + 0x240]
0020a4c4: call   qword ptr [r9 + 0x238]
0020a4cb: mov    rdi, qword ptr [rsp + 0x48]
0020a4d0: movaps xmm6, xmmword ptr [rsp + 0x20]
0020a4d5: add    rsp, 0x30
0020a4d9: pop    rbx
0020a4da: ret    
0020a4db: xor    edx, edx
0020a4dd: mov    rcx, rbx
0020a4e0: call   0x180208400                              ; BasePlayer$$ʴˀʿʺʽʺʳʸʷʷʺ
0020a4e5: mov    rdi, qword ptr [rsp + 0x48]
0020a4ea: movaps xmm6, xmmword ptr [rsp + 0x20]
0020a4ef: add    rsp, 0x30
0020a4f3: pop    rbx
0020a4f4: ret    
0020a4f5: call   0x182f60c50
0020a4fa: int3   
0020a4fb: int3   
