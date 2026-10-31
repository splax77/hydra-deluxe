002df7f0: mov    qword ptr [rsp + 0x20], rbx
002df7f5: push   r14
002df7f7: sub    rsp, 0x50
002df7fb: cmp    byte ptr [rip + 0x3c2e129], 0            ; [0x3f0d92b] (bss)
002df802: mov    r14d, edx
002df805: mov    rbx, rcx
002df808: jne    0x1802df81d
002df80a: lea    rcx, [rip + 0x39bd7cf]                   ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
002df811: call   0x182f609b0
002df816: mov    byte ptr [rip + 0x3c2e10e], 1            ; [0x3f0d92b] (bss)
002df81d: cmp    byte ptr [rbx + 0x88], 0
002df824: mov    qword ptr [rsp + 0x60], rbp
002df829: mov    qword ptr [rsp + 0x68], rsi
002df82e: mov    qword ptr [rsp + 0x70], rdi
002df833: movaps xmmword ptr [rsp + 0x40], xmm6
002df838: movaps xmmword ptr [rsp + 0x30], xmm7
002df83d: je     0x1802df9f9
002df843: mov    rcx, qword ptr [rbx + 0x38]
002df847: xor    esi, esi
002df849: mov    dword ptr [rbx + 0x58], esi
002df84c: mov    qword ptr [rbx + 0x60], rsi
002df850: mov    dword ptr [rbx + 0x7c], 0xffffffff
002df857: test   rcx, rcx
002df85a: je     0x1802dfc13
002df860: mov    rdi, qword ptr [rbx + 0x30]
002df864: xor    r8d, r8d
002df867: xor    edx, edx
002df869: call   0x182885020                              ; UnityEngine.SpriteRenderer$$set_sprite
002df86e: test   rdi, rdi
002df871: je     0x1802dfc13
002df877: xor    r8d, r8d
002df87a: xor    edx, edx
002df87c: mov    rcx, rdi
002df87f: call   0x182885020                              ; UnityEngine.SpriteRenderer$$set_sprite
002df884: mov    rax, qword ptr [rbx + 0x80]
002df88b: test   rax, rax
002df88e: je     0x1802dfc13
002df894: cmp    dword ptr [rax + 0x18], 1
002df898: mov    rdx, rax
002df89b: je     0x1802df9b8
002df8a1: mov    r9d, dword ptr [rdx + 0x18]
002df8a5: mov    ebp, esi
002df8a7: mov    ecx, esi
002df8a9: mov    eax, esi
002df8ab: nop    dword ptr [rax + rax]
002df8b0: cmp    eax, r9d
002df8b3: jge    0x1802df8d8
002df8b5: cmp    ecx, r9d
002df8b8: jae    0x1802dfc0d
002df8be: movsxd rax, ecx
002df8c1: mov    r8, qword ptr [rdx + rax*8 + 0x20]
002df8c6: test   r8, r8
002df8c9: je     0x1802df8d2
002df8cb: add    ebp, dword ptr [r8 + 0x1d0]
002df8d2: inc    ecx
002df8d4: mov    eax, ecx
002df8d6: jmp    0x1802df8b0
002df8d8: mov    rax, qword ptr [rbx + 0x50]
002df8dc: mov    edi, esi
002df8de: mov    ecx, esi
002df8e0: test   rax, rax
002df8e3: je     0x1802dfc13
002df8e9: nop    dword ptr [rax]
002df8f0: cmp    ecx, dword ptr [rax + 0x18]
002df8f3: jge    0x1802df9f2
002df8f9: mov    rax, qword ptr [rip + 0x39bd6e0]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
002df900: mov    rsi, qword ptr [rbx + 0x50]
002df904: cmp    dword ptr [rax + 0xe0], 0
002df90b: jne    0x1802df91c
002df90d: mov    rcx, rax
002df910: call   0x182f60cf0
002df915: mov    rax, qword ptr [rip + 0x39bd6c4]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
002df91c: mov    rax, qword ptr [rax + 0xb8]
002df923: mov    rcx, qword ptr [rax]
002df926: test   rcx, rcx
002df929: je     0x1802dfc13
002df92f: cmp    edi, dword ptr [rcx + 0x18]
002df932: jae    0x1802dfc0d
002df938: cmp    byte ptr [rip + 0x3c2c572], 0            ; [0x3f0beb1] (bss)
002df93f: movsxd rax, edi
002df942: movd   xmm7, ebp
002df946: cvtdq2ps xmm7, xmm7
002df949: movss  xmm6, dword ptr [rcx + rax*4 + 0x20]
002df94f: jne    0x1802df964
002df951: lea    rcx, [rip + 0x39c2e10]                   ; [0x3ca2768] meta:System.Math_TypeInfo
002df958: call   0x182f609b0
002df95d: mov    byte ptr [rip + 0x3c2c54d], 1            ; [0x3f0beb1] (bss)
002df964: mov    rcx, qword ptr [rip + 0x39c2dfd]         ; [0x3ca2768] meta:System.Math_TypeInfo
002df96b: cmp    dword ptr [rcx + 0xe0], 0
002df972: jne    0x1802df979
002df974: call   0x182f60cf0
002df979: test   rsi, rsi
002df97c: je     0x1802dfc13
002df982: cmp    edi, dword ptr [rsi + 0x18]
002df985: jae    0x1802dfc0d
002df98b: mulss  xmm6, xmm7
002df98f: cvtps2pd xmm0, xmm6
002df992: call   0x183031130
002df997: movsxd rax, edi
002df99a: inc    edi
002df99c: cvttsd2si ecx, xmm0
002df9a0: mov    dword ptr [rsi + rax*4 + 0x20], ecx
002df9a4: mov    ecx, edi
002df9a6: mov    rax, qword ptr [rbx + 0x50]
002df9aa: test   rax, rax
002df9ad: je     0x1802dfc13
002df9b3: jmp    0x1802df8f0
002df9b8: cmp    dword ptr [rax + 0x18], esi
002df9bb: jbe    0x1802dfc0d
002df9c1: mov    rcx, qword ptr [rax + 0x20]
002df9c5: test   rcx, rcx
002df9c8: je     0x1802dfc13
002df9ce: mov    r8, qword ptr [rbx + 0x50]
002df9d2: test   r8, r8
002df9d5: je     0x1802dfc13
002df9db: mov    r8d, dword ptr [r8 + 0x18]
002df9df: xor    r9d, r9d
002df9e2: mov    rdx, qword ptr [rbx + 0x50]
002df9e6: mov    rcx, qword ptr [rcx + 0xf8]
002df9ed: call   0x181b64830                              ; System.Array$$Copy
002df9f2: mov    byte ptr [rbx + 0x88], 0
002df9f9: mov    eax, dword ptr [rbx + 0x5c]
002df9fc: cmp    dword ptr [rbx + 0x58], eax
002df9ff: je     0x1802dfb8b
002dfa05: cmp    r14d, dword ptr [rbx + 0x7c]
002dfa09: je     0x1802dfb8b
002dfa0f: mov    rdi, qword ptr [rbx + 0x50]
002dfa13: test   rdi, rdi
002dfa16: je     0x1802dfc13
002dfa1c: movsxd rax, dword ptr [rbx + 0x58]
002dfa20: cmp    eax, dword ptr [rdi + 0x18]
002dfa23: jae    0x1802dfc0d
002dfa29: mov    edi, dword ptr [rdi + rax*4 + 0x20]
002dfa2d: mov    eax, dword ptr [rbx + 0x5c]
002dfa30: mov    dword ptr [rbx + 0x7c], r14d
002dfa34: cmp    dword ptr [rbx + 0x58], eax
002dfa37: jge    0x1802dfaf7
002dfa3d: nop    dword ptr [rax]
002dfa40: cmp    r14d, edi
002dfa43: jl     0x1802dfaf7
002dfa49: movsxd rdx, dword ptr [rbx + 0x58]
002dfa4d: mov    dword ptr [rbx + 0x60], edi
002dfa50: mov    rdi, qword ptr [rbx + 0x50]
002dfa54: lea    r9d, [rdx + 1]
002dfa58: mov    dword ptr [rbx + 0x58], r9d
002dfa5c: test   rdi, rdi
002dfa5f: je     0x1802dfc13
002dfa65: lea    eax, [rdx + 1]
002dfa68: cmp    eax, dword ptr [rdi + 0x18]
002dfa6b: jae    0x1802dfc0d
002dfa71: mov    r8, qword ptr [rbx + 0x40]
002dfa75: test   r8, r8
002dfa78: je     0x1802dfc13
002dfa7e: cmp    r9d, dword ptr [r8 + 0x18]
002dfa82: jae    0x1802dfc0d
002dfa88: mov    rcx, qword ptr [rbx + 0x38]
002dfa8c: test   rcx, rcx
002dfa8f: je     0x1802dfc13
002dfa95: mov    edi, dword ptr [rdi + rdx*4 + 0x24]
002dfa99: mov    rsi, qword ptr [rbx + 0x30]
002dfa9d: movsxd rax, r9d
002dfaa0: mov    rbp, qword ptr [r8 + rax*8 + 0x20]
002dfaa5: xor    r8d, r8d
002dfaa8: mov    rdx, rbp
002dfaab: call   0x182885020                              ; UnityEngine.SpriteRenderer$$set_sprite
002dfab0: test   rsi, rsi
002dfab3: je     0x1802dfc13
002dfab9: xor    r8d, r8d
002dfabc: mov    rdx, rbp
002dfabf: mov    rcx, rsi
002dfac2: call   0x182885020                              ; UnityEngine.SpriteRenderer$$set_sprite
002dfac7: mov    rcx, qword ptr [rbx + 0x48]
002dfacb: test   rcx, rcx
002dface: je     0x1802dfc13
002dfad4: xor    edx, edx
002dfad6: call   0x1828c7320                              ; UnityEngine.ParticleSystem$$Play
002dfadb: mov    eax, dword ptr [rbx + 0x5c]
002dfade: cmp    dword ptr [rbx + 0x58], eax
002dfae1: je     0x1802dfb19
002dfae3: mov    eax, edi
002dfae5: sub    eax, dword ptr [rbx + 0x60]
002dfae8: mov    dword ptr [rbx + 0x64], eax
002dfaeb: mov    eax, dword ptr [rbx + 0x5c]
002dfaee: cmp    dword ptr [rbx + 0x58], eax
002dfaf1: jl     0x1802dfa40
002dfaf7: cmp    dword ptr [rbx + 0x64], 0
002dfafb: movd   xmm2, r14d
002dfb00: cvtdq2ps xmm2, xmm2
002dfb03: jg     0x1802dfbb0
002dfb09: movd   xmm0, edi
002dfb0d: cvtdq2ps xmm0, xmm0
002dfb10: divss  xmm2, xmm0
002dfb14: jmp    0x1802dfbc8
002dfb19: mov    rcx, qword ptr [rbx + 0x20]
002dfb1d: test   rcx, rcx
002dfb20: je     0x1802dfc13
002dfb26: movss  xmm0, dword ptr [rbx + 0x68]
002dfb2b: lea    rdx, [rsp + 0x20]
002dfb30: xor    r8d, r8d
002dfb33: movss  dword ptr [rsp + 0x20], xmm0
002dfb39: mov    dword ptr [rsp + 0x24], 0x3e19999a
002dfb41: mov    dword ptr [rsp + 0x28], 0x3f800000
002dfb49: call   0x18288a650                              ; UnityEngine.Transform$$set_localScale
002dfb4e: movss  xmm0, dword ptr [rbx + 0x70]
002dfb53: mov    rcx, qword ptr [rbx + 0x28]
002dfb57: addss  xmm0, dword ptr [rbx + 0x6c]
002dfb5c: test   rcx, rcx
002dfb5f: je     0x1802dfc13
002dfb65: movss  dword ptr [rsp + 0x20], xmm0
002dfb6b: lea    rdx, [rsp + 0x20]
002dfb70: movss  xmm0, dword ptr [rbx + 0x74]
002dfb75: xor    r8d, r8d
002dfb78: movss  dword ptr [rsp + 0x24], xmm0
002dfb7e: mov    dword ptr [rsp + 0x28], 0
002dfb86: call   0x18288a510                              ; UnityEngine.Transform$$set_localPosition
002dfb8b: movaps xmm7, xmmword ptr [rsp + 0x30]
002dfb90: movaps xmm6, xmmword ptr [rsp + 0x40]
002dfb95: mov    rdi, qword ptr [rsp + 0x70]
002dfb9a: mov    rsi, qword ptr [rsp + 0x68]
002dfb9f: mov    rbp, qword ptr [rsp + 0x60]
002dfba4: mov    rbx, qword ptr [rsp + 0x78]
002dfba9: add    rsp, 0x50
002dfbad: pop    r14
002dfbaf: ret    
002dfbb0: movd   xmm0, dword ptr [rbx + 0x60]
002dfbb5: movd   xmm1, dword ptr [rbx + 0x64]
002dfbba: cvtdq2ps xmm0, xmm0
002dfbbd: cvtdq2ps xmm1, xmm1
002dfbc0: subss  xmm2, xmm0
002dfbc4: divss  xmm2, xmm1
002dfbc8: movss  dword ptr [rbx + 0x78], xmm2
002dfbcd: mov    rcx, qword ptr [rbx + 0x20]
002dfbd1: mulss  xmm2, dword ptr [rbx + 0x68]
002dfbd6: test   rcx, rcx
002dfbd9: je     0x1802dfc13
002dfbdb: xor    r8d, r8d
002dfbde: movss  dword ptr [rsp + 0x20], xmm2
002dfbe4: lea    rdx, [rsp + 0x20]
002dfbe9: mov    dword ptr [rsp + 0x24], 0x3e19999a
002dfbf1: mov    dword ptr [rsp + 0x28], 0x3f800000
002dfbf9: call   0x18288a650                              ; UnityEngine.Transform$$set_localScale
002dfbfe: movss  xmm0, dword ptr [rbx + 0x78]
002dfc03: mulss  xmm0, dword ptr [rbx + 0x70]
002dfc08: jmp    0x1802dfb53
002dfc0d: call   0x182f60c40
002dfc12: int3   
002dfc13: call   0x182f60c50
002dfc18: int3   
002dfc19: int3   
