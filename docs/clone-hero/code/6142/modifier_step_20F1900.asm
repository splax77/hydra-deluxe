020f1900: mov    qword ptr [rsp + 8], rcx
020f1905: push   rbx
020f1906: push   rsi
020f1907: push   rdi
020f1908: push   r12
020f190a: push   r14
020f190c: push   r15
020f190e: sub    rsp, 0x78
020f1912: movsx  rbx, dl
020f1916: mov    rdi, rcx
020f1919: cmp    byte ptr [rip + 0x1e27a51], 0            ; [0x3f19371] (bss)
020f1920: jne    0x1820f1959
020f1922: lea    rcx, [rip + 0x1bd097f]                   ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1929: call   0x182f609b0
020f192e: lea    rcx, [rip + 0x1bd0a33]                   ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f1935: call   0x182f609b0
020f193a: lea    rcx, [rip + 0x1bd0ae7]                   ; [0x3cc2428] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Current()
020f1941: call   0x182f609b0
020f1946: lea    rcx, [rip + 0x1be5593]                   ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f194d: call   0x182f609b0
020f1952: mov    byte ptr [rip + 0x1e27a18], 1            ; [0x3f19371] (bss)
020f1959: xorps  xmm0, xmm0
020f195c: xor    eax, eax
020f195e: movups xmmword ptr [rsp + 0x58], xmm0
020f1963: mov    qword ptr [rsp + 0x68], rax
020f1968: cmp    bl, 0xe
020f196b: ja     0x1820f1de1
020f1971: lea    rdx, [rip - 0x20f1978]                   ; [0x0] (bss)
020f1978: mov    eax, dword ptr [rdx + rbx*4 + 0x20f1e28]
020f197f: add    rax, rdx
020f1982: jmp    rax
020f1984: test   rdi, rdi
020f1987: je     0x1820f1e1f
020f198d: mov    r8, qword ptr [rip + 0x1be554c]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f1994: mov    rdx, rdi
020f1997: lea    rcx, [rsp + 0x40]
020f199c: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f19a1: movups xmm0, xmmword ptr [rsp + 0x40]
020f19a6: movups xmmword ptr [rsp + 0x58], xmm0
020f19ab: movsd  xmm1, qword ptr [rsp + 0x50]
020f19b1: movsd  qword ptr [rsp + 0x68], xmm1
020f19b7: mov    qword ptr [rsp + 0x40], 0
020f19c0: lea    rbx, [rsp + 0x58]
020f19c5: mov    qword ptr [rsp + 0x48], rbx
020f19ca: nop    word ptr [rax + rax]
020f19d0: mov    rdx, qword ptr [rip + 0x1bd0991]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f19d7: lea    rcx, [rsp + 0x58]
020f19dc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f19e1: test   al, al
020f19e3: je     0x1820f1a79
020f19e9: mov    r8, qword ptr [rsp + 0x68]
020f19ee: test   r8, r8
020f19f1: je     0x1820f1def
020f19f7: movzx  r9d, word ptr [r8 + 0x80]
020f19ff: cmp    r9w, 0x3f
020f1a04: je     0x1820f19d0
020f1a06: movzx  ecx, r9w
020f1a0a: and    cx, 0x10
020f1a0e: shl    cx, 2
020f1a12: movzx  edx, cx
020f1a15: or     dx, 0x20
020f1a19: test   r9b, 0x20
020f1a1d: cmove  dx, cx
020f1a21: movzx  ecx, dx
020f1a24: or     cx, 0x10
020f1a28: test   r9b, 0x40
020f1a2c: cmove  cx, dx
020f1a30: movzx  edx, cx
020f1a33: or     dx, 8
020f1a37: test   r9b, 2
020f1a3b: cmove  dx, cx
020f1a3f: movzx  ecx, dx
020f1a42: or     cx, 4
020f1a46: test   r9b, 4
020f1a4a: cmove  cx, dx
020f1a4e: movzx  edx, cx
020f1a51: or     dx, 2
020f1a55: test   r9b, 8
020f1a59: cmove  dx, cx
020f1a5d: movzx  eax, dx
020f1a60: or     ax, 1
020f1a64: test   r9b, 1
020f1a68: cmove  ax, dx
020f1a6c: mov    word ptr [r8 + 0x80], ax
020f1a74: jmp    0x1820f19d0
020f1a79: mov    rdx, qword ptr [rip + 0x1bd0828]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1a80: mov    rcx, rbx
020f1a83: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1a88: jmp    0x1820f1de1
020f1a8d: mov    rdx, qword ptr [rip + 0x1bd0814]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1a94: mov    rcx, qword ptr [rsp + 0x48]
020f1a99: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1a9e: mov    rcx, qword ptr [rsp + 0x40]
020f1aa3: test   rcx, rcx
020f1aa6: jne    0x1820f1df5
020f1aac: mov    rdi, qword ptr [rsp + 0xb0]
020f1ab4: jmp    0x1820f1abf
020f1ab6: test   rdi, rdi
020f1ab9: je     0x1820f1e1f
020f1abf: mov    r8, qword ptr [rip + 0x1be541a]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f1ac6: mov    rdx, rdi
020f1ac9: lea    rcx, [rsp + 0x40]
020f1ace: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f1ad3: movups xmm0, xmmword ptr [rsp + 0x40]
020f1ad8: movups xmmword ptr [rsp + 0x58], xmm0
020f1add: movsd  xmm1, qword ptr [rsp + 0x50]
020f1ae3: movsd  qword ptr [rsp + 0x68], xmm1
020f1ae9: mov    qword ptr [rsp + 0x40], 0
020f1af2: lea    rbx, [rsp + 0x58]
020f1af7: mov    qword ptr [rsp + 0x48], rbx
020f1afc: nop    dword ptr [rax]
020f1b00: mov    rdx, qword ptr [rip + 0x1bd0861]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f1b07: lea    rcx, [rsp + 0x58]
020f1b0c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f1b11: test   al, al
020f1b13: je     0x1820f1b9a
020f1b19: mov    r9, qword ptr [rsp + 0x68]
020f1b1e: test   r9, r9
020f1b21: je     0x1820f1dfb
020f1b27: movzx  r8d, word ptr [r9 + 0x80]
020f1b2f: cmp    r8w, 0x1f
020f1b34: je     0x1820f1b00
020f1b36: movzx  ecx, r8w
020f1b3a: and    cx, 2
020f1b3e: shl    cx, 4
020f1b42: movzx  edx, cx
020f1b45: or     dx, 0x10
020f1b49: test   r8b, 4
020f1b4d: cmove  dx, cx
020f1b51: movzx  ecx, dx
020f1b54: or     cx, 8
020f1b58: test   r8b, 8
020f1b5c: cmove  cx, dx
020f1b60: movzx  edx, cx
020f1b63: or     dx, 4
020f1b67: test   r8b, 0x10
020f1b6b: cmove  dx, cx
020f1b6f: movzx  ecx, dx
020f1b72: or     cx, 2
020f1b76: test   r8b, 0x20
020f1b7a: cmove  cx, dx
020f1b7e: movzx  eax, cx
020f1b81: or     ax, 1
020f1b85: test   r8b, 1
020f1b89: cmove  ax, cx
020f1b8d: mov    word ptr [r9 + 0x80], ax
020f1b95: jmp    0x1820f1b00
020f1b9a: mov    rdx, qword ptr [rip + 0x1bd0707]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1ba1: mov    rcx, rbx
020f1ba4: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1ba9: jmp    0x1820f1de1
020f1bae: mov    rdx, qword ptr [rip + 0x1bd06f3]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1bb5: mov    rcx, qword ptr [rsp + 0x48]
020f1bba: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1bbf: mov    rcx, qword ptr [rsp + 0x40]
020f1bc4: test   rcx, rcx
020f1bc7: jne    0x1820f1e01
020f1bcd: mov    rdi, qword ptr [rsp + 0xb0]
020f1bd5: jmp    0x1820f1be0
020f1bd7: test   rdi, rdi
020f1bda: je     0x1820f1e1f
020f1be0: mov    r8, qword ptr [rip + 0x1be52f9]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f1be7: mov    rdx, rdi
020f1bea: lea    rcx, [rsp + 0x40]
020f1bef: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f1bf4: movups xmm0, xmmword ptr [rsp + 0x40]
020f1bf9: movups xmmword ptr [rsp + 0x58], xmm0
020f1bfe: movsd  xmm1, qword ptr [rsp + 0x50]
020f1c04: movsd  qword ptr [rsp + 0x68], xmm1
020f1c0a: mov    qword ptr [rsp + 0x40], 0
020f1c13: lea    rbx, [rsp + 0x58]
020f1c18: mov    qword ptr [rsp + 0x48], rbx
020f1c1d: nop    dword ptr [rax]
020f1c20: mov    rdx, qword ptr [rip + 0x1bd0741]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f1c27: lea    rcx, [rsp + 0x58]
020f1c2c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f1c31: test   al, al
020f1c33: je     0x1820f1c8a
020f1c35: mov    rax, qword ptr [rsp + 0x68]
020f1c3a: test   rax, rax
020f1c3d: je     0x1820f1e07
020f1c43: movzx  ecx, word ptr [rax + 0x80]
020f1c4a: cmp    cx, 0xf
020f1c4e: je     0x1820f1c20
020f1c50: test   cl, 2
020f1c53: je     0x1820f1c5e
020f1c55: mov    word ptr [rax + 0x80], 0x10
020f1c5e: test   cl, 4
020f1c61: je     0x1820f1c6c
020f1c63: mov    word ptr [rax + 0x80], 8
020f1c6c: test   cl, 8
020f1c6f: je     0x1820f1c7a
020f1c71: mov    word ptr [rax + 0x80], 4
020f1c7a: test   cl, 0x10
020f1c7d: je     0x1820f1c20
020f1c7f: mov    word ptr [rax + 0x80], 2
020f1c88: jmp    0x1820f1c20
020f1c8a: mov    rdx, qword ptr [rip + 0x1bd0617]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1c91: mov    rcx, rbx
020f1c94: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1c99: jmp    0x1820f1cc2
020f1c9b: mov    rdx, qword ptr [rip + 0x1bd0606]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1ca2: mov    rcx, qword ptr [rsp + 0x48]
020f1ca7: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1cac: mov    rcx, qword ptr [rsp + 0x40]
020f1cb1: test   rcx, rcx
020f1cb4: jne    0x1820f1e0d
020f1cba: mov    rdi, qword ptr [rsp + 0xb0]
020f1cc2: mov    r8, qword ptr [rip + 0x1be5217]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f1cc9: mov    rdx, rdi
020f1ccc: lea    rcx, [rsp + 0x40]
020f1cd1: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f1cd6: movups xmm0, xmmword ptr [rsp + 0x40]
020f1cdb: movups xmmword ptr [rsp + 0x58], xmm0
020f1ce0: movsd  xmm1, qword ptr [rsp + 0x50]
020f1ce6: movsd  qword ptr [rsp + 0x68], xmm1
020f1cec: mov    qword ptr [rsp + 0x40], 0
020f1cf5: lea    rbx, [rsp + 0x58]
020f1cfa: mov    qword ptr [rsp + 0x48], rbx
020f1cff: mov    r14d, 0x40
020f1d05: mov    r15d, 0x20
020f1d0b: nop    dword ptr [rax + rax]
020f1d10: mov    rdx, qword ptr [rip + 0x1bd0651]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f1d17: lea    rcx, [rsp + 0x58]
020f1d1c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f1d21: test   al, al
020f1d23: je     0x1820f1db5
020f1d29: mov    rdi, qword ptr [rsp + 0x68]
020f1d2e: test   rdi, rdi
020f1d31: je     0x1820f1e13
020f1d37: movzx  esi, word ptr [rdi + 0x80]
020f1d3e: xor    edx, edx
020f1d40: mov    rcx, rdi
020f1d43: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f1d48: cmp    si, 0xf
020f1d4c: je     0x1820f1d10
020f1d4e: movzx  ecx, si
020f1d51: xor    cx, ax
020f1d54: test   r15b, sil
020f1d57: je     0x1820f1d71
020f1d59: test   cl, 0x10
020f1d5c: jne    0x1820f1d69
020f1d5e: mov    word ptr [rdi + 0x80], 0x80
020f1d67: jmp    0x1820f1d71
020f1d69: mov    word ptr [rdi + 0x80], r14w
020f1d71: test   r14b, sil
020f1d74: je     0x1820f1d8d
020f1d76: test   cl, 8
020f1d79: jne    0x1820f1d85
020f1d7b: mov    word ptr [rdi + 0x80], r14w
020f1d83: jmp    0x1820f1d8d
020f1d85: mov    word ptr [rdi + 0x80], r15w
020f1d8d: test   sil, sil
020f1d90: jns    0x1820f1d10
020f1d96: test   cl, 4
020f1d99: jne    0x1820f1da8
020f1d9b: mov    word ptr [rdi + 0x80], r15w
020f1da3: jmp    0x1820f1d10
020f1da8: mov    word ptr [rdi + 0x80], r14w
020f1db0: jmp    0x1820f1d10
020f1db5: mov    rdx, qword ptr [rip + 0x1bd04ec]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1dbc: mov    rcx, rbx
020f1dbf: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1dc4: jmp    0x1820f1de1
020f1dc6: mov    rdx, qword ptr [rip + 0x1bd04db]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1dcd: mov    rcx, qword ptr [rsp + 0x48]
020f1dd2: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1dd7: mov    rcx, qword ptr [rsp + 0x40]
020f1ddc: test   rcx, rcx
020f1ddf: jne    0x1820f1e19
020f1de1: add    rsp, 0x78
020f1de5: pop    r15
020f1de7: pop    r14
020f1de9: pop    r12
020f1deb: pop    rdi
020f1dec: pop    rsi
020f1ded: pop    rbx
020f1dee: ret    
020f1def: call   0x182f60c50
020f1df4: nop    
020f1df5: call   0x182f60ce0
020f1dfa: nop    
020f1dfb: call   0x182f60c50
020f1e00: nop    
020f1e01: call   0x182f60ce0
020f1e06: nop    
020f1e07: call   0x182f60c50
020f1e0c: nop    
020f1e0d: call   0x182f60ce0
020f1e12: nop    
020f1e13: call   0x182f60c50
020f1e18: nop    
020f1e19: call   0x182f60ce0
020f1e1e: int3   
020f1e1f: call   0x182f60c50
020f1e24: int3   
020f1e25: nop    dword ptr [rax]
020f1e28: mov    dh, 0x1a
020f1e2a: lar    esi, word ptr [rsi - 0x49fdf0e6]
020f1e31: sbb    cl, byte ptr [rdi]
020f1e33: add    dh, byte ptr [rsi - 0x7bfdf0e6]
020f1e39: sbb    dword ptr [rdi], ecx
020f1e3b: add    al, byte ptr [rcx + rbx + 0x1bd7020f]
020f1e42: lar    esi, word ptr [rsi - 0x1efdf0e6]
020f1e49: sbb    eax, 0x1bd7020f
020f1e4e: lar    eax, word ptr [rcx + rbx + 0x1984020f]
020f1e56: lar    esp, ecx
020f1e59: sbb    eax, 0x1de1020f
020f1e5e: lar    eax, word ptr [rcx + rbx - 0x3333fdf1]
020f1e66: int3   
020f1e67: int3   
