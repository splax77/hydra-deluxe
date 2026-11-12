02110920: mov    qword ptr [rsp + 8], rcx
02110925: push   rbx
02110926: push   rsi
02110927: push   rdi
02110928: push   r12
0211092a: push   r13
0211092c: push   r14
0211092e: push   r15
02110930: sub    rsp, 0x70
02110934: mov    rdi, rcx
02110937: cmp    byte ptr [rip + 0x1e08b27], 0            ; [0x3f19465] (bss)
0211093e: jne    0x1821109bf
02110940: lea    rcx, [rip + 0x1b983f9]                   ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110947: call   0x182f609b0
0211094c: lea    rcx, [rip + 0x1b998c5]                   ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110953: call   0x182f609b0
02110958: lea    rcx, [rip + 0x1baea11]                   ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0211095f: call   0x182f609b0
02110964: lea    rcx, [rip + 0x1baeac5]                   ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0211096b: call   0x182f609b0
02110970: lea    rcx, [rip + 0x1baeb79]                   ; [0x3cbf4f0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Current()
02110977: call   0x182f609b0
0211097c: lea    rcx, [rip + 0x1bae29d]                   ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
02110983: call   0x182f609b0
02110988: lea    rcx, [rip + 0x1bbd639]                   ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0211098f: call   0x182f609b0
02110994: lea    rcx, [rip + 0x1bbd6ed]                   ; [0x3cce088] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Count()
0211099b: call   0x182f609b0
021109a0: lea    rcx, [rip + 0x1bcad81]                   ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
021109a7: call   0x182f609b0
021109ac: lea    rcx, [rip + 0x1bf6375]                   ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021109b3: call   0x182f609b0
021109b8: mov    byte ptr [rip + 0x1e08aa6], 1            ; [0x3f19465] (bss)
021109bf: xorps  xmm0, xmm0
021109c2: xor    eax, eax
021109c4: movups xmmword ptr [rsp + 0x50], xmm0
021109c9: mov    qword ptr [rsp + 0x60], rax
021109ce: test   rdi, rdi
021109d1: je     0x182110d34
021109d7: mov    r14, qword ptr [rdi + 0x50]
021109db: mov    rcx, qword ptr [rip + 0x1bf6346]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021109e2: cmp    dword ptr [rcx + 0xe0], eax
021109e8: jne    0x1821109f6
021109ea: call   0x182f60cf0
021109ef: mov    rcx, qword ptr [rip + 0x1bf6332]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021109f6: mov    rax, qword ptr [rcx + 0xb8]
021109fd: mov    rsi, qword ptr [rax + 8]
02110a01: test   rsi, rsi
02110a04: jne    0x182110a74
02110a06: cmp    dword ptr [rcx + 0xe0], esi
02110a0c: jne    0x182110a1a
02110a0e: call   0x182f60cf0
02110a13: mov    rcx, qword ptr [rip + 0x1bf630e]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110a1a: mov    rax, qword ptr [rcx + 0xb8]
02110a21: mov    rbx, qword ptr [rax]
02110a24: mov    rcx, qword ptr [rip + 0x1bae1f5]         ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
02110a2b: call   0x182f60c00
02110a30: mov    rsi, rax
02110a33: xor    r9d, r9d
02110a36: mov    r8, qword ptr [rip + 0x1bcaceb]          ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
02110a3d: mov    rdx, rbx
02110a40: mov    rcx, rax
02110a43: call   0x18170ede0                              ; System.Data.Listeners.Func<object, object, bool>$$.ctor
02110a48: mov    rax, qword ptr [rip + 0x1bf62d9]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110a4f: mov    r8, qword ptr [rax + 0xb8]
02110a56: mov    qword ptr [r8 + 8], rsi
02110a5a: mov    rax, qword ptr [rip + 0x1bf62c7]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110a61: mov    rcx, qword ptr [rax + 0xb8]
02110a68: add    rcx, 8
02110a6c: mov    rdx, rsi
02110a6f: call   0x182f5fc00
02110a74: mov    r8, qword ptr [rip + 0x1b9979d]          ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110a7b: mov    rdx, rsi
02110a7e: mov    rcx, r14
02110a81: call   0x18048ee00                              ; System.Linq.Enumerable$$Where<object>
02110a86: mov    rdx, qword ptr [rip + 0x1b982b3]         ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110a8d: mov    rcx, rax
02110a90: call   0x18048e370                              ; System.Linq.Enumerable$$ToList<object>
02110a95: mov    r13, rax
02110a98: mov    qword ptr [rsp + 0x20], rax
02110a9d: mov    r14d, 1
02110aa3: mov    qword ptr [rdi + 0x10], r14
02110aa7: xor    r12d, r12d
02110aaa: mov    qword ptr [rdi + 0x18], r12
02110aae: mov    word ptr [rdi + 0x24], r14w
02110ab3: test   rax, rax
02110ab6: je     0x182110d34
02110abc: mov    r15d, r12d
02110abf: mov    dword ptr [rsp + 0xc0], r12d
02110ac7: mov    dword ptr [rsp + 0xc8], r14d
02110acf: mov    r8, qword ptr [rip + 0x1bbd4f2]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
02110ad6: mov    rdx, rax
02110ad9: lea    rcx, [rsp + 0x38]
02110ade: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02110ae3: movups xmm0, xmmword ptr [rsp + 0x38]
02110ae8: movups xmmword ptr [rsp + 0x50], xmm0
02110aed: movsd  xmm1, qword ptr [rsp + 0x48]
02110af3: movsd  qword ptr [rsp + 0x60], xmm1
02110af9: mov    qword ptr [rsp + 0x38], r12
02110afe: lea    rbx, [rsp + 0x50]
02110b03: mov    qword ptr [rsp + 0x40], rbx
02110b08: nop    dword ptr [rax + rax]
02110b10: mov    rdx, qword ptr [rip + 0x1bae919]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
02110b17: lea    rcx, [rsp + 0x50]
02110b1c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02110b21: test   al, al
02110b23: je     0x182110bc6
02110b29: mov    rsi, qword ptr [rsp + 0x60]
02110b2e: test   rsi, rsi
02110b31: je     0x182110d1c
02110b37: mov    eax, dword ptr [rsi + 0x14]
02110b3a: add    dword ptr [rdi + 0x18], eax
02110b3d: mov    eax, dword ptr [rsi + 0x10]
02110b40: add    dword ptr [rdi + 0x10], eax
02110b43: mov    eax, dword ptr [rsi + 0x18]
02110b46: add    dword ptr [rdi + 0x14], eax
02110b49: mov    eax, dword ptr [rsi + 0x1c]
02110b4c: add    dword ptr [rdi + 0x1c], eax
02110b4f: xor    edx, edx
02110b51: mov    rcx, rsi
02110b54: call   0x18212fa10                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʴʲʵʸʷʼʼˁʺʾ
02110b59: movzx  eax, al
02110b5c: add    r15d, eax
02110b5f: mov    dword ptr [rsp + 0xc0], r15d
02110b67: xor    edx, edx
02110b69: mov    rcx, rsi
02110b6c: call   0x18212f510                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʵʴʽʴˁʽʲʸʷʺ
02110b71: test   al, al
02110b73: je     0x182110b7a
02110b75: inc    dword ptr [rdi + 0x28]
02110b78: jmp    0x182110b80
02110b7a: mov    word ptr [rdi + 0x24], 1
02110b80: xor    edx, edx
02110b82: mov    rcx, rsi
02110b85: call   0x18212f4f0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʴʻʽʵʳˀʿʵʻʲ
02110b8a: test   al, al
02110b8c: je     0x182110b96
02110b8e: mov    eax, dword ptr [rdi + 0x2c]
02110b91: mov    dword ptr [rdi + 0x2c], eax
02110b94: jmp    0x182110b9a
02110b96: mov    byte ptr [rdi + 0x25], 1
02110b9a: xor    edx, edx
02110b9c: mov    rcx, rsi
02110b9f: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
02110ba4: cmp    r14d, eax
02110ba7: jge    0x182110b10
02110bad: xor    edx, edx
02110baf: mov    rcx, rsi
02110bb2: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
02110bb7: mov    r14d, eax
02110bba: mov    dword ptr [rsp + 0xc8], eax
02110bc1: jmp    0x182110b10
02110bc6: mov    rdx, qword ptr [rip + 0x1bae7a3]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
02110bcd: mov    rcx, rbx
02110bd0: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02110bd5: jmp    0x182110c16
02110bd7: mov    rdx, qword ptr [rip + 0x1bae792]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
02110bde: mov    rcx, qword ptr [rsp + 0x40]
02110be3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02110be8: mov    rcx, qword ptr [rsp + 0x38]
02110bed: test   rcx, rcx
02110bf0: jne    0x182110d22
02110bf6: xor    r12d, r12d
02110bf9: mov    rdi, qword ptr [rsp + 0xb0]
02110c01: mov    r15d, dword ptr [rsp + 0xc0]
02110c09: mov    r14d, dword ptr [rsp + 0xc8]
02110c11: mov    r13, qword ptr [rsp + 0x20]
02110c16: mov    eax, r15d
02110c19: cdq    
02110c1a: idiv   dword ptr [r13 + 0x18]
02110c1e: mov    byte ptr [rdi + 0x4c], al
02110c21: xor    r8d, r8d
02110c24: mov    edx, dword ptr [rdi + 0x10]
02110c27: mov    ecx, dword ptr [rdi + 0x18]
02110c2a: call   0x18210dd00                              ; ʿˁʺˀʷʼʽʼʶʸʻ$$ʳʴʽʲʺʹˁʵʼʶʴ
02110c2f: mov    dword ptr [rdi + 0x20], eax
02110c32: mov    ecx, dword ptr [rdi + 0x18]
02110c35: add    ecx, dword ptr [rdi + 0x14]
02110c38: mov    dword ptr [rdi + 0x18], ecx
02110c3b: mov    eax, dword ptr [rdi + 0x1c]
02110c3e: add    eax, ecx
02110c40: mov    dword ptr [rdi + 0x18], eax
02110c43: mov    r15b, 1
02110c46: mov    r8, qword ptr [rip + 0x1bbd37b]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
02110c4d: mov    rdx, r13
02110c50: lea    rcx, [rsp + 0x38]
02110c55: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02110c5a: movups xmm0, xmmword ptr [rsp + 0x38]
02110c5f: movups xmmword ptr [rsp + 0x50], xmm0
02110c64: movsd  xmm1, qword ptr [rsp + 0x48]
02110c6a: movsd  qword ptr [rsp + 0x60], xmm1
02110c70: mov    qword ptr [rsp + 0x38], r12
02110c75: lea    rbx, [rsp + 0x50]
02110c7a: mov    qword ptr [rsp + 0x40], rbx
02110c7f: nop    
02110c80: mov    rdx, qword ptr [rip + 0x1bae7a9]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
02110c87: lea    rcx, [rsp + 0x50]
02110c8c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02110c91: test   al, al
02110c93: je     0x182110ce0
02110c95: mov    rsi, qword ptr [rsp + 0x60]
02110c9a: test   rsi, rsi
02110c9d: je     0x182110d28
02110ca3: xor    edx, edx
02110ca5: mov    rcx, rsi
02110ca8: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
02110cad: cmp    r14d, eax
02110cb0: sete   dl
02110cb3: movzx  eax, dl
02110cb6: mov    ecx, r12d
02110cb9: cmp    r15b, 1
02110cbd: cmove  ecx, eax
02110cc0: test   ecx, ecx
02110cc2: jne    0x182110cd4
02110cc4: test   r15b, dl
02110cc7: je     0x182110cd8
02110cc9: mov    byte ptr [rdi + 0x4d], cl
02110ccc: mov    byte ptr [rsi + 0xa9], dl
02110cd2: jmp    0x182110c80
02110cd4: movzx  r15d, dl
02110cd8: mov    byte ptr [rsi + 0xa9], dl
02110cde: jmp    0x182110c80
02110ce0: mov    rdx, qword ptr [rip + 0x1bae689]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
02110ce7: mov    rcx, rbx
02110cea: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02110cef: jmp    0x182110d0c
02110cf1: mov    rdx, qword ptr [rip + 0x1bae678]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
02110cf8: mov    rcx, qword ptr [rsp + 0x40]
02110cfd: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02110d02: mov    rcx, qword ptr [rsp + 0x38]
02110d07: test   rcx, rcx
02110d0a: jne    0x182110d2e
02110d0c: add    rsp, 0x70
02110d10: pop    r15
02110d12: pop    r14
02110d14: pop    r13
02110d16: pop    r12
02110d18: pop    rdi
02110d19: pop    rsi
02110d1a: pop    rbx
02110d1b: ret    
02110d1c: call   0x182f60c50
02110d21: nop    
02110d22: call   0x182f60ce0
02110d27: nop    
02110d28: call   0x182f60c50
02110d2d: nop    
02110d2e: call   0x182f60ce0
02110d33: int3   
02110d34: call   0x182f60c50
02110d39: int3   
02110d3a: int3   
