0033c560: mov    qword ptr [rsp + 0x10], rsi
0033c565: mov    qword ptr [rsp + 0x18], rdi
0033c56a: push   rbp
0033c56b: push   r14
0033c56d: push   r15
0033c56f: lea    rbp, [rsp - 0x90]
0033c577: sub    rsp, 0x190
0033c57e: cmp    byte ptr [rip + 0x3bd1a3c], 0            ; [0x3f0dfc1] (bss)
0033c585: mov    r15, rdx
0033c588: mov    rdi, rcx
0033c58b: jne    0x18033c61c
0033c591: lea    rcx, [rip + 0x399b940]                   ; [0x3cd7ed8] metamethod:Method$BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>.ˀʹʲʹʶʺʳʲʸʿʹ()
0033c598: call   0x182f609b0
0033c59d: lea    rcx, [rip + 0x39b32ec]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
0033c5a4: call   0x182f609b0
0033c5a9: lea    rcx, [rip + 0x395b888]                   ; [0x3c97e38] meta:ʵʹʸʵˁˁʲʽʴʵʷ<ʾʸʸʻʷʸˁʾˁʽʸ>_TypeInfo
0033c5b0: call   0x182f609b0
0033c5b5: lea    rcx, [rip + 0x39ba3e4]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0033c5bc: call   0x182f609b0
0033c5c1: lea    rcx, [rip + 0x39cab08]                   ; [0x3d070d0] meta:ˀʾʴʼʳʽʲʲʴˁʺ.ʹʸʶʲʶʵʾʲˀʷˀ_TypeInfo
0033c5c8: call   0x182f609b0
0033c5cd: lea    rcx, [rip + 0x395bd4c]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0033c5d4: call   0x182f609b0
0033c5d9: lea    rcx, [rip + 0x395ca78]                   ; [0x3c99058] meta:ʺʺʸʶʿʵˀʸʲˀʲ_TypeInfo
0033c5e0: call   0x182f609b0
0033c5e5: lea    rcx, [rip + 0x3959cfc]                   ; [0x3c962e8] metamethod:Method$ʼʺʸʻʿʷˁʾʴʴʿ.ˁʾʻʴʷˀʶˁʵʻʻ()
0033c5ec: call   0x182f609b0
0033c5f1: lea    rcx, [rip + 0x39c31f8]                   ; [0x3cff7f0] metamethod:Method$ʾʿʶʺʾʽʺʺˁˁʹ<ʺʺʸʶʿʵˀʸʲˀʲ, ʾʸʸʻʷʸˁʾˁʽʸ>..ctor()
0033c5f8: call   0x182f609b0
0033c5fd: lea    rcx, [rip + 0x395ba64]                   ; [0x3c98068] meta:ʾʿʶʺʾʽʺʺˁˁʹ<ʺʺʸʶʿʵˀʸʲˀʲ, ʾʸʸʻʷʸˁʾˁʽʸ>_TypeInfo
0033c604: call   0x182f609b0
0033c609: lea    rcx, [rip + 0x39c3e58]                   ; [0x3d00468] metamethod:Method$ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>.ʴʺʴʺʴʷʲʺʷʽʿ()
0033c610: call   0x182f609b0
0033c615: mov    byte ptr [rip + 0x3bd19a5], 1            ; [0x3f0dfc1] (bss)
0033c61c: mov    rcx, qword ptr [rdi + 0x88]
0033c623: xorps  xmm0, xmm0
0033c626: mov    qword ptr [rsp + 0x1b0], rbx
0033c62e: xor    edx, edx
0033c630: movups xmmword ptr [rsp + 0x70], xmm0
0033c635: movaps xmmword ptr [rsp + 0x180], xmm6
0033c63d: call   0x18013f8e0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʾʻʷʿʿʶʵʻʲʷ
0033c642: xor    esi, esi
0033c644: movzx  r14d, al
0033c648: test   al, al
0033c64a: je     0x18033c796
0033c650: mov    rcx, qword ptr [rdi + 0x88]
0033c657: test   rcx, rcx
0033c65a: je     0x18033cb30
0033c660: mov    rdx, qword ptr [rcx + 0x10]
0033c664: test   rdx, rdx
0033c667: je     0x18033cb30
0033c66d: mov    rdx, qword ptr [rdx + 0xe0]
0033c674: test   rdx, rdx
0033c677: jne    0x18033c687
0033c679: mov    qword ptr [rdi + 0x1f0], rsi
0033c680: mov    eax, esi
0033c682: jmp    0x18033c707
0033c687: mov    r9, qword ptr [rip + 0x395b7aa]          ; [0x3c97e38] meta:ʵʹʸʵˁˁʲʽʴʵʷ<ʾʸʸʻʷʸˁʾˁʽʸ>_TypeInfo
0033c68e: mov    r8d, 1
0033c694: mov    r10, qword ptr [rdx]
0033c697: movzx  eax, byte ptr [r9 + 0x130]
0033c69f: cmp    byte ptr [r10 + 0x130], al
0033c6a6: jb     0x18033c6bc
0033c6a8: movzx  ecx, al
0033c6ab: mov    rax, qword ptr [r10 + 0xc8]
0033c6b2: cmp    qword ptr [rax + rcx*8 - 8], r9
0033c6b7: mov    ecx, r8d
0033c6ba: je     0x18033c6be
0033c6bc: mov    ecx, esi
0033c6be: test   ecx, ecx
0033c6c0: mov    rax, rsi
0033c6c3: cmovne rax, rdx
0033c6c7: mov    qword ptr [rdi + 0x1f0], rax
0033c6ce: mov    r9, qword ptr [rip + 0x395b763]          ; [0x3c97e38] meta:ʵʹʸʵˁˁʲʽʴʵʷ<ʾʸʸʻʷʸˁʾˁʽʸ>_TypeInfo
0033c6d5: mov    r10, qword ptr [rdx]
0033c6d8: movzx  eax, byte ptr [r9 + 0x130]
0033c6e0: cmp    byte ptr [r10 + 0x130], al
0033c6e7: jb     0x18033c6fa
0033c6e9: movzx  ecx, al
0033c6ec: mov    rax, qword ptr [r10 + 0xc8]
0033c6f3: cmp    qword ptr [rax + rcx*8 - 8], r9
0033c6f8: je     0x18033c6fd
0033c6fa: mov    r8d, esi
0033c6fd: test   r8d, r8d
0033c700: mov    rax, rsi
0033c703: cmovne rax, rdx
0033c707: lea    rcx, [rdi + 0x1f0]
0033c70e: mov    rdx, rax
0033c711: call   0x182f5fc00
0033c716: cmp    qword ptr [rdi + 0x1f0], rsi
0033c71d: je     0x18033ca78
0033c723: mov    rax, qword ptr [rip + 0x39b3166]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0033c72a: cmp    dword ptr [rax + 0xe0], esi
0033c730: jne    0x18033c741
0033c732: mov    rcx, rax
0033c735: call   0x182f60cf0
0033c73a: mov    rax, qword ptr [rip + 0x39b314f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0033c741: mov    rax, qword ptr [rax + 0xb8]
0033c748: mov    rcx, qword ptr [rax + 8]
0033c74c: test   rcx, rcx
0033c74f: je     0x18033cb30
0033c755: cmp    byte ptr [rcx + 0x73], sil
0033c759: je     0x18033c790
0033c75b: mov    rcx, qword ptr [rip + 0x39ca96e]         ; [0x3d070d0] meta:ˀʾʴʼʳʽʲʲʴˁʺ.ʹʸʶʲʶʵʾʲˀʷˀ_TypeInfo
0033c762: call   0x182f60c00
0033c767: mov    r8, qword ptr [rip + 0x3959b7a]          ; [0x3c962e8] metamethod:Method$ʼʺʸʻʿʷˁʾʴʴʿ.ˁʾʻʴʷˀʶˁʵʻʻ()
0033c76e: xor    r9d, r9d
0033c771: xor    edx, edx
0033c773: mov    rcx, rax
0033c776: mov    rbx, rax
0033c779: call   0x180102120                              ; LzrUkPAvUBBgsvLgPilAMZKkvEPN.uzJdlzBDoYVYRCgmPevdfmGnvNueA$$.ctor
0033c77e: mov    r8, qword ptr [rip + 0x399b753]          ; [0x3cd7ed8] metamethod:Method$BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>.ˀʹʲʹʶʺʳʲʸʿʹ()
0033c785: mov    rdx, rbx
0033c788: mov    rcx, rdi
0033c78b: call   0x18116b200                              ; BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>$$ˀʹʲʹʶʺʳʲʸʿʹ
0033c790: mov    dword ptr [rdi + 0x84], esi
0033c796: cmp    byte ptr [rip + 0x3bd182b], sil          ; [0x3f0dfc8] (bss)
0033c79d: jne    0x18033c7be
0033c79f: lea    rcx, [rip + 0x399b672]                   ; [0x3cd7e18] metamethod:Method$BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>.ʷʺʲʿʲʼʽʵʹʳʴ()
0033c7a6: call   0x182f609b0
0033c7ab: lea    rcx, [rip + 0x39ba1ee]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0033c7b2: call   0x182f609b0
0033c7b7: mov    byte ptr [rip + 0x3bd180a], 1            ; [0x3f0dfc8] (bss)
0033c7be: xorps  xmm0, xmm0
0033c7c1: mov    qword ptr [rsp + 0x20], rsi
0033c7c6: xorps  xmm1, xmm1
0033c7c9: mov    qword ptr [rsp + 0x60], rsi
0033c7ce: mov    rdx, rdi
0033c7d1: mov    qword ptr [rsp + 0x28], rdi
0033c7d6: lea    rcx, [rsp + 0x28]
0033c7db: movdqu xmmword ptr [rsp + 0x30], xmm0
0033c7e1: movdqu xmmword ptr [rsp + 0x40], xmm1
0033c7e7: movdqu xmmword ptr [rsp + 0x50], xmm0
0033c7ed: call   0x182f5fc00
0033c7f2: mov    rdx, rdi
0033c7f5: mov    qword ptr [rsp + 0x20], rdi
0033c7fa: lea    rcx, [rsp + 0x20]
0033c7ff: call   0x182f5fc00
0033c804: xor    edx, edx
0033c806: mov    qword ptr [rsp + 0x30], rsi
0033c80b: lea    rcx, [rsp + 0x30]
0033c810: call   0x182f5fc00
0033c815: mov    rax, qword ptr [rdi + 0x88]
0033c81c: test   rax, rax
0033c81f: je     0x18033cb2a
0033c825: mov    rax, qword ptr [rax + 0x10]
0033c829: test   rax, rax
0033c82c: je     0x18033cb2a
0033c832: cmp    byte ptr [rax + 0x10], 9
0033c836: mov    rax, qword ptr [rdi + 0x88]
0033c83d: sete   byte ptr [rsp + 0x38]
0033c842: test   rax, rax
0033c845: je     0x18033cb2a
0033c84b: mov    rcx, qword ptr [rax + 0x10]
0033c84f: test   rcx, rcx
0033c852: je     0x18033cb2a
0033c858: mov    rax, qword ptr [rcx + 0xd8]
0033c85f: test   rax, rax
0033c862: je     0x18033cb2a
0033c868: mov    eax, dword ptr [rax + 0x10]
0033c86b: mov    dword ptr [rsp + 0x3c], eax
0033c86f: mov    rax, qword ptr [rdi + 0x88]
0033c876: test   rax, rax
0033c879: je     0x18033cb2a
0033c87f: mov    rax, qword ptr [rax + 0x10]
0033c883: test   rax, rax
0033c886: je     0x18033cb2a
0033c88c: mov    rcx, qword ptr [rax + 0x38]
0033c890: test   rcx, rcx
0033c893: je     0x18033cb2a
0033c899: movaps xmmword ptr [rsp + 0x170], xmm7
0033c8a1: xor    edx, edx
0033c8a3: movaps xmmword ptr [rsp + 0x160], xmm8
0033c8ac: movaps xmmword ptr [rsp + 0x150], xmm9
0033c8b5: movaps xmmword ptr [rsp + 0x140], xmm10
0033c8be: call   0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
0033c8c3: mov    r8, qword ptr [rip + 0x399b54e]          ; [0x3cd7e18] metamethod:Method$BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>.ʷʺʲʿʲʼʽʵʹʳʴ()
0033c8ca: lea    rcx, [rbp - 0x80]
0033c8ce: mov    rdx, rdi
0033c8d1: mov    byte ptr [rsp + 0x40], al
0033c8d5: call   0x1811676a0                              ; BasePlayer<ʾʸʸʻʷʸˁʾˁʽʸ>$$ʷʺʲʿʲʼʽʵʹʳʴ
0033c8da: movups xmm0, xmmword ptr [rbp - 0x78]
0033c8de: mov    rax, qword ptr [rbp - 0x80]
0033c8e2: lea    rcx, [rsp + 0x48]
0033c8e7: movsd  xmm1, qword ptr [rbp - 0x68]
0033c8ec: xor    edx, edx
0033c8ee: movups xmmword ptr [rsp + 0x50], xmm0
0033c8f3: mov    qword ptr [rsp + 0x48], rax
0033c8f8: movsd  qword ptr [rsp + 0x60], xmm1
0033c8fe: call   0x182f5fc00
0033c903: mov    rcx, qword ptr [rip + 0x395c74e]         ; [0x3c99058] meta:ʺʺʸʶʿʵˀʸʲˀʲ_TypeInfo
0033c90a: movups xmm6, xmmword ptr [rsp + 0x20]
0033c90f: movups xmm7, xmmword ptr [rsp + 0x30]
0033c914: movups xmm8, xmmword ptr [rsp + 0x40]
0033c91a: movups xmm9, xmmword ptr [rsp + 0x50]
0033c920: movsd  xmm10, qword ptr [rsp + 0x60]
0033c927: call   0x182f60c00
0033c92c: movups xmm0, xmmword ptr [r15]
0033c930: xor    r9d, r9d
0033c933: lea    r8, [rsp + 0x20]
0033c938: movups xmm1, xmmword ptr [r15 + 0x10]
0033c93d: lea    rdx, [rbp - 0x60]
0033c941: mov    rcx, rax
0033c944: movups xmmword ptr [rbp - 0x60], xmm0
0033c948: mov    rsi, rax
0033c94b: movups xmm0, xmmword ptr [r15 + 0x20]
0033c950: movups xmmword ptr [rbp - 0x50], xmm1
0033c954: movups xmm1, xmmword ptr [r15 + 0x30]
0033c959: movups xmmword ptr [rbp - 0x40], xmm0
0033c95d: movups xmm0, xmmword ptr [r15 + 0x40]
0033c962: movups xmmword ptr [rbp - 0x30], xmm1
0033c966: movups xmm1, xmmword ptr [r15 + 0x50]
0033c96b: movups xmmword ptr [rbp - 0x20], xmm0
0033c96f: movups xmm0, xmmword ptr [r15 + 0x60]
0033c974: movups xmmword ptr [rbp - 0x10], xmm1
0033c978: movups xmm1, xmmword ptr [r15 + 0x70]
0033c97d: movups xmmword ptr [rbp], xmm0
0033c981: movups xmm0, xmmword ptr [r15 + 0x80]
0033c989: movups xmmword ptr [rbp + 0x10], xmm1
0033c98d: movups xmm1, xmmword ptr [r15 + 0x90]
0033c995: movups xmmword ptr [rbp + 0x20], xmm0
0033c999: movups xmmword ptr [rbp + 0x30], xmm1
0033c99d: movaps xmmword ptr [rsp + 0x20], xmm6
0033c9a2: movaps xmmword ptr [rsp + 0x30], xmm7
0033c9a7: movaps xmmword ptr [rsp + 0x40], xmm8
0033c9ad: movaps xmmword ptr [rsp + 0x50], xmm9
0033c9b3: movsd  qword ptr [rsp + 0x60], xmm10
0033c9ba: call   0x1820df680                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$.ctor
0033c9bf: mov    rax, qword ptr [rip + 0x395b95a]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0033c9c6: movaps xmm10, xmmword ptr [rsp + 0x140]
0033c9cf: movaps xmm9, xmmword ptr [rsp + 0x150]
0033c9d8: movaps xmm8, xmmword ptr [rsp + 0x160]
0033c9e1: mov    rcx, qword ptr [rax + 0xb8]
0033c9e8: movaps xmm7, xmmword ptr [rsp + 0x170]
0033c9f0: mov    rcx, qword ptr [rcx + 0x100]
0033c9f7: test   rcx, rcx
0033c9fa: je     0x18033cb30
0033ca00: xor    edx, edx
0033ca02: call   0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
0033ca07: test   al, al
0033ca09: je     0x18033caa3
0033ca0f: xor    edx, edx
0033ca11: lea    rcx, [rbp - 0x80]
0033ca15: call   0x1828717c0                              ; UnityEngine.Screen$$get_currentResolution
0033ca1a: xor    edx, edx
0033ca1c: lea    rcx, [rsp + 0x70]
0033ca21: movups xmm0, xmmword ptr [rax]
0033ca24: movups xmmword ptr [rsp + 0x70], xmm0
0033ca29: call   0x181bbd830                              ; UnityEngine.TextCore.FaceInfo$$get_familyName
0033ca2e: xor    edx, edx
0033ca30: lea    rcx, [rbp - 0x80]
0033ca34: mov    rbx, rax
0033ca37: call   0x1828717c0                              ; UnityEngine.Screen$$get_currentResolution
0033ca3c: xorps  xmm1, xmm1
0033ca3f: lea    rcx, [rsp + 0x70]
0033ca44: xor    edx, edx
0033ca46: movups xmm0, xmmword ptr [rax]
0033ca49: mov    eax, ebx
0033ca4b: movups xmmword ptr [rsp + 0x70], xmm0
0033ca50: cvtsi2sd xmm1, rax
0033ca55: cvtpd2ps xmm6, xmm1
0033ca59: call   0x181bbd830                              ; UnityEngine.TextCore.FaceInfo$$get_familyName
