0215b8c0: push   rbp
0215b8c2: push   r13
0215b8c4: sub    rsp, 0x48
0215b8c8: cmp    byte ptr [rip + 0x1dbde74], 0            ; [0x3f19743] (bss)
0215b8cf: mov    rbp, rcx
0215b8d2: jne    0x18215b90b
0215b8d4: lea    rcx, [rip + 0x1b76d65]                   ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215b8db: call   0x182f609b0
0215b8e0: lea    rcx, [rip + 0x1b7be39]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215b8e7: call   0x182f609b0
0215b8ec: lea    rcx, [rip + 0x1b7beed]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b8f3: call   0x182f609b0
0215b8f8: lea    rcx, [rip + 0x1b3a9d1]                   ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0215b8ff: call   0x182f609b0
0215b904: mov    byte ptr [rip + 0x1dbde38], 1            ; [0x3f19743] (bss)
0215b90b: mov    rax, qword ptr [rbp + 0x60]
0215b90f: xor    r13d, r13d
0215b912: mov    qword ptr [rsp + 0x68], rbx
0215b917: xor    ecx, ecx
0215b919: mov    qword ptr [rsp + 0x40], rsi
0215b91e: mov    qword ptr [rsp + 0x38], rdi
0215b923: mov    qword ptr [rsp + 0x30], r12
0215b928: mov    qword ptr [rsp + 0x28], r14
0215b92d: mov    qword ptr [rsp + 0x20], r15
0215b932: mov    dword ptr [rsp + 0x60], r13d
0215b937: test   rax, rax
0215b93a: je     0x18215bb0e
0215b940: cmp    ecx, dword ptr [rax + 0x18]
0215b943: jge    0x18215bae8
0215b949: mov    rcx, qword ptr [rbp + 0x60]
0215b94d: test   rcx, rcx
0215b950: je     0x18215bb0e
0215b956: mov    r8, qword ptr [rip + 0x1b7be83]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b95d: mov    edx, r13d
0215b960: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b965: test   rax, rax
0215b968: je     0x18215bacf
0215b96e: mov    edi, dword ptr [rax + 0x20]
0215b971: lea    ebx, [r13 + 1]
0215b975: mov    r12, qword ptr [rax + 0x10]
0215b979: mov    r14d, r13d
0215b97c: mov    dword ptr [rsp + 0x70], edi
0215b980: mov    rax, qword ptr [rbp + 0x60]
0215b984: test   rax, rax
0215b987: je     0x18215bb0e
0215b98d: cmp    ebx, dword ptr [rax + 0x18]
0215b990: jge    0x18215b9ee
0215b992: mov    r8, qword ptr [rip + 0x1b7be47]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b999: mov    edx, ebx
0215b99b: mov    rcx, rax
0215b99e: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b9a3: test   rax, rax
0215b9a6: je     0x18215b9ea
0215b9a8: cmp    qword ptr [rax + 0x10], r12
0215b9ac: jg     0x18215b9ee
0215b9ae: jne    0x18215b9ea
0215b9b0: cmp    dword ptr [rax + 0x20], edi
0215b9b3: jne    0x18215b9ea
0215b9b5: mov    rcx, qword ptr [rbp + 0x60]
0215b9b9: test   rcx, rcx
0215b9bc: je     0x18215bb0e
0215b9c2: mov    r8, qword ptr [rip + 0x1b7be17]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215b9c9: mov    edx, r14d
0215b9cc: mov    rdi, qword ptr [rax + 0x18]
0215b9d0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215b9d5: test   rax, rax
0215b9d8: je     0x18215bb0e
0215b9de: cmp    rdi, qword ptr [rax + 0x18]
0215b9e2: mov    edi, dword ptr [rsp + 0x70]
0215b9e6: cmovg  r14d, ebx
0215b9ea: inc    ebx
0215b9ec: jmp    0x18215b980
0215b9ee: mov    rcx, qword ptr [rbp + 0x60]
0215b9f2: test   rcx, rcx
0215b9f5: je     0x18215bb0e
0215b9fb: mov    r8, qword ptr [rip + 0x1b7bdde]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215ba02: mov    edx, r14d
0215ba05: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215ba0a: mov    r15, rax
0215ba0d: mov    esi, r13d
0215ba10: mov    rcx, qword ptr [rbp + 0x60]
0215ba14: test   rcx, rcx
0215ba17: je     0x18215bb0e
0215ba1d: cmp    esi, dword ptr [rcx + 0x18]
0215ba20: jge    0x18215baca
0215ba26: cmp    esi, r14d
0215ba29: je     0x18215bac3
0215ba2f: mov    r8, qword ptr [rip + 0x1b7bdaa]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215ba36: mov    edx, esi
0215ba38: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215ba3d: mov    rbx, rax
0215ba40: test   rax, rax
0215ba43: je     0x18215bac3
0215ba45: cmp    qword ptr [rax + 0x10], r12
0215ba49: jg     0x18215baca
0215ba4b: jne    0x18215bac3
0215ba4d: cmp    dword ptr [rax + 0x20], edi
0215ba50: jne    0x18215bac3
0215ba52: test   r15, r15
0215ba55: je     0x18215bb0e
0215ba5b: mov    rcx, qword ptr [rip + 0x1b3a86e]         ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0215ba62: mov    edi, dword ptr [r15 + 0x24]
0215ba66: mov    r13d, dword ptr [rax + 0x24]
0215ba6a: cmp    dword ptr [rcx + 0xe0], 0
0215ba71: jne    0x18215ba78
0215ba73: call   0x182f60cf0
0215ba78: mov    eax, edi
0215ba7a: or     edi, r13d
0215ba7d: or     eax, r13d
0215ba80: and    eax, 0xfffffff8
0215ba83: test   dil, 4
0215ba87: jne    0x18215ba9f
0215ba89: test   dil, 2
0215ba8d: jne    0x18215ba9a
0215ba8f: test   dil, 1
0215ba93: je     0x18215baa2
0215ba95: or     eax, 1
0215ba98: jmp    0x18215baa2
0215ba9a: or     eax, 2
0215ba9d: jmp    0x18215baa2
0215ba9f: or     eax, 4
0215baa2: mov    rcx, qword ptr [r15 + 0x38]
0215baa6: mov    dword ptr [r15 + 0x24], eax
0215baaa: test   rcx, rcx
0215baad: je     0x18215bb0e
0215baaf: mov    r8, qword ptr [rip + 0x1b76b8a]          ; [0x3cd2640] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.AddRange()
0215bab6: mov    rdx, qword ptr [rbx + 0x38]
0215baba: call   0x1807b4980                              ; System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>$$AddRange
0215babf: mov    edi, dword ptr [rsp + 0x70]
0215bac3: inc    esi
0215bac5: jmp    0x18215ba10
0215baca: mov    r13d, dword ptr [rsp + 0x60]
0215bacf: mov    rax, qword ptr [rbp + 0x60]
0215bad3: inc    r13d
0215bad6: mov    dword ptr [rsp + 0x60], r13d
0215badb: mov    ecx, r13d
0215bade: test   rax, rax
0215bae1: je     0x18215bb0e
0215bae3: jmp    0x18215b940
0215bae8: mov    r15, qword ptr [rsp + 0x20]
0215baed: mov    r14, qword ptr [rsp + 0x28]
0215baf2: mov    r12, qword ptr [rsp + 0x30]
0215baf7: mov    rdi, qword ptr [rsp + 0x38]
0215bafc: mov    rsi, qword ptr [rsp + 0x40]
0215bb01: mov    rbx, qword ptr [rsp + 0x68]
0215bb06: add    rsp, 0x48
0215bb0a: pop    r13
0215bb0c: pop    rbp
0215bb0d: ret    
0215bb0e: call   0x182f60c50
0215bb13: int3   
0215bb14: int3   
