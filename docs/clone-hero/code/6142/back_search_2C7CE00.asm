02c7ce00: push   rbx
02c7ce02: push   rbp
02c7ce03: push   rsi
02c7ce04: push   rdi
02c7ce05: push   r14
02c7ce07: sub    rsp, 0x80
02c7ce0e: mov    rdi, qword ptr [rsp + 0xd0]
02c7ce16: mov    ebx, r8d
02c7ce19: movzx  r14d, r9b
02c7ce1d: mov    rsi, rdx
02c7ce20: mov    rbp, rcx
02c7ce23: cmp    qword ptr [rdi + 0x38], 0
02c7ce28: jne    0x182c7ce45
02c7ce2a: lea    rcx, [rip + 0x101f39f]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
02c7ce31: call   0x182f609b0
02c7ce36: cmp    qword ptr [rdi + 0x38], 0
02c7ce3b: jne    0x182c7ce45
02c7ce3d: mov    rcx, rdi
02c7ce40: call   0x182f657d0
02c7ce45: xor    eax, eax
02c7ce47: xorps  xmm0, xmm0
02c7ce4a: mov    qword ptr [rsp + 0x58], rax
02c7ce4f: xorps  xmm1, xmm1
02c7ce52: mov    qword ptr [rsp + 0x70], rax
02c7ce57: xor    r9d, r9d
02c7ce5a: mov    rax, qword ptr [rdi + 0x38]
02c7ce5e: mov    r8d, ebx
02c7ce61: movups xmmword ptr [rsp + 0x48], xmm0
02c7ce66: mov    rdx, rbp
02c7ce69: movups xmmword ptr [rsp + 0x60], xmm1
02c7ce6e: mov    rcx, qword ptr [rax + 8]
02c7ce72: mov    qword ptr [rsp + 0x20], rcx
02c7ce77: lea    rcx, [rsp + 0x30]
02c7ce7c: call   0x1805fb640                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʸʶʼʵʾʺʹʲˁʴˁ<object>
02c7ce81: movups xmm0, xmmword ptr [rsp + 0x30]
02c7ce86: mov    r8, qword ptr [rdi + 0x38]
02c7ce8a: lea    rdx, [rsp + 0x60]
02c7ce8f: movsd  xmm1, qword ptr [rsp + 0x40]
02c7ce95: lea    rcx, [rsp + 0x30]
02c7ce9a: movups xmmword ptr [rsp + 0x60], xmm0
02c7ce9f: movsd  qword ptr [rsp + 0x70], xmm1
02c7cea5: mov    r8, qword ptr [r8 + 0x18]
02c7cea9: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
02c7ceae: movups xmm0, xmmword ptr [rsp + 0x30]
02c7ceb3: movsd  xmm1, qword ptr [rsp + 0x40]
02c7ceb9: movups xmmword ptr [rsp + 0x48], xmm0
02c7cebe: movsd  qword ptr [rsp + 0x58], xmm1
02c7cec4: nop    dword ptr [rax]
02c7cec8: nop    dword ptr [rax + rax]
02c7ced0: mov    rdx, qword ptr [rdi + 0x38]
02c7ced4: lea    rcx, [rsp + 0x48]
02c7ced9: mov    rdx, qword ptr [rdx + 0x40]
02c7cedd: call   0x180fec570                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʻʳʴʻʺʹʲʾʷʽʹ<__Il2CppFullySharedGenericType>$$ʿʵʷʻʵʻʷʵʶʺʼ
02c7cee2: test   al, al
02c7cee4: je     0x182c7cf55
02c7cee6: mov    r8, qword ptr [rdi + 0x38]
02c7ceea: lea    rdx, [rsp + 0x48]
02c7ceef: lea    rcx, [rsp + 0x30]
02c7cef4: mov    r8, qword ptr [r8 + 0x28]
02c7cef8: call   0x180fec4d0                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʻʳʴʻʺʹʲʾʷʽʹ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
02c7cefd: mov    rbx, qword ptr [rsp + 0x38]
02c7cf02: test   rbx, rbx
02c7cf05: je     0x182c7cf6a
02c7cf07: mov    rdx, qword ptr [rip + 0x101f2c2]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
02c7cf0e: mov    ecx, 1
02c7cf13: mov    ebp, dword ptr [rsp + 0x30]
02c7cf17: mov    r8, rbx
02c7cf1a: call   0x182c44210
02c7cf1f: cmp    rax, rsi
02c7cf22: jl     0x182c7cf5a
02c7cf24: mov    rdx, qword ptr [rip + 0x101f2a5]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
02c7cf2b: mov    ecx, 1
02c7cf30: mov    r8, rbx
02c7cf33: call   0x182c44210
02c7cf38: xor    edx, edx
02c7cf3a: cmp    rax, rsi
02c7cf3d: cmovle edx, r14d
02c7cf41: test   edx, edx
02c7cf43: je     0x182c7ced0
02c7cf45: mov    eax, ebp
02c7cf47: add    rsp, 0x80
02c7cf4e: pop    r14
02c7cf50: pop    rdi
02c7cf51: pop    rsi
02c7cf52: pop    rbp
02c7cf53: pop    rbx
02c7cf54: ret    
02c7cf55: mov    ebp, 0xffffffff
02c7cf5a: mov    eax, ebp
02c7cf5c: add    rsp, 0x80
02c7cf63: pop    r14
02c7cf65: pop    rdi
02c7cf66: pop    rsi
02c7cf67: pop    rbp
02c7cf68: pop    rbx
02c7cf69: ret    
02c7cf6a: call   0x182f60c50
02c7cf6f: int3   
02c7cf70: push   rbx
02c7cf72: push   rbp
02c7cf73: push   rsi
02c7cf74: push   rdi
02c7cf75: sub    rsp, 0x88
02c7cf7c: cmp    qword ptr [r9 + 0x38], 0
02c7cf81: mov    rbx, r9
02c7cf84: mov    edi, r8d
02c7cf87: mov    rsi, rdx
02c7cf8a: mov    rbp, rcx
02c7cf8d: jne    0x182c7cfaa
02c7cf8f: lea    rcx, [rip + 0x101f23a]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
02c7cf96: call   0x182f609b0
02c7cf9b: cmp    qword ptr [rbx + 0x38], 0
02c7cfa0: jne    0x182c7cfaa
02c7cfa2: mov    rcx, rbx
02c7cfa5: call   0x182f657d0
02c7cfaa: xor    eax, eax
02c7cfac: xorps  xmm0, xmm0
02c7cfaf: mov    qword ptr [rsp + 0x58], rax
02c7cfb4: xorps  xmm1, xmm1
02c7cfb7: mov    qword ptr [rsp + 0x70], rax
02c7cfbc: mov    r9d, 0x7fffffff
02c7cfc2: mov    rax, qword ptr [rbx + 0x38]
02c7cfc6: mov    r8d, edi
02c7cfc9: movups xmmword ptr [rsp + 0x48], xmm0
02c7cfce: mov    rdx, rbp
02c7cfd1: movups xmmword ptr [rsp + 0x60], xmm1
02c7cfd6: mov    rcx, qword ptr [rax + 8]
02c7cfda: mov    qword ptr [rsp + 0x20], rcx
02c7cfdf: lea    rcx, [rsp + 0x30]
02c7cfe4: call   0x1805fbc10                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʿʶʻʾʺʴʴʼʹʸʶ<object>
02c7cfe9: movups xmm0, xmmword ptr [rsp + 0x30]
02c7cfee: mov    r8, qword ptr [rbx + 0x38]
02c7cff2: lea    rdx, [rsp + 0x60]
02c7cff7: movsd  xmm1, qword ptr [rsp + 0x40]
