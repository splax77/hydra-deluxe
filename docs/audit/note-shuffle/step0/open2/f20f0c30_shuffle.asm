020f0c30: mov    qword ptr [rsp + 0x10], rbx
020f0c35: mov    qword ptr [rsp + 0x18], rsi
020f0c3a: push   rdi
020f0c3b: push   r12
020f0c3d: push   r13
020f0c3f: push   r14
020f0c41: push   r15
020f0c43: sub    rsp, 0xa0
020f0c4a: movsx  r14, dl
020f0c4e: mov    r15, rcx
020f0c51: cmp    byte ptr [rip + 0x1e28718], 0            ; [0x3f19370] (bss)
020f0c58: jne    0x1820f0cd9
020f0c5a: lea    rcx, [rip + 0x1bd1647]                   ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f0c61: call   0x182f609b0
020f0c66: lea    rcx, [rip + 0x1bd16fb]                   ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f0c6d: call   0x182f609b0
020f0c72: lea    rcx, [rip + 0x1bd17af]                   ; [0x3cc2428] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Current()
020f0c79: call   0x182f609b0
020f0c7e: lea    rcx, [rip + 0x1be625b]                   ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f0c85: call   0x182f609b0
020f0c8a: lea    rcx, [rip + 0x1be654f]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f0c91: call   0x182f609b0
020f0c96: lea    rcx, [rip + 0x1be6603]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0c9d: call   0x182f609b0
020f0ca2: lea    rcx, [rip + 0x1c10f87]                   ; [0x3d01c30] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʷʹʾʻʶˁʿʶʴʷʷ<int>()
020f0ca9: call   0x182f609b0
020f0cae: lea    rcx, [rip + 0x1ba22c3]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f0cb5: call   0x182f609b0
020f0cba: lea    rcx, [rip + 0x1ba7b87]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f0cc1: call   0x182f609b0
020f0cc6: lea    rcx, [rip + 0x1ba96d3]                   ; [0x3c9a3a0] meta:ʻʾʴʵʻʲʲʸʺˀʼ_TypeInfo
020f0ccd: call   0x182f609b0
020f0cd2: mov    byte ptr [rip + 0x1e28697], 1            ; [0x3f19370] (bss)
020f0cd9: xorps  xmm0, xmm0
020f0cdc: movups xmmword ptr [rsp + 0x30], xmm0
020f0ce1: xorps  xmm1, xmm1
020f0ce4: xor    eax, eax
020f0ce6: movups xmmword ptr [rsp + 0x78], xmm1
020f0ceb: mov    qword ptr [rsp + 0x88], rax
020f0cf3: xor    r13d, r13d
020f0cf6: mov    qword ptr [rsp + 0x48], r13
020f0cfb: mov    qword ptr [rsp + 0x50], r13
020f0d00: test   r15, r15
020f0d03: je     0x1820f11c4
020f0d09: mov    edi, r13d
020f0d0c: mov    esi, dword ptr [r15 + 0x18]
020f0d10: mov    rcx, qword ptr [rip + 0x1ba2261]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f0d17: cmp    dword ptr [rcx + 0xe0], eax
020f0d1d: jne    0x1820f0d24
020f0d1f: call   0x182f60cf0
020f0d24: mov    rbx, qword ptr [rip + 0x1c10f05]         ; [0x3d01c30] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʷʹʾʻʶˁʿʶʴʷʷ<int>()
020f0d2b: mov    dword ptr [rsp + 0xd0], esi
020f0d32: cmp    qword ptr [rbx + 0x38], rdi
020f0d36: jne    0x1820f0d40
020f0d38: mov    rcx, rbx
020f0d3b: call   0x182f657d0
020f0d40: mov    r8, qword ptr [rbx + 0x38]
020f0d44: mov    r8, qword ptr [r8 + 0x10]
020f0d48: xor    edx, edx
020f0d4a: lea    rcx, [rsp + 0xd0]
020f0d52: call   0x181b4e780                              ; System.Int32$$CompareTo
020f0d57: test   eax, eax
020f0d59: js     0x1820f0dbf
020f0d5b: mov    r8, qword ptr [rbx + 0x38]
020f0d5f: mov    r8, qword ptr [r8 + 0x10]
020f0d63: mov    edx, 4
020f0d68: lea    rcx, [rsp + 0xd0]
020f0d70: call   0x181b4e780                              ; System.Int32$$CompareTo
020f0d75: mov    ebx, r13d
020f0d78: test   eax, eax
020f0d7a: jg     0x1820f0d89
020f0d7c: mov    esi, dword ptr [rsp + 0xd0]
020f0d83: test   esi, esi
020f0d85: jle    0x1820f0dbf
020f0d87: jmp    0x1820f0d90
020f0d89: mov    esi, 4
020f0d8e: nop    
020f0d90: mov    r8, qword ptr [rip + 0x1be6509]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0d97: mov    edx, ebx
020f0d99: mov    rcx, r15
020f0d9c: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0da1: test   rax, rax
020f0da4: je     0x1820f11c4
020f0daa: movzx  ecx, word ptr [rax + 0x80]
020f0db1: imul   rcx, qword ptr [rax + 0x40]
020f0db6: add    rdi, rcx
020f0db9: inc    ebx
020f0dbb: cmp    ebx, esi
020f0dbd: jl     0x1820f0d90
020f0dbf: mov    rcx, qword ptr [rip + 0x1ba95da]         ; [0x3c9a3a0] meta:ʻʾʴʵʻʲʲʸʺˀʼ_TypeInfo
020f0dc6: call   0x182f60c00
020f0dcb: mov    rbx, rax
020f0dce: xor    edx, edx
020f0dd0: mov    rcx, rax
020f0dd3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f0dd8: lea    rcx, [rdi*8]
020f0de0: mov    qword ptr [rbx + 0x10], rcx
020f0de4: shr    rdi, 3
020f0de8: mov    qword ptr [rbx + 0x18], rdi
020f0dec: mov    qword ptr [rsp + 0x30], rbx
020f0df1: mov    rdx, rbx
020f0df4: lea    rcx, [rsp + 0x30]
020f0df9: call   0x182f5fc00
020f0dfe: mov    edx, r13d
020f0e01: cmp    r14b, 6
020f0e05: jne    0x1820f0e0c
020f0e07: mov    r12b, 1
020f0e0a: jmp    0x1820f0e1a
020f0e0c: cmp    r14b, 9
020f0e10: sete   r12b
020f0e14: cmp    r14b, 0xe
020f0e18: ja     0x1820f0e62
020f0e1a: lea    r8, [rip - 0x20f0e21]                    ; [0x0] (bss)
020f0e21: mov    eax, dword ptr [r8 + r14*4 + 0x20f11cc]
020f0e29: add    rax, r8
020f0e2c: jmp    rax
020f0e2e: mov    edx, 4
020f0e33: inc    edx
020f0e35: mov    dword ptr [rsp + 0x38], edx
020f0e39: jmp    0x1820f0e75
020f0e3b: mov    edx, 5
020f0e40: inc    edx
020f0e42: mov    dword ptr [rsp + 0x38], edx
020f0e46: jmp    0x1820f0e75
020f0e48: mov    edx, 7
020f0e4d: inc    edx
020f0e4f: mov    dword ptr [rsp + 0x38], edx
020f0e53: jmp    0x1820f0e75
020f0e55: mov    edx, 6
020f0e5a: inc    edx
020f0e5c: mov    dword ptr [rsp + 0x38], edx
020f0e60: jmp    0x1820f0e75
020f0e62: lea    r8, [rip - 0x20f0e69]                    ; [0x0] (bss)
020f0e69: inc    edx
020f0e6b: mov    dword ptr [rsp + 0x38], edx
020f0e6f: cmp    r14b, 0xe
020f0e73: ja     0x1820f0e99
020f0e75: mov    eax, dword ptr [r8 + r14*4 + 0x20f1208]
020f0e7d: add    rax, r8
020f0e80: jmp    rax
020f0e82: mov    edi, 1
020f0e87: movzx  esi, di
020f0e8a: jmp    0x1820f0ea1
020f0e8c: mov    edi, 1
020f0e91: mov    esi, edi
020f0e93: mov    dword ptr [rsp + 0x20], edi
020f0e97: jmp    0x1820f0ea5
020f0e99: mov    esi, r13d
020f0e9c: mov    edi, 1
020f0ea1: mov    dword ptr [rsp + 0x20], esi
020f0ea5: mov    dword ptr [rsp + 0xe8], r13d
020f0ead: mov    dword ptr [rsp + 0x24], r13d
020f0eb2: mov    dword ptr [rsp + 0x28], r13d
020f0eb7: mov    qword ptr [rsp + 0x58], r13
020f0ebc: xor    eax, eax
020f0ebe: mov    dword ptr [rsp + 0xd0], eax
020f0ec5: and    edx, 0x1f
020f0ec8: movzx  ecx, dl
020f0ecb: shl    di, cl
020f0ece: dec    di
020f0ed1: xor    di, si
020f0ed4: mov    dword ptr [rsp + 0x40], edi
020f0ed8: mov    r8, qword ptr [rip + 0x1be6001]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f0edf: mov    rdx, r15
020f0ee2: lea    rcx, [rsp + 0x60]
020f0ee7: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f0eec: movups xmm0, xmmword ptr [rsp + 0x60]
020f0ef1: movups xmmword ptr [rsp + 0x78], xmm0
020f0ef6: movsd  xmm1, qword ptr [rsp + 0x70]
020f0efc: movsd  qword ptr [rsp + 0x88], xmm1
020f0f05: mov    qword ptr [rsp + 0x60], r13
020f0f0a: lea    rbx, [rsp + 0x78]
020f0f0f: mov    qword ptr [rsp + 0x68], rbx
020f0f14: mov    r15d, 0xfe
020f0f1a: nop    word ptr [rax + rax]
020f0f20: mov    rdx, qword ptr [rip + 0x1bd1441]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f0f27: lea    rcx, [rsp + 0x78]
020f0f2c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f0f31: test   al, al
020f0f33: je     0x1820f116f
020f0f39: mov    r14, qword ptr [rsp + 0x88]
020f0f41: test   r14, r14
020f0f44: je     0x1820f11b8
020f0f4a: test   word ptr [r14 + 0x80], si
020f0f52: jne    0x1820f0f20
020f0f54: test   r12b, r12b
020f0f57: jne    0x1820f0f71
020f0f59: xor    edx, edx
020f0f5b: mov    rcx, r14
020f0f5e: call   0x18214b840                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʺʷʻʷʽʺʴʸʴʷʳ
020f0f63: test   al, al
020f0f65: jne    0x1820f0f20
020f0f67: cmp    word ptr [r14 + 0x80], di
020f0f6f: je     0x1820f0f20
020f0f71: movzx  edi, word ptr [r14 + 0x80]
020f0f79: mov    rcx, qword ptr [rip + 0x1ba78c8]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f0f80: cmp    dword ptr [rcx + 0xe0], 0
020f0f87: jne    0x1820f0f8e
020f0f89: call   0x182f60cf0
020f0f8e: xor    esi, esi
020f0f90: test   di, di
020f0f93: je     0x1820f0f9f
020f0f95: inc    esi
020f0f97: lea    eax, [rdi - 1]
020f0f9a: and    di, ax
020f0f9d: jne    0x1820f0f95
020f0f9f: test   r12b, r12b
020f0fa2: je     0x1820f0fb2
020f0fa4: cmp    r13, qword ptr [r14 + 0x40]
020f0fa8: jne    0x1820f0fb2
020f0faa: movzx  r13d, word ptr [rsp + 0x24]
020f0fb0: jmp    0x1820f0fcd
020f0fb2: xor    edx, edx
020f0fb4: mov    rcx, r14
020f0fb7: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f0fbc: movzx  r13d, ax
020f0fc0: and    r13w, r15w
020f0fc4: test   r12b, r12b
020f0fc7: je     0x1820f10ce
020f0fcd: mov    eax, dword ptr [rsp + 0x28]
020f0fd1: test   ax, ax
020f0fd4: je     0x1820f10ce
020f0fda: cmp    ax, r13w
020f0fde: jne    0x1820f10ce
020f0fe4: mov    rcx, qword ptr [rip + 0x1ba785d]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f0feb: cmp    dword ptr [rcx + 0xe0], 0
020f0ff2: jne    0x1820f0ff9
020f0ff4: call   0x182f60cf0
020f0ff9: xor    r15d, r15d
020f0ffc: xor    edi, edi
020f0ffe: movzx  ecx, r13w
020f1002: xor    edx, edx
020f1004: call   0x1820fe7e0                              ; ʹʾʽʼʷʲʴʸʷʼˁ$$ˁʿʸʲʺʳʴʳˁʼʾ
020f1009: mov    qword ptr [rsp + 0x50], rax
020f100e: xor    edx, edx
020f1010: lea    rcx, [rsp + 0x50]
020f1015: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
020f101a: mov    qword ptr [rsp + 0x48], rax
020f101f: nop    
020f1020: xor    edx, edx
020f1022: lea    rcx, [rsp + 0x48]
020f1027: call   0x1820fc610                              ; ʹʾʽʼʷʲʴʸʷʼˁ.ʶʽʶʷʺʾʼʵʳʶʽ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f102c: test   al, al
020f102e: je     0x1820f1086
020f1030: xor    edx, edx
020f1032: lea    rcx, [rsp + 0x48]
020f1037: call   0x1820fc5f0                              ; ʹʾʽʼʷʲʴʸʷʼˁ.ʶʽʶʷʺʾʼʵʳʶʽ$$ʳʳʷʹʺʸʶʹʹʶʾ
020f103c: movzx  ecx, word ptr [r14 + 0x80]
020f1044: cmp    eax, ecx
020f1046: je     0x1820f104c
020f1048: inc    edi
020f104a: jmp    0x1820f1020
020f104c: mov    rcx, qword ptr [rip + 0x1ba77f5]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f1053: cmp    dword ptr [rcx + 0xe0], r15d
020f105a: jne    0x1820f1061
020f105c: call   0x182f60cf0
020f1061: mov    eax, dword ptr [rsp + 0xe8]
020f1068: movzx  ecx, ax
020f106b: xor    edx, edx
020f106d: test   ax, ax
020f1070: je     0x1820f1080
020f1072: cmp    edx, edi
020f1074: je     0x1820f10c3
020f1076: lea    eax, [rcx - 1]
020f1079: inc    edx
020f107b: and    cx, ax
020f107e: jne    0x1820f1072
020f1080: xor    eax, eax
020f1082: movzx  r15d, ax
020f1086: mov    edi, dword ptr [rsp + 0xd0]
020f108d: mov    rax, qword ptr [rsp + 0x58]
020f1092: cmp    rax, qword ptr [r14 + 0x40]
020f1096: je     0x1820f1138
020f109c: mov    word ptr [rsp + 0xe8], di
020f10a4: movzx  eax, r15w
020f10a8: mov    dword ptr [rsp + 0xd0], eax
020f10af: mov    eax, dword ptr [rsp + 0x24]
020f10b3: mov    word ptr [rsp + 0x28], ax
020f10b8: mov    word ptr [rsp + 0x24], r13w
020f10be: jmp    0x1820f1151
020f10c3: movzx  eax, cx
020f10c6: neg    ax
020f10c9: and    ax, cx
020f10cc: jmp    0x1820f1082
020f10ce: mov    edi, dword ptr [rsp + 0xd0]
020f10d5: nop    word ptr [rax + rax]
020f10e0: xor    r8d, r8d
020f10e3: lea    rdx, [rsp + 0x30]
020f10e8: mov    ecx, esi
020f10ea: call   0x1820f0b30                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʾʷʳʷʼʵʴʹˀˁʼ
020f10ef: movzx  edx, ax
020f10f2: test   r12b, r12b
020f10f5: jne    0x1820f1106
020f10f7: cmp    word ptr [rsp + 0xe8], ax
020f10ff: je     0x1820f10e0
020f1101: test   r12b, r12b
020f1104: je     0x1820f1145
020f1106: movzx  r15d, dx
020f110a: movzx  ecx, di
020f110d: shr    cx, 3
020f1111: mov    eax, 0xfffd
020f1116: and    cx, ax
020f1119: or     cx, di
020f111c: and    cx, 0x1e
020f1120: movzx  eax, cx
020f1123: and    ax, 0x1c
020f1127: shl    ax, 3
020f112b: or     ax, cx
020f112e: test   dx, ax
020f1131: jne    0x1820f10e0
020f1133: jmp    0x1820f108d
020f1138: or     di, r15w
020f113c: mov    dword ptr [rsp + 0xd0], edi
020f1143: jmp    0x1820f1151
020f1145: mov    word ptr [rsp + 0xe8], dx
020f114d: movzx  r15d, dx
020f1151: mov    word ptr [r14 + 0x80], r15w
020f1159: mov    r13, qword ptr [r14 + 0x40]
020f115d: mov    qword ptr [rsp + 0x58], r13
020f1162: mov    edi, dword ptr [rsp + 0x40]
020f1166: mov    esi, dword ptr [rsp + 0x20]
020f116a: jmp    0x1820f0f14
020f116f: mov    rdx, qword ptr [rip + 0x1bd1132]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1176: mov    rcx, rbx
020f1179: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f117e: jmp    0x1820f119b
020f1180: mov    rdx, qword ptr [rip + 0x1bd1121]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f1187: mov    rcx, qword ptr [rsp + 0x68]
020f118c: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f1191: mov    rcx, qword ptr [rsp + 0x60]
020f1196: test   rcx, rcx
020f1199: jne    0x1820f11be
020f119b: lea    r11, [rsp + 0xa0]
020f11a3: mov    rbx, qword ptr [r11 + 0x38]
020f11a7: mov    rsi, qword ptr [r11 + 0x40]
020f11ab: mov    rsp, r11
020f11ae: pop    r15
020f11b0: pop    r14
020f11b2: pop    r13
020f11b4: pop    r12
020f11b6: pop    rdi
020f11b7: ret    
020f11b8: call   0x182f60c50
020f11bd: nop    
020f11be: call   0x182f60ce0
020f11c3: int3   
020f11c4: call   0x182f60c50
020f11c9: int3   
020f11ca: nop    
020f11cc: cmp    ecx, dword ptr [rsi]
020f11ce: lar    edi, word ptr [rbx]
