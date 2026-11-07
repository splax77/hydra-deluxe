020d2b30: push   rdi
020d2b32: push   r13
020d2b34: push   r14
020d2b36: sub    rsp, 0x60
020d2b3a: cmp    byte ptr [rip + 0x1e467d2], 0            ; [0x3f19313] (bss)
020d2b41: mov    rdi, r8
020d2b44: mov    r13, rdx
020d2b47: mov    r14, rcx
020d2b4a: jne    0x1820d2bcb
020d2b4c: lea    rcx, [rip + 0x1bfbcad]                   ; [0x3cce800] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.Add()
020d2b53: call   0x182f609b0
020d2b58: lea    rcx, [rip + 0x1bfbb21]                   ; [0x3cce680] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020d2b5f: call   0x182f609b0
020d2b64: lea    rcx, [rip + 0x1c04675]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d2b6b: call   0x182f609b0
020d2b70: lea    rcx, [rip + 0x1bcddf9]                   ; [0x3ca0970] metamethod:Method$System.Collections.Generic.List<long>.get_Count()
020d2b77: call   0x182f609b0
020d2b7c: lea    rcx, [rip + 0x1bcdea5]                   ; [0x3ca0a28] metamethod:Method$System.Collections.Generic.List<long>.get_Item()
020d2b83: call   0x182f609b0
020d2b88: lea    rcx, [rip + 0x1c04711]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2b8f: call   0x182f609b0
020d2b94: lea    rcx, [rip + 0x1c21b75]                   ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020d2b9b: call   0x182f609b0
020d2ba0: lea    rcx, [rip + 0x1bcfbc1]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020d2ba7: call   0x182f609b0
020d2bac: lea    rcx, [rip + 0x1bbf4ed]                   ; [0x3c920a0] meta:ʴʴʶʺʻˁʻʿʽʵʲ_TypeInfo
020d2bb3: call   0x182f609b0
020d2bb8: lea    rcx, [rip + 0x1bc5b29]                   ; [0x3c986e8] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʶʲˁʿʹʵˀʺʲˁʿ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d2bbf: call   0x182f609b0
020d2bc4: mov    byte ptr [rip + 0x1e46748], 1            ; [0x3f19313] (bss)
020d2bcb: mov    qword ptr [rsp + 0x80], rbx
020d2bd3: mov    qword ptr [rsp + 0x90], rbp
020d2bdb: mov    qword ptr [rsp + 0x98], rsi
020d2be3: mov    qword ptr [rsp + 0x58], r12
020d2be8: mov    qword ptr [rsp + 0x50], r15
020d2bed: test   r13, r13
020d2bf0: je     0x1820d2e8f
020d2bf6: test   rdi, rdi
020d2bf9: je     0x1820d2e8f
020d2bff: mov    rdi, qword ptr [rdi + 0x20]
020d2c03: mov    rcx, qword ptr [rip + 0x1c21b06]         ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020d2c0a: mov    rbx, qword ptr [r13 + 0x50]
020d2c0e: mov    qword ptr [rsp + 0x20], rdi
020d2c13: call   0x182f60c00
020d2c18: mov    rdx, qword ptr [rip + 0x1bfba61]         ; [0x3cce680] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020d2c1f: mov    rcx, rax
020d2c22: mov    r15, rax
020d2c25: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d2c2a: test   r14, r14
020d2c2d: je     0x1820d2c65
020d2c2f: cmp    dword ptr [r14 + 0x18], 0
020d2c34: jne    0x1820d2c65
020d2c36: mov    r12, qword ptr [rsp + 0x58]
020d2c3b: mov    rax, r15
020d2c3e: mov    r15, qword ptr [rsp + 0x50]
020d2c43: mov    rsi, qword ptr [rsp + 0x98]
020d2c4b: mov    rbp, qword ptr [rsp + 0x90]
020d2c53: mov    rbx, qword ptr [rsp + 0x80]
020d2c5b: add    rsp, 0x60
020d2c5f: pop    r14
020d2c61: pop    r13
020d2c63: pop    rdi
020d2c64: ret    
020d2c65: test   rbx, rbx
020d2c68: je     0x1820d2e8f
020d2c6e: movsd  xmm0, qword ptr [rbx + 0xb8]
020d2c76: mulsd  xmm0, qword ptr [rip + 0xf91332]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
020d2c7e: test   rdi, rdi
020d2c81: je     0x1820d2e8f
020d2c87: cvttsd2si rax, xmm0
020d2c8c: mov    ebx, 1
020d2c91: inc    rax
020d2c94: xor    ebp, ebp
020d2c96: mov    qword ptr [rsp + 0x30], rax
020d2c9b: mov    dword ptr [rsp + 0x88], ebp
020d2ca2: mov    eax, ebx
020d2ca4: cmp    ebx, dword ptr [rdi + 0x18]
020d2ca7: jge    0x1820d2c36
020d2ca9: mov    r8, qword ptr [rip + 0x1bcdd78]          ; [0x3ca0a28] metamethod:Method$System.Collections.Generic.List<long>.get_Item()
020d2cb0: lea    edx, [rbx - 1]
020d2cb3: mov    rcx, rdi
020d2cb6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2cbb: mov    r8, qword ptr [rip + 0x1bcdd66]          ; [0x3ca0a28] metamethod:Method$System.Collections.Generic.List<long>.get_Item()
020d2cc2: mov    edx, ebx
020d2cc4: mov    rcx, rdi
020d2cc7: mov    r12, rax
020d2cca: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2ccf: mov    r8, qword ptr [rip + 0x1bcdd52]          ; [0x3ca0a28] metamethod:Method$System.Collections.Generic.List<long>.get_Item()
020d2cd6: lea    edx, [rbx - 1]
020d2cd9: mov    rcx, rdi
020d2cdc: mov    qword ptr [rsp + 0x40], rax
020d2ce1: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2ce6: mov    r8, qword ptr [rip + 0x1bcdd3b]          ; [0x3ca0a28] metamethod:Method$System.Collections.Generic.List<long>.get_Item()
020d2ced: mov    edx, ebx
020d2cef: mov    rcx, rdi
020d2cf2: mov    qword ptr [rsp + 0x38], rax
020d2cf7: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2cfc: mov    r9, qword ptr [rip + 0x1bc59e5]          ; [0x3c986e8] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʶʲˁʿʹʵˀʺʲˁʿ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d2d03: mov    r8d, ebp
020d2d06: mov    rdx, rax
020d2d09: mov    qword ptr [rsp + 0x28], rax
020d2d0e: mov    rcx, r14
020d2d11: call   0x1805ddb50                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ʶʲˁʿʹʵˀʺʲˁʿ<object>
020d2d16: mov    ebp, eax
020d2d18: cmp    eax, -1
020d2d1b: je     0x1820d2e7d
020d2d21: test   r14, r14
020d2d24: je     0x1820d2e8f
020d2d2a: mov    r8, qword ptr [rip + 0x1c0456f]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2d31: mov    edx, eax
020d2d33: mov    rcx, r14
020d2d36: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2d3b: mov    rsi, rax
020d2d3e: test   rax, rax
020d2d41: je     0x1820d2e8f
020d2d47: mov    rcx, qword ptr [rip + 0x1bcfa1a]         ; [0x3ca2768] meta:System.Math_TypeInfo
020d2d4e: mov    rdi, qword ptr [rax + 0x40]
020d2d52: cmp    dword ptr [rcx + 0xe0], 0
020d2d59: jne    0x1820d2d60
020d2d5b: call   0x182f60cf0
020d2d60: sub    rdi, qword ptr [rsp + 0x28]
020d2d65: mov    rax, rdi
020d2d68: neg    rax
020d2d6b: cmovs  rax, rdi
020d2d6f: cmp    rax, qword ptr [rsp + 0x30]
020d2d74: jg     0x1820d2e66
020d2d7a: xor    r8d, r8d
020d2d7d: mov    dword ptr [rsp + 0x88], ebp
020d2d84: mov    edx, 0xffffffff
020d2d89: mov    rcx, rsi
020d2d8c: call   0x1820d51d0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ˀʶˀʾʴʳʹʶˁʹʹ
020d2d91: mov    rsi, rax
020d2d94: test   rax, rax
020d2d97: je     0x1820d2e59
020d2d9d: or     dword ptr [rax + 0x7c], 0x800
020d2da4: mov    rcx, qword ptr [rip + 0x1bbf2f5]         ; [0x3c920a0] meta:ʴʴʶʺʻˁʻʿʽʵʲ_TypeInfo
020d2dab: call   0x182f60c00
020d2db0: xor    edx, edx
020d2db2: mov    rcx, rax
020d2db5: mov    rdi, rax
020d2db8: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d2dbd: mov    rcx, qword ptr [r13 + 0x50]
020d2dc1: test   rcx, rcx
020d2dc4: je     0x1820d2e8f
020d2dca: sub    r12, qword ptr [rsp + 0x38]
020d2dcf: xor    r8d, r8d
020d2dd2: mov    rbp, qword ptr [rsp + 0x40]
020d2dd7: add    rbp, r12
020d2dda: mov    rdx, rbp
020d2ddd: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d2de2: test   rdi, rdi
020d2de5: je     0x1820d2e8f
020d2deb: movsd  qword ptr [rdi + 0x10], xmm0
020d2df0: mov    rcx, qword ptr [r13 + 0x50]
020d2df4: test   rcx, rcx
020d2df7: je     0x1820d2e8f
020d2dfd: mov    r8, qword ptr [rsi + 0x40]
020d2e01: xor    r9d, r9d
020d2e04: mov    rdx, rbp
020d2e07: call   0x1820cfce0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴʸˁʳˀʽʼʺʸʾˁ
020d2e0c: xorps  xmm1, xmm1
020d2e0f: mov    qword ptr [rdi + 0x20], rsi
020d2e13: cvtsd2ss xmm1, xmm0
020d2e17: lea    rcx, [rdi + 0x20]
020d2e1b: mov    rdx, rsi
020d2e1e: movss  dword ptr [rdi + 0x18], xmm1
020d2e23: call   0x182f5fc00
020d2e28: mov    qword ptr [rdi + 0x28], rbp
020d2e2c: mov    eax, dword ptr [rsi + 0x40]
020d2e2f: sub    eax, ebp
020d2e31: mov    byte ptr [rdi + 0x38], 1
020d2e35: cdqe   
020d2e37: mov    qword ptr [rdi + 0x30], rax
020d2e3b: test   r15, r15
020d2e3e: je     0x1820d2e8f
020d2e40: mov    r8, qword ptr [rip + 0x1bfb9b9]          ; [0x3cce800] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.Add()
020d2e47: mov    rdx, rdi
020d2e4a: mov    rcx, r15
020d2e4d: call   0x182c425f0
020d2e52: mov    ebp, dword ptr [rsp + 0x88]
020d2e59: mov    rdi, qword ptr [rsp + 0x20]
020d2e5e: add    ebx, 4
020d2e61: jmp    0x1820d2ca2
020d2e66: mov    ebp, dword ptr [rsp + 0x88]
020d2e6d: sub    ebx, 3
020d2e70: mov    rdi, qword ptr [rsp + 0x20]
020d2e75: add    ebx, 4
020d2e78: jmp    0x1820d2ca2
020d2e7d: mov    ebp, dword ptr [rsp + 0x88]
020d2e84: sub    ebx, 3
020d2e87: add    ebx, 4
020d2e8a: jmp    0x1820d2ca2
020d2e8f: call   0x182f60c50
020d2e94: int3   
020d2e95: int3   
