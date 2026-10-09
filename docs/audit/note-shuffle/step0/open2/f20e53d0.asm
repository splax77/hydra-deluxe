020e53d0: mov    qword ptr [rsp + 0x10], rbx
020e53d5: push   rbp
020e53d6: push   rsi
020e53d7: push   rdi
020e53d8: push   r12
020e53da: push   r13
020e53dc: push   r14
020e53de: push   r15
020e53e0: lea    rbp, [rsp - 0x27]
020e53e5: sub    rsp, 0x90
020e53ec: mov    r15, rdx
020e53ef: mov    r13, rcx
020e53f2: mov    rcx, qword ptr [rcx + 0x10]
020e53f6: xor    edx, edx
020e53f8: call   0x1820cf940
020e53fd: mov    eax, eax
020e53ff: mov    qword ptr [rbp - 0x39], rax
020e5403: test   rax, rax
020e5406: je     0x1820e57aa
020e540c: xor    r14d, r14d
020e540f: mov    byte ptr [rbp + 0x67], 0
020e5413: mov    dword ptr [rbp + 0x7f], r14d
020e5417: nop    word ptr [rax + rax]
020e5420: cmp    byte ptr [rip + 0x1e33ef4], 0            ; [0x3f1931b] (bss)
020e5427: mov    rdi, qword ptr [r13 + 0x10]
020e542b: jne    0x1820e5440
020e542d: lea    rcx, [rip + 0x1c23acc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5434: call   0x182f609b0
020e5439: mov    byte ptr [rip + 0x1e33edb], 1            ; [0x3f1931b] (bss)
020e5440: xor    ecx, ecx
020e5442: call   0x1801bd810
020e5447: mov    rcx, qword ptr [rip + 0x1c23ab2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e544e: mov    rbx, rax
020e5451: cmp    qword ptr [rcx + 0x38], 0
020e5456: jne    0x1820e545d
020e5458: call   0x182f657d0
020e545d: mov    qword ptr [rbp - 0x21], r14
020e5461: test   rbx, rbx
020e5464: je     0x1820e57df
020e546a: cmp    dword ptr [rbx + 0x18], 8
020e546e: jae    0x1820e5477
020e5470: xor    ecx, ecx
020e5472: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5477: lea    rax, [rbx + 0x20]
020e547b: mov    dword ptr [rbp - 0x21], 8
020e5482: mov    qword ptr [rbp - 0x29], rax
020e5486: lea    rdx, [rbp + 0x17]
020e548a: movaps xmm0, xmmword ptr [rbp - 0x29]
020e548e: xor    r8d, r8d
020e5491: mov    rcx, rdi
020e5494: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5499: call   0x1805f18b0
020e549e: cmp    dword ptr [rbx + 0x18], 0
020e54a2: jbe    0x1820e5803
020e54a8: cmp    byte ptr [rip + 0x1e33e6c], 0            ; [0x3f1931b] (bss)
020e54af: mov    r12, qword ptr [rbx + 0x20]
020e54b3: mov    rdi, qword ptr [r13 + 0x10]
020e54b7: jne    0x1820e54cc
020e54b9: lea    rcx, [rip + 0x1c23a40]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e54c0: call   0x182f609b0
020e54c5: mov    byte ptr [rip + 0x1e33e4f], 1            ; [0x3f1931b] (bss)
020e54cc: xor    ecx, ecx
020e54ce: call   0x1801bd810
020e54d3: mov    rcx, qword ptr [rip + 0x1c23a26]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e54da: mov    rbx, rax
020e54dd: cmp    qword ptr [rcx + 0x38], 0
020e54e2: jne    0x1820e54e9
020e54e4: call   0x182f657d0
020e54e9: mov    qword ptr [rbp - 0x11], r14
020e54ed: test   rbx, rbx
020e54f0: je     0x1820e57df
020e54f6: cmp    dword ptr [rbx + 0x18], 8
020e54fa: jae    0x1820e5503
020e54fc: xor    ecx, ecx
020e54fe: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5503: lea    rax, [rbx + 0x20]
020e5507: mov    dword ptr [rbp - 0x11], 8
020e550e: mov    qword ptr [rbp - 0x19], rax
020e5512: lea    rdx, [rbp + 0x17]
020e5516: movaps xmm0, xmmword ptr [rbp - 0x19]
020e551a: xor    r8d, r8d
020e551d: mov    rcx, rdi
020e5520: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5525: call   0x1805f18b0
020e552a: cmp    dword ptr [rbx + 0x18], 0
020e552e: jbe    0x1820e5803
020e5534: cmp    byte ptr [rip + 0x1e33ddf], 0            ; [0x3f1931a] (bss)
020e553b: mov    rsi, qword ptr [rbx + 0x20]
020e553f: mov    rdi, qword ptr [r13 + 0x10]
020e5543: jne    0x1820e5558
020e5545: lea    rcx, [rip + 0x1c239b4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e554c: call   0x182f609b0
020e5551: mov    byte ptr [rip + 0x1e33dc2], 1            ; [0x3f1931a] (bss)
020e5558: xor    ecx, ecx
020e555a: call   0x1801bd810
020e555f: mov    rcx, qword ptr [rip + 0x1c2399a]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5566: mov    rbx, rax
020e5569: cmp    qword ptr [rcx + 0x38], 0
020e556e: jne    0x1820e5575
020e5570: call   0x182f657d0
020e5575: mov    qword ptr [rbp - 1], r14
020e5579: test   rbx, rbx
020e557c: je     0x1820e57df
020e5582: cmp    dword ptr [rbx + 0x18], 4
020e5586: jae    0x1820e558f
020e5588: xor    ecx, ecx
020e558a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e558f: lea    rax, [rbx + 0x20]
020e5593: mov    dword ptr [rbp - 1], 4
020e559a: mov    qword ptr [rbp - 9], rax
020e559e: lea    rdx, [rbp + 0x17]
020e55a2: movaps xmm0, xmmword ptr [rbp - 9]
020e55a6: xor    r8d, r8d
020e55a9: mov    rcx, rdi
020e55ac: movdqa xmmword ptr [rbp + 0x17], xmm0
020e55b1: call   0x1805f18b0
020e55b6: cmp    dword ptr [rbx + 0x18], 0
020e55ba: jbe    0x1820e5803
020e55c0: cmp    byte ptr [rip + 0x1e33d53], 0            ; [0x3f1931a] (bss)
020e55c7: mov    r14d, dword ptr [rbx + 0x20]
020e55cb: mov    rdi, qword ptr [r13 + 0x10]
020e55cf: jne    0x1820e55e4
020e55d1: lea    rcx, [rip + 0x1c23928]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e55d8: call   0x182f609b0
020e55dd: mov    byte ptr [rip + 0x1e33d36], 1            ; [0x3f1931a] (bss)
020e55e4: xor    ecx, ecx
020e55e6: call   0x1801bd810
020e55eb: mov    rcx, qword ptr [rip + 0x1c2390e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e55f2: mov    rbx, rax
020e55f5: cmp    qword ptr [rcx + 0x38], 0
020e55fa: jne    0x1820e5601
020e55fc: call   0x182f657d0
020e5601: mov    qword ptr [rbp + 0xf], 0
020e5609: test   rbx, rbx
020e560c: je     0x1820e57df
020e5612: cmp    dword ptr [rbx + 0x18], 4
020e5616: jae    0x1820e561f
020e5618: xor    ecx, ecx
020e561a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e561f: lea    rax, [rbx + 0x20]
020e5623: mov    dword ptr [rbp + 0xf], 4
020e562a: mov    qword ptr [rbp + 7], rax
020e562e: lea    rdx, [rbp + 0x17]
020e5632: movaps xmm0, xmmword ptr [rbp + 7]
020e5636: xor    r8d, r8d
020e5639: mov    rcx, rdi
020e563c: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5641: call   0x1805f18b0
020e5646: cmp    dword ptr [rbx + 0x18], 0
020e564a: jbe    0x1820e5803
020e5650: test   r15, r15
020e5653: je     0x1820e57fd
020e5659: cmp    byte ptr [rip + 0x1e33cbe], 0            ; [0x3f1931e] (bss)
020e5660: mov    ebx, dword ptr [rbx + 0x20]
020e5663: jne    0x1820e5690
020e5665: lea    rcx, [rip + 0x1bf1e74]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e566c: call   0x182f609b0
020e5671: lea    rcx, [rip + 0x1bf20a8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e5678: call   0x182f609b0
020e567d: lea    rcx, [rip + 0x1c1552c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e5684: call   0x182f609b0
020e5689: mov    byte ptr [rip + 0x1e33c8e], 1            ; [0x3f1931e] (bss)
020e5690: mov    rax, qword ptr [r15 + 0x50]
020e5694: test   rax, rax
020e5697: je     0x1820e57fd
020e569d: movsxd rcx, dword ptr [rax + 0x114]
020e56a4: xor    eax, eax
020e56a6: cmp    rsi, rcx
020e56a9: cmovle rsi, rax
020e56ad: mov    rcx, qword ptr [r15 + 0x10]
020e56b1: test   rcx, rcx
020e56b4: je     0x1820e57fd
020e56ba: mov    rdx, qword ptr [rip + 0x1c154ef]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e56c1: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e56c6: test   rax, rax
020e56c9: je     0x1820e57fd
020e56cf: mov    qword ptr [rsp + 0x28], 0
020e56d8: mov    r9, rsi
020e56db: mov    r8d, r14d
020e56de: mov    dword ptr [rsp + 0x20], ebx
020e56e2: mov    rdx, r12
020e56e5: mov    rcx, rax
020e56e8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e56ed: mov    rcx, qword ptr [r15 + 0x60]
020e56f1: mov    r9, rax
020e56f4: test   rcx, rcx
020e56f7: je     0x1820e57fd
020e56fd: test   rax, rax
020e5700: je     0x1820e57fd
020e5706: mov    ecx, dword ptr [rcx + 0x18]
020e5709: mov    dword ptr [rax + 0x28], ecx
020e570c: mov    rcx, qword ptr [r15 + 0x60]
020e5710: test   rcx, rcx
020e5713: je     0x1820e57fd
020e5719: mov    r10, qword ptr [rip + 0x1bf1dc0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e5720: inc    dword ptr [rcx + 0x1c]
020e5723: mov    rdx, qword ptr [rcx + 0x10]
020e5727: test   rdx, rdx
020e572a: je     0x1820e57fd
020e5730: movsxd r8, dword ptr [rcx + 0x18]
020e5734: cmp    r8d, dword ptr [rdx + 0x18]
020e5738: jb     0x1820e5753
020e573a: mov    rax, qword ptr [r10 + 0x20]
020e573e: mov    rdx, r9
020e5741: mov    r8, qword ptr [rax + 0xc0]
020e5748: mov    r8, qword ptr [r8 + 0x70]
020e574c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e5751: jmp    0x1820e5779
020e5753: lea    eax, [r8 + 1]
020e5757: mov    dword ptr [rcx + 0x18], eax
020e575a: cmp    r8d, dword ptr [rdx + 0x18]
020e575e: jae    0x1820e5803
020e5764: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e5769: add    rdx, 0x20
020e576d: lea    rcx, [rdx + r8*8]
020e5771: mov    rdx, r9
020e5774: call   0x182f5fc00
020e5779: mov    ecx, dword ptr [rbp + 0x67]
020e577c: test   bl, 0x20
020e577f: movzx  ecx, cl
020e5782: mov    eax, 1
020e5787: cmovne ecx, eax
020e578a: mov    dword ptr [rbp + 0x67], ecx
020e578d: mov    edx, dword ptr [rbp + 0x7f]
020e5790: mov    r14d, 0
020e5796: inc    edx
020e5798: movsxd rax, edx
020e579b: mov    dword ptr [rbp + 0x7f], edx
020e579e: cmp    rax, qword ptr [rbp - 0x39]
020e57a2: jl     0x1820e5420
020e57a8: jmp    0x1820e57b1
020e57aa: xor    cl, cl
020e57ac: test   r15, r15
020e57af: je     0x1820e57fd
020e57b1: mov    byte ptr [r15 + 0xa2], cl
020e57b8: xor    r8d, r8d
020e57bb: mov    rcx, r15
020e57be: xor    edx, edx
020e57c0: mov    rbx, qword ptr [rsp + 0xd8]
020e57c8: add    rsp, 0x90
020e57cf: pop    r15
020e57d1: pop    r14
020e57d3: pop    r13
020e57d5: pop    r12
020e57d7: pop    rdi
020e57d8: pop    rsi
020e57d9: pop    rbp
020e57da: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e57df: xor    ecx, ecx
020e57e1: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e57e6: xorps  xmm0, xmm0
020e57e9: lea    rdx, [rbp + 0x17]
020e57ed: xor    r8d, r8d
020e57f0: movdqa xmmword ptr [rbp + 0x17], xmm0
020e57f5: mov    rcx, rdi
020e57f8: call   0x1805f18b0
020e57fd: call   0x182f60c50
020e5802: int3   
020e5803: call   0x182f60c40
020e5808: int3   
020e5809: int3   
