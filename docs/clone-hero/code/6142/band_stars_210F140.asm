0210f140: mov    qword ptr [rsp + 0x10], rbx
0210f145: mov    qword ptr [rsp + 8], rcx
0210f14a: push   rsi
0210f14b: push   rdi
0210f14c: push   r12
0210f14e: push   r14
0210f150: push   r15
0210f152: sub    rsp, 0x70
0210f156: mov    rdi, rcx
0210f159: cmp    byte ptr [rip + 0x1e0a301], 0            ; [0x3f19461] (bss)
0210f160: jne    0x18210f1e1
0210f162: lea    rcx, [rip + 0x1b99bd7]                   ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210f169: call   0x182f609b0
0210f16e: lea    rcx, [rip + 0x1b9b0a3]                   ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210f175: call   0x182f609b0
0210f17a: lea    rcx, [rip + 0x1bb01ef]                   ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210f181: call   0x182f609b0
0210f186: lea    rcx, [rip + 0x1bb02a3]                   ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210f18d: call   0x182f609b0
0210f192: lea    rcx, [rip + 0x1bb0357]                   ; [0x3cbf4f0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Current()
0210f199: call   0x182f609b0
0210f19e: lea    rcx, [rip + 0x1bafa7b]                   ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
0210f1a5: call   0x182f609b0
0210f1aa: lea    rcx, [rip + 0x1bbee17]                   ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210f1b1: call   0x182f609b0
0210f1b6: lea    rcx, [rip + 0x1bbeecb]                   ; [0x3cce088] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Count()
0210f1bd: call   0x182f609b0
0210f1c2: lea    rcx, [rip + 0x1bcc55f]                   ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
0210f1c9: call   0x182f609b0
0210f1ce: lea    rcx, [rip + 0x1bf7b53]                   ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f1d5: call   0x182f609b0
0210f1da: mov    byte ptr [rip + 0x1e0a280], 1            ; [0x3f19461] (bss)
0210f1e1: xorps  xmm0, xmm0
0210f1e4: xor    eax, eax
0210f1e6: movups xmmword ptr [rsp + 0x50], xmm0
0210f1eb: mov    qword ptr [rsp + 0x60], rax
0210f1f0: test   rdi, rdi
0210f1f3: je     0x18210f559
0210f1f9: mov    r14, qword ptr [rdi + 0x50]
0210f1fd: mov    rcx, qword ptr [rip + 0x1bf7b24]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f204: cmp    dword ptr [rcx + 0xe0], eax
0210f20a: jne    0x18210f218
0210f20c: call   0x182f60cf0
0210f211: mov    rcx, qword ptr [rip + 0x1bf7b10]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f218: mov    rax, qword ptr [rcx + 0xb8]
0210f21f: mov    rsi, qword ptr [rax + 8]
0210f223: test   rsi, rsi
0210f226: jne    0x18210f296
0210f228: cmp    dword ptr [rcx + 0xe0], esi
0210f22e: jne    0x18210f23c
0210f230: call   0x182f60cf0
0210f235: mov    rcx, qword ptr [rip + 0x1bf7aec]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f23c: mov    rax, qword ptr [rcx + 0xb8]
0210f243: mov    rbx, qword ptr [rax]
0210f246: mov    rcx, qword ptr [rip + 0x1baf9d3]         ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
0210f24d: call   0x182f60c00
0210f252: mov    rsi, rax
0210f255: xor    r9d, r9d
0210f258: mov    r8, qword ptr [rip + 0x1bcc4c9]          ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
0210f25f: mov    rdx, rbx
0210f262: mov    rcx, rax
0210f265: call   0x18170ede0                              ; System.Data.Listeners.Func<object, object, bool>$$.ctor
0210f26a: mov    rax, qword ptr [rip + 0x1bf7ab7]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f271: mov    r8, qword ptr [rax + 0xb8]
0210f278: mov    qword ptr [r8 + 8], rsi
0210f27c: mov    rax, qword ptr [rip + 0x1bf7aa5]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
0210f283: mov    rcx, qword ptr [rax + 0xb8]
0210f28a: add    rcx, 8
0210f28e: mov    rdx, rsi
0210f291: call   0x182f5fc00
0210f296: mov    r8, qword ptr [rip + 0x1b9af7b]          ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210f29d: mov    rdx, rsi
0210f2a0: mov    rcx, r14
0210f2a3: call   0x18048ee00                              ; System.Linq.Enumerable$$Where<object>
0210f2a8: mov    rdx, qword ptr [rip + 0x1b99a91]         ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
0210f2af: mov    rcx, rax
0210f2b2: call   0x18048e370                              ; System.Linq.Enumerable$$ToList<object>
0210f2b7: mov    r12, rax
0210f2ba: mov    qword ptr [rsp + 0x20], rax
0210f2bf: mov    r15d, 1
0210f2c5: mov    dword ptr [rdi + 0x10], r15d
0210f2c9: mov    dword ptr [rdi + 0x14], r15d
0210f2cd: mov    dword ptr [rdi + 0x18], r15d
0210f2d1: mov    dword ptr [rdi + 0x1c], r15d
0210f2d5: mov    word ptr [rdi + 0x24], 0x100
0210f2db: test   rax, rax
0210f2de: je     0x18210f559
0210f2e4: mov    dword ptr [rsp + 0xb0], r15d
0210f2ec: xor    r14d, r14d
0210f2ef: mov    dword ptr [rsp + 0xb8], r14d
0210f2f7: mov    r8, qword ptr [rip + 0x1bbecca]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210f2fe: mov    rdx, rax
0210f301: lea    rcx, [rsp + 0x38]
0210f306: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0210f30b: movups xmm0, xmmword ptr [rsp + 0x38]
0210f310: movups xmmword ptr [rsp + 0x50], xmm0
0210f315: movsd  xmm1, qword ptr [rsp + 0x48]
0210f31b: movsd  qword ptr [rsp + 0x60], xmm1
0210f321: mov    qword ptr [rsp + 0x38], r14
0210f326: lea    rbx, [rsp + 0x50]
0210f32b: mov    qword ptr [rsp + 0x40], rbx
0210f330: mov    rdx, qword ptr [rip + 0x1bb00f9]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210f337: lea    rcx, [rsp + 0x50]
0210f33c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0210f341: test   al, al
0210f343: je     0x18210f3e3
0210f349: mov    rsi, qword ptr [rsp + 0x60]
0210f34e: test   rsi, rsi
0210f351: je     0x18210f541
0210f357: mov    eax, dword ptr [rsi + 0x14]
0210f35a: add    dword ptr [rdi + 0x18], eax
0210f35d: mov    eax, dword ptr [rsi + 0x10]
0210f360: add    dword ptr [rdi + 0x10], eax
0210f363: mov    eax, dword ptr [rsi + 0x18]
0210f366: add    dword ptr [rdi + 0x14], eax
0210f369: mov    eax, dword ptr [rsi + 0x1c]
0210f36c: add    dword ptr [rdi + 0x1c], eax
0210f36f: xor    edx, edx
0210f371: mov    rcx, rsi
0210f374: call   0x182130450                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʼʵʺʳʽʾʹʾʾˁ
0210f379: movzx  eax, al
0210f37c: add    r15d, eax
0210f37f: mov    dword ptr [rsp + 0xb0], r15d
0210f387: xor    edx, edx
0210f389: mov    rcx, rsi
0210f38c: call   0x1821304e0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʶʾʽʻʲʴʴʴʼˁʸ
0210f391: test   al, al
0210f393: je     0x18210f39a
0210f395: inc    dword ptr [rdi + 0x28]
0210f398: jmp    0x18210f3a0
0210f39a: mov    word ptr [rdi + 0x24], 0x101
0210f3a0: xor    edx, edx
0210f3a2: mov    rcx, rsi
0210f3a5: call   0x18212f5d0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʷʵʹʲʼˀʶˀʲʴ
0210f3aa: test   al, al
0210f3ac: je     0x18210f3b3
0210f3ae: inc    dword ptr [rdi + 0x2c]
0210f3b1: jmp    0x18210f3b7
0210f3b3: mov    byte ptr [rdi + 0x25], 0
0210f3b7: xor    edx, edx
0210f3b9: mov    rcx, rsi
0210f3bc: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210f3c1: cmp    r14d, eax
0210f3c4: jge    0x18210f330
0210f3ca: xor    edx, edx
0210f3cc: mov    rcx, rsi
0210f3cf: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210f3d4: mov    r14d, eax
0210f3d7: mov    dword ptr [rsp + 0xb8], eax
0210f3de: jmp    0x18210f330
0210f3e3: mov    rdx, qword ptr [rip + 0x1baff86]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210f3ea: mov    rcx, rbx
0210f3ed: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210f3f2: jmp    0x18210f430
0210f3f4: mov    rdx, qword ptr [rip + 0x1baff75]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210f3fb: mov    rcx, qword ptr [rsp + 0x40]
0210f400: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210f405: mov    rcx, qword ptr [rsp + 0x38]
0210f40a: test   rcx, rcx
0210f40d: jne    0x18210f547
0210f413: mov    rdi, qword ptr [rsp + 0xa0]
0210f41b: mov    r15d, dword ptr [rsp + 0xb0]
0210f423: mov    r14d, dword ptr [rsp + 0xb8]
0210f42b: mov    r12, qword ptr [rsp + 0x20]
0210f430: mov    eax, r15d
0210f433: cdq    
0210f434: idiv   dword ptr [r12 + 0x18]
0210f439: mov    byte ptr [rdi + 0x4c], al
0210f43c: xor    r8d, r8d
0210f43f: mov    edx, dword ptr [rdi + 0x10]
0210f442: mov    ecx, dword ptr [rdi + 0x18]
0210f445: call   0x18210dd00                              ; ʿˁʺˀʷʼʽʼʶʸʻ$$ʳʴʽʲʺʹˁʵʼʶʴ
0210f44a: mov    dword ptr [rdi + 0x20], eax
0210f44d: mov    ecx, dword ptr [rdi + 0x18]
0210f450: add    ecx, dword ptr [rdi + 0x14]
0210f453: mov    dword ptr [rdi + 0x18], ecx
0210f456: mov    eax, dword ptr [rdi + 0x1c]
0210f459: add    eax, ecx
0210f45b: mov    dword ptr [rdi + 0x18], eax
0210f45e: mov    r15b, 1
0210f461: mov    r8, qword ptr [rip + 0x1bbeb60]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0210f468: mov    rdx, r12
0210f46b: lea    rcx, [rsp + 0x38]
0210f470: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0210f475: movups xmm0, xmmword ptr [rsp + 0x38]
0210f47a: movups xmmword ptr [rsp + 0x50], xmm0
0210f47f: movsd  xmm1, qword ptr [rsp + 0x48]
0210f485: movsd  qword ptr [rsp + 0x60], xmm1
0210f48b: mov    qword ptr [rsp + 0x38], 0
0210f494: lea    rbx, [rsp + 0x50]
0210f499: mov    qword ptr [rsp + 0x40], rbx
0210f49e: nop    
0210f4a0: mov    rdx, qword ptr [rip + 0x1baff89]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0210f4a7: lea    rcx, [rsp + 0x50]
0210f4ac: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0210f4b1: test   al, al
0210f4b3: je     0x18210f500
0210f4b5: mov    rsi, qword ptr [rsp + 0x60]
0210f4ba: test   rsi, rsi
0210f4bd: je     0x18210f54d
0210f4c3: xor    edx, edx
0210f4c5: mov    rcx, rsi
0210f4c8: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0210f4cd: cmp    r14d, eax
0210f4d0: sete   dl
0210f4d3: movzx  eax, dl
0210f4d6: xor    ecx, ecx
0210f4d8: cmp    r15b, 1
0210f4dc: cmove  ecx, eax
0210f4df: test   ecx, ecx
0210f4e1: jne    0x18210f4f4
0210f4e3: test   r15b, dl
0210f4e6: je     0x18210f4f8
0210f4e8: mov    byte ptr [rdi + 0x4d], 1
0210f4ec: mov    byte ptr [rsi + 0xa9], dl
0210f4f2: jmp    0x18210f4a0
0210f4f4: movzx  r15d, dl
0210f4f8: mov    byte ptr [rsi + 0xa9], dl
0210f4fe: jmp    0x18210f4a0
0210f500: mov    rdx, qword ptr [rip + 0x1bafe69]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210f507: mov    rcx, rbx
0210f50a: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210f50f: jmp    0x18210f52c
0210f511: mov    rdx, qword ptr [rip + 0x1bafe58]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
0210f518: mov    rcx, qword ptr [rsp + 0x40]
0210f51d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0210f522: mov    rcx, qword ptr [rsp + 0x38]
0210f527: test   rcx, rcx
0210f52a: jne    0x18210f553
0210f52c: mov    rbx, qword ptr [rsp + 0xa8]
0210f534: add    rsp, 0x70
0210f538: pop    r15
0210f53a: pop    r14
0210f53c: pop    r12
0210f53e: pop    rdi
0210f53f: pop    rsi
0210f540: ret    
0210f541: call   0x182f60c50
0210f546: nop    
0210f547: call   0x182f60ce0
0210f54c: nop    
0210f54d: call   0x182f60c50
0210f552: nop    
0210f553: call   0x182f60ce0
0210f558: int3   
0210f559: call   0x182f60c50
0210f55e: int3   
0210f55f: int3   
