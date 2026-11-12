021695e0: mov    qword ptr [rsp + 8], rcx
021695e5: push   rbp
021695e6: push   rsi
021695e7: push   rdi
021695e8: push   r12
021695ea: push   r13
021695ec: push   r14
021695ee: push   r15
021695f0: sub    rsp, 0x160
021695f7: lea    rbp, [rsp + 0x20]
021695fc: mov    qword ptr [rbp + 0x190], rbx
02169603: movaps xmmword ptr [rbp + 0x130], xmm6
0216960a: mov    rbx, rdx
0216960d: cmp    byte ptr [rip + 0x1db010b], 0            ; [0x3f1971f] (bss)
02169614: jne    0x1821696ed
0216961a: lea    rcx, [rip + 0x1b9e47f]                   ; [0x3d07aa0] metamethod:Method$System.Collections.Generic.Dictionary<string, Dictionary<string, string>>.TryGetValue()
02169621: call   0x182f609b0
02169626: lea    rcx, [rip + 0x1b28d03]                   ; [0x3c92330] metamethod:Method$System.Collections.Generic.Dictionary<string, string>..ctor()
0216962d: call   0x182f609b0
02169632: lea    rcx, [rip + 0x1b9e697]                   ; [0x3d07cd0] metamethod:Method$System.Collections.Generic.Dictionary<string, Dictionary<string, string>>.set_Item()
02169639: call   0x182f609b0
0216963e: lea    rcx, [rip + 0x1b2978b]                   ; [0x3c92dd0] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.set_Item()
02169645: call   0x182f609b0
0216964a: lea    rcx, [rip + 0x1b35877]                   ; [0x3c9eec8] meta:System.Collections.Generic.Dictionary<string, string>_TypeInfo
02169651: call   0x182f609b0
02169656: lea    rcx, [rip + 0x1b9fd0b]                   ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
0216965d: call   0x182f609b0
02169662: lea    rcx, [rip + 0x1b382bf]                   ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169669: call   0x182f609b0
0216966e: lea    rcx, [rip + 0x1b3836b]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169675: call   0x182f609b0
0216967a: lea    rcx, [rip + 0x1b3840f]                   ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
02169681: call   0x182f609b0
02169686: lea    rcx, [rip + 0x1b3861b]                   ; [0x3ca1ca8] metamethod:Method$System.ReadOnlySpan<char>.get_IsEmpty()
0216968d: call   0x182f609b0
02169692: lea    rcx, [rip + 0x1b386c7]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02169699: call   0x182f609b0
0216969e: lea    rcx, [rip + 0x1b40533]                   ; [0x3ca9bd8] metamethod:Method$System.Span<char>.Slice()
021696a5: call   0x182f609b0
021696aa: lea    rcx, [rip + 0x1b405df]                   ; [0x3ca9c90] metamethod:Method$System.Span<char>.ToString()
021696b1: call   0x182f609b0
021696b6: lea    rcx, [rip + 0x1b40013]                   ; [0x3ca96d0] metamethod:Method$System.Span<char>..ctor()
021696bd: call   0x182f609b0
021696c2: lea    rcx, [rip + 0x1b408a7]                   ; [0x3ca9f70] metamethod:Method$System.Span<char>.op_Implicit()
021696c9: call   0x182f609b0
021696ce: lea    rcx, [rip + 0x1b6da03]                   ; [0x3cd70d8] meta:string_TypeInfo
021696d5: call   0x182f609b0
021696da: lea    rcx, [rip + 0x1b35faf]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
021696e1: call   0x182f609b0
021696e6: mov    byte ptr [rip + 0x1db0032], 1            ; [0x3f1971f] (bss)
021696ed: xorps  xmm0, xmm0
021696f0: movups xmmword ptr [rbp + 0x10], xmm0
021696f4: xorps  xmm1, xmm1
021696f7: movups xmmword ptr [rbp + 0x20], xmm1
021696fb: movups xmmword ptr [rbp + 0x30], xmm0
021696ff: movups xmmword ptr [rbp + 0xa0], xmm1
02169706: movups xmmword ptr [rbp + 0xb0], xmm0
0216970d: movups xmmword ptr [rbp + 0xc0], xmm1
02169714: xor    r15d, r15d
02169717: mov    qword ptr [rbp + 0x188], r15
0216971e: mov    dword ptr [rbp + 0xd8], r15d
02169725: cmp    dword ptr [rbx + 8], r15d
02169729: jle    0x182169788
0216972b: jbe    0x182169de0
02169731: mov    rax, qword ptr [rbx]
02169734: mov    ecx, 0xfeff
02169739: cmp    word ptr [rax], cx
0216973c: jne    0x182169788
0216973e: mov    rsi, rax
02169741: mov    eax, dword ptr [rbx + 8]
02169744: mov    r14, qword ptr [rip + 0x1b38295]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0216974b: lea    edi, [rax - 1]
0216974e: cmp    eax, 1
02169751: jb     0x182169759
02169753: dec    eax
02169755: cmp    edi, eax
02169757: jbe    0x182169760
02169759: xor    ecx, ecx
0216975b: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169760: mov    dword ptr [rbp + 0x4c], r15d
02169764: mov    rcx, qword ptr [r14 + 0x20]
02169768: test   byte ptr [rcx + 0x135], 1
0216976f: jne    0x182169776
02169771: call   0x182f65750
02169776: lea    rax, [rsi + 2]
0216977a: mov    qword ptr [rbp + 0x40], rax
0216977e: mov    dword ptr [rbp + 0x48], edi
02169781: movups xmm0, xmmword ptr [rbp + 0x40]
02169785: movups xmmword ptr [rbx], xmm0
02169788: mov    eax, dword ptr [rsp]
0216978b: mov    eax, 0x800
02169790: sub    rsp, rax
02169793: lea    r14, [rsp + 0x20]
02169798: mov    eax, dword ptr [r14]
0216979b: xor    edx, edx
0216979d: mov    r8d, 0x800
021697a3: mov    rcx, r14
021697a6: call   0x18305d9d0
021697ab: nop    
021697ac: mov    rax, qword ptr [rip + 0x1b6d925]         ; [0x3cd70d8] meta:string_TypeInfo
021697b3: mov    rcx, qword ptr [rax + 0xb8]
021697ba: mov    rax, qword ptr [rcx]
021697bd: mov    qword ptr [rbp + 0x198], rax
021697c4: movups xmm0, xmmword ptr [rbx]
021697c7: movaps xmmword ptr [rbp + 0x10], xmm0
021697cb: nop    dword ptr [rax + rax]
021697d0: mov    rdx, qword ptr [rip + 0x1b384d1]         ; [0x3ca1ca8] metamethod:Method$System.ReadOnlySpan<char>.get_IsEmpty()
021697d7: lea    rcx, [rbp + 0x10]
021697db: call   0x18094e290                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$get_IsEmpty
021697e0: test   al, al
021697e2: jne    0x182169dbf
021697e8: mov    r15, qword ptr [rip + 0x1b9fb79]         ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
021697ef: mov    rdi, qword ptr [rbp + 0x10]
021697f3: mov    r12d, dword ptr [rbp + 0x18]
021697f7: mov    esi, dword ptr [rbp + 0x1c]
021697fa: cmp    qword ptr [r15 + 0x38], 0
021697ff: jne    0x182169834
02169801: lea    rcx, [rip + 0x1b5cbd0]                   ; [0x3cc63d8] meta:byte_var
02169808: call   0x182f609b0
0216980d: lea    rcx, [rip + 0x1b5dc44]                   ; [0x3cc7458] meta:char_var
02169814: call   0x182f609b0
02169819: lea    rcx, [rip + 0x1b7c958]                   ; [0x3ce6178] meta:System.Type_TypeInfo
02169820: call   0x182f609b0
02169825: cmp    qword ptr [r15 + 0x38], 0
0216982a: jne    0x182169834
0216982c: mov    rcx, r15
0216982f: call   0x182f657d0
02169834: mov    rbx, qword ptr [r15 + 0x38]
02169838: mov    rbx, qword ptr [rbx]
0216983b: mov    rcx, qword ptr [rip + 0x1b7c936]         ; [0x3ce6178] meta:System.Type_TypeInfo
02169842: cmp    dword ptr [rcx + 0xe0], 0
02169849: jne    0x182169850
0216984b: call   0x182f60cf0
02169850: xor    edx, edx
02169852: mov    rcx, rbx
02169855: call   0x181b7a070                              ; System.Type$$GetTypeFromHandle
0216985a: mov    rbx, rax
0216985d: xor    edx, edx
0216985f: mov    rcx, qword ptr [rip + 0x1b5cb72]         ; [0x3cc63d8] meta:byte_var
02169866: call   0x181b7a070                              ; System.Type$$GetTypeFromHandle
0216986b: xor    r8d, r8d
0216986e: mov    rdx, rax
02169871: mov    rcx, rbx
02169874: call   0x180574970                              ; Unity.Burst.Unsafe$$AreSame<__Il2CppFullySharedGenericType>
02169879: mov    rdx, qword ptr [r15 + 0x38]
0216987d: test   al, al
0216987f: jne    0x18216992d
02169885: mov    rbx, qword ptr [rdx]
02169888: mov    rcx, qword ptr [rip + 0x1b7c8e9]         ; [0x3ce6178] meta:System.Type_TypeInfo
0216988f: cmp    dword ptr [rcx + 0xe0], 0
02169896: jne    0x18216989d
02169898: call   0x182f60cf0
0216989d: xor    edx, edx
0216989f: mov    rcx, rbx
021698a2: call   0x181b7a070                              ; System.Type$$GetTypeFromHandle
021698a7: mov    rbx, rax
021698aa: xor    edx, edx
021698ac: mov    rcx, qword ptr [rip + 0x1b5dba5]         ; [0x3cc7458] meta:char_var
021698b3: call   0x181b7a070                              ; System.Type$$GetTypeFromHandle
021698b8: xor    r8d, r8d
021698bb: mov    rdx, rax
021698be: mov    rcx, rbx
021698c1: call   0x180574970                              ; Unity.Burst.Unsafe$$AreSame<__Il2CppFullySharedGenericType>
021698c6: mov    rdx, qword ptr [r15 + 0x38]
021698ca: mov    rdx, qword ptr [rdx + 0x10]
021698ce: test   al, al
021698d0: jne    0x182169902
021698d2: mov    qword ptr [rbp + 0x40], rdi
021698d6: mov    dword ptr [rbp + 0x48], r12d
021698da: mov    dword ptr [rbp + 0x4c], esi
021698dd: lea    rcx, [rbp + 0x40]
021698e1: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
021698e6: mov    r9, qword ptr [r15 + 0x38]
021698ea: mov    edx, 0xa
021698ef: mov    r9, qword ptr [r9 + 0x38]
021698f3: mov    r8d, r12d
021698f6: mov    rcx, rax
021698f9: call   0x1804fcec0                              ; System.SpanHelpers$$IndexOf<char>
021698fe: mov    ebx, eax
02169900: jmp    0x182169957
02169902: mov    qword ptr [rbp + 0x60], rdi
02169906: mov    dword ptr [rbp + 0x68], r12d
0216990a: mov    dword ptr [rbp + 0x6c], esi
0216990d: lea    rcx, [rbp + 0x60]
02169911: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
02169916: mov    edx, 0xa
0216991b: xor    r9d, r9d
0216991e: mov    r8d, r12d
02169921: mov    rcx, rax
02169924: call   0x181b71620                              ; System.SpanHelpers$$IndexOf
02169929: mov    ebx, eax
0216992b: jmp    0x182169957
0216992d: mov    qword ptr [rbp + 0x70], rdi
02169931: mov    dword ptr [rbp + 0x78], r12d
02169935: mov    dword ptr [rbp + 0x7c], esi
02169938: mov    rdx, qword ptr [rdx + 0x10]
0216993c: lea    rcx, [rbp + 0x70]
02169940: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
02169945: xor    r9d, r9d
02169948: mov    r8d, r12d
0216994b: mov    dl, 0xa
0216994d: mov    rcx, rax
02169950: call   0x181b71ae0                              ; System.SpanHelpers$$IndexOf
02169955: mov    ebx, eax
02169957: mov    rax, qword ptr [rbp + 0x18]
0216995b: cmp    ebx, -1
0216995e: cmove  ebx, eax
02169961: mov    rdi, qword ptr [rip + 0x1b38078]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169968: cmp    ebx, eax
0216996a: jbe    0x182169973
0216996c: xor    ecx, ecx
0216996e: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169973: mov    rsi, qword ptr [rbp + 0x10]
02169977: xorps  xmm0, xmm0
0216997a: movups xmmword ptr [rbp], xmm0
0216997e: mov    rcx, qword ptr [rdi + 0x20]
02169982: test   byte ptr [rcx + 0x135], 1
02169989: jne    0x182169990
0216998b: call   0x182f65750
02169990: mov    qword ptr [rbp], rsi
02169994: mov    dword ptr [rbp + 8], ebx
02169997: movaps xmm0, xmmword ptr [rbp]
0216999b: movdqa xmmword ptr [rbp], xmm0
021699a0: xor    r8d, r8d
021699a3: lea    rdx, [rbp]
021699a7: lea    rcx, [rbp + 0x100]
021699ae: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
021699b3: movups xmm0, xmmword ptr [rax]
021699b6: movaps xmmword ptr [rbp + 0x20], xmm0
021699ba: mov    rdi, qword ptr [rbp + 0x18]
021699be: cmp    ebx, edi
021699c0: je     0x182169a0d
021699c2: mov    rsi, qword ptr [rip + 0x1b37f5f]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
021699c9: inc    ebx
021699cb: cmp    ebx, edi
021699cd: jbe    0x1821699da
021699cf: xor    ecx, ecx
021699d1: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
021699d6: mov    rdi, qword ptr [rbp + 0x18]
021699da: mov    r15, qword ptr [rbp + 0x10]
021699de: xorps  xmm0, xmm0
021699e1: movups xmmword ptr [rbp], xmm0
021699e5: mov    rcx, qword ptr [rsi + 0x20]
021699e9: test   byte ptr [rcx + 0x135], 1
021699f0: jne    0x1821699f7
021699f2: call   0x182f65750
021699f7: movsxd rax, ebx
021699fa: lea    rcx, [r15 + rax*2]
021699fe: mov    qword ptr [rbp], rcx
02169a02: sub    edi, ebx
02169a04: mov    dword ptr [rbp + 8], edi
02169a07: movaps xmm0, xmmword ptr [rbp]
02169a0b: jmp    0x182169a33
02169a0d: xorps  xmm0, xmm0
02169a10: movdqa xmmword ptr [rbp], xmm0
02169a15: mov    r8, qword ptr [rip + 0x1b40554]          ; [0x3ca9f70] metamethod:Method$System.Span<char>.op_Implicit()
02169a1c: lea    rdx, [rbp]
02169a20: lea    rcx, [rbp + 0xf0]
02169a27: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
02169a2c: movaps xmm0, xmmword ptr [rbp + 0xf0]
02169a33: movdqa xmmword ptr [rbp + 0x10], xmm0
02169a38: mov    rdx, qword ptr [rip + 0x1b38269]         ; [0x3ca1ca8] metamethod:Method$System.ReadOnlySpan<char>.get_IsEmpty()
02169a3f: lea    rcx, [rbp + 0x20]
02169a43: call   0x18094e290                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$get_IsEmpty
02169a48: test   al, al
02169a4a: jne    0x1821697d0
02169a50: mov    rcx, qword ptr [rbp + 0x28]
02169a54: test   ecx, ecx
02169a56: je     0x182169de6
02169a5c: mov    rbx, qword ptr [rbp + 0x20]
02169a60: movzx  eax, word ptr [rbx]
02169a63: cmp    ax, 0x23
02169a67: je     0x1821697d0
02169a6d: cmp    ax, 0x3b
02169a71: je     0x1821697d0
02169a77: cmp    ax, 0x5b
02169a7b: jne    0x182169b7d
02169a81: lea    eax, [rcx - 1]
02169a84: cmp    eax, ecx
02169a86: jae    0x182169dec
02169a8c: movsxd rax, ecx
02169a8f: cmp    word ptr [rbx + rax*2 - 2], 0x5d
02169a95: jne    0x182169b7d
02169a9b: mov    rsi, qword ptr [rip + 0x1b37f3e]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169aa2: lea    edi, [rcx - 2]
02169aa5: cmp    ecx, 1
02169aa8: jb     0x182169ab1
02169aaa: lea    eax, [rcx - 1]
02169aad: cmp    edi, eax
02169aaf: jbe    0x182169abc
02169ab1: xor    ecx, ecx
02169ab3: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169ab8: mov    rbx, qword ptr [rbp + 0x20]
02169abc: xorps  xmm0, xmm0
02169abf: movups xmmword ptr [rbp + 0x30], xmm0
02169ac3: mov    rcx, qword ptr [rsi + 0x20]
02169ac7: test   byte ptr [rcx + 0x135], 1
02169ace: jne    0x182169ad5
02169ad0: call   0x182f65750
02169ad5: add    rbx, 2
02169ad9: mov    rsi, qword ptr [rip + 0x1b400f8]         ; [0x3ca9bd8] metamethod:Method$System.Span<char>.Slice()
02169ae0: cmp    edi, 0x400
02169ae6: jbe    0x182169aef
02169ae8: xor    ecx, ecx
02169aea: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169aef: xorps  xmm0, xmm0
02169af2: movups xmmword ptr [rbp], xmm0
02169af6: mov    rcx, qword ptr [rsi + 0x20]
02169afa: test   byte ptr [rcx + 0x135], 1
02169b01: jne    0x182169b08
02169b03: call   0x182f65750
02169b08: mov    qword ptr [rbp], r14
02169b0c: mov    dword ptr [rbp + 8], edi
02169b0f: movaps xmm6, xmmword ptr [rbp]
02169b13: movdqa xmmword ptr [rbp + 0xa0], xmm6
02169b1b: mov    rcx, qword ptr [rip + 0x1b35b6e]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
02169b22: cmp    dword ptr [rcx + 0xe0], 0
02169b29: jne    0x182169b30
02169b2b: call   0x182f60cf0
02169b30: movdqa xmmword ptr [rbp], xmm6
02169b35: mov    qword ptr [rbp + 0x80], rbx
02169b3c: mov    dword ptr [rbp + 0x88], edi
02169b42: mov    eax, dword ptr [rbp + 0x3c]
02169b45: mov    dword ptr [rbp + 0x8c], eax
02169b4b: xor    r8d, r8d
02169b4e: lea    rdx, [rbp]
02169b52: lea    rcx, [rbp + 0x80]
02169b59: call   0x182152890
02169b5e: mov    rdx, qword ptr [rip + 0x1b4012b]         ; [0x3ca9c90] metamethod:Method$System.Span<char>.ToString()
02169b65: lea    rcx, [rbp + 0xa0]
02169b6c: call   0x1809b6a30                              ; System.Span<char>$$ToString
02169b71: mov    qword ptr [rbp + 0x198], rax
02169b78: jmp    0x1821697d0
02169b7d: movaps xmm0, xmmword ptr [rbp + 0x20]
02169b81: movdqa xmmword ptr [rbp + 0x30], xmm0
02169b86: mov    edx, 0x3d
02169b8b: mov    r8, qword ptr [rip + 0x1b9f7d6]          ; [0x3d09368] metamethod:Method$System.MemoryExtensions.IndexOf<char>()
02169b92: lea    rcx, [rbp + 0x30]
02169b96: call   0x182c616c0
02169b9b: mov    esi, eax
02169b9d: cmp    eax, -1
02169ba0: je     0x1821697d0
02169ba6: mov    rbx, qword ptr [rip + 0x1b37e33]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169bad: cmp    eax, dword ptr [rbp + 0x28]
02169bb0: jbe    0x182169bb9
02169bb2: xor    ecx, ecx
02169bb4: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169bb9: mov    rdi, qword ptr [rbp + 0x20]
02169bbd: xorps  xmm0, xmm0
02169bc0: movups xmmword ptr [rbp], xmm0
02169bc4: mov    rcx, qword ptr [rbx + 0x20]
02169bc8: test   byte ptr [rcx + 0x135], 1
02169bcf: jne    0x182169bd6
02169bd1: call   0x182f65750
02169bd6: mov    qword ptr [rbp], rdi
02169bda: mov    dword ptr [rbp + 8], esi
02169bdd: movaps xmm0, xmmword ptr [rbp]
02169be1: movdqa xmmword ptr [rbp + 0x30], xmm0
02169be6: xor    r8d, r8d
02169be9: lea    rdx, [rbp + 0x30]
02169bed: lea    rcx, [rbp + 0x110]
02169bf4: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
02169bf9: mov    rbx, qword ptr [rax]
02169bfc: mov    r15d, dword ptr [rax + 8]
02169c00: mov    edi, dword ptr [rax + 0xc]
02169c03: mov    r13, qword ptr [rip + 0x1b3ffce]         ; [0x3ca9bd8] metamethod:Method$System.Span<char>.Slice()
02169c0a: cmp    r15d, 0x400
02169c11: jbe    0x182169c1a
02169c13: xor    ecx, ecx
02169c15: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169c1a: xorps  xmm0, xmm0
02169c1d: movups xmmword ptr [rbp], xmm0
02169c21: mov    rcx, qword ptr [r13 + 0x20]
02169c25: test   byte ptr [rcx + 0x135], 1
02169c2c: jne    0x182169c33
02169c2e: call   0x182f65750
02169c33: mov    qword ptr [rbp], r14
02169c37: mov    dword ptr [rbp + 8], r15d
02169c3b: movaps xmm6, xmmword ptr [rbp]
02169c3f: movdqa xmmword ptr [rbp + 0xb0], xmm6
02169c47: mov    rcx, qword ptr [rip + 0x1b35a42]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
02169c4e: cmp    dword ptr [rcx + 0xe0], 0
02169c55: jne    0x182169c5c
02169c57: call   0x182f60cf0
02169c5c: movdqa xmmword ptr [rbp + 0x30], xmm6
02169c61: mov    qword ptr [rbp + 0x90], rbx
02169c68: mov    dword ptr [rbp + 0x98], r15d
02169c6f: mov    dword ptr [rbp + 0x9c], edi
02169c75: xor    r8d, r8d
02169c78: lea    rdx, [rbp + 0x30]
02169c7c: lea    rcx, [rbp + 0x90]
02169c83: call   0x182152890
02169c88: mov    rdi, qword ptr [rip + 0x1b37c99]         ; [0x3ca1928] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02169c8f: inc    esi
02169c91: mov    rbx, qword ptr [rbp + 0x28]
02169c95: cmp    esi, ebx
02169c97: jbe    0x182169ca4
02169c99: xor    ecx, ecx
02169c9b: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02169ca0: mov    rbx, qword ptr [rbp + 0x28]
02169ca4: mov    r15, qword ptr [rbp + 0x20]
02169ca8: xorps  xmm0, xmm0
02169cab: movups xmmword ptr [rbp], xmm0
02169caf: mov    rcx, qword ptr [rdi + 0x20]
02169cb3: test   byte ptr [rcx + 0x135], 1
02169cba: jne    0x182169cc1
02169cbc: call   0x182f65750
02169cc1: movsxd rax, esi
02169cc4: lea    rcx, [r15 + rax*2]
02169cc8: mov    qword ptr [rbp], rcx
02169ccc: sub    ebx, esi
02169cce: mov    dword ptr [rbp + 8], ebx
02169cd1: movaps xmm0, xmmword ptr [rbp]
02169cd5: movdqa xmmword ptr [rbp + 0x30], xmm0
02169cda: xor    r8d, r8d
02169cdd: lea    rdx, [rbp + 0x30]
02169ce1: lea    rcx, [rbp + 0x120]
02169ce8: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
02169ced: movups xmm0, xmmword ptr [rax]
02169cf0: movups xmmword ptr [rbp + 0xc0], xmm0
02169cf7: mov    r15, qword ptr [rbp + 0x180]
02169cfe: mov    rcx, qword ptr [r15 + 0x10]
02169d02: test   rcx, rcx
02169d05: je     0x182169dfb
02169d0b: mov    r9, qword ptr [rip + 0x1b9dd8e]          ; [0x3d07aa0] metamethod:Method$System.Collections.Generic.Dictionary<string, Dictionary<string, string>>.TryGetValue()
02169d12: lea    r8, [rbp + 0x188]
02169d19: mov    rsi, qword ptr [rbp + 0x198]
02169d20: mov    rdx, rsi
02169d23: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
02169d28: test   al, al
02169d2a: jne    0x182169d70
02169d2c: mov    rcx, qword ptr [rip + 0x1b35195]         ; [0x3c9eec8] meta:System.Collections.Generic.Dictionary<string, string>_TypeInfo
02169d33: call   0x182f60c00
02169d38: mov    rbx, rax
02169d3b: mov    rdx, qword ptr [rip + 0x1b285ee]         ; [0x3c92330] metamethod:Method$System.Collections.Generic.Dictionary<string, string>..ctor()
02169d42: mov    rcx, rax
02169d45: call   0x1813f1810                              ; System.Collections.Generic.Dictionary<object, StyleComplexSelector.PseudoStateData>$$.ctor
02169d4a: mov    qword ptr [rbp + 0x188], rbx
02169d51: mov    rcx, qword ptr [r15 + 0x10]
02169d55: test   rcx, rcx
02169d58: je     0x182169df1
02169d5e: mov    r9, qword ptr [rip + 0x1b9df6b]          ; [0x3d07cd0] metamethod:Method$System.Collections.Generic.Dictionary<string, Dictionary<string, string>>.set_Item()
02169d65: mov    r8, rbx
02169d68: mov    rdx, rsi
02169d6b: call   0x181403e00                              ; System.Collections.Generic.Dictionary<object, object>$$set_Item
02169d70: mov    rbx, qword ptr [rbp + 0x188]
02169d77: mov    rdx, qword ptr [rip + 0x1b3ff12]         ; [0x3ca9c90] metamethod:Method$System.Span<char>.ToString()
02169d7e: lea    rcx, [rbp + 0xb0]
02169d85: call   0x1809b6a30                              ; System.Span<char>$$ToString
02169d8a: mov    rdi, rax
02169d8d: mov    rdx, qword ptr [rip + 0x1b37cfc]         ; [0x3ca1a90] metamethod:Method$System.ReadOnlySpan<char>.ToString()
02169d94: lea    rcx, [rbp + 0xc0]
02169d9b: call   0x180952850                              ; System.ReadOnlySpan<char>$$ToString
02169da0: test   rbx, rbx
02169da3: je     0x182169df6
02169da5: mov    r9, qword ptr [rip + 0x1b29024]          ; [0x3c92dd0] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.set_Item()
02169dac: mov    r8, rax
02169daf: mov    rdx, rdi
02169db2: mov    rcx, rbx
02169db5: call   0x181403e00                              ; System.Collections.Generic.Dictionary<object, object>$$set_Item
02169dba: jmp    0x1821697d0
02169dbf: mov    rbx, qword ptr [rbp + 0x190]
02169dc6: movaps xmm6, xmmword ptr [rbp + 0x130]
02169dcd: lea    rsp, [rbp + 0x140]
02169dd4: pop    r15
02169dd6: pop    r14
02169dd8: pop    r13
02169dda: pop    r12
02169ddc: pop    rdi
02169ddd: pop    rsi
02169dde: pop    rbp
02169ddf: ret    
02169de0: call   0x182f60c40
02169de5: nop    
02169de6: call   0x182f60c40
02169deb: nop    
02169dec: call   0x182f60c40
02169df1: call   0x182f60c50
02169df6: call   0x182f60c50
02169dfb: call   0x182f60c50
02169e00: nop    
02169e01: lea    rcx, [rbp + 0xd0]
02169e08: call   0x182c41290
02169e0d: mov    rcx, rax
02169e10: call   0x182f60ce0
02169e15: int3   
02169e16: int3   
