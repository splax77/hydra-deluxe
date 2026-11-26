02114a80: mov    qword ptr [rsp + 0x10], rbx
02114a85: mov    qword ptr [rsp + 8], rcx
02114a8a: push   rsi
02114a8b: push   rdi
02114a8c: push   r12
02114a8e: push   r14
02114a90: push   r15
02114a92: sub    rsp, 0x140
02114a99: mov    r12, rcx
02114a9c: cmp    byte ptr [rip + 0x1e04a68], 0            ; [0x3f1950b] (bss)
02114aa3: jne    0x182114b88
02114aa9: lea    rcx, [rip + 0x1bcfa18]                   ; [0x3ce44c8] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>.GetEnumerator()
02114ab0: call   0x182f609b0
02114ab5: lea    rcx, [rip + 0x1bd167c]                   ; [0x3ce6138] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.get_Item()
02114abc: call   0x182f609b0
02114ac1: lea    rcx, [rip + 0x1bd33a8]                   ; [0x3ce7e70] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.get_Item()
02114ac8: call   0x182f609b0
02114acd: lea    rcx, [rip + 0x1bf3acc]                   ; [0x3d085a0] metamethod:Method$System.Collections.Generic.Dictionary<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.GetEnumerator()
02114ad4: call   0x182f609b0
02114ad9: lea    rcx, [rip + 0x1bb6048]                   ; [0x3ccab28] metamethod:Method$System.Collections.Generic.Dictionary.Enumerator<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.Dispose()
02114ae0: call   0x182f609b0
02114ae5: lea    rcx, [rip + 0x1bae2fc]                   ; [0x3cc2de8] metamethod:Method$System.Collections.Generic.List.Enumerator<ˁʼʹʿʴʳˀʻʾʶʷ>.Dispose()
02114aec: call   0x182f609b0
02114af1: lea    rcx, [rip + 0x1bae3b0]                   ; [0x3cc2ea8] metamethod:Method$System.Collections.Generic.List.Enumerator<ˁʼʹʿʴʳˀʻʾʶʷ>.MoveNext()
02114af8: call   0x182f609b0
02114afd: lea    rcx, [rip + 0x1bb60e4]                   ; [0x3ccabe8] metamethod:Method$System.Collections.Generic.Dictionary.Enumerator<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.MoveNext()
02114b04: call   0x182f609b0
02114b09: lea    rcx, [rip + 0x1bb6198]                   ; [0x3ccaca8] metamethod:Method$System.Collections.Generic.Dictionary.Enumerator<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.get_Current()
02114b10: call   0x182f609b0
02114b15: lea    rcx, [rip + 0x1bae44c]                   ; [0x3cc2f68] metamethod:Method$System.Collections.Generic.List.Enumerator<ˁʼʹʿʴʳˀʻʾʶʷ>.get_Current()
02114b1c: call   0x182f609b0
02114b21: lea    rcx, [rip + 0x1be91a8]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
02114b28: call   0x182f609b0
02114b2d: lea    rcx, [rip + 0x1bc341c]                   ; [0x3cd7f50] meta:System.Collections.Generic.IEnumerator<ˀʸʲˁʽʴʶʶʷʴʾ>_TypeInfo
02114b34: call   0x182f609b0
02114b39: lea    rcx, [rip + 0x1bea6c0]                   ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
02114b40: call   0x182f609b0
02114b45: lea    rcx, [rip + 0x1bd7e54]                   ; [0x3cec9a0] metamethod:Method$System.Collections.Generic.KeyValuePair<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.Deconstruct()
02114b4c: call   0x182f609b0
02114b51: lea    rcx, [rip + 0x1bc4488]                   ; [0x3cd8fe0] metamethod:Method$System.Collections.Generic.List<ˁʼʹʿʴʳˀʻʾʶʷ>.GetEnumerator()
02114b58: call   0x182f609b0
02114b5d: lea    rcx, [rip + 0x1bf9404]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114b64: call   0x182f609b0
02114b69: lea    rcx, [rip + 0x1b8abd0]                   ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
02114b70: call   0x182f609b0
02114b75: lea    rcx, [rip + 0x1bbd19c]                   ; [0x3cd1d18] str:'%internal%'
02114b7c: call   0x182f609b0
02114b81: mov    byte ptr [rip + 0x1e04983], 1            ; [0x3f1950b] (bss)
02114b88: xorps  xmm0, xmm0
02114b8b: xor    eax, eax
02114b8d: movups xmmword ptr [rsp + 0xa0], xmm0
02114b95: movups xmmword ptr [rsp + 0xb0], xmm0
02114b9d: mov    qword ptr [rsp + 0xc0], rax
02114ba5: movups xmmword ptr [rsp + 0xc8], xmm0
02114bad: xor    r15d, r15d
02114bb0: mov    qword ptr [rsp + 0x40], r15
02114bb5: mov    qword ptr [rsp + 0x38], r15
02114bba: movups xmmword ptr [rsp + 0x80], xmm0
02114bc2: movups xmmword ptr [rsp + 0x90], xmm0
02114bca: mov    qword ptr [rsp + 0x188], r15
02114bd2: mov    rax, qword ptr [rip + 0x1bf938f]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114bd9: cmp    dword ptr [rax + 0xe0], r15d
02114be0: jne    0x182114bf1
02114be2: mov    rcx, rax
02114be5: call   0x182f60cf0
02114bea: mov    rax, qword ptr [rip + 0x1bf9377]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114bf1: mov    rax, qword ptr [rax + 0xb8]
02114bf8: mov    rcx, qword ptr [rax + 8]
02114bfc: test   rcx, rcx
02114bff: je     0x18211509e
02114c05: mov    r8, qword ptr [rip + 0x1bd152c]          ; [0x3ce6138] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.get_Item()
02114c0c: mov    rdx, r12
02114c0f: call   0x18131b8e0                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$get_Item
02114c14: xor    r8d, r8d
02114c17: mov    rdx, qword ptr [rip + 0x1bbd0fa]         ; [0x3cd1d18] str:'%internal%'
02114c1e: mov    rcx, rax
02114c21: call   0x181989ec0                              ; System.String$$op_Inequality
02114c26: test   al, al
02114c28: je     0x182114cb5
02114c2e: mov    rax, qword ptr [rip + 0x1bf9333]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114c35: cmp    dword ptr [rax + 0xe0], r15d
02114c3c: jne    0x182114c4d
02114c3e: mov    rcx, rax
02114c41: call   0x182f60cf0
02114c46: mov    rax, qword ptr [rip + 0x1bf931b]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114c4d: mov    rax, qword ptr [rax + 0xb8]
02114c54: mov    rcx, qword ptr [rax + 8]
02114c58: test   rcx, rcx
02114c5b: je     0x18211509e
02114c61: mov    r8, qword ptr [rip + 0x1bd14d0]          ; [0x3ce6138] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.get_Item()
02114c68: mov    rdx, r12
02114c6b: call   0x18131b8e0                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$get_Item
02114c70: xor    edx, edx
02114c72: mov    rcx, rax
02114c75: call   0x181b1ab40                              ; System.IO.File$$Exists
02114c7a: test   al, al
02114c7c: jne    0x182114cb5
02114c7e: mov    rcx, qword ptr [rip + 0x1bf92e3]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114c85: cmp    dword ptr [rcx + 0xe0], r15d
02114c8c: jne    0x182114c93
02114c8e: call   0x182f60cf0
02114c93: xor    edx, edx
02114c95: mov    rcx, r12
02114c98: call   0x182112db0                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʷʶʿʲʾʽʷʺʳʲʵ
02114c9d: mov    rbx, qword ptr [rsp + 0x178]
02114ca5: add    rsp, 0x140
02114cac: pop    r15
02114cae: pop    r14
02114cb0: pop    r12
02114cb2: pop    rdi
02114cb3: pop    rsi
02114cb4: ret    
02114cb5: mov    rcx, qword ptr [rip + 0x1bf92ac]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114cbc: cmp    dword ptr [rcx + 0xe0], r15d
02114cc3: jne    0x182114cca
02114cc5: call   0x182f60cf0
02114cca: xor    edx, edx
02114ccc: mov    rcx, r12
02114ccf: call   0x182115160                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˁʲʹʼʷʴʿʲʻʿʾ
02114cd4: mov    rbx, rax
02114cd7: xor    edx, edx
02114cd9: mov    rcx, r12
02114cdc: call   0x182114600                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʽʽʵʶʸʹʸʽʼʳʹ
02114ce1: mov    r14, rax
02114ce4: mov    qword ptr [rsp + 0x30], rax
02114ce9: test   rax, rax
02114cec: je     0x182114c9d
02114cee: test   rbx, rbx
02114cf1: je     0x18211509e
02114cf7: mov    byte ptr [rsp + 0x180], r15b
02114cff: mov    r8, qword ptr [rip + 0x1bf389a]          ; [0x3d085a0] metamethod:Method$System.Collections.Generic.Dictionary<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.GetEnumerator()
02114d06: mov    rdx, rbx
02114d09: lea    rcx, [rsp + 0xf0]
02114d11: call   0x181371020                              ; System.Collections.Generic.Dictionary<StyleSheetCache.SheetHandleKey, object>$$GetEnumerator
02114d16: movups xmm0, xmmword ptr [rsp + 0xf0]
02114d1e: movups xmmword ptr [rsp + 0xa0], xmm0
02114d26: movups xmm1, xmmword ptr [rsp + 0x100]
02114d2e: movups xmmword ptr [rsp + 0xb0], xmm1
02114d36: movsd  xmm0, qword ptr [rsp + 0x110]
02114d3f: movsd  qword ptr [rsp + 0xc0], xmm0
02114d48: mov    qword ptr [rsp + 0x58], r15
02114d4d: lea    rax, [rsp + 0xa0]
02114d55: mov    qword ptr [rsp + 0x60], rax
02114d5a: nop    word ptr [rax + rax]
02114d60: mov    rdx, qword ptr [rip + 0x1bb5e81]         ; [0x3ccabe8] metamethod:Method$System.Collections.Generic.Dictionary.Enumerator<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.MoveNext()
02114d67: lea    rcx, [rsp + 0xa0]
02114d6f: call   0x1815172a0                              ; System.Collections.Generic.Dictionary.Enumerator<object, object>$$MoveNext
02114d74: test   al, al
02114d76: je     0x182115014
02114d7c: movups xmm0, xmmword ptr [rsp + 0xb0]
02114d84: movups xmmword ptr [rsp + 0xc8], xmm0
02114d8c: mov    r9, qword ptr [rip + 0x1bd7c0d]          ; [0x3cec9a0] metamethod:Method$System.Collections.Generic.KeyValuePair<string, List<ˁʼʹʿʴʳˀʻʾʶʷ>>.Deconstruct()
02114d93: lea    r8, [rsp + 0x38]
02114d98: lea    rdx, [rsp + 0x40]
02114d9d: lea    rcx, [rsp + 0xc8]
02114da5: call   0x181845280                              ; System.Collections.Generic.KeyValuePair<object, object>$$Deconstruct
02114daa: mov    rbx, qword ptr [rsp + 0x38]
02114daf: xor    r8d, r8d
02114db2: mov    rdx, r12
02114db5: mov    rcx, qword ptr [rsp + 0x40]
02114dba: call   0x181989ec0                              ; System.String$$op_Inequality
02114dbf: test   al, al
02114dc1: jne    0x182114d60
02114dc3: test   rbx, rbx
02114dc6: je     0x1821150c9
02114dcc: mov    r8, qword ptr [rip + 0x1bc420d]          ; [0x3cd8fe0] metamethod:Method$System.Collections.Generic.List<ˁʼʹʿʴʳˀʻʾʶʷ>.GetEnumerator()
02114dd3: mov    rdx, rbx
02114dd6: lea    rcx, [rsp + 0xf0]
02114dde: call   0x1806fe760                              ; System.Collections.Generic.List<ʼʾʾʹʾʼˀʵʵʲˀ.ʳʸʸʽˀʻʼʳʳʴˀ>$$GetEnumerator
02114de3: movups xmm0, xmmword ptr [rsp + 0xf0]
02114deb: movups xmmword ptr [rsp + 0x80], xmm0
02114df3: movups xmm1, xmmword ptr [rsp + 0x100]
02114dfb: movups xmmword ptr [rsp + 0x90], xmm1
02114e03: mov    qword ptr [rsp + 0x48], r15
02114e08: lea    rax, [rsp + 0x80]
02114e10: mov    qword ptr [rsp + 0x50], rax
02114e15: nop    word ptr [rax + rax]
02114e20: mov    rdx, qword ptr [rip + 0x1bae081]         ; [0x3cc2ea8] metamethod:Method$System.Collections.Generic.List.Enumerator<ˁʼʹʿʴʳˀʻʾʶʷ>.MoveNext()
02114e27: lea    rcx, [rsp + 0x80]
02114e2f: call   0x1814ebbf0                              ; System.Collections.Generic.List.Enumerator<wqWfZOGhxxtNsrDaalzcdlAjfYfLb.sgyglzNxTWOwXlsxalwgLLVZzVgL>$$MoveNext
02114e34: test   al, al
02114e36: je     0x182114fca
02114e3c: mov    rbx, qword ptr [rsp + 0x90]
02114e44: mov    rsi, qword ptr [rsp + 0x98]
02114e4c: mov    rax, qword ptr [rip + 0x1bf9115]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114e53: cmp    dword ptr [rax + 0xe0], 0
02114e5a: jne    0x182114e6b
02114e5c: mov    rcx, rax
02114e5f: call   0x182f60cf0
02114e64: mov    rax, qword ptr [rip + 0x1bf90fd]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02114e6b: mov    rax, qword ptr [rax + 0xb8]
02114e72: mov    rcx, qword ptr [rax]
02114e75: test   rcx, rcx
02114e78: je     0x1821150be
02114e7e: mov    qword ptr [rsp + 0xe0], rbx
02114e86: mov    qword ptr [rsp + 0xe8], rsi
02114e8e: mov    r8, qword ptr [rip + 0x1bd2fdb]          ; [0x3ce7e70] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.get_Item()
02114e95: lea    rdx, [rsp + 0xe0]
02114e9d: call   0x1812fcec0                              ; System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, object>$$get_Item
02114ea2: test   rax, rax
02114ea5: je     0x1821150b9
02114eab: mov    rdx, qword ptr [rip + 0x1bcf616]         ; [0x3ce44c8] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>.GetEnumerator()
02114eb2: mov    rcx, rax
02114eb5: call   0x1812f5a60                              ; System.Collections.Concurrent.ConcurrentBag<object>$$GetEnumerator
02114eba: mov    qword ptr [rsp + 0x188], rax
02114ec2: mov    qword ptr [rsp + 0x68], r15
02114ec7: lea    rbx, [rsp + 0x188]
02114ecf: mov    qword ptr [rsp + 0x70], rbx
02114ed4: mov    r8, qword ptr [rsp + 0x188]
02114edc: test   r8, r8
02114edf: je     0x1821150ae
02114ee5: xor    ecx, ecx
02114ee7: mov    rdx, qword ptr [rip + 0x1bea312]         ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
02114eee: call   0x182c44210
02114ef3: test   al, al
02114ef5: je     0x182114f6f
02114efb: mov    r8, qword ptr [rsp + 0x188]
02114f03: test   r8, r8
02114f06: je     0x1821150a9
02114f0c: xor    ecx, ecx
02114f0e: mov    rdx, qword ptr [rip + 0x1bc303b]         ; [0x3cd7f50] meta:System.Collections.Generic.IEnumerator<ˀʸʲˁʽʴʶʶʷʴʾ>_TypeInfo
02114f15: call   0x182c44210
02114f1a: mov    rdi, rax
02114f1d: test   rax, rax
02114f20: je     0x1821150a4
02114f26: mov    ecx, 3
02114f2b: mov    qword ptr [rsp + 0x20], rsi
02114f30: mov    r9, r14
02114f33: mov    r8, rax
02114f36: mov    rdx, qword ptr [rip + 0x1b8a803]         ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
02114f3d: call   0x182c44a20
02114f42: test   al, al
02114f44: je     0x182114ed4
02114f46: mov    byte ptr [rsp + 0x180], 1
02114f4e: mov    ecx, 2
02114f53: mov    qword ptr [rsp + 0x20], rsi
02114f58: mov    r9, r14
02114f5b: mov    r8, rdi
02114f5e: mov    rdx, qword ptr [rip + 0x1b8a7db]         ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
02114f65: call   0x182c44a20
02114f6a: jmp    0x182114ed4
02114f6f: cmp    qword ptr [rbx], 0
02114f73: je     0x182114f86
02114f75: xor    ecx, ecx
02114f77: mov    r8, qword ptr [rbx]
