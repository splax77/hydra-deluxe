0215f610: mov    qword ptr [rsp + 8], rbx
0215f615: push   rdi
0215f616: sub    rsp, 0x60
0215f61a: mov    rdi, rcx
0215f61d: cmp    byte ptr [rip + 0x1dba11b], 0            ; [0x3f1973f] (bss)
0215f624: jne    0x18215f65d
0215f626: lea    rcx, [rip + 0x1b5f8c3]                   ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215f62d: call   0x182f609b0
0215f632: lea    rcx, [rip + 0x1b5f977]                   ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
0215f639: call   0x182f609b0
0215f63e: lea    rcx, [rip + 0x1b5fa2b]                   ; [0x3cbf070] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Current()
0215f645: call   0x182f609b0
0215f64a: lea    rcx, [rip + 0x1b6dd77]                   ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
0215f651: call   0x182f609b0
0215f656: mov    byte ptr [rip + 0x1dba0e2], 1            ; [0x3f1973f] (bss)
0215f65d: xorps  xmm0, xmm0
0215f660: xor    eax, eax
0215f662: movups xmmword ptr [rsp + 0x40], xmm0
0215f667: mov    qword ptr [rsp + 0x50], rax
0215f66c: mov    rax, qword ptr [rdi + 0x50]
0215f670: test   rax, rax
0215f673: je     0x18215f763
0215f679: cmp    qword ptr [rax + 0x100], 0
0215f681: je     0x18215f741
0215f687: mov    rdx, qword ptr [rdi + 0x88]
0215f68e: test   rdx, rdx
0215f691: je     0x18215f763
0215f697: mov    r8, qword ptr [rip + 0x1b6dd2a]          ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
0215f69e: lea    rcx, [rsp + 0x28]
0215f6a3: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215f6a8: movups xmm0, xmmword ptr [rsp + 0x28]
0215f6ad: movups xmmword ptr [rsp + 0x40], xmm0
0215f6b2: movsd  xmm1, qword ptr [rsp + 0x38]
0215f6b8: movsd  qword ptr [rsp + 0x50], xmm1
0215f6be: mov    qword ptr [rsp + 0x28], 0
0215f6c7: lea    rbx, [rsp + 0x40]
0215f6cc: mov    qword ptr [rsp + 0x30], rbx
0215f6d1: mov    rdx, qword ptr [rip + 0x1b5f8d8]         ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
0215f6d8: lea    rcx, [rsp + 0x40]
0215f6dd: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215f6e2: test   al, al
0215f6e4: je     0x18215f715
0215f6e6: mov    rcx, qword ptr [rsp + 0x50]
0215f6eb: test   rcx, rcx
0215f6ee: je     0x18215f757
0215f6f0: mov    rax, qword ptr [rdi + 0x50]
0215f6f4: test   rax, rax
0215f6f7: je     0x18215f752
0215f6f9: mov    rax, qword ptr [rax + 0x100]
0215f700: test   rax, rax
0215f703: je     0x18215f74c
0215f705: mov    rax, qword ptr [rax + 0x10]
0215f709: cmp    qword ptr [rcx + 0x10], rax
0215f70d: setl   al
0215f710: mov    byte ptr [rcx + 0x20], al
0215f713: jmp    0x18215f6d1
0215f715: mov    rdx, qword ptr [rip + 0x1b5f7d4]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215f71c: mov    rcx, rbx
0215f71f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215f724: jmp    0x18215f741
0215f726: mov    rdx, qword ptr [rip + 0x1b5f7c3]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215f72d: mov    rcx, qword ptr [rsp + 0x30]
0215f732: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215f737: mov    rcx, qword ptr [rsp + 0x28]
0215f73c: test   rcx, rcx
0215f73f: jne    0x18215f75d
0215f741: mov    rbx, qword ptr [rsp + 0x70]
0215f746: add    rsp, 0x60
0215f74a: pop    rdi
0215f74b: ret    
0215f74c: call   0x182f60c50
0215f751: nop    
0215f752: call   0x182f60c50
0215f757: call   0x182f60c50
0215f75c: nop    
0215f75d: call   0x182f60ce0
0215f762: int3   
0215f763: call   0x182f60c50
0215f768: int3   
0215f769: int3   
