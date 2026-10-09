020e4e90: push   rbp
020e4e92: push   rbx
020e4e93: push   rsi
020e4e94: push   rdi
020e4e95: push   r14
020e4e97: push   r15
020e4e99: sub    rsp, 0x98
020e4ea0: lea    rbp, [rsp + 0x60]
020e4ea5: cmp    byte ptr [rip + 0x1e34401], 0            ; [0x3f192ad] (bss)
020e4eac: mov    rdi, rcx
020e4eaf: movaps xmmword ptr [rbp + 0x20], xmm6
020e4eb3: jne    0x1820e4ef8
020e4eb5: lea    rcx, [rip + 0x1c24c04]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4ebc: call   0x182f609b0
020e4ec1: lea    rcx, [rip + 0x1bbc578]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4ec8: call   0x182f609b0
020e4ecd: lea    rcx, [rip + 0x1bc3d34]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e4ed4: call   0x182f609b0
020e4ed9: lea    rcx, [rip + 0x1c294f0]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e4ee0: call   0x182f609b0
020e4ee5: lea    rcx, [rip + 0x1bb8314]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eec: call   0x182f609b0
020e4ef1: mov    byte ptr [rip + 0x1e343b5], 1            ; [0x3f192ad] (bss)
020e4ef8: mov    rax, qword ptr [rip + 0x1bb8301]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eff: cmp    dword ptr [rax + 0xe0], 0
020e4f06: jne    0x1820e4f17
020e4f08: mov    rcx, rax
020e4f0b: call   0x182f60cf0
020e4f10: mov    rax, qword ptr [rip + 0x1bb82e9]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4f17: mov    rax, qword ptr [rax + 0xb8]
020e4f1e: mov    rsi, qword ptr [rax]
020e4f21: test   rsi, rsi
020e4f24: je     0x1820e5094
020e4f2a: movsxd rbx, dword ptr [rsi + 0x18]
020e4f2e: xor    r15d, r15d
020e4f31: test   ebx, ebx
020e4f33: je     0x1820e4f5e
020e4f35: lea    rcx, [rbx + 0xf]
020e4f39: cmp    rcx, rbx
020e4f3c: ja     0x1820e4f48
020e4f3e: movabs rcx, 0xffffffffffffff0
020e4f48: and    rcx, 0xfffffffffffffff0
020e4f4c: mov    rax, rcx
020e4f4f: call   0x18302b960
020e4f54: sub    rsp, rcx
020e4f57: lea    r14, [rsp + 0x60]
020e4f5c: jmp    0x1820e4f61
020e4f5e: mov    r14, r15
020e4f61: mov    r8, rbx
020e4f64: xor    edx, edx
020e4f66: mov    rcx, r14
020e4f69: call   0x18305d9d0
020e4f6e: mov    dword ptr [rbp + 0xc], r15d
020e4f72: test   ebx, ebx
020e4f74: jns    0x1820e4f7d
020e4f76: xor    ecx, ecx
020e4f78: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4f7d: mov    rcx, qword ptr [rdi + 0x10]
020e4f81: lea    rdx, [rbp]
020e4f85: mov    qword ptr [rbp], r14
020e4f89: xor    r8d, r8d
020e4f8c: mov    dword ptr [rbp + 8], ebx
020e4f8f: movaps xmm6, xmmword ptr [rbp]
020e4f93: movdqa xmmword ptr [rbp], xmm6
020e4f98: call   0x1805f18b0
020e4f9d: mov    r8, qword ptr [rip + 0x1bbc49c]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4fa4: lea    rcx, [rbp]
020e4fa8: mov    rdx, rsi
020e4fab: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e4fb0: movaps xmm0, xmmword ptr [rbp]
020e4fb4: lea    rdx, [rbp]
020e4fb8: mov    r8, qword ptr [rip + 0x1c24b01]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4fbf: lea    rcx, [rbp + 0x10]
020e4fc3: movdqa xmmword ptr [rbp], xmm0
020e4fc8: movdqa xmmword ptr [rbp + 0x10], xmm6
020e4fcd: call   0x182c56fc0
020e4fd2: test   al, al
020e4fd4: je     0x1820e50e3
020e4fda: mov    rcx, qword ptr [rdi + 0x10]
020e4fde: xor    edx, edx
020e4fe0: call   0x1820cf940
020e4fe5: cmp    eax, -0x3d
020e4fe8: jne    0x1820e509a
020e4fee: mov    rcx, qword ptr [rdi + 0x10]
020e4ff2: xor    edx, edx
020e4ff4: call   0x1820cf940
020e4ff9: mov    rcx, qword ptr [rip + 0x1c293d0]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e5000: movss  xmm6, dword ptr [rdi + 0x18]
020e5005: mov    esi, eax
020e5007: call   0x182f60c00
020e500c: mov    qword ptr [rsp + 0x50], r15
020e5011: movaps xmm3, xmm6
020e5014: mov    byte ptr [rsp + 0x48], 1
020e5019: xor    r8d, r8d
020e501c: mov    dword ptr [rsp + 0x40], r15d
020e5021: mov    dl, 1
020e5023: mov    dword ptr [rsp + 0x38], r15d
020e5028: mov    rcx, rax
020e502b: mov    dword ptr [rsp + 0x30], 0xffffffff
020e5033: mov    rbx, rax
020e5036: mov    byte ptr [rsp + 0x28], 1
020e503b: mov    dword ptr [rsp + 0x20], 0xffffffff
020e5043: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e5048: test   rbx, rbx
020e504b: je     0x1820e5094
020e504d: mov    rcx, qword ptr [rbx + 0x30]
020e5051: test   rcx, rcx
020e5054: je     0x1820e5094
020e5056: xorps  xmm1, xmm1
020e5059: xor    r8d, r8d
020e505c: cvtsi2sd xmm1, rsi
020e5061: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e5066: mov    rcx, qword ptr [rbx + 0x30]
020e506a: test   rcx, rcx
020e506d: je     0x1820e5094
020e506f: movzx  r8d, byte ptr [rdi + 0x1c]
020e5074: xor    r9d, r9d
020e5077: movzx  edx, byte ptr [rdi + 0x1d]
020e507b: call   0x18212b860                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʹʵʺʲˀˁʽʳˁʹ
020e5080: movaps xmm6, xmmword ptr [rbp + 0x20]
020e5084: mov    rax, rbx
020e5087: lea    rsp, [rbp + 0x38]
020e508b: pop    r15
020e508d: pop    r14
020e508f: pop    rdi
020e5090: pop    rsi
020e5091: pop    rbx
020e5092: pop    rbp
020e5093: ret    
020e5094: call   0x182f60c50
020e5099: int3   
020e509a: lea    rcx, [rip + 0x1c00fcf]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50a1: call   0x182f609d0
020e50a6: mov    rcx, rax
020e50a9: call   0x182f60c00
020e50ae: lea    rcx, [rip + 0x1bbca6b]                   ; [0x3ca1b20] str:'All Taps'
020e50b5: mov    rbx, rax
020e50b8: call   0x182f609d0
020e50bd: mov    rdx, rax
020e50c0: xor    r8d, r8d
020e50c3: mov    rcx, rbx
020e50c6: call   0x181b92e30                              ; System.Exception$$.ctor
020e50cb: lea    rcx, [rip + 0x1bb7ba6]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e50d2: call   0x182f609d0
020e50d7: mov    rdx, rax
020e50da: mov    rcx, rbx
020e50dd: call   0x182f60c10
020e50e2: int3   
020e50e3: lea    rcx, [rip + 0x1c00f86]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50ea: call   0x182f609d0
020e50ef: mov    rcx, rax
020e50f2: call   0x182f60c00
020e50f7: lea    rcx, [rip + 0x1bfd02a]                   ; [0x3ce2128] str:'+'
020e50fe: mov    rbx, rax
020e5101: call   0x182f609d0
020e5106: mov    rdx, rax
020e5109: xor    r8d, r8d
020e510c: mov    rcx, rbx
020e510f: call   0x181b92e30                              ; System.Exception$$.ctor
020e5114: lea    rcx, [rip + 0x1bb7b5d]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e511b: call   0x182f609d0
020e5120: mov    rdx, rax
020e5123: mov    rcx, rbx
020e5126: call   0x182f60c10
020e512b: int3   
020e512c: int3   
