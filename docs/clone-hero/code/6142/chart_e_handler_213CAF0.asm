0213caf0: mov    qword ptr [rsp + 0x10], rbx
0213caf5: mov    qword ptr [rsp + 0x20], r9
0213cafa: push   rbp
0213cafb: push   rsi
0213cafc: push   rdi
0213cafd: push   r12
0213caff: push   r13
0213cb01: push   r14
0213cb03: push   r15
0213cb05: mov    rbp, rsp
0213cb08: sub    rsp, 0x80
0213cb0f: cmp    byte ptr [rip + 0x1ddcb48], 0            ; [0x3f1965e] (bss)
0213cb16: mov    rbx, r9
0213cb19: mov    r15, r8
0213cb1c: mov    rsi, rdx
0213cb1f: mov    r12, rcx
0213cb22: jne    0x18213cbcb
0213cb28: lea    rcx, [rip + 0x1b95091]                   ; [0x3cd1bc0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Count()
0213cb2f: call   0x182f609b0
0213cb34: lea    rcx, [rip + 0x1b95145]                   ; [0x3cd1c80] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Item()
0213cb3b: call   0x182f609b0
0213cb40: lea    rcx, [rip + 0x1bcd1a9]                   ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0213cb47: call   0x182f609b0
0213cb4c: lea    rcx, [rip + 0x1b64e8d]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0213cb53: call   0x182f609b0
0213cb58: lea    rcx, [rip + 0x1bb6641]                   ; [0x3cf31a0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʴʸʲʾʹˀʿʳʴʸʷ()
0213cb5f: call   0x182f609b0
0213cb64: lea    rcx, [rip + 0x1bb66f5]                   ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213cb6b: call   0x182f609b0
0213cb70: lea    rcx, [rip + 0x1b57e61]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213cb77: call   0x182f609b0
0213cb7c: lea    rcx, [rip + 0x1b5974d]                   ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0213cb83: call   0x182f609b0
0213cb88: lea    rcx, [rip + 0x1b5bcb9]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0213cb8f: call   0x182f609b0
0213cb94: lea    rcx, [rip + 0x1b7a4f5]                   ; [0x3cb7090] str:'solo'
0213cb9b: call   0x182f609b0
0213cba0: lea    rcx, [rip + 0x1ba9101]                   ; [0x3ce5ca8] str:'mix'
0213cba7: call   0x182f609b0
0213cbac: lea    rcx, [rip + 0x1b7c095]                   ; [0x3cb8c48] str:'soloend'
0213cbb3: call   0x182f609b0
0213cbb8: lea    rcx, [rip + 0x1bc40d9]                   ; [0x3d00c98] str:'[mix'
0213cbbf: call   0x182f609b0
0213cbc4: mov    byte ptr [rip + 0x1ddca93], 1            ; [0x3f1965e] (bss)
0213cbcb: mov    rcx, qword ptr [rip + 0x1b57e06]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213cbd2: xor    eax, eax
0213cbd4: movsxd r13, dword ptr [rbx + 8]
0213cbd8: xorps  xmm0, xmm0
0213cbdb: mov    r14, qword ptr [r12]
0213cbdf: mov    edi, dword ptr [r12 + 8]
0213cbe4: mov    ebx, dword ptr [rbx + 8]
0213cbe7: movups xmmword ptr [rbp - 0x40], xmm0
0213cbeb: mov    qword ptr [rbp - 0x30], rax
0213cbef: mov    dword ptr [rbp + 0x40], eax
0213cbf2: cmp    dword ptr [rcx + 0xe0], eax
0213cbf8: jne    0x18213cbff
0213cbfa: call   0x182f60cf0
0213cbff: cmp    byte ptr [rip + 0x1dd145c], 0            ; [0x3f0e062] (bss)
0213cc06: jne    0x18213cc1b
0213cc08: lea    rcx, [rip + 0x1b65151]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
0213cc0f: call   0x182f609b0
0213cc14: mov    byte ptr [rip + 0x1dd1447], 1            ; [0x3f0e062] (bss)
0213cc1b: movaps xmmword ptr [rsp + 0x70], xmm6
0213cc20: movaps xmmword ptr [rsp + 0x60], xmm7
0213cc25: cmp    ebx, edi
0213cc27: jge    0x18213cc69
0213cc29: nop    dword ptr [rax]
0213cc30: jae    0x18213d150
0213cc36: movsxd rax, ebx
0213cc39: movzx  ecx, word ptr [r14 + rax*2]
0213cc3e: cmp    cx, 0xa
0213cc42: je     0x18213cc69
0213cc44: cmp    cx, 0xd
0213cc48: jne    0x18213cc63
0213cc4a: lea    eax, [rbx + 1]
0213cc4d: cmp    eax, edi
0213cc4f: jge    0x18213cc63
0213cc51: jae    0x18213d150
0213cc57: movsxd rax, ebx
0213cc5a: cmp    word ptr [r14 + rax*2 + 2], 0xa
0213cc61: je     0x18213cc69
0213cc63: inc    ebx
0213cc65: cmp    ebx, edi
0213cc67: jl     0x18213cc30
0213cc69: mov    rax, qword ptr [rbp + 0x58]
0213cc6d: mov    dword ptr [rax + 8], ebx
0213cc70: sub    ebx, r13d
0213cc73: mov    r14, qword ptr [rip + 0x1b64d66]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0213cc7a: cmp    r13d, edi
0213cc7d: ja     0x18213cc88
0213cc7f: mov    eax, edi
0213cc81: sub    eax, r13d
0213cc84: cmp    ebx, eax
0213cc86: jbe    0x18213cc8f
0213cc88: xor    ecx, ecx
0213cc8a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0213cc8f: mov    rax, qword ptr [r12]
0213cc93: xor    r12d, r12d
0213cc96: mov    rcx, qword ptr [r14 + 0x20]
0213cc9a: mov    dword ptr [rbp - 0x54], r12d
0213cc9e: lea    rdi, [rax + r13*2]
0213cca2: test   byte ptr [rcx + 0x135], 1
0213cca9: jne    0x18213ccb0
0213ccab: call   0x182f65750
0213ccb0: mov    qword ptr [rbp - 0x60], rdi
0213ccb4: lea    rdx, [rbp - 0x60]
0213ccb8: mov    dword ptr [rbp - 0x58], ebx
0213ccbb: lea    rcx, [rbp - 0x50]
0213ccbf: movaps xmm0, xmmword ptr [rbp - 0x60]
0213ccc3: xor    r8d, r8d
0213ccc6: movdqa xmmword ptr [rbp - 0x60], xmm0
0213cccb: call   0x181b53ef0                              ; System.MemoryExtensions$$Trim
0213ccd0: mov    rdi, qword ptr [rbp + 0x58]
0213ccd4: cmp    byte ptr [rdi + 0xc], r12b
0213ccd8: jne    0x18213d013
0213ccde: cmp    byte ptr [rip + 0x1dcf476], r12b         ; [0x3f0c15b] (bss)
0213cce5: movups xmm7, xmmword ptr [rax]
0213cce8: mov    rbx, qword ptr [rip + 0x1bc3fa9]         ; [0x3d00c98] str:'[mix'
0213ccef: jne    0x18213cd04
0213ccf1: lea    rcx, [rip + 0x1b648b0]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0213ccf8: call   0x182f609b0
0213ccfd: mov    byte ptr [rip + 0x1dcf457], 1            ; [0x3f0c15b] (bss)
0213cd04: test   rbx, rbx
0213cd07: je     0x18213cd27
0213cd09: xor    edx, edx
0213cd0b: mov    dword ptr [rbp - 0x54], r12d
0213cd0f: mov    rcx, rbx
0213cd12: call   0x1819829d0                              ; System.String$$GetRawStringData
0213cd17: mov    qword ptr [rbp - 0x60], rax
0213cd1b: mov    eax, dword ptr [rbx + 0x10]
0213cd1e: mov    dword ptr [rbp - 0x58], eax
0213cd21: movaps xmm0, xmmword ptr [rbp - 0x60]
0213cd25: jmp    0x18213cd2a
0213cd27: xorps  xmm0, xmm0
0213cd2a: mov    r8, qword ptr [rip + 0x1bccfbf]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0213cd31: lea    rdx, [rbp - 0x60]
0213cd35: lea    rcx, [rbp - 0x50]
0213cd39: movdqa xmmword ptr [rbp - 0x60], xmm0
0213cd3e: movdqa xmmword ptr [rbp - 0x50], xmm7
0213cd43: call   0x182c61990
0213cd48: test   al, al
0213cd4a: jne    0x18213d038
0213cd50: cmp    byte ptr [rip + 0x1dcf404], r12b         ; [0x3f0c15b] (bss)
0213cd57: mov    rbx, qword ptr [rip + 0x1ba8f4a]         ; [0x3ce5ca8] str:'mix'
0213cd5e: jne    0x18213cd73
0213cd60: lea    rcx, [rip + 0x1b64841]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0213cd67: call   0x182f609b0
0213cd6c: mov    byte ptr [rip + 0x1dcf3e8], 1            ; [0x3f0c15b] (bss)
0213cd73: test   rbx, rbx
0213cd76: je     0x18213cd96
0213cd78: xor    edx, edx
0213cd7a: mov    dword ptr [rbp - 0x54], r12d
0213cd7e: mov    rcx, rbx
0213cd81: call   0x1819829d0                              ; System.String$$GetRawStringData
0213cd86: mov    qword ptr [rbp - 0x60], rax
0213cd8a: mov    eax, dword ptr [rbx + 0x10]
0213cd8d: mov    dword ptr [rbp - 0x58], eax
0213cd90: movaps xmm0, xmmword ptr [rbp - 0x60]
0213cd94: jmp    0x18213cd99
0213cd96: xorps  xmm0, xmm0
0213cd99: mov    r8, qword ptr [rip + 0x1bccf50]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0213cda0: lea    rdx, [rbp - 0x50]
0213cda4: lea    rcx, [rbp - 0x60]
0213cda8: movdqa xmmword ptr [rbp - 0x50], xmm0
0213cdad: movdqa xmmword ptr [rbp - 0x60], xmm7
0213cdb2: call   0x182c61990
0213cdb7: test   al, al
0213cdb9: jne    0x18213d038
0213cdbf: cmp    byte ptr [rip + 0x1dcf395], r12b         ; [0x3f0c15b] (bss)
0213cdc6: mov    rbx, qword ptr [rip + 0x1b7a2c3]         ; [0x3cb7090] str:'solo'
0213cdcd: jne    0x18213cde2
0213cdcf: lea    rcx, [rip + 0x1b647d2]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0213cdd6: call   0x182f609b0
0213cddb: mov    byte ptr [rip + 0x1dcf379], 1            ; [0x3f0c15b] (bss)
0213cde2: test   rbx, rbx
0213cde5: je     0x18213ce05
0213cde7: xor    edx, edx
0213cde9: mov    dword ptr [rbp - 0x54], r12d
0213cded: mov    rcx, rbx
0213cdf0: call   0x1819829d0                              ; System.String$$GetRawStringData
0213cdf5: mov    qword ptr [rbp - 0x60], rax
0213cdf9: mov    eax, dword ptr [rbx + 0x10]
0213cdfc: mov    dword ptr [rbp - 0x58], eax
0213cdff: movaps xmm6, xmmword ptr [rbp - 0x60]
0213ce03: jmp    0x18213ce08
0213ce05: xorps  xmm6, xmm6
0213ce08: mov    rcx, qword ptr [rip + 0x1b5ba39]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0213ce0f: cmp    dword ptr [rcx + 0xe0], r12d
0213ce16: jne    0x18213ce1d
0213ce18: call   0x182f60cf0
0213ce1d: xor    r9d, r9d
0213ce20: movdqa xmmword ptr [rbp - 0x50], xmm6
0213ce25: xor    r8d, r8d
0213ce28: movdqa xmmword ptr [rbp - 0x60], xmm7
0213ce2d: lea    rdx, [rbp - 0x50]
0213ce31: lea    rcx, [rbp - 0x60]
0213ce35: call   0x1820fe510                              ; ʹʾʽʼʷʲʴʸʷʼˁ$$ʼʽʸʵʻʺʼʶʶʲʹ
0213ce3a: test   al, al
0213ce3c: jne    0x18213cf1f
0213ce42: cmp    byte ptr [rip + 0x1dcf312], r12b         ; [0x3f0c15b] (bss)
0213ce49: mov    rbx, qword ptr [rip + 0x1b7bdf8]         ; [0x3cb8c48] str:'soloend'
0213ce50: jne    0x18213ce65
0213ce52: lea    rcx, [rip + 0x1b6474f]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0213ce59: call   0x182f609b0
0213ce5e: mov    byte ptr [rip + 0x1dcf2f6], 1            ; [0x3f0c15b] (bss)
0213ce65: test   rbx, rbx
0213ce68: je     0x18213ce88
0213ce6a: xor    edx, edx
0213ce6c: mov    dword ptr [rbp - 0x54], r12d
0213ce70: mov    rcx, rbx
0213ce73: call   0x1819829d0                              ; System.String$$GetRawStringData
0213ce78: mov    qword ptr [rbp - 0x60], rax
0213ce7c: mov    eax, dword ptr [rbx + 0x10]
0213ce7f: mov    dword ptr [rbp - 0x58], eax
0213ce82: movaps xmm6, xmmword ptr [rbp - 0x60]
0213ce86: jmp    0x18213ce8b
0213ce88: xorps  xmm6, xmm6
0213ce8b: mov    rcx, qword ptr [rip + 0x1b5b9b6]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0213ce92: cmp    dword ptr [rcx + 0xe0], r12d
0213ce99: jne    0x18213cea0
0213ce9b: call   0x182f60cf0
0213cea0: xor    r9d, r9d
0213cea3: movdqa xmmword ptr [rbp - 0x50], xmm6
0213cea8: xor    r8d, r8d
0213ceab: movdqa xmmword ptr [rbp - 0x60], xmm7
0213ceb0: lea    rdx, [rbp - 0x50]
0213ceb4: lea    rcx, [rbp - 0x60]
0213ceb8: call   0x1820fe510                              ; ʹʾʽʼʷʲʴʸʷʼˁ$$ʼʽʸʵʻʺʼʶʶʲʹ
0213cebd: test   al, al
0213cebf: je     0x18213d013
0213cec5: mov    r8, qword ptr [rip + 0x1bb6394]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213cecc: lea    rcx, [rdi + 0xe]
0213ced0: lea    rdx, [rbp + 0x40]
0213ced4: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213ced9: test   al, al
0213cedb: je     0x18213d013
0213cee1: test   rsi, rsi
0213cee4: je     0x18213d14a
0213ceea: mov    rcx, qword ptr [rsi + 0x78]
0213ceee: test   rcx, rcx
0213cef1: je     0x18213d14a
0213cef7: mov    r8, qword ptr [rip + 0x1b94d82]          ; [0x3cd1c80] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Item()
0213cefe: mov    edx, dword ptr [rbp + 0x40]
0213cf01: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0213cf06: test   rax, rax
0213cf09: je     0x18213d14a
0213cf0f: sub    r15, qword ptr [rax + 0x10]
0213cf13: inc    r15
0213cf16: mov    qword ptr [rax + 0x18], r15
0213cf1a: jmp    0x18213d013
0213cf1f: test   rsi, rsi
0213cf22: je     0x18213d14a
0213cf28: mov    rdx, qword ptr [rsi + 0x78]
0213cf2c: test   rdx, rdx
0213cf2f: je     0x18213d14a
0213cf35: mov    r8, qword ptr [rip + 0x1bb6264]          ; [0x3cf31a0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʴʸʲʾʹˀʿʳʴʸʷ()
0213cf3c: lea    rcx, [rdi + 0xe]
0213cf40: mov    edx, dword ptr [rdx + 0x18]
0213cf43: call   0x180f9e570                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʴʸʲʾʹˀʿʳʴʸʷ
0213cf48: cmp    byte ptr [rip + 0x1ddc3d2], r12b         ; [0x3f19321] (bss)
0213cf4f: jne    0x18213cf70
0213cf51: lea    rcx, [rip + 0x1b948a8]                   ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
0213cf58: call   0x182f609b0
0213cf5d: lea    rcx, [rip + 0x1bbcaac]                   ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213cf64: call   0x182f609b0
0213cf69: mov    byte ptr [rip + 0x1ddc3b1], 1            ; [0x3f19321] (bss)
0213cf70: mov    rcx, qword ptr [rsi + 0x40]
0213cf74: test   rcx, rcx
0213cf77: je     0x18213d14a
0213cf7d: mov    rdx, qword ptr [rip + 0x1bbca8c]         ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213cf84: mov    rbx, qword ptr [rsi + 0x78]
0213cf88: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0213cf8d: test   rax, rax
0213cf90: je     0x18213d14a
0213cf96: xor    r9d, r9d
0213cf99: xor    r8d, r8d
0213cf9c: mov    rdx, r15
0213cf9f: mov    rcx, rax
0213cfa2: call   0x1820d5e60                              ; ʸʵʵʾʿˀʺʽʲʾˁ$$ʹʳʾˁʺʾʹˁʻʹʲ
0213cfa7: mov    r9, rax
0213cfaa: test   rbx, rbx
0213cfad: je     0x18213d14a
0213cfb3: mov    rax, qword ptr [rip + 0x1b94846]         ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
0213cfba: inc    dword ptr [rbx + 0x1c]
0213cfbd: mov    rcx, qword ptr [rbx + 0x10]
0213cfc1: test   rcx, rcx
0213cfc4: je     0x18213d14a
0213cfca: movsxd rdx, dword ptr [rbx + 0x18]
0213cfce: cmp    edx, dword ptr [rcx + 0x18]
0213cfd1: jb     0x18213cfef
0213cfd3: mov    rax, qword ptr [rax + 0x20]
0213cfd7: mov    rdx, r9
0213cfda: mov    rcx, rbx
0213cfdd: mov    r8, qword ptr [rax + 0xc0]
0213cfe4: mov    r8, qword ptr [r8 + 0x70]
0213cfe8: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0213cfed: jmp    0x18213d013
0213cfef: lea    eax, [rdx + 1]
0213cff2: mov    dword ptr [rbx + 0x18], eax
0213cff5: cmp    edx, dword ptr [rcx + 0x18]
0213cff8: jae    0x18213d150
0213cffe: mov    qword ptr [rcx + rdx*8 + 0x20], r9
0213d003: lea    rcx, [rcx + rdx*8]
0213d007: add    rcx, 0x20
0213d00b: mov    rdx, r9
0213d00e: call   0x182f5fc00
0213d013: movaps xmm7, xmmword ptr [rsp + 0x60]
0213d018: movaps xmm6, xmmword ptr [rsp + 0x70]
0213d01d: mov    rbx, qword ptr [rsp + 0xc8]
0213d025: add    rsp, 0x80
0213d02c: pop    r15
0213d02e: pop    r14
0213d030: pop    r13
0213d032: pop    r12
0213d034: pop    rdi
0213d035: pop    rsi
0213d036: pop    rbp
0213d037: ret    
0213d038: mov    rcx, qword ptr [rip + 0x1b59291]         ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0213d03f: cmp    dword ptr [rcx + 0xe0], r12d
0213d046: jne    0x18213d04d
0213d048: call   0x182f60cf0
0213d04d: xor    r9d, r9d
0213d050: movdqa xmmword ptr [rbp - 0x50], xmm7
0213d055: lea    r8, [rbp - 0x40]
0213d059: mov    rdx, r15
0213d05c: lea    rcx, [rbp - 0x50]
0213d060: call   0x18215c750                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʼʻʹʾʲʼʹʷʸʳ
0213d065: test   al, al
0213d067: je     0x18213d013
0213d069: test   rsi, rsi
0213d06c: je     0x18213d14a
0213d072: mov    rbx, qword ptr [rbp - 0x40]
0213d076: cmp    bl, byte ptr [rsi + 0x59]
0213d079: jne    0x18213d013
0213d07b: cmp    byte ptr [rip + 0x1ddc5e3], r12b         ; [0x3f19665] (bss)
0213d082: mov    r14, qword ptr [rbp - 0x38]
0213d086: mov    r15, qword ptr [rbp - 0x30]
0213d08a: jne    0x18213d0ab
0213d08c: lea    rcx, [rip + 0x1b90ab5]                   ; [0x3ccdb48] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.Add()
0213d093: call   0x182f609b0
0213d098: lea    rcx, [rip + 0x1bbb429]                   ; [0x3cf84c8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲˀʿʹʹʻʵʺʴʻʷ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213d09f: call   0x182f609b0
0213d0a4: mov    byte ptr [rip + 0x1ddc5ba], 1            ; [0x3f19665] (bss)
0213d0ab: mov    rcx, qword ptr [rsi + 0x28]
0213d0af: test   rcx, rcx
0213d0b2: je     0x18213d14a
0213d0b8: mov    rdx, qword ptr [rip + 0x1bbb409]         ; [0x3cf84c8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲˀʿʹʹʻʵʺʴʻʷ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213d0bf: mov    rdi, qword ptr [rsi + 0x90]
0213d0c6: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0213d0cb: mov    rsi, rax
0213d0ce: test   rax, rax
0213d0d1: je     0x18213d14a
0213d0d3: lea    rcx, [rax + 0x20]
0213d0d7: mov    qword ptr [rax + 0x10], r15
0213d0db: mov    rdx, r14
0213d0de: mov    byte ptr [rax + 0x18], bl
0213d0e1: mov    qword ptr [rax + 0x20], r14
0213d0e5: call   0x182f5fc00
0213d0ea: test   rdi, rdi
0213d0ed: je     0x18213d14a
0213d0ef: mov    rax, qword ptr [rip + 0x1b90a52]         ; [0x3ccdb48] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.Add()
0213d0f6: inc    dword ptr [rdi + 0x1c]
0213d0f9: mov    rcx, qword ptr [rdi + 0x10]
0213d0fd: test   rcx, rcx
0213d100: je     0x18213d14a
0213d102: movsxd rdx, dword ptr [rdi + 0x18]
0213d106: cmp    edx, dword ptr [rcx + 0x18]
0213d109: jb     0x18213d12a
0213d10b: mov    rax, qword ptr [rax + 0x20]
0213d10f: mov    rdx, rsi
0213d112: mov    rcx, rdi
0213d115: mov    r8, qword ptr [rax + 0xc0]
0213d11c: mov    r8, qword ptr [r8 + 0x70]
0213d120: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0213d125: jmp    0x18213d013
0213d12a: lea    eax, [rdx + 1]
0213d12d: mov    dword ptr [rdi + 0x18], eax
0213d130: cmp    edx, dword ptr [rcx + 0x18]
0213d133: jae    0x18213d150
0213d135: mov    qword ptr [rcx + rdx*8 + 0x20], rsi
0213d13a: lea    rcx, [rcx + rdx*8]
0213d13e: add    rcx, 0x20
0213d142: mov    rdx, rsi
0213d145: jmp    0x18213d00e
0213d14a: call   0x182f60c50
0213d14f: int3   
0213d150: call   0x182f60c40
0213d155: int3   
0213d156: int3   
