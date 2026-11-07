020d18f0: mov    qword ptr [rsp + 0x10], rdx
020d18f5: push   rbp
020d18f6: push   rdi
020d18f7: push   r12
020d18f9: lea    rbp, [rsp - 0x47]
020d18fe: sub    rsp, 0x90
020d1905: cmp    byte ptr [rip + 0x1e479fd], 0            ; [0x3f19309] (bss)
020d190c: mov    rdi, rdx
020d190f: mov    r12, rcx
020d1912: jne    0x1820d194b
020d1914: lea    rcx, [rip + 0x1c058c5]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d191b: call   0x182f609b0
020d1920: lea    rcx, [rip + 0x1c00299]                   ; [0x3cd1bc0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Count()
020d1927: call   0x182f609b0
020d192c: lea    rcx, [rip + 0x1c0596d]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1933: call   0x182f609b0
020d1938: lea    rcx, [rip + 0x1c00341]                   ; [0x3cd1c80] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Item()
020d193f: call   0x182f609b0
020d1944: mov    byte ptr [rip + 0x1e479be], 1            ; [0x3f19309] (bss)
020d194b: mov    qword ptr [rsp + 0xb0], rbx
020d1953: xor    eax, eax
020d1955: mov    qword ptr [rsp + 0x88], rsi
020d195d: xorps  xmm0, xmm0
020d1960: mov    qword ptr [rsp + 0x80], r13
020d1968: xorps  xmm1, xmm1
020d196b: mov    qword ptr [rsp + 0x78], r14
020d1970: mov    qword ptr [rsp + 0x70], r15
020d1975: mov    qword ptr [rbp - 0x19], rax
020d1979: mov    qword ptr [rbp - 1], rax
020d197d: movups xmmword ptr [rbp - 0x29], xmm0
020d1981: movups xmmword ptr [rbp - 0x11], xmm1
020d1985: test   rdi, rdi
020d1988: je     0x1820d1bfa
020d198e: cmp    byte ptr [rdi + 0x58], 6
020d1992: jne    0x1820d199c
020d1994: mov    r13d, 1
020d199a: jmp    0x1820d19a4
020d199c: cmp    byte ptr [rdi + 0x58], 9
020d19a0: sete   r13b
020d19a4: xor    esi, esi
020d19a6: xor    ebx, ebx
020d19a8: xor    ecx, ecx
020d19aa: mov    dword ptr [rbp + 0x7f], ebx
020d19ad: nop    dword ptr [rax]
020d19b0: mov    rax, qword ptr [rdi + 0x78]
020d19b4: test   rax, rax
020d19b7: je     0x1820d1bfa
020d19bd: cmp    ecx, dword ptr [rax + 0x18]
020d19c0: jge    0x1820d1b83
020d19c6: mov    r8, qword ptr [rip + 0x1c002b3]          ; [0x3cd1c80] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Item()
020d19cd: mov    edx, ebx
020d19cf: mov    rcx, rax
020d19d2: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d19d7: mov    r14, rax
020d19da: nop    word ptr [rax + rax]
020d19e0: test   r12, r12
020d19e3: je     0x1820d1bfa
020d19e9: xor    r15d, r15d
020d19ec: xor    edi, edi
020d19ee: nop    
020d19f0: cmp    esi, dword ptr [r12 + 0x18]
020d19f5: jge    0x1820d1a56
020d19f7: mov    r8, qword ptr [rip + 0x1c058a2]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d19fe: mov    edx, esi
020d1a00: mov    rcx, r12
020d1a03: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1a08: mov    rbx, rax
020d1a0b: test   rax, rax
020d1a0e: je     0x1820d1bfa
020d1a14: test   r14, r14
020d1a17: je     0x1820d1bfa
020d1a1d: mov    rax, qword ptr [r14 + 0x10]
020d1a21: cmp    qword ptr [rbx + 0x40], rax
020d1a25: jl     0x1820d1a52
020d1a27: xor    edx, edx
020d1a29: mov    rcx, rbx
020d1a2c: call   0x18214bcc0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿˀʼʷʲʸʶʾˀʷʹ
020d1a31: test   al, al
020d1a33: je     0x1820d1a3a
020d1a35: test   r13b, r13b
020d1a38: je     0x1820d1a52
020d1a3a: mov    rax, qword ptr [r14 + 0x18]
020d1a3e: add    rax, qword ptr [r14 + 0x10]
020d1a42: cmp    qword ptr [rbx + 0x40], rax
020d1a46: jge    0x1820d1a56
020d1a48: test   rdi, rdi
020d1a4b: cmove  rdi, rbx
020d1a4f: inc    r15d
020d1a52: inc    esi
020d1a54: jmp    0x1820d19f0
020d1a56: test   r15d, r15d
020d1a59: je     0x1820d1bb1
020d1a5f: test   rdi, rdi
020d1a62: je     0x1820d1bfa
020d1a68: xor    r8d, r8d
020d1a6b: lea    rcx, [rbp + 7]
020d1a6f: mov    rdx, rdi
020d1a72: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d1a77: xor    r8d, r8d
020d1a7a: lea    rdx, [rbp - 0x11]
020d1a7e: lea    rcx, [rbp + 7]
020d1a82: movups xmm0, xmmword ptr [rax]
020d1a85: movups xmmword ptr [rbp - 0x11], xmm0
020d1a89: movsd  xmm1, qword ptr [rax + 0x10]
020d1a8e: movsd  qword ptr [rbp - 1], xmm1
020d1a93: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d1a98: xor    edx, edx
020d1a9a: lea    rcx, [rbp - 0x29]
020d1a9e: movups xmm0, xmmword ptr [rax]
020d1aa1: movups xmmword ptr [rbp - 0x29], xmm0
020d1aa5: movsd  xmm1, qword ptr [rax + 0x10]
020d1aaa: movsd  qword ptr [rbp - 0x19], xmm1
020d1aaf: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d1ab4: test   al, al
020d1ab6: je     0x1820d1ae3
020d1ab8: nop    dword ptr [rax + rax]
020d1ac0: mov    rcx, qword ptr [rbp - 0x21]
020d1ac4: test   rcx, rcx
020d1ac7: je     0x1820d1bfa
020d1acd: or     dword ptr [rcx + 0x7c], 0x200
020d1ad4: xor    edx, edx
020d1ad6: lea    rcx, [rbp - 0x29]
020d1ada: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d1adf: test   al, al
020d1ae1: jne    0x1820d1ac0
020d1ae3: mov    r8, qword ptr [rip + 0x1c057b6]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1aea: lea    edx, [rsi - 1]
020d1aed: mov    rcx, r12
020d1af0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1af5: test   rax, rax
020d1af8: je     0x1820d1bfa
020d1afe: xor    r8d, r8d
020d1b01: lea    rcx, [rbp + 7]
020d1b05: mov    rdx, rax
020d1b08: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d1b0d: xor    r8d, r8d
020d1b10: lea    rdx, [rbp - 0x11]
020d1b14: lea    rcx, [rbp + 7]
020d1b18: movups xmm0, xmmword ptr [rax]
020d1b1b: movups xmmword ptr [rbp - 0x11], xmm0
020d1b1f: movsd  xmm1, qword ptr [rax + 0x10]
020d1b24: movsd  qword ptr [rbp - 1], xmm1
020d1b29: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d1b2e: xor    edx, edx
020d1b30: lea    rcx, [rbp - 0x29]
020d1b34: movups xmm0, xmmword ptr [rax]
020d1b37: movups xmmword ptr [rbp - 0x29], xmm0
020d1b3b: movsd  xmm1, qword ptr [rax + 0x10]
020d1b40: movsd  qword ptr [rbp - 0x19], xmm1
020d1b45: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d1b4a: test   al, al
020d1b4c: je     0x1820d1b73
020d1b4e: nop    
020d1b50: mov    rcx, qword ptr [rbp - 0x21]
020d1b54: test   rcx, rcx
020d1b57: je     0x1820d1bfa
020d1b5d: or     dword ptr [rcx + 0x7c], 0x400
020d1b64: xor    edx, edx
020d1b66: lea    rcx, [rbp - 0x29]
020d1b6a: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d1b6f: test   al, al
020d1b71: jne    0x1820d1b50
020d1b73: mov    ebx, dword ptr [rbp + 0x7f]
020d1b76: mov    rdi, qword ptr [rbp + 0x6f]
020d1b7a: inc    ebx
020d1b7c: mov    ecx, ebx
020d1b7e: jmp    0x1820d19aa
020d1b83: mov    r15, qword ptr [rsp + 0x70]
020d1b88: mov    r14, qword ptr [rsp + 0x78]
020d1b8d: mov    r13, qword ptr [rsp + 0x80]
020d1b95: mov    rsi, qword ptr [rsp + 0x88]
020d1b9d: mov    rbx, qword ptr [rsp + 0xb0]
020d1ba5: add    rsp, 0x90
020d1bac: pop    r12
020d1bae: pop    rdi
020d1baf: pop    rbp
020d1bb0: ret    
020d1bb1: lea    rcx, [rip + 0x1c144b8]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020d1bb8: call   0x182f609d0
020d1bbd: mov    rcx, rax
020d1bc0: call   0x182f60c00
020d1bc5: lea    rcx, [rip + 0x1c0f164]                   ; [0x3ce0d30] str:'Solo sections should always have atleast 1 note!'
020d1bcc: mov    rbx, rax
020d1bcf: call   0x182f609d0
020d1bd4: mov    rdx, rax
020d1bd7: xor    r8d, r8d
020d1bda: mov    rcx, rbx
020d1bdd: call   0x181b92e30                              ; System.Exception$$.ctor
020d1be2: lea    rcx, [rip + 0x1c2a797]                   ; [0x3cfc380] metamethod:Method$ʲʽʶʳʺʹʳˀʾʴʹ.ʷʴʹʷʷʶˁʷʷʿʸ()
020d1be9: call   0x182f609d0
020d1bee: mov    rdx, rax
020d1bf1: mov    rcx, rbx
020d1bf4: call   0x182f60c10
020d1bf9: int3   
020d1bfa: call   0x182f60c50
020d1bff: int3   
020d1c00: push   rbp
020d1c02: sub    rsp, 0x30
020d1c06: cmp    byte ptr [rip + 0x1e476fe], 0            ; [0x3f1930b] (bss)
020d1c0d: mov    rbp, rcx
020d1c10: jne    0x1820d1c31
020d1c12: lea    rcx, [rip + 0x1c055c7]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d1c19: call   0x182f609b0
020d1c1e: lea    rcx, [rip + 0x1c0567b]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1c25: call   0x182f609b0
020d1c2a: mov    byte ptr [rip + 0x1e476da], 1            ; [0x3f1930b] (bss)
020d1c31: mov    qword ptr [rsp + 0x40], rbx
020d1c36: mov    qword ptr [rsp + 0x48], rsi
020d1c3b: mov    qword ptr [rsp + 0x50], rdi
020d1c40: mov    qword ptr [rsp + 0x58], r14
020d1c45: movaps xmmword ptr [rsp + 0x20], xmm6
020d1c4a: test   rbp, rbp
020d1c4d: je     0x1820d1d0e
020d1c53: xor    r14d, r14d
020d1c56: xorps  xmm6, xmm6
020d1c59: xor    eax, eax
020d1c5b: nop    dword ptr [rax + rax]
020d1c60: cmp    eax, dword ptr [rbp + 0x18]
020d1c63: jge    0x1820d1cef
020d1c69: mov    r8, qword ptr [rip + 0x1c05630]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1c70: mov    edx, r14d
020d1c73: mov    rcx, rbp
020d1c76: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1c7b: mov    rdi, rax
020d1c7e: test   rax, rax
020d1c81: je     0x1820d1d0e
020d1c87: movss  xmm0, dword ptr [rax + 0x38]
020d1c8c: ucomiss xmm0, xmm6
020d1c8f: jp     0x1820d1c93
020d1c91: je     0x1820d1ce4
020d1c93: mov    rsi, qword ptr [rax + 0x48]
020d1c97: lea    ebx, [r14 + 1]
020d1c9b: add    rsi, qword ptr [rax + 0x40]
020d1c9f: nop    
020d1ca0: cmp    ebx, dword ptr [rbp + 0x18]
020d1ca3: jge    0x1820d1ce4
020d1ca5: mov    r8, qword ptr [rip + 0x1c055f4]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1cac: mov    edx, ebx
020d1cae: mov    rcx, rbp
020d1cb1: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1cb6: test   rax, rax
020d1cb9: je     0x1820d1d0e
020d1cbb: cmp    qword ptr [rax + 0x40], rsi
020d1cbf: jg     0x1820d1ce4
020d1cc1: mov    rcx, qword ptr [rdi + 0x40]
020d1cc5: cmp    qword ptr [rax + 0x40], rcx
020d1cc9: jle    0x1820d1cd1
020d1ccb: cmp    qword ptr [rax + 0x40], rsi
020d1ccf: jl     0x1820d1cd8
020d1cd1: cmp    qword ptr [rax + 0x48], 0
020d1cd6: jne    0x1820d1ce0
020d1cd8: or     dword ptr [rdi + 0x7c], 8
020d1cdc: or     dword ptr [rax + 0x7c], 8
020d1ce0: inc    ebx
020d1ce2: jmp    0x1820d1ca0
020d1ce4: inc    r14d
020d1ce7: mov    eax, r14d
020d1cea: jmp    0x1820d1c60
020d1cef: movaps xmm6, xmmword ptr [rsp + 0x20]
020d1cf4: mov    r14, qword ptr [rsp + 0x58]
020d1cf9: mov    rdi, qword ptr [rsp + 0x50]
020d1cfe: mov    rsi, qword ptr [rsp + 0x48]
020d1d03: mov    rbx, qword ptr [rsp + 0x40]
020d1d08: add    rsp, 0x30
020d1d0c: pop    rbp
020d1d0d: ret    
020d1d0e: call   0x182f60c50
020d1d13: int3   
020d1d14: int3   
