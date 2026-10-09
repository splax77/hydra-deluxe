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
