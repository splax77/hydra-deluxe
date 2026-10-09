021544e0: mov    qword ptr [rsp + 8], rcx
021544e5: push   rbp
021544e6: push   rsi
021544e7: lea    rbp, [rsp - 0x48]
021544ec: sub    rsp, 0x148
021544f3: cmp    byte ptr [rip + 0x1dc519e], 0            ; [0x3f19698] (bss)
021544fa: mov    rsi, rdx
021544fd: jne    0x1821545da
02154503: lea    rcx, [rip + 0x1b3e5ee]                   ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
0215450a: call   0x182f609b0
0215450f: lea    rcx, [rip + 0x1b7b96a]                   ; [0x3ccfe80] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Count()
02154516: call   0x182f609b0
0215451b: lea    rcx, [rip + 0x1b7ba1e]                   ; [0x3ccff40] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Item()
02154522: call   0x182f609b0
02154527: lea    rcx, [rip + 0x1bb564a]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215452e: call   0x182f609b0
02154533: lea    rcx, [rip + 0x1bb57b6]                   ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0215453a: call   0x182f609b0
0215453f: lea    rcx, [rip + 0x1b4d49a]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02154546: call   0x182f609b0
0215454b: lea    rcx, [rip + 0x1b4d53e]                   ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
02154552: call   0x182f609b0
02154557: lea    rcx, [rip + 0x1b4d802]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
0215455e: call   0x182f609b0
02154563: lea    rcx, [rip + 0x1b402f6]                   ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
0215456a: call   0x182f609b0
0215456f: lea    rcx, [rip + 0x1b40b02]                   ; [0x3c95078] meta:ʶʿʺʴʻʽʶʽʺʼʺ_TypeInfo
02154576: call   0x182f609b0
0215457b: lea    rcx, [rip + 0x1b4bce6]                   ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
02154582: call   0x182f609b0
02154587: lea    rcx, [rip + 0x1b74e82]                   ; [0x3cc9410] str:'coda'
0215458e: call   0x182f609b0
02154593: lea    rcx, [rip + 0x1bac1d6]                   ; [0x3d00770] str:'[coda]'
0215459a: call   0x182f609b0
0215459f: lea    rcx, [rip + 0x1bb33a2]                   ; [0x3d07948] str:'_ENDOFSONG'
021545a6: call   0x182f609b0
021545ab: lea    rcx, [rip + 0x1bac9d6]                   ; [0x3d00f88] str:'[section'
021545b2: call   0x182f609b0
021545b7: lea    rcx, [rip + 0x1bac852]                   ; [0x3d00e10] str:'[prc'
021545be: call   0x182f609b0
021545c3: lea    rcx, [rip + 0x1bac31e]                   ; [0x3d008e8] str:'[end]'
021545ca: call   0x182f609b0
021545cf: mov    rcx, qword ptr [rbp + 0x60]
021545d3: mov    byte ptr [rip + 0x1dc50be], 1            ; [0x3f19698] (bss)
021545da: mov    rsi, qword ptr [rsi]
021545dd: xorps  xmm0, xmm0
021545e0: mov    qword ptr [rsp + 0x170], rbx
021545e8: mov    qword ptr [rsp + 0x140], rdi
021545f0: mov    qword ptr [rsp + 0x138], r12
021545f8: mov    qword ptr [rsp + 0x130], r13
02154600: mov    qword ptr [rsp + 0x128], r14
02154608: mov    qword ptr [rsp + 0x120], r15
02154610: mov    qword ptr [rbp + 0x68], 0
02154618: mov    qword ptr [rbp + 0x78], 0
02154620: movups xmmword ptr [rsp + 0x30], xmm0
02154625: test   rsi, rsi
02154628: je     0x182154c1f
0215462e: test   rcx, rcx
02154631: je     0x182154c1f
02154637: mov    rsi, qword ptr [rsi + 0x30]
0215463b: xor    r12d, r12d
0215463e: xor    eax, eax
02154640: cmp    eax, dword ptr [rcx + 0x18]
02154643: jge    0x182154be5
02154649: mov    r8, qword ptr [rip + 0x1b7b8f0]          ; [0x3ccff40] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Item()
02154650: mov    edx, r12d
02154653: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
02154658: test   rax, rax
0215465b: je     0x182154bd6
02154661: mov    rcx, qword ptr [rip + 0x1b4bc00]         ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
02154668: xor    r14d, r14d
0215466b: cmp    qword ptr [rax], rcx
0215466e: cmove  r14, rax
02154672: test   r14, r14
02154675: je     0x182154bd6
0215467b: cmp    qword ptr [r14 + 0x20], 0
02154680: je     0x182154bd6
02154686: cmp    byte ptr [rip + 0x1db793c], 0            ; [0x3f0bfc9] (bss)
0215468d: mov    rbx, qword ptr [r14 + 0x20]
02154691: jne    0x1821546a6
02154693: lea    rcx, [rip + 0x1b4cf0e]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215469a: call   0x182f609b0
0215469f: mov    byte ptr [rip + 0x1db7923], 1            ; [0x3f0bfc9] (bss)
021546a6: xorps  xmm0, xmm0
021546a9: movups xmmword ptr [rsp + 0x20], xmm0
021546ae: xor    edx, edx
021546b0: mov    dword ptr [rsp + 0x4c], 0
021546b8: mov    rcx, rbx
021546bb: call   0x1819829d0                              ; System.String$$GetRawStringData
021546c0: mov    qword ptr [rsp + 0x40], rax
021546c5: lea    rdx, [rsp + 0x20]
021546ca: mov    eax, dword ptr [rbx + 0x10]
021546cd: lea    rcx, [rbp + 0x10]
021546d1: mov    dword ptr [rsp + 0x48], eax
021546d5: xor    r8d, r8d
021546d8: movaps xmm0, xmmword ptr [rsp + 0x40]
021546dd: movdqa xmmword ptr [rsp + 0x20], xmm0
021546e3: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
021546e8: cmp    byte ptr [rip + 0x1db7a6c], 0            ; [0x3f0c15b] (bss)
021546ef: mov    r13, qword ptr [rip + 0x1bac892]         ; [0x3d00f88] str:'[section'
021546f6: mov    rbx, qword ptr [rax]
021546f9: mov    r15d, dword ptr [rax + 8]
021546fd: mov    edi, dword ptr [rax + 0xc]
02154700: jne    0x182154715
02154702: lea    rcx, [rip + 0x1b4ce9f]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02154709: call   0x182f609b0
0215470e: mov    byte ptr [rip + 0x1db7a46], 1            ; [0x3f0c15b] (bss)
02154715: xorps  xmm0, xmm0
02154718: test   r13, r13
0215471b: je     0x182154741
0215471d: xor    edx, edx
0215471f: mov    dword ptr [rsp + 0x5c], 0
02154727: mov    rcx, r13
0215472a: call   0x1819829d0                              ; System.String$$GetRawStringData
0215472f: mov    qword ptr [rsp + 0x50], rax
02154734: mov    eax, dword ptr [r13 + 0x10]
02154738: mov    dword ptr [rsp + 0x58], eax
0215473c: movaps xmm0, xmmword ptr [rsp + 0x50]
02154741: mov    r8, qword ptr [rip + 0x1bb55a8]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
02154748: lea    rdx, [rsp + 0x20]
0215474d: lea    rcx, [rsp + 0x60]
02154752: movdqa xmmword ptr [rsp + 0x20], xmm0
02154758: mov    qword ptr [rsp + 0x60], rbx
0215475d: mov    dword ptr [rsp + 0x68], r15d
02154762: mov    dword ptr [rsp + 0x6c], edi
02154766: call   0x182c61990
0215476b: test   al, al
0215476d: je     0x182154885
02154773: mov    rcx, qword ptr [r14 + 0x20]
02154777: test   rcx, rcx
0215477a: je     0x182154c1f
02154780: mov    rdx, qword ptr [rip + 0x1bb31c1]         ; [0x3d07948] str:'_ENDOFSONG'
02154787: xor    r8d, r8d
0215478a: call   0x18197f670                              ; System.String$$Contains
0215478f: test   al, al
02154791: jne    0x182154885
02154797: mov    r13, qword ptr [rip + 0x1b4d242]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215479e: lea    edi, [r15 - 0xa]
021547a2: cmp    r15d, 9
021547a6: jb     0x1821547b0
021547a8: lea    eax, [r15 - 9]
021547ac: cmp    edi, eax
021547ae: jbe    0x1821547b7
021547b0: xor    ecx, ecx
021547b2: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
021547b7: mov    rcx, qword ptr [r13 + 0x20]
021547bb: add    rbx, 0x12
021547bf: mov    dword ptr [rsp + 0x7c], 0
021547c7: test   byte ptr [rcx + 0x135], 1
021547ce: jne    0x1821547d5
021547d0: call   0x182f65750
021547d5: mov    rdx, qword ptr [rip + 0x1b4d2b4]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
021547dc: lea    rcx, [rsp + 0x30]
021547e1: mov    qword ptr [rsp + 0x70], rbx
021547e6: mov    dword ptr [rsp + 0x78], edi
021547ea: movaps xmm0, xmmword ptr [rsp + 0x70]
021547ef: movdqa xmmword ptr [rsp + 0x30], xmm0
021547f5: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
021547fa: mov    rcx, qword ptr [rip + 0x1b4005f]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02154801: mov    rbx, rax
02154804: cmp    dword ptr [rcx + 0xe0], 0
0215480b: jne    0x182154819
0215480d: call   0x182f60cf0
02154812: mov    rcx, qword ptr [rip + 0x1b40047]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02154819: mov    rcx, qword ptr [rcx + 0xb8]
02154820: mov    rcx, qword ptr [rcx]
02154823: test   rcx, rcx
02154826: je     0x182154c1f
0215482c: mov    r9, qword ptr [rip + 0x1b3e2c5]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
02154833: lea    r8, [rbp + 0x68]
02154837: mov    rdx, rbx
0215483a: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
0215483f: test   al, al
02154841: jne    0x182154873
02154843: test   rbx, rbx
02154846: je     0x182154c1f
0215484c: mov    edx, 0x5f
02154851: mov    r8d, 0x20
02154857: xor    r9d, r9d
0215485a: mov    rcx, rbx
0215485d: call   0x181986c10                              ; System.String$$Replace
02154862: test   rsi, rsi
02154865: je     0x182154c1f
0215486b: mov    rdx, rax
0215486e: jmp    0x182154bc7
02154873: test   rsi, rsi
02154876: je     0x182154c1f
0215487c: mov    rdx, qword ptr [rbp + 0x68]
02154880: jmp    0x182154bc7
02154885: cmp    byte ptr [rip + 0x1db78cf], 0            ; [0x3f0c15b] (bss)
0215488c: mov    r13, qword ptr [rip + 0x1bac57d]         ; [0x3d00e10] str:'[prc'
02154893: jne    0x1821548a8
02154895: lea    rcx, [rip + 0x1b4cd0c]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215489c: call   0x182f609b0
021548a1: mov    byte ptr [rip + 0x1db78b3], 1            ; [0x3f0c15b] (bss)
021548a8: xorps  xmm0, xmm0
021548ab: test   r13, r13
021548ae: je     0x1821548d0
021548b0: xor    edx, edx
021548b2: mov    dword ptr [rbp - 0x74], 0
021548b9: mov    rcx, r13
021548bc: call   0x1819829d0                              ; System.String$$GetRawStringData
021548c1: mov    qword ptr [rbp - 0x80], rax
021548c5: mov    eax, dword ptr [r13 + 0x10]
021548c9: mov    dword ptr [rbp - 0x78], eax
021548cc: movaps xmm0, xmmword ptr [rbp - 0x80]
021548d0: mov    r8, qword ptr [rip + 0x1bb5419]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
021548d7: lea    rdx, [rsp + 0x20]
021548dc: lea    rcx, [rbp - 0x70]
021548e0: movdqa xmmword ptr [rsp + 0x20], xmm0
021548e6: mov    qword ptr [rbp - 0x70], rbx
021548ea: mov    dword ptr [rbp - 0x68], r15d
021548ee: mov    dword ptr [rbp - 0x64], edi
021548f1: call   0x182c61990
021548f6: test   al, al
021548f8: jne    0x182154aed
021548fe: cmp    byte ptr [rip + 0x1db7857], al           ; [0x3f0c15b] (bss)
02154904: mov    r13, qword ptr [rip + 0x1babfdd]         ; [0x3d008e8] str:'[end]'
0215490b: jne    0x182154920
0215490d: lea    rcx, [rip + 0x1b4cc94]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02154914: call   0x182f609b0
02154919: mov    byte ptr [rip + 0x1db783b], 1            ; [0x3f0c15b] (bss)
02154920: xorps  xmm0, xmm0
02154923: test   r13, r13
02154926: je     0x182154948
02154928: xor    edx, edx
0215492a: mov    dword ptr [rbp - 0x54], 0
02154931: mov    rcx, r13
02154934: call   0x1819829d0                              ; System.String$$GetRawStringData
02154939: mov    qword ptr [rbp - 0x60], rax
0215493d: mov    eax, dword ptr [r13 + 0x10]
02154941: mov    dword ptr [rbp - 0x58], eax
02154944: movaps xmm0, xmmword ptr [rbp - 0x60]
02154948: mov    r8, qword ptr [rip + 0x1bb5229]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215494f: lea    rdx, [rsp + 0x20]
02154954: lea    rcx, [rbp - 0x50]
02154958: movdqa xmmword ptr [rsp + 0x20], xmm0
0215495e: mov    qword ptr [rbp - 0x50], rbx
02154962: mov    dword ptr [rbp - 0x48], r15d
02154966: mov    dword ptr [rbp - 0x44], edi
02154969: call   0x182c61850
0215496e: test   al, al
02154970: jne    0x182154ac6
02154976: cmp    byte ptr [rip + 0x1db77df], al           ; [0x3f0c15b] (bss)
0215497c: mov    r13, qword ptr [rip + 0x1babded]         ; [0x3d00770] str:'[coda]'
02154983: jne    0x182154998
02154985: lea    rcx, [rip + 0x1b4cc1c]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215498c: call   0x182f609b0
02154991: mov    byte ptr [rip + 0x1db77c3], 1            ; [0x3f0c15b] (bss)
02154998: xorps  xmm0, xmm0
0215499b: test   r13, r13
0215499e: je     0x1821549c0
021549a0: xor    edx, edx
021549a2: mov    dword ptr [rbp - 0x34], 0
021549a9: mov    rcx, r13
021549ac: call   0x1819829d0                              ; System.String$$GetRawStringData
021549b1: mov    qword ptr [rbp - 0x40], rax
021549b5: mov    eax, dword ptr [r13 + 0x10]
021549b9: mov    dword ptr [rbp - 0x38], eax
021549bc: movaps xmm0, xmmword ptr [rbp - 0x40]
021549c0: mov    r8, qword ptr [rip + 0x1bb51b1]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
021549c7: lea    rdx, [rsp + 0x20]
021549cc: lea    rcx, [rbp - 0x30]
021549d0: movdqa xmmword ptr [rsp + 0x20], xmm0
021549d6: mov    qword ptr [rbp - 0x30], rbx
021549da: mov    dword ptr [rbp - 0x28], r15d
021549de: mov    dword ptr [rbp - 0x24], edi
021549e1: call   0x182c61850
021549e6: test   al, al
021549e8: jne    0x182154a62
021549ea: cmp    byte ptr [rip + 0x1db776b], al           ; [0x3f0c15b] (bss)
021549f0: mov    r13, qword ptr [rip + 0x1b74a19]         ; [0x3cc9410] str:'coda'
021549f7: jne    0x182154a0c
021549f9: lea    rcx, [rip + 0x1b4cba8]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02154a00: call   0x182f609b0
02154a05: mov    byte ptr [rip + 0x1db774f], 1            ; [0x3f0c15b] (bss)
02154a0c: xorps  xmm0, xmm0
02154a0f: test   r13, r13
02154a12: je     0x182154a34
02154a14: xor    edx, edx
02154a16: mov    dword ptr [rbp - 0x14], 0
02154a1d: mov    rcx, r13
02154a20: call   0x1819829d0                              ; System.String$$GetRawStringData
02154a25: mov    qword ptr [rbp - 0x20], rax
02154a29: mov    eax, dword ptr [r13 + 0x10]
02154a2d: mov    dword ptr [rbp - 0x18], eax
02154a30: movaps xmm0, xmmword ptr [rbp - 0x20]
02154a34: mov    r8, qword ptr [rip + 0x1bb513d]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02154a3b: lea    rdx, [rsp + 0x20]
02154a40: lea    rcx, [rbp - 0x10]
02154a44: movdqa xmmword ptr [rsp + 0x20], xmm0
02154a4a: mov    qword ptr [rbp - 0x10], rbx
02154a4e: mov    dword ptr [rbp - 8], r15d
02154a52: mov    dword ptr [rbp - 4], edi
02154a55: call   0x182c61850
02154a5a: test   al, al
02154a5c: je     0x182154bd6
02154a62: test   rsi, rsi
02154a65: je     0x182154c1f
02154a6b: cmp    qword ptr [rsi + 0x100], 0
02154a73: jne    0x182154bd6
02154a79: mov    rcx, qword ptr [rip + 0x1b405f8]         ; [0x3c95078] meta:ʶʿʺʴʻʽʶʽʺʼʺ_TypeInfo
02154a80: mov    rbx, qword ptr [r14 + 0x10]
02154a84: call   0x182f60c00
02154a89: mov    rdx, qword ptr [rip + 0x1b74980]         ; [0x3cc9410] str:'coda'
02154a90: xor    r9d, r9d
02154a93: mov    r8, rbx
02154a96: mov    rcx, rax
02154a99: mov    rdi, rax
02154a9c: call   0x18211ba30                              ; ʶʿʺʴʻʽʶʽʺʼʺ$$.ctor
02154aa1: lea    rcx, [rsi + 0x100]
02154aa8: mov    qword ptr [rsi + 0x100], rdi
02154aaf: mov    rdx, rdi
02154ab2: call   0x182f5fc00
02154ab7: mov    rcx, qword ptr [rbp + 0x60]
02154abb: inc    r12d
02154abe: mov    eax, r12d
02154ac1: jmp    0x182154640
02154ac6: test   rsi, rsi
02154ac9: je     0x182154c1f
02154acf: mov    rdx, qword ptr [r14 + 0x10]
02154ad3: xor    r8d, r8d
02154ad6: mov    rcx, rsi
02154ad9: call   0x18212e310                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ˁʶʲʾʹʻʿʾˁʾʾ
02154ade: mov    rcx, qword ptr [rbp + 0x60]
02154ae2: inc    r12d
02154ae5: mov    eax, r12d
02154ae8: jmp    0x182154640
02154aed: mov    r13, qword ptr [rip + 0x1b4ceec]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02154af4: lea    edi, [r15 - 6]
02154af8: cmp    r15d, 5
02154afc: jb     0x182154b06
02154afe: lea    eax, [r15 - 5]
02154b02: cmp    edi, eax
02154b04: jbe    0x182154b0d
02154b06: xor    ecx, ecx
02154b08: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02154b0d: mov    rcx, qword ptr [r13 + 0x20]
02154b11: add    rbx, 0xa
02154b15: mov    dword ptr [rbp + 0xc], 0
02154b1c: test   byte ptr [rcx + 0x135], 1
02154b23: jne    0x182154b2a
02154b25: call   0x182f65750
02154b2a: mov    rdx, qword ptr [rip + 0x1b4cf5f]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
02154b31: lea    rcx, [rsp + 0x30]
02154b36: mov    qword ptr [rbp], rbx
02154b3a: mov    dword ptr [rbp + 8], edi
02154b3d: movaps xmm0, xmmword ptr [rbp]
02154b41: movdqa xmmword ptr [rsp + 0x30], xmm0
02154b47: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
02154b4c: mov    rcx, qword ptr [rip + 0x1b3fd0d]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02154b53: mov    rbx, rax
02154b56: cmp    dword ptr [rcx + 0xe0], 0
02154b5d: jne    0x182154b6b
02154b5f: call   0x182f60cf0
02154b64: mov    rcx, qword ptr [rip + 0x1b3fcf5]         ; [0x3c94860] meta:ʶʷʶʴʴˁʾʺʲʺʳ_TypeInfo
02154b6b: mov    rcx, qword ptr [rcx + 0xb8]
02154b72: mov    rcx, qword ptr [rcx]
02154b75: test   rcx, rcx
02154b78: je     0x182154c1f
02154b7e: mov    r9, qword ptr [rip + 0x1b3df73]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
02154b85: lea    r8, [rbp + 0x78]
02154b89: mov    rdx, rbx
02154b8c: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
02154b91: test   al, al
02154b93: jne    0x182154bbe
02154b95: test   rbx, rbx
02154b98: je     0x182154c1f
02154b9e: mov    edx, 0x5f
02154ba3: mov    r8d, 0x20
02154ba9: xor    r9d, r9d
02154bac: mov    rcx, rbx
02154baf: call   0x181986c10                              ; System.String$$Replace
02154bb4: test   rsi, rsi
02154bb7: je     0x182154c1f
02154bb9: mov    rdx, rax
02154bbc: jmp    0x182154bc7
02154bbe: test   rsi, rsi
02154bc1: je     0x182154c1f
02154bc3: mov    rdx, qword ptr [rbp + 0x78]
02154bc7: mov    r8, qword ptr [r14 + 0x10]
02154bcb: xor    r9d, r9d
02154bce: mov    rcx, rsi
02154bd1: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
02154bd6: mov    rcx, qword ptr [rbp + 0x60]
02154bda: inc    r12d
02154bdd: mov    eax, r12d
02154be0: jmp    0x182154640
02154be5: mov    r15, qword ptr [rsp + 0x120]
02154bed: mov    r14, qword ptr [rsp + 0x128]
02154bf5: mov    r13, qword ptr [rsp + 0x130]
02154bfd: mov    r12, qword ptr [rsp + 0x138]
02154c05: mov    rdi, qword ptr [rsp + 0x140]
02154c0d: mov    rbx, qword ptr [rsp + 0x170]
02154c15: add    rsp, 0x148
02154c1c: pop    rsi
02154c1d: pop    rbp
02154c1e: ret    
02154c1f: call   0x182f60c50
02154c24: int3   
02154c25: int3   
