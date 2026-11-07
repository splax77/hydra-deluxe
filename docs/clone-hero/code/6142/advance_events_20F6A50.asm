020f6a50: mov    qword ptr [rsp + 0x10], rbx
020f6a55: push   rdi
020f6a56: sub    rsp, 0x80
020f6a5d: cmp    byte ptr [rip + 0x1e2294a], 0            ; [0x3f193ae] (bss)
020f6a64: movzx  edi, r8b
020f6a68: movaps xmmword ptr [rsp + 0x50], xmm8
020f6a6e: mov    rbx, rcx
020f6a71: movaps xmm8, xmm1
020f6a75: jne    0x1820f6ad2
020f6a77: lea    rcx, [rip + 0x1be0762]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f6a7e: call   0x182f609b0
020f6a83: lea    rcx, [rip + 0x1bdb8b6]                   ; [0x3cd2340] metamethod:Method$System.Collections.Generic.List<ʸʸʽˀʹʺʹʻʼʴʴ>.get_Count()
020f6a8a: call   0x182f609b0
020f6a8f: lea    rcx, [rip + 0x1bd64b2]                   ; [0x3cccf48] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Count()
020f6a96: call   0x182f609b0
020f6a9b: lea    rcx, [rip + 0x1bdb95e]                   ; [0x3cd2400] metamethod:Method$System.Collections.Generic.List<ʸʸʽˀʹʺʹʻʼʴʴ>.get_Item()
020f6aa2: call   0x182f609b0
020f6aa7: lea    rcx, [rip + 0x1be07f2]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f6aae: call   0x182f609b0
020f6ab3: lea    rcx, [rip + 0x1bd654e]                   ; [0x3ccd008] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Item()
020f6aba: call   0x182f609b0
020f6abf: lea    rcx, [rip + 0x1b9c4b2]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6ac6: call   0x182f609b0
020f6acb: mov    byte ptr [rip + 0x1e228dc], 1            ; [0x3f193ae] (bss)
020f6ad2: mov    qword ptr [rsp + 0x90], rsi
020f6ada: movaps xmmword ptr [rsp + 0x70], xmm6
020f6adf: movaps xmmword ptr [rsp + 0x60], xmm7
020f6ae4: movaps xmmword ptr [rsp + 0x40], xmm9
020f6aea: movaps xmmword ptr [rsp + 0x30], xmm10
020f6af0: movaps xmmword ptr [rsp + 0x20], xmm11
020f6af6: test   dil, dil
020f6af9: je     0x1820f6cb3
020f6aff: mov    rax, qword ptr [rbx + 0x1e0]
020f6b06: movsd  xmm10, qword ptr [rbx + 0x100]
020f6b0f: mov    esi, dword ptr [rbx + 0x50]
020f6b12: test   rax, rax
020f6b15: je     0x1820f6eaa
020f6b1b: movsd  xmm11, qword ptr [rip + 0xf6de0c]        ; [0x3064930] dbl=-0.0 q=0x8000000000000000
020f6b24: cmp    esi, dword ptr [rax + 0x18]
020f6b27: jge    0x1820f6cb3
020f6b2d: mov    rcx, qword ptr [rbx + 0x1e0]
020f6b34: test   rcx, rcx
020f6b37: je     0x1820f6eaa
020f6b3d: mov    r8, qword ptr [rip + 0x1be075c]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f6b44: mov    edx, esi
020f6b46: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f6b4b: xor    r8d, r8d
020f6b4e: mov    rdx, rax
020f6b51: mov    rcx, rbx
020f6b54: mov    rdi, rax
020f6b57: call   0x1820f3a90                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʴʼʶʿʴʲʶʵʼʳˁ
020f6b5c: movaps xmm2, xmm0
020f6b5f: movaps xmm9, xmm0
020f6b63: xorps  xmm2, xmm11
020f6b67: test   rdi, rdi
020f6b6a: je     0x1820f6eaa
020f6b70: movsd  xmm7, qword ptr [rdi + 0x28]
020f6b75: movsd  xmm1, qword ptr [rdi + 0x28]
020f6b7a: subsd  xmm7, xmm2
020f6b7e: subsd  xmm1, xmm7
020f6b82: comisd xmm1, xmm2
020f6b86: jbe    0x1820f6bb0
020f6b88: mov    rcx, qword ptr [rip + 0x1b9c3e9]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6b8f: cmp    dword ptr [rcx + 0xe0], 0
020f6b96: jne    0x1820f6b9d
020f6b98: call   0x182f60cf0
020f6b9d: xor    r8d, r8d
020f6ba0: mov    edx, 1
020f6ba5: movaps xmm0, xmm7
020f6ba8: call   0x180f96aa0
020f6bad: movaps xmm7, xmm0
020f6bb0: movsd  xmm6, qword ptr [rdi + 0x28]
020f6bb5: movsd  xmm0, qword ptr [rdi + 0x28]
020f6bba: subsd  xmm6, xmm9
020f6bbf: subsd  xmm0, xmm6
020f6bc3: comisd xmm0, xmm9
020f6bc8: jbe    0x1820f6bf2
020f6bca: mov    rcx, qword ptr [rip + 0x1b9c3a7]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6bd1: cmp    dword ptr [rcx + 0xe0], 0
020f6bd8: jne    0x1820f6bdf
020f6bda: call   0x182f60cf0
020f6bdf: xor    r8d, r8d
020f6be2: mov    edx, 1
020f6be7: movaps xmm0, xmm6
020f6bea: call   0x180f96aa0
020f6bef: movaps xmm6, xmm0
020f6bf2: comisd xmm7, xmm8
020f6bf7: jb     0x1820f6c04
020f6bf9: comisd xmm6, xmm8
020f6bfe: jae    0x1820f6cb3
020f6c04: comisd xmm6, xmmword ptr [rbx + 0x100]
020f6c0c: jbe    0x1820f6c1a
020f6c0e: comisd xmm8, xmm6
020f6c13: jbe    0x1820f6c1a
020f6c15: movaps xmm1, xmm6
020f6c18: jmp    0x1820f6c62
020f6c1a: comisd xmm10, xmm6
020f6c1f: jbe    0x1820f6c75
020f6c21: comisd xmm7, xmm10
020f6c26: jbe    0x1820f6c75
020f6c28: mov    rcx, qword ptr [rip + 0x1b9c349]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6c2f: cmp    dword ptr [rcx + 0xe0], 0
020f6c36: jne    0x1820f6c3d
020f6c38: call   0x182f60cf0
020f6c3d: xor    r8d, r8d
020f6c40: mov    edx, 1
020f6c45: movaps xmm0, xmm10
020f6c49: call   0x180f96aa0
020f6c4e: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6c56: jbe    0x1820f6c75
020f6c58: comisd xmm8, xmm0
020f6c5d: jbe    0x1820f6c75
020f6c5f: movaps xmm1, xmm0
020f6c62: mov    rax, qword ptr [rbx]
020f6c65: mov    rcx, rbx
020f6c68: mov    r8, qword ptr [rax + 0x240]
020f6c6f: call   qword ptr [rax + 0x238]
020f6c75: comisd xmm7, xmmword ptr [rbx + 0x100]
020f6c7d: jbe    0x1820f6c9c
020f6c7f: comisd xmm8, xmm7
020f6c84: jbe    0x1820f6c9c
020f6c86: mov    rax, qword ptr [rbx]
020f6c89: movaps xmm1, xmm7
020f6c8c: mov    rcx, rbx
020f6c8f: mov    r8, qword ptr [rax + 0x240]
020f6c96: call   qword ptr [rax + 0x238]
020f6c9c: mov    rax, qword ptr [rbx + 0x1e0]
020f6ca3: inc    esi
020f6ca5: test   rax, rax
020f6ca8: je     0x1820f6eaa
020f6cae: jmp    0x1820f6b24
020f6cb3: movsd  xmm0, qword ptr [rbx + 0x28]
020f6cb8: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6cc0: jbe    0x1820f6cdf
020f6cc2: comisd xmm8, xmm0
020f6cc7: jbe    0x1820f6cdf
020f6cc9: mov    rax, qword ptr [rbx]
020f6ccc: movaps xmm1, xmm0
020f6ccf: mov    rcx, rbx
020f6cd2: mov    r8, qword ptr [rax + 0x240]
020f6cd9: call   qword ptr [rax + 0x238]
020f6cdf: mov    rax, qword ptr [rbx + 0x148]
020f6ce6: test   rax, rax
020f6ce9: je     0x1820f6eaa
020f6cef: mov    rcx, qword ptr [rbx + 0x208]
020f6cf6: test   rcx, rcx
020f6cf9: je     0x1820f6eaa
020f6cff: mov    edx, dword ptr [rax + 0x20]
020f6d02: lea    eax, [rdx + 1]
020f6d05: cmp    dword ptr [rcx + 0x18], eax
020f6d08: jle    0x1820f6d4d
020f6d0a: mov    r8, qword ptr [rip + 0x1bd62f7]          ; [0x3ccd008] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.get_Item()
020f6d11: inc    edx
020f6d13: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f6d18: test   rax, rax
020f6d1b: je     0x1820f6eaa
020f6d21: movsd  xmm0, qword ptr [rax + 0x18]
020f6d26: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6d2e: jbe    0x1820f6d4d
020f6d30: comisd xmm8, xmm0
020f6d35: jbe    0x1820f6d4d
020f6d37: mov    rdx, qword ptr [rbx]
020f6d3a: movaps xmm1, xmm0
020f6d3d: mov    rcx, rbx
020f6d40: mov    r8, qword ptr [rdx + 0x240]
020f6d47: call   qword ptr [rdx + 0x238]
020f6d4d: cmp    byte ptr [rbx + 0x154], 0
020f6d54: jne    0x1820f6d93
020f6d56: mov    rax, qword ptr [rbx + 0x210]
020f6d5d: test   rax, rax
020f6d60: je     0x1820f6eaa
020f6d66: mov    eax, dword ptr [rax + 0x18]
020f6d69: cmp    dword ptr [rbx + 0x178], eax
020f6d6f: jge    0x1820f6e3a
020f6d75: mov    r8, qword ptr [rip + 0x1bdb684]          ; [0x3cd2400] metamethod:Method$System.Collections.Generic.List<ʸʸʽˀʹʺʹʻʼʴʴ>.get_Item()
020f6d7c: mov    edx, dword ptr [rbx + 0x178]
020f6d82: mov    rcx, qword ptr [rbx + 0x210]
020f6d89: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f6d8e: mov    rdi, rax
020f6d91: jmp    0x1820f6d9a
020f6d93: mov    rdi, qword ptr [rbx + 0x158]
020f6d9a: test   rdi, rdi
020f6d9d: je     0x1820f6e3a
020f6da3: cmp    byte ptr [rbx + 0x154], 0
020f6daa: jne    0x1820f6dd8
020f6dac: movsd  xmm0, qword ptr [rdi + 0x10]
020f6db1: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6db9: jbe    0x1820f6dd8
020f6dbb: comisd xmm8, xmm0
020f6dc0: jbe    0x1820f6dd8
020f6dc2: mov    rax, qword ptr [rbx]
020f6dc5: movaps xmm1, xmm0
020f6dc8: mov    rcx, rbx
020f6dcb: mov    r8, qword ptr [rax + 0x240]
020f6dd2: call   qword ptr [rax + 0x238]
020f6dd8: xor    edx, edx
020f6dda: mov    rcx, rdi
020f6ddd: call   0x182175a00                              ; ʸʸʽˀʹʺʹʻʼʴʴ$$ʴʹʹˁʶʷˁʳˁʼˁ
020f6de2: mov    rcx, qword ptr [rip + 0x1b9c18f]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6de9: movaps xmm6, xmm0
020f6dec: movsd  xmm7, qword ptr [rbx + 0x30]
020f6df1: cmp    dword ptr [rcx + 0xe0], 0
020f6df8: jne    0x1820f6dff
020f6dfa: call   0x182f60cf0
020f6dff: subsd  xmm6, xmm7
020f6e03: xor    r8d, r8d
020f6e06: mov    edx, 1
020f6e0b: movaps xmm0, xmm6
020f6e0e: call   0x180f96990
020f6e13: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6e1b: jbe    0x1820f6e3a
020f6e1d: comisd xmm8, xmm0
020f6e22: jbe    0x1820f6e3a
020f6e24: mov    rax, qword ptr [rbx]
020f6e27: movaps xmm1, xmm0
020f6e2a: mov    rcx, rbx
020f6e2d: mov    r8, qword ptr [rax + 0x240]
020f6e34: call   qword ptr [rax + 0x238]
020f6e3a: cmp    byte ptr [rbx + 0x42], 0
020f6e3e: je     0x1820f6e6f
020f6e40: movsd  xmm0, qword ptr [rbx + 0x120]
020f6e48: comisd xmm0, xmmword ptr [rbx + 0x100]
020f6e50: jbe    0x1820f6e6f
020f6e52: comisd xmm8, xmm0
020f6e57: jbe    0x1820f6e6f
020f6e59: mov    rax, qword ptr [rbx]
020f6e5c: movaps xmm1, xmm0
020f6e5f: mov    rcx, rbx
020f6e62: mov    r8, qword ptr [rax + 0x240]
020f6e69: call   qword ptr [rax + 0x238]
020f6e6f: movaps xmm11, xmmword ptr [rsp + 0x20]
020f6e75: movaps xmm10, xmmword ptr [rsp + 0x30]
020f6e7b: movaps xmm9, xmmword ptr [rsp + 0x40]
020f6e81: movaps xmm7, xmmword ptr [rsp + 0x60]
020f6e86: movaps xmm6, xmmword ptr [rsp + 0x70]
020f6e8b: mov    rsi, qword ptr [rsp + 0x90]
020f6e93: mov    rbx, qword ptr [rsp + 0x98]
020f6e9b: movaps xmm8, xmmword ptr [rsp + 0x50]
020f6ea1: add    rsp, 0x80
020f6ea8: pop    rdi
020f6ea9: ret    
020f6eaa: call   0x182f60c50
020f6eaf: int3   
