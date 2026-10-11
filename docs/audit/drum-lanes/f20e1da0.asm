020e1da0: push   rbx
020e1da2: push   rsi
020e1da3: push   rdi
020e1da4: push   r14
020e1da6: sub    rsp, 0x78
020e1daa: mov    rsi, rdx
020e1dad: mov    rbx, rcx
020e1db0: cmp    byte ptr [rip + 0x1e374ed], 0            ; [0x3f192a4] (bss)
020e1db7: jne    0x1820e1dfc
020e1db9: lea    rcx, [rip + 0x1bdd7f0]                   ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020e1dc0: call   0x182f609b0
020e1dc5: lea    rcx, [rip + 0x1bdd8a4]                   ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
020e1dcc: call   0x182f609b0
020e1dd1: lea    rcx, [rip + 0x1bdd958]                   ; [0x3cbf730] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Current()
020e1dd8: call   0x182f609b0
020e1ddd: lea    rcx, [rip + 0x1bec59c]                   ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
020e1de4: call   0x182f609b0
020e1de9: lea    rcx, [rip + 0x1bec710]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
020e1df0: call   0x182f609b0
020e1df5: mov    byte ptr [rip + 0x1e374a8], 1            ; [0x3f192a4] (bss)
020e1dfc: xorps  xmm0, xmm0
020e1dff: xor    eax, eax
020e1e01: movups xmmword ptr [rsp + 0x50], xmm0
020e1e06: mov    qword ptr [rsp + 0x60], rax
020e1e0b: test   rbx, rbx
020e1e0e: je     0x1820e1ff1
020e1e14: mov    rax, qword ptr [rbx + 0x80]
020e1e1b: test   rax, rax
020e1e1e: je     0x1820e1ff1
020e1e24: mov    edi, dword ptr [rax + 0x18]
020e1e27: xor    ecx, ecx
020e1e29: call   0x1801bd810
020e1e2e: mov    rdx, rax
020e1e31: test   rax, rax
020e1e34: je     0x1820e1ff1
020e1e3a: cmp    dword ptr [rax + 0x18], 0
020e1e3e: jbe    0x1820e1fca
020e1e44: mov    dword ptr [rax + 0x20], edi
020e1e47: test   rsi, rsi
020e1e4a: je     0x1820e1ff1
020e1e50: mov    r10, qword ptr [rsi]
020e1e53: mov    rax, qword ptr [r10 + 0x3a0]
020e1e5a: mov    qword ptr [rsp + 0x20], rax
020e1e5f: mov    r9d, 4
020e1e65: xor    r8d, r8d
020e1e68: mov    rcx, rsi
020e1e6b: call   qword ptr [r10 + 0x398]
020e1e72: mov    rdx, qword ptr [rbx + 0x80]
020e1e79: test   rdx, rdx
020e1e7c: je     0x1820e1ff1
020e1e82: mov    r8, qword ptr [rip + 0x1bec4f7]          ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
020e1e89: lea    rcx, [rsp + 0x38]
020e1e8e: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020e1e93: movups xmm0, xmmword ptr [rsp + 0x38]
020e1e98: movups xmmword ptr [rsp + 0x50], xmm0
020e1e9d: movsd  xmm1, qword ptr [rsp + 0x48]
020e1ea3: movsd  qword ptr [rsp + 0x60], xmm1
020e1ea9: mov    qword ptr [rsp + 0x38], 0
020e1eb2: lea    rbx, [rsp + 0x50]
020e1eb7: mov    qword ptr [rsp + 0x40], rbx
020e1ebc: nop    dword ptr [rax]
020e1ec0: mov    rdx, qword ptr [rip + 0x1bdd7a9]         ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
020e1ec7: lea    rcx, [rsp + 0x50]
020e1ecc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020e1ed1: test   al, al
020e1ed3: je     0x1820e1f94
020e1ed9: mov    rdi, qword ptr [rsp + 0x60]
020e1ede: test   rdi, rdi
020e1ee1: je     0x1820e1fe5
020e1ee7: mov    r14, qword ptr [rdi + 0x10]
020e1eeb: xor    ecx, ecx
020e1eed: call   0x1801bd810
020e1ef2: mov    rdx, rax
020e1ef5: test   rax, rax
020e1ef8: je     0x1820e1fe0
020e1efe: cmp    dword ptr [rax + 0x18], 0
020e1f02: jbe    0x1820e1fd0
020e1f08: mov    qword ptr [rax + 0x20], r14
020e1f0c: mov    r10, qword ptr [rsi]
020e1f0f: mov    rax, qword ptr [r10 + 0x3a0]
020e1f16: mov    qword ptr [rsp + 0x20], rax
020e1f1b: mov    r9d, 8
020e1f21: xor    r8d, r8d
020e1f24: mov    rcx, rsi
020e1f27: call   qword ptr [r10 + 0x398]
020e1f2e: mov    r14, qword ptr [rdi + 0x18]
020e1f32: xor    ecx, ecx
020e1f34: call   0x1801bd810
020e1f39: mov    rdx, rax
020e1f3c: test   rax, rax
020e1f3f: je     0x1820e1fdb
020e1f45: cmp    dword ptr [rax + 0x18], 0
020e1f49: jbe    0x1820e1fd6
020e1f4f: mov    qword ptr [rax + 0x20], r14
020e1f53: mov    r10, qword ptr [rsi]
020e1f56: mov    rax, qword ptr [r10 + 0x3a0]
020e1f5d: mov    qword ptr [rsp + 0x20], rax
020e1f62: mov    r9d, 8
020e1f68: xor    r8d, r8d
020e1f6b: mov    rcx, rsi
020e1f6e: call   qword ptr [r10 + 0x398]
020e1f75: mov    rax, qword ptr [rsi]
020e1f78: cmp    byte ptr [rdi + 0x21], 0
020e1f7c: setne  dl
020e1f7f: mov    r8, qword ptr [rax + 0x3c0]
020e1f86: mov    rcx, rsi
020e1f89: call   qword ptr [rax + 0x3b8]
020e1f8f: jmp    0x1820e1ec0
020e1f94: mov    rdx, qword ptr [rip + 0x1bdd615]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020e1f9b: mov    rcx, rbx
020e1f9e: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020e1fa3: jmp    0x1820e1fc0
020e1fa5: mov    rdx, qword ptr [rip + 0x1bdd604]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020e1fac: mov    rcx, qword ptr [rsp + 0x40]
020e1fb1: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020e1fb6: mov    rcx, qword ptr [rsp + 0x38]
020e1fbb: test   rcx, rcx
020e1fbe: jne    0x1820e1feb
020e1fc0: add    rsp, 0x78
020e1fc4: pop    r14
020e1fc6: pop    rdi
020e1fc7: pop    rsi
020e1fc8: pop    rbx
020e1fc9: ret    
020e1fca: call   0x182f60c40
020e1fcf: nop    
020e1fd0: call   0x182f60c40
020e1fd5: nop    
020e1fd6: call   0x182f60c40
020e1fdb: call   0x182f60c50
020e1fe0: call   0x182f60c50
020e1fe5: call   0x182f60c50
020e1fea: nop    
020e1feb: call   0x182f60ce0
020e1ff0: int3   
020e1ff1: call   0x182f60c50
020e1ff6: int3   
020e1ff7: int3   
