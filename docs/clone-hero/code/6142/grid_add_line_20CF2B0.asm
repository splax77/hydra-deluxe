020cf2b0: push   rbp
020cf2b2: push   rsi
020cf2b3: push   rdi
020cf2b4: push   r14
020cf2b6: sub    rsp, 0x68
020cf2ba: cmp    byte ptr [rip + 0x1e4a065], 0            ; [0x3f19326] (bss)
020cf2c1: mov    r14d, r9d
020cf2c4: mov    rsi, r8
020cf2c7: mov    rdi, rdx
020cf2ca: mov    rbp, rcx
020cf2cd: jne    0x1820cf31e
020cf2cf: lea    rcx, [rip + 0x1bd1482]                   ; [0x3ca0758] metamethod:Method$System.Collections.Generic.List<long>.Add()
020cf2d6: call   0x182f609b0
020cf2db: lea    rcx, [rip + 0x1c3f0f6]                   ; [0x3d0e3d8] metamethod:Method$System.Collections.Generic.List<double>.Add()
020cf2e2: call   0x182f609b0
020cf2e7: lea    rcx, [rip + 0x1c03d12]                   ; [0x3cd3000] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>.Add()
020cf2ee: call   0x182f609b0
020cf2f3: lea    rcx, [rip + 0x1c055c6]                   ; [0x3cd48c0] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Count()
020cf2fa: call   0x182f609b0
020cf2ff: lea    rcx, [rip + 0x1c0567a]                   ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
020cf306: call   0x182f609b0
020cf30b: lea    rcx, [rip + 0x1bd1d5e]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020cf312: call   0x182f609b0
020cf317: mov    byte ptr [rip + 0x1e4a008], 1            ; [0x3f19326] (bss)
020cf31e: mov    qword ptr [rsp + 0x90], rbx
020cf326: movaps xmmword ptr [rsp + 0x50], xmm6
020cf32b: movaps xmmword ptr [rsp + 0x30], xmm8
020cf331: test   rdi, rdi
020cf334: je     0x1820cf5af
020cf33a: mov    rbx, qword ptr [rsp + 0xb0]
020cf342: mov    ecx, dword ptr [rbx]
020cf344: nop    dword ptr [rax]
020cf348: nop    dword ptr [rax + rax]
020cf350: mov    rax, qword ptr [rdi + 0xe0]
020cf357: test   rax, rax
020cf35a: je     0x1820cf5af
020cf360: mov    eax, dword ptr [rax + 0x18]
020cf363: dec    eax
020cf365: cmp    ecx, eax
020cf367: jge    0x1820cf393
020cf369: mov    r8, qword ptr [rip + 0x1c05610]          ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
020cf370: mov    edx, ecx
020cf372: mov    rcx, qword ptr [rdi + 0xe0]
020cf379: inc    edx
020cf37b: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cf380: test   rax, rax
020cf383: je     0x1820cf5af
020cf389: cmp    qword ptr [rax + 0x10], rsi
020cf38d: jg     0x1820cf393
020cf38f: inc    dword ptr [rbx]
020cf391: jmp    0x1820cf342
020cf393: mov    rcx, qword ptr [rdi + 0xe0]
020cf39a: test   rcx, rcx
020cf39d: je     0x1820cf5af
020cf3a3: mov    r8, qword ptr [rip + 0x1c055d6]          ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
020cf3aa: mov    edx, dword ptr [rbx]
020cf3ac: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cf3b1: test   rax, rax
020cf3b4: je     0x1820cf5af
020cf3ba: mov    rcx, qword ptr [rip + 0x1bd1caf]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020cf3c1: movsd  xmm8, qword ptr [rax + 0x20]
020cf3c7: mov    rbx, qword ptr [rax + 0x10]
020cf3cb: movaps xmmword ptr [rsp + 0x40], xmm7
020cf3d0: cmp    dword ptr [rcx + 0xe0], 0
020cf3d7: movsd  xmm7, qword ptr [rax + 0x18]
020cf3dc: movaps xmmword ptr [rsp + 0x20], xmm9
020cf3e2: movsd  xmm9, qword ptr [rdi + 0xb0]
020cf3eb: jne    0x1820cf3f2
020cf3ed: call   0x182f60cf0
020cf3f2: mov    rcx, qword ptr [rbp + 0x10]
020cf3f6: xorps  xmm6, xmm6
020cf3f9: mulsd  xmm7, xmm9
020cf3fe: mov    rax, rsi
020cf401: sub    rax, rbx
020cf404: movaps xmm9, xmmword ptr [rsp + 0x20]
020cf40a: cvtsi2sd xmm6, rax
020cf40f: mulsd  xmm6, qword ptr [rip + 0xf967b9]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
020cf417: divsd  xmm6, xmm7
020cf41b: movaps xmm7, xmmword ptr [rsp + 0x40]
020cf420: test   rcx, rcx
020cf423: je     0x1820cf5af
020cf429: mov    r9, qword ptr [rip + 0x1c3efa8]          ; [0x3d0e3d8] metamethod:Method$System.Collections.Generic.List<double>.Add()
020cf430: inc    dword ptr [rcx + 0x1c]
020cf433: mov    rdx, qword ptr [rcx + 0x10]
020cf437: test   rdx, rdx
020cf43a: je     0x1820cf5af
020cf440: movsxd r8, dword ptr [rcx + 0x18]
020cf444: addsd  xmm6, xmm8
020cf449: cmp    r8d, dword ptr [rdx + 0x18]
020cf44d: jb     0x1820cf468
020cf44f: mov    rax, qword ptr [r9 + 0x20]
020cf453: movaps xmm1, xmm6
020cf456: mov    r8, qword ptr [rax + 0xc0]
020cf45d: mov    r8, qword ptr [r8 + 0x70]
020cf461: call   0x180746ea0                              ; System.Collections.Generic.List<double>$$AddWithResize
020cf466: jmp    0x1820cf480
020cf468: lea    eax, [r8 + 1]
020cf46c: mov    dword ptr [rcx + 0x18], eax
020cf46f: cmp    r8d, dword ptr [rdx + 0x18]
020cf473: jae    0x1820cf5b5
020cf479: movsd  qword ptr [rdx + r8*8 + 0x20], xmm6
020cf480: mov    rcx, qword ptr [rbp + 0x18]
020cf484: test   rcx, rcx
020cf487: je     0x1820cf5af
020cf48d: mov    r9, qword ptr [rip + 0x1c03b6c]          ; [0x3cd3000] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>.Add()
020cf494: inc    dword ptr [rcx + 0x1c]
020cf497: mov    rdx, qword ptr [rcx + 0x10]
020cf49b: test   rdx, rdx
020cf49e: je     0x1820cf5af
020cf4a4: movsxd r8, dword ptr [rcx + 0x18]
020cf4a8: cmp    r8d, dword ptr [rdx + 0x18]
020cf4ac: jb     0x1820cf4c7
020cf4ae: mov    rax, qword ptr [r9 + 0x20]
020cf4b2: mov    edx, r14d
020cf4b5: mov    r8, qword ptr [rax + 0xc0]
020cf4bc: mov    r8, qword ptr [r8 + 0x70]
020cf4c0: call   0x180756100                              ; System.Collections.Generic.List<UInt32Enum>$$AddWithResize
020cf4c5: jmp    0x1820cf4dd
020cf4c7: lea    eax, [r8 + 1]
020cf4cb: mov    dword ptr [rcx + 0x18], eax
020cf4ce: cmp    r8d, dword ptr [rdx + 0x18]
020cf4d2: jae    0x1820cf5b5
020cf4d8: mov    dword ptr [rdx + r8*4 + 0x20], r14d
020cf4dd: test   r14d, r14d
020cf4e0: jne    0x1820cf57c
020cf4e6: mov    rcx, qword ptr [rbp + 0x20]
020cf4ea: test   rcx, rcx
020cf4ed: je     0x1820cf5af
020cf4f3: mov    r9, qword ptr [rip + 0x1bd125e]          ; [0x3ca0758] metamethod:Method$System.Collections.Generic.List<long>.Add()
020cf4fa: inc    dword ptr [rcx + 0x1c]
020cf4fd: mov    rdx, qword ptr [rcx + 0x10]
020cf501: test   rdx, rdx
020cf504: je     0x1820cf5af
020cf50a: movsxd r8, dword ptr [rcx + 0x18]
020cf50e: cmp    r8d, dword ptr [rdx + 0x18]
020cf512: jb     0x1820cf52d
020cf514: mov    rax, qword ptr [r9 + 0x20]
020cf518: mov    rdx, rsi
020cf51b: mov    r8, qword ptr [rax + 0xc0]
020cf522: mov    r8, qword ptr [r8 + 0x70]
020cf526: call   0x1807561d0                              ; System.Collections.Generic.List<ulong>$$AddWithResize
020cf52b: jmp    0x1820cf53f
020cf52d: lea    eax, [r8 + 1]
020cf531: mov    dword ptr [rcx + 0x18], eax
020cf534: cmp    r8d, dword ptr [rdx + 0x18]
020cf538: jae    0x1820cf5b5
020cf53a: mov    qword ptr [rdx + r8*8 + 0x20], rsi
020cf53f: mov    rcx, qword ptr [rbp + 0x28]
020cf543: test   rcx, rcx
020cf546: je     0x1820cf5af
020cf548: mov    r9, qword ptr [rip + 0x1c3ee89]          ; [0x3d0e3d8] metamethod:Method$System.Collections.Generic.List<double>.Add()
020cf54f: inc    dword ptr [rcx + 0x1c]
020cf552: mov    rdx, qword ptr [rcx + 0x10]
020cf556: test   rdx, rdx
020cf559: je     0x1820cf5af
020cf55b: movsxd r8, dword ptr [rcx + 0x18]
020cf55f: cmp    r8d, dword ptr [rdx + 0x18]
020cf563: jb     0x1820cf599
020cf565: mov    rax, qword ptr [r9 + 0x20]
020cf569: movaps xmm1, xmm6
020cf56c: mov    r8, qword ptr [rax + 0xc0]
020cf573: mov    r8, qword ptr [r8 + 0x70]
020cf577: call   0x180746ea0                              ; System.Collections.Generic.List<double>$$AddWithResize
020cf57c: movaps xmm8, xmmword ptr [rsp + 0x30]
020cf582: movaps xmm6, xmmword ptr [rsp + 0x50]
020cf587: mov    rbx, qword ptr [rsp + 0x90]
020cf58f: add    rsp, 0x68
020cf593: pop    r14
020cf595: pop    rdi
020cf596: pop    rsi
020cf597: pop    rbp
020cf598: ret    
020cf599: lea    eax, [r8 + 1]
020cf59d: mov    dword ptr [rcx + 0x18], eax
020cf5a0: cmp    r8d, dword ptr [rdx + 0x18]
020cf5a4: jae    0x1820cf5b5
020cf5a6: movsd  qword ptr [rdx + r8*8 + 0x20], xmm6
020cf5ad: jmp    0x1820cf57c
020cf5af: call   0x182f60c50
020cf5b4: int3   
020cf5b5: call   0x182f60c40
020cf5ba: int3   
020cf5bb: int3   
