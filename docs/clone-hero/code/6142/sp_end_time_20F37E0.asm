020f37e0: mov    qword ptr [rsp + 0x18], rbx
020f37e5: push   rbp
020f37e6: sub    rsp, 0x50
020f37ea: cmp    byte ptr [rip + 0x1e25bd7], 0            ; [0x3f193c8] (bss)
020f37f1: mov    rbp, rdx
020f37f4: movaps xmmword ptr [rsp + 0x40], xmm6
020f37f9: mov    rbx, rcx
020f37fc: movaps xmm6, xmm2
020f37ff: jne    0x1820f3820
020f3801: lea    rcx, [rip + 0x1bd9740]                   ; [0x3cccf48] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Count()
020f3808: call   0x182f609b0
020f380d: lea    rcx, [rip + 0x1bd97f4]                   ; [0x3ccd008] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Item()
020f3814: call   0x182f609b0
020f3819: mov    byte ptr [rip + 0x1e25ba8], 1            ; [0x3f193c8] (bss)
020f3820: mov    rax, qword ptr [rbx + 0x230]
020f3827: mov    qword ptr [rsp + 0x60], rsi
020f382c: mov    qword ptr [rsp + 0x68], rdi
020f3831: movaps xmmword ptr [rsp + 0x30], xmm7
020f3836: movaps xmmword ptr [rsp + 0x20], xmm8
020f383c: test   rax, rax
020f383f: je     0x1820f395e
020f3845: mov    rdi, qword ptr [rbx + 0x208]
020f384c: xor    r8d, r8d
020f384f: movsd  xmm8, qword ptr [rax + 0xb0]
020f3858: mov    rcx, rdi
020f385b: movaps xmm1, xmm6
020f385e: call   0x18216a720                              ; ʿʴʴʾʾˁʲʹʲʾʳ$$ʳʴʼʼʽʸʲʹʼʹˀ
020f3863: mov    rsi, rax
020f3866: test   rax, rax
020f3869: je     0x1820f395e
020f386f: mov    rax, qword ptr [rbx + 0x230]
020f3876: test   rax, rax
020f3879: je     0x1820f395e
020f387f: subsd  xmm6, qword ptr [rsi + 0x18]
020f3884: movsd  xmm2, qword ptr [rax + 0xb0]
020f388c: xor    r9d, r9d
020f388f: mov    rdx, rsi
020f3892: movaps xmm0, xmm6
020f3895: call   0x1820d1d20                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʷʾʻʿʵʻʸʵʺʹʺ
020f389a: xorps  xmm6, xmm6
020f389d: cvtsi2sd xmm6, rbp
020f38a2: test   rdi, rdi
020f38a5: je     0x1820f395e
020f38ab: movsd  xmm7, qword ptr [rsi + 0x18]
020f38b0: addsd  xmm6, xmm0
020f38b4: mov    ebx, dword ptr [rsi + 0x20]
020f38b7: cmp    ebx, dword ptr [rdi + 0x18]
020f38ba: jge    0x1820f3936
020f38bc: mov    r8, qword ptr [rip + 0x1bd9745]          ; [0x3ccd008] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Item()
020f38c3: mov    edx, ebx
020f38c5: mov    rcx, rdi
020f38c8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f38cd: mov    ecx, dword ptr [rdi + 0x18]
020f38d0: mov    rsi, rax
020f38d3: dec    ecx
020f38d5: cmp    ebx, ecx
020f38d7: jge    0x1820f3920
020f38d9: mov    r8, qword ptr [rip + 0x1bd9728]          ; [0x3ccd008] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Item()
020f38e0: lea    edx, [rbx + 1]
020f38e3: mov    rcx, rdi
020f38e6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f38eb: test   rax, rax
020f38ee: je     0x1820f395e
020f38f0: test   rsi, rsi
020f38f3: je     0x1820f395e
020f38f5: movsd  xmm0, qword ptr [rax + 0x38]
020f38fa: subsd  xmm0, qword ptr [rsi + 0x38]
020f38ff: comisd xmm6, xmm0
020f3903: jb     0x1820f3920
020f3905: xor    r9d, r9d
020f3908: subsd  xmm6, xmm0
020f390c: movaps xmm2, xmm8
020f3910: mov    rdx, rsi
020f3913: call   0x1820d4a70                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʽʻʾʺʻʼˀˀʻʲʲ
020f3918: addsd  xmm7, xmm0
020f391c: inc    ebx
020f391e: jmp    0x1820f38b7
020f3920: xor    r9d, r9d
020f3923: movaps xmm2, xmm8
020f3927: mov    rdx, rsi
020f392a: movaps xmm0, xmm6
020f392d: call   0x1820d4a70                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʽʻʾʺʻʼˀˀʻʲʲ
020f3932: addsd  xmm7, xmm0
020f3936: movaps xmm8, xmmword ptr [rsp + 0x20]
020f393c: movaps xmm0, xmm7
020f393f: movaps xmm7, xmmword ptr [rsp + 0x30]
020f3944: mov    rdi, qword ptr [rsp + 0x68]
020f3949: mov    rsi, qword ptr [rsp + 0x60]
020f394e: mov    rbx, qword ptr [rsp + 0x70]
020f3953: movaps xmm6, xmmword ptr [rsp + 0x40]
020f3958: add    rsp, 0x50
020f395c: pop    rbp
020f395d: ret    
020f395e: call   0x182f60c50
020f3963: int3   
020f3964: int3   
