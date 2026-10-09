020f79e0: push   rbx
020f79e2: push   rsi
020f79e3: push   rdi
020f79e4: sub    rsp, 0xf0
020f79eb: cmp    byte ptr [rip + 0x1e219c6], 0            ; [0x3f193b8] (bss)
020f79f2: mov    rdi, rdx
020f79f5: mov    qword ptr [rsp + 0x120], r14
020f79fd: mov    rsi, rcx
020f7a00: mov    r14, r8
020f7a03: jne    0x1820f7a6c
020f7a05: lea    rcx, [rip + 0x1c16854]                   ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020f7a0c: call   0x182f609b0
020f7a11: lea    rcx, [rip + 0x1bf07d0]                   ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020f7a18: call   0x182f609b0
020f7a1d: lea    rcx, [rip + 0x1baad44]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f7a24: call   0x182f609b0
020f7a29: lea    rcx, [rip + 0x1c0a438]                   ; [0x3d01e68] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʻˀˀˁʶʲʹʵˀʷʲ<int>()
020f7a30: call   0x182f609b0
020f7a35: lea    rcx, [rip + 0x1b9b53c]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f7a3c: call   0x182f609b0
020f7a41: lea    rcx, [rip + 0x1ba05e8]                   ; [0x3c98030] meta:ʹʷʽʶʶʸʻʿˁʾʵ_TypeInfo
020f7a48: call   0x182f609b0
020f7a4d: lea    rcx, [rip + 0x1bb4274]                   ; [0x3cabcc8] meta:ʽʿʸʸʾʶʶʾʶʹʲ[]_TypeInfo
020f7a54: call   0x182f609b0
020f7a59: lea    rcx, [rip + 0x1ba6e20]                   ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
020f7a60: call   0x182f609b0
020f7a65: mov    byte ptr [rip + 0x1e2194c], 1            ; [0x3f193b8] (bss)
020f7a6c: movsd  xmm1, qword ptr [rip + 0x1048cdc]        ; [0x3140750] dbl=-2.0 q=0xc000000000000000
020f7a74: lea    rcx, [rsp + 0x20]
020f7a79: movabs rax, 0x3fb1eb851eb851ec
020f7a83: mov    dword ptr [rsi + 0x54], 0xffffffff
020f7a8a: xorps  xmm0, xmm0
020f7a8d: mov    qword ptr [rsi + 0x30], rax
020f7a91: xor    r9d, r9d
020f7a94: mov    dword ptr [rsi + 0x70], 0x3f000000
020f7a9b: xor    r8d, r8d
020f7a9e: mov    dword ptr [rsi + 0x74], 0x3be21965
020f7aa5: movups xmmword ptr [rsp + 0x20], xmm0
020f7aaa: mov    dword ptr [rsi + 0x78], 0x3ce2eb1c
020f7ab1: mov    dword ptr [rsi + 0x7c], 0x3d3851ec
020f7ab8: mov    dword ptr [rsi + 0xac], 0xffffffff
020f7ac2: call   0x182117120                              ; ʴʻʽʻʲʾʸʵʺʶʲ$$.ctor
020f7ac7: movups xmm0, xmmword ptr [rsp + 0x20]
020f7acc: movups xmmword ptr [rsi + 0x100], xmm0
020f7ad3: mov    rcx, qword ptr [rip + 0x1bf070e]         ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020f7ada: call   0x182f60c00
020f7adf: mov    rdx, qword ptr [rip + 0x1c1677a]         ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020f7ae6: mov    rcx, rax
020f7ae9: mov    rbx, rax
020f7aec: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020f7af1: lea    rcx, [rsi + 0x180]
020f7af8: mov    qword ptr [rsi + 0x180], rbx
020f7aff: mov    rdx, rbx
020f7b02: call   0x182f5fc00
020f7b07: mov    rcx, qword ptr [rip + 0x1bf06da]         ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020f7b0e: call   0x182f60c00
020f7b13: mov    rdx, qword ptr [rip + 0x1c16746]         ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020f7b1a: mov    rcx, rax
020f7b1d: mov    rbx, rax
020f7b20: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020f7b25: lea    rcx, [rsi + 0x188]
020f7b2c: mov    qword ptr [rsi + 0x188], rbx
020f7b33: mov    rdx, rbx
020f7b36: call   0x182f5fc00
020f7b3b: xor    edx, edx
020f7b3d: mov    rcx, rsi
020f7b40: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f7b45: mov    rcx, qword ptr [rip + 0x1ba6d34]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
020f7b4c: mov    rbx, qword ptr [r14 + 0x18]
020f7b50: cmp    dword ptr [rcx + 0xe0], 0
020f7b57: jne    0x1820f7b5e
020f7b59: call   0x182f60cf0
020f7b5e: mov    qword ptr [rsp + 0x118], rbp
020f7b66: xor    edx, edx
020f7b68: movaps xmmword ptr [rsp + 0xe0], xmm6
020f7b70: mov    rcx, rbx
020f7b73: movaps xmmword ptr [rsp + 0xd0], xmm7
020f7b7b: call   0x1818d8c10                              ; UnityEngine.ParticleSystem$$get_emission
020f7b80: lea    rcx, [rsi + 0x280]
020f7b87: mov    qword ptr [rsi + 0x280], rax
020f7b8e: xor    edx, edx
020f7b90: call   0x182f5fc00
020f7b95: mov    eax, dword ptr [r14 + 8]
020f7b99: lea    rcx, [rsi + 0x1a0]
020f7ba0: mov    dword ptr [rsi + 0x198], eax
020f7ba6: movzx  eax, byte ptr [r14 + 0xc]
020f7bab: mov    byte ptr [rsi + 0x194], al
020f7bb1: movzx  eax, byte ptr [r14 + 0xd]
020f7bb6: mov    byte ptr [rsi + 0x196], al
020f7bbc: movzx  eax, byte ptr [r14 + 0xe]
020f7bc1: mov    byte ptr [rsi + 0x195], al
020f7bc7: mov    eax, dword ptr [r14 + 0x10]
020f7bcb: mov    dword ptr [rsi + 0x190], eax
020f7bd1: mov    rdx, qword ptr [r14]
020f7bd4: mov    qword ptr [rsi + 0x1a0], rdx
020f7bdb: call   0x182f5fc00
020f7be0: movups xmm0, xmmword ptr [rdi]
020f7be3: lea    rcx, [rsi + 0x1e0]
020f7bea: xor    edx, edx
020f7bec: movups xmm1, xmmword ptr [rdi + 0x10]
020f7bf0: movups xmmword ptr [rsi + 0x1e0], xmm0
020f7bf7: movups xmm0, xmmword ptr [rdi + 0x20]
020f7bfb: movups xmmword ptr [rsi + 0x1f0], xmm1
020f7c02: movups xmm1, xmmword ptr [rdi + 0x30]
020f7c06: movups xmmword ptr [rsi + 0x200], xmm0
020f7c0d: movups xmm0, xmmword ptr [rdi + 0x40]
020f7c11: movups xmmword ptr [rsi + 0x210], xmm1
020f7c18: movups xmm1, xmmword ptr [rdi + 0x50]
020f7c1c: movups xmmword ptr [rsi + 0x220], xmm0
020f7c23: movups xmm0, xmmword ptr [rdi + 0x60]
020f7c27: movups xmmword ptr [rsi + 0x230], xmm1
020f7c2e: movups xmm1, xmmword ptr [rdi + 0x70]
020f7c32: movups xmmword ptr [rsi + 0x240], xmm0
020f7c39: movups xmm0, xmmword ptr [rdi + 0x80]
020f7c40: movups xmmword ptr [rsi + 0x250], xmm1
020f7c47: movups xmm1, xmmword ptr [rdi + 0x90]
020f7c4e: movups xmmword ptr [rsi + 0x260], xmm0
020f7c55: movups xmmword ptr [rsi + 0x270], xmm1
020f7c5c: call   0x182f5fc00
020f7c61: mov    rdi, qword ptr [rdi + 0x50]
020f7c65: mov    r14, qword ptr [rsp + 0x120]
020f7c6d: test   rdi, rdi
020f7c70: je     0x1820f7ffb
020f7c76: mov    rcx, qword ptr [rip + 0x1b9b2fb]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f7c7d: movsd  xmm6, qword ptr [rdi + 0xb8]
020f7c85: cmp    dword ptr [rcx + 0xe0], 0
020f7c8c: jne    0x1820f7c93
020f7c8e: call   0x182f60cf0
020f7c93: mov    rbx, qword ptr [rip + 0x1c0a1ce]         ; [0x3d01e68] metamethod:Method$ʴˀʽʺˀʲʲʻʷʾʻ.ʻˀˀˁʶʲʹʵˀʷʲ<int>()
020f7c9a: cvttsd2si eax, xmm6
020f7c9e: cmp    qword ptr [rbx + 0x38], 0
020f7ca3: mov    dword ptr [rsp + 0x110], eax
020f7caa: jne    0x1820f7cb4
020f7cac: mov    rcx, rbx
020f7caf: call   0x182f657d0
020f7cb4: mov    r8, qword ptr [rbx + 0x38]
020f7cb8: lea    rcx, [rsp + 0x110]
020f7cc0: mov    ebx, 0x19
020f7cc5: mov    edx, ebx
020f7cc7: mov    r8, qword ptr [r8 + 0x10]
020f7ccb: call   0x181b4e780                              ; System.Int32$$CompareTo
020f7cd0: test   eax, eax
020f7cd2: mov    eax, 0x51eb851f
020f7cd7: cmovg  ebx, dword ptr [rsp + 0x110]
020f7cdf: imul   ebx
020f7ce1: sar    edx, 3
020f7ce4: mov    eax, edx
020f7ce6: shr    eax, 0x1f
020f7ce9: add    edx, eax
020f7ceb: mov    dword ptr [rsi + 0x1a8], edx
020f7cf1: cvttsd2si rax, qword ptr [rdi + 0xb0]
020f7cfa: cqo    
020f7cfc: and    edx, 3
020f7cff: add    rax, rdx
020f7d02: sar    rax, 2
020f7d06: mov    qword ptr [rsi + 0x1c8], rax
020f7d0d: mov    rcx, qword ptr [rip + 0x1baaa54]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f7d14: movsd  xmm6, qword ptr [rdi + 0xb8]
020f7d1c: cmp    dword ptr [rcx + 0xe0], 0
020f7d23: jne    0x1820f7d2a
020f7d25: call   0x182f60cf0
020f7d2a: mulsd  xmm6, qword ptr [rip + 0x1048a16]        ; [0x3140748] dbl=7.5 q=0x401e000000000000
020f7d32: movaps xmm0, xmm6
020f7d35: call   0x182c49a60
020f7d3a: cvttsd2si rcx, xmm0
020f7d3f: mov    qword ptr [rsi + 0x1c0], rcx
020f7d46: lea    rax, [rcx + rcx]
020f7d4a: mov    qword ptr [rsi + 0x1b8], rax
020f7d51: lea    rax, [rcx*4]
020f7d59: mov    qword ptr [rsi + 0x1b0], rax
020f7d60: mov    rcx, qword ptr [rip + 0x1ba02c9]         ; [0x3c98030] meta:ʹʷʽʶʶʸʻʿˁʾʵ_TypeInfo
020f7d67: mov    rbx, qword ptr [rsi + 0x238]
020f7d6e: call   0x182f60c00
020f7d73: xor    r8d, r8d
020f7d76: mov    rdx, rbx
020f7d79: mov    rcx, rax
020f7d7c: mov    rdi, rax
020f7d7f: call   0x1820dc780                              ; ʹʷʽʶʶʸʻʿˁʾʵ$$.ctor
020f7d84: lea    rcx, [rsi + 0x1d8]
020f7d8b: mov    qword ptr [rsi + 0x1d8], rdi
020f7d92: mov    rdx, rdi
020f7d95: call   0x182f5fc00
020f7d9a: movzx  eax, byte ptr [rsi + 0x218]
020f7da1: cmp    al, 6
020f7da3: jne    0x1820f7da9
020f7da5: mov    dl, 1
020f7da7: jmp    0x1820f7dae
020f7da9: cmp    al, 9
020f7dab: sete   dl
020f7dae: mov    rax, qword ptr [rsi + 0x230]
020f7db5: test   rax, rax
020f7db8: je     0x1820f7ffb
020f7dbe: movsd  xmm2, qword ptr [rax + 0xb8]
020f7dc6: xor    r9d, r9d
020f7dc9: mov    rcx, rsi
020f7dcc: call   0x1820f4680                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʶʴˀʴʿʷʵʺʺʽʿ
020f7dd1: movsd  xmm1, qword ptr [rsi + 0x30]
020f7dd6: mov    edx, 0x20
020f7ddb: movsd  xmm0, qword ptr [rsi + 0x30]
020f7de0: addsd  xmm1, xmm1
020f7de4: xorps  xmm0, xmmword ptr [rip + 0xf6cb45]       ; [0x3064930] xmm dbl=(-0.0, -0.0) flt=(0.0, -0.0, 0.0, -0.0)
020f7deb: mov    dword ptr [rsi + 0x1d0], eax
020f7df1: mov    rax, qword ptr [rsi + 0x30]
020f7df5: movsd  qword ptr [rsi + 0x18], xmm0
020f7dfa: movsd  qword ptr [rsi + 0x20], xmm1
020f7dff: mov    dword ptr [rsi + 0x48], 1
020f7e06: mov    qword ptr [rsi + 0x10], rax
020f7e0a: mov    rcx, qword ptr [rip + 0x1bb3eb7]         ; [0x3cabcc8] meta:ʽʿʸʸʾʶʶʾʶʹʲ[]_TypeInfo
020f7e11: call   0x182f5fc80
020f7e16: lea    rcx, [rsi + 0x58]
020f7e1a: mov    qword ptr [rsi + 0x58], rax
020f7e1e: mov    rdx, rax
020f7e21: call   0x182f5fc00
020f7e26: movzx  ecx, byte ptr [rsi + 0x218]
020f7e2d: xor    ebx, ebx
020f7e2f: cmp    cl, 6
020f7e32: jne    0x1820f7e38
020f7e34: mov    eax, ebx
020f7e36: jmp    0x1820f7e3e
020f7e38: cmp    cl, 9
020f7e3b: setne  al
020f7e3e: cmp    byte ptr [rip + 0x1e21582], 0            ; [0x3f193c7] (bss)
020f7e45: mov    byte ptr [rsi + 0x19c], al
020f7e4b: jne    0x1820f7e78
020f7e4d: lea    rcx, [rip + 0x1baab9c]                   ; [0x3ca29f0] meta:int[]_TypeInfo
020f7e54: call   0x182f609b0
020f7e59: lea    rcx, [rip + 0x1b9b118]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f7e60: call   0x182f609b0
020f7e65: lea    rcx, [rip + 0x1ba5174]                   ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
020f7e6c: call   0x182f609b0
020f7e71: mov    byte ptr [rip + 0x1e2154f], 1            ; [0x3f193c7] (bss)
020f7e78: cmp    qword ptr [rsi + 0xf8], 0
020f7e80: jne    0x1820f7fa5
020f7e86: mov    rcx, qword ptr [rip + 0x1baab63]         ; [0x3ca29f0] meta:int[]_TypeInfo
020f7e8d: mov    edx, 9
020f7e92: call   0x182f5fc80
020f7e97: lea    rcx, [rsi + 0xf8]
020f7e9e: mov    qword ptr [rsi + 0xf8], rax
020f7ea5: mov    rdx, rax
020f7ea8: call   0x182f5fc00
020f7ead: mov    rax, qword ptr [rsi + 0xf8]
020f7eb4: mov    ecx, ebx
020f7eb6: test   rax, rax
020f7eb9: je     0x1820f7ffb
020f7ebf: nop    
020f7ec0: cmp    ecx, dword ptr [rax + 0x18]
020f7ec3: jge    0x1820f7fa5
020f7ec9: mov    rax, qword ptr [rip + 0x1ba5110]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
020f7ed0: mov    rdi, qword ptr [rsi + 0xf8]
020f7ed7: mov    ebp, dword ptr [rsi + 0x1d0]
020f7edd: cmp    dword ptr [rax + 0xe0], 0
020f7ee4: jne    0x1820f7ef5
020f7ee6: mov    rcx, rax
020f7ee9: call   0x182f60cf0
020f7eee: mov    rax, qword ptr [rip + 0x1ba50eb]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
020f7ef5: mov    rax, qword ptr [rax + 0xb8]
020f7efc: mov    rcx, qword ptr [rax]
020f7eff: test   rcx, rcx
020f7f02: je     0x1820f7ffb
020f7f08: cmp    ebx, dword ptr [rcx + 0x18]
020f7f0b: jae    0x1820f8001
020f7f11: movsxd rax, ebx
020f7f14: movss  xmm7, dword ptr [rcx + rax*4 + 0x20]
020f7f1a: mov    rcx, qword ptr [rip + 0x1b9b057]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f7f21: cmp    dword ptr [rcx + 0xe0], 0
020f7f28: jne    0x1820f7f2f
020f7f2a: call   0x182f60cf0
020f7f2f: cmp    byte ptr [rip + 0x1e13d46], 0            ; [0x3f0bc7c] (bss)
020f7f36: movd   xmm6, ebp
020f7f3a: cvtdq2ps xmm6, xmm6
020f7f3d: jne    0x1820f7f52
020f7f3f: lea    rcx, [rip + 0x1baa822]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f7f46: call   0x182f609b0
020f7f4b: mov    byte ptr [rip + 0x1e13d2a], 1            ; [0x3f0bc7c] (bss)
020f7f52: mov    rcx, qword ptr [rip + 0x1baa80f]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f7f59: cmp    dword ptr [rcx + 0xe0], 0
020f7f60: jne    0x1820f7f67
020f7f62: call   0x182f60cf0
020f7f67: test   rdi, rdi
020f7f6a: je     0x1820f7ffb
020f7f70: cmp    ebx, dword ptr [rdi + 0x18]
020f7f73: jae    0x1820f8001
020f7f79: mulss  xmm6, xmm7
020f7f7d: cvtps2pd xmm0, xmm6
020f7f80: call   0x183031130
020f7f85: movsxd rax, ebx
020f7f88: inc    ebx
020f7f8a: cvttsd2si ecx, xmm0
020f7f8e: mov    dword ptr [rdi + rax*4 + 0x20], ecx
020f7f92: mov    ecx, ebx
020f7f94: mov    rax, qword ptr [rsi + 0xf8]
020f7f9b: test   rax, rax
020f7f9e: je     0x1820f7ffb
020f7fa0: jmp    0x1820f7ec0
020f7fa5: mov    r8, qword ptr [rsi + 0x148]
020f7fac: xor    r9d, r9d
020f7faf: mov    rdx, qword ptr [rsi + 0x108]
020f7fb6: mov    rcx, qword ptr [rsi + 0x208]
020f7fbd: call   0x1820d4aa0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʽʿʽʹʿʲʹʴʸʹʳ
020f7fc2: lea    rcx, [rsi + 0x148]
020f7fc9: mov    qword ptr [rsi + 0x148], rax
020f7fd0: mov    rdx, rax
020f7fd3: call   0x182f5fc00
020f7fd8: movaps xmm7, xmmword ptr [rsp + 0xd0]
020f7fe0: movaps xmm6, xmmword ptr [rsp + 0xe0]
020f7fe8: mov    rbp, qword ptr [rsp + 0x118]
020f7ff0: add    rsp, 0xf0
020f7ff7: pop    rdi
020f7ff8: pop    rsi
020f7ff9: pop    rbx
020f7ffa: ret    
020f7ffb: call   0x182f60c50
020f8000: int3   
020f8001: call   0x182f60c40
020f8006: int3   
020f8007: int3   
