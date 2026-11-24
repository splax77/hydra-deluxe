0212cbf0: push   rsi
0212cbf2: push   r14
0212cbf4: sub    rsp, 0x78
0212cbf8: cmp    byte ptr [rip + 0x1dec980], 0            ; [0x3f1957f] (bss)
0212cbff: mov    rsi, rdx
0212cc02: movaps xmmword ptr [rsp + 0x60], xmm6
0212cc07: mov    r14, rcx
0212cc0a: movaps xmmword ptr [rsp + 0x50], xmm7
0212cc0f: movaps xmm7, xmm2
0212cc12: jne    0x18212cc57
0212cc14: lea    rcx, [rip + 0x1ba7a65]                   ; [0x3cd4680] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.Add()
0212cc1b: call   0x182f609b0
0212cc20: lea    rcx, [rip + 0x1ba7c99]                   ; [0x3cd48c0] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Count()
0212cc27: call   0x182f609b0
0212cc2c: lea    rcx, [rip + 0x1ba7d4d]                   ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
0212cc33: call   0x182f609b0
0212cc38: lea    rcx, [rip + 0x1bcdb09]                   ; [0x3cfa748] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʼʳʿʼʼʾʺʿʺʸʸ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0212cc3f: call   0x182f609b0
0212cc44: lea    rcx, [rip + 0x1b74425]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212cc4b: call   0x182f609b0
0212cc50: mov    byte ptr [rip + 0x1dec928], 1            ; [0x3f1957f] (bss)
0212cc57: xorps  xmm6, xmm6
0212cc5a: mov    qword ptr [rsp + 0x90], rbx
0212cc62: comisd xmm6, xmm7
0212cc66: mov    qword ptr [rsp + 0xa8], rdi
0212cc6e: jae    0x18212ce38
0212cc74: movabs rcx, 0x7fffffffffffffff
0212cc7e: movq   rax, xmm7
0212cc83: and    rax, rcx
0212cc86: movabs rcx, 0x7ff0000000000000
0212cc90: cmp    rax, rcx
0212cc93: jae    0x18212ce38
0212cc99: mov    qword ptr [rsp + 0x98], rbp
0212cca1: mov    rdi, qword ptr [r14 + 0xe0]
0212cca8: test   rdi, rdi
0212ccab: je     0x18212cec0
0212ccb1: mov    ebp, dword ptr [rdi + 0x18]
0212ccb4: test   ebp, ebp
0212ccb6: jle    0x18212cccf
0212ccb8: mov    r8, qword ptr [rip + 0x1ba7cc1]          ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
0212ccbf: lea    edx, [rbp - 1]
0212ccc2: mov    rcx, rdi
0212ccc5: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0212ccca: mov    rbx, rax
0212cccd: jmp    0x18212ccd1
0212cccf: xor    ebx, ebx
0212ccd1: test   rsi, rsi
0212ccd4: jle    0x18212cd64
0212ccda: test   rbx, rbx
0212ccdd: je     0x18212cec0
0212cce3: mov    rcx, qword ptr [rip + 0x1b74386]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
0212ccea: mov    qword ptr [rsp + 0x70], r15
0212ccef: mov    r15, qword ptr [rbx + 0x10]
0212ccf3: movaps xmmword ptr [rsp + 0x40], xmm8
0212ccf9: cmp    dword ptr [rcx + 0xe0], 0
0212cd00: movsd  xmm8, qword ptr [rbx + 0x18]
0212cd06: movaps xmmword ptr [rsp + 0x30], xmm9
0212cd0c: movsd  xmm9, qword ptr [rbx + 0x20]
0212cd12: movaps xmmword ptr [rsp + 0x20], xmm10
0212cd18: movsd  xmm10, qword ptr [r14 + 0xb0]
0212cd21: jne    0x18212cd28
0212cd23: call   0x182f60cf0
0212cd28: xorps  xmm6, xmm6
0212cd2b: mulsd  xmm8, xmm10
0212cd30: mov    rax, rsi
0212cd33: movaps xmm10, xmmword ptr [rsp + 0x20]
0212cd39: sub    rax, r15
0212cd3c: mov    r15, qword ptr [rsp + 0x70]
0212cd41: cvtsi2sd xmm6, rax
0212cd46: mulsd  xmm6, qword ptr [rip + 0xf38e82]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
0212cd4e: divsd  xmm6, xmm8
0212cd53: movaps xmm8, xmmword ptr [rsp + 0x40]
0212cd59: addsd  xmm6, xmm9
0212cd5e: movaps xmm9, xmmword ptr [rsp + 0x30]
0212cd64: test   ebp, ebp
0212cd66: je     0x18212cd7b
0212cd68: test   rbx, rbx
0212cd6b: je     0x18212cec0
0212cd71: cmp    qword ptr [rbx + 0x10], rsi
0212cd75: je     0x18212ce00
0212cd7b: mov    rcx, qword ptr [r14 + 0x10]
0212cd7f: test   rcx, rcx
0212cd82: je     0x18212cec0
0212cd88: mov    rdx, qword ptr [rip + 0x1bcd9b9]         ; [0x3cfa748] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʼʳʿʼʼʾʺʿʺʸʸ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0212cd8f: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0212cd94: mov    r8, qword ptr [rip + 0x1ba78e5]          ; [0x3cd4680] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.Add()
0212cd9b: mov    rbx, rax
0212cd9e: inc    dword ptr [rdi + 0x1c]
0212cda1: mov    rdx, qword ptr [rdi + 0x10]
0212cda5: test   rdx, rdx
0212cda8: je     0x18212cec0
0212cdae: movsxd rcx, dword ptr [rdi + 0x18]
0212cdb2: cmp    ecx, dword ptr [rdx + 0x18]
0212cdb5: jb     0x18212cdd3
0212cdb7: mov    rax, qword ptr [r8 + 0x20]
0212cdbb: mov    rdx, rbx
0212cdbe: mov    rcx, rdi
0212cdc1: mov    r8, qword ptr [rax + 0xc0]
0212cdc8: mov    r8, qword ptr [r8 + 0x70]
0212cdcc: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0212cdd1: jmp    0x18212cdf7
0212cdd3: lea    eax, [rcx + 1]
0212cdd6: mov    dword ptr [rdi + 0x18], eax
0212cdd9: cmp    ecx, dword ptr [rdx + 0x18]
0212cddc: jae    0x18212ceba
0212cde2: mov    qword ptr [rdx + rcx*8 + 0x20], rbx
0212cde7: add    rdx, 0x20
0212cdeb: lea    rcx, [rdx + rcx*8]
0212cdef: mov    rdx, rbx
0212cdf2: call   0x182f5fc00
0212cdf7: test   rbx, rbx
0212cdfa: je     0x18212cec0
0212ce00: mov    rbp, qword ptr [rsp + 0x98]
0212ce08: mov    rdi, qword ptr [rsp + 0xa8]
0212ce10: movsd  qword ptr [rbx + 0x18], xmm7
0212ce15: movaps xmm7, xmmword ptr [rsp + 0x50]
0212ce1a: movsd  qword ptr [rbx + 0x20], xmm6
0212ce1f: movaps xmm6, xmmword ptr [rsp + 0x60]
0212ce24: mov    qword ptr [rbx + 0x10], rsi
0212ce28: mov    rbx, qword ptr [rsp + 0x90]
0212ce30: add    rsp, 0x78
0212ce34: pop    r14
0212ce36: pop    rsi
0212ce37: ret    
0212ce38: lea    rcx, [rip + 0x1bb15c1]                   ; [0x3cde400] meta:double_TypeInfo
0212ce3f: movsd  qword ptr [rsp + 0xa0], xmm7
0212ce48: call   0x182f609d0
0212ce4d: mov    rcx, rax
0212ce50: lea    rdx, [rsp + 0xa0]
0212ce58: call   0x182f5fbe0
0212ce5d: lea    rcx, [rip + 0x1b7876c]                   ; [0x3ca55d0] str:'Invalid tempo value: {0}'
0212ce64: mov    rbx, rax
0212ce67: call   0x182f609d0
0212ce6c: mov    rcx, rax
0212ce6f: xor    r8d, r8d
0212ce72: mov    rdx, rbx
0212ce75: call   0x181982590                              ; System.String$$Format
0212ce7a: lea    rcx, [rip + 0x1b8965f]                   ; [0x3cb64e0] meta:System.ArgumentException_TypeInfo
0212ce81: mov    rbx, rax
0212ce84: call   0x182f609d0
0212ce89: mov    rcx, rax
0212ce8c: call   0x182f60c00
0212ce91: xor    r8d, r8d
0212ce94: mov    rdx, rbx
0212ce97: mov    rcx, rax
0212ce9a: mov    rdi, rax
0212ce9d: call   0x181a8b6d0                              ; System.ArgumentException$$.ctor
0212cea2: lea    rcx, [rip + 0x1b72747]                   ; [0x3c9f5f0] metamethod:Method$ˁʿʺʲʲʹˀʴʾʻʻ.ʷʿʳʹʳʲʽʺʵʻʽ()
0212cea9: call   0x182f609d0
0212ceae: mov    rdx, rax
0212ceb1: mov    rcx, rdi
0212ceb4: call   0x182f60c10
0212ceb9: int3   
0212ceba: call   0x182f60c40
0212cebf: int3   
0212cec0: call   0x182f60c50
0212cec5: int3   
0212cec6: int3   
