02151650: push   rbx
02151652: push   rbp
02151653: push   rsi
02151654: push   rdi
02151655: push   r14
02151657: push   r15
02151659: sub    rsp, 0x48
0215165d: mov    r14, qword ptr [r9]
02151660: mov    r15, r9
02151663: mov    qword ptr [rsp + 0x98], 0
0215166f: mov    edi, edx
02151671: mov    rsi, rcx
02151674: test   r14, r14
02151677: je     0x182151824
0215167d: movups xmm0, xmmword ptr [r8]
02151681: mov    r14, qword ptr [r14 + 0x30]
02151685: lea    rcx, [rsp + 0x30]
0215168a: xor    edx, edx
0215168c: movaps xmmword ptr [rsp + 0x30], xmm0
02151691: call   0x182151330
02151696: movzx  ebp, ax
02151699: movzx  ebx, ax
0215169c: shr    bp, 8
021516a0: movsx  eax, bpl
021516a4: sub    eax, 0xc
021516a7: cmp    eax, 1
021516aa: jbe    0x1821517ff
021516b0: cmp    bpl, 0xf
021516b4: je     0x1821517ff
021516ba: cmp    bl, 0xff
021516bd: je     0x1821517d9
021516c3: test   r14, r14
021516c6: je     0x182151824
021516cc: cmp    byte ptr [rip + 0x1dbccd5], 0            ; [0x3f0e3a8] (bss)
021516d3: jne    0x1821516e8
021516d5: lea    rcx, [rip + 0x1b7fd64]                   ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
021516dc: call   0x182f609b0
021516e1: mov    byte ptr [rip + 0x1dbccc0], 1            ; [0x3f0e3a8] (bss)
021516e8: xor    r9d, r9d
021516eb: movzx  r8d, bpl
021516ef: movzx  edx, bl
021516f2: mov    rcx, r14
021516f5: call   0x1803931c0
021516fa: cmp    eax, -1
021516fd: je     0x182151774
021516ff: mov    rcx, qword ptr [r14 + 0xd0]
02151706: test   rcx, rcx
02151709: je     0x182151824
0215170f: mov    r8, qword ptr [rip + 0x1b7fd2a]          ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
02151716: mov    edx, eax
02151718: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215171d: mov    rdx, rax
02151720: mov    qword ptr [rsp + 0x98], rax
02151728: lea    rcx, [rsp + 0x98]
02151730: call   0x182f5fc00
02151735: mov    rax, qword ptr [rsp + 0x98]
0215173d: test   rax, rax
02151740: je     0x18215178f
02151742: cmp    byte ptr [rax + 0xa0], 0
02151749: je     0x18215178f
0215174b: mov    rdx, qword ptr [rsi]
0215174e: mov    ecx, dword ptr [rsi + 8]
02151751: cmp    edi, ecx
02151753: jae    0x18215182a
02151759: movsxd rax, edi
0215175c: inc    edi
0215175e: cmp    word ptr [rdx + rax*2], 0x7d
02151763: jne    0x182151751
02151765: mov    eax, edi
02151767: add    rsp, 0x48
0215176b: pop    r15
0215176d: pop    r14
0215176f: pop    rdi
02151770: pop    rsi
02151771: pop    rbp
02151772: pop    rbx
02151773: ret    
02151774: xor    edx, edx
02151776: mov    qword ptr [rsp + 0x98], 0
02151782: lea    rcx, [rsp + 0x98]
0215178a: call   0x182f5fc00
0215178f: xor    r9d, r9d
02151792: movzx  r8d, bl
02151796: movzx  edx, bpl
0215179a: mov    rcx, r14
0215179d: call   0x1820cfd40
021517a2: movups xmm0, xmmword ptr [rsi]
021517a5: mov    qword ptr [rsp + 0x28], 0
021517ae: lea    rdx, [rsp + 0x30]
021517b3: movzx  r9d, bpl
021517b7: mov    qword ptr [rsp + 0x20], r15
021517bc: mov    r8d, edi
021517bf: movaps xmmword ptr [rsp + 0x30], xmm0
021517c4: mov    rcx, rax
021517c7: call   0x18213dd40                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʺʼˁʳˁˀʻʿʵˀʴ
021517cc: add    rsp, 0x48
021517d0: pop    r15
021517d2: pop    r14
021517d4: pop    rdi
021517d5: pop    rsi
021517d6: pop    rbp
021517d7: pop    rbx
021517d8: ret    
021517d9: mov    rdx, qword ptr [rsi]
021517dc: mov    ecx, dword ptr [rsi + 8]
021517df: nop    
021517e0: cmp    edi, ecx
021517e2: jae    0x18215182a
021517e4: movsxd rax, edi
021517e7: inc    edi
021517e9: cmp    word ptr [rdx + rax*2], 0x7d
021517ee: jne    0x1821517e0
021517f0: mov    eax, edi
021517f2: add    rsp, 0x48
021517f6: pop    r15
021517f8: pop    r14
021517fa: pop    rdi
021517fb: pop    rsi
021517fc: pop    rbp
021517fd: pop    rbx
021517fe: ret    
021517ff: mov    rdx, qword ptr [rsi]
02151802: mov    ecx, dword ptr [rsi + 8]
02151805: cmp    edi, ecx
02151807: jae    0x18215182a
02151809: movsxd rax, edi
0215180c: inc    edi
0215180e: cmp    word ptr [rdx + rax*2], 0x7d
02151813: jne    0x182151805
02151815: mov    eax, edi
02151817: add    rsp, 0x48
0215181b: pop    r15
0215181d: pop    r14
0215181f: pop    rdi
02151820: pop    rsi
02151821: pop    rbp
02151822: pop    rbx
02151823: ret    
02151824: call   0x182f60c50
02151829: int3   
0215182a: call   0x182f60c40
0215182f: int3   
02151830: mov    qword ptr [rsp + 8], rcx
02151835: push   rbp
02151836: push   rbx
02151837: push   rdi
02151838: push   r15
0215183a: lea    rbp, [rsp - 0xc8]
02151842: sub    rsp, 0x1c8
02151849: cmp    byte ptr [rip + 0x1dc7f0e], 0            ; [0x3f1975e] (bss)
02151850: mov    rbx, r8
02151853: mov    edi, edx
02151855: mov    r15, rcx
02151858: jne    0x182151985
0215185e: lea    rcx, [rip + 0x1b74d73]                   ; [0x3cc65d8] meta:char_TypeInfo
02151865: call   0x182f609b0
0215186a: lea    rcx, [rip + 0x1b41287]                   ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
02151871: call   0x182f609b0
02151876: lea    rcx, [rip + 0x1bbd263]                   ; [0x3d0eae0] meta:long_TypeInfo
0215187d: call   0x182f609b0
02151882: lea    rcx, [rip + 0x1bb82ef]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02151889: call   0x182f609b0
0215188e: lea    rcx, [rip + 0x1bb845b]                   ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02151895: call   0x182f609b0
0215189a: lea    rcx, [rip + 0x1b50087]                   ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
021518a1: call   0x182f609b0
021518a6: lea    rcx, [rip + 0x1b50133]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
021518ad: call   0x182f609b0
021518b2: lea    rcx, [rip + 0x1b501d7]                   ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
021518b9: call   0x182f609b0
021518be: lea    rcx, [rip + 0x1b5049b]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
021518c5: call   0x182f609b0
021518ca: lea    rcx, [rip + 0x1b50547]                   ; [0x3ca1e18] metamethod:Method$System.ReadOnlySpan<char>.op_Equality()
021518d1: call   0x182f609b0
021518d6: lea    rcx, [rip + 0x1b857fb]                   ; [0x3cd70d8] meta:string_TypeInfo
021518dd: call   0x182f609b0
021518e2: lea    rcx, [rip + 0x1b42f77]                   ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
021518e9: call   0x182f609b0
021518ee: lea    rcx, [rip + 0x1b430e3]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
021518f5: call   0x182f609b0
021518fa: lea    rcx, [rip + 0x1b43777]                   ; [0x3c95078] meta:ʶʿʺʴʻʽʶʽʺʼʺ_TypeInfo
02151901: call   0x182f609b0
02151906: lea    rcx, [rip + 0x1b41cb3]                   ; [0x3c935c0] str:'prc'
0215190d: call   0x182f609b0
02151912: lea    rcx, [rip + 0x1bb537f]                   ; [0x3d06c98] str:'phrase_start'
02151919: call   0x182f609b0
0215191e: lea    rcx, [rip + 0x1b77aeb]                   ; [0x3cc9410] str:'coda'
02151925: call   0x182f609b0
0215192a: lea    rcx, [rip + 0x1b58947]                   ; [0x3caa278] str:'section'
02151931: call   0x182f609b0
02151936: lea    rcx, [rip + 0x1bb52a3]                   ; [0x3d06be0] str:'phrase_end'
0215193d: call   0x182f609b0
02151942: lea    rcx, [rip + 0x1b4d487]                   ; [0x3c9edd0] str:'The event track event occuring at offset {0} is out of order. It comes after the event at {1} which is illegal.'
02151949: call   0x182f609b0
0215194e: lea    rcx, [rip + 0x1b9fb73]                   ; [0x3cf14c8] str:'end'
02151955: call   0x182f609b0
0215195a: lea    rcx, [rip + 0x1b843a7]                   ; [0x3cd5d08] str:'lyric '
02151961: call   0x182f609b0
02151966: lea    rcx, [rip + 0x1b6840b]                   ; [0x3cb9d78] str:'Expected event string.'
0215196d: call   0x182f609b0
02151972: lea    rcx, [rip + 0x1b684b7]                   ; [0x3cb9e30] str:"Expected event track event, got '{0}' instead."
02151979: call   0x182f609b0
0215197e: mov    byte ptr [rip + 0x1dc7dd9], 1            ; [0x3f1975e] (bss)
02151985: mov    rax, qword ptr [rbx]
02151988: xorps  xmm0, xmm0
0215198b: mov    qword ptr [rsp + 0x1f8], rsi
02151993: mov    qword ptr [rsp + 0x1c0], r12
0215199b: mov    qword ptr [rsp + 0x1b8], r13
021519a3: mov    qword ptr [rsp + 0x1b0], r14
021519ab: mov    qword ptr [rsp + 0x78], 0
021519b4: mov    qword ptr [rsp + 0x70], 0
021519bd: movups xmmword ptr [rsp + 0x60], xmm0
021519c2: test   rax, rax
021519c5: je     0x1821525c7
021519cb: cmp    edi, dword ptr [r15 + 8]
021519cf: jae    0x1821525cd
021519d5: movzx  ecx, byte ptr [rax + 0x2c]
021519d9: xor    r12d, r12d
021519dc: mov    byte ptr [rbp + 0x100], cl
021519e2: mov    rcx, qword ptr [rax + 0x30]
021519e6: mov    qword ptr [rsp + 0x30], rcx
021519eb: mov    rcx, qword ptr [r15]
021519ee: nop    
021519f0: movsxd rax, edi
021519f3: movzx  edx, word ptr [rcx + rax*2]
021519f7: cmp    edx, 0x20
021519fa: jbe    0x182151a3a
021519fc: cmp    edx, 0x7b
021519ff: je     0x1821525ba
02151a05: cmp    edx, 0x7d
02151a08: jne    0x182151a6a
02151a0a: mov    r14, qword ptr [rsp + 0x1b0]
02151a12: lea    eax, [rdi + 1]
02151a15: mov    r13, qword ptr [rsp + 0x1b8]
02151a1d: mov    r12, qword ptr [rsp + 0x1c0]
02151a25: mov    rsi, qword ptr [rsp + 0x1f8]
02151a2d: add    rsp, 0x1c8
02151a34: pop    r15
02151a36: pop    rdi
02151a37: pop    rbx
02151a38: pop    rbp
02151a39: ret    
02151a3a: mov    ecx, edx
02151a3c: sub    ecx, 9
02151a3f: je     0x1821525ba
02151a45: sub    ecx, 1
02151a48: je     0x1821525ba
02151a4e: sub    ecx, 1
02151a51: je     0x182151a6a
02151a53: sub    ecx, 1
02151a56: je     0x182151a6a
02151a58: cmp    ecx, 1
02151a5b: je     0x1821525ba
02151a61: cmp    edx, 0x20
02151a64: je     0x1821525ba
02151a6a: mov    rcx, qword ptr [rip + 0x1b42f67]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151a71: mov    rsi, qword ptr [r15]
02151a74: mov    r14d, dword ptr [r15 + 8]
02151a78: mov    ebx, dword ptr [r15 + 0xc]
02151a7c: cmp    dword ptr [rcx + 0xe0], 0
02151a83: jne    0x182151a8a
02151a85: call   0x182f60cf0
02151a8a: cmp    byte ptr [rip + 0x1dc7bd5], 0            ; [0x3f19666] (bss)
02151a91: jne    0x182151ab2
02151a93: lea    rcx, [rip + 0x1b42f3e]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151a9a: call   0x182f609b0
02151a9f: lea    rcx, [rip + 0x1b95a12]                   ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
02151aa6: call   0x182f609b0
02151aab: mov    byte ptr [rip + 0x1dc7bb4], 1            ; [0x3f19666] (bss)
02151ab2: mov    rcx, qword ptr [rip + 0x1b42f1f]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151ab9: cmp    dword ptr [rcx + 0xe0], 0
02151ac0: jne    0x182151ac7
02151ac2: call   0x182f60cf0
02151ac7: xor    r10d, r10d
02151aca: nop    word ptr [rax + rax]
02151ad0: cmp    edi, r14d
02151ad3: jae    0x1821526ef
02151ad9: movsxd rax, edi
02151adc: movzx  edx, word ptr [rsi + rax*2]
02151ae0: lea    eax, [rdx - 0x30]
02151ae3: cmp    ax, 9
02151ae7: ja     0x182151af9
02151ae9: lea    r10, [r10 + r10*4]
02151aed: inc    edi
02151aef: lea    r10, [r10 - 0x18]
02151af3: lea    r10, [rdx + r10*2]
02151af7: jmp    0x182151ad0
02151af9: movsxd rax, edi
02151afc: cmp    word ptr [rsi + rax*2], 0x20
02151b01: jne    0x1821526c2
02151b07: lea    eax, [rdi + 1]
02151b0a: cmp    eax, r14d
02151b0d: jae    0x1821526ef
02151b13: movsxd rax, edi
02151b16: cmp    word ptr [rsi + rax*2 + 2], 0x3d
02151b1c: jne    0x1821526c2
02151b22: lea    eax, [rdi + 2]
02151b25: cmp    eax, r14d
02151b28: jae    0x1821526ef
02151b2e: movsxd rax, edi
02151b31: cmp    word ptr [rsi + rax*2 + 4], 0x20
02151b37: jne    0x1821526c2
02151b3d: lea    ecx, [rdi + 3]
02151b40: cmp    ecx, r14d
02151b43: jae    0x1821526ef
02151b49: lea    eax, [rdi + 4]
02151b4c: cmp    eax, r14d
02151b4f: jae    0x1821526ef
02151b55: cdqe   
02151b57: mov    r8d, 5
02151b5d: cmp    word ptr [rsi + rax*2], 0x20
02151b62: je     0x182151b6a
02151b64: mov    r8d, 6
02151b6a: mov    r9, qword ptr [rip + 0x1b95947]          ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
02151b71: add    edi, r8d
02151b74: movsxd rax, ecx
02151b77: xorps  xmm0, xmm0
02151b7a: mov    r8, r10
02151b7d: lea    rcx, [rbp - 0x80]
02151b81: movzx  edx, word ptr [rsi + rax*2]
02151b85: movups xmmword ptr [rbp - 0x80], xmm0
02151b89: call   0x180ce1560                              ; System.ValueTuple<short, long>$$.ctor
02151b8e: movzx  eax, word ptr [rbp - 0x80]
02151b92: cmp    ax, 0x45
02151b96: jne    0x182152513
02151b9c: mov    r13, qword ptr [rbp - 0x78]
02151ba0: cmp    r13, r12
02151ba3: jl     0x182152600
02151ba9: mov    rcx, qword ptr [rip + 0x1b42e28]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151bb0: mov    r14, rsi
02151bb3: mov    ebx, dword ptr [r15 + 8]
02151bb7: cmp    dword ptr [rcx + 0xe0], 0
02151bbe: jne    0x182151bc5
02151bc0: call   0x182f60cf0
02151bc5: mov    esi, edi
02151bc7: cmp    esi, ebx
02151bc9: jae    0x1821525cd
02151bcf: movsxd rax, esi
02151bd2: cmp    word ptr [r14 + rax*2], 0x22
02151bd8: je     0x182151bde
02151bda: inc    esi
02151bdc: jmp    0x182151bc7
02151bde: cmp    byte ptr [rip + 0x1dbc47d], 0            ; [0x3f0e062] (bss)
02151be5: mov    r15d, edi
02151be8: jne    0x182151bfd
02151bea: lea    rcx, [rip + 0x1b5016f]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02151bf1: call   0x182f609b0
02151bf6: mov    byte ptr [rip + 0x1dbc465], 1            ; [0x3f0e062] (bss)
02151bfd: cmp    edi, ebx
02151bff: jge    0x182151c40
02151c01: cmp    r15d, ebx
02151c04: jae    0x1821525cd
02151c0a: movsxd rax, r15d
02151c0d: movzx  ecx, word ptr [r14 + rax*2]
02151c12: cmp    cx, 0xa
02151c16: je     0x182151c40
02151c18: cmp    cx, 0xd
02151c1c: jne    0x182151c38
02151c1e: lea    eax, [r15 + 1]
02151c22: cmp    eax, ebx
02151c24: jge    0x182151c38
02151c26: jae    0x1821525cd
02151c2c: movsxd rax, r15d
02151c2f: cmp    word ptr [r14 + rax*2 + 2], 0xa
02151c36: je     0x182151c40
02151c38: inc    r15d
02151c3b: cmp    r15d, ebx
02151c3e: jl     0x182151c04
02151c40: mov    r12, qword ptr [rbp + 0xf0]
02151c47: cmp    esi, r15d
02151c4a: jg     0x1821525d3
02151c50: mov    rdi, qword ptr [rip + 0x1b4fd89]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02151c57: lea    ecx, [rsi + 1]
02151c5a: mov    ebx, r15d
02151c5d: sub    ebx, esi
02151c5f: cmp    ecx, dword ptr [r12 + 8]
02151c64: ja     0x182151c71
02151c66: mov    eax, dword ptr [r12 + 8]
02151c6b: sub    eax, ecx
02151c6d: cmp    ebx, eax
02151c6f: jbe    0x182151c78
02151c71: xor    ecx, ecx
02151c73: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02151c78: mov    rax, qword ptr [r12]
02151c7c: movsxd rcx, esi
02151c7f: lea    r14, [rax + rcx*2]
02151c83: mov    rcx, qword ptr [rdi + 0x20]
02151c87: test   byte ptr [rcx + 0x135], 1
02151c8e: jne    0x182151c95
02151c90: call   0x182f65750
02151c95: mov    rcx, qword ptr [rip + 0x1b42d3c]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151c9c: cmp    dword ptr [rcx + 0xe0], 0
02151ca3: jne    0x182151caa
02151ca5: call   0x182f60cf0
02151caa: lea    ecx, [rbx - 1]
02151cad: test   ecx, ecx
02151caf: jle    0x182151cc8
02151cb1: cmp    ecx, ebx
02151cb3: jae    0x1821525cd
02151cb9: cmp    word ptr [r14 + rcx*2 + 2], 0x22
02151cc0: je     0x182151cc8
02151cc2: dec    ecx
02151cc4: test   ecx, ecx
02151cc6: jg     0x182151cb1
02151cc8: mov    rdi, qword ptr [rip + 0x1b4fd11]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02151ccf: dec    ebx
02151cd1: cmp    ecx, -1
02151cd4: cmovne ebx, ecx
02151cd7: inc    esi
02151cd9: cmp    esi, dword ptr [r12 + 8]
02151cde: ja     0x182151ceb
02151ce0: mov    eax, dword ptr [r12 + 8]
02151ce5: sub    eax, esi
02151ce7: cmp    ebx, eax
02151ce9: jbe    0x182151cf2
02151ceb: xor    ecx, ecx
02151ced: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02151cf2: mov    rax, qword ptr [r12]
02151cf6: movsxd rcx, esi
02151cf9: mov    dword ptr [rbp - 0x64], 0
02151d00: lea    rsi, [rax + rcx*2]
02151d04: mov    rcx, qword ptr [rdi + 0x20]
02151d08: test   byte ptr [rcx + 0x135], 1
02151d0f: jne    0x182151d16
02151d11: call   0x182f65750
02151d16: mov    qword ptr [rbp - 0x70], rsi
02151d1a: lea    rdx, [rsp + 0x40]
02151d1f: mov    dword ptr [rbp - 0x68], ebx
02151d22: lea    rcx, [rbp + 0xa0]
02151d29: movaps xmm0, xmmword ptr [rbp - 0x70]
02151d2d: xor    r8d, r8d
02151d30: movdqa xmmword ptr [rsp + 0x40], xmm0
02151d36: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
02151d3b: mov    edi, dword ptr [rax + 8]
02151d3e: mov    r14, qword ptr [rax]
02151d41: mov    ebx, dword ptr [rax + 0xc]
02151d44: test   edi, edi
02151d46: jle    0x182151da2
02151d48: cmp    word ptr [r14], 0x5b
02151d4d: jne    0x182151da2
02151d4f: lea    ecx, [rdi - 1]
02151d52: cmp    ecx, edi
02151d54: jae    0x1821525cd
02151d5a: mov    rsi, qword ptr [rip + 0x1b4fc7f]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02151d61: movsxd rax, ecx
02151d64: cmp    word ptr [r14 + rax*2], 0x5d
02151d6a: lea    eax, [rdi - 1]
02151d6d: cmovne ecx, edi
02151d70: lea    ebx, [rcx - 1]
02151d73: cmp    ebx, eax
02151d75: jbe    0x182151d7e
02151d77: xor    ecx, ecx
02151d79: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02151d7e: mov    rcx, qword ptr [rsi + 0x20]
02151d82: add    r14, 2
02151d86: xorps  xmm0, xmm0
02151d89: movups xmmword ptr [rsp + 0x40], xmm0
02151d8e: test   byte ptr [rcx + 0x135], 1
02151d95: jne    0x182151d9c
02151d97: call   0x182f65750
02151d9c: mov    edi, ebx
02151d9e: mov    ebx, dword ptr [rsp + 0x4c]
02151da2: movzx  r12d, byte ptr [rbp + 0x100]
02151daa: test   r12b, r12b
02151dad: jne    0x182152128
02151db3: cmp    byte ptr [rip + 0x1dba3a1], r12b         ; [0x3f0c15b] (bss)
02151dba: mov    rsi, qword ptr [rip + 0x1b584b7]         ; [0x3caa278] str:'section'
02151dc1: jne    0x182151dd6
02151dc3: lea    rcx, [rip + 0x1b4f7de]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02151dca: call   0x182f609b0
02151dcf: mov    byte ptr [rip + 0x1dba385], 1            ; [0x3f0c15b] (bss)
02151dd6: xorps  xmm0, xmm0
02151dd9: test   rsi, rsi
02151ddc: je     0x182151dfd
02151dde: xor    edx, edx
02151de0: mov    dword ptr [rbp - 0x54], 0
02151de7: mov    rcx, rsi
02151dea: call   0x1819829d0                              ; System.String$$GetRawStringData
02151def: mov    qword ptr [rbp - 0x60], rax
02151df3: mov    eax, dword ptr [rsi + 0x10]
02151df6: mov    dword ptr [rbp - 0x58], eax
02151df9: movaps xmm0, xmmword ptr [rbp - 0x60]
02151dfd: mov    r8, qword ptr [rip + 0x1bb7eec]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02151e04: lea    rdx, [rsp + 0x40]
02151e09: lea    rcx, [rbp - 0x50]
02151e0d: movdqa xmmword ptr [rsp + 0x40], xmm0
02151e13: mov    qword ptr [rbp - 0x50], r14
02151e17: mov    dword ptr [rbp - 0x48], edi
02151e1a: mov    dword ptr [rbp - 0x44], ebx
02151e1d: call   0x182c61990
02151e22: test   al, al
02151e24: jne    0x182151fd8
02151e2a: cmp    byte ptr [rip + 0x1dba32b], al           ; [0x3f0c15b] (bss)
02151e30: mov    rsi, qword ptr [rip + 0x1b41789]         ; [0x3c935c0] str:'prc'
02151e37: jne    0x182151e4c
02151e39: lea    rcx, [rip + 0x1b4f768]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02151e40: call   0x182f609b0
02151e45: mov    byte ptr [rip + 0x1dba30f], 1            ; [0x3f0c15b] (bss)
02151e4c: xorps  xmm0, xmm0
02151e4f: test   rsi, rsi
02151e52: je     0x182151e73
02151e54: xor    edx, edx
02151e56: mov    dword ptr [rbp - 0x34], 0
02151e5d: mov    rcx, rsi
02151e60: call   0x1819829d0                              ; System.String$$GetRawStringData
02151e65: mov    qword ptr [rbp - 0x40], rax
02151e69: mov    eax, dword ptr [rsi + 0x10]
02151e6c: mov    dword ptr [rbp - 0x38], eax
02151e6f: movaps xmm0, xmmword ptr [rbp - 0x40]
02151e73: mov    r8, qword ptr [rip + 0x1bb7e76]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02151e7a: lea    rdx, [rsp + 0x40]
02151e7f: lea    rcx, [rbp - 0x30]
02151e83: movdqa xmmword ptr [rsp + 0x40], xmm0
02151e89: mov    qword ptr [rbp - 0x30], r14
02151e8d: mov    dword ptr [rbp - 0x28], edi
02151e90: mov    dword ptr [rbp - 0x24], ebx
02151e93: call   0x182c61990
02151e98: test   al, al
02151e9a: je     0x182152128
02151ea0: mov    rax, qword ptr [rip + 0x1b41719]         ; [0x3c935c0] str:'prc'
02151ea7: test   rax, rax
02151eaa: je     0x1821525c7
02151eb0: mov    ebx, dword ptr [rax + 0x10]
02151eb3: mov    rsi, qword ptr [rip + 0x1b4fa6e]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02151eba: inc    ebx
02151ebc: cmp    ebx, edi
02151ebe: jbe    0x182151ec7
02151ec0: xor    ecx, ecx
02151ec2: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02151ec7: mov    rcx, qword ptr [rsi + 0x20]
02151ecb: movsxd rax, ebx
02151ece: mov    dword ptr [rbp - 0x14], 0
02151ed5: test   byte ptr [rcx + 0x135], 1
02151edc: lea    r14, [r14 + rax*2]
02151ee0: jne    0x182151ee7
02151ee2: call   0x182f65750
02151ee7: mov    rdx, qword ptr [rip + 0x1b4fba2]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
02151eee: lea    rcx, [rsp + 0x60]
02151ef3: sub    edi, ebx
02151ef5: mov    qword ptr [rbp - 0x20], r14
02151ef9: mov    dword ptr [rbp - 0x18], edi
02151efc: movaps xmm0, xmmword ptr [rbp - 0x20]
02151f00: movdqa xmmword ptr [rsp + 0x60], xmm0
02151f06: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
02151f0b: mov    rcx, qword ptr [rip + 0x1b4294e]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02151f12: mov    rbx, rax
02151f15: cmp    dword ptr [rcx + 0xe0], 0
02151f1c: jne    0x182151f2a
02151f1e: call   0x182f60cf0
02151f23: mov    rcx, qword ptr [rip + 0x1b42936]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02151f2a: mov    rcx, qword ptr [rcx + 0xb8]
02151f31: mov    rcx, qword ptr [rcx]
02151f34: test   rcx, rcx
02151f37: je     0x1821525c7
02151f3d: mov    r9, qword ptr [rip + 0x1b40bb4]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
02151f44: lea    r8, [rsp + 0x70]
02151f49: mov    rdx, rbx
02151f4c: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
02151f51: test   al, al
02151f53: jne    0x182151fa5
02151f55: test   rbx, rbx
02151f58: je     0x1821525c7
02151f5e: mov    edx, 0x5f
02151f63: mov    r8d, 0x20
02151f69: xor    r9d, r9d
02151f6c: mov    rcx, rbx
02151f6f: call   0x181986c10                              ; System.String$$Replace
02151f74: mov    rsi, qword ptr [rsp + 0x30]
02151f79: test   rsi, rsi
02151f7c: je     0x1821525c7
02151f82: xor    r9d, r9d
02151f85: mov    r8, r13
02151f88: mov    rdx, rax
02151f8b: mov    rcx, rsi
02151f8e: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
02151f93: mov    edi, r15d
02151f96: mov    r12, r13
02151f99: mov    r15, qword ptr [rbp + 0xf0]
02151fa0: jmp    0x1821525bc
02151fa5: mov    rsi, qword ptr [rsp + 0x30]
02151faa: test   rsi, rsi
02151fad: je     0x1821525c7
02151fb3: mov    rdx, qword ptr [rsp + 0x70]
02151fb8: xor    r9d, r9d
02151fbb: mov    r8, r13
02151fbe: mov    rcx, rsi
02151fc1: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
02151fc6: mov    edi, r15d
02151fc9: mov    r12, r13
02151fcc: mov    r15, qword ptr [rbp + 0xf0]
02151fd3: jmp    0x1821525bc
02151fd8: mov    rax, qword ptr [rip + 0x1b58299]         ; [0x3caa278] str:'section'
02151fdf: test   rax, rax
02151fe2: je     0x1821525c7
02151fe8: cmp    edi, dword ptr [rax + 0x10]
02151feb: jne    0x182152000
02151fed: mov    rax, qword ptr [rip + 0x1b850e4]         ; [0x3cd70d8] meta:string_TypeInfo
02151ff4: mov    rcx, qword ptr [rax + 0xb8]
02151ffb: mov    rbx, qword ptr [rcx]
02151ffe: jmp    0x18215205e
02152000: mov    ebx, dword ptr [rax + 0x10]
02152003: mov    rsi, qword ptr [rip + 0x1b4f91e]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215200a: inc    ebx
0215200c: cmp    ebx, edi
0215200e: jbe    0x182152017
02152010: xor    ecx, ecx
02152012: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02152017: mov    rcx, qword ptr [rsi + 0x20]
0215201b: movsxd rax, ebx
0215201e: mov    dword ptr [rbp - 4], 0
02152025: test   byte ptr [rcx + 0x135], 1
0215202c: lea    r14, [r14 + rax*2]
02152030: jne    0x182152037
02152032: call   0x182f65750
02152037: mov    rdx, qword ptr [rip + 0x1b4fa52]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
0215203e: lea    rcx, [rsp + 0x60]
02152043: sub    edi, ebx
02152045: mov    qword ptr [rbp - 0x10], r14
02152049: mov    dword ptr [rbp - 8], edi
0215204c: movaps xmm0, xmmword ptr [rbp - 0x10]
02152050: movdqa xmmword ptr [rsp + 0x60], xmm0
02152056: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
0215205b: mov    rbx, rax
0215205e: mov    rcx, qword ptr [rip + 0x1b427fb]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02152065: cmp    dword ptr [rcx + 0xe0], 0
0215206c: jne    0x18215207a
0215206e: call   0x182f60cf0
02152073: mov    rcx, qword ptr [rip + 0x1b427e6]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
0215207a: mov    rcx, qword ptr [rcx + 0xb8]
02152081: mov    rcx, qword ptr [rcx]
02152084: test   rcx, rcx
02152087: je     0x1821525c7
0215208d: mov    r9, qword ptr [rip + 0x1b40a64]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
02152094: lea    r8, [rsp + 0x78]
02152099: mov    rdx, rbx
0215209c: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
021520a1: test   al, al
021520a3: jne    0x1821520f5
021520a5: test   rbx, rbx
021520a8: je     0x1821525c7
021520ae: mov    edx, 0x5f
021520b3: mov    r8d, 0x20
021520b9: xor    r9d, r9d
021520bc: mov    rcx, rbx
021520bf: call   0x181986c10                              ; System.String$$Replace
021520c4: mov    rsi, qword ptr [rsp + 0x30]
021520c9: test   rsi, rsi
021520cc: je     0x1821525c7
021520d2: xor    r9d, r9d
021520d5: mov    r8, r13
021520d8: mov    rdx, rax
021520db: mov    rcx, rsi
021520de: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
021520e3: mov    edi, r15d
021520e6: mov    r12, r13
021520e9: mov    r15, qword ptr [rbp + 0xf0]
021520f0: jmp    0x1821525bc
021520f5: mov    rsi, qword ptr [rsp + 0x30]
021520fa: test   rsi, rsi
021520fd: je     0x1821525c7
02152103: mov    rdx, qword ptr [rsp + 0x78]
02152108: xor    r9d, r9d
0215210b: mov    r8, r13
0215210e: mov    rcx, rsi
02152111: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
02152116: mov    edi, r15d
02152119: mov    r12, r13
0215211c: mov    r15, qword ptr [rbp + 0xf0]
02152123: jmp    0x1821525bc
02152128: cmp    byte ptr [rip + 0x1dba02c], 0            ; [0x3f0c15b] (bss)
0215212f: mov    rsi, qword ptr [rip + 0x1bb4b62]         ; [0x3d06c98] str:'phrase_start'
02152136: jne    0x18215214b
02152138: lea    rcx, [rip + 0x1b4f469]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215213f: call   0x182f609b0
02152144: mov    byte ptr [rip + 0x1dba010], 1            ; [0x3f0c15b] (bss)
0215214b: xorps  xmm0, xmm0
0215214e: test   rsi, rsi
02152151: je     0x182152172
02152153: xor    edx, edx
02152155: mov    dword ptr [rbp + 0xc], 0
0215215c: mov    rcx, rsi
0215215f: call   0x1819829d0                              ; System.String$$GetRawStringData
02152164: mov    qword ptr [rbp], rax
02152168: mov    eax, dword ptr [rsi + 0x10]
0215216b: mov    dword ptr [rbp + 8], eax
0215216e: movaps xmm0, xmmword ptr [rbp]
02152172: mov    r8, qword ptr [rip + 0x1bb7b77]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02152179: lea    rdx, [rsp + 0x40]
0215217e: lea    rcx, [rbp + 0x10]
02152182: movdqa xmmword ptr [rsp + 0x40], xmm0
02152188: mov    qword ptr [rbp + 0x10], r14
0215218c: mov    dword ptr [rbp + 0x18], edi
0215218f: mov    dword ptr [rbp + 0x1c], ebx
02152192: call   0x182c61990
02152197: test   al, al
02152199: jne    0x1821524cc
0215219f: cmp    byte ptr [rip + 0x1db9fb6], al           ; [0x3f0c15b] (bss)
021521a5: mov    rsi, qword ptr [rip + 0x1bb4a34]         ; [0x3d06be0] str:'phrase_end'
021521ac: jne    0x1821521c1
021521ae: lea    rcx, [rip + 0x1b4f3f3]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
021521b5: call   0x182f609b0
021521ba: mov    byte ptr [rip + 0x1db9f9a], 1            ; [0x3f0c15b] (bss)
021521c1: xorps  xmm0, xmm0
021521c4: test   rsi, rsi
021521c7: je     0x1821521e8
021521c9: xor    edx, edx
021521cb: mov    dword ptr [rbp + 0x2c], 0
021521d2: mov    rcx, rsi
021521d5: call   0x1819829d0                              ; System.String$$GetRawStringData
021521da: mov    qword ptr [rbp + 0x20], rax
021521de: mov    eax, dword ptr [rsi + 0x10]
021521e1: mov    dword ptr [rbp + 0x28], eax
021521e4: movaps xmm0, xmmword ptr [rbp + 0x20]
021521e8: mov    r8, qword ptr [rip + 0x1bb7b01]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
021521ef: lea    rdx, [rsp + 0x40]
021521f4: lea    rcx, [rbp + 0x30]
021521f8: movdqa xmmword ptr [rsp + 0x40], xmm0
021521fe: mov    qword ptr [rbp + 0x30], r14
02152202: mov    dword ptr [rbp + 0x38], edi
02152205: mov    dword ptr [rbp + 0x3c], ebx
02152208: call   0x182c61990
0215220d: test   al, al
0215220f: jne    0x1821524b7
02152215: cmp    byte ptr [rip + 0x1db9f40], al           ; [0x3f0c15b] (bss)
0215221b: mov    rsi, qword ptr [rip + 0x1b83ae6]         ; [0x3cd5d08] str:'lyric '
02152222: jne    0x182152237
02152224: lea    rcx, [rip + 0x1b4f37d]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215222b: call   0x182f609b0
02152230: mov    byte ptr [rip + 0x1db9f24], 1            ; [0x3f0c15b] (bss)
02152237: xorps  xmm0, xmm0
0215223a: test   rsi, rsi
0215223d: je     0x18215225e
0215223f: xor    edx, edx
02152241: mov    dword ptr [rbp + 0x4c], 0
02152248: mov    rcx, rsi
0215224b: call   0x1819829d0                              ; System.String$$GetRawStringData
02152250: mov    qword ptr [rbp + 0x40], rax
02152254: mov    eax, dword ptr [rsi + 0x10]
02152257: mov    dword ptr [rbp + 0x48], eax
0215225a: movaps xmm0, xmmword ptr [rbp + 0x40]
0215225e: mov    r8, qword ptr [rip + 0x1bb7a8b]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02152265: lea    rdx, [rsp + 0x40]
0215226a: lea    rcx, [rbp + 0x50]
0215226e: movdqa xmmword ptr [rsp + 0x40], xmm0
02152274: mov    qword ptr [rbp + 0x50], r14
02152278: mov    dword ptr [rbp + 0x58], edi
0215227b: mov    dword ptr [rbp + 0x5c], ebx
0215227e: call   0x182c61990
02152283: test   al, al
02152285: jne    0x18215242f
0215228b: test   r12b, r12b
0215228e: jne    0x182152501
02152294: cmp    byte ptr [rip + 0x1db9ec0], r12b         ; [0x3f0c15b] (bss)
0215229b: mov    rsi, qword ptr [rip + 0x1b9f226]         ; [0x3cf14c8] str:'end'
021522a2: jne    0x1821522b7
021522a4: lea    rcx, [rip + 0x1b4f2fd]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
021522ab: call   0x182f609b0
021522b0: mov    byte ptr [rip + 0x1db9ea4], 1            ; [0x3f0c15b] (bss)
021522b7: xorps  xmm0, xmm0
021522ba: test   rsi, rsi
021522bd: je     0x1821522de
021522bf: xor    edx, edx
021522c1: mov    dword ptr [rbp + 0x6c], 0
021522c8: mov    rcx, rsi
021522cb: call   0x1819829d0                              ; System.String$$GetRawStringData
021522d0: mov    qword ptr [rbp + 0x60], rax
021522d4: mov    eax, dword ptr [rsi + 0x10]
021522d7: mov    dword ptr [rbp + 0x68], eax
021522da: movaps xmm0, xmmword ptr [rbp + 0x60]
021522de: mov    r8, qword ptr [rip + 0x1b4fb33]          ; [0x3ca1e18] metamethod:Method$System.ReadOnlySpan<char>.op_Equality()
021522e5: lea    rdx, [rsp + 0x40]
021522ea: lea    rcx, [rbp + 0x70]
021522ee: movdqa xmmword ptr [rsp + 0x40], xmm0
021522f4: mov    qword ptr [rbp + 0x70], r14
021522f8: mov    dword ptr [rbp + 0x78], edi
021522fb: mov    dword ptr [rbp + 0x7c], ebx
021522fe: call   0x18094e3d0                              ; System.ReadOnlySpan<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Equality
02152303: test   al, al
02152305: jne    0x182152401
0215230b: cmp    byte ptr [rip + 0x1db9e4a], al           ; [0x3f0c15b] (bss)
02152311: mov    rsi, qword ptr [rip + 0x1b770f8]         ; [0x3cc9410] str:'coda'
02152318: jne    0x18215232d
0215231a: lea    rcx, [rip + 0x1b4f287]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02152321: call   0x182f609b0
02152326: mov    byte ptr [rip + 0x1db9e2e], 1            ; [0x3f0c15b] (bss)
0215232d: xorps  xmm0, xmm0
02152330: test   rsi, rsi
02152333: je     0x182152360
02152335: xor    edx, edx
02152337: mov    dword ptr [rbp + 0x8c], 0
02152341: mov    rcx, rsi
02152344: call   0x1819829d0                              ; System.String$$GetRawStringData
02152349: mov    qword ptr [rbp + 0x80], rax
02152350: mov    eax, dword ptr [rsi + 0x10]
02152353: mov    dword ptr [rbp + 0x88], eax
02152359: movaps xmm0, xmmword ptr [rbp + 0x80]
02152360: mov    r8, qword ptr [rip + 0x1bb7811]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02152367: lea    rdx, [rsp + 0x40]
0215236c: lea    rcx, [rbp + 0x90]
02152373: movdqa xmmword ptr [rsp + 0x40], xmm0
02152379: mov    qword ptr [rbp + 0x90], r14
02152380: mov    dword ptr [rbp + 0x98], edi
02152386: mov    dword ptr [rbp + 0x9c], ebx
0215238c: call   0x182c61850
02152391: test   al, al
02152393: je     0x182152501
02152399: mov    rsi, qword ptr [rsp + 0x30]
0215239e: test   rsi, rsi
021523a1: je     0x1821525c7
021523a7: cmp    qword ptr [rsi + 0x100], 0
021523af: jne    0x182152501
021523b5: mov    rcx, qword ptr [rip + 0x1b42cbc]         ; [0x3c95078] meta:ʶʿʺʴʻʽʶʽʺʼʺ_TypeInfo
021523bc: call   0x182f60c00
021523c1: mov    rdx, qword ptr [rip + 0x1b77048]         ; [0x3cc9410] str:'coda'
021523c8: xor    r9d, r9d
021523cb: mov    r8, r13
021523ce: mov    rcx, rax
021523d1: mov    rbx, rax
021523d4: call   0x18211ba30                              ; ʶʿʺʴʻʽʶʽʺʼʺ$$.ctor
021523d9: lea    rcx, [rsi + 0x100]
021523e0: mov    qword ptr [rsi + 0x100], rbx
021523e7: mov    rdx, rbx
021523ea: call   0x182f5fc00
021523ef: mov    edi, r15d
021523f2: mov    r12, r13
021523f5: mov    r15, qword ptr [rbp + 0xf0]
021523fc: jmp    0x1821525bc
02152401: mov    rsi, qword ptr [rsp + 0x30]
02152406: test   rsi, rsi
02152409: je     0x1821525c7
0215240f: xor    r8d, r8d
02152412: mov    rdx, r13
02152415: mov    rcx, rsi
02152418: call   0x18212e310                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ˁʶʲʾʹʻʿʾˁʾʾ
0215241d: mov    edi, r15d
02152420: mov    r12, r13
02152423: mov    r15, qword ptr [rbp + 0xf0]
0215242a: jmp    0x1821525bc
0215242f: mov    rax, qword ptr [rip + 0x1b838d2]         ; [0x3cd5d08] str:'lyric '
02152436: test   rax, rax
02152439: je     0x1821525c7
0215243f: movsxd rbx, dword ptr [rax + 0x10]
02152443: mov    rsi, qword ptr [rip + 0x1b4f4de]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215244a: cmp    ebx, edi
0215244c: jbe    0x182152455
0215244e: xor    ecx, ecx
02152450: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02152455: mov    rcx, qword ptr [rsi + 0x20]
02152459: lea    r14, [r14 + rbx*2]
0215245d: xor    r12d, r12d
02152460: mov    dword ptr [rsp + 0x5c], r12d
02152465: test   byte ptr [rcx + 0x135], 1
0215246c: jne    0x182152473
0215246e: call   0x182f65750
02152473: mov    rdx, qword ptr [rip + 0x1b4f616]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
0215247a: lea    rcx, [rsp + 0x60]
0215247f: sub    edi, ebx
02152481: mov    qword ptr [rsp + 0x50], r14
02152486: mov    dword ptr [rsp + 0x58], edi
0215248a: movaps xmm0, xmmword ptr [rsp + 0x50]
0215248f: movdqa xmmword ptr [rsp + 0x60], xmm0
02152495: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
0215249a: mov    rsi, qword ptr [rsp + 0x30]
0215249f: test   rsi, rsi
021524a2: je     0x1821525c7
021524a8: mov    qword ptr [rsp + 0x20], r12
021524ad: mov    r9, rax
021524b0: mov    edx, 2
021524b5: jmp    0x1821524f6
021524b7: mov    rsi, qword ptr [rsp + 0x30]
021524bc: test   rsi, rsi
021524bf: je     0x1821525c7
021524c5: mov    edx, 1
021524ca: jmp    0x1821524dc
021524cc: mov    rsi, qword ptr [rsp + 0x30]
021524d1: test   rsi, rsi
021524d4: je     0x1821525c7
021524da: xor    edx, edx
021524dc: mov    rax, qword ptr [rip + 0x1b84bf5]         ; [0x3cd70d8] meta:string_TypeInfo
021524e3: mov    qword ptr [rsp + 0x20], 0
021524ec: mov    r9, qword ptr [rax + 0xb8]
021524f3: mov    r9, qword ptr [r9]
021524f6: mov    r8, r13
021524f9: mov    rcx, rsi
021524fc: call   0x182152cc0
02152501: mov    edi, r15d
02152504: mov    r12, r13
02152507: mov    r15, qword ptr [rbp + 0xf0]
0215250e: jmp    0x1821525bc
02152513: cmp    ax, 0x48
02152517: jne    0x18215266e
0215251d: mov    rcx, qword ptr [rip + 0x1b424b4]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02152524: mov    ebx, r14d
02152527: cmp    dword ptr [rcx + 0xe0], 0
0215252e: jne    0x182152535
02152530: call   0x182f60cf0
02152535: cmp    edi, ebx
02152537: jae    0x1821525cd
0215253d: movsxd rax, edi
02152540: movzx  ecx, word ptr [rsi + rax*2]
02152544: sub    cx, 0x30
02152548: cmp    cx, 9
0215254c: ja     0x182152552
0215254e: inc    edi
02152550: jmp    0x182152535
02152552: cmp    byte ptr [rip + 0x1dbbb08], 0            ; [0x3f0e061] (bss)
02152559: jne    0x18215256e
0215255b: lea    rcx, [rip + 0x1b4f7fe]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02152562: call   0x182f609b0
02152567: mov    byte ptr [rip + 0x1dbbaf3], 1            ; [0x3f0e061] (bss)
0215256e: cmp    edi, ebx
02152570: jge    0x1821525a3
02152572: jae    0x1821525cd
02152574: movsxd rax, edi
02152577: movzx  edx, word ptr [rsi + rax*2]
0215257b: mov    ecx, edx
0215257d: sub    ecx, 9
02152580: je     0x18215259b
02152582: sub    ecx, 1
02152585: je     0x18215259b
02152587: sub    ecx, 1
0215258a: je     0x1821525a1
0215258c: sub    ecx, 1
0215258f: je     0x1821525a1
02152591: cmp    ecx, 1
02152594: je     0x18215259b
02152596: cmp    edx, 0x20
02152599: jne    0x1821525a1
0215259b: inc    edi
0215259d: cmp    edi, ebx
0215259f: jl     0x182152572
021525a1: cmp    edi, ebx
021525a3: jae    0x1821525cd
021525a5: movsxd rax, edi
021525a8: movzx  ecx, word ptr [rsi + rax*2]
021525ac: sub    cx, 0x30
021525b0: cmp    cx, 9
021525b4: ja     0x1821525bc
021525b6: inc    edi
021525b8: jmp    0x1821525a1
021525ba: inc    edi
021525bc: cmp    edi, dword ptr [r15 + 8]
021525c0: jae    0x1821525cd
021525c2: jmp    0x1821519eb
021525c7: call   0x182f60c50
021525cc: int3   
021525cd: call   0x182f60c40
021525d2: int3   
021525d3: movups xmm0, xmmword ptr [r12]
021525d8: mov    r9, qword ptr [rip + 0x1b67799]          ; [0x3cb9d78] str:'Expected event string.'
021525df: lea    rdx, [rsp + 0x40]
021525e4: mov    rcx, qword ptr [rsp + 0x30]
021525e9: mov    r8d, edi
021525ec: movaps xmmword ptr [rsp + 0x40], xmm0
021525f1: mov    qword ptr [rsp + 0x20], 0
021525fa: call   0x1821624a0                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʷʴʼʾˁʴʾʶʵʵʷ
021525ff: int3   
02152600: mov    rcx, qword ptr [rip + 0x1bbc4d9]         ; [0x3d0eae0] meta:long_TypeInfo
02152607: lea    rdx, [rbp + 0x100]
0215260e: mov    qword ptr [rbp + 0x100], r13
02152615: call   0x182f5fbe0
0215261a: mov    rcx, qword ptr [rip + 0x1bbc4bf]         ; [0x3d0eae0] meta:long_TypeInfo
02152621: lea    rdx, [rbp - 0x80]
02152625: mov    rbx, rax
02152628: mov    qword ptr [rbp - 0x80], r12
0215262c: call   0x182f5fbe0
02152631: mov    rcx, qword ptr [rip + 0x1b4c798]         ; [0x3c9edd0] str:'The event track event occuring at offset {0} is out of order. It comes after the event at {1} which is illegal.'
02152638: xor    r9d, r9d
0215263b: mov    r8, rax
0215263e: mov    rdx, rbx
02152641: call   0x181982840                              ; System.String$$Format
02152646: movups xmm0, xmmword ptr [r15]
0215264a: mov    rcx, qword ptr [rsp + 0x30]
0215264f: lea    rdx, [rsp + 0x40]
02152654: mov    r9, rax
02152657: mov    qword ptr [rsp + 0x20], 0
02152660: mov    r8d, edi
02152663: movaps xmmword ptr [rsp + 0x40], xmm0
02152668: call   0x1821624a0                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʷʴʼʾˁʴʾʶʵʵʷ
0215266d: int3   
0215266e: mov    rcx, qword ptr [rip + 0x1b73f63]         ; [0x3cc65d8] meta:char_TypeInfo
02152675: lea    rdx, [rbp + 0x100]
0215267c: mov    word ptr [rbp + 0x100], ax
02152683: call   0x182f5fbe0
02152688: mov    rcx, qword ptr [rip + 0x1b677a1]         ; [0x3cb9e30] str:"Expected event track event, got '{0}' instead."
0215268f: xor    r8d, r8d
02152692: mov    rdx, rax
02152695: call   0x181982590                              ; System.String$$Format
0215269a: movups xmm0, xmmword ptr [r15]
0215269e: mov    rcx, qword ptr [rsp + 0x30]
021526a3: lea    rdx, [rsp + 0x40]
021526a8: mov    r9, rax
021526ab: mov    qword ptr [rsp + 0x20], 0
021526b4: mov    r8d, edi
021526b7: movaps xmmword ptr [rsp + 0x40], xmm0
021526bc: call   0x1821624a0                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʷʴʼʾˁʴʾʶʵʵʷ
021526c1: int3   
021526c2: mov    rcx, qword ptr [rsp + 0x30]
021526c7: lea    rdx, [rsp + 0x50]
021526cc: xor    r9d, r9d
021526cf: mov    qword ptr [rsp + 0x50], rsi
021526d4: mov    r8d, edi
021526d7: mov    dword ptr [rsp + 0x58], r14d
021526dc: mov    dword ptr [rsp + 0x5c], ebx
021526e0: mov    qword ptr [rsp + 0x20], 0
021526e9: call   0x182162810                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʸˀʹʺʶʳʴʿʾʻʵ
021526ee: int3   
021526ef: call   0x182f60c40
021526f4: int3   
021526f5: int3   
