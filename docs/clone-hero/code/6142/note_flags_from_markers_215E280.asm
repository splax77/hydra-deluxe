0215e280: mov    qword ptr [rsp + 0x10], rbx
0215e285: mov    qword ptr [rsp + 8], rcx
0215e28a: push   rdi
0215e28b: sub    rsp, 0x60
0215e28f: mov    rdi, rcx
0215e292: cmp    byte ptr [rip + 0x1dbb49f], 0            ; [0x3f19738] (bss)
0215e299: jne    0x18215e2d2
0215e29b: lea    rcx, [rip + 0x1b64246]                   ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e2a2: call   0x182f609b0
0215e2a7: lea    rcx, [rip + 0x1b642fa]                   ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215e2ae: call   0x182f609b0
0215e2b3: lea    rcx, [rip + 0x1b643ae]                   ; [0x3cc2668] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Current()
0215e2ba: call   0x182f609b0
0215e2bf: lea    rcx, [rip + 0x1b7939a]                   ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215e2c6: call   0x182f609b0
0215e2cb: mov    byte ptr [rip + 0x1dbb466], 1            ; [0x3f19738] (bss)
0215e2d2: xorps  xmm0, xmm0
0215e2d5: xor    eax, eax
0215e2d7: movups xmmword ptr [rsp + 0x40], xmm0
0215e2dc: mov    qword ptr [rsp + 0x50], rax
0215e2e1: xor    r9d, r9d
0215e2e4: xor    r8d, r8d
0215e2e7: mov    dl, 7
0215e2e9: mov    rcx, rdi
0215e2ec: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215e2f1: xor    r9d, r9d
0215e2f4: xor    r8d, r8d
0215e2f7: mov    dl, 6
0215e2f9: mov    rcx, rdi
0215e2fc: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215e301: xor    r9d, r9d
0215e304: xor    r8d, r8d
0215e307: mov    dl, 8
0215e309: mov    rcx, rdi
0215e30c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215e311: xor    r9d, r9d
0215e314: xor    r8d, r8d
0215e317: mov    dl, 0xa
0215e319: mov    rcx, rdi
0215e31c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215e321: xor    r9d, r9d
0215e324: xor    r8d, r8d
0215e327: mov    dl, 9
0215e329: mov    rcx, rdi
0215e32c: call   0x18215dbd0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʹʵʼʶʳʾʸʷʹʵ
0215e331: mov    rdx, qword ptr [rdi + 0x60]
0215e335: test   rdx, rdx
0215e338: je     0x18215e402
0215e33e: mov    r8, qword ptr [rip + 0x1b7931b]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215e345: lea    rcx, [rsp + 0x28]
0215e34a: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215e34f: movups xmm0, xmmword ptr [rsp + 0x28]
0215e354: movups xmmword ptr [rsp + 0x40], xmm0
0215e359: movsd  xmm1, qword ptr [rsp + 0x38]
0215e35f: movsd  qword ptr [rsp + 0x50], xmm1
0215e365: mov    qword ptr [rsp + 0x28], 0
0215e36e: lea    rbx, [rsp + 0x40]
0215e373: mov    qword ptr [rsp + 0x30], rbx
0215e378: nop    dword ptr [rax + rax]
0215e380: mov    rdx, qword ptr [rip + 0x1b64221]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215e387: lea    rcx, [rsp + 0x40]
0215e38c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215e391: test   al, al
0215e393: je     0x18215e3a8
0215e395: mov    rcx, qword ptr [rsp + 0x50]
0215e39a: test   rcx, rcx
0215e39d: je     0x18215e408
0215e39f: xor    edx, edx
0215e3a1: call   0x18214cee0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʷʿʹʿˁʺʲʼʳʴʿ
0215e3a6: jmp    0x18215e380
0215e3a8: mov    rdx, qword ptr [rip + 0x1b64139]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e3af: mov    rcx, rbx
0215e3b2: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215e3b7: jmp    0x18215e3d9
0215e3b9: mov    rdx, qword ptr [rip + 0x1b64128]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215e3c0: mov    rcx, qword ptr [rsp + 0x30]
0215e3c5: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215e3ca: mov    rcx, qword ptr [rsp + 0x28]
0215e3cf: test   rcx, rcx
0215e3d2: jne    0x18215e40e
0215e3d4: mov    rdi, qword ptr [rsp + 0x70]
0215e3d9: xor    edx, edx
0215e3db: mov    rcx, rdi
0215e3de: call   0x18215cd50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʵʲʷʿʺʼˁˀʷʽʵ
0215e3e3: xor    edx, edx
0215e3e5: mov    rcx, rdi
0215e3e8: call   0x18215c040                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʺʺʹʴʼʻʲʶʹʴ
0215e3ed: xor    edx, edx
0215e3ef: mov    rcx, rdi
0215e3f2: call   0x18215d450                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʵʻʷʹʾʸʿʿˁʸʶ
0215e3f7: mov    rbx, qword ptr [rsp + 0x78]
0215e3fc: add    rsp, 0x60
0215e400: pop    rdi
0215e401: ret    
0215e402: call   0x182f60c50
0215e407: nop    
0215e408: call   0x182f60c50
0215e40d: nop    
0215e40e: call   0x182f60ce0
0215e413: int3   
0215e414: int3   
