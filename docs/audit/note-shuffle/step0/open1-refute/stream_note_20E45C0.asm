020e45c0: push   rbp
020e45c2: push   rbx
020e45c3: push   rsi
020e45c4: push   rdi
020e45c5: push   r12
020e45c7: push   r13
020e45c9: push   r14
020e45cb: push   r15
020e45cd: lea    rbp, [rsp - 0x1f]
020e45d2: sub    rsp, 0x88
020e45d9: mov    r15, rdx
020e45dc: mov    byte ptr [rbp + 0x67], 1
020e45e0: mov    r13, rcx
020e45e3: xor    edx, edx
020e45e5: mov    rcx, qword ptr [rcx + 0x10]
020e45e9: call   0x1820cf940
020e45ee: test   eax, eax
020e45f0: je     0x1820e4974
020e45f6: xor    r14d, r14d
020e45f9: nop    dword ptr [rax]
020e4600: cmp    byte ptr [rip + 0x1e34d14], 0            ; [0x3f1931b] (bss)
020e4607: mov    rdi, qword ptr [r13 + 0x10]
020e460b: jne    0x1820e4620
020e460d: lea    rcx, [rip + 0x1c248ec]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4614: call   0x182f609b0
020e4619: mov    byte ptr [rip + 0x1e34cfb], 1            ; [0x3f1931b] (bss)
020e4620: xor    ecx, ecx
020e4622: call   0x1801bd810
020e4627: mov    rcx, qword ptr [rip + 0x1c248d2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e462e: mov    rbx, rax
020e4631: cmp    qword ptr [rcx + 0x38], 0
020e4636: jne    0x1820e463d
020e4638: call   0x182f657d0
020e463d: mov    qword ptr [rbp - 0x31], r14
020e4641: test   rbx, rbx
020e4644: je     0x1820e49a1
020e464a: cmp    dword ptr [rbx + 0x18], 8
020e464e: jae    0x1820e4657
020e4650: xor    ecx, ecx
020e4652: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4657: lea    rax, [rbx + 0x20]
020e465b: mov    dword ptr [rbp - 0x31], 8
020e4662: mov    qword ptr [rbp - 0x39], rax
020e4666: lea    rdx, [rbp + 7]
020e466a: movaps xmm0, xmmword ptr [rbp - 0x39]
020e466e: xor    r8d, r8d
020e4671: mov    rcx, rdi
020e4674: movdqa xmmword ptr [rbp + 7], xmm0
020e4679: call   0x1805f18b0
020e467e: cmp    dword ptr [rbx + 0x18], 0
020e4682: jbe    0x1820e49c5
020e4688: cmp    byte ptr [rip + 0x1e34c8c], 0            ; [0x3f1931b] (bss)
020e468f: mov    r12, qword ptr [rbx + 0x20]
020e4693: mov    rdi, qword ptr [r13 + 0x10]
020e4697: jne    0x1820e46ac
020e4699: lea    rcx, [rip + 0x1c24860]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e46a0: call   0x182f609b0
020e46a5: mov    byte ptr [rip + 0x1e34c6f], 1            ; [0x3f1931b] (bss)
020e46ac: xor    ecx, ecx
020e46ae: call   0x1801bd810
020e46b3: mov    rcx, qword ptr [rip + 0x1c24846]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e46ba: mov    rbx, rax
020e46bd: cmp    qword ptr [rcx + 0x38], 0
020e46c2: jne    0x1820e46c9
020e46c4: call   0x182f657d0
020e46c9: mov    qword ptr [rbp - 0x21], r14
020e46cd: test   rbx, rbx
020e46d0: je     0x1820e49a1
020e46d6: cmp    dword ptr [rbx + 0x18], 8
020e46da: jae    0x1820e46e3
020e46dc: xor    ecx, ecx
020e46de: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e46e3: lea    rax, [rbx + 0x20]
020e46e7: mov    dword ptr [rbp - 0x21], 8
020e46ee: mov    qword ptr [rbp - 0x29], rax
020e46f2: lea    rdx, [rbp + 7]
020e46f6: movaps xmm0, xmmword ptr [rbp - 0x29]
020e46fa: xor    r8d, r8d
020e46fd: mov    rcx, rdi
020e4700: movdqa xmmword ptr [rbp + 7], xmm0
020e4705: call   0x1805f18b0
020e470a: cmp    dword ptr [rbx + 0x18], 0
020e470e: jbe    0x1820e49c5
020e4714: cmp    byte ptr [rip + 0x1e34bff], 0            ; [0x3f1931a] (bss)
020e471b: mov    rsi, qword ptr [rbx + 0x20]
020e471f: mov    rdi, qword ptr [r13 + 0x10]
020e4723: jne    0x1820e4738
020e4725: lea    rcx, [rip + 0x1c247d4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e472c: call   0x182f609b0
020e4731: mov    byte ptr [rip + 0x1e34be2], 1            ; [0x3f1931a] (bss)
020e4738: xor    ecx, ecx
020e473a: call   0x1801bd810
020e473f: mov    rcx, qword ptr [rip + 0x1c247ba]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4746: mov    rbx, rax
020e4749: cmp    qword ptr [rcx + 0x38], 0
020e474e: jne    0x1820e4755
020e4750: call   0x182f657d0
020e4755: mov    qword ptr [rbp - 0x11], r14
020e4759: test   rbx, rbx
020e475c: je     0x1820e49a1
020e4762: cmp    dword ptr [rbx + 0x18], 4
020e4766: jae    0x1820e476f
020e4768: xor    ecx, ecx
020e476a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e476f: lea    rax, [rbx + 0x20]
020e4773: mov    dword ptr [rbp - 0x11], 4
020e477a: mov    qword ptr [rbp - 0x19], rax
020e477e: lea    rdx, [rbp + 7]
020e4782: movaps xmm0, xmmword ptr [rbp - 0x19]
020e4786: xor    r8d, r8d
020e4789: mov    rcx, rdi
020e478c: movdqa xmmword ptr [rbp + 7], xmm0
020e4791: call   0x1805f18b0
020e4796: cmp    dword ptr [rbx + 0x18], 0
020e479a: jbe    0x1820e49c5
020e47a0: cmp    byte ptr [rip + 0x1e34b73], 0            ; [0x3f1931a] (bss)
020e47a7: mov    r14d, dword ptr [rbx + 0x20]
020e47ab: mov    rdi, qword ptr [r13 + 0x10]
020e47af: jne    0x1820e47c4
020e47b1: lea    rcx, [rip + 0x1c24748]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e47b8: call   0x182f609b0
020e47bd: mov    byte ptr [rip + 0x1e34b56], 1            ; [0x3f1931a] (bss)
020e47c4: xor    ecx, ecx
020e47c6: call   0x1801bd810
020e47cb: mov    rcx, qword ptr [rip + 0x1c2472e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e47d2: mov    rbx, rax
020e47d5: cmp    qword ptr [rcx + 0x38], 0
020e47da: jne    0x1820e47e1
020e47dc: call   0x182f657d0
020e47e1: mov    qword ptr [rbp - 1], 0
020e47e9: test   rbx, rbx
020e47ec: je     0x1820e49a1
020e47f2: cmp    dword ptr [rbx + 0x18], 4
020e47f6: jae    0x1820e47ff
020e47f8: xor    ecx, ecx
020e47fa: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e47ff: lea    rax, [rbx + 0x20]
020e4803: mov    dword ptr [rbp - 1], 4
020e480a: mov    qword ptr [rbp - 9], rax
020e480e: lea    rdx, [rbp + 7]
020e4812: movaps xmm0, xmmword ptr [rbp - 9]
020e4816: xor    r8d, r8d
020e4819: mov    rcx, rdi
020e481c: movdqa xmmword ptr [rbp + 7], xmm0
020e4821: call   0x1805f18b0
020e4826: cmp    dword ptr [rbx + 0x18], 0
020e482a: jbe    0x1820e49c5
020e4830: test   r15, r15
020e4833: je     0x1820e49bf
020e4839: cmp    byte ptr [rip + 0x1e34ade], 0            ; [0x3f1931e] (bss)
020e4840: mov    ebx, dword ptr [rbx + 0x20]
020e4843: jne    0x1820e4870
020e4845: lea    rcx, [rip + 0x1bf2c94]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e484c: call   0x182f609b0
020e4851: lea    rcx, [rip + 0x1bf2ec8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e4858: call   0x182f609b0
020e485d: lea    rcx, [rip + 0x1c1634c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e4864: call   0x182f609b0
020e4869: mov    byte ptr [rip + 0x1e34aae], 1            ; [0x3f1931e] (bss)
020e4870: mov    rax, qword ptr [r15 + 0x50]
020e4874: test   rax, rax
020e4877: je     0x1820e49bf
020e487d: movsxd rcx, dword ptr [rax + 0x114]
020e4884: xor    eax, eax
020e4886: cmp    rsi, rcx
020e4889: cmovle rsi, rax
020e488d: mov    rcx, qword ptr [r15 + 0x10]
020e4891: test   rcx, rcx
020e4894: je     0x1820e49bf
020e489a: mov    rdx, qword ptr [rip + 0x1c1630f]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e48a1: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e48a6: test   rax, rax
020e48a9: je     0x1820e49bf
020e48af: mov    qword ptr [rsp + 0x28], 0
020e48b8: mov    r9, rsi
020e48bb: mov    r8d, r14d
020e48be: mov    dword ptr [rsp + 0x20], ebx
020e48c2: mov    rdx, r12
020e48c5: mov    rcx, rax
020e48c8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e48cd: mov    rcx, qword ptr [r15 + 0x60]
020e48d1: mov    r9, rax
020e48d4: test   rcx, rcx
020e48d7: je     0x1820e49bf
020e48dd: test   rax, rax
020e48e0: je     0x1820e49bf
020e48e6: mov    ecx, dword ptr [rcx + 0x18]
020e48e9: mov    dword ptr [rax + 0x28], ecx
020e48ec: mov    rcx, qword ptr [r15 + 0x60]
020e48f0: test   rcx, rcx
020e48f3: je     0x1820e49bf
020e48f9: mov    r10, qword ptr [rip + 0x1bf2be0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e4900: inc    dword ptr [rcx + 0x1c]
020e4903: mov    rdx, qword ptr [rcx + 0x10]
020e4907: test   rdx, rdx
020e490a: je     0x1820e49bf
020e4910: movsxd r8, dword ptr [rcx + 0x18]
020e4914: cmp    r8d, dword ptr [rdx + 0x18]
020e4918: jb     0x1820e4933
020e491a: mov    rax, qword ptr [r10 + 0x20]
020e491e: mov    rdx, r9
020e4921: mov    r8, qword ptr [rax + 0xc0]
020e4928: mov    r8, qword ptr [r8 + 0x70]
020e492c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e4931: jmp    0x1820e4959
020e4933: lea    eax, [r8 + 1]
020e4937: mov    dword ptr [rcx + 0x18], eax
020e493a: cmp    r8d, dword ptr [rdx + 0x18]
020e493e: jae    0x1820e49c5
020e4944: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e4949: add    rdx, 0x20
020e494d: lea    rcx, [rdx + r8*8]
020e4951: mov    rdx, r9
020e4954: call   0x182f5fc00
020e4959: movzx  eax, byte ptr [rbp + 0x67]
020e495d: mov    r14d, 0
020e4963: test   bl, 0x54
020e4966: mov    ecx, r14d
020e4969: cmove  ecx, eax
020e496c: mov    byte ptr [rbp + 0x67], cl
020e496f: jmp    0x1820e4600
020e4974: test   r15, r15
020e4977: je     0x1820e49bf
020e4979: xor    r8d, r8d
020e497c: mov    byte ptr [r15 + 0xa2], 1
020e4984: xor    edx, edx
020e4986: mov    rcx, r15
020e4989: add    rsp, 0x88
020e4990: pop    r15
020e4992: pop    r14
020e4994: pop    r13
020e4996: pop    r12
020e4998: pop    rdi
020e4999: pop    rsi
020e499a: pop    rbx
020e499b: pop    rbp
020e499c: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e49a1: xor    ecx, ecx
020e49a3: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e49a8: xorps  xmm0, xmm0
020e49ab: lea    rdx, [rbp + 7]
020e49af: xor    r8d, r8d
020e49b2: movdqa xmmword ptr [rbp + 7], xmm0
020e49b7: mov    rcx, rdi
020e49ba: call   0x1805f18b0
