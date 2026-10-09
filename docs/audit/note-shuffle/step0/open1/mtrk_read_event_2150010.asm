02150010: push   rbp
02150012: push   rbx
02150013: push   rsi
02150014: push   rdi
02150015: push   r12
02150017: push   r14
02150019: push   r15
0215001b: sub    rsp, 0x60
0215001f: lea    rbp, [rsp + 0x30]
02150024: cmp    byte ptr [rip + 0x1dc95bf], 0            ; [0x3f195ea] (bss)
0215002b: mov    r15, r9
0215002e: movaps xmmword ptr [rbp + 0x20], xmm6
02150032: mov    rdi, r8
02150035: mov    rbx, rdx
02150038: jne    0x182150095
0215003a: lea    rcx, [rip + 0x1bb9a7f]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
02150041: call   0x182f609b0
02150046: lea    rcx, [rip + 0x1b513f3]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
0215004d: call   0x182f609b0
02150052: lea    rcx, [rip + 0x1b58baf]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
02150059: call   0x182f609b0
0215005e: lea    rcx, [rip + 0x1baa27b]                   ; [0x3cfa2e0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʺʾʺˀʾʶʲʷʷʺʳ>.ʺʹʽʳʵʼʺʾʾʷʿ()
02150065: call   0x182f609b0
0215006a: lea    rcx, [rip + 0x1baa55f]                   ; [0x3cfa5d0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʻʻʹʲʽʽʸˀʽʺˀ>.ʺʹʽʳʵʼʺʾʾʷʿ()
02150071: call   0x182f609b0
02150076: lea    rcx, [rip + 0x1bab113]                   ; [0x3cfb190] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ˀʷʲʾʵʻʼʷʶʲʶ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215007d: call   0x182f609b0
02150082: lea    rcx, [rip + 0x1b49437]                   ; [0x3c994c0] meta:ʺʾʺˀʾʶʲʷʷʺʳ_TypeInfo
02150089: call   0x182f609b0
0215008e: mov    byte ptr [rip + 0x1dc9555], 1            ; [0x3f195ea] (bss)
02150095: test   rdi, rdi
02150098: je     0x182150233
0215009e: xor    edx, edx
021500a0: mov    rcx, rdi
021500a3: call   0x18213b340                              ; ʺʵˀʹˀˀʲˁʼʸʸ$$ʾʻʳʾˁʻʻʾʹʲʴ
021500a8: mov    ecx, dword ptr [rsp]
021500ab: mov    r12d, eax
021500ae: sub    rsp, 0x10
021500b2: lea    rdx, [rsp + 0x30]
021500b7: mov    ecx, dword ptr [rdx]
021500b9: xor    eax, eax
021500bb: mov    qword ptr [rbp + 8], 3
021500c3: mov    word ptr [rdx], ax
021500c6: mov    byte ptr [rdx + 2], al
021500c9: mov    qword ptr [rbp], rdx
021500cd: xor    r8d, r8d
021500d0: movaps xmm6, xmmword ptr [rbp]
021500d4: lea    rdx, [rbp]
021500d8: mov    rcx, rdi
021500db: movdqa xmmword ptr [rbp], xmm6
021500e0: call   0x18213b240
021500e5: mov    rdx, qword ptr [rip + 0x1b493d4]         ; [0x3c994c0] meta:ʺʾʺˀʾʶʲʷʷʺʳ_TypeInfo
021500ec: cmp    dword ptr [rdx + 0xe0], 0
021500f3: jne    0x182150104
021500f5: mov    rcx, rdx
021500f8: call   0x182f60cf0
021500fd: mov    rdx, qword ptr [rip + 0x1b493bc]         ; [0x3c994c0] meta:ʺʾʺˀʾʶʲʷʷʺʳ_TypeInfo
02150104: mov    rdx, qword ptr [rdx + 0xb8]
0215010b: lea    rcx, [rbp]
0215010f: mov    r8, qword ptr [rip + 0x1b5132a]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
02150116: mov    rdx, qword ptr [rdx]
02150119: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
0215011e: movaps xmm0, xmmword ptr [rbp]
02150122: lea    rdx, [rbp]
02150126: mov    r8, qword ptr [rip + 0x1bb9993]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
0215012d: lea    rcx, [rbp + 0x10]
02150131: movdqa xmmword ptr [rbp], xmm0
02150136: movdqa xmmword ptr [rbp + 0x10], xmm6
0215013b: call   0x182c56fc0
02150140: test   al, al
02150142: je     0x1821501bb
02150144: test   rbx, rbx
02150147: je     0x182150233
0215014d: mov    rcx, qword ptr [rbx + 0xd8]
02150154: test   rcx, rcx
02150157: je     0x182150233
0215015d: mov    rdx, qword ptr [rip + 0x1baa17c]         ; [0x3cfa2e0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʺʾʺˀʾʶʲʷʷʺʳ>.ʺʹʽʳʵʼʺʾʾʷʿ()
02150164: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
02150169: mov    rsi, rax
0215016c: test   rax, rax
0215016f: je     0x182150233
02150175: mov    r10, qword ptr [rax]
02150178: mov    r9d, r12d
0215017b: mov    r14d, dword ptr [rdi + 0x14]
0215017f: mov    r8, r15
02150182: mov    rdx, rdi
02150185: mov    rcx, qword ptr [r10 + 0x190]
0215018c: mov    qword ptr [rsp + 0x20], rcx
02150191: mov    rcx, rax
02150194: call   qword ptr [r10 + 0x188]
0215019b: cmp    byte ptr [rsi + 0x24], 0
0215019f: je     0x1821501b7
021501a1: mov    rax, rsi
021501a4: movaps xmm6, xmmword ptr [rbp + 0x20]
021501a8: lea    rsp, [rbp + 0x30]
021501ac: pop    r15
021501ae: pop    r14
021501b0: pop    r12
021501b2: pop    rdi
021501b3: pop    rsi
021501b4: pop    rbx
021501b5: pop    rbp
021501b6: ret    
021501b7: mov    dword ptr [rdi + 0x14], r14d
021501bb: movzx  eax, byte ptr [rbp + 0x90]
021501c2: cmp    al, 0xf0
021501c4: jne    0x1821501e0
021501c6: test   rbx, rbx
021501c9: je     0x182150233
021501cb: mov    rcx, qword ptr [rbx + 0xe0]
021501d2: test   rcx, rcx
021501d5: je     0x182150233
021501d7: mov    rdx, qword ptr [rip + 0x1baa3f2]         ; [0x3cfa5d0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʻʻʹʲʽʽʸˀʽʺˀ>.ʺʹʽʳʵʼʺʾʾʷʿ()
021501de: jmp    0x1821501fc
021501e0: cmp    al, 0xf7
021501e2: jne    0x182150233
021501e4: test   rbx, rbx
021501e7: je     0x182150233
021501e9: mov    rcx, qword ptr [rbx + 0xe8]
021501f0: test   rcx, rcx
021501f3: je     0x182150233
021501f5: mov    rdx, qword ptr [rip + 0x1baaf94]         ; [0x3cfb190] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ˀʷʲʾʵʻʼʷʶʲʶ>.ʺʹʽʳʵʼʺʾʾʷʿ()
021501fc: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
02150201: mov    rbx, rax
02150204: test   rax, rax
02150207: je     0x182150233
02150209: mov    r10, qword ptr [rax]
0215020c: mov    r9d, r12d
0215020f: mov    r8, r15
02150212: mov    rdx, rdi
02150215: mov    rcx, qword ptr [r10 + 0x190]
0215021c: mov    qword ptr [rsp + 0x20], rcx
02150221: mov    rcx, rax
02150224: call   qword ptr [r10 + 0x188]
0215022b: mov    rax, rbx
0215022e: jmp    0x1821501a4
02150233: call   0x182f60c50
02150238: int3   
02150239: int3   
