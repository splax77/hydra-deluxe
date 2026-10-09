0215c560: mov    qword ptr [rsp + 0x18], rbx
0215c565: push   rbp
0215c566: push   rdi
0215c567: push   r15
0215c569: sub    rsp, 0x20
0215c56d: cmp    byte ptr [rip + 0x1dbd1ba], 0            ; [0x3f1972e] (bss)
0215c574: mov    rdi, r9
0215c577: movsxd r15, r8d
0215c57a: mov    rbx, rdx
0215c57d: mov    rbp, rcx
0215c580: jne    0x18215c5b9
0215c582: lea    rcx, [rip + 0x1b760b7]                   ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215c589: call   0x182f609b0
0215c58e: lea    rcx, [rip + 0x1b7616b]                   ; [0x3cd2700] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Clear()
0215c595: call   0x182f609b0
0215c59a: lea    rcx, [rip + 0x1b7b17f]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215c5a1: call   0x182f609b0
0215c5a6: lea    rcx, [rip + 0x1b7b233]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c5ad: call   0x182f609b0
0215c5b2: mov    byte ptr [rip + 0x1dbd175], 1            ; [0x3f1972e] (bss)
0215c5b9: mov    qword ptr [rsp + 0x40], rsi
0215c5be: mov    qword ptr [rsp + 0x48], r14
0215c5c3: test   rbx, rbx
0215c5c6: je     0x18215c74a
0215c5cc: mov    r14, qword ptr [rbx + 0x10]
0215c5d0: sub    r14, r15
0215c5d3: cmp    r14, rdi
0215c5d6: cmovl  r14, rdi
0215c5da: mov    esi, dword ptr [rbx + 0x28]
0215c5dd: mov    edi, esi
0215c5df: test   esi, esi
0215c5e1: js     0x18215c614
0215c5e3: mov    rcx, qword ptr [rbp + 0x60]
0215c5e7: test   rcx, rcx
0215c5ea: je     0x18215c74a
0215c5f0: mov    r8, qword ptr [rip + 0x1b7b1e9]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c5f7: mov    edx, edi
0215c5f9: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c5fe: test   rax, rax
0215c601: je     0x18215c74a
0215c607: cmp    qword ptr [rax + 0x10], r14
0215c60b: jl     0x18215c614
0215c60d: mov    esi, edi
0215c60f: sub    edi, 1
0215c612: jns    0x18215c5e3
0215c614: cmp    esi, dword ptr [rbx + 0x28]
0215c617: je     0x18215c732
0215c61d: mov    rcx, qword ptr [rbp + 0x60]
0215c621: test   rcx, rcx
0215c624: je     0x18215c74a
0215c62a: mov    r8, qword ptr [rip + 0x1b7b1af]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c631: mov    edx, esi
0215c633: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c638: test   rax, rax
0215c63b: je     0x18215c74a
0215c641: cmp    qword ptr [rax + 0x10], r14
0215c645: jl     0x18215c732
0215c64b: mov    rdx, qword ptr [rbx + 0x10]
0215c64f: sub    rdx, qword ptr [rax + 0x10]
0215c653: cmp    rdx, r15
0215c656: jge    0x18215c732
0215c65c: mov    rax, qword ptr [rax + 0x10]
0215c660: mov    qword ptr [rbx + 0x10], rax
0215c664: mov    byte ptr [rbx + 0x35], 1
0215c668: mov    rax, qword ptr [rbp + 0x60]
0215c66c: test   rax, rax
0215c66f: je     0x18215c74a
0215c675: mov    ecx, esi
0215c677: xor    r14d, r14d
0215c67a: nop    word ptr [rax + rax]
0215c680: cmp    ecx, dword ptr [rax + 0x18]
0215c683: jge    0x18215c732
0215c689: mov    rcx, qword ptr [rbp + 0x60]
0215c68d: test   rcx, rcx
0215c690: je     0x18215c74a
0215c696: mov    r8, qword ptr [rip + 0x1b7b143]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c69d: mov    edx, esi
0215c69f: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c6a4: mov    rdi, rax
0215c6a7: test   rax, rax
0215c6aa: je     0x18215c74a
0215c6b0: mov    ecx, dword ptr [rbx + 0x28]
0215c6b3: cmp    dword ptr [rax + 0x28], ecx
0215c6b6: je     0x18215c732
0215c6b8: mov    ecx, dword ptr [rbx + 0x20]
0215c6bb: cmp    dword ptr [rax + 0x20], ecx
0215c6be: jne    0x18215c6ef
0215c6c0: mov    ecx, dword ptr [rax + 0x24]
0215c6c3: mov    dword ptr [rbx + 0x24], ecx
0215c6c6: mov    rcx, qword ptr [rbx + 0x38]
0215c6ca: test   rcx, rcx
0215c6cd: je     0x18215c74a
0215c6cf: inc    dword ptr [rcx + 0x1c]
0215c6d2: mov    dword ptr [rcx + 0x18], r14d
0215c6d6: mov    rcx, qword ptr [rbx + 0x38]
0215c6da: test   rcx, rcx
0215c6dd: je     0x18215c74a
0215c6df: mov    r8, qword ptr [rip + 0x1b75f5a]          ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215c6e6: mov    rdx, qword ptr [rax + 0x38]
0215c6ea: call   0x1807b4980                              ; System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>$$AddRange
0215c6ef: cmp    dword ptr [rdi + 0x20], 0xe
0215c6f3: je     0x18215c6fb
0215c6f5: cmp    dword ptr [rdi + 0x20], 0xf
0215c6f9: jne    0x18215c720
0215c6fb: cmp    dword ptr [rbx + 0x20], 0xe
0215c6ff: je     0x18215c707
0215c701: cmp    dword ptr [rbx + 0x20], 0xf
0215c705: jne    0x18215c720
0215c707: mov    ecx, dword ptr [rbx + 0x24]
0215c70a: and    ecx, 0xffffff3f
0215c710: mov    dword ptr [rbx + 0x24], ecx
0215c713: mov    eax, dword ptr [rdi + 0x24]
0215c716: and    eax, 0xc0
0215c71b: or     eax, ecx
0215c71d: mov    dword ptr [rbx + 0x24], eax
0215c720: mov    rax, qword ptr [rbp + 0x60]
0215c724: inc    esi
0215c726: mov    ecx, esi
0215c728: test   rax, rax
0215c72b: je     0x18215c74a
0215c72d: jmp    0x18215c680
0215c732: mov    r14, qword ptr [rsp + 0x48]
0215c737: mov    rsi, qword ptr [rsp + 0x40]
0215c73c: mov    rbx, qword ptr [rsp + 0x50]
0215c741: add    rsp, 0x20
0215c745: pop    r15
0215c747: pop    rdi
0215c748: pop    rbp
0215c749: ret    
0215c74a: call   0x182f60c50
0215c74f: int3   
0215c750: push   rbp
0215c752: push   rbx
0215c753: push   rsi
0215c754: push   rdi
0215c755: push   r12
0215c757: push   r13
0215c759: push   r14
0215c75b: push   r15
0215c75d: sub    rsp, 0x88
0215c764: lea    rbp, [rsp + 0x30]
0215c769: cmp    byte ptr [rip + 0x1dbcfc7], 0            ; [0x3f19737] (bss)
0215c770: mov    r13, r8
0215c773: movaps xmmword ptr [rbp + 0x40], xmm6
0215c777: mov    r12, rdx
0215c77a: mov    rbx, rcx
0215c77d: jne    0x18215c832
0215c783: lea    rcx, [rip + 0x1bacbde]                   ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
0215c78a: call   0x182f609b0
0215c78f: lea    rcx, [rip + 0x1bad3e2]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215c796: call   0x182f609b0
0215c79b: lea    rcx, [rip + 0x1bad54e]                   ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0215c7a2: call   0x182f609b0
0215c7a7: lea    rcx, [rip + 0x1b4517a]                   ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215c7ae: call   0x182f609b0
0215c7b3: lea    rcx, [rip + 0x1b45226]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215c7ba: call   0x182f609b0
0215c7bf: lea    rcx, [rip + 0x1b452ca]                   ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
0215c7c6: call   0x182f609b0
0215c7cb: lea    rcx, [rip + 0x1b4558e]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
0215c7d2: call   0x182f609b0
0215c7d7: lea    rcx, [rip + 0x1b4cef2]                   ; [0x3ca96d0] metamethod:Method$System.Span<char>..ctor()
0215c7de: call   0x182f609b0
0215c7e3: lea    rcx, [rip + 0x1b4d6ce]                   ; [0x3ca9eb8] metamethod:Method$System.Span<char>.get_Length()
0215c7ea: call   0x182f609b0
0215c7ef: lea    rcx, [rip + 0x1b4d77a]                   ; [0x3ca9f70] metamethod:Method$System.Span<char>.op_Implicit()
0215c7f6: call   0x182f609b0
0215c7fb: lea    rcx, [rip + 0x1bad7a6]                   ; [0x3d09fa8] metamethod:Method$ʷʿʽʽʵʻʶʹʼʼʺ.ʴʼʻʹʾʲʼʹʷʸʳ()
0215c802: call   0x182f609b0
0215c807: lea    rcx, [rip + 0x1b8d812]                   ; [0x3cea020] metamethod:Method$System.ValueTuple<ˁʹʴʸʸʲʸˁʺʸʸ, string, long>..ctor()
0215c80e: call   0x182f609b0
0215c813: lea    rcx, [rip + 0x1b8948e]                   ; [0x3ce5ca8] str:'mix'
0215c81a: call   0x182f609b0
0215c81f: lea    rcx, [rip + 0x1b8c422]                   ; [0x3ce8c48] str:'drums'
0215c826: call   0x182f609b0
0215c82b: mov    byte ptr [rip + 0x1dbcf05], 1            ; [0x3f19737] (bss)
0215c832: xorps  xmm0, xmm0
0215c835: lea    rdx, [rbp + 0x10]
0215c839: movups xmmword ptr [rbp + 0x30], xmm0
0215c83d: xor    eax, eax
0215c83f: xor    r15d, r15d
0215c842: movups xmm0, xmmword ptr [rbx]
0215c845: mov    dword ptr [rbp + 0xa0], r15d
0215c84c: xor    r8d, r8d
0215c84f: xorps  xmm1, xmm1
0215c852: lea    rcx, [rbp]
0215c856: movups xmmword ptr [r13], xmm1
0215c85b: mov    qword ptr [r13 + 0x10], rax
0215c85f: movaps xmmword ptr [rbp + 0x10], xmm0
0215c863: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
0215c868: mov    ebx, dword ptr [rax + 8]
0215c86b: mov    rdi, qword ptr [rax]
0215c86e: test   ebx, ebx
0215c870: jle    0x18215c8be
0215c872: cmp    word ptr [rdi], 0x5b
0215c876: jne    0x18215c8be
0215c878: lea    ecx, [rbx - 1]
0215c87b: cmp    ecx, ebx
0215c87d: jae    0x18215cc73
0215c883: mov    r14, qword ptr [rip + 0x1b45156]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215c88a: movsxd rax, ecx
0215c88d: cmp    word ptr [rdi + rax*2], 0x5d
0215c892: lea    eax, [rbx - 1]
0215c895: cmovne ecx, ebx
0215c898: lea    esi, [rcx - 1]
0215c89b: cmp    esi, eax
0215c89d: jbe    0x18215c8a6
0215c89f: xor    ecx, ecx
0215c8a1: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c8a6: mov    rcx, qword ptr [r14 + 0x20]
0215c8aa: add    rdi, 2
0215c8ae: test   byte ptr [rcx + 0x135], 1
0215c8b5: jne    0x18215c8bc
0215c8b7: call   0x182f65750
0215c8bc: mov    ebx, esi
0215c8be: movsxd r8, ebx
0215c8c1: add    r8, r8
0215c8c4: je     0x18215c8ec
0215c8c6: lea    rax, [r8 + 0xf]
0215c8ca: cmp    rax, r8
0215c8cd: ja     0x18215c8d9
0215c8cf: movabs rax, 0xffffffffffffff0
0215c8d9: and    rax, 0xfffffffffffffff0
0215c8dd: call   0x18302b960
0215c8e2: sub    rsp, rax
0215c8e5: lea    rsi, [rsp + 0x30]
0215c8ea: jmp    0x18215c8ef
0215c8ec: mov    rsi, r15
0215c8ef: xor    edx, edx
0215c8f1: mov    rcx, rsi
0215c8f4: call   0x18305d9d0
0215c8f9: xorps  xmm0, xmm0
0215c8fc: movups xmmword ptr [rbp + 0x10], xmm0
0215c900: mov    r8d, 0x20
0215c906: test   ebx, ebx
0215c908: jns    0x18215c9e0
0215c90e: xor    ecx, ecx
0215c910: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c915: mov    eax, dword ptr [rbp + 0x1c]
0215c918: lea    rdx, [rbp]
0215c91c: mov    r8, qword ptr [rip + 0x1b4d64d]          ; [0x3ca9f70] metamethod:Method$System.Span<char>.op_Implicit()
0215c923: lea    rcx, [rbp + 0x10]
0215c927: mov    dword ptr [rbp + 0xc], eax
0215c92a: mov    qword ptr [rbp], rsi
0215c92e: mov    dword ptr [rbp + 8], ebx
0215c931: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
0215c936: mov    eax, dword ptr [rbp + 0x1c]
0215c939: lea    rcx, [rbp]
0215c93d: mov    r14, qword ptr [rbp + 0x10]
0215c941: mov    edx, 0x20
0215c946: mov    r8, qword ptr [rip + 0x1baca1b]          ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
0215c94d: mov    esi, dword ptr [rbp + 0x18]
0215c950: mov    qword ptr [rbp], r14
0215c954: mov    dword ptr [rbp + 0xc], eax
0215c957: mov    dword ptr [rbp + 8], esi
0215c95a: call   0x182c616c0
0215c95f: movsxd rbx, eax
0215c962: cmp    ebx, -1
0215c965: je     0x18215cc5c
0215c96b: mov    rdi, qword ptr [rip + 0x1b4506e]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215c972: cmp    ebx, esi
0215c974: jbe    0x18215c97d
0215c976: xor    ecx, ecx
0215c978: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c97d: mov    rcx, qword ptr [rdi + 0x20]
0215c981: mov    dword ptr [rbp + 0xc], r15d
0215c985: test   byte ptr [rcx + 0x135], 1
0215c98c: jne    0x18215c993
0215c98e: call   0x182f65750
0215c993: cmp    byte ptr [rip + 0x1daf7c1], r15b         ; [0x3f0c15b] (bss)
0215c99a: mov    rdi, qword ptr [rip + 0x1b89307]         ; [0x3ce5ca8] str:'mix'
0215c9a1: mov    qword ptr [rbp], r14
0215c9a5: mov    dword ptr [rbp + 8], ebx
0215c9a8: jne    0x18215c9bd
0215c9aa: lea    rcx, [rip + 0x1b44bf7]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215c9b1: call   0x182f609b0
0215c9b6: mov    byte ptr [rip + 0x1daf79e], 1            ; [0x3f0c15b] (bss)
0215c9bd: test   rdi, rdi
0215c9c0: je     0x18215ca19
0215c9c2: xor    edx, edx
0215c9c4: mov    dword ptr [rbp + 0x1c], r15d
0215c9c8: mov    rcx, rdi
0215c9cb: call   0x1819829d0                              ; System.String$$GetRawStringData
0215c9d0: mov    qword ptr [rbp + 0x10], rax
0215c9d4: mov    eax, dword ptr [rdi + 0x10]
0215c9d7: mov    dword ptr [rbp + 0x18], eax
0215c9da: movaps xmm0, xmmword ptr [rbp + 0x10]
0215c9de: jmp    0x18215ca1c
0215c9e0: mov    ecx, r15d
0215c9e3: test   ebx, ebx
0215c9e5: jle    0x18215c915
0215c9eb: nop    dword ptr [rax + rax]
0215c9f0: cmp    ecx, ebx
0215c9f2: jae    0x18215cc73
0215c9f8: mov    eax, ecx
0215c9fa: movzx  edx, word ptr [rdi + rax*2]
0215c9fe: mov    eax, ecx
0215ca00: cmp    dx, 0x5f
0215ca04: jne    0x18215ca0a
0215ca06: movzx  edx, r8w
0215ca0a: inc    ecx
0215ca0c: mov    word ptr [rsi + rax*2], dx
0215ca10: cmp    ecx, ebx
0215ca12: jl     0x18215c9f2
0215ca14: jmp    0x18215c915
0215ca19: xorps  xmm0, xmm0
0215ca1c: mov    r8, qword ptr [rip + 0x1bad155]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215ca23: lea    rdx, [rbp + 0x10]
0215ca27: movdqa xmmword ptr [rbp + 0x10], xmm0
0215ca2c: lea    rcx, [rbp]
0215ca30: movaps xmm0, xmmword ptr [rbp]
0215ca34: movdqa xmmword ptr [rbp], xmm0
0215ca39: call   0x182c61850
0215ca3e: test   al, al
0215ca40: je     0x18215cc5c
0215ca46: mov    rdi, qword ptr [rip + 0x1b44edb]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215ca4d: cmp    ebx, esi
0215ca4f: jbe    0x18215ca58
0215ca51: xor    ecx, ecx
0215ca53: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215ca58: mov    rcx, qword ptr [rdi + 0x20]
0215ca5c: lea    r14, [r14 + rbx*2]
0215ca60: mov    dword ptr [rbp + 0x1c], r15d
0215ca64: test   byte ptr [rcx + 0x135], 1
0215ca6b: jne    0x18215ca72
0215ca6d: call   0x182f65750
0215ca72: mov    qword ptr [rbp + 0x10], r14
0215ca76: lea    rdx, [rbp + 0x10]
0215ca7a: sub    esi, ebx
0215ca7c: lea    rcx, [rbp]
0215ca80: mov    dword ptr [rbp + 0x18], esi
0215ca83: xor    r8d, r8d
0215ca86: movaps xmm0, xmmword ptr [rbp + 0x10]
0215ca8a: movdqa xmmword ptr [rbp + 0x10], xmm0
0215ca8f: call   0x181b53de0                              ; System.MemoryExtensions$$TrimStart
0215ca94: mov    r8, qword ptr [rip + 0x1bac8cd]          ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
0215ca9b: lea    rcx, [rbp + 0x10]
0215ca9f: mov    edx, 0x20
0215caa4: mov    r14, qword ptr [rax]
0215caa7: mov    edi, dword ptr [rax + 8]
0215caaa: mov    eax, dword ptr [rax + 0xc]
0215caad: mov    qword ptr [rbp + 0x10], r14
0215cab1: mov    dword ptr [rbp + 0x1c], eax
0215cab4: mov    dword ptr [rbp + 0x18], edi
0215cab7: call   0x182c616c0
0215cabc: movsxd rbx, eax
0215cabf: cmp    ebx, -1
0215cac2: je     0x18215cc5c
0215cac8: mov    rsi, qword ptr [rip + 0x1b44f11]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215cacf: cmp    ebx, edi
0215cad1: jbe    0x18215cada
0215cad3: xor    ecx, ecx
0215cad5: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215cada: mov    rcx, qword ptr [rsi + 0x20]
0215cade: mov    dword ptr [rbp + 0x1c], r15d
0215cae2: test   byte ptr [rcx + 0x135], 1
0215cae9: jne    0x18215caf0
0215caeb: call   0x182f65750
0215caf0: mov    qword ptr [rbp + 0x10], r14
0215caf4: lea    rdx, [rbp + 0xa0]
0215cafb: mov    dword ptr [rbp + 0x18], ebx
0215cafe: lea    rcx, [rbp + 0x10]
0215cb02: movaps xmm0, xmmword ptr [rbp + 0x10]
0215cb06: xor    r8d, r8d
0215cb09: movdqa xmmword ptr [rbp + 0x10], xmm0
0215cb0e: call   0x181b4f510                              ; System.Int32$$TryParse
0215cb13: test   al, al
0215cb15: je     0x18215cc5c
0215cb1b: mov    ecx, dword ptr [rbp + 0xa0]
0215cb21: xor    edx, edx
0215cb23: call   0x18210d990                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʻˁˁʿʶʷʴˀʵʻʺ
0215cb28: movzx  r15d, al
0215cb2c: cmp    al, 0xff
0215cb2e: je     0x18215cc5c
0215cb34: mov    rsi, qword ptr [rip + 0x1b44ded]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215cb3b: cmp    ebx, edi
0215cb3d: jbe    0x18215cb46
0215cb3f: xor    ecx, ecx
0215cb41: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215cb46: mov    rcx, qword ptr [rsi + 0x20]
0215cb4a: lea    r14, [r14 + rbx*2]
0215cb4e: mov    dword ptr [rbp + 0x1c], 0
0215cb55: test   byte ptr [rcx + 0x135], 1
0215cb5c: jne    0x18215cb63
0215cb5e: call   0x182f65750
0215cb63: mov    qword ptr [rbp + 0x10], r14
0215cb67: lea    rdx, [rbp + 0x10]
0215cb6b: sub    edi, ebx
0215cb6d: lea    rcx, [rbp]
0215cb71: mov    dword ptr [rbp + 0x18], edi
0215cb74: xor    r8d, r8d
0215cb77: movaps xmm0, xmmword ptr [rbp + 0x10]
0215cb7b: movdqa xmmword ptr [rbp + 0x10], xmm0
0215cb80: call   0x181b53de0                              ; System.MemoryExtensions$$TrimStart
0215cb85: cmp    byte ptr [rip + 0x1daf5cf], 0            ; [0x3f0c15b] (bss)
0215cb8c: mov    rbx, qword ptr [rip + 0x1b8c0b5]         ; [0x3ce8c48] str:'drums'
0215cb93: movups xmm0, xmmword ptr [rax]
0215cb96: movups xmmword ptr [rbp + 0x30], xmm0
0215cb9a: movups xmm6, xmmword ptr [rax]
0215cb9d: jne    0x18215cbb2
0215cb9f: lea    rcx, [rip + 0x1b44a02]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215cba6: call   0x182f609b0
0215cbab: mov    byte ptr [rip + 0x1daf5a9], 1            ; [0x3f0c15b] (bss)
0215cbb2: test   rbx, rbx
0215cbb5: je     0x18215cbd8
0215cbb7: xor    edx, edx
0215cbb9: mov    dword ptr [rbp + 0x1c], 0
0215cbc0: mov    rcx, rbx
0215cbc3: call   0x1819829d0                              ; System.String$$GetRawStringData
0215cbc8: mov    qword ptr [rbp + 0x10], rax
0215cbcc: mov    eax, dword ptr [rbx + 0x10]
0215cbcf: mov    dword ptr [rbp + 0x18], eax
0215cbd2: movaps xmm0, xmmword ptr [rbp + 0x10]
0215cbd6: jmp    0x18215cbdb
0215cbd8: xorps  xmm0, xmm0
0215cbdb: mov    r8, qword ptr [rip + 0x1bad10e]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0215cbe2: lea    rdx, [rbp + 0x10]
0215cbe6: lea    rcx, [rbp]
0215cbea: movdqa xmmword ptr [rbp + 0x10], xmm0
0215cbef: movdqa xmmword ptr [rbp], xmm6
0215cbf4: call   0x182c61990
0215cbf9: test   al, al
0215cbfb: je     0x18215cc5c
0215cbfd: mov    rdx, qword ptr [rip + 0x1b44e8c]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
0215cc04: lea    rcx, [rbp + 0x30]
0215cc08: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
0215cc0d: xor    ecx, ecx
0215cc0f: xorps  xmm0, xmm0
0215cc12: mov    qword ptr [rbp + 0x20], rcx
0215cc16: mov    r9, r12
0215cc19: mov    rcx, qword ptr [rip + 0x1b8d400]         ; [0x3cea020] metamethod:Method$System.ValueTuple<ˁʹʴʸʸʲʸˁʺʸʸ, string, long>..ctor()
0215cc20: mov    r8, rax
0215cc23: mov    qword ptr [rsp + 0x20], rcx
0215cc28: movzx  edx, r15b
0215cc2c: lea    rcx, [rbp + 0x10]
0215cc30: movups xmmword ptr [rbp + 0x10], xmm0
0215cc34: call   0x180d059f0                              ; System.ValueTuple<SByteEnum, object, long>$$.ctor
0215cc39: movups xmm0, xmmword ptr [rbp + 0x10]
0215cc3d: lea    rcx, [r13 + 8]
0215cc41: xor    edx, edx
0215cc43: movsd  xmm1, qword ptr [rbp + 0x20]
0215cc48: movups xmmword ptr [r13], xmm0
0215cc4d: movsd  qword ptr [r13 + 0x10], xmm1
0215cc53: call   0x182f5fc00
0215cc58: mov    al, 1
0215cc5a: jmp    0x18215cc5e
0215cc5c: xor    al, al
0215cc5e: movaps xmm6, xmmword ptr [rbp + 0x40]
0215cc62: lea    rsp, [rbp + 0x58]
0215cc66: pop    r15
0215cc68: pop    r14
0215cc6a: pop    r13
0215cc6c: pop    r12
0215cc6e: pop    rdi
0215cc6f: pop    rsi
0215cc70: pop    rbx
0215cc71: pop    rbp
0215cc72: ret    
0215cc73: call   0x182f60c40
0215cc78: int3   
0215cc79: int3   
