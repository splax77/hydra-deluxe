020f2720: mov    qword ptr [rsp + 0x10], rbx
020f2725: push   rdi
020f2726: sub    rsp, 0x80
020f272d: cmp    byte ptr [rip + 0x1e26c32], 0            ; [0x3f19366] (bss)
020f2734: mov    rbx, rdx
020f2737: mov    rdi, rcx
020f273a: jne    0x1820f277f
020f273c: lea    rcx, [rip + 0x1c0ff65]                   ; [0x3d026a8] metamethod:Method$System.Nullable<Hash>.GetValueOrDefault()
020f2743: call   0x182f609b0
020f2748: lea    rcx, [rip + 0x1c10019]                   ; [0x3d02768] metamethod:Method$System.Nullable<Hash>.get_HasValue()
020f274f: call   0x182f609b0
020f2754: lea    rcx, [rip + 0x1bb6c95]                   ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
020f275b: call   0x182f609b0
020f2760: lea    rcx, [rip + 0x1ba66d9]                   ; [0x3c98e40] metamethod:Method$ʽʺˀʹʹʸʴʻʶʵʹ.ʷʿʽʲʸʺʼʽʺʿʺ<double>()
020f2767: call   0x182f609b0
020f276c: lea    rcx, [rip + 0x1baaa8d]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020f2773: call   0x182f609b0
020f2778: mov    byte ptr [rip + 0x1e26be7], 1            ; [0x3f19366] (bss)
020f277f: mov    qword ptr [rsp + 0x90], rsi
020f2787: xorps  xmm0, xmm0
020f278a: movups xmmword ptr [rsp + 0x60], xmm0
020f278f: movups xmmword ptr [rsp + 0x70], xmm0
020f2794: test   rbx, rbx
020f2797: je     0x1820f29a3
020f279d: mov    rcx, qword ptr [rbx + 0x20]
020f27a1: test   rcx, rcx
020f27a4: je     0x1820f29a3
020f27aa: xor    r8d, r8d
020f27ad: mov    rdx, rdi
020f27b0: call   0x182108ba0                              ; ʼʵʴʹʻʿʲʶʷʹʴ$$ʿʻʵʸʸʳʸʵʵʶʳ
020f27b5: mov    rax, qword ptr [rbx + 0x20]
020f27b9: test   rax, rax
020f27bc: je     0x1820f29a3
020f27c2: cmp    byte ptr [rax + 0x60], 3
020f27c6: je     0x1820f27e0
020f27c8: mov    rcx, qword ptr [rbx + 0x28]
020f27cc: test   rcx, rcx
020f27cf: je     0x1820f29a3
020f27d5: xor    r8d, r8d
020f27d8: mov    rdx, rdi
020f27db: call   0x182131de0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʿʻʵʸʸʳʸʵʵʶʳ
020f27e0: mov    rax, qword ptr [rbx + 0x20]
020f27e4: test   rax, rax
020f27e7: je     0x1820f29a3
020f27ed: movzx  ecx, byte ptr [rax + 0x60]
020f27f1: test   cl, 0xfc
020f27f4: jne    0x1820f2882
020f27fa: cmp    cl, 2
020f27fd: je     0x1820f2887
020f2803: mov    rcx, qword ptr [rip + 0x1baa9f6]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020f280a: mov    rsi, qword ptr [rbx + 0x18]
020f280e: cmp    dword ptr [rcx + 0xe0], 0
020f2815: jne    0x1820f281c
020f2817: call   0x182f60cf0
020f281c: xor    r8d, r8d
020f281f: mov    rdx, rdi
020f2822: mov    rcx, rsi
020f2825: call   0x1820e2370                              ; ʾʵʻʹʾʹʴˁʼʶˁ$$ʷʲʲʹʾʽʻʵʿʼˁ
020f282a: mov    rcx, qword ptr [rbx + 0x10]
020f282e: xor    r8d, r8d
020f2831: mov    rdx, rdi
020f2834: call   0x1820e1bb0                              ; ʾʵʻʹʾʹʴˁʼʶˁ$$ʲˀʷʻʽʾʸʸʶʻˀ
020f2839: mov    rax, qword ptr [rbx + 0x20]
020f283d: test   rax, rax
020f2840: je     0x1820f29a3
020f2846: cmp    byte ptr [rax + 0x60], 1
020f284a: jne    0x1820f296d
020f2850: mov    rax, qword ptr [rbx + 0x58]
020f2854: test   rax, rax
020f2857: je     0x1820f29a3
020f285d: mov    esi, dword ptr [rax + 0x18]
020f2860: xor    ecx, ecx
020f2862: call   0x1801bd810
020f2867: mov    rdx, rax
020f286a: test   rax, rax
020f286d: je     0x1820f29a3
020f2873: cmp    dword ptr [rax + 0x18], 0
020f2877: jbe    0x1820f29a9
020f287d: jmp    0x1820f2923
020f2882: cmp    cl, 2
020f2885: jne    0x1820f2839
020f2887: cmp    byte ptr [rbx + 0x30], 0
020f288b: je     0x1820f2897
020f288d: movups xmm0, xmmword ptr [rbx + 0x31]
020f2891: movups xmm1, xmmword ptr [rbx + 0x41]
020f2895: jmp    0x1820f28b8
020f2897: mov    rdx, qword ptr [rbx + 0x18]
020f289b: test   rdx, rdx
020f289e: je     0x1820f29af
020f28a4: xor    r8d, r8d
020f28a7: lea    rcx, [rsp + 0x40]
020f28ac: call   0x18215f280                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʽʳʸʿʷʽʼˁʸʻʼ
020f28b1: movups xmm0, xmmword ptr [rax]
020f28b4: movups xmm1, xmmword ptr [rax + 0x10]
020f28b8: xor    r8d, r8d
020f28bb: lea    rdx, [rsp + 0x60]
020f28c0: lea    rcx, [rsp + 0x40]
020f28c5: movups xmmword ptr [rsp + 0x60], xmm0
020f28ca: movups xmmword ptr [rsp + 0x70], xmm1
020f28cf: call   0x180002aa0                              ; Blake3.Hash$$AsSpan
020f28d4: mov    r8, qword ptr [rip + 0x1bb6b15]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
020f28db: lea    rdx, [rsp + 0x30]
020f28e0: lea    rcx, [rsp + 0x40]
020f28e5: movups xmm0, xmmword ptr [rax]
020f28e8: movaps xmmword ptr [rsp + 0x30], xmm0
020f28ed: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020f28f2: test   rdi, rdi
020f28f5: je     0x1820f29a3
020f28fb: mov    rax, qword ptr [rdi]
020f28fe: lea    rdx, [rsp + 0x40]
020f2903: movaps xmm0, xmmword ptr [rsp + 0x40]
020f2908: mov    rcx, rdi
020f290b: movdqa xmmword ptr [rsp + 0x40], xmm0
020f2911: mov    r8, qword ptr [rax + 0x3b0]
020f2918: call   qword ptr [rax + 0x3a8]
020f291e: jmp    0x1820f2839
020f2923: mov    dword ptr [rax + 0x20], esi
020f2926: test   rdi, rdi
020f2929: je     0x1820f29a3
020f292b: mov    r10, qword ptr [rdi]
020f292e: mov    r9d, 4
020f2934: xor    r8d, r8d
020f2937: mov    rcx, rdi
020f293a: mov    rax, qword ptr [r10 + 0x3a0]
020f2941: mov    qword ptr [rsp + 0x20], rax
020f2946: call   qword ptr [r10 + 0x398]
020f294d: mov    r8, qword ptr [rbx + 0x58]
020f2951: test   r8, r8
020f2954: je     0x1820f29a3
020f2956: mov    r9, qword ptr [rip + 0x1ba64e3]          ; [0x3c98e40] metamethod:Method$ʽʺˀʹʹʸʴʻʶʵʹ.ʷʿʽʲʸʺʼʽʺʿʺ<double>()
020f295d: mov    rcx, rdi
020f2960: mov    r8d, dword ptr [r8 + 0x18]
020f2964: mov    rdx, qword ptr [rbx + 0x58]
020f2968: call   0x1805dec80                              ; ʽʺˀʹʹʸʴʻʶʵʹ$$ʷʿʽʲʸʺʼʽʺʿʺ<double>
020f296d: mov    rax, qword ptr [rbx + 0x20]
020f2971: test   rax, rax
020f2974: je     0x1820f29a3
020f2976: cmp    byte ptr [rax + 0x60], 2
020f297a: ja     0x1820f298a
020f297c: xor    r8d, r8d
020f297f: mov    rdx, rbx
020f2982: mov    rcx, rdi
020f2985: call   0x1820f2c90                              ; ʾʾʷʴʶʽʾʴˁʼʶ$$ʼʷʴʲʽʻʾʻʷʻʿ
020f298a: mov    rsi, qword ptr [rsp + 0x90]
020f2992: mov    rbx, qword ptr [rsp + 0x98]
020f299a: add    rsp, 0x80
020f29a1: pop    rdi
020f29a2: ret    
020f29a3: call   0x182f60c50
020f29a8: int3   
020f29a9: call   0x182f60c40
020f29ae: int3   
020f29af: lea    rcx, [rip + 0x1bf36ba]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020f29b6: call   0x182f609d0
020f29bb: mov    rcx, rax
020f29be: call   0x182f60c00
020f29c3: lea    rcx, [rip + 0x1bc815e]                   ; [0x3cbab28] str:'Track hash is null and chart is null. Cannot write external chart replay.'
020f29ca: mov    rbx, rax
020f29cd: call   0x182f609d0
020f29d2: mov    rdx, rax
020f29d5: xor    r8d, r8d
020f29d8: mov    rcx, rbx
020f29db: call   0x181b92e30                              ; System.Exception$$.ctor
020f29e0: lea    rcx, [rip + 0x1ba8241]                   ; [0x3c9ac28] metamethod:Method$ʾʾʷʴʶʽʾʴˁʼʶ.ʷʷʿʷʹʳʸʿʿʸˁ()
020f29e7: call   0x182f609d0
020f29ec: mov    rdx, rax
020f29ef: mov    rcx, rbx
020f29f2: call   0x182f60c10
020f29f7: int3   
020f29f8: int3   
