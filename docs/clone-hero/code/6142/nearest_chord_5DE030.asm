005de030: mov    qword ptr [rsp + 0x10], rbx
005de035: mov    qword ptr [rsp + 0x18], rbp
005de03a: push   rsi
005de03b: push   rdi
005de03c: push   r12
005de03e: push   r13
005de040: push   r15
005de042: sub    rsp, 0x30
005de046: mov    rdi, qword ptr [rsp + 0x88]
005de04e: mov    r15, r9
005de051: mov    r12, r8
005de054: mov    r13, rdx
005de057: mov    rsi, rcx
005de05a: cmp    qword ptr [rdi + 0x38], 0
005de05f: jne    0x1805de07c
005de061: lea    rcx, [rip + 0x36be168]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de068: call   0x182f609b0
005de06d: cmp    qword ptr [rdi + 0x38], 0
005de072: jne    0x1805de07c
005de074: mov    rcx, rdi
005de077: call   0x182f657d0
005de07c: mov    r9, qword ptr [rdi + 0x38]
005de080: mov    rdx, r13
005de083: mov    r8d, dword ptr [rsp + 0x80]
005de08b: mov    rcx, rsi
005de08e: mov    qword ptr [rsp + 0x60], r14
005de093: mov    r9, qword ptr [r9 + 8]
005de097: call   0x182c7cf70
005de09c: cmp    eax, -1
005de09f: mov    r8d, eax
005de0a2: mov    ebp, eax
005de0a4: cmove  r8d, dword ptr [rsp + 0x80]
005de0ad: mov    rcx, qword ptr [rdi + 0x38]
005de0b1: mov    r9b, 1
005de0b4: mov    rdx, qword ptr [rcx + 0x10]
005de0b8: mov    rcx, rsi
005de0bb: mov    qword ptr [rsp + 0x20], rdx
005de0c0: mov    rdx, r13
005de0c3: call   0x182c7ce00
005de0c8: cmp    eax, -1
005de0cb: jne    0x1805de10c
005de0cd: cmp    ebp, eax
005de0cf: je     0x1805de1e0
005de0d5: test   rsi, rsi
005de0d8: je     0x1805de1e7
005de0de: mov    r8, qword ptr [rdi + 0x38]
005de0e2: mov    edx, ebp
005de0e4: mov    rcx, rsi
005de0e7: mov    r8, qword ptr [r8 + 0x20]
005de0eb: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005de0f0: mov    r14, qword ptr [rsp + 0x60]
005de0f5: mov    rbx, qword ptr [rsp + 0x68]
005de0fa: mov    rbp, qword ptr [rsp + 0x70]
005de0ff: add    rsp, 0x30
005de103: pop    r15
005de105: pop    r13
005de107: pop    r12
005de109: pop    rdi
005de10a: pop    rsi
005de10b: ret    
005de10c: test   rsi, rsi
005de10f: je     0x1805de1e7
005de115: mov    r8, qword ptr [rdi + 0x38]
005de119: mov    edx, eax
005de11b: mov    rcx, rsi
005de11e: mov    r8, qword ptr [r8 + 0x20]
005de122: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005de127: cmp    ebp, -1
005de12a: je     0x1805de0f0
005de12c: mov    r8, qword ptr [rdi + 0x38]
005de130: mov    edx, ebp
005de132: mov    rcx, rsi
005de135: mov    r14, rax
005de138: mov    r8, qword ptr [r8 + 0x20]
005de13c: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005de141: mov    rdi, rax
005de144: test   r14, r14
005de147: je     0x1805de1e7
005de14d: mov    rdx, qword ptr [rip + 0x36be07c]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de154: mov    ecx, 1
005de159: mov    r8, r14
005de15c: call   0x182c44210
005de161: mov    rbx, rax
005de164: test   rdi, rdi
005de167: je     0x1805de1e7
005de169: mov    rdx, qword ptr [rip + 0x36be060]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de170: mov    ecx, 1
005de175: mov    r8, rdi
005de178: call   0x182c44210
005de17d: cmp    r15, rax
005de180: setl   dl
005de183: cmp    r12, rbx
005de186: setg   cl
005de189: test   cl, dl
005de18b: jne    0x1805de1e0
005de18d: cmp    r12, rbx
005de190: jg     0x1805de1d8
005de192: cmp    r15, rax
005de195: jl     0x1805de1d0
005de197: mov    rdx, qword ptr [rip + 0x36be032]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de19e: mov    ecx, 1
005de1a3: mov    r8, rdi
005de1a6: call   0x182c44210
005de1ab: mov    rdx, qword ptr [rip + 0x36be01e]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de1b2: mov    ecx, 1
005de1b7: mov    r8, r14
005de1ba: mov    rbx, rax
005de1bd: call   0x182c44210
005de1c2: mov    rcx, r13
005de1c5: sub    rbx, r13
005de1c8: sub    rcx, rax
005de1cb: cmp    rbx, rcx
005de1ce: jle    0x1805de1d8
005de1d0: mov    rax, r14
005de1d3: jmp    0x1805de0f0
005de1d8: mov    rax, rdi
005de1db: jmp    0x1805de0f0
005de1e0: xor    eax, eax
005de1e2: jmp    0x1805de0f0
005de1e7: call   0x182f60c50
005de1ec: int3   
005de1ed: int3   
