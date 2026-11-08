0035c110: mov    byte ptr [rsp + 0x20], r9b
0035c115: mov    byte ptr [rsp + 0x10], dl
0035c119: mov    qword ptr [rsp + 8], rcx
0035c11e: push   rbp
0035c11f: push   rbx
0035c120: push   rdi
0035c121: push   r12
0035c123: push   r14
0035c125: push   r15
0035c127: lea    rbp, [rsp - 0x27]
0035c12c: sub    rsp, 0xd8
0035c133: cmp    byte ptr [rip + 0x3bb1f0f], 0            ; [0x3f0e049] (bss)
0035c13a: movzx  r15d, r8b
0035c13e: movzx  r14d, dl
0035c142: mov    rbx, rcx
0035c145: jne    0x18035c1c6
0035c147: lea    rcx, [rip + 0x397c202]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0035c14e: call   0x182f609b0
0035c153: lea    rcx, [rip + 0x3993736]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c15a: call   0x182f609b0
0035c15f: lea    rcx, [rip + 0x39acd9a]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
0035c166: call   0x182f609b0
0035c16b: lea    rcx, [rip + 0x394d27e]                   ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
0035c172: call   0x182f609b0
0035c177: lea    rcx, [rip + 0x39b2252]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
0035c17e: call   0x182f609b0
0035c183: lea    rcx, [rip + 0x39381b6]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0035c18a: call   0x182f609b0
0035c18f: lea    rcx, [rip + 0x393c18a]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0035c196: call   0x182f609b0
0035c19b: lea    rcx, [rip + 0x396709e]                   ; [0x3cc3240] str:'Failed to load chart data: '
0035c1a2: call   0x182f609b0
0035c1a7: lea    rcx, [rip + 0x3967152]                   ; [0x3cc3300] str:'Failed to load chart: '
0035c1ae: call   0x182f609b0
0035c1b3: lea    rcx, [rip + 0x3969646]                   ; [0x3cc5800] str:' needs to be rescanned!'
0035c1ba: call   0x182f609b0
0035c1bf: mov    byte ptr [rip + 0x3bb1e83], 1            ; [0x3f0e049] (bss)
0035c1c6: xor    r12d, r12d
0035c1c9: lea    rcx, [rbp - 1]
0035c1cd: xorps  xmm0, xmm0
0035c1d0: mov    qword ptr [rbp - 0x49], r12
0035c1d4: xor    r9d, r9d
0035c1d7: mov    byte ptr [rbx + 0x59], r12b
0035c1db: movzx  r8d, r14b
0035c1df: mov    rdx, rbx
0035c1e2: movups xmmword ptr [rbp - 0x21], xmm0
0035c1e6: call   0x18035fa10                              ; SongEntry$$ˁˀˀʺʴʶʾʿʺʾʷ
0035c1eb: mov    rdi, qword ptr [rax]
0035c1ee: mov    qword ptr [rbp - 0x41], rdi
0035c1f2: test   rdi, rdi
0035c1f5: je     0x18035c918
0035c1fb: mov    qword ptr [rsp + 0x120], rsi
0035c203: mov    esi, dword ptr [rax + 8]
0035c206: mov    dword ptr [rbp - 0x4d], esi
0035c209: mov    rcx, qword ptr [rip + 0x39accf0]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
0035c210: cmp    qword ptr [rcx + 0x38], r12
0035c214: jne    0x18035c21b
0035c216: call   0x182f657d0
0035c21b: mov    dword ptr [rbp - 0x25], r12d
0035c21f: cmp    esi, dword ptr [rdi + 0x18]
0035c222: jbe    0x18035c22b
0035c224: xor    ecx, ecx
0035c226: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0035c22b: mov    r8, qword ptr [rip + 0x394d1be]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
0035c232: lea    rax, [rdi + 0x20]
0035c236: mov    qword ptr [rbp - 0x31], rax
0035c23a: lea    rdx, [rbp - 0x31]
0035c23e: mov    dword ptr [rbp - 0x29], esi
0035c241: lea    rcx, [rbp - 0x11]
0035c245: movaps xmm0, xmmword ptr [rbp - 0x31]
0035c249: movdqa xmmword ptr [rbp - 0x31], xmm0
0035c24e: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
0035c253: test   r14b, r14b
0035c256: jne    0x18035c357
0035c25c: cmp    byte ptr [rbx + 0xb2], r12b
0035c263: jne    0x18035c357
0035c269: test   r15b, r15b
0035c26c: jne    0x18035c357
0035c272: xorps  xmm0, xmm0
0035c275: lea    rcx, [rbp - 0x21]
0035c279: xor    r9d, r9d
0035c27c: mov    r8d, esi
0035c27f: mov    rdx, rdi
0035c282: movups xmmword ptr [rbp - 0x21], xmm0
0035c286: call   0x1820fa8f0                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʽʳʸʿʷʽʼˁʸʻʼ
0035c28b: movups xmm0, xmmword ptr [rbx + 0x38]
0035c28f: xor    r8d, r8d
0035c292: lea    rdx, [rbp - 0x31]
0035c296: lea    rcx, [rbp - 0x21]
0035c29a: movaps xmmword ptr [rbp - 0x31], xmm0
0035c29e: call   0x1820f9910                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$Equals
0035c2a3: test   al, al
0035c2a5: jne    0x18035c357
0035c2ab: mov    rax, qword ptr [rip + 0x39935de]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c2b2: cmp    dword ptr [rax + 0xe0], r12d
0035c2b9: jne    0x18035c2ca
0035c2bb: mov    rcx, rax
0035c2be: call   0x182f60cf0
0035c2c3: mov    rax, qword ptr [rip + 0x39935c6]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c2ca: mov    rax, qword ptr [rax + 0xb8]
0035c2d1: mov    rcx, qword ptr [rax + 8]
0035c2d5: test   rcx, rcx
0035c2d8: je     0x18035c96f
0035c2de: mov    byte ptr [rcx + 0x78], 1
0035c2e2: mov    rax, qword ptr [rip + 0x39935a7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c2e9: mov    rcx, qword ptr [rax + 0xb8]
0035c2f0: mov    rax, qword ptr [rcx + 8]
0035c2f4: test   rax, rax
0035c2f7: je     0x18035c96f
0035c2fd: mov    rax, qword ptr [rax + 0x28]
0035c301: test   rax, rax
0035c304: je     0x18035c96f
0035c30a: mov    rax, qword ptr [rax + 0x88]
0035c311: test   rax, rax
0035c314: je     0x18035c96f
0035c31a: cmp    dword ptr [rax + 0x18], r12d
0035c31e: jbe    0x18035c969
0035c324: mov    rcx, qword ptr [rax + 0x20]
0035c328: test   rcx, rcx
0035c32b: je     0x18035c96f
0035c331: mov    rdx, qword ptr [rip + 0x39694c8]         ; [0x3cc5800] str:' needs to be rescanned!'
0035c338: xor    r8d, r8d
0035c33b: mov    rcx, qword ptr [rcx + 0x10]
0035c33f: call   0x18197f200                              ; System.String$$Concat
0035c344: mov    rcx, qword ptr [rip + 0x397c005]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0035c34b: cmp    dword ptr [rcx + 0xe0], r12d
0035c352: jmp    0x18035c4fe
0035c357: mov    rax, qword ptr [rip + 0x3993532]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c35e: cmp    dword ptr [rax + 0xe0], r12d
0035c365: jne    0x18035c376
0035c367: mov    rcx, rax
0035c36a: call   0x182f60cf0
0035c36f: mov    rax, qword ptr [rip + 0x399351a]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0035c376: mov    rax, qword ptr [rax + 0xb8]
0035c37d: mov    rbx, qword ptr [rax + 8]
0035c381: test   rbx, rbx
0035c384: je     0x18035c96f
0035c38a: mov    rbx, qword ptr [rbx + 0x30]
0035c38e: test   rbx, rbx
0035c391: je     0x18035c96f
0035c397: mov    rax, qword ptr [rip + 0x393bf82]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0035c39e: mov    rcx, qword ptr [rax + 0xb8]
0035c3a5: mov    rcx, qword ptr [rcx + 0x10]
0035c3a9: test   rcx, rcx
0035c3ac: je     0x18035c96f
0035c3b2: movzx  eax, byte ptr [rbx + 0x38]
0035c3b6: xor    edx, edx
0035c3b8: mov    qword ptr [rsp + 0xd0], r13
0035c3c0: movzx  r13d, byte ptr [rbx + 0x39]
0035c3c5: mov    byte ptr [rbp - 0x51], al
0035c3c8: movaps xmmword ptr [rsp + 0xc0], xmm6
0035c3d0: call   0x18210c090                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʸʺˀʳˁʽʸʺʲʻˁ
0035c3d5: mov    r12d, dword ptr [rbx + 0x18]
0035c3d9: movaps xmm6, xmm0
0035c3dc: movzx  r14d, byte ptr [rbx + 0x10]
0035c3e1: mov    esi, dword ptr [rbx + 0x14]
0035c3e4: mov    edi, dword ptr [rbx + 0x1c]
0035c3e7: mov    rcx, qword ptr [rip + 0x39b1fe2]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
0035c3ee: mov    ebx, dword ptr [rbx + 0x20]
0035c3f1: call   0x182f60c00
0035c3f6: movzx  edx, byte ptr [rbp - 0x51]
0035c3fa: mov    r15, rax
0035c3fd: movzx  eax, byte ptr [rbp + 0x67]
0035c401: movaps xmm3, xmm6
0035c404: mov    qword ptr [rsp + 0x50], 0
0035c40d: movzx  r8d, r13b
0035c411: mov    byte ptr [rsp + 0x48], al
0035c415: mov    rcx, r15
0035c418: mov    dword ptr [rsp + 0x40], ebx
0035c41c: mov    dword ptr [rsp + 0x38], edi
0035c420: mov    dword ptr [rsp + 0x30], esi
0035c424: mov    byte ptr [rsp + 0x28], r14b
0035c429: mov    dword ptr [rsp + 0x20], r12d
0035c42e: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
0035c433: mov    rbx, qword ptr [rbp + 0x5f]
0035c437: xor    edx, edx
0035c439: mov    rcx, rbx
0035c43c: mov    qword ptr [rbp - 0x49], r15
0035c440: call   0x1803607d0                              ; SongEntry$$get_IsMIDIChart
0035c445: mov    r13, qword ptr [rsp + 0xd0]
0035c44d: test   al, al
0035c44f: jne    0x18035c4a0
0035c451: movaps xmm0, xmmword ptr [rbp - 0x11]
0035c455: xor    r9d, r9d
0035c458: movdqa xmmword ptr [rbp - 0x11], xmm0
0035c45d: lea    r8, [rbp - 0x11]
0035c461: mov    rdx, rbx
0035c464: lea    rcx, [rbp - 1]
0035c468: call   0x180356f30                              ; SongEntry$$ʳʻˀʿʾʼʴʳʷʵʺ
0035c46d: cmp    byte ptr [rbp + 0x77], 0
0035c471: movups xmm6, xmmword ptr [rax]
0035c474: je     0x18035c489
0035c476: xor    r8d, r8d
0035c479: movaps xmmword ptr [rbp - 0x11], xmm6
0035c47d: lea    rdx, [rbp - 0x11]
0035c481: mov    rcx, rbx
0035c484: call   0x18035c980                              ; SongEntry$$ʿʳʴʾʽʻʴʽʿʵʽ
0035c489: xor    r8d, r8d
0035c48c: movdqa xmmword ptr [rbp - 0x11], xmm6
0035c491: lea    rdx, [rbp - 0x49]
0035c495: lea    rcx, [rbp - 0x11]
0035c499: call   0x182161690                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʳʳʽʲʵʽˀʵʶʹʽ
0035c49e: jmp    0x18035c4c8
0035c4a0: mov    rcx, qword ptr [rip + 0x3937e99]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0035c4a7: cmp    dword ptr [rcx + 0xe0], 0
0035c4ae: jne    0x18035c4b5
0035c4b0: call   0x182f60cf0
0035c4b5: mov    edx, dword ptr [rbp - 0x4d]
0035c4b8: lea    r8, [rbp - 0x49]
0035c4bc: mov    rcx, qword ptr [rbp - 0x41]
0035c4c0: xor    r9d, r9d
0035c4c3: call   0x182157780                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʻʵˀʸʲʶˀˁʲʵˁ
0035c4c8: movaps xmm6, xmmword ptr [rsp + 0xc0]
0035c4d0: test   al, al
0035c4d2: jne    0x18035c52d
0035c4d4: xor    edx, edx
0035c4d6: mov    rcx, rbx
0035c4d9: call   0x1803602c0                              ; SongEntry$$get_ChartPath
0035c4de: mov    rcx, qword ptr [rip + 0x3966e1b]         ; [0x3cc3300] str:'Failed to load chart: '
0035c4e5: xor    r8d, r8d
0035c4e8: mov    rdx, rax
0035c4eb: call   0x18197f200                              ; System.String$$Concat
0035c4f0: mov    rcx, qword ptr [rip + 0x397be59]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0035c4f7: cmp    dword ptr [rcx + 0xe0], 0
0035c4fe: mov    rbx, rax
0035c501: jne    0x18035c508
0035c503: call   0x182f60cf0
0035c508: xor    edx, edx
0035c50a: mov    rcx, rbx
0035c50d: call   0x18285fd00                              ; UnityEngine.Debug$$Log
0035c512: xor    eax, eax
0035c514: mov    rsi, qword ptr [rsp + 0x120]
0035c51c: add    rsp, 0xd8
0035c523: pop    r15
0035c525: pop    r14
0035c527: pop    r12
0035c529: pop    rdi
0035c52a: pop    rbx
0035c52b: pop    rbp
0035c52c: ret    
0035c52d: mov    rdi, qword ptr [rbp - 0x49]
0035c531: test   rdi, rdi
0035c534: je     0x18035c96f
0035c53a: cmp    byte ptr [rip + 0x3bb1b1b], 0            ; [0x3f0e05c] (bss)
0035c541: mov    rdi, qword ptr [rdi + 0x30]
0035c545: jne    0x18035c55a
0035c547: lea    rcx, [rip + 0x3943142]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c54e: call   0x182f609b0
0035c553: mov    byte ptr [rip + 0x3bb1b02], 1            ; [0x3f0e05c] (bss)
0035c55a: mov    rax, qword ptr [rbx + 0x88]
0035c561: test   rax, rax
0035c564: je     0x18035c96f
0035c56a: cmp    dword ptr [rax + 0x18], 0
0035c56e: jbe    0x18035c969
0035c574: mov    rcx, qword ptr [rax + 0x20]
0035c578: test   rcx, rcx
0035c57b: je     0x18035c96f
0035c581: mov    rsi, qword ptr [rcx + 0x10]
0035c585: mov    rcx, qword ptr [rip + 0x3943104]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c58c: cmp    dword ptr [rcx + 0xe0], 0
0035c593: jne    0x18035c59a
0035c595: call   0x182f60cf0
0035c59a: xor    r8d, r8d
0035c59d: xor    edx, edx
0035c59f: mov    rcx, rsi
0035c5a2: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c5a7: test   rdi, rdi
0035c5aa: je     0x18035c96f
0035c5b0: lea    rcx, [rdi + 0x70]
0035c5b4: mov    qword ptr [rdi + 0x70], rax
0035c5b8: mov    rdx, rax
0035c5bb: call   0x182f5fc00
0035c5c0: mov    rdi, qword ptr [rbp - 0x49]
0035c5c4: test   rdi, rdi
0035c5c7: je     0x18035c96f
0035c5cd: cmp    byte ptr [rip + 0x3bb1a71], 0            ; [0x3f0e045] (bss)
0035c5d4: mov    rdi, qword ptr [rdi + 0x30]
0035c5d8: jne    0x18035c5ed
0035c5da: lea    rcx, [rip + 0x39430af]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c5e1: call   0x182f609b0
0035c5e6: mov    byte ptr [rip + 0x3bb1a58], 1            ; [0x3f0e045] (bss)
0035c5ed: mov    rax, qword ptr [rbx + 0x88]
0035c5f4: test   rax, rax
0035c5f7: je     0x18035c96f
0035c5fd: cmp    dword ptr [rax + 0x18], 1
0035c601: jbe    0x18035c969
0035c607: mov    rcx, qword ptr [rax + 0x28]
0035c60b: test   rcx, rcx
0035c60e: je     0x18035c96f
0035c614: mov    rsi, qword ptr [rcx + 0x10]
0035c618: mov    rcx, qword ptr [rip + 0x3943071]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c61f: cmp    dword ptr [rcx + 0xe0], 0
0035c626: jne    0x18035c62d
0035c628: call   0x182f60cf0
0035c62d: xor    r8d, r8d
0035c630: xor    edx, edx
0035c632: mov    rcx, rsi
0035c635: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c63a: test   rdi, rdi
0035c63d: je     0x18035c96f
0035c643: lea    rcx, [rdi + 0x78]
0035c647: mov    qword ptr [rdi + 0x78], rax
0035c64b: mov    rdx, rax
0035c64e: call   0x182f5fc00
0035c653: mov    rdi, qword ptr [rbp - 0x49]
0035c657: test   rdi, rdi
0035c65a: je     0x18035c96f
0035c660: cmp    byte ptr [rip + 0x3bb19ef], 0            ; [0x3f0e056] (bss)
0035c667: mov    rdi, qword ptr [rdi + 0x30]
0035c66b: jne    0x18035c680
0035c66d: lea    rcx, [rip + 0x394301c]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c674: call   0x182f609b0
0035c679: mov    byte ptr [rip + 0x3bb19d6], 1            ; [0x3f0e056] (bss)
0035c680: mov    rax, qword ptr [rbx + 0x88]
0035c687: test   rax, rax
0035c68a: je     0x18035c96f
0035c690: cmp    dword ptr [rax + 0x18], 2
0035c694: jbe    0x18035c969
0035c69a: mov    rcx, qword ptr [rax + 0x30]
0035c69e: test   rcx, rcx
0035c6a1: je     0x18035c96f
0035c6a7: mov    rsi, qword ptr [rcx + 0x10]
0035c6ab: mov    rcx, qword ptr [rip + 0x3942fde]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c6b2: cmp    dword ptr [rcx + 0xe0], 0
0035c6b9: jne    0x18035c6c0
0035c6bb: call   0x182f60cf0
0035c6c0: xor    r8d, r8d
0035c6c3: xor    edx, edx
0035c6c5: mov    rcx, rsi
0035c6c8: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c6cd: test   rdi, rdi
0035c6d0: je     0x18035c96f
0035c6d6: lea    rcx, [rdi + 0x80]
0035c6dd: mov    qword ptr [rdi + 0x80], rax
0035c6e4: mov    rdx, rax
0035c6e7: call   0x182f5fc00
0035c6ec: mov    rdi, qword ptr [rbp - 0x49]
0035c6f0: test   rdi, rdi
0035c6f3: je     0x18035c96f
0035c6f9: cmp    byte ptr [rip + 0x3bb1943], 0            ; [0x3f0e043] (bss)
0035c700: mov    rdi, qword ptr [rdi + 0x30]
0035c704: jne    0x18035c719
0035c706: lea    rcx, [rip + 0x3942f83]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c70d: call   0x182f609b0
0035c712: mov    byte ptr [rip + 0x3bb192a], 1            ; [0x3f0e043] (bss)
0035c719: mov    rax, qword ptr [rbx + 0x88]
0035c720: test   rax, rax
0035c723: je     0x18035c96f
0035c729: cmp    dword ptr [rax + 0x18], 3
0035c72d: jbe    0x18035c969
0035c733: mov    rcx, qword ptr [rax + 0x38]
0035c737: test   rcx, rcx
0035c73a: je     0x18035c96f
0035c740: mov    rsi, qword ptr [rcx + 0x10]
0035c744: mov    rcx, qword ptr [rip + 0x3942f45]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c74b: cmp    dword ptr [rcx + 0xe0], 0
0035c752: jne    0x18035c759
0035c754: call   0x182f60cf0
0035c759: xor    r8d, r8d
0035c75c: xor    edx, edx
0035c75e: mov    rcx, rsi
0035c761: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c766: test   rdi, rdi
0035c769: je     0x18035c96f
0035c76f: lea    rcx, [rdi + 0x88]
0035c776: mov    qword ptr [rdi + 0x88], rax
0035c77d: mov    rdx, rax
0035c780: call   0x182f5fc00
0035c785: mov    rdi, qword ptr [rbp - 0x49]
0035c789: test   rdi, rdi
0035c78c: je     0x18035c96f
0035c792: cmp    byte ptr [rip + 0x3bb18b8], 0            ; [0x3f0e051] (bss)
0035c799: mov    rdi, qword ptr [rdi + 0x30]
0035c79d: jne    0x18035c7b2
0035c79f: lea    rcx, [rip + 0x3942eea]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c7a6: call   0x182f609b0
0035c7ab: mov    byte ptr [rip + 0x3bb189f], 1            ; [0x3f0e051] (bss)
0035c7b2: mov    rax, qword ptr [rbx + 0x88]
0035c7b9: test   rax, rax
0035c7bc: je     0x18035c96f
0035c7c2: cmp    dword ptr [rax + 0x18], 4
0035c7c6: jbe    0x18035c969
0035c7cc: mov    rcx, qword ptr [rax + 0x40]
0035c7d0: test   rcx, rcx
0035c7d3: je     0x18035c96f
0035c7d9: mov    rsi, qword ptr [rcx + 0x10]
0035c7dd: mov    rcx, qword ptr [rip + 0x3942eac]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c7e4: cmp    dword ptr [rcx + 0xe0], 0
0035c7eb: jne    0x18035c7f2
0035c7ed: call   0x182f60cf0
0035c7f2: xor    r8d, r8d
0035c7f5: xor    edx, edx
0035c7f7: mov    rcx, rsi
0035c7fa: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c7ff: test   rdi, rdi
0035c802: je     0x18035c96f
0035c808: lea    rcx, [rdi + 0x90]
0035c80f: mov    qword ptr [rdi + 0x90], rax
0035c816: mov    rdx, rax
0035c819: call   0x182f5fc00
0035c81e: mov    rdi, qword ptr [rbp - 0x49]
0035c822: test   rdi, rdi
0035c825: je     0x18035c96f
0035c82b: cmp    byte ptr [rip + 0x3bb1827], 0            ; [0x3f0e059] (bss)
0035c832: mov    rdi, qword ptr [rdi + 0x30]
0035c836: jne    0x18035c84b
0035c838: lea    rcx, [rip + 0x3942e51]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c83f: call   0x182f609b0
0035c844: mov    byte ptr [rip + 0x3bb180e], 1            ; [0x3f0e059] (bss)
0035c84b: mov    rax, qword ptr [rbx + 0x88]
0035c852: test   rax, rax
0035c855: je     0x18035c96f
0035c85b: cmp    dword ptr [rax + 0x18], 5
0035c85f: jbe    0x18035c969
0035c865: mov    rcx, qword ptr [rax + 0x48]
0035c869: test   rcx, rcx
0035c86c: je     0x18035c96f
0035c872: mov    rsi, qword ptr [rcx + 0x10]
0035c876: mov    rcx, qword ptr [rip + 0x3942e13]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
0035c87d: cmp    dword ptr [rcx + 0xe0], 0
0035c884: jne    0x18035c88b
0035c886: call   0x182f60cf0
0035c88b: xor    r8d, r8d
0035c88e: xor    edx, edx
0035c890: mov    rcx, rsi
0035c893: call   0x18216b0d0                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʷʷʲʴʿʾʾʵʳʳʾ
0035c898: test   rdi, rdi
0035c89b: je     0x18035c96f
0035c8a1: lea    rcx, [rdi + 0x98]
0035c8a8: mov    qword ptr [rdi + 0x98], rax
0035c8af: mov    rdx, rax
0035c8b2: call   0x182f5fc00
0035c8b7: mov    rcx, qword ptr [rbp - 0x49]
0035c8bb: test   rcx, rcx
0035c8be: je     0x18035c96f
0035c8c4: mov    rcx, qword ptr [rcx + 0x30]
0035c8c8: test   rcx, rcx
0035c8cb: je     0x18035c96f
0035c8d1: mov    rdx, qword ptr [rbx + 0x28]
0035c8d5: mov    qword ptr [rcx + 0xa0], rdx
0035c8dc: add    rcx, 0xa0
0035c8e3: call   0x182f5fc00
0035c8e8: mov    rcx, qword ptr [rbp - 0x49]
0035c8ec: test   rcx, rcx
0035c8ef: je     0x18035c96f
0035c8f1: mov    rcx, qword ptr [rcx + 0x30]
0035c8f5: test   rcx, rcx
0035c8f8: je     0x18035c96f
0035c8fa: mov    eax, dword ptr [rbx + 0x80]
0035c900: mov    dword ptr [rcx + 0xa8], eax
0035c906: mov    rax, qword ptr [rbp - 0x49]
0035c90a: test   rax, rax
0035c90d: je     0x18035c96f
0035c90f: mov    rax, qword ptr [rax + 0x30]
0035c913: jmp    0x18035c514
0035c918: xor    edx, edx
0035c91a: mov    rcx, rbx
0035c91d: call   0x1803602c0                              ; SongEntry$$get_ChartPath
0035c922: mov    rcx, qword ptr [rip + 0x3966917]         ; [0x3cc3240] str:'Failed to load chart data: '
0035c929: xor    r8d, r8d
0035c92c: mov    rdx, rax
0035c92f: call   0x18197f200                              ; System.String$$Concat
0035c934: mov    rcx, qword ptr [rip + 0x397ba15]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
0035c93b: mov    rbx, rax
0035c93e: cmp    dword ptr [rcx + 0xe0], r12d
0035c945: jne    0x18035c94c
0035c947: call   0x182f60cf0
0035c94c: xor    edx, edx
0035c94e: mov    rcx, rbx
0035c951: call   0x18285fd00                              ; UnityEngine.Debug$$Log
0035c956: xor    eax, eax
0035c958: add    rsp, 0xd8
0035c95f: pop    r15
0035c961: pop    r14
0035c963: pop    r12
0035c965: pop    rdi
0035c966: pop    rbx
0035c967: pop    rbp
0035c968: ret    
0035c969: call   0x182f60c40
0035c96e: int3   
0035c96f: call   0x182f60c50
0035c974: int3   
0035c975: int3   
