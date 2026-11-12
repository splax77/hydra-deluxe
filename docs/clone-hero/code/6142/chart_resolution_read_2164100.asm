02164100: mov    qword ptr [rsp + 0x18], r8
02164105: push   rbp
02164106: push   r14
02164108: push   r15
0216410a: lea    rbp, [rsp - 0x47]
0216410f: sub    rsp, 0xe0
02164116: cmp    byte ptr [rip + 0x1db558c], 0            ; [0x3f196a9] (bss)
0216411d: mov    r14d, edx
02164120: mov    r15, rcx
02164123: jne    0x182164168
02164125: lea    rcx, [rip + 0x1ba5a4c]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0216412c: call   0x182f609b0
02164131: lea    rcx, [rip + 0x1b3d8a8]                   ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02164138: call   0x182f609b0
0216413d: lea    rcx, [rip + 0x1b3dc1c]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02164144: call   0x182f609b0
02164149: lea    rcx, [rip + 0x1b30888]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02164150: call   0x182f609b0
02164155: lea    rcx, [rip + 0x1b57da4]                   ; [0x3cbbf00] str:'Resolution'
0216415c: call   0x182f609b0
02164161: mov    byte ptr [rip + 0x1db5541], 1            ; [0x3f196a9] (bss)
02164168: mov    qword ptr [rsp + 0x118], rbx
02164170: mov    qword ptr [rsp + 0xd8], rsi
02164178: mov    qword ptr [rsp + 0xd0], rdi
02164180: mov    qword ptr [rsp + 0xc8], r12
02164188: mov    qword ptr [rsp + 0xc0], r13
02164190: movaps xmmword ptr [rsp + 0xb0], xmm6
02164198: movaps xmmword ptr [rsp + 0xa0], xmm7
021641a0: movaps xmmword ptr [rsp + 0x90], xmm8
021641a9: cmp    r14d, dword ptr [r15 + 8]
021641ad: jae    0x182164727
021641b3: movsd  xmm8, qword ptr [rip + 0xfdc7b4]         ; [0x3140970] dbl=65.0 q=0x4050400000000000
021641bc: xor    sil, sil
021641bf: movsd  xmm7, qword ptr [rip + 0xeffde9]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
021641c7: mov    byte ptr [rbp + 0x6f], sil
021641cb: mov    rcx, qword ptr [r15]
021641ce: nop    
021641d0: movsxd rax, r14d
021641d3: movzx  edx, word ptr [rcx + rax*2]
021641d7: cmp    edx, 0x20
021641da: jbe    0x1821642b3
021641e0: cmp    edx, 0x7b
021641e3: je     0x1821646d0
021641e9: cmp    edx, 0x7d
021641ec: jne    0x1821642e3
021641f2: mov    ebx, dword ptr [r15 + 8]
021641f6: mov    rdi, rcx
021641f9: mov    rcx, qword ptr [rip + 0x1b307d8]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02164200: inc    r14d
02164203: cmp    dword ptr [rcx + 0xe0], 0
0216420a: jne    0x182164211
0216420c: call   0x182f60cf0
02164211: cmp    r14d, ebx
02164214: jae    0x182164727
0216421a: movsxd rax, r14d
0216421d: movzx  ecx, word ptr [rdi + rax*2]
02164221: cmp    cx, 9
02164225: je     0x18216422d
02164227: cmp    cx, 0x20
0216422b: jne    0x182164232
0216422d: inc    r14d
02164230: jmp    0x182164211
02164232: cmp    r14d, ebx
02164235: jge    0x1821646d3
0216423b: jae    0x182164727
02164241: movsxd rcx, r14d
02164244: cmp    word ptr [rdi + rcx*2], 0xa
02164249: je     0x182164259
0216424b: movsxd rcx, r14d
0216424e: cmp    word ptr [rdi + rcx*2], 0xd
02164253: jne    0x1821646d3
02164259: test   sil, sil
0216425c: je     0x18216472d
02164262: movaps xmm8, xmmword ptr [rsp + 0x90]
0216426b: mov    eax, r14d
0216426e: movaps xmm7, xmmword ptr [rsp + 0xa0]
02164276: movaps xmm6, xmmword ptr [rsp + 0xb0]
0216427e: mov    r13, qword ptr [rsp + 0xc0]
02164286: mov    r12, qword ptr [rsp + 0xc8]
0216428e: mov    rdi, qword ptr [rsp + 0xd0]
02164296: mov    rsi, qword ptr [rsp + 0xd8]
0216429e: mov    rbx, qword ptr [rsp + 0x118]
021642a6: add    rsp, 0xe0
021642ad: pop    r15
021642af: pop    r14
021642b1: pop    rbp
021642b2: ret    
021642b3: mov    ecx, edx
021642b5: sub    ecx, 9
021642b8: je     0x1821646d0
021642be: sub    ecx, 1
021642c1: je     0x1821646d0
021642c7: sub    ecx, 1
021642ca: je     0x1821642e3
021642cc: sub    ecx, 1
021642cf: je     0x1821642e3
021642d1: cmp    ecx, 1
021642d4: je     0x1821646d0
021642da: cmp    edx, 0x20
021642dd: je     0x1821646d0
021642e3: mov    rcx, qword ptr [rip + 0x1b306ee]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
021642ea: mov    edx, r14d
021642ed: mov    rdi, qword ptr [r15]
021642f0: mov    ebx, dword ptr [r15 + 8]
021642f4: mov    dword ptr [rbp + 0x67], edx
021642f7: cmp    dword ptr [rcx + 0xe0], 0
021642fe: jne    0x182164310
02164300: call   0x182f60cf0
02164305: mov    edx, r14d
02164308: nop    dword ptr [rax + rax]
02164310: cmp    r14d, ebx
02164313: jae    0x182164727
02164319: movsxd rax, r14d
0216431c: movzx  ecx, word ptr [rdi + rax*2]
02164320: cmp    cx, 9
02164324: je     0x182164331
02164326: cmp    cx, 0x20
0216432a: je     0x182164331
0216432c: inc    r14d
0216432f: jmp    0x182164310
02164331: mov    rbx, qword ptr [rip + 0x1b3d6a8]         ; [0x3ca19e0] metamethod:Method$System.ReadOnlySpan<char>.Slice()
02164338: mov    r12d, r14d
0216433b: sub    r12d, edx
0216433e: cmp    edx, dword ptr [r15 + 8]
02164342: ja     0x18216434f
02164344: mov    eax, dword ptr [r15 + 8]
02164348: sub    eax, edx
0216434a: cmp    r12d, eax
0216434d: jbe    0x182164356
0216434f: xor    ecx, ecx
02164351: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02164356: mov    rcx, qword ptr [rbx + 0x20]
0216435a: xorps  xmm0, xmm0
0216435d: movups xmmword ptr [rbp - 0x19], xmm0
02164361: test   byte ptr [rcx + 0x135], 1
02164368: jne    0x18216436f
0216436a: call   0x182f65750
0216436f: cmp    byte ptr [rip + 0x1da7de5], 0            ; [0x3f0c15b] (bss)
02164376: mov    rbx, qword ptr [rip + 0x1b57b83]         ; [0x3cbbf00] str:'Resolution'
0216437d: jne    0x182164392
0216437f: lea    rcx, [rip + 0x1b3d222]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
02164386: call   0x182f609b0
0216438b: mov    byte ptr [rip + 0x1da7dc9], 1            ; [0x3f0c15b] (bss)
02164392: xorps  xmm1, xmm1
02164395: test   rbx, rbx
02164398: je     0x1821643b9
0216439a: xor    edx, edx
0216439c: mov    dword ptr [rbp - 0x5d], 0
021643a3: mov    rcx, rbx
021643a6: call   0x1819829d0                              ; System.String$$GetRawStringData
021643ab: mov    qword ptr [rbp - 0x69], rax
021643af: mov    eax, dword ptr [rbx + 0x10]
021643b2: mov    dword ptr [rbp - 0x61], eax
021643b5: movaps xmm1, xmmword ptr [rbp - 0x69]
021643b9: mov    r13, qword ptr [rip + 0x1ba57b8]         ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
021643c0: movdqa xmm0, xmm1
021643c4: psrldq xmm0, 8
021643c9: movq   rbx, xmm1
021643ce: movd   esi, xmm0
021643d2: psrldq xmm1, 0xc
021643d7: cmp    qword ptr [r13 + 0x38], 0
021643dc: mov    dword ptr [rbp - 0x79], esi
021643df: movd   edi, xmm1
021643e3: jne    0x1821643ed
021643e5: mov    rcx, r13
021643e8: call   0x182f657d0
021643ed: mov    rdx, qword ptr [r13 + 0x38]
021643f1: lea    rcx, [rbp - 0x71]
021643f5: mov    qword ptr [rbp - 0x71], 0
021643fd: mov    rdx, qword ptr [rdx + 0x20]
02164401: call   0x180256510                              ; System.MemoryExtensions$$IsTypeComparableAsBytes<__Il2CppFullySharedGenericType>
02164406: cmp    r12d, esi
02164409: jne    0x182164657
0216440f: movsxd rdx, dword ptr [rbp + 0x67]
02164413: mov    rcx, qword ptr [r15]
02164416: lea    r8, [rcx + rdx*2]
0216441a: mov    rdx, qword ptr [r13 + 0x38]
0216441e: test   al, al
02164420: mov    eax, dword ptr [rbp - 0xd]
02164423: mov    rdx, qword ptr [rdx + 0x28]
02164427: jne    0x182164476
02164429: lea    rcx, [rbp - 0x59]
0216442d: mov    qword ptr [rbp - 0x59], r8
02164431: mov    dword ptr [rbp - 0x51], r12d
02164435: mov    dword ptr [rbp - 0x4d], eax
02164438: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
0216443d: mov    rdx, qword ptr [r13 + 0x38]
02164441: lea    rcx, [rbp - 0x49]
02164445: mov    rsi, rax
02164448: mov    qword ptr [rbp - 0x49], rbx
0216444c: mov    eax, dword ptr [rbp - 0x79]
0216444f: mov    dword ptr [rbp - 0x41], eax
02164452: mov    rdx, qword ptr [rdx + 0x28]
02164456: mov    dword ptr [rbp - 0x3d], edi
02164459: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
0216445e: mov    r9, qword ptr [r13 + 0x38]
02164462: mov    r8d, r12d
02164465: mov    rdx, rax
02164468: mov    rcx, rsi
0216446b: mov    r9, qword ptr [r9 + 0x38]
0216446f: call   0x1804ffed0                              ; System.SpanHelpers$$SequenceEqual<char>
02164474: jmp    0x1821644c1
02164476: lea    rcx, [rbp - 0x39]
0216447a: mov    qword ptr [rbp - 0x39], r8
0216447e: mov    dword ptr [rbp - 0x31], r12d
02164482: mov    dword ptr [rbp - 0x2d], eax
02164485: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
0216448a: mov    rdx, qword ptr [r13 + 0x38]
0216448e: lea    rcx, [rbp - 0x29]
02164492: mov    rsi, rax
02164495: mov    qword ptr [rbp - 0x29], rbx
02164499: mov    eax, dword ptr [rbp - 0x79]
0216449c: mov    dword ptr [rbp - 0x21], eax
0216449f: mov    rdx, qword ptr [rdx + 0x28]
021644a3: mov    dword ptr [rbp - 0x1d], edi
021644a6: call   0x18010fab0                              ; System.Runtime.CompilerServices.Unsafe$$ReadUnaligned<ulong>
021644ab: movsxd r8, r12d
021644ae: xor    r9d, r9d
021644b1: imul   r8, qword ptr [rbp - 0x71]
021644b6: mov    rdx, rax
021644b9: mov    rcx, rsi
021644bc: call   0x181b72d30                              ; System.SpanHelpers$$SequenceEqual
021644c1: test   al, al
021644c3: je     0x182164657
021644c9: lea    eax, [r14 + 3]
021644cd: cmp    eax, dword ptr [r15 + 8]
021644d1: jae    0x182164727
021644d7: mov    rax, qword ptr [r15]
021644da: xor    edx, edx
021644dc: mov    ebx, dword ptr [r15 + 8]
021644e0: mov    rdi, rax
021644e3: movsxd rcx, r14d
021644e6: cmp    word ptr [rax + rcx*2 + 6], 0x22
021644ec: mov    rcx, qword ptr [rip + 0x1b304e5]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
021644f3: sete   dl
021644f6: add    edx, 3
021644f9: add    r14d, edx
021644fc: cmp    dword ptr [rcx + 0xe0], 0
02164503: jne    0x18216450a
02164505: call   0x182f60cf0
0216450a: xor    edx, edx
0216450c: nop    dword ptr [rax]
02164510: cmp    r14d, ebx
02164513: jae    0x182164727
02164519: movsxd rax, r14d
0216451c: movzx  r8d, word ptr [rdi + rax*2]
02164521: lea    eax, [r8 - 0x30]
02164525: cmp    ax, 9
02164529: ja     0x18216453c
0216452b: lea    rdx, [rdx + rdx*4]
0216452f: inc    r14d
02164532: lea    rdx, [rdx - 0x18]
02164536: lea    rdx, [r8 + rdx*2]
0216453a: jmp    0x182164510
0216453c: cmp    r14d, ebx
0216453f: jae    0x182164727
02164545: movsxd rcx, r14d
02164548: cmp    word ptr [rdi + rcx*2], 0x22
0216454d: jne    0x182164552
0216454f: inc    r14d
02164552: test   rdx, rdx
02164555: jle    0x1821646de
0216455b: mov    rsi, qword ptr [rbp + 0x77]
0216455f: test   rsi, rsi
02164562: je     0x182164776
02164568: mov    rcx, qword ptr [rsi + 0x30]
0216456c: test   rcx, rcx
0216456f: je     0x182164776
02164575: xorps  xmm1, xmm1
02164578: xor    r8d, r8d
0216457b: cvtsi2sd xmm1, rdx
02164580: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
02164585: cmp    byte ptr [rip + 0x1db511a], 0            ; [0x3f196a6] (bss)
0216458c: jne    0x1821645a1
0216458e: lea    rcx, [rip + 0x1b3cadb]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02164595: call   0x182f609b0
0216459a: mov    byte ptr [rip + 0x1db5105], 1            ; [0x3f196a6] (bss)
021645a1: mov    rax, qword ptr [rsi + 0x30]
021645a5: test   rax, rax
021645a8: je     0x182164776
021645ae: cmp    dword ptr [rsi + 0x18], 0
021645b2: jg     0x182164604
021645b4: cmp    byte ptr [rsi + 0x1c], 0
021645b8: movsd  xmm6, qword ptr [rax + 0xb8]
021645c0: jne    0x1821645f8
021645c2: mov    rax, qword ptr [rip + 0x1b3caa7]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
021645c9: cmp    dword ptr [rax + 0xe0], 0
021645d0: jne    0x1821645e1
021645d2: mov    rcx, rax
021645d5: call   0x182f60cf0
021645da: mov    rax, qword ptr [rip + 0x1b3ca8f]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
021645e1: mov    rax, qword ptr [rax + 0xb8]
021645e8: mulsd  xmm6, xmm8
021645ed: divsd  xmm6, qword ptr [rax + 0x68]
021645f2: cvttsd2si eax, xmm6
021645f6: jmp    0x182164607
021645f8: mulsd  xmm6, xmm7
021645fc: cvttsd2si eax, xmm6
02164600: inc    eax
02164602: jmp    0x182164607
02164604: mov    eax, dword ptr [rsi + 0x18]
02164607: mov    rcx, qword ptr [rsi + 0x30]
0216460b: test   rcx, rcx
0216460e: je     0x182164776
02164614: mov    dword ptr [rcx + 0x118], eax
0216461a: mov    rax, qword ptr [rsi + 0x30]
0216461e: test   rax, rax
02164621: je     0x182164776
02164627: mov    byte ptr [rax + 0x11c], 0
0216462e: cmp    dword ptr [rsi + 0x24], 0
02164632: jge    0x182164638
02164634: xor    ecx, ecx
02164636: jmp    0x18216463b
02164638: mov    ecx, dword ptr [rsi + 0x24]
0216463b: mov    rax, qword ptr [rsi + 0x30]
0216463f: test   rax, rax
02164642: je     0x182164776
02164648: mov    sil, 1
0216464b: mov    dword ptr [rax + 0x114], ecx
02164651: mov    byte ptr [rbp + 0x6f], sil
02164655: jmp    0x18216465b
02164657: movzx  esi, byte ptr [rbp + 0x6f]
0216465b: mov    rcx, qword ptr [rip + 0x1b30376]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02164662: mov    rdi, qword ptr [r15]
02164665: mov    ebx, dword ptr [r15 + 8]
02164669: cmp    dword ptr [rcx + 0xe0], 0
02164670: jne    0x182164677
02164672: call   0x182f60cf0
02164677: cmp    byte ptr [rip + 0x1da99e4], 0            ; [0x3f0e062] (bss)
0216467e: jne    0x182164693
02164680: lea    rcx, [rip + 0x1b3d6d9]                   ; [0x3ca1d60] metamethod:Method$System.ReadOnlySpan<char>.get_Length()
02164687: call   0x182f609b0
0216468c: mov    byte ptr [rip + 0x1da99cf], 1            ; [0x3f0e062] (bss)
02164693: cmp    r14d, ebx
02164696: jge    0x1821646d3
02164698: jae    0x182164727
0216469e: movsxd rax, r14d
021646a1: movzx  ecx, word ptr [rdi + rax*2]
021646a5: cmp    cx, 0xa
021646a9: je     0x1821646d3
021646ab: cmp    cx, 0xd
021646af: jne    0x1821646c6
021646b1: lea    eax, [r14 + 1]
021646b5: cmp    eax, ebx
021646b7: jge    0x1821646c6
021646b9: jae    0x182164727
021646bb: movsxd rax, r14d
021646be: cmp    word ptr [rdi + rax*2 + 2], 0xa
021646c4: je     0x1821646d3
021646c6: inc    r14d
021646c9: cmp    r14d, ebx
021646cc: jl     0x182164698
021646ce: jmp    0x1821646d3
021646d0: inc    r14d
021646d3: cmp    r14d, dword ptr [r15 + 8]
021646d7: jae    0x182164727
021646d9: jmp    0x1821641cb
021646de: lea    rcx, [rip + 0x1b3102b]                   ; [0x3c95710] meta:ʷʶʴʳʽʷʿʳʾˁʺ_TypeInfo
021646e5: call   0x182f609d0
021646ea: mov    rcx, rax
021646ed: call   0x182f60c00
021646f2: lea    rcx, [rip + 0x1b402e7]                   ; [0x3ca49e0] str:'Invalid resolution value in header.'
021646f9: mov    rbx, rax
021646fc: call   0x182f609d0
02164701: mov    rdx, rax
02164704: xor    r8d, r8d
02164707: mov    rcx, rbx
0216470a: call   0x1820d5f30                              ; ʷʶʴʳʽʷʿʳʾˁʺ$$.ctor
0216470f: lea    rcx, [rip + 0x1b2ddda]                   ; [0x3c924f0] metamethod:Method$ʺʹˁʿʺʼʼʷʳʴʶ.ˁʸʷʶʳˀʼʹʿʺʳ()
02164716: call   0x182f609d0
0216471b: mov    rdx, rax
0216471e: mov    rcx, rbx
02164721: call   0x182f60c10
02164726: int3   
02164727: call   0x182f60c40
0216472c: int3   
0216472d: lea    rcx, [rip + 0x1b8193c]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
02164734: call   0x182f609d0
02164739: mov    rcx, rax
0216473c: call   0x182f60c00
02164741: lea    rcx, [rip + 0x1b7cd68]                   ; [0x3ce14b0] str:'Song Resolution not found in header.'
02164748: mov    rbx, rax
0216474b: call   0x182f609d0
02164750: mov    rdx, rax
02164753: xor    r8d, r8d
02164756: mov    rcx, rbx
02164759: call   0x181b92e30                              ; System.Exception$$.ctor
0216475e: lea    rcx, [rip + 0x1b2dd8b]                   ; [0x3c924f0] metamethod:Method$ʺʹˁʿʺʼʼʷʳʴʶ.ˁʸʷʶʳˀʼʹʿʺʳ()
02164765: call   0x182f609d0
0216476a: mov    rdx, rax
0216476d: mov    rcx, rbx
02164770: call   0x182f60c10
02164775: int3   
02164776: call   0x182f60c50
0216477b: int3   
0216477c: int3   
