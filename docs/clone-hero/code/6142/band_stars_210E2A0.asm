0210e2a0: mov    qword ptr [rsp + 8], rcx
0210e2a5: push   rbx
0210e2a6: push   rsi
0210e2a7: push   rdi
0210e2a8: push   r12
0210e2aa: push   r13
0210e2ac: push   r14
0210e2ae: push   r15
0210e2b0: sub    rsp, 0x70
0210e2b4: mov    rdi, rcx
0210e2b7: cmp    byte ptr [rip + 0x1e0b1a4], 0            ; [0x3f19462] (bss)
0210e2be: jne    0x18210e33f
0210e2c0: lea    rcx, [rip + 0x1b9aa79]                   ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210e2c7: call   0x182f609b0
0210e2cc: lea    rcx, [rip + 0x1b9bf45]                   ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210e2d3: call   0x182f609b0
0210e2d8: lea    rcx, [rip + 0x1bb1091]                   ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210e2df: call   0x182f609b0
0210e2e4: lea    rcx, [rip + 0x1bb1145]                   ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210e2eb: call   0x182f609b0
0210e2f0: lea    rcx, [rip + 0x1bb11f9]                   ; [0x3cbf4f0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Current()
0210e2f7: call   0x182f609b0
0210e2fc: lea    rcx, [rip + 0x1bb091d]                   ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
0210e303: call   0x182f609b0
0210e308: lea    rcx, [rip + 0x1bbfcb9]                   ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210e30f: call   0x182f609b0
0210e314: lea    rcx, [rip + 0x1bbfd6d]                   ; [0x3cce088] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Count()
0210e31b: call   0x182f609b0
0210e320: lea    rcx, [rip + 0x1bcd401]                   ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
0210e327: call   0x182f609b0
0210e32c: lea    rcx, [rip + 0x1bf89f5]                   ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e333: call   0x182f609b0
0210e338: mov    byte ptr [rip + 0x1e0b123], 1            ; [0x3f19462] (bss)
0210e33f: xorps  xmm0, xmm0
0210e342: xor    eax, eax
0210e344: movups xmmword ptr [rsp + 0x50], xmm0
0210e349: mov    qword ptr [rsp + 0x60], rax
0210e34e: test   rdi, rdi
0210e351: je     0x18210e6b4
0210e357: mov    r14, qword ptr [rdi + 0x50]
0210e35b: mov    rcx, qword ptr [rip + 0x1bf89c6]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e362: cmp    dword ptr [rcx + 0xe0], eax
0210e368: jne    0x18210e376
0210e36a: call   0x182f60cf0
0210e36f: mov    rcx, qword ptr [rip + 0x1bf89b2]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e376: mov    rax, qword ptr [rcx + 0xb8]
0210e37d: mov    rsi, qword ptr [rax + 8]
0210e381: test   rsi, rsi
0210e384: jne    0x18210e3f4
0210e386: cmp    dword ptr [rcx + 0xe0], esi
0210e38c: jne    0x18210e39a
0210e38e: call   0x182f60cf0
0210e393: mov    rcx, qword ptr [rip + 0x1bf898e]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e39a: mov    rax, qword ptr [rcx + 0xb8]
0210e3a1: mov    rbx, qword ptr [rax]
0210e3a4: mov    rcx, qword ptr [rip + 0x1bb0875]         ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
0210e3ab: call   0x182f60c00
0210e3b0: mov    rsi, rax
0210e3b3: xor    r9d, r9d
0210e3b6: mov    r8, qword ptr [rip + 0x1bcd36b]          ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
0210e3bd: mov    rdx, rbx
0210e3c0: mov    rcx, rax
0210e3c3: call   0x18170ede0                              ; System.Data.Listeners.Func<object, object, bool>$$.ctor
0210e3c8: mov    rax, qword ptr [rip + 0x1bf8959]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e3cf: mov    r8, qword ptr [rax + 0xb8]
0210e3d6: mov    qword ptr [r8 + 8], rsi
0210e3da: mov    rax, qword ptr [rip + 0x1bf8947]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210e3e1: mov    rcx, qword ptr [rax + 0xb8]
0210e3e8: add    rcx, 8
0210e3ec: mov    rdx, rsi
0210e3ef: call   0x182f5fc00
0210e3f4: mov    r8, qword ptr [rip + 0x1b9be1d]          ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210e3fb: mov    rdx, rsi
0210e3fe: mov    rcx, r14
0210e401: call   0x18048ee00                              ; System.Linq.Enumerable$$Where<object>
0210e406: mov    rdx, qword ptr [rip + 0x1b9a933]         ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210e40d: mov    rcx, rax
0210e410: call   0x18048e370                              ; System.Linq.Enumerable$$ToList<object>
0210e415: mov    r13, rax
0210e418: mov    qword ptr [rsp + 0x20], rax
0210e41d: xor    r12d, r12d
0210e420: mov    qword ptr [rdi + 0x10], r12
0210e424: mov    qword ptr [rdi + 0x18], r12
0210e428: mov    word ptr [rdi + 0x24], 0x101
0210e42e: test   rax, rax
0210e431: je     0x18210e6b4
0210e437: mov    r15d, r12d
0210e43a: mov    dword ptr [rsp + 0xc0], r12d
0210e442: mov    r14d, r12d
0210e445: mov    dword ptr [rsp + 0xc8], r12d
0210e44d: mov    r8, qword ptr [rip + 0x1bbfb74]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210e454: mov    rdx, rax
0210e457: lea    rcx, [rsp + 0x38]
0210e45c: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0210e461: movups xmm0, xmmword ptr [rsp + 0x38]
0210e466: movups xmmword ptr [rsp + 0x50], xmm0
0210e46b: movsd  xmm1, qword ptr [rsp + 0x48]
0210e471: movsd  qword ptr [rsp + 0x60], xmm1
0210e477: mov    qword ptr [rsp + 0x38], r12
0210e47c: lea    rbx, [rsp + 0x50]
0210e481: mov    qword ptr [rsp + 0x40], rbx
0210e486: nop    word ptr [rax + rax]
0210e490: mov    rdx, qword ptr [rip + 0x1bb0f99]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210e497: lea    rcx, [rsp + 0x50]
0210e49c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0210e4a1: test   al, al
0210e4a3: je     0x18210e543
0210e4a9: mov    rsi, qword ptr [rsp + 0x60]
0210e4ae: test   rsi, rsi
0210e4b1: je     0x18210e69c
0210e4b7: mov    eax, dword ptr [rsi + 0x14]
0210e4ba: add    dword ptr [rdi + 0x18], eax
0210e4bd: mov    eax, dword ptr [rsi + 0x10]
0210e4c0: add    dword ptr [rdi + 0x10], eax
0210e4c3: mov    eax, dword ptr [rsi + 0x18]
0210e4c6: add    dword ptr [rdi + 0x14], eax
0210e4c9: mov    eax, dword ptr [rsi + 0x1c]
0210e4cc: add    dword ptr [rdi + 0x1c], eax
0210e4cf: xor    edx, edx
0210e4d1: mov    rcx, rsi
0210e4d4: call   0x18212fa10                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʴʲʵʸʷʼʼˁʺʾ
0210e4d9: movzx  eax, al
0210e4dc: add    r15d, eax
0210e4df: mov    dword ptr [rsp + 0xc0], r15d
0210e4e7: xor    edx, edx
0210e4e9: mov    rcx, rsi
0210e4ec: call   0x1821304e0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʶʾʽʻʲʴʴʴʼˁʸ
0210e4f1: test   al, al
0210e4f3: je     0x18210e4fa
0210e4f5: inc    dword ptr [rdi + 0x28]
0210e4f8: jmp    0x18210e500
0210e4fa: mov    word ptr [rdi + 0x24], 0
0210e500: xor    edx, edx
0210e502: mov    rcx, rsi
0210e505: call   0x18212f4f0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʴʻʽʵʳˀʿʵʻʲ
0210e50a: test   al, al
0210e50c: je     0x18210e513
0210e50e: inc    dword ptr [rdi + 0x2c]
0210e511: jmp    0x18210e517
0210e513: mov    byte ptr [rdi + 0x25], 0
0210e517: xor    edx, edx
0210e519: mov    rcx, rsi
0210e51c: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210e521: cmp    r14d, eax
0210e524: jge    0x18210e490
0210e52a: xor    edx, edx
0210e52c: mov    rcx, rsi
0210e52f: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210e534: mov    r14d, eax
0210e537: mov    dword ptr [rsp + 0xc8], eax
0210e53e: jmp    0x18210e490
0210e543: mov    rdx, qword ptr [rip + 0x1bb0e26]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210e54a: mov    rcx, rbx
0210e54d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210e552: jmp    0x18210e593
0210e554: mov    rdx, qword ptr [rip + 0x1bb0e15]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210e55b: mov    rcx, qword ptr [rsp + 0x40]
0210e560: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210e565: mov    rcx, qword ptr [rsp + 0x38]
0210e56a: test   rcx, rcx
0210e56d: jne    0x18210e6a2
0210e573: xor    r12d, r12d
0210e576: mov    rdi, qword ptr [rsp + 0xb0]
0210e57e: mov    r15d, dword ptr [rsp + 0xc0]
0210e586: mov    r14d, dword ptr [rsp + 0xc8]
0210e58e: mov    r13, qword ptr [rsp + 0x20]
0210e593: mov    eax, r15d
0210e596: cdq    
0210e597: idiv   dword ptr [r13 + 0x18]
0210e59b: mov    byte ptr [rdi + 0x4c], al
0210e59e: xor    r8d, r8d
0210e5a1: mov    edx, dword ptr [rdi + 0x10]
0210e5a4: mov    ecx, dword ptr [rdi + 0x18]
0210e5a7: call   0x18210dd00                              ; ʿˁʺˀʷʼʽʼʶʸʻ$$ʳʴʽʲʺʹˁʵʼʶʴ
0210e5ac: mov    dword ptr [rdi + 0x20], eax
0210e5af: mov    ecx, dword ptr [rdi + 0x18]
0210e5b2: add    ecx, dword ptr [rdi + 0x14]
0210e5b5: mov    dword ptr [rdi + 0x18], ecx
0210e5b8: mov    eax, dword ptr [rdi + 0x1c]
0210e5bb: add    eax, ecx
0210e5bd: mov    dword ptr [rdi + 0x18], eax
0210e5c0: xor    r15b, r15b
0210e5c3: mov    r8, qword ptr [rip + 0x1bbf9fe]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210e5ca: mov    rdx, r13
0210e5cd: lea    rcx, [rsp + 0x38]
0210e5d2: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0210e5d7: movups xmm0, xmmword ptr [rsp + 0x38]
0210e5dc: movups xmmword ptr [rsp + 0x50], xmm0
0210e5e1: movsd  xmm1, qword ptr [rsp + 0x48]
0210e5e7: movsd  qword ptr [rsp + 0x60], xmm1
0210e5ed: mov    qword ptr [rsp + 0x38], r12
0210e5f2: lea    rbx, [rsp + 0x50]
0210e5f7: mov    qword ptr [rsp + 0x40], rbx
0210e5fc: nop    dword ptr [rax]
0210e600: mov    rdx, qword ptr [rip + 0x1bb0e29]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210e607: lea    rcx, [rsp + 0x50]
0210e60c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0210e611: test   al, al
0210e613: je     0x18210e660
0210e615: mov    rsi, qword ptr [rsp + 0x60]
0210e61a: test   rsi, rsi
0210e61d: je     0x18210e6a8
0210e623: xor    edx, edx
0210e625: mov    rcx, rsi
0210e628: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210e62d: cmp    r14d, eax
0210e630: sete   dl
0210e633: movzx  eax, dl
0210e636: mov    ecx, r12d
0210e639: test   r15b, r15b
0210e63c: cmove  ecx, eax
0210e63f: test   ecx, ecx
0210e641: jne    0x18210e654
0210e643: test   r15b, dl
0210e646: je     0x18210e658
0210e648: mov    byte ptr [rdi + 0x4d], 1
0210e64c: mov    byte ptr [rsi + 0xa9], dl
0210e652: jmp    0x18210e600
0210e654: movzx  r15d, dl
0210e658: mov    byte ptr [rsi + 0xa9], dl
0210e65e: jmp    0x18210e600
0210e660: mov    rdx, qword ptr [rip + 0x1bb0d09]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210e667: mov    rcx, rbx
0210e66a: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210e66f: jmp    0x18210e68c
0210e671: mov    rdx, qword ptr [rip + 0x1bb0cf8]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210e678: mov    rcx, qword ptr [rsp + 0x40]
0210e67d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210e682: mov    rcx, qword ptr [rsp + 0x38]
0210e687: test   rcx, rcx
0210e68a: jne    0x18210e6ae
0210e68c: add    rsp, 0x70
0210e690: pop    r15
0210e692: pop    r14
0210e694: pop    r13
0210e696: pop    r12
0210e698: pop    rdi
0210e699: pop    rsi
0210e69a: pop    rbx
0210e69b: ret    
0210e69c: call   0x182f60c50
0210e6a1: nop    
0210e6a2: call   0x182f60ce0
0210e6a7: nop    
0210e6a8: call   0x182f60c50
0210e6ad: nop    
0210e6ae: call   0x182f60ce0
0210e6b3: int3   
0210e6b4: call   0x182f60c50
0210e6b9: int3   
0210e6ba: int3   
