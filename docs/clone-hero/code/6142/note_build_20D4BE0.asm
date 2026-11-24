020d4be0: mov    rax, rsp
020d4be3: mov    byte ptr [rax + 0x20], r9b
020d4be7: mov    byte ptr [rax + 0x18], r8b
020d4beb: mov    qword ptr [rax + 0x10], rdx
020d4bef: mov    qword ptr [rax + 8], rcx
020d4bf3: push   rbx
020d4bf4: push   rsi
020d4bf5: push   rdi
020d4bf6: push   r12
020d4bf8: push   r13
020d4bfa: push   r14
020d4bfc: push   r15
020d4bfe: sub    rsp, 0xc0
020d4c05: movaps xmmword ptr [rax - 0x48], xmm6
020d4c09: movaps xmmword ptr [rax - 0x58], xmm7
020d4c0d: mov    r13, rdx
020d4c10: cmp    byte ptr [rip + 0x1e446ed], 0            ; [0x3f19304] (bss)
020d4c17: jne    0x1820d4c8c
020d4c19: lea    rcx, [rip + 0x1bed8c8]                   ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
020d4c20: call   0x182f609b0
020d4c25: lea    rcx, [rip + 0x1bed97c]                   ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
020d4c2c: call   0x182f609b0
020d4c31: lea    rcx, [rip + 0x1beda30]                   ; [0x3cc2668] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Current()
020d4c38: call   0x182f609b0
020d4c3d: lea    rcx, [rip + 0x1c0211c]                   ; [0x3cd6d60] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.Add()
020d4c44: call   0x182f609b0
020d4c49: lea    rcx, [rip + 0x1c02a10]                   ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
020d4c50: call   0x182f609b0
020d4c55: lea    rcx, [rip + 0x1c01f84]                   ; [0x3cd6be0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>..ctor()
020d4c5c: call   0x182f609b0
020d4c61: lea    rcx, [rip + 0x1c02ab8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020d4c68: call   0x182f609b0
020d4c6d: lea    rcx, [rip + 0x1c21114]                   ; [0x3cf5d88] meta:System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020d4c74: call   0x182f609b0
020d4c79: lea    rcx, [rip + 0x1bc7e78]                   ; [0x3c9caf8] meta:ʽʿʸʸʾʶʶʾʶʹʲ_TypeInfo
020d4c80: call   0x182f609b0
020d4c85: mov    byte ptr [rip + 0x1e44678], 1            ; [0x3f19304] (bss)
020d4c8c: xorps  xmm0, xmm0
020d4c8f: xor    eax, eax
020d4c91: movups xmmword ptr [rsp + 0x68], xmm0
020d4c96: mov    qword ptr [rsp + 0x78], rax
020d4c9b: mov    rcx, qword ptr [rip + 0x1c210e6]         ; [0x3cf5d88] meta:System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020d4ca2: call   0x182f60c00
020d4ca7: mov    rdi, rax
020d4caa: mov    qword ptr [rsp + 0x90], rax
020d4cb2: mov    rdx, qword ptr [rip + 0x1c01f27]         ; [0x3cd6be0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>..ctor()
020d4cb9: mov    rcx, rax
020d4cbc: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d4cc1: test   r13, r13
020d4cc4: je     0x1820d51a9
020d4cca: mov    rcx, qword ptr [r13 + 0x60]
020d4cce: test   rcx, rcx
020d4cd1: je     0x1820d51a9
020d4cd7: mov    r15, rdi
020d4cda: mov    qword ptr [rsp + 0x88], rdi
020d4ce2: cmp    dword ptr [rcx + 0x18], 0
020d4ce6: je     0x1820d51a4
020d4cec: xor    ebx, ebx
020d4cee: mov    r14d, ebx
020d4cf1: mov    r12d, ebx
020d4cf4: mov    qword ptr [rsp + 0x48], rbx
020d4cf9: mov    r8, qword ptr [rip + 0x1c02960]          ; [0x3cd7660] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.GetEnumerator()
020d4d00: mov    rdx, rcx
020d4d03: lea    rcx, [rsp + 0x50]
020d4d08: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d4d0d: movups xmm0, xmmword ptr [rsp + 0x50]
020d4d12: movups xmmword ptr [rsp + 0x68], xmm0
020d4d17: movsd  xmm1, qword ptr [rsp + 0x60]
020d4d1d: movsd  qword ptr [rsp + 0x78], xmm1
020d4d23: mov    qword ptr [rsp + 0x50], rbx
020d4d28: lea    rbx, [rsp + 0x68]
020d4d2d: mov    qword ptr [rsp + 0x58], rbx
020d4d32: mov    rdx, qword ptr [rip + 0x1bed86f]         ; [0x3cc25a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.MoveNext()
020d4d39: lea    rcx, [rsp + 0x68]
020d4d3e: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d4d43: test   al, al
020d4d45: je     0x1820d5069
020d4d4b: mov    rsi, qword ptr [rsp + 0x78]
020d4d50: test   rsi, rsi
020d4d53: je     0x1820d51c4
020d4d59: xor    r8d, r8d
020d4d5c: mov    rdx, r13
020d4d5f: mov    rcx, rsi
020d4d62: call   0x18214c9e0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʵʺʶʺʶʼʹˁʷʸʳ
020d4d67: cmp    rax, r14
020d4d6a: je     0x1820d4d32
020d4d6c: xor    edx, edx
020d4d6e: mov    rcx, rsi
020d4d71: call   0x1820e7880                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʼʶˀʵʳˁʴˀʼʽ
020d4d76: movzx  r14d, al
020d4d7a: shl    r14d, 4
020d4d7e: xor    edx, edx
020d4d80: mov    rcx, rsi
020d4d83: call   0x1820e7900                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʹʵʷʲʾʶʴʽʲˁ
020d4d88: mov    edi, r14d
020d4d8b: or     edi, 0x20
020d4d8e: test   al, al
020d4d90: cmove  edi, r14d
020d4d94: xor    edx, edx
020d4d96: mov    rcx, rsi
020d4d99: call   0x1820e78f0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʽʺʲʾˁʷʵʹʻʵʹ
020d4d9e: mov    r14d, edi
020d4da1: or     r14d, 0x40
020d4da5: test   al, al
020d4da7: cmove  r14d, edi
020d4dab: xor    edx, edx
020d4dad: mov    rcx, rsi
020d4db0: call   0x18214d580                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʼˁˀʺʻʹʸʸʽʺʹ
020d4db5: mov    edi, r14d
020d4db8: or     edi, 1
020d4dbb: test   al, al
020d4dbd: cmove  edi, r14d
020d4dc1: xor    edx, edx
020d4dc3: mov    rcx, rsi
020d4dc6: call   0x18214d2c0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʸˁʽʹʿˁʶʵʴʽ
020d4dcb: mov    r14d, edi
020d4dce: bts    r14d, 0x14
020d4dd3: test   al, al
020d4dd5: cmove  r14d, edi
020d4dd9: xor    edx, edx
020d4ddb: mov    rcx, rsi
020d4dde: call   0x18214cdc0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʷʶʳˀˀʸʿʷʲʷʷ
020d4de3: mov    edi, r14d
020d4de6: bts    edi, 0xc
020d4dea: test   al, al
020d4dec: cmove  edi, r14d
020d4df0: xor    edx, edx
020d4df2: mov    rcx, rsi
020d4df5: call   0x18214c880                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʴʹˀʿʶʵʷʺʿʸʺ
020d4dfa: mov    r14d, edi
020d4dfd: bts    r14d, 0xd
020d4e02: test   al, al
020d4e04: cmove  r14d, edi
020d4e08: xor    edx, edx
020d4e0a: mov    rcx, rsi
020d4e0d: call   0x18214cdb0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʷʵʾʾʼʸʺˁʵʺʾ
020d4e12: mov    edi, r14d
020d4e15: bts    edi, 0x11
020d4e19: test   al, al
020d4e1b: cmove  edi, r14d
020d4e1f: xor    edx, edx
020d4e21: mov    rcx, rsi
020d4e24: call   0x18214dc90                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʴˀʽʳʴʹʻʾʴʵ
020d4e29: test   al, al
020d4e2b: jne    0x1820d4e41
020d4e2d: xor    edx, edx
020d4e2f: mov    rcx, rsi
020d4e32: call   0x18214c8b0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʵʶʲʵʹʸʺʼʶʵʷ
020d4e37: test   al, al
020d4e39: je     0x1820d4e45
020d4e3b: bts    edi, 0x12
020d4e3f: jmp    0x1820d4e45
020d4e41: bts    edi, 0x13
020d4e45: xor    edx, edx
020d4e47: mov    rcx, rsi
020d4e4a: call   0x18214cdc0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʷʶʳˀˀʸʿʷʲʷʷ
020d4e4f: mov    ecx, dword ptr [rsi + 0x20]
020d4e52: test   al, al
020d4e54: jne    0x1820d4e83
020d4e56: cmp    ecx, 0xe
020d4e59: je     0x1820d4e93
020d4e5b: xor    edx, edx
020d4e5d: mov    rcx, rsi
020d4e60: call   0x18214c5d0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʲʾʴʴˀʷˀʶʴʼʻ
020d4e65: test   al, al
020d4e67: jne    0x1820d4e7d
020d4e69: xor    edx, edx
020d4e6b: mov    rcx, rsi
020d4e6e: call   0x18214dca0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʸʷʽʸʸʼʼʲʾʼ
020d4e73: test   al, al
020d4e75: je     0x1820d4e97
020d4e77: bts    edi, 0x15
020d4e7b: jmp    0x1820d4e97
020d4e7d: bts    edi, 0x16
020d4e81: jmp    0x1820d4e97
020d4e83: cmp    ecx, 0xf
020d4e86: je     0x1820d4e93
020d4e88: cmp    ecx, 0xe
020d4e8b: jne    0x1820d4e97
020d4e8d: bts    edi, 0x15
020d4e91: jmp    0x1820d4e97
020d4e93: bts    edi, 0x17
020d4e97: xor    r9d, r9d
020d4e9a: movzx  r8d, byte ptr [rsp + 0x110]
020d4ea3: mov    rdx, rsi
020d4ea6: mov    rcx, r13
020d4ea9: call   0x1820d36a0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʺʷʸˀʷʴʿʲʵʼʾ
020d4eae: movzx  r9d, ax
020d4eb2: mov    word ptr [rsp + 0x40], r9w
020d4eb8: mov    r14, rsi
020d4ebb: mov    qword ptr [rsp + 0x80], rsi
020d4ec3: mov    r13d, dword ptr [rsi + 0x2c]
020d4ec7: mov    eax, dword ptr [rsi + 0x28]
020d4eca: mov    dword ptr [rsp + 0x44], eax
020d4ece: cmp    byte ptr [rsi + 0x34], 0
020d4ed2: je     0x1820d4f62
020d4ed8: cmp    byte ptr [rsp + 0x118], 1
020d4ee0: jne    0x1820d4f40
020d4ee2: cmp    r13d, eax
020d4ee5: je     0x1820d4f40
020d4ee7: mov    r8d, r9d
020d4eea: shr    r8d, 3
020d4eee: and    r8d, 0xfffffffe
020d4ef2: or     r8d, r9d
020d4ef5: mov    rdx, r12
020d4ef8: test   r12, r12
020d4efb: je     0x1820d4f40
020d4efd: nop    dword ptr [rax]
020d4f00: movzx  eax, word ptr [rdx + 0x80]
020d4f07: mov    ecx, eax
020d4f09: shr    ecx, 3
020d4f0c: and    ecx, 0xfffffffe
020d4f0f: or     ecx, eax
020d4f11: and    ecx, r8d
020d4f14: test   cl, 0xf
020d4f17: jne    0x1820d4f28
020d4f19: mov    rdx, qword ptr [rdx + 0x10]
020d4f1d: test   rdx, rdx
020d4f20: jne    0x1820d4f00
020d4f22: mov    eax, dword ptr [rsp + 0x44]
020d4f26: jmp    0x1820d4f40
020d4f28: movzx  eax, r9b
020d4f2c: or     word ptr [rdx + 0x80], ax
020d4f33: mov    r13, qword ptr [rsp + 0x108]
020d4f3b: jmp    0x1820d4d32
020d4f40: movzx  ecx, byte ptr [rsp + 0x110]
020d4f48: cmp    cl, 6
020d4f4b: je     0x1820d4f5f
020d4f4d: cmp    cl, 9
020d4f50: je     0x1820d4f5f
020d4f52: or     edi, 0xa
020d4f55: cmp    r13d, eax
020d4f58: je     0x1820d4f62
020d4f5a: or     edi, 4
020d4f5d: jmp    0x1820d4f62
020d4f5f: or     edi, 2
020d4f62: mov    r14, qword ptr [rsp + 0x100]
020d4f6a: test   r14, r14
020d4f6d: je     0x1820d51bf
020d4f73: xor    r8d, r8d
020d4f76: mov    rdx, qword ptr [rsi + 0x10]
020d4f7a: mov    rcx, r14
020d4f7d: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d4f82: movaps xmm7, xmm0
020d4f85: xor    edx, edx
020d4f87: mov    rcx, rsi
020d4f8a: call   0x182109730                              ; ʼʿʷʵʲʶʽʵʿʳʷ$$ʼʺʻʼˀʸʴʻʼˁʽ
020d4f8f: xor    r8d, r8d
020d4f92: mov    rdx, rax
020d4f95: mov    rcx, r14
020d4f98: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d4f9d: movaps xmm6, xmm0
020d4fa0: mov    r14, qword ptr [rsi + 0x10]
020d4fa4: mov    r12, qword ptr [rsi + 0x18]
020d4fa8: mov    rcx, qword ptr [rip + 0x1bc7b49]         ; [0x3c9caf8] meta:ʽʿʸʸʾʶʶʾʶʹʲ_TypeInfo
020d4faf: call   0x182f60c00
020d4fb4: mov    rsi, rax
020d4fb7: movsxd rcx, r12d
020d4fba: mov    qword ptr [rsp + 0x38], 0
020d4fc3: mov    qword ptr [rsp + 0x30], rcx
020d4fc8: mov    qword ptr [rsp + 0x28], r14
020d4fcd: mov    dword ptr [rsp + 0x20], edi
020d4fd1: movzx  r9d, word ptr [rsp + 0x40]
020d4fd7: movaps xmm2, xmm6
020d4fda: movaps xmm1, xmm7
020d4fdd: mov    rcx, rax
020d4fe0: call   0x18214bf10                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$.ctor
020d4fe5: test   rsi, rsi
020d4fe8: je     0x1820d51ba
020d4fee: xor    edx, edx
020d4ff0: mov    rcx, rsi
020d4ff3: call   0x18214b800                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʺʴʲʹʽʶʹʳʻʼʽ
020d4ff8: test   al, al
020d4ffa: je     0x1820d5031
020d4ffc: cmp    r13d, dword ptr [rsp + 0x44]
020d5001: je     0x1820d5031
020d5003: mov    rdi, qword ptr [rsp + 0x48]
020d5008: mov    qword ptr [rsi + 0x10], rdi
020d500c: lea    rcx, [rsi + 0x10]
020d5010: mov    rdx, rdi
020d5013: call   0x182f5fc00
020d5018: test   rdi, rdi
020d501b: je     0x1820d51af
020d5021: mov    qword ptr [rdi + 0x18], rsi
020d5025: lea    rcx, [rdi + 0x18]
020d5029: mov    rdx, rsi
020d502c: call   0x182f5fc00
020d5031: test   r15, r15
020d5034: je     0x1820d51b5
020d503a: mov    r8, qword ptr [rip + 0x1c01d1f]          ; [0x3cd6d60] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.Add()
020d5041: mov    rdx, rsi
020d5044: mov    rcx, r15
020d5047: call   0x182c425f0
020d504c: mov    r12, rsi
020d504f: mov    qword ptr [rsp + 0x48], rsi
020d5054: mov    r14, qword ptr [rsp + 0x80]
020d505c: mov    r13, qword ptr [rsp + 0x108]
020d5064: jmp    0x1820d4d32
020d5069: mov    rdx, qword ptr [rip + 0x1bed478]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
020d5070: mov    rcx, rbx
020d5073: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d5078: xor    r14d, r14d
020d507b: jmp    0x1820d50a7
020d507d: mov    rdx, qword ptr [rip + 0x1bed464]         ; [0x3cc24e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʾʳʿʽʸʸʷʾʼʴʿ>.Dispose()
020d5084: mov    rcx, qword ptr [rsp + 0x58]
020d5089: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d508e: mov    rcx, qword ptr [rsp + 0x50]
020d5093: test   rcx, rcx
020d5096: jne    0x1820d51ca
020d509c: xor    r14d, r14d
020d509f: mov    r15, qword ptr [rsp + 0x88]
020d50a7: cmp    byte ptr [rip + 0x1e4425d], 0            ; [0x3f1930b] (bss)
020d50ae: jne    0x1820d50cf
020d50b0: lea    rcx, [rip + 0x1c02129]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d50b7: call   0x182f609b0
020d50bc: lea    rcx, [rip + 0x1c021dd]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d50c3: call   0x182f609b0
020d50c8: mov    byte ptr [rip + 0x1e4423c], 1            ; [0x3f1930b] (bss)
020d50cf: test   r15, r15
020d50d2: je     0x1820d51a9
020d50d8: xorps  xmm6, xmm6
020d50db: mov    r12, qword ptr [rsp + 0x90]
020d50e3: mov    eax, r14d
020d50e6: nop    word ptr [rax + rax]
020d50f0: cmp    eax, dword ptr [r12 + 0x18]
020d50f5: jge    0x1820d517e
020d50fb: mov    r8, qword ptr [rip + 0x1c0219e]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d5102: mov    edx, r14d
020d5105: mov    rcx, r15
020d5108: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d510d: mov    rdi, rax
020d5110: test   rax, rax
020d5113: je     0x1820d51a9
020d5119: movss  xmm0, dword ptr [rax + 0x38]
020d511e: ucomiss xmm0, xmm6
020d5121: jp     0x1820d5125
020d5123: je     0x1820d5176
020d5125: mov    rsi, qword ptr [rax + 0x48]
020d5129: add    rsi, qword ptr [rax + 0x40]
020d512d: lea    ebx, [r14 + 1]
020d5131: cmp    ebx, dword ptr [r15 + 0x18]
020d5135: jge    0x1820d5176
020d5137: mov    r8, qword ptr [rip + 0x1c02162]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d513e: mov    edx, ebx
020d5140: mov    rcx, r15
020d5143: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d5148: test   rax, rax
020d514b: je     0x1820d51a9
020d514d: cmp    qword ptr [rax + 0x40], rsi
020d5151: jg     0x1820d5176
020d5153: mov    rcx, qword ptr [rdi + 0x40]
020d5157: cmp    qword ptr [rax + 0x40], rcx
020d515b: jle    0x1820d5163
020d515d: cmp    qword ptr [rax + 0x40], rsi
020d5161: jl     0x1820d516a
020d5163: cmp    qword ptr [rax + 0x48], 0
020d5168: jne    0x1820d5172
020d516a: or     dword ptr [rdi + 0x7c], 8
020d516e: or     dword ptr [rax + 0x7c], 8
020d5172: inc    ebx
020d5174: jmp    0x1820d5131
020d5176: inc    r14d
020d5179: jmp    0x1820d50e3
020d517e: mov    rax, r15
020d5181: movaps xmm6, xmmword ptr [rsp + 0xb0]
020d5189: movaps xmm7, xmmword ptr [rsp + 0xa0]
020d5191: add    rsp, 0xc0
020d5198: pop    r15
020d519a: pop    r14
020d519c: pop    r13
020d519e: pop    r12
020d51a0: pop    rdi
020d51a1: pop    rsi
020d51a2: pop    rbx
020d51a3: ret    
020d51a4: mov    rax, rdi
020d51a7: jmp    0x1820d5181
020d51a9: call   0x182f60c50
020d51ae: nop    
020d51af: call   0x182f60c50
020d51b4: nop    
020d51b5: call   0x182f60c50
020d51ba: call   0x182f60c50
020d51bf: call   0x182f60c50
020d51c4: call   0x182f60c50
020d51c9: nop    
020d51ca: call   0x182f60ce0
020d51cf: int3   
020d51d0: mov    qword ptr [rsp + 0x10], rbx
020d51d5: push   rdi
020d51d6: sub    rsp, 0x70
020d51da: cmp    byte ptr [rip + 0x1e4412d], 0            ; [0x3f1930e] (bss)
020d51e1: mov    ebx, edx
020d51e3: mov    rdi, rcx
020d51e6: jne    0x1820d51fb
020d51e8: lea    rcx, [rip + 0x1bc3659]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d51ef: call   0x182f609b0
020d51f4: mov    byte ptr [rip + 0x1e44113], 1            ; [0x3f1930e] (bss)
020d51fb: xor    eax, eax
020d51fd: xorps  xmm0, xmm0
020d5200: mov    qword ptr [rsp + 0x30], rax
020d5205: xorps  xmm1, xmm1
020d5208: mov    qword ptr [rsp + 0x48], rax
020d520d: movups xmmword ptr [rsp + 0x20], xmm0
020d5212: movups xmmword ptr [rsp + 0x38], xmm1
020d5217: test   rdi, rdi
020d521a: je     0x1820d5307
020d5220: xor    edx, edx
020d5222: mov    qword ptr [rsp + 0x80], rsi
020d522a: mov    rcx, rdi
020d522d: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020d5232: mov    rcx, qword ptr [rip + 0x1bc360f]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d5239: movzx  esi, ax
020d523c: cmp    dword ptr [rcx + 0xe0], 0
020d5243: jne    0x1820d524a
020d5245: call   0x182f60cf0
020d524a: movzx  ecx, bx
020d524d: xor    r8d, r8d
020d5250: and    ecx, esi
020d5252: mov    rdx, rdi
020d5255: mov    eax, ecx
020d5257: shr    eax, 1
020d5259: or     eax, ecx
020d525b: mov    ecx, eax
020d525d: shr    ecx, 2
020d5260: or     ecx, eax
020d5262: mov    eax, ecx
020d5264: shr    eax, 4
020d5267: or     eax, ecx
020d5269: lea    rcx, [rsp + 0x50]
020d526e: mov    ebx, eax
020d5270: shr    ebx, 8
020d5273: or     ebx, eax
020d5275: inc    ebx
020d5277: shr    ebx, 1
020d5279: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d527e: xor    r8d, r8d
020d5281: lea    rdx, [rsp + 0x38]
020d5286: lea    rcx, [rsp + 0x50]
020d528b: movups xmm0, xmmword ptr [rax]
020d528e: movups xmmword ptr [rsp + 0x38], xmm0
020d5293: movsd  xmm1, qword ptr [rax + 0x10]
020d5298: movsd  qword ptr [rsp + 0x48], xmm1
020d529e: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d52a3: mov    rsi, qword ptr [rsp + 0x80]
020d52ab: movups xmm0, xmmword ptr [rax]
020d52ae: movups xmmword ptr [rsp + 0x20], xmm0
020d52b3: movsd  xmm1, qword ptr [rax + 0x10]
020d52b8: movsd  qword ptr [rsp + 0x30], xmm1
020d52be: nop    
020d52c0: xor    edx, edx
020d52c2: lea    rcx, [rsp + 0x20]
020d52c7: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d52cc: test   al, al
020d52ce: je     0x1820d52f7
020d52d0: mov    rax, qword ptr [rsp + 0x28]
020d52d5: test   rax, rax
020d52d8: je     0x1820d5307
020d52da: movzx  ecx, word ptr [rax + 0x80]
020d52e1: and    cx, bx
020d52e4: cmp    cx, bx
020d52e7: jne    0x1820d52c0
020d52e9: mov    rbx, qword ptr [rsp + 0x88]
020d52f1: add    rsp, 0x70
020d52f5: pop    rdi
020d52f6: ret    
020d52f7: mov    rbx, qword ptr [rsp + 0x88]
020d52ff: xor    eax, eax
020d5301: add    rsp, 0x70
020d5305: pop    rdi
020d5306: ret    
020d5307: call   0x182f60c50
020d530c: int3   
020d530d: int3   
