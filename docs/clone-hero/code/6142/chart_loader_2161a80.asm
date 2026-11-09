02161a80: mov    qword ptr [rsp + 8], rbx
02161a85: mov    qword ptr [rsp + 0x18], rsi
02161a8a: push   rdi
02161a8b: push   r12
02161a8d: push   r13
02161a8f: push   r14
02161a91: push   r15
02161a93: sub    rsp, 0xc0
02161a9a: mov    r12, rdx
02161a9d: mov    rdi, rcx
02161aa0: cmp    byte ptr [rip + 0x1db7c04], 0            ; [0x3f196ab] (bss)
02161aa7: jne    0x182161b04
02161aa9: lea    rcx, [rip + 0x1b5eac0]                   ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
02161ab0: call   0x182f609b0
02161ab5: lea    rcx, [rip + 0x1b5eb74]                   ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
02161abc: call   0x182f609b0
02161ac1: lea    rcx, [rip + 0x1b5ec20]                   ; [0x3cc06e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Current()
02161ac8: call   0x182f609b0
02161acd: lea    rcx, [rip + 0x1b6f7ec]                   ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
02161ad4: call   0x182f609b0
02161ad9: lea    rcx, [rip + 0x1b3ff00]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02161ae0: call   0x182f609b0
02161ae5: lea    rcx, [rip + 0x1b40274]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02161aec: call   0x182f609b0
02161af1: lea    rcx, [rip + 0x1b40cf8]                   ; [0x3ca27f0] metamethod:Method$System.ReadOnlySpan<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Length()
02161af8: call   0x182f609b0
02161afd: mov    byte ptr [rip + 0x1db7ba7], 1            ; [0x3f196ab] (bss)
02161b04: xorps  xmm0, xmm0
02161b07: movups xmmword ptr [rsp + 0x80], xmm0
02161b0f: xorps  xmm1, xmm1
02161b12: xor    eax, eax
02161b14: movups xmmword ptr [rsp + 0x60], xmm1
02161b19: mov    qword ptr [rsp + 0x70], rax
02161b1e: mov    rax, qword ptr [r12]
02161b22: test   rax, rax
02161b25: je     0x182161f07
02161b2b: mov    r13, qword ptr [rax + 0x30]
02161b2f: mov    qword ptr [rsp + 0xf8], r13
02161b37: xor    r15d, r15d
02161b3a: mov    qword ptr [rsp + 0x90], r15
02161b42: mov    ebx, r15d
02161b45: cmp    dword ptr [rdi + 8], ebx
02161b48: jle    0x182161cd7
02161b4e: nop    
02161b50: cmp    ebx, dword ptr [rdi + 8]
02161b53: jge    0x182161cd7
02161b59: jae    0x182161e70
02161b5f: movsxd rcx, ebx
02161b62: mov    rax, qword ptr [rdi]
02161b65: movzx  edx, word ptr [rax + rcx*2]
02161b69: mov    ecx, edx
02161b6b: sub    ecx, 9
02161b6e: je     0x182161cd0
02161b74: sub    ecx, 1
02161b77: je     0x182161cd0
02161b7d: sub    ecx, 1
02161b80: je     0x182161eac
02161b86: sub    ecx, 1
02161b89: je     0x182161eac
02161b8f: cmp    ecx, 1
02161b92: je     0x182161cd0
02161b98: cmp    edx, 0x20
02161b9b: je     0x182161cd0
02161ba1: cmp    edx, 0x5b
02161ba4: jne    0x182161eac
02161baa: inc    ebx
02161bac: movsxd r14, ebx
02161baf: mov    esi, r15d
02161bb2: cmp    ebx, dword ptr [rdi + 8]
02161bb5: jae    0x182161e76
02161bbb: movsxd rcx, ebx
02161bbe: mov    rax, qword ptr [rdi]
02161bc1: cmp    word ptr [rax + rcx*2], 0x5d
02161bc6: je     0x182161bce
02161bc8: inc    ebx
02161bca: inc    esi
02161bcc: jmp    0x182161bb2
02161bce: mov    r15, qword ptr [rip + 0x1b3fe0b]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02161bd5: cmp    r14d, dword ptr [rdi + 8]
02161bd9: ja     0x182161be5
02161bdb: mov    eax, dword ptr [rdi + 8]
02161bde: sub    eax, r14d
02161be1: cmp    esi, eax
02161be3: jbe    0x182161bec
02161be5: xor    ecx, ecx
02161be7: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02161bec: xorps  xmm0, xmm0
02161bef: movups xmmword ptr [rsp + 0x30], xmm0
02161bf4: mov    rcx, qword ptr [r15 + 0x20]
02161bf8: test   byte ptr [rcx + 0x135], 1
02161bff: jne    0x182161c06
02161c01: call   0x182f65750
02161c06: mov    rax, qword ptr [rdi]
02161c09: lea    rcx, [rax + r14*2]
02161c0d: mov    qword ptr [rsp + 0x30], rcx
02161c12: mov    dword ptr [rsp + 0x38], esi
02161c16: inc    ebx
02161c18: nop    dword ptr [rax + rax]
02161c20: cmp    ebx, dword ptr [rdi + 8]
02161c23: jae    0x182161e7b
02161c29: movsxd rcx, ebx
02161c2c: mov    rax, qword ptr [rdi]
02161c2f: movzx  edx, word ptr [rax + rcx*2]
02161c33: mov    ecx, edx
02161c35: sub    ecx, 9
02161c38: je     0x182161c16
02161c3a: sub    ecx, 1
02161c3d: je     0x182161c16
02161c3f: sub    ecx, 1
02161c42: je     0x182161e85
02161c48: sub    ecx, 1
02161c4b: je     0x182161e85
02161c51: cmp    ecx, 1
02161c54: je     0x182161c16
02161c56: cmp    edx, 0x20
02161c59: je     0x182161c16
02161c5b: cmp    edx, 0x7b
02161c5e: jne    0x182161e85
02161c64: inc    ebx
02161c66: cmp    ebx, dword ptr [rdi + 8]
02161c69: jae    0x182161e80
02161c6f: movsxd rcx, ebx
02161c72: mov    rax, qword ptr [rdi]
02161c75: movzx  edx, word ptr [rax + rcx*2]
02161c79: mov    ecx, edx
02161c7b: sub    ecx, 9
02161c7e: je     0x182161c64
02161c80: sub    ecx, 1
02161c83: je     0x182161c64
02161c85: sub    ecx, 1
02161c88: je     0x182161c99
02161c8a: sub    ecx, 1
02161c8d: je     0x182161c99
02161c8f: cmp    ecx, 1
02161c92: je     0x182161c64
02161c94: cmp    edx, 0x20
02161c97: je     0x182161c64
02161c99: movups xmm0, xmmword ptr [rdi]
02161c9c: movaps xmmword ptr [rsp + 0x40], xmm0
02161ca1: movaps xmm1, xmmword ptr [rsp + 0x30]
02161ca6: movdqa xmmword ptr [rsp + 0x30], xmm1
02161cac: xor    r15d, r15d
02161caf: mov    qword ptr [rsp + 0x20], r15
02161cb4: mov    r9, r12
02161cb7: mov    r8d, ebx
02161cba: lea    rdx, [rsp + 0x40]
02161cbf: lea    rcx, [rsp + 0x30]
02161cc4: call   0x182150c00
02161cc9: mov    ebx, eax
02161ccb: jmp    0x182161b50
02161cd0: inc    ebx
02161cd2: jmp    0x182161b50
02161cd7: test   r13, r13
02161cda: je     0x182161ef5
02161ce0: xor    edx, edx
02161ce2: mov    rcx, r13
02161ce5: call   0x18212d820                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʽʳʶʲʾʵʴʻʽʻʺ
02161cea: xor    dil, dil
02161ced: xor    sil, sil
02161cf0: xor    r9d, r9d
02161cf3: lea    r8, [rsp + 0x80]
02161cfb: mov    dl, 6
02161cfd: mov    rcx, r13
02161d00: call   0x182152b20
02161d05: test   al, al
02161d07: je     0x182161d56
02161d09: mov    r9, qword ptr [rsp + 0x80]
02161d11: mov    r8d, dword ptr [rsp + 0x88]
02161d19: mov    ecx, r15d
02161d1c: nop    dword ptr [rax]
02161d20: cmp    ecx, r8d
02161d23: jge    0x182161d53
02161d25: jae    0x182161edf
02161d2b: movsxd rax, ecx
02161d2e: mov    rdx, qword ptr [r9 + rax*8]
02161d32: test   rdx, rdx
02161d35: je     0x182161d4f
02161d37: test   dil, dil
02161d3a: jne    0x182161d43
02161d3c: movzx  edi, byte ptr [rdx + 0xa1]
02161d43: test   sil, sil
02161d46: jne    0x182161d4f
02161d48: movzx  esi, byte ptr [rdx + 0xa2]
02161d4f: inc    ecx
02161d51: jmp    0x182161d20
02161d53: xor    r15d, r15d
02161d56: mov    rdx, qword ptr [r13 + 0xd0]
02161d5d: test   rdx, rdx
02161d60: je     0x182161eeb
02161d66: mov    r8, qword ptr [rip + 0x1b6f553]          ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
02161d6d: lea    rcx, [rsp + 0x40]
02161d72: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02161d77: movups xmm0, xmmword ptr [rsp + 0x40]
02161d7c: movups xmmword ptr [rsp + 0x60], xmm0
02161d81: movsd  xmm1, qword ptr [rsp + 0x50]
02161d87: movsd  qword ptr [rsp + 0x70], xmm1
02161d8d: mov    qword ptr [rsp + 0x30], r15
02161d92: lea    rbx, [rsp + 0x60]
02161d97: mov    qword ptr [rsp + 0x38], rbx
02161d9c: nop    dword ptr [rax]
02161da0: mov    rdx, qword ptr [rip + 0x1b5e889]         ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
02161da7: lea    rcx, [rsp + 0x60]
02161dac: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02161db1: test   al, al
02161db3: je     0x182161df5
02161db5: mov    rcx, qword ptr [rsp + 0x70]
02161dba: test   rcx, rcx
02161dbd: je     0x182161da0
02161dbf: cmp    byte ptr [rcx + 0x58], 6
02161dc3: je     0x182161dcb
02161dc5: cmp    byte ptr [rcx + 0x58], 9
02161dc9: jne    0x182161dd9
02161dcb: mov    byte ptr [rcx + 0xa1], dil
02161dd2: mov    byte ptr [rcx + 0xa2], sil
02161dd9: mov    rax, qword ptr [r12]
02161ddd: test   rax, rax
02161de0: je     0x182161ee5
02161de6: cmp    byte ptr [rax + 0x2c], 0
02161dea: jne    0x182161da0
02161dec: xor    edx, edx
02161dee: call   0x18215c320                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʻʳʶʷʷʽʶʼʽʽ
02161df3: jmp    0x182161da0
02161df5: mov    rdx, qword ptr [rip + 0x1b5e774]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
02161dfc: mov    rcx, rbx
02161dff: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02161e04: nop    
02161e05: jmp    0x182161e53
02161e07: mov    rdx, qword ptr [rip + 0x1b5e762]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
02161e0e: mov    rcx, qword ptr [rsp + 0x38]
02161e13: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02161e18: mov    rcx, qword ptr [rsp + 0x30]
02161e1d: test   rcx, rcx
02161e20: jne    0x182161ef0
02161e26: jmp    0x182161e05
02161e28: mov    rax, qword ptr [rsp + 0xf8]
02161e30: test   rax, rax
02161e33: je     0x182161efb
02161e39: mov    byte ptr [rax + 0xc0], 0
02161e40: jmp    0x182161e53
02161e42: mov    rcx, qword ptr [rsp + 0x90]
02161e4a: test   rcx, rcx
02161e4d: jne    0x182161f01
02161e53: lea    r11, [rsp + 0xc0]
02161e5b: mov    rbx, qword ptr [r11 + 0x30]
02161e5f: mov    rsi, qword ptr [r11 + 0x40]
02161e63: mov    rsp, r11
02161e66: pop    r15
02161e68: pop    r14
02161e6a: pop    r13
02161e6c: pop    r12
02161e6e: pop    rdi
02161e6f: ret    
02161e70: call   0x182f60c40
02161e75: nop    
02161e76: call   0x182f60c40
02161e7b: call   0x182f60c40
02161e80: call   0x182f60c40
02161e85: movups xmm0, xmmword ptr [rdi]
02161e88: movaps xmmword ptr [rsp + 0x40], xmm0
02161e8d: xor    eax, eax
02161e8f: mov    qword ptr [rsp + 0x28], rax
02161e94: mov    qword ptr [rsp + 0x20], rax
02161e99: xor    r9d, r9d
02161e9c: mov    r8d, ebx
02161e9f: lea    rdx, [rsp + 0x40]
02161ea4: mov    rcx, r13
02161ea7: call   0x182163100                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʽʻˀʺʳʲʴʳʶʿʵ
02161eac: mov    r8d, dword ptr [rdi + 8]
02161eb0: mov    edx, ebx
02161eb2: mov    rcx, rax
02161eb5: call   0x182c409a0
02161eba: movups xmm0, xmmword ptr [rdi]
02161ebd: movaps xmmword ptr [rsp + 0x40], xmm0
02161ec2: mov    qword ptr [rsp + 0x28], r15
02161ec7: mov    qword ptr [rsp + 0x20], r15
02161ecc: xor    r9d, r9d
02161ecf: mov    r8d, ebx
02161ed2: lea    rdx, [rsp + 0x40]
02161ed7: mov    rcx, r13
02161eda: call   0x182163100                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʽʻˀʺʳʲʴʳʶʿʵ
02161edf: call   0x182f60c40
02161ee4: nop    
02161ee5: call   0x182f60c50
02161eea: nop    
02161eeb: call   0x182f60c50
02161ef0: call   0x182f60ce0
02161ef5: call   0x182f60c50
02161efa: nop    
02161efb: call   0x182f60c50
02161f00: nop    
02161f01: call   0x182f60ce0
02161f06: int3   
02161f07: call   0x182f60c50
02161f0c: int3   
02161f0d: int3   
