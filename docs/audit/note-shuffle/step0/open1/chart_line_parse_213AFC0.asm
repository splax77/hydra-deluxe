0213afc0: push   rbx
0213afc2: push   rbp
0213afc3: push   rsi
0213afc4: push   rdi
0213afc5: push   r12
0213afc7: push   r14
0213afc9: push   r15
0213afcb: sub    rsp, 0x40
0213afcf: cmp    byte ptr [rip + 0x1dde690], 0            ; [0x3f19666] (bss)
0213afd6: mov    r12, r9
0213afd9: mov    rbx, r8
0213afdc: mov    rdi, rdx
0213afdf: mov    r15, rcx
0213afe2: jne    0x18213b003
0213afe4: lea    rcx, [rip + 0x1b599ed]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213afeb: call   0x182f609b0
0213aff0: lea    rcx, [rip + 0x1bac4c1]                   ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
0213aff7: call   0x182f609b0
0213affc: mov    byte ptr [rip + 0x1dde663], 1            ; [0x3f19666] (bss)
0213b003: mov    rcx, qword ptr [rip + 0x1b599ce]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0213b00a: mov    r14, qword ptr [rdi]
0213b00d: mov    ebp, dword ptr [rdi + 8]
0213b010: cmp    dword ptr [rcx + 0xe0], 0
0213b017: jne    0x18213b01e
0213b019: call   0x182f60cf0
0213b01e: mov    r8d, dword ptr [rbx]
0213b021: xor    esi, esi
0213b023: cmp    r8d, ebp
0213b026: jae    0x18213b11d
0213b02c: nop    dword ptr [rax]
0213b030: movsxd rax, r8d
0213b033: movsxd r8, dword ptr [rbx]
0213b036: movzx  edx, word ptr [r14 + rax*2]
0213b03b: lea    eax, [rdx - 0x30]
0213b03e: cmp    ax, 9
0213b042: ja     0x18213b061
0213b044: lea    rsi, [rsi + rsi*4]
0213b048: inc    r8d
0213b04b: mov    dword ptr [rbx], r8d
0213b04e: lea    rsi, [rsi - 0x18]
0213b052: lea    rsi, [rdx + rsi*2]
0213b056: cmp    r8d, ebp
0213b059: jae    0x18213b11d
0213b05f: jmp    0x18213b030
0213b061: mov    eax, dword ptr [rdi + 0xc]
0213b064: cmp    r8d, ebp
0213b067: jae    0x18213b11d
0213b06d: cmp    word ptr [r14 + r8*2], 0x20
0213b073: jne    0x18213b09f
0213b075: lea    ecx, [r8 + 1]
0213b079: cmp    ecx, ebp
0213b07b: jae    0x18213b11d
0213b081: cmp    word ptr [r14 + r8*2 + 2], 0x3d
0213b088: jne    0x18213b09f
0213b08a: lea    ecx, [r8 + 2]
0213b08e: cmp    ecx, ebp
0213b090: jae    0x18213b11d
0213b096: cmp    word ptr [r14 + r8*2 + 4], 0x20
0213b09d: je     0x18213b0c5
0213b09f: xor    r9d, r9d
0213b0a2: mov    qword ptr [rsp + 0x30], r14
0213b0a7: lea    rdx, [rsp + 0x30]
0213b0ac: mov    dword ptr [rsp + 0x38], ebp
0213b0b0: mov    rcx, r12
0213b0b3: mov    dword ptr [rsp + 0x3c], eax
0213b0b7: mov    qword ptr [rsp + 0x20], 0
0213b0c0: call   0x182162810                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʸˀʹʺʶʳʴʿʾʻʵ
0213b0c5: movsxd rdx, dword ptr [rbx]
0213b0c8: lea    eax, [rdx + 3]
0213b0cb: mov    dword ptr [rbx], eax
0213b0cd: cmp    eax, ebp
0213b0cf: jae    0x18213b11d
0213b0d1: inc    eax
0213b0d3: cmp    eax, ebp
0213b0d5: jae    0x18213b11d
0213b0d7: cmp    word ptr [r14 + rdx*2 + 8], 0x20
0213b0de: movzx  r10d, word ptr [r14 + rdx*2 + 6]
0213b0e4: je     0x18213b0eb
0213b0e6: lea    eax, [rdx + 4]
0213b0e9: mov    dword ptr [rbx], eax
0213b0eb: add    dword ptr [rbx], 2
0213b0ee: xorps  xmm0, xmm0
0213b0f1: movups xmmword ptr [r15], xmm0
0213b0f5: mov    r9, qword ptr [rip + 0x1bac3bc]          ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
0213b0fc: mov    r8, rsi
0213b0ff: movzx  edx, r10w
0213b103: mov    rcx, r15
0213b106: call   0x180ce1560                              ; System.ValueTuple<short, long>$$.ctor
0213b10b: mov    rax, r15
0213b10e: add    rsp, 0x40
0213b112: pop    r15
0213b114: pop    r14
0213b116: pop    r12
0213b118: pop    rdi
0213b119: pop    rsi
0213b11a: pop    rbp
0213b11b: pop    rbx
0213b11c: ret    
0213b11d: call   0x182f60c40
0213b122: int3   
0213b123: int3   
