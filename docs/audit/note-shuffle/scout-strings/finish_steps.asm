=== 215ED20
0215ed20: mov    qword ptr [rsp + 8], rbx
0215ed25: mov    qword ptr [rsp + 0x10], rsi
0215ed2a: push   rdi
0215ed2b: sub    rsp, 0x60
0215ed2f: mov    rdi, rcx
0215ed32: cmp    byte ptr [rip + 0x1dbaa05], 0            ; [0x3f1973e] (bss)
0215ed39: jne    0x18215ed96
0215ed3b: lea    rcx, [rip + 0x1b637a6]                   ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215ed42: call   0x182f609b0
0215ed47: lea    rcx, [rip + 0x1b6385a]                   ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215ed4e: call   0x182f609b0
0215ed53: lea    rcx, [rip + 0x1b6390e]                   ; [0x3cc2668] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Current()
0215ed5a: call   0x182f609b0
0215ed5f: lea    rcx, [rip + 0x1b788fa]                   ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215ed66: call   0x182f609b0
0215ed6b: lea    rcx, [rip + 0x1ba466e]                   ; [0x3d033e0] meta:ʷʿʽʽʵʻʶʹʼʼʺ.ʴʼʳʼˀʳˁʶʵˁʸ_TypeInfo
0215ed72: call   0x182f609b0
0215ed77: lea    rcx, [rip + 0x1b79bf2]                   ; [0x3cd8970] metamethod:Method$ʷʿʽʽʵʻʶʹʼʼʺ.<>c.ˀʸʾʹʾʺʳʾˁʸˀ()
0215ed7e: call   0x182f609b0
0215ed83: lea    rcx, [rip + 0x1ba4596]                   ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215ed8a: call   0x182f609b0
0215ed8f: mov    byte ptr [rip + 0x1dba9a8], 1            ; [0x3f1973e] (bss)
0215ed96: xorps  xmm0, xmm0
0215ed99: xor    eax, eax
0215ed9b: movups xmmword ptr [rsp + 0x40], xmm0
0215eda0: mov    qword ptr [rsp + 0x50], rax
0215eda5: mov    rcx, qword ptr [rip + 0x1ba4574]         ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215edac: cmp    dword ptr [rcx + 0xe0], eax
0215edb2: jne    0x18215edc0
0215edb4: call   0x182f60cf0
0215edb9: mov    rcx, qword ptr [rip + 0x1ba4560]         ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215edc0: mov    rax, qword ptr [rcx + 0xb8]
0215edc7: mov    rsi, qword ptr [rax + 8]
0215edcb: test   rsi, rsi
0215edce: jne    0x18215ee3e
0215edd0: cmp    dword ptr [rcx + 0xe0], esi
0215edd6: jne    0x18215ede4
0215edd8: call   0x182f60cf0
0215eddd: mov    rcx, qword ptr [rip + 0x1ba453c]         ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215ede4: mov    rax, qword ptr [rcx + 0xb8]
0215edeb: mov    rbx, qword ptr [rax]
0215edee: mov    rcx, qword ptr [rip + 0x1ba45eb]         ; [0x3d033e0] meta:ʷʿʽʽʵʻʶʹʼʼʺ.ʴʼʳʼˀʳˁʶʵˁʸ_TypeInfo
0215edf5: call   0x182f60c00
0215edfa: mov    rsi, rax
0215edfd: xor    r9d, r9d
0215ee00: mov    r8, qword ptr [rip + 0x1b79b69]          ; [0x3cd8970] metamethod:Method$ʷʿʽʽʵʻʶʹʼʼʺ.<>c.ˀʸʾʹʾʺʳʾˁʸˀ()
0215ee07: mov    rdx, rbx
0215ee0a: mov    rcx, rax
0215ee0d: call   0x1817a3c20                              ; UnityEngine.Windows.WebCam.PhotoCapture.OnCapturedToMemoryCallback$$.ctor
0215ee12: mov    rax, qword ptr [rip + 0x1ba4507]         ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215ee19: mov    rcx, qword ptr [rax + 0xb8]
0215ee20: mov    qword ptr [rcx + 8], rsi
0215ee24: mov    rax, qword ptr [rip + 0x1ba44f5]         ; [0x3d03320] meta:ʷʿʽʽʵʻʶʹʼʼʺ.<>c_TypeInfo
0215ee2b: mov    rcx, qword ptr [rax + 0xb8]
0215ee32: add    rcx, 8
0215ee36: mov    rdx, rsi
0215ee39: call   0x182f5fc00
0215ee3e: test   rdi, rdi
0215ee41: je     0x18215ef64
0215ee47: xor    r9d, r9d
0215ee4a: mov    r8, rsi
0215ee4d: mov    dl, 1
0215ee4f: mov    rcx, rdi
0215ee52: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215ee57: xor    edx, edx
0215ee59: mov    rcx, rdi
0215ee5c: call   0x18215c040                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʺʺʹʴʼʻʲʶʹʴ
0215ee61: xor    r9d, r9d
0215ee64: xor    r8d, r8d
0215ee67: mov    dl, 2
0215ee69: mov    rcx, rdi
0215ee6c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215ee71: xor    r9d, r9d
0215ee74: xor    r8d, r8d
0215ee77: mov    dl, 3
0215ee79: mov    rcx, rdi
0215ee7c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215ee81: xor    r9d, r9d
0215ee84: xor    r8d, r8d
0215ee87: mov    dl, 4
0215ee89: mov    rcx, rdi
0215ee8c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215ee91: xor    r9d, r9d
0215ee94: xor    r8d, r8d
0215ee97: mov    dl, 5
0215ee99: mov    rcx, rdi
0215ee9c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215eea1: mov    rdx, qword ptr [rdi + 0x60]
0215eea5: test   rdx, rdx
0215eea8: je     0x18215ef64
0215eeae: mov    r8, qword ptr [rip + 0x1b787ab]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215eeb5: lea    rcx, [rsp + 0x28]
0215eeba: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215eebf: movups xmm0, xmmword ptr [rsp + 0x28]
0215eec4: movups xmmword ptr [rsp + 0x40], xmm0
0215eec9: movsd  xmm1, qword ptr [rsp + 0x38]
0215eecf: movsd  qword ptr [rsp + 0x50], xmm1
0215eed5: mov    qword ptr [rsp + 0x28], 0
0215eede: lea    rbx, [rsp + 0x40]
0215eee3: mov    qword ptr [rsp + 0x30], rbx
0215eee8: nop    dword ptr [rax + rax]
0215eef0: mov    rdx, qword ptr [rip + 0x1b636b1]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215eef7: lea    rcx, [rsp + 0x40]
0215eefc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215ef01: test   al, al
0215ef03: je     0x18215ef1c
0215ef05: mov    rcx, qword ptr [rsp + 0x50]
0215ef0a: test   rcx, rcx
0215ef0d: je     0x18215ef58
0215ef0f: xor    r8d, r8d
0215ef12: mov    rdx, rdi
0215ef15: call   0x18214d980                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʾʶʴʴʺʻʼʸˁʻˁ
0215ef1a: jmp    0x18215eef0
0215ef1c: mov    rdx, qword ptr [rip + 0x1b635c5]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215ef23: mov    rcx, rbx
0215ef26: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215ef2b: jmp    0x18215ef48
0215ef2d: mov    rdx, qword ptr [rip + 0x1b635b4]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215ef34: mov    rcx, qword ptr [rsp + 0x30]
0215ef39: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215ef3e: mov    rcx, qword ptr [rsp + 0x28]
0215ef43: test   rcx, rcx
0215ef46: jne    0x18215ef5e
0215ef48: mov    rbx, qword ptr [rsp + 0x70]
0215ef4d: mov    rsi, qword ptr [rsp + 0x78]
0215ef52: add    rsp, 0x60
0215ef56: pop    rdi
0215ef57: ret    
0215ef58: call   0x182f60c50
0215ef5d: nop    
0215ef5e: call   0x182f60ce0
0215ef63: int3   
0215ef64: call   0x182f60c50
0215ef69: int3   
0215ef6a: int3   
=== 215E120
0215e120: mov    qword ptr [rsp + 8], rcx
0215e125: push   rbx
0215e126: push   rsi
0215e127: push   rdi
0215e128: push   r14
0215e12a: sub    rsp, 0x78
0215e12e: mov    rdi, rcx
0215e131: cmp    byte ptr [rip + 0x1dbb5f5], 0            ; [0x3f1972d] (bss)
0215e138: jne    0x18215e171
0215e13a: lea    rcx, [rip + 0x1b643a7]                   ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e141: call   0x182f609b0
0215e146: lea    rcx, [rip + 0x1b6445b]                   ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215e14d: call   0x182f609b0
0215e152: lea    rcx, [rip + 0x1b6450f]                   ; [0x3cc2668] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Current()
0215e159: call   0x182f609b0
0215e15e: lea    rcx, [rip + 0x1b794fb]                   ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215e165: call   0x182f609b0
0215e16a: mov    byte ptr [rip + 0x1dbb5bc], 1            ; [0x3f1972d] (bss)
0215e171: xorps  xmm0, xmm0
0215e174: xor    eax, eax
0215e176: movups xmmword ptr [rsp + 0x50], xmm0
0215e17b: mov    qword ptr [rsp + 0x60], rax
0215e180: mov    rcx, qword ptr [rdi + 0x50]
0215e184: test   rcx, rcx
0215e187: je     0x18215e267
0215e18d: xor    edx, edx
0215e18f: call   0x18212c7c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʽʴʷʿˁˁʸʷʳʴʿ
0215e194: mov    esi, eax
0215e196: test   eax, eax
0215e198: jle    0x18215e25d
0215e19e: mov    rdx, qword ptr [rdi + 0x60]
0215e1a2: test   rdx, rdx
0215e1a5: je     0x18215e267
0215e1ab: mov    r8, qword ptr [rip + 0x1b794ae]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215e1b2: lea    rcx, [rsp + 0x38]
0215e1b7: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215e1bc: movups xmm0, xmmword ptr [rsp + 0x38]
0215e1c1: movups xmmword ptr [rsp + 0x50], xmm0
0215e1c6: movsd  xmm1, qword ptr [rsp + 0x48]
0215e1cc: movsd  qword ptr [rsp + 0x60], xmm1
0215e1d2: xor    r14d, r14d
0215e1d5: mov    qword ptr [rsp + 0x38], r14
0215e1da: lea    rbx, [rsp + 0x50]
0215e1df: mov    qword ptr [rsp + 0x40], rbx
0215e1e4: mov    rdx, qword ptr [rip + 0x1b643bd]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215e1eb: lea    rcx, [rsp + 0x50]
0215e1f0: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215e1f5: test   al, al
0215e1f7: je     0x18215e213
0215e1f9: mov    qword ptr [rsp + 0x20], r14
0215e1fe: xor    r9d, r9d
0215e201: mov    r8d, esi
0215e204: mov    rdx, qword ptr [rsp + 0x60]
0215e209: mov    rcx, rdi
0215e20c: call   0x18215c560                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʻʻʾʽʲˁʼʶʽʷ
0215e211: jmp    0x18215e1e4
0215e213: mov    rdx, qword ptr [rip + 0x1b642ce]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e21a: mov    rcx, rbx
0215e21d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215e222: jmp    0x18215e247
0215e224: mov    rdx, qword ptr [rip + 0x1b642bd]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e22b: mov    rcx, qword ptr [rsp + 0x40]
0215e230: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215e235: mov    rcx, qword ptr [rsp + 0x38]
0215e23a: test   rcx, rcx
0215e23d: jne    0x18215e26d
0215e23f: mov    rdi, qword ptr [rsp + 0xa0]
0215e247: cmp    byte ptr [rdi + 0x58], 6
0215e24b: je     0x18215e25d
0215e24d: cmp    byte ptr [rdi + 0x58], 9
0215e251: je     0x18215e25d
0215e253: xor    edx, edx
0215e255: mov    rcx, rdi
0215e258: call   0x18215fb50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʾʺˁʿʷʽʴʽʹʶʾ
0215e25d: add    rsp, 0x78
0215e261: pop    r14
0215e263: pop    rdi
0215e264: pop    rsi
0215e265: pop    rbx
0215e266: ret    
0215e267: call   0x182f60c50
0215e26c: int3   
0215e26d: call   0x182f60ce0
0215e272: int3   
0215e273: int3   
=== 215C040
0215c040: push   rbx
0215c042: push   rbp
0215c043: push   rsi
0215c044: push   rdi
0215c045: push   r13
0215c047: push   r14
0215c049: sub    rsp, 0x48
0215c04d: cmp    byte ptr [rip + 0x1dbd6db], 0            ; [0x3f1972f] (bss)
0215c054: mov    r13, rcx
0215c057: jne    0x18215c0a8
0215c059: lea    rcx, [rip + 0x1b7b6c0]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215c060: call   0x182f609b0
0215c065: lea    rcx, [rip + 0x1b7b774]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c06c: call   0x182f609b0
0215c071: lea    rcx, [rip + 0x1b4f810]                   ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c078: call   0x182f609b0
0215c07d: lea    rcx, [rip + 0x1b4f74c]                   ; [0x3cab7d0] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c084: call   0x182f609b0
0215c089: lea    rcx, [rip + 0x1b4f8b0]                   ; [0x3cab940] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Length()
0215c090: call   0x182f609b0
0215c095: lea    rcx, [rip + 0x1b95ad4]                   ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215c09c: call   0x182f609b0
0215c0a1: mov    byte ptr [rip + 0x1dbd687], 1            ; [0x3f1972f] (bss)
0215c0a8: mov    rbx, qword ptr [rip + 0x1b95ac1]         ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215c0af: mov    rdi, qword ptr [r13 + 0x60]
0215c0b3: cmp    qword ptr [rbx + 0x38], 0
0215c0b8: jne    0x18215c0c2
0215c0ba: mov    rcx, rbx
0215c0bd: call   0x182f657d0
0215c0c2: mov    r8, qword ptr [rbx + 0x38]
0215c0c6: lea    rcx, [rsp + 0x20]
0215c0cb: mov    rdx, rdi
0215c0ce: mov    qword ptr [rsp + 0x80], r15
0215c0d6: mov    r8, qword ptr [r8 + 8]
0215c0da: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0215c0df: mov    rax, qword ptr [r13 + 0x60]
0215c0e3: xor    ecx, ecx
0215c0e5: mov    ebp, dword ptr [rsp + 0x28]
0215c0e9: xor    esi, esi
0215c0eb: xor    r14d, r14d
0215c0ee: test   rax, rax
0215c0f1: je     0x18215c30e
0215c0f7: mov    rbx, qword ptr [rsp + 0x20]
0215c0fc: nop    dword ptr [rax]
0215c100: cmp    ecx, dword ptr [rax + 0x18]
0215c103: jge    0x18215c1e0
0215c109: mov    rcx, qword ptr [r13 + 0x60]
0215c10d: test   rcx, rcx
0215c110: je     0x18215c30e
0215c116: mov    r8, qword ptr [rip + 0x1b7b6c3]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c11d: mov    edx, r14d
0215c120: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c125: mov    rcx, qword ptr [r13 + 0x60]
0215c129: mov    rdi, rax
0215c12c: test   rcx, rcx
0215c12f: je     0x18215c30e
0215c135: mov    r8, qword ptr [rip + 0x1b7b6a4]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c13c: mov    edx, esi
0215c13e: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c143: test   rax, rax
0215c146: je     0x18215c30e
0215c14c: test   rdi, rdi
0215c14f: je     0x18215c30e
0215c155: mov    rcx, qword ptr [rdi + 0x10]
0215c159: cmp    qword ptr [rax + 0x10], rcx
0215c15d: je     0x18215c1c8
0215c15f: mov    r15, qword ptr [rip + 0x1b4f722]         ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c166: mov    edi, r14d
0215c169: sub    edi, esi
0215c16b: cmp    esi, ebp
0215c16d: ja     0x18215c177
0215c16f: mov    eax, ebp
0215c171: sub    eax, esi
0215c173: cmp    edi, eax
0215c175: jbe    0x18215c17e
0215c177: xor    ecx, ecx
0215c179: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c17e: mov    rcx, qword ptr [r15 + 0x20]
0215c182: xorps  xmm0, xmm0
0215c185: movups xmmword ptr [rsp + 0x30], xmm0
0215c18a: test   byte ptr [rcx + 0x135], 1
0215c191: jne    0x18215c198
0215c193: call   0x182f65750
0215c198: cmp    edi, 1
0215c19b: jle    0x18215c1c5
0215c19d: movsxd rax, esi
0215c1a0: lea    rdx, [rsp + 0x20]
0215c1a5: xor    r8d, r8d
0215c1a8: mov    dword ptr [rsp + 0x28], edi
0215c1ac: lea    rcx, [rbx + rax*8]
0215c1b0: mov    eax, dword ptr [rsp + 0x3c]
0215c1b4: mov    qword ptr [rsp + 0x20], rcx
0215c1b9: mov    rcx, r13
0215c1bc: mov    dword ptr [rsp + 0x2c], eax
0215c1c0: call   0x18215e020                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʷʻʻʷʲʺˀʷˁʴʹ
0215c1c5: mov    esi, r14d
0215c1c8: mov    rax, qword ptr [r13 + 0x60]
0215c1cc: inc    r14d
0215c1cf: mov    ecx, r14d
0215c1d2: test   rax, rax
0215c1d5: je     0x18215c30e
0215c1db: jmp    0x18215c100
0215c1e0: mov    rdi, qword ptr [rip + 0x1b4f5e9]         ; [0x3cab7d0] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c1e7: cmp    esi, ebp
0215c1e9: jbe    0x18215c1f2
0215c1eb: xor    ecx, ecx
0215c1ed: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c1f2: mov    rcx, qword ptr [rdi + 0x20]
0215c1f6: test   byte ptr [rcx + 0x135], 1
0215c1fd: jne    0x18215c204
0215c1ff: call   0x182f65750
0215c204: sub    ebp, esi
0215c206: cmp    ebp, 1
0215c209: jle    0x18215c2ed
0215c20f: cmp    byte ptr [rip + 0x1dbd52a], 0            ; [0x3f19740] (bss)
0215c216: movsxd rax, esi
0215c219: lea    r15, [rbx + rax*8]
0215c21d: jne    0x18215c232
0215c21f: lea    rcx, [rip + 0x1b4f71a]                   ; [0x3cab940] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Length()
0215c226: call   0x182f609b0
0215c22b: mov    byte ptr [rip + 0x1dbd50e], 1            ; [0x3f19740] (bss)
0215c232: mov    r14d, ebp
0215c235: xor    r10b, r10b
0215c238: mov    ebx, 1
0215c23d: cmp    r14d, ebx
0215c240: jle    0x18215c2ed
0215c246: nop    word ptr [rax + rax]
0215c250: lea    eax, [rbx - 1]
0215c253: cmp    eax, ebp
0215c255: jae    0x18215c314
0215c25b: cmp    ebx, ebp
0215c25d: jae    0x18215c314
0215c263: mov    eax, ebx
0215c265: mov    rcx, qword ptr [r15 + rax*8 - 8]
0215c26a: lea    r9, [r15 + rax*8]
0215c26e: test   rcx, rcx
0215c271: je     0x18215c30e
0215c277: mov    eax, ebx
0215c279: lea    rsi, [r15 + rax*8]
0215c27d: mov    rax, qword ptr [r15 + rax*8]
0215c281: test   rax, rax
0215c284: je     0x18215c30e
0215c28a: mov    eax, dword ptr [rax + 0x20]
0215c28d: cmp    dword ptr [rcx + 0x20], eax
0215c290: jne    0x18215c2a0
0215c292: mov    rax, qword ptr [rsi]
0215c295: mov    rdx, rcx
0215c298: mov    ecx, dword ptr [rax + 0x24]
0215c29b: cmp    dword ptr [rdx + 0x24], ecx
0215c29e: jg     0x18215c2b0
0215c2a0: mov    rax, qword ptr [rsi]
0215c2a3: mov    r8, qword ptr [r9 - 8]
0215c2a7: mov    edx, dword ptr [rax + 0x20]
0215c2aa: cmp    dword ptr [r8 + 0x20], edx
0215c2ae: jle    0x18215c2d5
0215c2b0: mov    rdx, qword ptr [rsi]
0215c2b3: lea    rcx, [r9 - 8]
0215c2b7: mov    rdi, qword ptr [r9 - 8]
0215c2bb: mov    qword ptr [r9 - 8], rdx
0215c2bf: call   0x182f5fc00
0215c2c4: mov    rdx, rdi
0215c2c7: mov    qword ptr [rsi], rdi
0215c2ca: mov    rcx, rsi
0215c2cd: call   0x182f5fc00
0215c2d2: mov    r10b, 1
0215c2d5: inc    ebx
0215c2d7: cmp    ebx, r14d
0215c2da: jl     0x18215c250
0215c2e0: test   r10b, r10b
0215c2e3: je     0x18215c2ed
0215c2e5: dec    r14d
0215c2e8: jmp    0x18215c235
0215c2ed: xor    r8d, r8d
0215c2f0: xor    edx, edx
0215c2f2: mov    rcx, r13
0215c2f5: mov    r15, qword ptr [rsp + 0x80]
0215c2fd: add    rsp, 0x48
0215c301: pop    r14
0215c303: pop    r13
0215c305: pop    rdi
0215c306: pop    rsi
0215c307: pop    rbp
0215c308: pop    rbx
0215c309: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
0215c30e: call   0x182f60c50
0215c313: int3   
0215c314: call   0x182f60c40
0215c319: int3   
0215c31a: int3   
=== 215B8C0
0215b8c0: push   rbp
0215b8c2: push   r13
0215b8c4: sub    rsp, 0x48
0215b8c8: cmp    byte ptr [rip + 0x1dbde74], 0            ; [0x3f19743] (bss)
0215b8cf: mov    rbp, rcx
0215b8d2: jne    0x18215b90b
0215b8d4: lea    rcx, [rip + 0x1b76d65]                   ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215b8db: call   0x182f609b0
0215b8e0: lea    rcx, [rip + 0x1b7be39]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215b8e7: call   0x182f609b0
0215b8ec: lea    rcx, [rip + 0x1b7beed]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b8f3: call   0x182f609b0
0215b8f8: lea    rcx, [rip + 0x1b3a9d1]                   ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0215b8ff: call   0x182f609b0
0215b904: mov    byte ptr [rip + 0x1dbde38], 1            ; [0x3f19743] (bss)
0215b90b: mov    rax, qword ptr [rbp + 0x60]
0215b90f: xor    r13d, r13d
0215b912: mov    qword ptr [rsp + 0x68], rbx
0215b917: xor    ecx, ecx
0215b919: mov    qword ptr [rsp + 0x40], rsi
0215b91e: mov    qword ptr [rsp + 0x38], rdi
0215b923: mov    qword ptr [rsp + 0x30], r12
0215b928: mov    qword ptr [rsp + 0x28], r14
0215b92d: mov    qword ptr [rsp + 0x20], r15
0215b932: mov    dword ptr [rsp + 0x60], r13d
0215b937: test   rax, rax
0215b93a: je     0x18215bb0e
0215b940: cmp    ecx, dword ptr [rax + 0x18]
0215b943: jge    0x18215bae8
0215b949: mov    rcx, qword ptr [rbp + 0x60]
0215b94d: test   rcx, rcx
0215b950: je     0x18215bb0e
0215b956: mov    r8, qword ptr [rip + 0x1b7be83]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b95d: mov    edx, r13d
0215b960: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b965: test   rax, rax
0215b968: je     0x18215bacf
0215b96e: mov    edi, dword ptr [rax + 0x20]
0215b971: lea    ebx, [r13 + 1]
0215b975: mov    r12, qword ptr [rax + 0x10]
0215b979: mov    r14d, r13d
0215b97c: mov    dword ptr [rsp + 0x70], edi
0215b980: mov    rax, qword ptr [rbp + 0x60]
0215b984: test   rax, rax
0215b987: je     0x18215bb0e
0215b98d: cmp    ebx, dword ptr [rax + 0x18]
0215b990: jge    0x18215b9ee
0215b992: mov    r8, qword ptr [rip + 0x1b7be47]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b999: mov    edx, ebx
0215b99b: mov    rcx, rax
0215b99e: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b9a3: test   rax, rax
0215b9a6: je     0x18215b9ea
0215b9a8: cmp    qword ptr [rax + 0x10], r12
0215b9ac: jg     0x18215b9ee
0215b9ae: jne    0x18215b9ea
0215b9b0: cmp    dword ptr [rax + 0x20], edi
0215b9b3: jne    0x18215b9ea
0215b9b5: mov    rcx, qword ptr [rbp + 0x60]
0215b9b9: test   rcx, rcx
0215b9bc: je     0x18215bb0e
0215b9c2: mov    r8, qword ptr [rip + 0x1b7be17]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b9c9: mov    edx, r14d
0215b9cc: mov    rdi, qword ptr [rax + 0x18]
0215b9d0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b9d5: test   rax, rax
0215b9d8: je     0x18215bb0e
0215b9de: cmp    rdi, qword ptr [rax + 0x18]
0215b9e2: mov    edi, dword ptr [rsp + 0x70]
0215b9e6: cmovg  r14d, ebx
0215b9ea: inc    ebx
0215b9ec: jmp    0x18215b980
0215b9ee: mov    rcx, qword ptr [rbp + 0x60]
0215b9f2: test   rcx, rcx
0215b9f5: je     0x18215bb0e
0215b9fb: mov    r8, qword ptr [rip + 0x1b7bdde]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215ba02: mov    edx, r14d
0215ba05: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215ba0a: mov    r15, rax
0215ba0d: mov    esi, r13d
0215ba10: mov    rcx, qword ptr [rbp + 0x60]
0215ba14: test   rcx, rcx
0215ba17: je     0x18215bb0e
0215ba1d: cmp    esi, dword ptr [rcx + 0x18]
0215ba20: jge    0x18215baca
0215ba26: cmp    esi, r14d
0215ba29: je     0x18215bac3
0215ba2f: mov    r8, qword ptr [rip + 0x1b7bdaa]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215ba36: mov    edx, esi
0215ba38: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215ba3d: mov    rbx, rax
0215ba40: test   rax, rax
0215ba43: je     0x18215bac3
0215ba45: cmp    qword ptr [rax + 0x10], r12
0215ba49: jg     0x18215baca
0215ba4b: jne    0x18215bac3
0215ba4d: cmp    dword ptr [rax + 0x20], edi
0215ba50: jne    0x18215bac3
0215ba52: test   r15, r15
0215ba55: je     0x18215bb0e
0215ba5b: mov    rcx, qword ptr [rip + 0x1b3a86e]         ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0215ba62: mov    edi, dword ptr [r15 + 0x24]
0215ba66: mov    r13d, dword ptr [rax + 0x24]
0215ba6a: cmp    dword ptr [rcx + 0xe0], 0
0215ba71: jne    0x18215ba78
0215ba73: call   0x182f60cf0
0215ba78: mov    eax, edi
0215ba7a: or     edi, r13d
0215ba7d: or     eax, r13d
0215ba80: and    eax, 0xfffffff8
0215ba83: test   dil, 4
0215ba87: jne    0x18215ba9f
0215ba89: test   dil, 2
0215ba8d: jne    0x18215ba9a
0215ba8f: test   dil, 1
0215ba93: je     0x18215baa2
0215ba95: or     eax, 1
0215ba98: jmp    0x18215baa2
0215ba9a: or     eax, 2
0215ba9d: jmp    0x18215baa2
0215ba9f: or     eax, 4
0215baa2: mov    rcx, qword ptr [r15 + 0x38]
0215baa6: mov    dword ptr [r15 + 0x24], eax
0215baaa: test   rcx, rcx
0215baad: je     0x18215bb0e
0215baaf: mov    r8, qword ptr [rip + 0x1b76b8a]          ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215bab6: mov    rdx, qword ptr [rbx + 0x38]
0215baba: call   0x1807b4980                              ; System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>$$AddRange
0215babf: mov    edi, dword ptr [rsp + 0x70]
0215bac3: inc    esi
0215bac5: jmp    0x18215ba10
0215baca: mov    r13d, dword ptr [rsp + 0x60]
0215bacf: mov    rax, qword ptr [rbp + 0x60]
0215bad3: inc    r13d
0215bad6: mov    dword ptr [rsp + 0x60], r13d
0215badb: mov    ecx, r13d
0215bade: test   rax, rax
0215bae1: je     0x18215bb0e
0215bae3: jmp    0x18215b940
0215bae8: mov    r15, qword ptr [rsp + 0x20]
0215baed: mov    r14, qword ptr [rsp + 0x28]
0215baf2: mov    r12, qword ptr [rsp + 0x30]
0215baf7: mov    rdi, qword ptr [rsp + 0x38]
0215bafc: mov    rsi, qword ptr [rsp + 0x40]
0215bb01: mov    rbx, qword ptr [rsp + 0x68]
0215bb06: add    rsp, 0x48
0215bb0a: pop    r13
0215bb0c: pop    rbp
0215bb0d: ret    
0215bb0e: call   0x182f60c50
0215bb13: int3   
0215bb14: int3   
