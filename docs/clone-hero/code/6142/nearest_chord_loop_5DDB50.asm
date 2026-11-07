005ddb50: push   rbx
005ddb52: push   rbp
005ddb53: push   rsi
005ddb54: push   rdi
005ddb55: push   r14
005ddb57: push   r15
005ddb59: sub    rsp, 0x38
005ddb5d: cmp    qword ptr [r9 + 0x38], 0
005ddb62: mov    rdi, r9
005ddb65: mov    r14d, r8d
005ddb68: mov    rbp, rdx
005ddb6b: mov    rsi, rcx
005ddb6e: jne    0x1805ddb8b
005ddb70: lea    rcx, [rip + 0x36be659]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddb77: call   0x182f609b0
005ddb7c: cmp    qword ptr [rdi + 0x38], 0
005ddb81: jne    0x1805ddb8b
005ddb83: mov    rcx, rdi
005ddb86: call   0x182f657d0
005ddb8b: mov    r9, qword ptr [rdi + 0x38]
005ddb8f: mov    r8d, r14d
005ddb92: mov    rdx, rbp
005ddb95: mov    rcx, rsi
005ddb98: mov    r9, qword ptr [r9 + 8]
005ddb9c: call   0x182c7cf70
005ddba1: cmp    eax, -1
005ddba4: mov    r8d, eax
005ddba7: mov    ebx, eax
005ddba9: cmove  r8d, r14d
005ddbad: mov    rcx, qword ptr [rdi + 0x38]
005ddbb1: mov    r9b, 1
005ddbb4: mov    rdx, qword ptr [rcx + 0x10]
005ddbb8: mov    rcx, rsi
005ddbbb: mov    qword ptr [rsp + 0x20], rdx
005ddbc0: mov    rdx, rbp
005ddbc3: call   0x182c7ce00
005ddbc8: mov    r15d, eax
005ddbcb: cmp    eax, -1
005ddbce: jne    0x1805ddbea
005ddbd0: cmp    ebx, eax
005ddbd2: jne    0x1805ddc67
005ddbd8: mov    eax, 0xffffffff
005ddbdd: add    rsp, 0x38
005ddbe1: pop    r15
005ddbe3: pop    r14
005ddbe5: pop    rdi
005ddbe6: pop    rsi
005ddbe7: pop    rbp
005ddbe8: pop    rbx
005ddbe9: ret    
005ddbea: cmp    ebx, -1
005ddbed: je     0x1805ddc76
005ddbf3: test   rsi, rsi
005ddbf6: je     0x1805ddc86
005ddbfc: mov    r8, qword ptr [rdi + 0x38]
005ddc00: mov    edx, r15d
005ddc03: mov    rcx, rsi
005ddc06: mov    r8, qword ptr [r8 + 0x18]
005ddc0a: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005ddc0f: mov    r8, qword ptr [rdi + 0x38]
005ddc13: mov    edx, ebx
005ddc15: mov    rcx, rsi
005ddc18: mov    r14, rax
005ddc1b: mov    r8, qword ptr [r8 + 0x18]
005ddc1f: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005ddc24: test   rax, rax
005ddc27: je     0x1805ddc86
005ddc29: mov    rdx, qword ptr [rip + 0x36be5a0]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddc30: mov    ecx, 1
005ddc35: mov    r8, rax
005ddc38: call   0x182c44210
005ddc3d: mov    rdi, rax
005ddc40: test   r14, r14
005ddc43: je     0x1805ddc86
005ddc45: mov    rdx, qword ptr [rip + 0x36be584]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddc4c: mov    ecx, 1
005ddc51: mov    r8, r14
005ddc54: call   0x182c44210
005ddc59: mov    rcx, rbp
005ddc5c: sub    rdi, rbp
005ddc5f: sub    rcx, rax
005ddc62: cmp    rdi, rcx
005ddc65: jg     0x1805ddc76
005ddc67: mov    eax, ebx
005ddc69: add    rsp, 0x38
005ddc6d: pop    r15
005ddc6f: pop    r14
005ddc71: pop    rdi
005ddc72: pop    rsi
005ddc73: pop    rbp
005ddc74: pop    rbx
005ddc75: ret    
005ddc76: mov    eax, r15d
005ddc79: add    rsp, 0x38
005ddc7d: pop    r15
005ddc7f: pop    r14
005ddc81: pop    rdi
005ddc82: pop    rsi
005ddc83: pop    rbp
005ddc84: pop    rbx
005ddc85: ret    
005ddc86: call   0x182f60c50
005ddc8b: int3   
005ddc8c: int3   
