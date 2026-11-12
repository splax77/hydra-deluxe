0041be70: mov    qword ptr [rsp + 0x10], rdx
0041be75: push   rsi
0041be76: push   rdi
0041be77: push   r13
0041be79: push   r14
0041be7b: push   r15
0041be7d: sub    rsp, 0xe0
0041be84: mov    rsi, qword ptr [rsp + 0x150]
0041be8c: mov    r15, r9
0041be8f: mov    r13, r8
0041be92: mov    r14, rdx
0041be95: mov    rdi, rcx
0041be98: cmp    qword ptr [rsi + 0x38], 0
0041be9d: jne    0x18041bec6
0041be9f: lea    rcx, [rip + 0x387c9a2]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0041bea6: call   0x182f609b0
0041beab: lea    rcx, [rip + 0x388031e]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
0041beb2: call   0x182f609b0
0041beb7: cmp    qword ptr [rsi + 0x38], 0
0041bebc: jne    0x18041bec6
0041bebe: mov    rcx, rsi
0041bec1: call   0x182f657d0
0041bec6: xor    eax, eax
0041bec8: mov    qword ptr [rsp + 0x28], rax
0041becd: mov    qword ptr [rsp + 0x30], rax
0041bed2: test   r14, r14
0041bed5: je     0x18041c2ce
0041bedb: mov    qword ptr [rsp + 0x110], rbx
0041bee3: mov    qword ptr [rsp + 0x120], rbp
0041beeb: mov    qword ptr [rsp + 0x128], r12
0041bef3: movaps xmmword ptr [rsp + 0xd0], xmm6
0041befb: movaps xmmword ptr [rsp + 0xc0], xmm7
0041bf03: movaps xmmword ptr [rsp + 0xb0], xmm8
0041bf0c: movaps xmmword ptr [rsp + 0xa0], xmm9
0041bf15: movaps xmmword ptr [rsp + 0x90], xmm10
0041bf1e: movaps xmmword ptr [rsp + 0x80], xmm11
0041bf27: test   r13, r13
0041bf2a: je     0x18041c2de
0041bf30: cmp    byte ptr [rip + 0x3af055d], al           ; [0x3f0c493] (bss)
0041bf36: mov    ebp, dword ptr [r13 + 0x18]
0041bf3a: mov    dword ptr [rsp + 0x20], ebp
0041bf3e: jne    0x18041bf53
0041bf40: lea    rcx, [rip + 0x38d5491]                   ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
0041bf47: call   0x182f609b0
0041bf4c: mov    byte ptr [rip + 0x3af0540], 1            ; [0x3f0c493] (bss)
0041bf53: mov    rax, qword ptr [rip + 0x38d547e]         ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
0041bf5a: mov    ebx, dword ptr [rsp + 0x140]
0041bf61: mov    rax, qword ptr [rax + 0xb8]
0041bf68: movsd  xmm0, qword ptr [rax]
0041bf6c: mov    eax, dword ptr [rsp + 0x130]
0041bf73: movsd  qword ptr [rsp + 0x40], xmm0
0041bf79: cmp    eax, dword ptr [rsp + 0x138]
0041bf80: jg     0x18041c280
0041bf86: movss  xmm10, dword ptr [rsp + 0x148]
0041bf90: movss  xmm11, dword ptr [rip + 0x2c4841b]       ; [0x30643b4] flt=0.5 i=1056964608
0041bf99: nop    dword ptr [rax]
0041bfa0: cmp    eax, dword ptr [r14 + 0x18]
0041bfa4: jge    0x18041c280
0041bfaa: cmp    ebx, ebp
0041bfac: jge    0x18041c280
0041bfb2: mov    r8, qword ptr [rsi + 0x38]
0041bfb6: mov    edx, eax
0041bfb8: mov    rcx, r14
0041bfbb: mov    r8, qword ptr [r8 + 8]
0041bfbf: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0041bfc4: mov    qword ptr [rsp + 0x38], rax
0041bfc9: test   rax, rax
0041bfcc: je     0x18041c2de
0041bfd2: mov    rdx, qword ptr [rip + 0x38801f7]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
0041bfd9: mov    ecx, 3
0041bfde: mov    r8, rax
0041bfe1: call   0x182c44210
0041bfe6: mov    rcx, qword ptr [rip + 0x387c85b]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0041bfed: movzx  esi, ax
0041bff0: cmp    dword ptr [rcx + 0xe0], 0
0041bff7: jne    0x18041bffe
0041bff9: call   0x182f60cf0
0041bffe: mov    ecx, esi
0041c000: xor    edx, edx
0041c002: call   0x1820fe7e0                              ; ʹʾʽʼʷʲʴʸʷʼˁ$$ˁʿʸʲʺʳʴʳˁʼʾ
0041c007: xor    edx, edx
0041c009: mov    qword ptr [rsp + 0x30], rax
0041c00e: lea    rcx, [rsp + 0x30]
0041c013: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
0041c018: xor    edx, edx
0041c01a: mov    qword ptr [rsp + 0x28], rax
0041c01f: lea    rcx, [rsp + 0x28]
0041c024: call   0x1820fc610                              ; ʹʾʽʼʷʲʴʸʷʼˁ.ʶʽʶʷʺʾʼʵʳʶʽ$$ʿʵʷʻʵʻʷʵʶʺʼ
0041c029: test   al, al
0041c02b: je     0x18041c25a
0041c031: xor    edx, edx
0041c033: lea    rcx, [rsp + 0x28]
0041c038: call   0x1820fc5f0                              ; ʹʾʽʼʷʲʴʸʷʼˁ.ʶʽʶʷʺʾʼʵʳʶʽ$$ʳʳʷʹʺʸʶʹʹʶʾ
0041c03d: mov    r12d, eax
0041c040: cmp    ebx, ebp
0041c042: jge    0x18041c252
0041c048: mov    rax, qword ptr [rdi]
0041c04b: movzx  edx, r12w
0041c04f: mov    rcx, rdi
0041c052: mov    r8, qword ptr [rax + 0x1f0]
0041c059: call   qword ptr [rax + 0x1e8]
0041c05f: mov    ebp, eax
0041c061: cmp    ebx, dword ptr [r13 + 0x18]
0041c065: jae    0x18041c2e4
0041c06b: movsxd rax, ebx
0041c06e: mov    rsi, qword ptr [r13 + rax*8 + 0x20]
0041c073: test   rsi, rsi
0041c076: je     0x18041c2de
0041c07c: xor    r8d, r8d
0041c07f: mov    dl, 1
0041c081: mov    rcx, rsi
0041c084: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0041c089: test   r15, r15
0041c08c: je     0x18041c2de
0041c092: cmp    ebx, dword ptr [r15 + 0x18]
0041c096: jae    0x18041c2e4
0041c09c: movsxd rax, ebx
0041c09f: mov    r14, qword ptr [r15 + rax*8 + 0x20]
0041c0a4: test   r14, r14
0041c0a7: je     0x18041c2de
0041c0ad: xor    edx, edx
0041c0af: mov    rcx, r14
0041c0b2: call   0x182884e20                              ; UnityEngine.SpriteRenderer$$get_size
0041c0b7: mov    rcx, qword ptr [rsp + 0x150]
0041c0bf: mov    edx, ebp
0041c0c1: mov    rax, qword ptr [rcx + 0x20]
0041c0c5: mov    rcx, rdi
0041c0c8: mov    r8, qword ptr [rax + 0xc0]
0041c0cf: mov    r8, qword ptr [r8 + 0x10]
0041c0d3: call   0x181160b90                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʵʾʷʼˀʲʹʽʺʿʾ
0041c0d8: mov    r8, qword ptr [rsp + 0x38]
0041c0dd: mov    ecx, 5
0041c0e2: mov    rdx, qword ptr [rip + 0x38800e7]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
0041c0e9: movaps xmm6, xmm0
0041c0ec: call   0x182c44210
0041c0f1: mulss  xmm0, dword ptr [rdi + 0x34]
0041c0f6: xor    r8d, r8d
0041c0f9: mov    rcx, r14
0041c0fc: addss  xmm0, xmm10
0041c101: unpcklps xmm6, xmm0
0041c104: movq   rdx, xmm6
0041c109: call   0x182884fe0                              ; UnityEngine.SpriteRenderer$$set_size
0041c10e: mov    rax, qword ptr [rdi]
0041c111: lea    rcx, [rsp + 0x70]
0041c116: movzx  r8d, r12w
0041c11a: mov    rdx, rdi
0041c11d: mov    r9, qword ptr [rax + 0x210]
0041c124: call   qword ptr [rax + 0x208]
0041c12a: xor    r8d, r8d
0041c12d: mov    dword ptr [rsp + 0x6c], 0x3f400000
0041c135: lea    rdx, [rsp + 0x60]
0041c13a: mov    rcx, r14
0041c13d: movss  xmm0, dword ptr [rax]
0041c141: movss  xmm1, dword ptr [rax + 4]
0041c146: movss  dword ptr [rsp + 0x60], xmm0
0041c14c: movss  xmm0, dword ptr [rax + 8]
0041c151: movss  dword ptr [rsp + 0x68], xmm0
0041c157: movss  dword ptr [rsp + 0x64], xmm1
0041c15d: call   0x182884f00                              ; UnityEngine.SpriteRenderer$$set_color
0041c162: mov    r8, qword ptr [rsp + 0x38]
0041c167: mov    ecx, 4
0041c16c: mov    rdx, qword ptr [rip + 0x388005d]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
0041c173: call   0x182c44210
0041c178: mov    rax, qword ptr [rdi + 0x28]
0041c17c: movaps xmm6, xmm0
0041c17f: test   rax, rax
0041c182: je     0x18041c2de
0041c188: mov    rcx, qword ptr [rip + 0x387c6b9]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0041c18f: movsd  xmm7, qword ptr [rax + 0x78]
0041c194: movss  xmm8, dword ptr [rdi + 0x48]
0041c19a: movss  xmm9, dword ptr [rdi + 0x34]
0041c1a0: cmp    dword ptr [rcx + 0xe0], 0
0041c1a7: jne    0x18041c1ae
0041c1a9: call   0x182f60cf0
0041c1ae: subsd  xmm6, xmm7
0041c1b2: cvtps2pd xmm0, xmm9
0041c1b6: cvtps2pd xmm1, xmm8
0041c1ba: mulsd  xmm6, xmm0
0041c1be: movaps xmm0, xmm10
0041c1c2: mulss  xmm0, xmm11
0041c1c7: addsd  xmm6, xmm1
0041c1cb: xorps  xmm1, xmm1
0041c1ce: cvtss2sd xmm1, xmm0
0041c1d2: subsd  xmm6, xmm1
0041c1d6: test   ebp, ebp
0041c1d8: jg     0x18041c1df
0041c1da: xorps  xmm0, xmm0
0041c1dd: jmp    0x18041c1f9
0041c1df: lea    eax, [rbp - 1]
0041c1e2: movd   xmm0, eax
0041c1e6: cvtdq2ps xmm0, xmm0
0041c1e9: mulss  xmm0, dword ptr [rdi + 0xec]
0041c1f1: addss  xmm0, dword ptr [rdi + 0xe8]
0041c1f9: xor    edx, edx
0041c1fb: movss  dword ptr [rsp + 0x40], xmm0
0041c201: mov    rcx, rsi
0041c204: cvtsd2ss xmm6, xmm6
0041c208: call   0x18287b3e0                              ; UnityEngine.GameObject$$get_transform
0041c20d: test   rax, rax
0041c210: je     0x18041c2de
0041c216: movsd  xmm0, qword ptr [rsp + 0x40]
0041c21c: lea    rdx, [rsp + 0x50]
0041c221: xor    r8d, r8d
0041c224: movsd  qword ptr [rsp + 0x50], xmm0
0041c22a: mov    rcx, rax
0041c22d: movss  dword ptr [rsp + 0x58], xmm6
0041c233: call   0x18288a510                              ; UnityEngine.Transform$$set_localPosition
0041c238: xor    edx, edx
0041c23a: lea    rcx, [rsp + 0x28]
0041c23f: inc    ebx
0041c241: call   0x1820fc610                              ; ʹʾʽʼʷʲʴʸʷʼˁ.ʶʽʶʷʺʾʼʵʳʶʽ$$ʿʵʷʻʵʻʷʵʶʺʼ
0041c246: mov    ebp, dword ptr [rsp + 0x20]
0041c24a: test   al, al
0041c24c: jne    0x18041c031
0041c252: mov    r14, qword ptr [rsp + 0x118]
0041c25a: mov    eax, dword ptr [rsp + 0x130]
0041c261: inc    eax
0041c263: mov    dword ptr [rsp + 0x130], eax
0041c26a: cmp    eax, dword ptr [rsp + 0x138]
0041c271: jg     0x18041c280
0041c273: mov    rsi, qword ptr [rsp + 0x150]
0041c27b: jmp    0x18041bfa0
0041c280: movaps xmm10, xmmword ptr [rsp + 0x90]
0041c289: mov    eax, ebx
0041c28b: mov    rbx, qword ptr [rsp + 0x110]
0041c293: movaps xmm9, xmmword ptr [rsp + 0xa0]
0041c29c: movaps xmm8, xmmword ptr [rsp + 0xb0]
0041c2a5: movaps xmm7, xmmword ptr [rsp + 0xc0]
0041c2ad: movaps xmm6, xmmword ptr [rsp + 0xd0]
0041c2b5: mov    r12, qword ptr [rsp + 0x128]
0041c2bd: mov    rbp, qword ptr [rsp + 0x120]
0041c2c5: movaps xmm11, xmmword ptr [rsp + 0x80]
0041c2ce: add    rsp, 0xe0
0041c2d5: pop    r15
0041c2d7: pop    r14
0041c2d9: pop    r13
0041c2db: pop    rdi
0041c2dc: pop    rsi
0041c2dd: ret    
0041c2de: call   0x182f60c50
0041c2e3: int3   
0041c2e4: call   0x182f60c40
0041c2e9: int3   
0041c2ea: int3   
