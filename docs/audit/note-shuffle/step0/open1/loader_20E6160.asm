020e6160: push   rbp
020e6162: push   rbx
020e6163: push   rsi
020e6164: push   rdi
020e6165: push   r14
020e6167: sub    rsp, 0x90
020e616e: lea    rbp, [rsp + 0x60]
020e6173: cmp    byte ptr [rip + 0x1e33132], 0            ; [0x3f192ac] (bss)
020e617a: mov    rdi, rcx
020e617d: movaps xmmword ptr [rbp + 0x20], xmm6
020e6181: jne    0x1820e61c6
020e6183: lea    rcx, [rip + 0x1c23936]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e618a: call   0x182f609b0
020e618f: lea    rcx, [rip + 0x1bbb2aa]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e6196: call   0x182f609b0
020e619b: lea    rcx, [rip + 0x1bc2a66]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e61a2: call   0x182f609b0
020e61a7: lea    rcx, [rip + 0x1c28222]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e61ae: call   0x182f609b0
020e61b3: lea    rcx, [rip + 0x1bb7046]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61ba: call   0x182f609b0
020e61bf: mov    byte ptr [rip + 0x1e330e6], 1            ; [0x3f192ac] (bss)
020e61c6: mov    rax, qword ptr [rip + 0x1bb7033]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61cd: cmp    dword ptr [rax + 0xe0], 0
020e61d4: jne    0x1820e61e5
020e61d6: mov    rcx, rax
020e61d9: call   0x182f60cf0
020e61de: mov    rax, qword ptr [rip + 0x1bb701b]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61e5: mov    rax, qword ptr [rax + 0xb8]
020e61ec: mov    rsi, qword ptr [rax]
020e61ef: test   rsi, rsi
020e61f2: je     0x1820e636c
020e61f8: movsxd rbx, dword ptr [rsi + 0x18]
020e61fc: test   ebx, ebx
020e61fe: je     0x1820e6229
020e6200: lea    rcx, [rbx + 0xf]
020e6204: cmp    rcx, rbx
020e6207: ja     0x1820e6213
020e6209: movabs rcx, 0xffffffffffffff0
020e6213: and    rcx, 0xfffffffffffffff0
020e6217: mov    rax, rcx
020e621a: call   0x18302b960
020e621f: sub    rsp, rcx
020e6222: lea    r14, [rsp + 0x60]
020e6227: jmp    0x1820e622c
020e6229: xor    r14d, r14d
020e622c: mov    r8, rbx
020e622f: xor    edx, edx
020e6231: mov    rcx, r14
020e6234: call   0x18305d9d0
020e6239: mov    dword ptr [rbp + 0xc], 0
020e6240: test   ebx, ebx
020e6242: jns    0x1820e624b
020e6244: xor    ecx, ecx
020e6246: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e624b: mov    rcx, qword ptr [rdi + 0x10]
020e624f: lea    rdx, [rbp]
020e6253: mov    qword ptr [rbp], r14
020e6257: xor    r8d, r8d
020e625a: mov    dword ptr [rbp + 8], ebx
020e625d: movaps xmm6, xmmword ptr [rbp]
020e6261: movdqa xmmword ptr [rbp], xmm6
020e6266: call   0x1805f18b0
020e626b: mov    r8, qword ptr [rip + 0x1bbb1ce]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e6272: lea    rcx, [rbp]
020e6276: mov    rdx, rsi
020e6279: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e627e: movaps xmm0, xmmword ptr [rbp]
020e6282: lea    rdx, [rbp]
020e6286: mov    r8, qword ptr [rip + 0x1c23833]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e628d: lea    rcx, [rbp + 0x10]
020e6291: movdqa xmmword ptr [rbp], xmm0
020e6296: movdqa xmmword ptr [rbp + 0x10], xmm6
020e629b: call   0x182c56fc0
020e62a0: test   al, al
020e62a2: je     0x1820e63bb
020e62a8: mov    rcx, qword ptr [rdi + 0x10]
020e62ac: xor    edx, edx
020e62ae: call   0x1820cf940
020e62b3: cmp    eax, -0x5e
020e62b6: jne    0x1820e6372
020e62bc: mov    rcx, qword ptr [rdi + 0x10]
020e62c0: xor    edx, edx
020e62c2: call   0x1820cf940
020e62c7: mov    rcx, qword ptr [rip + 0x1c28102]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e62ce: movss  xmm6, dword ptr [rdi + 0x18]
020e62d3: mov    esi, eax
020e62d5: call   0x182f60c00
020e62da: mov    qword ptr [rsp + 0x50], 0
020e62e3: mov    r8b, 1
020e62e6: mov    byte ptr [rsp + 0x48], 1
020e62eb: movaps xmm3, xmm6
020e62ee: mov    dword ptr [rsp + 0x40], 1
020e62f6: movzx  edx, r8b
020e62fa: mov    dword ptr [rsp + 0x38], 1
020e6302: mov    rcx, rax
020e6305: mov    dword ptr [rsp + 0x30], 0xffffffff
020e630d: mov    rbx, rax
020e6310: mov    byte ptr [rsp + 0x28], 0
020e6315: mov    dword ptr [rsp + 0x20], 0xffffffff
020e631d: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e6322: test   rbx, rbx
020e6325: je     0x1820e636c
020e6327: mov    rcx, qword ptr [rbx + 0x30]
020e632b: test   rcx, rcx
020e632e: je     0x1820e636c
020e6330: xorps  xmm1, xmm1
020e6333: xor    r8d, r8d
020e6336: cvtsi2sd xmm1, rsi
020e633b: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e6340: mov    rcx, qword ptr [rbx + 0x30]
020e6344: test   rcx, rcx
020e6347: je     0x1820e636c
020e6349: movzx  r8d, byte ptr [rdi + 0x1c]
020e634e: xor    r9d, r9d
020e6351: movzx  edx, byte ptr [rdi + 0x1d]
020e6355: call   0x18212b860                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʹʵʺʲˀˁʽʳˁʹ
020e635a: movaps xmm6, xmmword ptr [rbp + 0x20]
020e635e: mov    rax, rbx
020e6361: lea    rsp, [rbp + 0x30]
020e6365: pop    r14
020e6367: pop    rdi
020e6368: pop    rsi
020e6369: pop    rbx
020e636a: pop    rbp
020e636b: ret    
020e636c: call   0x182f60c50
020e6371: int3   
020e6372: lea    rcx, [rip + 0x1bffcf7]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e6379: call   0x182f609d0
020e637e: mov    rcx, rax
020e6381: call   0x182f60c00
020e6386: lea    rcx, [rip + 0x1bf5eb3]                   ; [0x3cdc240] str:'max'
020e638d: mov    rbx, rax
020e6390: call   0x182f609d0
020e6395: mov    rdx, rax
020e6398: xor    r8d, r8d
020e639b: mov    rcx, rbx
020e639e: call   0x181b92e30                              ; System.Exception$$.ctor
020e63a3: lea    rcx, [rip + 0x1bb6a36]                   ; [0x3c9cde0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ˀʾʸʸʾʲʸʷʼʿʺ()
020e63aa: call   0x182f609d0
020e63af: mov    rdx, rax
020e63b2: mov    rcx, rbx
020e63b5: call   0x182f60c10
020e63ba: int3   
020e63bb: lea    rcx, [rip + 0x1bffcae]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e63c2: call   0x182f609d0
020e63c7: mov    rcx, rax
020e63ca: call   0x182f60c00
020e63cf: lea    rcx, [rip + 0x1be681a]                   ; [0x3cccbf0] str:'sustain_yellow'
020e63d6: mov    rbx, rax
020e63d9: call   0x182f609d0
020e63de: mov    rdx, rax
020e63e1: xor    r8d, r8d
020e63e4: mov    rcx, rbx
020e63e7: call   0x181b92e30                              ; System.Exception$$.ctor
020e63ec: lea    rcx, [rip + 0x1bb69ed]                   ; [0x3c9cde0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ˀʾʸʸʾʲʸʷʼʿʺ()
020e63f3: call   0x182f609d0
020e63f8: mov    rdx, rax
020e63fb: mov    rcx, rbx
020e63fe: call   0x182f60c10
020e6403: int3   
020e6404: int3   
