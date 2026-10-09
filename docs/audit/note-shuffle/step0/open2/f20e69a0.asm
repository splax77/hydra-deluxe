020e69a0: mov    qword ptr [rsp + 0x10], rbx
020e69a5: push   rbp
020e69a6: push   rsi
020e69a7: push   rdi
020e69a8: push   r12
020e69aa: push   r13
020e69ac: push   r14
020e69ae: push   r15
020e69b0: mov    rbp, rsp
020e69b3: sub    rsp, 0x80
020e69ba: mov    r15, rdx
020e69bd: mov    r13, rcx
020e69c0: mov    rcx, qword ptr [rcx + 0x10]
020e69c4: xor    edx, edx
020e69c6: call   0x1820cf940
020e69cb: mov    eax, eax
020e69cd: mov    qword ptr [rbp + 0x58], rax
020e69d1: cmp    rax, 1
020e69d5: jbe    0x1820e6d66
020e69db: mov    dword ptr [rbp + 0x40], 1
020e69e2: xor    r14d, r14d
020e69e5: nop    word ptr [rax + rax]
020e69f0: cmp    byte ptr [rip + 0x1e32924], 0            ; [0x3f1931b] (bss)
020e69f7: mov    rdi, qword ptr [r13 + 0x10]
020e69fb: jne    0x1820e6a10
020e69fd: lea    rcx, [rip + 0x1c224fc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a04: call   0x182f609b0
020e6a09: mov    byte ptr [rip + 0x1e3290b], 1            ; [0x3f1931b] (bss)
020e6a10: xor    ecx, ecx
020e6a12: call   0x1801bd810
020e6a17: mov    rcx, qword ptr [rip + 0x1c224e2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a1e: mov    rbx, rax
020e6a21: cmp    qword ptr [rcx + 0x38], 0
020e6a26: jne    0x1820e6a2d
020e6a28: call   0x182f657d0
020e6a2d: mov    qword ptr [rbp - 0x48], r14
020e6a31: test   rbx, rbx
020e6a34: je     0x1820e6d9a
020e6a3a: cmp    dword ptr [rbx + 0x18], 8
020e6a3e: jae    0x1820e6a47
020e6a40: xor    ecx, ecx
020e6a42: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6a47: lea    rax, [rbx + 0x20]
020e6a4b: mov    dword ptr [rbp - 0x48], 8
020e6a52: mov    qword ptr [rbp - 0x50], rax
020e6a56: lea    rdx, [rbp - 0x10]
020e6a5a: movaps xmm0, xmmword ptr [rbp - 0x50]
020e6a5e: xor    r8d, r8d
020e6a61: mov    rcx, rdi
020e6a64: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6a69: call   0x1805f18b0
020e6a6e: cmp    dword ptr [rbx + 0x18], 0
020e6a72: jbe    0x1820e6dbe
020e6a78: cmp    byte ptr [rip + 0x1e3289c], 0            ; [0x3f1931b] (bss)
020e6a7f: mov    r12, qword ptr [rbx + 0x20]
020e6a83: mov    rdi, qword ptr [r13 + 0x10]
020e6a87: jne    0x1820e6a9c
020e6a89: lea    rcx, [rip + 0x1c22470]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a90: call   0x182f609b0
020e6a95: mov    byte ptr [rip + 0x1e3287f], 1            ; [0x3f1931b] (bss)
020e6a9c: xor    ecx, ecx
020e6a9e: call   0x1801bd810
020e6aa3: mov    rcx, qword ptr [rip + 0x1c22456]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6aaa: mov    rbx, rax
020e6aad: cmp    qword ptr [rcx + 0x38], 0
020e6ab2: jne    0x1820e6ab9
020e6ab4: call   0x182f657d0
020e6ab9: mov    qword ptr [rbp - 0x38], r14
020e6abd: test   rbx, rbx
020e6ac0: je     0x1820e6d9a
020e6ac6: cmp    dword ptr [rbx + 0x18], 8
020e6aca: jae    0x1820e6ad3
020e6acc: xor    ecx, ecx
020e6ace: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6ad3: lea    rax, [rbx + 0x20]
020e6ad7: mov    dword ptr [rbp - 0x38], 8
020e6ade: mov    qword ptr [rbp - 0x40], rax
020e6ae2: lea    rdx, [rbp - 0x10]
020e6ae6: movaps xmm0, xmmword ptr [rbp - 0x40]
020e6aea: xor    r8d, r8d
020e6aed: mov    rcx, rdi
020e6af0: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6af5: call   0x1805f18b0
020e6afa: cmp    dword ptr [rbx + 0x18], 0
020e6afe: jbe    0x1820e6dbe
020e6b04: cmp    byte ptr [rip + 0x1e3280f], 0            ; [0x3f1931a] (bss)
020e6b0b: mov    rsi, qword ptr [rbx + 0x20]
020e6b0f: mov    rdi, qword ptr [r13 + 0x10]
020e6b13: jne    0x1820e6b28
020e6b15: lea    rcx, [rip + 0x1c223e4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6b1c: call   0x182f609b0
020e6b21: mov    byte ptr [rip + 0x1e327f2], 1            ; [0x3f1931a] (bss)
020e6b28: xor    ecx, ecx
020e6b2a: call   0x1801bd810
020e6b2f: mov    rcx, qword ptr [rip + 0x1c223ca]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6b36: mov    rbx, rax
020e6b39: cmp    qword ptr [rcx + 0x38], 0
020e6b3e: jne    0x1820e6b45
020e6b40: call   0x182f657d0
020e6b45: mov    qword ptr [rbp - 0x28], r14
020e6b49: test   rbx, rbx
020e6b4c: je     0x1820e6d9a
020e6b52: cmp    dword ptr [rbx + 0x18], 4
020e6b56: jae    0x1820e6b5f
020e6b58: xor    ecx, ecx
020e6b5a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6b5f: lea    rax, [rbx + 0x20]
020e6b63: mov    dword ptr [rbp - 0x28], 4
020e6b6a: mov    qword ptr [rbp - 0x30], rax
020e6b6e: lea    rdx, [rbp - 0x10]
020e6b72: movaps xmm0, xmmword ptr [rbp - 0x30]
020e6b76: xor    r8d, r8d
020e6b79: mov    rcx, rdi
020e6b7c: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6b81: call   0x1805f18b0
020e6b86: cmp    dword ptr [rbx + 0x18], 0
020e6b8a: jbe    0x1820e6dbe
020e6b90: cmp    byte ptr [rip + 0x1e32783], 0            ; [0x3f1931a] (bss)
020e6b97: mov    r14d, dword ptr [rbx + 0x20]
020e6b9b: mov    rdi, qword ptr [r13 + 0x10]
020e6b9f: jne    0x1820e6bb4
020e6ba1: lea    rcx, [rip + 0x1c22358]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6ba8: call   0x182f609b0
020e6bad: mov    byte ptr [rip + 0x1e32766], 1            ; [0x3f1931a] (bss)
020e6bb4: xor    ecx, ecx
020e6bb6: call   0x1801bd810
020e6bbb: mov    rcx, qword ptr [rip + 0x1c2233e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6bc2: mov    rbx, rax
020e6bc5: cmp    qword ptr [rcx + 0x38], 0
020e6bca: jne    0x1820e6bd1
020e6bcc: call   0x182f657d0
020e6bd1: mov    qword ptr [rbp - 0x18], 0
020e6bd9: test   rbx, rbx
020e6bdc: je     0x1820e6d9a
020e6be2: cmp    dword ptr [rbx + 0x18], 4
020e6be6: jae    0x1820e6bef
020e6be8: xor    ecx, ecx
020e6bea: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6bef: lea    rax, [rbx + 0x20]
020e6bf3: mov    dword ptr [rbp - 0x18], 4
020e6bfa: mov    qword ptr [rbp - 0x20], rax
020e6bfe: lea    rdx, [rbp - 0x10]
020e6c02: movaps xmm0, xmmword ptr [rbp - 0x20]
020e6c06: xor    r8d, r8d
020e6c09: mov    rcx, rdi
020e6c0c: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6c11: call   0x1805f18b0
020e6c16: cmp    dword ptr [rbx + 0x18], 0
020e6c1a: jbe    0x1820e6dbe
020e6c20: test   r15, r15
020e6c23: je     0x1820e6db8
020e6c29: cmp    byte ptr [rip + 0x1e326ee], 0            ; [0x3f1931e] (bss)
020e6c30: mov    ebx, dword ptr [rbx + 0x20]
020e6c33: jne    0x1820e6c60
020e6c35: lea    rcx, [rip + 0x1bf08a4]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e6c3c: call   0x182f609b0
020e6c41: lea    rcx, [rip + 0x1bf0ad8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e6c48: call   0x182f609b0
020e6c4d: lea    rcx, [rip + 0x1c13f5c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e6c54: call   0x182f609b0
020e6c59: mov    byte ptr [rip + 0x1e326be], 1            ; [0x3f1931e] (bss)
020e6c60: mov    rax, qword ptr [r15 + 0x50]
020e6c64: test   rax, rax
020e6c67: je     0x1820e6db8
020e6c6d: movsxd rcx, dword ptr [rax + 0x114]
020e6c74: xor    eax, eax
020e6c76: cmp    rsi, rcx
020e6c79: cmovle rsi, rax
020e6c7d: mov    rcx, qword ptr [r15 + 0x10]
020e6c81: test   rcx, rcx
020e6c84: je     0x1820e6db8
020e6c8a: mov    rdx, qword ptr [rip + 0x1c13f1f]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e6c91: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e6c96: test   rax, rax
020e6c99: je     0x1820e6db8
020e6c9f: mov    qword ptr [rsp + 0x28], 0
020e6ca8: mov    r9, rsi
020e6cab: mov    r8d, r14d
020e6cae: mov    dword ptr [rsp + 0x20], ebx
020e6cb2: mov    rdx, r12
020e6cb5: mov    rcx, rax
020e6cb8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e6cbd: mov    rcx, qword ptr [r15 + 0x60]
020e6cc1: mov    r9, rax
020e6cc4: test   rcx, rcx
020e6cc7: je     0x1820e6db8
020e6ccd: test   rax, rax
020e6cd0: je     0x1820e6db8
020e6cd6: mov    ecx, dword ptr [rcx + 0x18]
020e6cd9: mov    dword ptr [rax + 0x28], ecx
020e6cdc: mov    rcx, qword ptr [r15 + 0x60]
020e6ce0: test   rcx, rcx
020e6ce3: je     0x1820e6db8
020e6ce9: mov    r10, qword ptr [rip + 0x1bf07f0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e6cf0: inc    dword ptr [rcx + 0x1c]
020e6cf3: mov    rdx, qword ptr [rcx + 0x10]
020e6cf7: test   rdx, rdx
020e6cfa: je     0x1820e6db8
020e6d00: movsxd r8, dword ptr [rcx + 0x18]
020e6d04: cmp    r8d, dword ptr [rdx + 0x18]
020e6d08: jb     0x1820e6d23
020e6d0a: mov    rax, qword ptr [r10 + 0x20]
020e6d0e: mov    rdx, r9
020e6d11: mov    r8, qword ptr [rax + 0xc0]
020e6d18: mov    r8, qword ptr [r8 + 0x70]
020e6d1c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e6d21: jmp    0x1820e6d49
020e6d23: lea    eax, [r8 + 1]
020e6d27: mov    dword ptr [rcx + 0x18], eax
020e6d2a: cmp    r8d, dword ptr [rdx + 0x18]
020e6d2e: jae    0x1820e6dbe
020e6d34: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e6d39: add    rdx, 0x20
020e6d3d: lea    rcx, [rdx + r8*8]
020e6d41: mov    rdx, r9
020e6d44: call   0x182f5fc00
020e6d49: mov    ecx, dword ptr [rbp + 0x40]
020e6d4c: mov    r14d, 0
020e6d52: inc    ecx
020e6d54: movsxd rax, ecx
020e6d57: mov    dword ptr [rbp + 0x40], ecx
020e6d5a: cmp    rax, qword ptr [rbp + 0x58]
020e6d5e: jl     0x1820e69f0
020e6d64: jmp    0x1820e6d6b
020e6d66: test   r15, r15
020e6d69: je     0x1820e6db8
020e6d6b: xor    r8d, r8d
020e6d6e: mov    byte ptr [r15 + 0xa2], 0
020e6d76: xor    edx, edx
020e6d78: mov    rcx, r15
020e6d7b: mov    rbx, qword ptr [rsp + 0xc8]
020e6d83: add    rsp, 0x80
020e6d8a: pop    r15
020e6d8c: pop    r14
020e6d8e: pop    r13
020e6d90: pop    r12
020e6d92: pop    rdi
020e6d93: pop    rsi
020e6d94: pop    rbp
020e6d95: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e6d9a: xor    ecx, ecx
020e6d9c: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6da1: xorps  xmm0, xmm0
020e6da4: lea    rdx, [rbp - 0x10]
020e6da8: xor    r8d, r8d
020e6dab: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6db0: mov    rcx, rdi
020e6db3: call   0x1805f18b0
020e6db8: call   0x182f60c50
020e6dbd: int3   
020e6dbe: call   0x182f60c40
020e6dc3: int3   
020e6dc4: int3   
