0215cd50: push   rbp
0215cd52: push   rbx
0215cd53: push   rsi
0215cd54: push   rdi
0215cd55: push   r13
0215cd57: lea    rbp, [rsp - 0x37]
0215cd5c: sub    rsp, 0xc0
0215cd63: cmp    byte ptr [rip + 0x1dbc9e5], 0            ; [0x3f1974f] (bss)
0215cd6a: mov    r13, rcx
0215cd6d: movaps xmmword ptr [rsp + 0xa0], xmm6
0215cd75: jne    0x18215ce1e
0215cd7b: lea    rcx, [rip + 0x1b7a99e]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215cd82: call   0x182f609b0
0215cd87: lea    rcx, [rip + 0x1b70f3a]                   ; [0x3ccdcc8] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.get_Count()
0215cd8e: call   0x182f609b0
0215cd93: lea    rcx, [rip + 0x1b7aa46]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215cd9a: call   0x182f609b0
0215cd9f: lea    rcx, [rip + 0x1b70fe2]                   ; [0x3ccdd88] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.get_Item()
0215cda6: call   0x182f609b0
0215cdab: lea    rcx, [rip + 0x1bacdc6]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215cdb2: call   0x182f609b0
0215cdb7: lea    rcx, [rip + 0x1bacf32]                   ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0215cdbe: call   0x182f609b0
0215cdc3: lea    rcx, [rip + 0x1b44c16]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215cdca: call   0x182f609b0
0215cdcf: lea    rcx, [rip + 0x1b44f8a]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
0215cdd6: call   0x182f609b0
0215cddb: lea    rcx, [rip + 0x1b4ec16]                   ; [0x3cab9f8] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.op_Implicit()
0215cde2: call   0x182f609b0
0215cde7: lea    rcx, [rip + 0x1b3d732]                   ; [0x3c9a520] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʾˁʲʻʹʽʶʹʶʲʹ<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215cdee: call   0x182f609b0
0215cdf3: lea    rcx, [rip + 0x1b94d76]                   ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215cdfa: call   0x182f609b0
0215cdff: lea    rcx, [rip + 0x1b865f2]                   ; [0x3ce33f8] str:'dnoflip'
0215ce06: call   0x182f609b0
0215ce0b: lea    rcx, [rip + 0x1b8be36]                   ; [0x3ce8c48] str:'drums'
0215ce12: call   0x182f609b0
0215ce17: mov    byte ptr [rip + 0x1dbc931], 1            ; [0x3f1974f] (bss)
0215ce1e: mov    rbx, qword ptr [rip + 0x1b94d4b]         ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215ce25: mov    rdi, qword ptr [r13 + 0x60]
0215ce29: cmp    qword ptr [rbx + 0x38], 0
0215ce2e: jne    0x18215ce38
0215ce30: mov    rcx, rbx
0215ce33: call   0x182f657d0
0215ce38: mov    r8, qword ptr [rbx + 0x38]
0215ce3c: lea    rcx, [rbp - 0x19]
0215ce40: mov    qword ptr [rsp + 0xf8], r12
0215ce48: mov    rdx, rdi
0215ce4b: mov    qword ptr [rsp + 0xb8], r14
0215ce53: mov    qword ptr [rsp + 0xb0], r15
0215ce5b: mov    r8, qword ptr [r8 + 8]
0215ce5f: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0215ce64: movaps xmm0, xmmword ptr [rbp - 0x19]
0215ce68: lea    rdx, [rbp - 0x19]
0215ce6c: mov    r8, qword ptr [rip + 0x1b4eb85]          ; [0x3cab9f8] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.op_Implicit()
0215ce73: lea    rcx, [rbp - 0x29]
0215ce77: movdqa xmmword ptr [rbp - 0x19], xmm0
0215ce7c: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
0215ce81: mov    rax, qword ptr [r13 + 0x90]
0215ce88: xor    esi, esi
0215ce8a: movaps xmm6, xmmword ptr [rbp - 0x29]
0215ce8e: xor    ecx, ecx
0215ce90: mov    dword ptr [rbp + 0x77], esi
0215ce93: test   rax, rax
0215ce96: je     0x18215d28a
0215ce9c: nop    dword ptr [rax]
0215cea0: cmp    ecx, dword ptr [rax + 0x18]
0215cea3: jge    0x18215d25c
0215cea9: mov    rcx, qword ptr [r13 + 0x90]
0215ceb0: test   rcx, rcx
0215ceb3: je     0x18215d28a
0215ceb9: mov    r8, qword ptr [rip + 0x1b70ec8]          ; [0x3ccdd88] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.get_Item()
0215cec0: mov    edx, esi
0215cec2: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215cec7: mov    r12, rax
0215ceca: test   rax, rax
0215cecd: je     0x18215d28a
0215ced3: cmp    byte ptr [rip + 0x1daf0ef], 0            ; [0x3f0bfc9] (bss)
0215ceda: mov    rbx, qword ptr [rax + 0x20]
0215cede: jne    0x18215cef3
0215cee0: lea    rcx, [rip + 0x1b446c1]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215cee7: call   0x182f609b0
0215ceec: mov    byte ptr [rip + 0x1daf0d6], 1            ; [0x3f0bfc9] (bss)
0215cef3: xorps  xmm1, xmm1
0215cef6: test   rbx, rbx
0215cef9: je     0x18215cf1a
0215cefb: xor    edx, edx
0215cefd: mov    dword ptr [rbp - 0x5d], 0
0215cf04: mov    rcx, rbx
0215cf07: call   0x1819829d0                              ; System.String$$GetRawStringData
0215cf0c: mov    qword ptr [rbp - 0x69], rax
0215cf10: mov    eax, dword ptr [rbx + 0x10]
0215cf13: mov    dword ptr [rbp - 0x61], eax
0215cf16: movaps xmm1, xmmword ptr [rbp - 0x69]
0215cf1a: movdqa xmm0, xmm1
0215cf1e: psrldq xmm0, 8
0215cf23: movd   eax, xmm0
0215cf27: cmp    eax, 6
0215cf2a: jg     0x18215cf36
0215cf2c: xor    ebx, ebx
0215cf2e: mov    byte ptr [rbp + 0x67], bl
0215cf31: jmp    0x18215d007
0215cf36: jbe    0x18215d290
0215cf3c: movq   rdi, xmm1
0215cf41: cmp    word ptr [rdi + 0xc], 0x64
0215cf46: sete   bl
0215cf49: cmp    eax, 0xd
0215cf4c: jge    0x18215cf57
0215cf4e: mov    byte ptr [rbp + 0x67], 0
0215cf52: jmp    0x18215d007
0215cf57: mov    rsi, qword ptr [rip + 0x1b44a82]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
0215cf5e: add    eax, -6
0215cf61: cmp    eax, 7
0215cf64: jae    0x18215cf6d
0215cf66: xor    ecx, ecx
0215cf68: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215cf6d: mov    rcx, qword ptr [rsi + 0x20]
0215cf71: add    rdi, 0xc
0215cf75: mov    dword ptr [rbp - 0x3d], 0
0215cf7c: test   byte ptr [rcx + 0x135], 1
0215cf83: jne    0x18215cf8a
0215cf85: call   0x182f65750
0215cf8a: cmp    byte ptr [rip + 0x1daf1ca], 0            ; [0x3f0c15b] (bss)
0215cf91: mov    qword ptr [rbp - 0x49], rdi
0215cf95: mov    rdi, qword ptr [rip + 0x1b8645c]         ; [0x3ce33f8] str:'dnoflip'
0215cf9c: mov    dword ptr [rbp - 0x41], 7
0215cfa3: jne    0x18215cfb8
0215cfa5: lea    rcx, [rip + 0x1b445fc]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215cfac: call   0x182f609b0
0215cfb1: mov    byte ptr [rip + 0x1daf1a3], 1            ; [0x3f0c15b] (bss)
0215cfb8: xorps  xmm0, xmm0
0215cfbb: test   rdi, rdi
0215cfbe: je     0x18215cfdf
0215cfc0: xor    edx, edx
0215cfc2: mov    dword ptr [rbp - 0x4d], 0
0215cfc9: mov    rcx, rdi
0215cfcc: call   0x1819829d0                              ; System.String$$GetRawStringData
0215cfd1: mov    qword ptr [rbp - 0x59], rax
0215cfd5: mov    eax, dword ptr [rdi + 0x10]
0215cfd8: mov    dword ptr [rbp - 0x51], eax
0215cfdb: movaps xmm0, xmmword ptr [rbp - 0x59]
0215cfdf: mov    r8, qword ptr [rip + 0x1bacb92]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0215cfe6: lea    rdx, [rbp - 9]
0215cfea: movdqa xmmword ptr [rbp - 9], xmm0
0215cfef: lea    rcx, [rbp + 7]
0215cff3: movaps xmm0, xmmword ptr [rbp - 0x49]
0215cff7: movdqa xmmword ptr [rbp + 7], xmm0
0215cffc: call   0x182c61850
0215d001: mov    esi, dword ptr [rbp + 0x77]
0215d004: mov    byte ptr [rbp + 0x67], al
0215d007: movzx  ecx, byte ptr [r13 + 0x59]
0215d00c: cmp    byte ptr [r12 + 0x18], cl
0215d011: jne    0x18215d244
0215d017: test   bl, bl
0215d019: je     0x18215d244
0215d01f: mov    r15d, esi
0215d022: movzx  r8d, byte ptr [r13 + 0x59]
0215d027: xor    r9d, r9d
0215d02a: mov    edx, r15d
0215d02d: mov    rcx, r13
0215d030: call   0x1821600c0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʸʻʿʶʲʲʲʼʼʾ
0215d035: mov    r15d, eax
0215d038: cmp    eax, -1
0215d03b: je     0x18215d17a
0215d041: mov    rcx, qword ptr [r13 + 0x90]
0215d048: test   rcx, rcx
0215d04b: je     0x18215d28a
0215d051: mov    r8, qword ptr [rip + 0x1b70d30]          ; [0x3ccdd88] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.get_Item()
0215d058: mov    edx, eax
0215d05a: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215d05f: test   rax, rax
0215d062: je     0x18215d28a
0215d068: cmp    byte ptr [rip + 0x1daef5a], 0            ; [0x3f0bfc9] (bss)
0215d06f: mov    rbx, qword ptr [rax + 0x20]
0215d073: jne    0x18215d088
0215d075: lea    rcx, [rip + 0x1b4452c]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215d07c: call   0x182f609b0
0215d081: mov    byte ptr [rip + 0x1daef41], 1            ; [0x3f0bfc9] (bss)
0215d088: xorps  xmm1, xmm1
0215d08b: test   rbx, rbx
0215d08e: je     0x18215d0af
0215d090: xor    edx, edx
0215d092: mov    dword ptr [rbp - 0x2d], 0
0215d099: mov    rcx, rbx
0215d09c: call   0x1819829d0                              ; System.String$$GetRawStringData
0215d0a1: mov    qword ptr [rbp - 0x39], rax
0215d0a5: mov    eax, dword ptr [rbx + 0x10]
0215d0a8: mov    dword ptr [rbp - 0x31], eax
0215d0ab: movaps xmm1, xmmword ptr [rbp - 0x39]
0215d0af: cmp    byte ptr [rip + 0x1daf0a5], 0            ; [0x3f0c15b] (bss)
0215d0b6: movdqa xmm0, xmm1
0215d0ba: mov    rsi, qword ptr [rip + 0x1b8bb87]         ; [0x3ce8c48] str:'drums'
0215d0c1: movq   rbx, xmm1
0215d0c6: psrldq xmm0, 8
0215d0cb: psrldq xmm1, 0xc
0215d0d0: movd   r14d, xmm0
0215d0d5: movd   edi, xmm1
0215d0d9: jne    0x18215d0ee
0215d0db: lea    rcx, [rip + 0x1b444c6]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0215d0e2: call   0x182f609b0
0215d0e7: mov    byte ptr [rip + 0x1daf06d], 1            ; [0x3f0c15b] (bss)
0215d0ee: xorps  xmm0, xmm0
0215d0f1: test   rsi, rsi
0215d0f4: je     0x18215d115
0215d0f6: xor    edx, edx
0215d0f8: mov    dword ptr [rbp - 0x1d], 0
0215d0ff: mov    rcx, rsi
0215d102: call   0x1819829d0                              ; System.String$$GetRawStringData
0215d107: mov    qword ptr [rbp - 0x29], rax
0215d10b: mov    eax, dword ptr [rsi + 0x10]
0215d10e: mov    dword ptr [rbp - 0x21], eax
0215d111: movaps xmm0, xmmword ptr [rbp - 0x29]
0215d115: mov    r8, qword ptr [rip + 0x1bacbd4]          ; [0x3d09cf0] metamethod:Method$System.MemoryExtensions.StartsWith<char>()
0215d11c: lea    rdx, [rbp + 7]
0215d120: lea    rcx, [rbp - 0x19]
0215d124: movdqa xmmword ptr [rbp + 7], xmm0
0215d129: mov    qword ptr [rbp - 0x19], rbx
0215d12d: mov    dword ptr [rbp - 0x11], r14d
0215d131: mov    dword ptr [rbp - 0xd], edi
0215d134: call   0x182c61990
0215d139: test   al, al
0215d13b: je     0x18215d022
0215d141: cmp    r14d, 6
0215d145: jne    0x18215d022
0215d14b: mov    rcx, qword ptr [r13 + 0x90]
0215d152: test   rcx, rcx
0215d155: je     0x18215d28a
0215d15b: mov    r8, qword ptr [rip + 0x1b70c26]          ; [0x3ccdd88] metamethod:Method$System.Collections.Generic.List<ʲˀʿʹʹʻʵʺʴʻʷ>.get_Item()
0215d162: mov    edx, r15d
0215d165: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215d16a: mov    rbx, qword ptr [r12 + 0x10]
0215d16f: test   rax, rax
0215d172: je     0x18215d17f
0215d174: mov    r8, qword ptr [rax + 0x10]
0215d178: jmp    0x18215d1ae
0215d17a: mov    rbx, qword ptr [r12 + 0x10]
0215d17f: mov    rcx, qword ptr [r13 + 0x60]
0215d183: test   rcx, rcx
0215d186: je     0x18215d28a
0215d18c: mov    edx, dword ptr [rcx + 0x18]
0215d18f: mov    r8, qword ptr [rip + 0x1b7a64a]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215d196: dec    edx
0215d198: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215d19d: test   rax, rax
0215d1a0: je     0x18215d28a
0215d1a6: mov    r8, qword ptr [rax + 0x10]
0215d1aa: add    r8, 2
0215d1ae: cmp    rbx, r8
0215d1b1: je     0x18215d241
0215d1b7: mov    r9, qword ptr [rip + 0x1b3d362]          ; [0x3c9a520] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʾˁʲʻʹʽʶʹʶʲʹ<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215d1be: lea    rcx, [rbp + 7]
0215d1c2: dec    r8
0215d1c5: movdqa xmmword ptr [rbp + 7], xmm6
0215d1ca: mov    rdx, rbx
0215d1cd: call   0x1805fad90                              ; ʽˁʷʳʸʶʳʵʸʾʿ$$ʾˁʲʻʹʽʶʹʶʲʹ<object>
0215d1d2: mov    rbx, rax
0215d1d5: cmp    eax, -1
0215d1d8: je     0x18215d241
0215d1da: mov    rdi, rax
0215d1dd: shr    rdi, 0x20
0215d1e1: cmp    edi, -1
0215d1e4: je     0x18215d241
0215d1e6: cmp    eax, edi
0215d1e8: jg     0x18215d241
0215d1ea: movzx  esi, byte ptr [rbp + 0x67]
0215d1ee: nop    
0215d1f0: mov    rcx, qword ptr [r13 + 0x60]
0215d1f4: test   rcx, rcx
0215d1f7: je     0x18215d28a
0215d1fd: mov    r8, qword ptr [rip + 0x1b7a5dc]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215d204: mov    edx, ebx
0215d206: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215d20b: test   rax, rax
0215d20e: je     0x18215d28a
0215d210: mov    ecx, dword ptr [rax + 0x20]
0215d213: cmp    ecx, 0xe
0215d216: je     0x18215d21d
0215d218: cmp    ecx, 0xf
0215d21b: jne    0x18215d23b
0215d21d: test   sil, sil
0215d220: jne    0x18215d231
0215d222: mov    ecx, dword ptr [rax + 0x24]
0215d225: and    ecx, 0xffffffbf
0215d228: bts    ecx, 7
0215d22c: mov    dword ptr [rax + 0x24], ecx
0215d22f: jmp    0x18215d23b
0215d231: test   byte ptr [rax + 0x24], 0x80
0215d235: jne    0x18215d23b
0215d237: or     dword ptr [rax + 0x24], 0x40
0215d23b: inc    ebx
0215d23d: cmp    ebx, edi
0215d23f: jle    0x18215d1f0
0215d241: mov    esi, dword ptr [rbp + 0x77]
0215d244: mov    rax, qword ptr [r13 + 0x90]
0215d24b: inc    esi
0215d24d: mov    dword ptr [rbp + 0x77], esi
0215d250: mov    ecx, esi
0215d252: test   rax, rax
0215d255: je     0x18215d28a
0215d257: jmp    0x18215cea0
0215d25c: mov    r15, qword ptr [rsp + 0xb0]
0215d264: mov    r14, qword ptr [rsp + 0xb8]
0215d26c: mov    r12, qword ptr [rsp + 0xf8]
0215d274: movaps xmm6, xmmword ptr [rsp + 0xa0]
0215d27c: add    rsp, 0xc0
0215d283: pop    r13
0215d285: pop    rdi
0215d286: pop    rsi
0215d287: pop    rbx
0215d288: pop    rbp
0215d289: ret    
0215d28a: call   0x182f60c50
0215d28f: int3   
0215d290: call   0x182f60c40
0215d295: int3   
0215d296: int3   
