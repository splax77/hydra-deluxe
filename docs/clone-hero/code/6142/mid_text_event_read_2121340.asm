02121340: push   rbp
02121342: push   rbx
02121343: push   rsi
02121344: push   rdi
02121345: push   r14
02121347: push   r15
02121349: sub    rsp, 0x58
0212134d: lea    rbp, [rsp + 0x20]
02121352: cmp    byte ptr [rip + 0x1df8221], 0            ; [0x3f1957a] (bss)
02121359: mov    rsi, r8
0212135c: movaps xmmword ptr [rbp + 0x20], xmm6
02121360: mov    r14, rdx
02121363: movsxd rbx, r9d
02121366: mov    rdi, rcx
02121369: jne    0x1821213ba
0212136b: lea    rcx, [rip + 0x1b87896]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
02121372: call   0x182f609b0
02121377: lea    rcx, [rip + 0x1b88072]                   ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
0212137e: call   0x182f609b0
02121383: lea    rcx, [rip + 0x1bb5d4e]                   ; [0x3cd70d8] meta:string_TypeInfo
0212138a: call   0x182f609b0
0212138f: lea    rcx, [rip + 0x1b7a732]                   ; [0x3c9bac8] meta:ʽʳʸʼʳʷʻʷʳʷˁ_TypeInfo
02121396: call   0x182f609b0
0212139b: lea    rcx, [rip + 0x1beabe6]                   ; [0x3d0bf88] str:'Text event cannot be read since the size is negative number.'
021213a2: call   0x182f609b0
021213a7: lea    rcx, [rip + 0x1b90602]                   ; [0x3cb19b0] str:'size'
021213ae: call   0x182f609b0
021213b3: mov    byte ptr [rip + 0x1df81c0], 1            ; [0x3f1957a] (bss)
021213ba: mov    r8, qword ptr [rip + 0x1beabc7]          ; [0x3d0bf88] str:'Text event cannot be read since the size is negative number.'
021213c1: xor    r9d, r9d
021213c4: mov    rcx, qword ptr [rip + 0x1b905e5]         ; [0x3cb19b0] str:'size'
021213cb: mov    edx, ebx
021213cd: call   0x1820fc6d0                              ; ʸʽʾʾʺˁʸʵʿʲʼ$$ʵˀʹˁʺʺʵʽʹʴʺ
021213d2: test   ebx, ebx
021213d4: je     0x1821214e1
021213da: lea    rcx, [rbx + 0xf]
021213de: cmp    rcx, rbx
021213e1: ja     0x1821213ed
021213e3: movabs rcx, 0xffffffffffffff0
021213ed: and    rcx, 0xfffffffffffffff0
021213f1: mov    rax, rcx
021213f4: call   0x18302b960
021213f9: sub    rsp, rcx
021213fc: mov    r8, rbx
021213ff: xor    edx, edx
02121401: lea    r15, [rsp + 0x20]
02121406: mov    rcx, r15
02121409: call   0x18305d9d0
0212140e: mov    dword ptr [rbp + 0xc], 0
02121415: test   ebx, ebx
02121417: jns    0x182121420
02121419: xor    ecx, ecx
0212141b: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02121420: mov    qword ptr [rbp], r15
02121424: mov    dword ptr [rbp + 8], ebx
02121427: test   r14, r14
0212142a: je     0x182121510
02121430: movaps xmm6, xmmword ptr [rbp]
02121434: lea    rdx, [rbp + 0x10]
02121438: xor    r8d, r8d
0212143b: movdqa xmmword ptr [rbp + 0x10], xmm6
02121440: mov    rcx, r14
02121443: call   0x1820fb700
02121448: test   rsi, rsi
0212144b: je     0x182121510
02121451: mov    rbx, qword ptr [rsi + 0x20]
02121455: test   rbx, rbx
02121458: je     0x182121475
0212145a: mov    r8, qword ptr [rip + 0x1b87f8f]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
02121461: lea    rdx, [rbp]
02121465: lea    rcx, [rbp + 0x10]
02121469: movdqa xmmword ptr [rbp], xmm6
0212146e: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
02121473: jmp    0x1821214c0
02121475: mov    rax, qword ptr [rip + 0x1b7a64c]         ; [0x3c9bac8] meta:ʽʳʸʼʳʷʻʷʳʷˁ_TypeInfo
0212147c: cmp    dword ptr [rax + 0xe0], 0
02121483: jne    0x182121494
02121485: mov    rcx, rax
02121488: call   0x182f60cf0
0212148d: mov    rax, qword ptr [rip + 0x1b7a634]         ; [0x3c9bac8] meta:ʽʳʸʼʳʷʻʷʳʷˁ_TypeInfo
02121494: mov    rax, qword ptr [rax + 0xb8]
0212149b: mov    rbx, qword ptr [rax + 0xe0]
021214a2: mov    r8, qword ptr [rip + 0x1b87f47]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
021214a9: lea    rdx, [rbp]
021214ad: lea    rcx, [rbp + 0x10]
021214b1: movdqa xmmword ptr [rbp], xmm6
021214b6: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
021214bb: test   rbx, rbx
021214be: je     0x182121510
021214c0: movaps xmm0, xmmword ptr [rbp + 0x10]
021214c4: lea    rdx, [rbp + 0x10]
021214c8: xor    r8d, r8d
021214cb: movdqa xmmword ptr [rbp + 0x10], xmm0
021214d0: mov    rcx, rbx
021214d3: call   0x1819a8580                              ; System.Text.Encoding$$GetString
021214d8: mov    qword ptr [rdi + 0x20], rax
021214dc: mov    rdx, rax
021214df: jmp    0x1821214f6
021214e1: mov    rax, qword ptr [rip + 0x1bb5bf0]         ; [0x3cd70d8] meta:string_TypeInfo
021214e8: mov    rcx, qword ptr [rax + 0xb8]
021214ef: mov    rdx, qword ptr [rcx]
021214f2: mov    qword ptr [rdi + 0x20], rdx
021214f6: lea    rcx, [rdi + 0x20]
021214fa: call   0x182f5fc00
021214ff: movaps xmm6, xmmword ptr [rbp + 0x20]
02121503: lea    rsp, [rbp + 0x38]
02121507: pop    r15
02121509: pop    r14
0212150b: pop    rdi
0212150c: pop    rsi
0212150d: pop    rbx
0212150e: pop    rbp
0212150f: ret    
02121510: call   0x182f60c50
02121515: int3   
02121516: int3   
