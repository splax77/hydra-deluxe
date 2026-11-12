002f57c0: mov    qword ptr [rsp + 8], rbx
002f57c5: mov    qword ptr [rsp + 0x10], rsi
002f57ca: push   rdi
002f57cb: sub    rsp, 0x70
002f57cf: mov    rsi, rcx
002f57d2: cmp    byte ptr [rip + 0x3c18230], 0            ; [0x3f0da09] (bss)
002f57d9: jne    0x1802f5842
002f57db: lea    rcx, [rip + 0x39c8286]                   ; [0x3cbda68] meta:System.IO.BinaryWriter_TypeInfo
002f57e2: call   0x182f609b0
002f57e7: lea    rcx, [rip + 0x39cc63a]                   ; [0x3cc1e28] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.Dispose()
002f57ee: call   0x182f609b0
002f57f3: lea    rcx, [rip + 0x39cc6ee]                   ; [0x3cc1ee8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.MoveNext()
002f57fa: call   0x182f609b0
002f57ff: lea    rcx, [rip + 0x39cc7a2]                   ; [0x3cc1fa8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.get_Current()
002f5806: call   0x182f609b0
002f580b: lea    rcx, [rip + 0x3a084be]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002f5812: call   0x182f609b0
002f5817: lea    rcx, [rip + 0x39e0e82]                   ; [0x3cd66a0] metamethod:Method$System.Collections.Generic.List<ʽʵʳʲʺˀˀʷʵʸˀ>.GetEnumerator()
002f581e: call   0x182f609b0
002f5823: lea    rcx, [rip + 0x39e0f36]                   ; [0x3cd6760] metamethod:Method$System.Collections.Generic.List<ʽʵʳʲʺˀˀʷʵʸˀ>.get_Count()
002f582a: call   0x182f609b0
002f582f: lea    rcx, [rip + 0x39a850a]                   ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f5836: call   0x182f609b0
002f583b: mov    byte ptr [rip + 0x3c181c7], 1            ; [0x3f0da09] (bss)
002f5842: mov    qword ptr [rsp + 0x90], 0
002f584e: xorps  xmm0, xmm0
002f5851: xor    eax, eax
002f5853: movups xmmword ptr [rsp + 0x58], xmm0
002f5858: mov    qword ptr [rsp + 0x68], rax
002f585d: mov    rcx, qword ptr [rip + 0x39a84dc]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f5864: cmp    dword ptr [rcx + 0xe0], eax
002f586a: jne    0x1802f5878
002f586c: call   0x182f60cf0
002f5871: mov    rcx, qword ptr [rip + 0x39a84c8]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f5878: mov    rcx, qword ptr [rcx + 0xb8]
002f587f: xor    edx, edx
002f5881: mov    rcx, qword ptr [rcx]
002f5884: call   0x181b1b6e0                              ; System.IO.File$$OpenWrite
002f5889: mov    rbx, rax
002f588c: mov    rcx, qword ptr [rip + 0x39c81d5]         ; [0x3cbda68] meta:System.IO.BinaryWriter_TypeInfo
002f5893: call   0x182f60c00
002f5898: mov    rdi, rax
002f589b: xor    r8d, r8d
002f589e: mov    rdx, rbx
002f58a1: mov    rcx, rax
002f58a4: call   0x181b10660                              ; System.IO.BinaryWriter$$.ctor
002f58a9: mov    qword ptr [rsp + 0x90], rdi
002f58b1: mov    qword ptr [rsp + 0x48], 0
002f58ba: lea    rdi, [rsp + 0x90]
002f58c2: mov    qword ptr [rsp + 0x50], rdi
002f58c7: mov    rcx, qword ptr [rsp + 0x90]
002f58cf: test   rcx, rcx
002f58d2: je     0x1802f5a2d
002f58d8: mov    rax, qword ptr [rcx]
002f58db: mov    r8, qword ptr [rax + 0x2b0]
002f58e2: mov    edx, 0x1346541
002f58e7: call   qword ptr [rax + 0x2a8]
002f58ed: test   rsi, rsi
002f58f0: je     0x1802f5a28
002f58f6: mov    rcx, qword ptr [rsp + 0x90]
002f58fe: test   rcx, rcx
002f5901: je     0x1802f5a23
002f5907: mov    rax, qword ptr [rcx]
002f590a: mov    r8, qword ptr [rax + 0x2b0]
002f5911: mov    edx, dword ptr [rsi + 0x18]
002f5914: call   qword ptr [rax + 0x2a8]
002f591a: mov    r8, qword ptr [rip + 0x39e0d7f]          ; [0x3cd66a0] metamethod:Method$System.Collections.Generic.List<ʽʵʳʲʺˀˀʷʵʸˀ>.GetEnumerator()
002f5921: mov    rdx, rsi
002f5924: lea    rcx, [rsp + 0x30]
002f5929: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
002f592e: movups xmm0, xmmword ptr [rsp + 0x30]
002f5933: movups xmmword ptr [rsp + 0x58], xmm0
002f5938: movsd  xmm1, qword ptr [rsp + 0x40]
002f593e: movsd  qword ptr [rsp + 0x68], xmm1
002f5944: mov    qword ptr [rsp + 0x30], 0
002f594d: lea    rbx, [rsp + 0x58]
002f5952: mov    qword ptr [rsp + 0x38], rbx
002f5957: nop    word ptr [rax + rax]
002f5960: mov    rdx, qword ptr [rip + 0x39cc581]         ; [0x3cc1ee8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.MoveNext()
002f5967: lea    rcx, [rsp + 0x58]
002f596c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
002f5971: test   al, al
002f5973: je     0x1802f5995
002f5975: mov    rcx, qword ptr [rsp + 0x68]
002f597a: test   rcx, rcx
002f597d: je     0x1802f5a11
002f5983: xor    r8d, r8d
002f5986: mov    rdx, qword ptr [rsp + 0x90]
002f598e: call   0x1802167b0                              ; ʽʵʳʲʺˀˀʷʵʸˀ$$ʾʿʳʽʳʴʴˁʽʸʽ
002f5993: jmp    0x1802f5960
002f5995: mov    rdx, qword ptr [rip + 0x39cc48c]         ; [0x3cc1e28] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.Dispose()
002f599c: mov    rcx, rbx
002f599f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002f59a4: nop    
002f59a5: cmp    qword ptr [rdi], 0
002f59a9: je     0x1802f59bc
002f59ab: xor    ecx, ecx
002f59ad: mov    r8, qword ptr [rdi]
002f59b0: mov    rdx, qword ptr [rip + 0x3a08319]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002f59b7: call   0x182c42550
002f59bc: jmp    0x1802f59ff
002f59be: mov    rdx, qword ptr [rip + 0x39cc463]         ; [0x3cc1e28] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʵʳʲʺˀˀʷʵʸˀ>.Dispose()
002f59c5: mov    rcx, qword ptr [rsp + 0x38]
002f59ca: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002f59cf: mov    rcx, qword ptr [rsp + 0x30]
002f59d4: test   rcx, rcx
002f59d7: jne    0x1802f5a17
002f59d9: mov    r8, qword ptr [rsp + 0x50]
002f59de: cmp    qword ptr [r8], 0
002f59e2: je     0x1802f59f5
002f59e4: xor    ecx, ecx
002f59e6: mov    r8, qword ptr [r8]
002f59e9: mov    rdx, qword ptr [rip + 0x3a082e0]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002f59f0: call   0x182c42550
002f59f5: mov    rcx, qword ptr [rsp + 0x48]
002f59fa: test   rcx, rcx
002f59fd: jne    0x1802f5a1d
002f59ff: lea    r11, [rsp + 0x70]
002f5a04: mov    rbx, qword ptr [r11 + 0x10]
002f5a08: mov    rsi, qword ptr [r11 + 0x18]
002f5a0c: mov    rsp, r11
002f5a0f: pop    rdi
002f5a10: ret    
002f5a11: call   0x182f60c50
002f5a16: nop    
002f5a17: call   0x182f60ce0
002f5a1c: nop    
002f5a1d: call   0x182f60ce0
002f5a22: nop    
002f5a23: call   0x182f60c50
002f5a28: call   0x182f60c50
002f5a2d: call   0x182f60c50
002f5a32: int3   
002f5a33: int3   
