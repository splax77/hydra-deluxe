0215da70: mov    qword ptr [rsp + 8], rbx
0215da75: push   rdi
0215da76: sub    rsp, 0x60
0215da7a: mov    rdi, rcx
0215da7d: cmp    byte ptr [rip + 0x1dbbcb1], 0            ; [0x3f19735] (bss)
0215da84: jne    0x18215dabd
0215da86: lea    rcx, [rip + 0x1b61463]                   ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215da8d: call   0x182f609b0
0215da92: lea    rcx, [rip + 0x1b61517]                   ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
0215da99: call   0x182f609b0
0215da9e: lea    rcx, [rip + 0x1b615cb]                   ; [0x3cbf070] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Current()
0215daa5: call   0x182f609b0
0215daaa: lea    rcx, [rip + 0x1b6f917]                   ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
0215dab1: call   0x182f609b0
0215dab6: mov    byte ptr [rip + 0x1dbbc78], 1            ; [0x3f19735] (bss)
0215dabd: xorps  xmm0, xmm0
0215dac0: xor    eax, eax
0215dac2: movups xmmword ptr [rsp + 0x40], xmm0
0215dac7: mov    qword ptr [rsp + 0x50], rax
0215dacc: mov    rax, qword ptr [rdi + 0x50]
0215dad0: test   rax, rax
0215dad3: je     0x18215dbc3
0215dad9: cmp    qword ptr [rax + 0x100], 0
0215dae1: je     0x18215dba1
0215dae7: mov    rdx, qword ptr [rdi + 0x88]
0215daee: test   rdx, rdx
0215daf1: je     0x18215dbc3
0215daf7: mov    r8, qword ptr [rip + 0x1b6f8ca]          ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
0215dafe: lea    rcx, [rsp + 0x28]
0215db03: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215db08: movups xmm0, xmmword ptr [rsp + 0x28]
0215db0d: movups xmmword ptr [rsp + 0x40], xmm0
0215db12: movsd  xmm1, qword ptr [rsp + 0x38]
0215db18: movsd  qword ptr [rsp + 0x50], xmm1
0215db1e: mov    qword ptr [rsp + 0x28], 0
0215db27: lea    rbx, [rsp + 0x40]
0215db2c: mov    qword ptr [rsp + 0x30], rbx
0215db31: mov    rdx, qword ptr [rip + 0x1b61478]         ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
0215db38: lea    rcx, [rsp + 0x40]
0215db3d: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0215db42: test   al, al
0215db44: je     0x18215db75
0215db46: mov    rcx, qword ptr [rsp + 0x50]
0215db4b: test   rcx, rcx
0215db4e: je     0x18215dbb7
0215db50: mov    rax, qword ptr [rdi + 0x50]
0215db54: test   rax, rax
0215db57: je     0x18215dbb2
0215db59: mov    rax, qword ptr [rax + 0x100]
0215db60: test   rax, rax
0215db63: je     0x18215dbac
0215db65: mov    rax, qword ptr [rax + 0x10]
0215db69: cmp    qword ptr [rcx + 0x10], rax
0215db6d: setge  al
0215db70: mov    byte ptr [rcx + 0x20], al
0215db73: jmp    0x18215db31
0215db75: mov    rdx, qword ptr [rip + 0x1b61374]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215db7c: mov    rcx, rbx
0215db7f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215db84: jmp    0x18215dba1
0215db86: mov    rdx, qword ptr [rip + 0x1b61363]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
0215db8d: mov    rcx, qword ptr [rsp + 0x30]
0215db92: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215db97: mov    rcx, qword ptr [rsp + 0x28]
0215db9c: test   rcx, rcx
0215db9f: jne    0x18215dbbd
0215dba1: mov    rbx, qword ptr [rsp + 0x70]
0215dba6: add    rsp, 0x60
0215dbaa: pop    rdi
0215dbab: ret    
0215dbac: call   0x182f60c50
0215dbb1: nop    
0215dbb2: call   0x182f60c50
0215dbb7: call   0x182f60c50
0215dbbc: nop    
0215dbbd: call   0x182f60ce0
0215dbc2: int3   
0215dbc3: call   0x182f60c50
0215dbc8: int3   
0215dbc9: int3   
