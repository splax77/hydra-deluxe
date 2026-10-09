020e5130: push   rbp
020e5132: push   rbx
020e5133: push   rsi
020e5134: push   rdi
020e5135: push   r14
020e5137: push   r15
020e5139: sub    rsp, 0x98
020e5140: lea    rbp, [rsp + 0x60]
020e5145: cmp    byte ptr [rip + 0x1e34162], 0            ; [0x3f192ae] (bss)
020e514c: mov    rdi, rcx
020e514f: movaps xmmword ptr [rbp + 0x20], xmm6
020e5153: jne    0x1820e5198
020e5155: lea    rcx, [rip + 0x1c24964]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e515c: call   0x182f609b0
020e5161: lea    rcx, [rip + 0x1bbc2d8]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e5168: call   0x182f609b0
020e516d: lea    rcx, [rip + 0x1bc3a94]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e5174: call   0x182f609b0
020e5179: lea    rcx, [rip + 0x1c29250]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e5180: call   0x182f609b0
020e5185: lea    rcx, [rip + 0x1bb8074]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e518c: call   0x182f609b0
020e5191: mov    byte ptr [rip + 0x1e34116], 1            ; [0x3f192ae] (bss)
020e5198: mov    rax, qword ptr [rip + 0x1bb8061]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e519f: cmp    dword ptr [rax + 0xe0], 0
020e51a6: jne    0x1820e51b7
020e51a8: mov    rcx, rax
020e51ab: call   0x182f60cf0
020e51b0: mov    rax, qword ptr [rip + 0x1bb8049]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e51b7: mov    rax, qword ptr [rax + 0xb8]
020e51be: mov    rsi, qword ptr [rax]
020e51c1: test   rsi, rsi
020e51c4: je     0x1820e5336
020e51ca: movsxd rbx, dword ptr [rsi + 0x18]
020e51ce: xor    r15d, r15d
020e51d1: test   ebx, ebx
020e51d3: je     0x1820e51fe
020e51d5: lea    rcx, [rbx + 0xf]
020e51d9: cmp    rcx, rbx
020e51dc: ja     0x1820e51e8
020e51de: movabs rcx, 0xffffffffffffff0
020e51e8: and    rcx, 0xfffffffffffffff0
020e51ec: mov    rax, rcx
020e51ef: call   0x18302b960
020e51f4: sub    rsp, rcx
020e51f7: lea    r14, [rsp + 0x60]
020e51fc: jmp    0x1820e5201
020e51fe: mov    r14, r15
020e5201: mov    r8, rbx
020e5204: xor    edx, edx
020e5206: mov    rcx, r14
020e5209: call   0x18305d9d0
020e520e: mov    dword ptr [rbp + 0xc], r15d
020e5212: test   ebx, ebx
020e5214: jns    0x1820e521d
020e5216: xor    ecx, ecx
020e5218: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e521d: mov    rcx, qword ptr [rdi + 0x10]
020e5221: lea    rdx, [rbp]
020e5225: mov    qword ptr [rbp], r14
020e5229: xor    r8d, r8d
020e522c: mov    dword ptr [rbp + 8], ebx
020e522f: movaps xmm6, xmmword ptr [rbp]
020e5233: movdqa xmmword ptr [rbp], xmm6
020e5238: call   0x1805f18b0
020e523d: mov    r8, qword ptr [rip + 0x1bbc1fc]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e5244: lea    rcx, [rbp]
020e5248: mov    rdx, rsi
020e524b: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e5250: movaps xmm0, xmmword ptr [rbp]
020e5254: lea    rdx, [rbp]
020e5258: mov    r8, qword ptr [rip + 0x1c24861]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e525f: lea    rcx, [rbp + 0x10]
020e5263: movdqa xmmword ptr [rbp], xmm0
020e5268: movdqa xmmword ptr [rbp + 0x10], xmm6
020e526d: call   0x182c56fc0
020e5272: test   al, al
020e5274: je     0x1820e5385
020e527a: mov    rcx, qword ptr [rdi + 0x10]
020e527e: xor    edx, edx
020e5280: call   0x1820cf940
020e5285: cmp    eax, 0x134d7c0
020e528a: jne    0x1820e533c
020e5290: mov    rcx, qword ptr [rdi + 0x10]
020e5294: xor    edx, edx
020e5296: call   0x1820cf940
020e529b: mov    rcx, qword ptr [rip + 0x1c2912e]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e52a2: movss  xmm6, dword ptr [rdi + 0x18]
020e52a7: mov    esi, eax
020e52a9: call   0x182f60c00
020e52ae: mov    qword ptr [rsp + 0x50], r15
020e52b3: movaps xmm3, xmm6
020e52b6: mov    byte ptr [rsp + 0x48], r15b
020e52bb: xor    r8d, r8d
020e52be: mov    dword ptr [rsp + 0x40], r15d
020e52c3: xor    edx, edx
020e52c5: mov    dword ptr [rsp + 0x38], r15d
020e52ca: mov    rcx, rax
020e52cd: mov    dword ptr [rsp + 0x30], 0xffffffff
020e52d5: mov    rbx, rax
020e52d8: mov    byte ptr [rsp + 0x28], r15b
020e52dd: mov    dword ptr [rsp + 0x20], 0xffffffff
020e52e5: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e52ea: test   rbx, rbx
020e52ed: je     0x1820e5336
020e52ef: mov    rcx, qword ptr [rbx + 0x30]
020e52f3: test   rcx, rcx
020e52f6: je     0x1820e5336
020e52f8: xorps  xmm1, xmm1
020e52fb: xor    r8d, r8d
020e52fe: cvtsi2sd xmm1, rsi
020e5303: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e5308: mov    rcx, qword ptr [rbx + 0x30]
020e530c: test   rcx, rcx
020e530f: je     0x1820e5336
020e5311: movzx  r8d, byte ptr [rdi + 0x1c]
020e5316: xor    r9d, r9d
020e5319: movzx  edx, byte ptr [rdi + 0x1d]
020e531d: call   0x1820cfd40
020e5322: movaps xmm6, xmmword ptr [rbp + 0x20]
020e5326: mov    rax, rbx
020e5329: lea    rsp, [rbp + 0x38]
020e532d: pop    r15
020e532f: pop    r14
020e5331: pop    rdi
020e5332: pop    rsi
020e5333: pop    rbx
020e5334: pop    rbp
020e5335: ret    
020e5336: call   0x182f60c50
020e533b: int3   
020e533c: lea    rcx, [rip + 0x1c00d2d]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e5343: call   0x182f609d0
020e5348: mov    rcx, rax
020e534b: call   0x182f60c00
020e5350: lea    rcx, [rip + 0x1bf2d29]                   ; [0x3cd8080] str:'Unsupported file version'
020e5357: mov    rbx, rax
020e535a: call   0x182f609d0
020e535f: mov    rdx, rax
020e5362: xor    r8d, r8d
020e5365: mov    rcx, rbx
020e5368: call   0x181b92e30                              ; System.Exception$$.ctor
020e536d: lea    rcx, [rip + 0x1bb79b4]                   ; [0x3c9cd28] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʿʳʼʴˁʺʶʳʻʸ()
020e5374: call   0x182f609d0
020e5379: mov    rdx, rax
020e537c: mov    rcx, rbx
020e537f: call   0x182f60c10
020e5384: int3   
020e5385: lea    rcx, [rip + 0x1c00ce4]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e538c: call   0x182f609d0
020e5391: mov    rcx, rax
020e5394: call   0x182f60c00
020e5399: lea    rcx, [rip + 0x1bbce50]                   ; [0x3ca21f0] str:'Invalid file format'
020e53a0: mov    rbx, rax
020e53a3: call   0x182f609d0
020e53a8: mov    rdx, rax
020e53ab: xor    r8d, r8d
020e53ae: mov    rcx, rbx
020e53b1: call   0x181b92e30                              ; System.Exception$$.ctor
020e53b6: lea    rcx, [rip + 0x1bb796b]                   ; [0x3c9cd28] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʿʳʼʴˁʺʶʳʻʸ()
020e53bd: call   0x182f609d0
020e53c2: mov    rdx, rax
020e53c5: mov    rcx, rbx
020e53c8: call   0x182f60c10
020e53cd: int3   
020e53ce: int3   
