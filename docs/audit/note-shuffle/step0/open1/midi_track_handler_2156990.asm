02156990: mov    rax, rsp
02156993: mov    qword ptr [rax + 8], rbx
02156997: mov    qword ptr [rax + 0x20], rsi
0215699b: mov    byte ptr [rax + 0x18], r8b
0215699f: push   rdi
021569a0: push   r12
021569a2: push   r13
021569a4: push   r14
021569a6: push   r15
021569a8: sub    rsp, 0x180
021569af: movaps xmmword ptr [rax - 0x38], xmm6
021569b3: movzx  r13d, r8b
021569b7: mov    rbx, rdx
021569ba: mov    r12, rcx
021569bd: cmp    byte ptr [rip + 0x1dc2cd5], 0            ; [0x3f19699] (bss)
021569c4: jne    0x182156ac1
021569ca: lea    rcx, [rip + 0x1b694df]                   ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021569d1: call   0x182f609b0
021569d6: lea    rcx, [rip + 0x1b68bd3]                   ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
021569dd: call   0x182f609b0
021569e2: lea    rcx, [rip + 0x1b68c87]                   ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
021569e9: call   0x182f609b0
021569ee: lea    rcx, [rip + 0x1b6957b]                   ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
021569f5: call   0x182f609b0
021569fa: lea    rcx, [rip + 0x1b6962f]                   ; [0x3cc0030] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Current()
02156a01: call   0x182f609b0
02156a06: lea    rcx, [rip + 0x1b68d23]                   ; [0x3cbf730] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Current()
02156a0d: call   0x182f609b0
02156a12: lea    rcx, [rip + 0x1b78da7]                   ; [0x3ccf7c0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.AddRange()
02156a19: call   0x182f609b0
02156a1e: lea    rcx, [rip + 0x1b7ae9b]                   ; [0x3cd18c0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.AddRange()
02156a25: call   0x182f609b0
02156a2a: lea    rcx, [rip + 0x1b76817]                   ; [0x3ccd248] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.AddRange()
02156a31: call   0x182f609b0
02156a36: lea    rcx, [rip + 0x1b777c3]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
02156a3d: call   0x182f609b0
02156a42: lea    rcx, [rip + 0x1b792b7]                   ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
02156a49: call   0x182f609b0
02156a4e: lea    rcx, [rip + 0x1b7792b]                   ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
02156a55: call   0x182f609b0
02156a5a: lea    rcx, [rip + 0x1b4bd8f]                   ; [0x3ca27f0] metamethod:Method$System.ReadOnlySpan<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Length()
02156a61: call   0x182f609b0
02156a66: lea    rcx, [rip + 0x1b3d8d3]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156a6d: call   0x182f609b0
02156a72: lea    rcx, [rip + 0x1b3e01f]                   ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02156a79: call   0x182f609b0
02156a7e: lea    rcx, [rip + 0x1b42a3b]                   ; [0x3c994c0] meta:ʺʾʺˀʾʶʲʷʷʺʳ_TypeInfo
02156a85: call   0x182f609b0
02156a8a: lea    rcx, [rip + 0x1b49507]                   ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02156a91: call   0x182f609b0
02156a96: lea    rcx, [rip + 0x1b497cb]                   ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
02156a9d: call   0x182f609b0
02156aa2: lea    rcx, [rip + 0x1ba5f37]                   ; [0x3cfc9e0] str:'[ENHANCED_OPENS]'
02156aa9: call   0x182f609b0
02156aae: lea    rcx, [rip + 0x1b4f9eb]                   ; [0x3ca64a0] str:'ENHANCED_OPENS'
02156ab5: call   0x182f609b0
02156aba: mov    byte ptr [rip + 0x1dc2bd8], 1            ; [0x3f19699] (bss)
02156ac1: xorps  xmm0, xmm0
02156ac4: movups xmmword ptr [rsp + 0xe8], xmm0
02156acc: xorps  xmm1, xmm1
02156acf: xor    eax, eax
02156ad1: movups xmmword ptr [rsp + 0xa0], xmm1
02156ad9: mov    qword ptr [rsp + 0xb0], rax
02156ae1: movups xmmword ptr [rsp + 0xf8], xmm0
02156ae9: mov    qword ptr [rsp + 0x108], rax
02156af1: mov    rax, qword ptr [rbx]
02156af4: test   rax, rax
02156af7: je     0x182157726
02156afd: mov    r15, qword ptr [rax + 0x30]
02156b01: mov    qword ptr [rsp + 0x48], r15
02156b06: mov    rcx, qword ptr [rip + 0x1b3d833]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156b0d: cmp    dword ptr [rcx + 0xe0], 0
02156b14: jne    0x182156b1b
02156b16: call   0x182f60cf0
02156b1b: xor    ecx, ecx
02156b1d: call   0x1821581e0                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʿʷʺˁʲʷʴʸʼʽʵ
02156b22: test   al, al
02156b24: jne    0x1821575a2
02156b2a: test   r15, r15
02156b2d: je     0x182157726
02156b33: xor    r9d, r9d
02156b36: mov    r8b, 3
02156b39: movzx  edx, r13b
02156b3d: mov    rcx, r15
02156b40: call   0x1820cfd40
02156b45: mov    r15, rax
02156b48: mov    rcx, qword ptr [rip + 0x1b3d7f1]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156b4f: cmp    dword ptr [rcx + 0xe0], 0
02156b56: jne    0x182156b5d
02156b58: call   0x182f60cf0
02156b5d: cmp    byte ptr [rip + 0x1dc2b3b], 0            ; [0x3f1969f] (bss)
02156b64: jne    0x182156b79
02156b66: lea    rcx, [rip + 0x1b3d7d3]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156b6d: call   0x182f609b0
02156b72: mov    byte ptr [rip + 0x1dc2b26], 1            ; [0x3f1969f] (bss)
02156b79: mov    rax, qword ptr [rbx]
02156b7c: test   rax, rax
02156b7f: je     0x182157726
02156b85: mov    ecx, dword ptr [rax + 0x20]
02156b88: cmp    ecx, 0x67
02156b8b: je     0x182156c87
02156b91: cmp    ecx, 0x74
02156b94: je     0x182156c87
02156b9a: mov    rcx, qword ptr [rip + 0x1b3d79f]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156ba1: cmp    dword ptr [rcx + 0xe0], 0
02156ba8: jne    0x182156baf
02156baa: call   0x182f60cf0
02156baf: cmp    byte ptr [rip + 0x1dc2ae0], 0            ; [0x3f19696] (bss)
02156bb6: jne    0x182156be3
02156bb8: lea    rcx, [rip + 0x1b54429]                   ; [0x3caafe8] metamethod:Method$System.Span<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Length()
02156bbf: call   0x182f609b0
02156bc4: lea    rcx, [rip + 0x1b3decd]                   ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02156bcb: call   0x182f609b0
02156bd0: lea    rcx, [rip + 0x1b9a999]                   ; [0x3cf1570] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʵʸʸʲˁˁʾʶʿʿʷ>()
02156bd7: call   0x182f609b0
02156bdc: mov    byte ptr [rip + 0x1dc2ab3], 1            ; [0x3f19696] (bss)
02156be3: xor    edi, edi
02156be5: mov    ebx, edi
02156be7: mov    rsi, qword ptr [rip + 0x1b9a982]         ; [0x3cf1570] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʵʸʸʲˁˁʾʶʿʿʷ>()
02156bee: cmp    qword ptr [rsi + 0x38], rbx
02156bf2: jne    0x182156bfc
02156bf4: mov    rcx, rsi
02156bf7: call   0x182f657d0
02156bfc: mov    r8, qword ptr [rsi + 0x38]
02156c00: mov    r8, qword ptr [r8 + 8]
02156c04: mov    rdx, r12
02156c07: lea    rcx, [rsp + 0x60]
02156c0c: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
02156c11: mov    r9, qword ptr [rsp + 0x60]
02156c16: mov    r8d, dword ptr [rsp + 0x68]
02156c1b: mov    ecx, edi
02156c1d: test   r8d, r8d
02156c20: jle    0x182156c65
02156c22: mov    r10, qword ptr [rip + 0x1b3de6f]         ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02156c29: nop    dword ptr [rax]
02156c30: cmp    ecx, r8d
02156c33: jae    0x182157767
02156c39: mov    eax, ecx
02156c3b: mov    rdx, qword ptr [r9 + rax*8]
02156c3f: test   rdx, rdx
02156c42: je     0x182156c71
02156c44: mov    rax, rdi
02156c47: cmp    qword ptr [rdx], r10
02156c4a: cmove  rax, rdx
02156c4e: test   rax, rax
02156c51: je     0x182156c71
02156c53: cmp    qword ptr [rax + 0x30], rdi
02156c57: je     0x182156c71
02156c59: cmp    byte ptr [rax + 0x28], 0x67
02156c5d: je     0x182156c6f
02156c5f: cmp    byte ptr [rax + 0x28], 0x74
02156c63: jne    0x182156c71
02156c65: mov    byte ptr [rsp + 0x1b8], 0x74
02156c6d: jmp    0x182156c9d
02156c6f: inc    ebx
02156c71: inc    ecx
02156c73: cmp    ecx, r8d
02156c76: jl     0x182156c33
02156c78: cmp    ebx, 2
02156c7b: jl     0x182156c65
02156c7d: mov    byte ptr [rsp + 0x1b8], 0x67
02156c85: jmp    0x182156c9d
02156c87: test   rax, rax
02156c8a: je     0x182157726
02156c90: movzx  eax, byte ptr [rax + 0x20]
02156c94: mov    byte ptr [rsp + 0x1b8], al
02156c9b: xor    edi, edi
02156c9d: test   r12, r12
02156ca0: je     0x182157726
02156ca6: mov    qword ptr [rsp + 0xe0], r15
02156cae: mov    byte ptr [rsp + 0x40], 0
02156cb3: mov    r8, qword ptr [rip + 0x1b79046]          ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
02156cba: mov    rdx, r12
02156cbd: lea    rcx, [rsp + 0x60]
02156cc2: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02156cc7: movups xmm0, xmmword ptr [rsp + 0x60]
02156ccc: movups xmmword ptr [rsp + 0xa0], xmm0
02156cd4: movsd  xmm1, qword ptr [rsp + 0x70]
02156cda: movsd  qword ptr [rsp + 0xb0], xmm1
02156ce3: mov    qword ptr [rsp + 0x50], rdi
02156ce8: lea    rbx, [rsp + 0xa0]
02156cf0: mov    qword ptr [rsp + 0x58], rbx
02156cf5: jmp    0x182156d00
02156cf7: movzx  r13d, byte ptr [rsp + 0x1c0]
02156d00: mov    rsi, qword ptr [rsp + 0x48]
02156d05: mov    rdx, qword ptr [rip + 0x1b69264]         ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
02156d0c: lea    rcx, [rsp + 0xa0]
02156d14: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02156d19: test   al, al
02156d1b: je     0x182157331
02156d21: mov    r14, qword ptr [rsp + 0xb0]
02156d29: test   r14, r14
02156d2c: je     0x182156d05
02156d2e: mov    rsi, rdi
02156d31: mov    rax, qword ptr [rip + 0x1b3dd60]         ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02156d38: cmp    qword ptr [r14], rax
02156d3b: cmove  rsi, r14
02156d3f: test   rsi, rsi
02156d42: je     0x182157007
02156d48: mov    rax, qword ptr [rsi + 0x30]
02156d4c: test   rax, rax
02156d4f: je     0x18215774a
02156d55: mov    r12, qword ptr [rsi + 0x10]
02156d59: mov    rcx, qword ptr [rax + 0x10]
02156d5d: mov    qword ptr [rsp + 0xc0], rcx
02156d65: mov    rdx, rcx
02156d68: sub    rdx, r12
02156d6b: mov    qword ptr [rsp + 0x80], rdx
02156d73: movzx  eax, byte ptr [rsp + 0x1b8]
02156d7b: cmp    byte ptr [rsi + 0x28], al
02156d7e: je     0x18215727c
02156d84: movzx  eax, byte ptr [rsi + 0x28]
02156d88: cmp    al, 0x67
02156d8a: jne    0x182156dab
02156d8c: test   r15, r15
02156d8f: je     0x18215772c
02156d95: xor    r9d, r9d
02156d98: mov    r8, rdx
02156d9b: mov    rdx, r12
02156d9e: mov    rcx, r15
02156da1: call   0x1820cf1b0
02156da6: jmp    0x182156d00
02156dab: cmp    al, 0x68
02156dad: jne    0x182156e40
02156db3: xorps  xmm0, xmm0
02156db6: movups xmmword ptr [rsp + 0xc0], xmm0
02156dbe: movups xmmword ptr [rsp + 0xd0], xmm0
02156dc6: mov    qword ptr [rsp + 0x30], rdi
02156dcb: mov    dword ptr [rsp + 0x28], edi
02156dcf: mov    byte ptr [rsp + 0x20], 0xff
02156dd4: mov    r9b, 5
02156dd7: mov    r8, rcx
02156dda: mov    rdx, r12
02156ddd: lea    rcx, [rsp + 0xc0]
02156de5: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
02156dea: movups xmm0, xmmword ptr [rsp + 0xc0]
02156df2: movups xmmword ptr [rsp + 0x110], xmm0
02156dfa: movups xmm1, xmmword ptr [rsp + 0xd0]
02156e02: movups xmmword ptr [rsp + 0x120], xmm1
02156e0a: mov    rcx, qword ptr [rip + 0x1b49187]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02156e11: cmp    dword ptr [rcx + 0xe0], 0
02156e18: jne    0x182156e1f
02156e1a: call   0x182f60cf0
02156e1f: xor    r9d, r9d
02156e22: lea    r8, [rsp + 0x110]
02156e2a: movzx  edx, r13b
02156e2e: mov    rsi, qword ptr [rsp + 0x48]
02156e33: mov    rcx, rsi
02156e36: call   0x1820cfbc0
02156e3b: jmp    0x182156d05
02156e40: cmp    al, 0x78
02156e42: jne    0x182156e68
02156e44: test   r15, r15
02156e47: je     0x182157731
02156e4d: mov    qword ptr [rsp + 0x20], rdi
02156e52: xor    r9d, r9d
02156e55: mov    r8, rdx
02156e58: mov    rdx, r12
02156e5b: mov    rcx, r15
02156e5e: call   0x1820cedc0
02156e63: jmp    0x182156d00
02156e68: cmp    al, 0x7e
02156e6a: jne    0x182156edb
02156e6c: movzx  esi, byte ptr [rsi + 0x29]
02156e70: mov    rcx, qword ptr [rip + 0x1b3d4c9]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156e77: cmp    dword ptr [rcx + 0xe0], 0
02156e7e: jne    0x182156e8d
02156e80: call   0x182f60cf0
02156e85: mov    rdx, qword ptr [rsp + 0x80]
02156e8d: cmp    sil, 0x33
02156e91: jae    0x182156eb1
02156e93: cmp    sil, 0x29
02156e97: jae    0x182156ead
02156e99: cmp    sil, 0x1f
02156e9d: jae    0x182156ea9
02156e9f: cmp    sil, 0x15
02156ea3: sbb    al, al
02156ea5: and    al, 3
02156ea7: jmp    0x182156eb3
02156ea9: mov    al, 1
02156eab: jmp    0x182156eb3
02156ead: mov    al, 2
02156eaf: jmp    0x182156eb3
02156eb1: mov    al, 3
02156eb3: test   r15, r15
02156eb6: je     0x182157736
02156ebc: xor    r9d, r9d
02156ebf: mov    qword ptr [rsp + 0x28], rdi
02156ec4: mov    byte ptr [rsp + 0x20], al
02156ec8: mov    r8, rdx
02156ecb: mov    rdx, r12
02156ece: mov    rcx, r15
02156ed1: call   0x1820ceec0
02156ed6: jmp    0x182156d00
02156edb: cmp    al, 0x7f
02156edd: jne    0x182156f34
02156edf: movzx  esi, byte ptr [rsi + 0x29]
02156ee3: mov    rcx, qword ptr [rip + 0x1b3d456]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02156eea: cmp    dword ptr [rcx + 0xe0], 0
02156ef1: jne    0x182156f00
02156ef3: call   0x182f60cf0
02156ef8: mov    rdx, qword ptr [rsp + 0x80]
02156f00: cmp    sil, 0x33
02156f04: jae    0x182156f24
02156f06: cmp    sil, 0x29
02156f0a: jae    0x182156f20
02156f0c: cmp    sil, 0x1f
02156f10: jae    0x182156f1c
02156f12: cmp    sil, 0x15
02156f16: sbb    al, al
02156f18: and    al, 3
02156f1a: jmp    0x182156f26
02156f1c: mov    al, 1
02156f1e: jmp    0x182156f26
02156f20: mov    al, 2
02156f22: jmp    0x182156f26
02156f24: mov    al, 3
02156f26: test   r15, r15
02156f29: je     0x18215773b
02156f2f: mov    r9b, 1
02156f32: jmp    0x182156ebf
02156f34: movzx  ecx, byte ptr [rsi + 0x28]
02156f38: xor    edx, edx
02156f3a: call   0x18210d9c0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʼʴʷʽˀʺʻʹʿʺʲ
02156f3f: movzx  r13d, al
02156f43: cmp    al, 0xff
02156f45: je     0x182156cf7
02156f4b: xor    edx, edx
02156f4d: movzx  ecx, byte ptr [rsi + 0x28]
02156f51: call   0x18210d780                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʳʺʾʶʷʻʷʴʴʶˀ
02156f56: test   al, al
02156f58: jne    0x1821571db
02156f5e: xor    edx, edx
02156f60: movzx  ecx, byte ptr [rsi + 0x28]
02156f64: call   0x18210d850                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʹʲʽʶʶʶʾʻʵʸʺ
02156f69: test   al, al
02156f6b: jne    0x18215719d
02156f71: movsx  edx, byte ptr [rsp + 0x1c0]
02156f79: lea    eax, [rdx - 4]
02156f7c: cmp    eax, 1
02156f7f: jbe    0x182156fac
02156f81: lea    eax, [rdx - 0xa]
02156f84: cmp    eax, 1
02156f87: jbe    0x182156fac
02156f89: cmp    dl, 0xe
02156f8c: je     0x182156fac
02156f8e: xor    edx, edx
02156f90: movzx  ecx, byte ptr [rsi + 0x28]
02156f94: call   0x18210da30                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʽʿʳʵʾʲʺʷʻʴʼ
02156f99: mov    esi, eax
02156f9b: cmp    byte ptr [rsp + 0x40], 0
02156fa0: jne    0x182156fb9
02156fa2: cmp    eax, 1
02156fa5: jne    0x182156fb9
02156fa7: jmp    0x182156cf7
02156fac: xor    edx, edx
02156fae: movzx  ecx, byte ptr [rsi + 0x28]
02156fb2: call   0x18210d8e0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʻʳʼʵʷʹʵˁʺʿʼ
02156fb7: mov    esi, eax
02156fb9: test   esi, esi
02156fbb: je     0x182156cf7
02156fc1: xor    r9d, r9d
02156fc4: movzx  r8d, r13b
02156fc8: movzx  r13d, byte ptr [rsp + 0x1c0]
02156fd1: movzx  edx, r13b
02156fd5: mov    rcx, qword ptr [rsp + 0x48]
02156fda: call   0x1820cfd40
02156fdf: test   rax, rax
02156fe2: je     0x182157740
02156fe8: mov    qword ptr [rsp + 0x28], rdi
02156fed: mov    dword ptr [rsp + 0x20], edi
02156ff1: mov    r9, qword ptr [rsp + 0x80]
02156ff9: mov    r8d, esi
02156ffc: mov    rdx, r12
02156fff: mov    rcx, rax
02157002: call   0x1820cec60
02157007: mov    rcx, rdi
0215700a: mov    rax, qword ptr [rip + 0x1b49257]         ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
02157011: cmp    qword ptr [r14], rax
02157014: cmove  rcx, r14
02157018: test   rcx, rcx
0215701b: je     0x18215729b
02157021: cmp    qword ptr [rcx + 0x20], 0
02157026: je     0x18215729b
0215702c: mov    rsi, qword ptr [rcx + 0x20]
02157030: cmp    byte ptr [rip + 0x1db4f92], 0            ; [0x3f0bfc9] (bss)
02157037: jne    0x18215704c
02157039: lea    rcx, [rip + 0x1b4a568]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02157040: call   0x182f609b0
02157045: mov    byte ptr [rip + 0x1db4f7d], 1            ; [0x3f0bfc9] (bss)
0215704c: xorps  xmm6, xmm6
0215704f: test   rsi, rsi
02157052: je     0x18215707f
02157054: xor    edx, edx
02157056: mov    rcx, rsi
02157059: call   0x1819829d0                              ; System.String$$GetRawStringData
0215705e: mov    dword ptr [rsp + 0x13c], edi
02157065: mov    qword ptr [rsp + 0x130], rax
0215706d: mov    eax, dword ptr [rsi + 0x10]
02157070: mov    dword ptr [rsp + 0x138], eax
02157077: movaps xmm6, xmmword ptr [rsp + 0x130]
0215707f: mov    rsi, qword ptr [rip + 0x1b4f41a]         ; [0x3ca64a0] str:'ENHANCED_OPENS'
02157086: cmp    byte ptr [rip + 0x1db50ce], 0            ; [0x3f0c15b] (bss)
0215708d: jne    0x1821570a2
0215708f: lea    rcx, [rip + 0x1b4a512]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02157096: call   0x182f609b0
0215709b: mov    byte ptr [rip + 0x1db50b9], 1            ; [0x3f0c15b] (bss)
021570a2: xorps  xmm0, xmm0
021570a5: test   rsi, rsi
021570a8: je     0x1821570d5
021570aa: xor    edx, edx
021570ac: mov    rcx, rsi
021570af: call   0x1819829d0                              ; System.String$$GetRawStringData
021570b4: mov    dword ptr [rsp + 0x14c], edi
021570bb: mov    qword ptr [rsp + 0x140], rax
021570c3: mov    eax, dword ptr [rsi + 0x10]
021570c6: mov    dword ptr [rsp + 0x148], eax
021570cd: movaps xmm0, xmmword ptr [rsp + 0x140]
021570d5: movdqa xmmword ptr [rsp + 0xc0], xmm0
021570de: movdqa xmmword ptr [rsp + 0x80], xmm6
021570e7: xor    r9d, r9d
021570ea: mov    r8d, 5
021570f0: lea    rdx, [rsp + 0xc0]
021570f8: lea    rcx, [rsp + 0x80]
02157100: call   0x181b53020                              ; System.MemoryExtensions$$Equals
02157105: test   al, al
02157107: jne    0x182157193
0215710d: mov    rsi, qword ptr [rip + 0x1ba58cc]         ; [0x3cfc9e0] str:'[ENHANCED_OPENS]'
02157114: cmp    byte ptr [rip + 0x1db5041], al           ; [0x3f0c15b] (bss)
0215711a: jne    0x18215712f
0215711c: lea    rcx, [rip + 0x1b4a485]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02157123: call   0x182f609b0
02157128: mov    byte ptr [rip + 0x1db502c], 1            ; [0x3f0c15b] (bss)
0215712f: xorps  xmm0, xmm0
02157132: test   rsi, rsi
02157135: je     0x182157156
02157137: xor    edx, edx
02157139: mov    rcx, rsi
0215713c: call   0x1819829d0                              ; System.String$$GetRawStringData
02157141: mov    dword ptr [rsp + 0x6c], edi
02157145: mov    qword ptr [rsp + 0x60], rax
0215714a: mov    eax, dword ptr [rsi + 0x10]
0215714d: mov    dword ptr [rsp + 0x68], eax
02157151: movaps xmm0, xmmword ptr [rsp + 0x60]
02157156: movdqa xmmword ptr [rsp + 0xc0], xmm0
0215715f: movdqa xmmword ptr [rsp + 0x80], xmm6
02157168: xor    r9d, r9d
0215716b: mov    r8d, 5
02157171: lea    rdx, [rsp + 0xc0]
02157179: lea    rcx, [rsp + 0x80]
02157181: call   0x181b53020                              ; System.MemoryExtensions$$Equals
02157186: test   al, al
02157188: mov    rsi, qword ptr [rsp + 0x48]
0215718d: je     0x182156d05
02157193: mov    byte ptr [rsp + 0x40], 1
02157198: jmp    0x182156d00
0215719d: xorps  xmm0, xmm0
021571a0: movups xmmword ptr [rsp + 0x80], xmm0
021571a8: movups xmmword ptr [rsp + 0x90], xmm0
021571b0: mov    qword ptr [rsp + 0x30], rdi
021571b5: mov    dword ptr [rsp + 0x28], edi
021571b9: mov    byte ptr [rsp + 0x20], r13b
021571be: mov    r9b, 2
021571c1: mov    r8, qword ptr [rsp + 0xc0]
021571c9: mov    rdx, r12
021571cc: lea    rcx, [rsp + 0x80]
021571d4: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
021571d9: jmp    0x182157217
021571db: xorps  xmm0, xmm0
021571de: movups xmmword ptr [rsp + 0x80], xmm0
021571e6: movups xmmword ptr [rsp + 0x90], xmm0
021571ee: mov    qword ptr [rsp + 0x30], rdi
021571f3: mov    dword ptr [rsp + 0x28], edi
021571f7: mov    byte ptr [rsp + 0x20], r13b
021571fc: mov    r9b, 3
021571ff: mov    r8, qword ptr [rsp + 0xc0]
02157207: mov    rdx, r12
0215720a: lea    rcx, [rsp + 0x80]
02157212: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
02157217: mov    rcx, qword ptr [rip + 0x1b48d7a]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
0215721e: movups xmm1, xmmword ptr [rsp + 0x90]
02157226: movups xmm0, xmmword ptr [rsp + 0x80]
0215722e: cmp    dword ptr [rcx + 0xe0], 0
02157235: movups xmmword ptr [rsp + 0x120], xmm1
0215723d: movups xmmword ptr [rsp + 0x110], xmm0
02157245: jne    0x18215724c
02157247: call   0x182f60cf0
0215724c: mov    qword ptr [rsp + 0x20], rdi
02157251: lea    r9, [rsp + 0x110]
02157259: movzx  r8d, r13b
0215725d: movzx  r13d, byte ptr [rsp + 0x1c0]
02157266: movzx  edx, r13b
0215726a: mov    rsi, qword ptr [rsp + 0x48]
0215726f: mov    rcx, rsi
02157272: call   0x1820cfb80                              ; ˀˀʴʸʵʷʾʵʻʼʹ$$ʷʶʺʺˁʿʾʹʽʼˁ
02157277: jmp    0x182156d05
0215727c: test   r15, r15
0215727f: je     0x182157745
02157285: xor    r9d, r9d
02157288: mov    r8, rdx
0215728b: mov    rdx, r12
0215728e: mov    rcx, r15
02157291: call   0x1820cf0b0
02157296: jmp    0x182156d00
0215729b: mov    rsi, rdi
0215729e: mov    rax, qword ptr [rip + 0x1b4221b]         ; [0x3c994c0] meta:ʺʾʺˀʾʶʲʷʷʺʳ_TypeInfo
021572a5: cmp    qword ptr [r14], rax
021572a8: cmove  rsi, r14
021572ac: test   rsi, rsi
021572af: je     0x182156d00
021572b5: cmp    byte ptr [rsi + 0x22], 1
021572b9: je     0x1821572fb
021572bb: cmp    byte ptr [rsi + 0x22], 4
021572bf: jne    0x182156d00
021572c5: mov    rcx, qword ptr [rip + 0x1b48ccc]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
021572cc: cmp    dword ptr [rcx + 0xe0], 0
021572d3: jne    0x1821572da
021572d5: call   0x182f60cf0
021572da: mov    qword ptr [rsp + 0x20], rdi
021572df: mov    r9, rsi
021572e2: mov    r8b, 5
021572e5: movzx  edx, r13b
021572e9: mov    rsi, qword ptr [rsp + 0x48]
021572ee: mov    rcx, rsi
021572f1: call   0x182152970
021572f6: jmp    0x182156d05
021572fb: mov    rcx, qword ptr [rip + 0x1b48c96]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02157302: cmp    dword ptr [rcx + 0xe0], 0
02157309: jne    0x182157310
0215730b: call   0x182f60cf0
02157310: mov    qword ptr [rsp + 0x20], rdi
02157315: mov    r9, rsi
02157318: mov    r8b, 1
0215731b: movzx  edx, r13b
0215731f: mov    rsi, qword ptr [rsp + 0x48]
02157324: mov    rcx, rsi
02157327: call   0x182152970
0215732c: jmp    0x182156d05
02157331: mov    rdx, qword ptr [rip + 0x1b68b78]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
02157338: mov    rcx, rbx
0215733b: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02157340: jmp    0x182157370
02157342: mov    rdx, qword ptr [rip + 0x1b68b67]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
02157349: mov    rcx, qword ptr [rsp + 0x58]
0215734e: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02157353: mov    rcx, qword ptr [rsp + 0x50]
02157358: test   rcx, rcx
0215735b: jne    0x182157750
02157361: xor    edi, edi
02157363: mov    rsi, qword ptr [rsp + 0x48]
02157368: mov    r15, qword ptr [rsp + 0xe0]
02157370: xor    r9d, r9d
02157373: lea    r8, [rsp + 0xe8]
0215737b: movzx  edx, byte ptr [rsp + 0x1c0]
02157383: mov    rcx, rsi
02157386: call   0x182152b20
0215738b: test   al, al
0215738d: je     0x182157580
02157393: mov    r13, qword ptr [rsp + 0xe8]
0215739b: mov    qword ptr [rsp + 0xc0], r13
021573a3: mov    r12d, dword ptr [rsp + 0xf0]
021573ab: mov    dword ptr [rsp + 0x48], r12d
021573b0: mov    r14d, edi
021573b3: mov    dword ptr [rsp + 0x1b8], r14d
021573bb: nop    dword ptr [rax + rax]
021573c0: cmp    r14d, r12d
021573c3: jge    0x182157547
021573c9: jae    0x182157767
021573cf: movsxd rax, r14d
021573d2: mov    rsi, qword ptr [r13 + rax*8]
021573d7: test   rsi, rsi
021573da: je     0x18215753f
021573e0: cmp    rsi, r15
021573e3: je     0x18215753f
021573e9: test   r15, r15
021573ec: je     0x182157726
021573f2: mov    rcx, qword ptr [rsi + 0x78]
021573f6: test   rcx, rcx
021573f9: je     0x182157726
021573ff: mov    r8, qword ptr [rip + 0x1b7a4ba]          ; [0x3cd18c0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.AddRange()
02157406: mov    rdx, qword ptr [r15 + 0x78]
0215740a: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
0215740f: mov    rcx, qword ptr [rsi + 0x70]
02157413: test   rcx, rcx
02157416: je     0x182157726
0215741c: mov    r8, qword ptr [rip + 0x1b7839d]          ; [0x3ccf7c0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.AddRange()
02157423: mov    rdx, qword ptr [r15 + 0x70]
02157427: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
0215742c: mov    rcx, qword ptr [rsi + 0x88]
02157433: test   rcx, rcx
02157436: je     0x182157726
0215743c: mov    r8, qword ptr [rip + 0x1b75e05]          ; [0x3ccd248] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.AddRange()
02157443: mov    rdx, qword ptr [r15 + 0x88]
0215744a: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
0215744f: mov    rdx, qword ptr [r15 + 0x80]
02157456: test   rdx, rdx
02157459: je     0x182157726
0215745f: mov    r8, qword ptr [rip + 0x1b76f1a]          ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
02157466: lea    rcx, [rsp + 0x60]
0215746b: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02157470: movups xmm0, xmmword ptr [rsp + 0x60]
02157475: movups xmmword ptr [rsp + 0xf8], xmm0
0215747d: movsd  xmm1, qword ptr [rsp + 0x70]
02157483: movsd  qword ptr [rsp + 0x108], xmm1
0215748c: mov    qword ptr [rsp + 0x50], rdi
02157491: lea    rbx, [rsp + 0xf8]
02157499: mov    qword ptr [rsp + 0x58], rbx
0215749e: nop    
021574a0: mov    rdx, qword ptr [rip + 0x1b681c9]         ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
021574a7: lea    rcx, [rsp + 0xf8]
021574af: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
021574b4: test   al, al
021574b6: je     0x1821574f0
021574b8: mov    rdx, qword ptr [rsp + 0x108]
021574c0: test   rdx, rdx
021574c3: je     0x18215775b
021574c9: movzx  eax, byte ptr [rdx + 0x20]
021574cd: cmp    byte ptr [rsi + 0x59], al
021574d0: jl     0x1821574a0
021574d2: mov    rcx, qword ptr [rsi + 0x80]
021574d9: test   rcx, rcx
021574dc: je     0x182157756
021574e2: mov    r8, qword ptr [rip + 0x1b76d17]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
021574e9: call   0x182c425f0
021574ee: jmp    0x1821574a0
021574f0: mov    rdx, qword ptr [rip + 0x1b680b9]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
021574f7: mov    rcx, rbx
021574fa: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021574ff: jmp    0x18215753f
02157501: mov    rdx, qword ptr [rip + 0x1b680a8]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
02157508: mov    rcx, qword ptr [rsp + 0x58]
0215750d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02157512: mov    rcx, qword ptr [rsp + 0x50]
02157517: test   rcx, rcx
0215751a: jne    0x182157761
02157520: xor    edi, edi
02157522: mov    r15, qword ptr [rsp + 0xe0]
0215752a: mov    r14d, dword ptr [rsp + 0x1b8]
02157532: mov    r12d, dword ptr [rsp + 0x48]
02157537: mov    r13, qword ptr [rsp + 0xc0]
0215753f: inc    r14d
02157542: jmp    0x1821573b3
02157547: mov    rsi, qword ptr [rsp + 0xe8]
0215754f: mov    ebx, dword ptr [rsp + 0xf0]
02157556: test   ebx, ebx
02157558: jle    0x182157580
0215755a: nop    word ptr [rax + rax]
02157560: cmp    edi, ebx
02157562: jae    0x182157767
02157568: mov    eax, edi
0215756a: mov    rcx, qword ptr [rsi + rax*8]
0215756e: test   rcx, rcx
02157571: je     0x18215757a
02157573: xor    edx, edx
02157575: call   0x18215c320                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʻʳʶʷʷʽʶʼʽʽ
0215757a: inc    edi
0215757c: cmp    edi, ebx
0215757e: jl     0x182157562
02157580: lea    r11, [rsp + 0x180]
02157588: mov    rbx, qword ptr [r11 + 0x30]
0215758c: mov    rsi, qword ptr [r11 + 0x48]
02157590: movaps xmm6, xmmword ptr [r11 - 0x10]
02157595: mov    rsp, r11
02157598: pop    r15
0215759a: pop    r14
0215759c: pop    r13
0215759e: pop    r12
021575a0: pop    rdi
021575a1: ret    
021575a2: test   r12, r12
021575a5: je     0x182157726
021575ab: xor    r14b, r14b
021575ae: mov    r8, qword ptr [rip + 0x1b7874b]          ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
021575b5: mov    rdx, r12
021575b8: lea    rcx, [rsp + 0x60]
021575bd: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
021575c2: movups xmm0, xmmword ptr [rsp + 0x60]
021575c7: movups xmmword ptr [rsp + 0xa0], xmm0
021575cf: movsd  xmm1, qword ptr [rsp + 0x70]
021575d5: movsd  qword ptr [rsp + 0xb0], xmm1
021575de: xor    edi, edi
021575e0: mov    qword ptr [rsp + 0x50], rdi
021575e5: lea    rbx, [rsp + 0xa0]
021575ed: mov    qword ptr [rsp + 0x58], rbx
021575f2: nop    dword ptr [rax]
021575f6: nop    word ptr [rax + rax]
02157600: mov    rdx, qword ptr [rip + 0x1b68969]         ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
02157607: lea    rcx, [rsp + 0xa0]
0215760f: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02157614: test   al, al
02157616: je     0x1821576f2
0215761c: mov    rdx, qword ptr [rsp + 0xb0]
02157624: test   rdx, rdx
02157627: je     0x182157600
02157629: mov    rcx, rdi
0215762c: mov    rax, qword ptr [rip + 0x1b3d465]         ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02157633: cmp    qword ptr [rdx], rax
02157636: cmove  rcx, rdx
0215763a: test   rcx, rcx
0215763d: je     0x182157600
0215763f: movzx  ecx, byte ptr [rcx + 0x28]
02157643: xor    edx, edx
02157645: call   0x18210d9c0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʼʴʷʽˀʺʻʹʿʺʲ
0215764a: cmp    al, 0xff
0215764c: je     0x182157600
0215764e: mov    ecx, eax
02157650: and    ecx, 0x1f
02157653: mov    esi, 1
02157658: shl    sil, cl
0215765b: movzx  ecx, sil
0215765f: and    cl, r14b
02157662: cmp    cl, sil
02157665: je     0x182157600
02157667: test   r15, r15
0215766a: je     0x182157772
02157670: xor    r9d, r9d
02157673: movzx  r8d, al
02157677: movzx  edx, r13b
0215767b: mov    rcx, r15
0215767e: call   0x1820cfd40
02157683: test   rax, rax
02157686: je     0x18215776d
0215768c: mov    qword ptr [rsp + 0x28], rdi
02157691: mov    dword ptr [rsp + 0x20], edi
02157695: xor    r9d, r9d
02157698: xor    r8d, r8d
0215769b: xor    edx, edx
0215769d: mov    rcx, rax
021576a0: call   0x1820cec60
021576a5: mov    rdx, qword ptr [rip + 0x1b3cc94]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021576ac: cmp    dword ptr [rdx + 0xe0], 0
021576b3: jne    0x1821576c4
021576b5: mov    rcx, rdx
021576b8: call   0x182f60cf0
021576bd: mov    rdx, qword ptr [rip + 0x1b3cc7c]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021576c4: or     r14b, sil
021576c7: mov    rax, qword ptr [rdx + 0xb8]
021576ce: movzx  ecx, r14b
021576d2: and    ecx, dword ptr [rax + 8]
021576d5: cmp    ecx, dword ptr [rax + 8]
021576d8: jne    0x182157600
021576de: mov    rdx, qword ptr [rip + 0x1b687cb]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021576e5: mov    rcx, rbx
021576e8: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021576ed: jmp    0x182157580
021576f2: mov    rdx, qword ptr [rip + 0x1b687b7]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021576f9: mov    rcx, rbx
021576fc: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02157701: jmp    0x182157580
02157706: mov    rdx, qword ptr [rip + 0x1b687a3]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
0215770d: mov    rcx, qword ptr [rsp + 0x58]
02157712: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02157717: mov    rcx, qword ptr [rsp + 0x50]
0215771c: test   rcx, rcx
0215771f: jne    0x182157778
02157721: jmp    0x182157580
02157726: call   0x182f60c50
0215772b: nop    
0215772c: call   0x182f60c50
02157731: call   0x182f60c50
02157736: call   0x182f60c50
0215773b: call   0x182f60c50
02157740: call   0x182f60c50
02157745: call   0x182f60c50
0215774a: call   0x182f60c50
0215774f: nop    
02157750: call   0x182f60ce0
02157755: nop    
02157756: call   0x182f60c50
0215775b: call   0x182f60c50
02157760: nop    
02157761: call   0x182f60ce0
02157766: int3   
02157767: call   0x182f60c40
0215776c: nop    
0215776d: call   0x182f60c50
02157772: call   0x182f60c50
02157777: nop    
02157778: call   0x182f60ce0
0215777d: int3   
0215777e: int3   
