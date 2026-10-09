0215d450: mov    qword ptr [rsp + 0x10], rbx
0215d455: mov    qword ptr [rsp + 0x18], rsi
0215d45a: mov    qword ptr [rsp + 8], rcx
0215d45f: push   rdi
0215d460: sub    rsp, 0x60
0215d464: mov    rsi, rcx
0215d467: cmp    byte ptr [rip + 0x1dbc2e9], 0            ; [0x3f19757] (bss)
0215d46e: jne    0x18215d4bf
0215d470: lea    rcx, [rip + 0x1b65071]                   ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215d477: call   0x182f609b0
0215d47c: lea    rcx, [rip + 0x1b65125]                   ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215d483: call   0x182f609b0
0215d488: lea    rcx, [rip + 0x1b651d9]                   ; [0x3cc2668] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Current()
0215d48f: call   0x182f609b0
0215d494: lea    rcx, [rip + 0x1b7a1c5]                   ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215d49b: call   0x182f609b0
0215d4a0: lea    rcx, [rip + 0x1b7a279]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215d4a7: call   0x182f609b0
0215d4ac: lea    rcx, [rip + 0x1b7a32d]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215d4b3: call   0x182f609b0
0215d4b8: mov    byte ptr [rip + 0x1dbc298], 1            ; [0x3f19757] (bss)
0215d4bf: xorps  xmm0, xmm0
0215d4c2: xor    eax, eax
0215d4c4: movups xmmword ptr [rsp + 0x48], xmm0
0215d4c9: mov    qword ptr [rsp + 0x58], rax
0215d4ce: cmp    byte ptr [rsi + 0xa1], al
0215d4d4: je     0x18215d4de
0215d4d6: cmp    byte ptr [rsi + 0xa2], al
0215d4dc: je     0x18215d505
0215d4de: mov    rax, qword ptr [rsi + 0x50]
0215d4e2: test   rax, rax
0215d4e5: je     0x18215d846
0215d4eb: mov    rcx, qword ptr [rax + 0xc8]
0215d4f2: test   rcx, rcx
0215d4f5: je     0x18215d846
0215d4fb: cmp    byte ptr [rcx + 0x10], 0
0215d4ff: je     0x18215d68d
0215d505: mov    rdx, qword ptr [rsi + 0x60]
0215d509: test   rdx, rdx
0215d50c: je     0x18215d846
0215d512: mov    r8, qword ptr [rip + 0x1b7a147]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215d519: lea    rcx, [rsp + 0x30]
0215d51e: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215d523: movups xmm0, xmmword ptr [rsp + 0x30]
0215d528: movups xmmword ptr [rsp + 0x48], xmm0
0215d52d: movsd  xmm1, qword ptr [rsp + 0x40]
0215d533: movsd  qword ptr [rsp + 0x58], xmm1
0215d539: mov    qword ptr [rsp + 0x30], 0
0215d542: lea    rbx, [rsp + 0x48]
0215d547: mov    qword ptr [rsp + 0x38], rbx
0215d54c: nop    dword ptr [rax]
0215d550: mov    rdx, qword ptr [rip + 0x1b65051]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215d557: lea    rcx, [rsp + 0x48]
0215d55c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215d561: test   al, al
0215d563: je     0x18215d655
0215d569: mov    rdi, qword ptr [rsp + 0x58]
0215d56e: test   rdi, rdi
0215d571: je     0x18215d83a
0215d577: mov    ecx, dword ptr [rdi + 0x20]
0215d57a: sub    ecx, 0xe
0215d57d: je     0x18215d644
0215d583: sub    ecx, 1
0215d586: je     0x18215d633
0215d58c: sub    ecx, 1
0215d58f: je     0x18215d644
0215d595: sub    ecx, 1
0215d598: je     0x18215d5ec
0215d59a: cmp    ecx, 1
0215d59d: jne    0x18215d550
0215d59f: mov    eax, dword ptr [rdi + 0x24]
0215d5a2: and    eax, 0xffffffef
0215d5a5: or     eax, 0x20
0215d5a8: mov    dword ptr [rdi + 0x24], eax
0215d5ab: mov    dword ptr [rdi + 0x20], 0x11
0215d5b2: xor    r9d, r9d
0215d5b5: mov    r8d, 0x11
0215d5bb: mov    rdx, rsi
0215d5be: mov    rcx, rdi
0215d5c1: call   0x18214db20                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˀʸʽʻʹʹʻʲˁʲʲ
0215d5c6: mov    rdi, rax
0215d5c9: test   rax, rax
0215d5cc: je     0x18215d550
0215d5ce: xor    edx, edx
0215d5d0: mov    rcx, rax
0215d5d3: call   0x18214c5d0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʲʾʴʴˀʷˀʶʴʼʻ
0215d5d8: test   al, al
0215d5da: je     0x18215d550
0215d5e0: mov    dword ptr [rdi + 0x20], 0x10
0215d5e7: jmp    0x18215d550
0215d5ec: mov    eax, dword ptr [rdi + 0x24]
0215d5ef: and    eax, 0xffffffdf
0215d5f2: or     eax, 0x10
0215d5f5: mov    dword ptr [rdi + 0x24], eax
0215d5f8: xor    r9d, r9d
0215d5fb: mov    r8d, 0x11
0215d601: mov    rdx, rsi
0215d604: mov    rcx, rdi
0215d607: call   0x18214db20                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˀʸʽʻʹʹʻʲˁʲʲ
0215d60c: test   rax, rax
0215d60f: je     0x18215d550
0215d615: xor    edx, edx
0215d617: mov    rcx, rax
0215d61a: call   0x18214dca0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʸʷʽʸʸʼʼʲʾʼ
0215d61f: test   al, al
0215d621: je     0x18215d550
0215d627: mov    dword ptr [rdi + 0x20], 0x10
0215d62e: jmp    0x18215d550
0215d633: mov    eax, dword ptr [rdi + 0x24]
0215d636: and    eax, 0xffffffef
0215d639: or     eax, 0x20
0215d63c: mov    dword ptr [rdi + 0x24], eax
0215d63f: jmp    0x18215d550
0215d644: mov    eax, dword ptr [rdi + 0x24]
0215d647: and    eax, 0xffffffdf
0215d64a: or     eax, 0x10
0215d64d: mov    dword ptr [rdi + 0x24], eax
0215d650: jmp    0x18215d550
0215d655: mov    rdx, qword ptr [rip + 0x1b64e8c]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215d65c: mov    rcx, rbx
0215d65f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215d664: jmp    0x18215d827
0215d669: mov    rdx, qword ptr [rip + 0x1b64e78]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215d670: mov    rcx, qword ptr [rsp + 0x38]
0215d675: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215d67a: mov    rcx, qword ptr [rsp + 0x30]
0215d67f: test   rcx, rcx
0215d682: jne    0x18215d840
0215d688: mov    rsi, qword ptr [rsp + 0x70]
0215d68d: xor    edi, edi
0215d68f: xor    ecx, ecx
0215d691: mov    rax, qword ptr [rsi + 0x60]
0215d695: test   rax, rax
0215d698: je     0x18215d846
0215d69e: nop    
0215d6a0: cmp    ecx, dword ptr [rax + 0x18]
0215d6a3: jge    0x18215d756
0215d6a9: mov    rcx, qword ptr [rsi + 0x60]
0215d6ad: test   rcx, rcx
0215d6b0: je     0x18215d846
0215d6b6: mov    r8, qword ptr [rip + 0x1b7a123]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215d6bd: mov    edx, edi
0215d6bf: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215d6c4: mov    rbx, rax
0215d6c7: test   rax, rax
0215d6ca: je     0x18215d846
0215d6d0: mov    edx, dword ptr [rax + 0x20]
0215d6d3: sub    edx, 0xe
0215d6d6: je     0x18215d740
0215d6d8: sub    edx, 1
0215d6db: je     0x18215d740
0215d6dd: sub    edx, 1
0215d6e0: je     0x18215d740
0215d6e2: sub    edx, 1
0215d6e5: je     0x18215d6f5
0215d6e7: cmp    edx, 1
0215d6ea: jne    0x18215d740
0215d6ec: mov    dword ptr [rax + 0x20], 0x11
0215d6f3: jmp    0x18215d740
0215d6f5: mov    eax, dword ptr [rax + 0x24]
0215d6f8: and    eax, 0xffffffdf
0215d6fb: or     eax, 0x10
0215d6fe: mov    dword ptr [rbx + 0x24], eax
0215d701: xor    r9d, r9d
0215d704: mov    r8d, 0x12
0215d70a: mov    rdx, rsi
0215d70d: mov    rcx, rbx
0215d710: call   0x18214db20                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˀʸʽʻʹʹʻʲˁʲʲ
0215d715: test   rax, rax
0215d718: je     0x18215d740
0215d71a: xor    r9d, r9d
0215d71d: mov    r8d, 0x10
0215d723: mov    rdx, rsi
0215d726: mov    rcx, rbx
0215d729: call   0x18214db20                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˀʸʽʻʹʹʻʲˁʲʲ
0215d72e: test   rax, rax
0215d731: je     0x18215d739
0215d733: mov    eax, dword ptr [rax + 0x24]
0215d736: mov    dword ptr [rbx + 0x24], eax
0215d739: mov    dword ptr [rbx + 0x20], 0x10
0215d740: inc    edi
0215d742: mov    ecx, edi
0215d744: mov    rax, qword ptr [rsi + 0x60]
0215d748: test   rax, rax
0215d74b: je     0x18215d846
0215d751: jmp    0x18215d6a0
0215d756: cmp    byte ptr [rsi + 0xa1], 0
0215d75d: jne    0x18215d827
0215d763: cmp    byte ptr [rsi + 0xa2], 0
0215d76a: jne    0x18215d827
0215d770: mov    rdx, qword ptr [rsi + 0x60]
0215d774: test   rdx, rdx
0215d777: je     0x18215d846
0215d77d: mov    r8, qword ptr [rip + 0x1b79edc]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
0215d784: lea    rcx, [rsp + 0x30]
0215d789: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215d78e: movups xmm0, xmmword ptr [rsp + 0x30]
0215d793: movups xmmword ptr [rsp + 0x48], xmm0
0215d798: movsd  xmm1, qword ptr [rsp + 0x40]
0215d79e: movsd  qword ptr [rsp + 0x58], xmm1
0215d7a4: mov    qword ptr [rsp + 0x30], 0
0215d7ad: lea    rbx, [rsp + 0x48]
0215d7b2: mov    qword ptr [rsp + 0x38], rbx
0215d7b7: nop    word ptr [rax + rax]
0215d7c0: mov    rdx, qword ptr [rip + 0x1b64de1]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
0215d7c7: lea    rcx, [rsp + 0x48]
0215d7cc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215d7d1: test   al, al
0215d7d3: je     0x18215d7fb
0215d7d5: mov    rdi, qword ptr [rsp + 0x58]
0215d7da: test   rdi, rdi
0215d7dd: je     0x18215d84c
0215d7df: xor    edx, edx
0215d7e1: mov    rcx, rdi
0215d7e4: call   0x18214c8a0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʴʾʲʼʿʲʺʷʴʽʿ
0215d7e9: test   al, al
0215d7eb: jne    0x18215d7c0
0215d7ed: mov    eax, dword ptr [rdi + 0x24]
0215d7f0: and    eax, 0xffffffdf
0215d7f3: or     eax, 0x10
0215d7f6: mov    dword ptr [rdi + 0x24], eax
0215d7f9: jmp    0x18215d7c0
0215d7fb: mov    rdx, qword ptr [rip + 0x1b64ce6]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215d802: mov    rcx, rbx
0215d805: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215d80a: jmp    0x18215d827
0215d80c: mov    rdx, qword ptr [rip + 0x1b64cd5]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
0215d813: mov    rcx, qword ptr [rsp + 0x38]
0215d818: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215d81d: mov    rcx, qword ptr [rsp + 0x30]
0215d822: test   rcx, rcx
0215d825: jne    0x18215d852
0215d827: mov    rbx, qword ptr [rsp + 0x78]
0215d82c: mov    rsi, qword ptr [rsp + 0x80]
0215d834: add    rsp, 0x60
0215d838: pop    rdi
0215d839: ret    
0215d83a: call   0x182f60c50
0215d83f: nop    
0215d840: call   0x182f60ce0
0215d845: int3   
0215d846: call   0x182f60c50
0215d84b: nop    
0215d84c: call   0x182f60c50
0215d851: nop    
0215d852: call   0x182f60ce0
0215d857: int3   
0215d858: int3   
