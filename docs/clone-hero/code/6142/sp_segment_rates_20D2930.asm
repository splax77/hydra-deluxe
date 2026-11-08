020d2930: mov    rax, rsp
020d2933: mov    qword ptr [rax + 8], rbx
020d2937: push   rdi
020d2938: sub    rsp, 0xd0
020d293f: movaps xmmword ptr [rax - 0x18], xmm6
020d2943: movaps xmmword ptr [rax - 0x28], xmm7
020d2947: movaps xmmword ptr [rax - 0x38], xmm8
020d294c: movaps xmmword ptr [rax - 0x48], xmm9
020d2951: movaps xmmword ptr [rax - 0x58], xmm10
020d2956: movaps xmmword ptr [rax - 0x68], xmm11
020d295b: movaps xmmword ptr [rax - 0x78], xmm12
020d2960: movaps xmm8, xmm1
020d2964: mov    rbx, rcx
020d2967: cmp    byte ptr [rip + 0x1e4699e], 0            ; [0x3f1930c] (bss)
020d296e: jne    0x1820d29a7
020d2970: lea    rcx, [rip + 0x1bec339]                   ; [0x3cbecb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.Dispose()
020d2977: call   0x182f609b0
020d297c: lea    rcx, [rip + 0x1bec3ed]                   ; [0x3cbed70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.MoveNext()
020d2983: call   0x182f609b0
020d2988: lea    rcx, [rip + 0x1bec4a1]                   ; [0x3cbee30] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Current()
020d298f: call   0x182f609b0
020d2994: lea    rcx, [rip + 0x1bfa4ed]                   ; [0x3ccce88] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.GetEnumerator()
020d299b: call   0x182f609b0
020d29a0: mov    byte ptr [rip + 0x1e46965], 1            ; [0x3f1930c] (bss)
020d29a7: xorps  xmm0, xmm0
020d29aa: xor    eax, eax
020d29ac: movups xmmword ptr [rsp + 0x40], xmm0
020d29b1: mov    qword ptr [rsp + 0x50], rax
020d29b6: test   rbx, rbx
020d29b9: je     0x1820d2b20
020d29bf: xorps  xmm7, xmm7
020d29c2: xorps  xmm6, xmm6
020d29c5: xor    edi, edi
020d29c7: mov    r8, qword ptr [rip + 0x1bfa4ba]          ; [0x3ccce88] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.GetEnumerator()
020d29ce: mov    rdx, rbx
020d29d1: lea    rcx, [rsp + 0x28]
020d29d6: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d29db: movups xmm0, xmmword ptr [rsp + 0x28]
020d29e0: movups xmmword ptr [rsp + 0x40], xmm0
020d29e5: movsd  xmm1, qword ptr [rsp + 0x38]
020d29eb: movsd  qword ptr [rsp + 0x50], xmm1
020d29f1: mov    qword ptr [rsp + 0x28], rdi
020d29f6: lea    rbx, [rsp + 0x40]
020d29fb: mov    qword ptr [rsp + 0x30], rbx
020d2a00: movsd  xmm10, qword ptr [rip + 0xf931b7]        ; [0x3065bc0] dbl=4.0 q=0x4010000000000000
020d2a09: movsd  xmm11, qword ptr [rip + 0xf931b6]        ; [0x3065bc8] dbl=8.0 q=0x4020000000000000
020d2a12: movsd  xmm12, qword ptr [rip + 0xf937d5]        ; [0x30661f0] dbl=30.0 q=0x403e000000000000
020d2a1b: movsd  xmm9, qword ptr [rip + 0xf931ac]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
020d2a24: mov    rdx, qword ptr [rip + 0x1bec345]         ; [0x3cbed70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.MoveNext()
020d2a2b: lea    rcx, [rsp + 0x40]
020d2a30: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d2a35: test   al, al
020d2a37: je     0x1820d2ab4
020d2a39: mov    rcx, qword ptr [rsp + 0x50]
020d2a3e: test   rcx, rcx
020d2a41: je     0x1820d2b14
020d2a47: mov    eax, dword ptr [rcx + 0x28]
020d2a4a: xorps  xmm0, xmm0
020d2a4d: cvtsi2sd xmm0, rax
020d2a52: movaps xmm2, xmm10
020d2a56: divsd  xmm2, xmm0
020d2a5a: mov    eax, dword ptr [rcx + 0x24]
020d2a5d: xorps  xmm1, xmm1
020d2a60: cvtsi2sd xmm1, rax
020d2a65: mulsd  xmm2, xmm1
020d2a69: mulsd  xmm2, xmm11
020d2a6e: divsd  xmm2, xmm12
020d2a73: movsd  qword ptr [rcx + 0x40], xmm2
020d2a78: test   rdi, rdi
020d2a7b: je     0x1820d2aa2
020d2a7d: movsd  xmm1, qword ptr [rcx + 0x18]
020d2a82: subsd  xmm1, xmm7
020d2a86: mulsd  xmm1, xmm8
020d2a8b: mulsd  xmm1, qword ptr [rdi + 0x30]
020d2a90: movsd  xmm0, qword ptr [rdi + 0x40]
020d2a95: mulsd  xmm0, xmm9
020d2a9a: divsd  xmm1, xmm0
020d2a9e: addsd  xmm6, xmm1
020d2aa2: movsd  qword ptr [rcx + 0x38], xmm6
020d2aa7: movsd  xmm7, qword ptr [rcx + 0x18]
020d2aac: mov    rdi, rcx
020d2aaf: jmp    0x1820d2a24
020d2ab4: mov    rdx, qword ptr [rip + 0x1bec1f5]         ; [0x3cbecb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.Dispose()
020d2abb: mov    rcx, rbx
020d2abe: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d2ac3: jmp    0x1820d2ae0
020d2ac5: mov    rdx, qword ptr [rip + 0x1bec1e4]         ; [0x3cbecb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʲʺʴʺʼʷʸʾʷʹ>.Dispose()
020d2acc: mov    rcx, qword ptr [rsp + 0x30]
020d2ad1: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d2ad6: mov    rcx, qword ptr [rsp + 0x28]
020d2adb: test   rcx, rcx
020d2ade: jne    0x1820d2b1a
020d2ae0: lea    r11, [rsp + 0xd0]
020d2ae8: mov    rbx, qword ptr [r11 + 0x10]
020d2aec: movaps xmm6, xmmword ptr [r11 - 0x10]
020d2af1: movaps xmm7, xmmword ptr [r11 - 0x20]
020d2af6: movaps xmm8, xmmword ptr [r11 - 0x30]
020d2afb: movaps xmm9, xmmword ptr [r11 - 0x40]
020d2b00: movaps xmm10, xmmword ptr [r11 - 0x50]
020d2b05: movaps xmm11, xmmword ptr [r11 - 0x60]
020d2b0a: movaps xmm12, xmmword ptr [r11 - 0x70]
020d2b0f: mov    rsp, r11
020d2b12: pop    rdi
020d2b13: ret    
020d2b14: call   0x182f60c50
020d2b19: nop    
020d2b1a: call   0x182f60ce0
020d2b1f: int3   
020d2b20: call   0x182f60c50
020d2b25: int3   
020d2b26: int3   
