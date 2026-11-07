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
02c7cffd: lea    rcx, [rsp + 0x30]
02c7d002: movups xmmword ptr [rsp + 0x60], xmm0
02c7d007: movsd  qword ptr [rsp + 0x70], xmm1
02c7d00d: mov    r8, qword ptr [r8 + 0x18]
02c7d011: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
02c7d016: movups xmm0, xmmword ptr [rsp + 0x30]
02c7d01b: movsd  xmm1, qword ptr [rsp + 0x40]
02c7d021: movups xmmword ptr [rsp + 0x48], xmm0
02c7d026: movsd  qword ptr [rsp + 0x58], xmm1
02c7d02c: nop    dword ptr [rax]
02c7d030: mov    rdx, qword ptr [rbx + 0x38]
02c7d034: lea    rcx, [rsp + 0x48]
02c7d039: mov    rdx, qword ptr [rdx + 0x40]
02c7d03d: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
02c7d042: test   al, al
02c7d044: je     0x182c7d08f
02c7d046: mov    r8, qword ptr [rbx + 0x38]
02c7d04a: lea    rdx, [rsp + 0x48]
02c7d04f: lea    rcx, [rsp + 0x30]
02c7d054: mov    r8, qword ptr [r8 + 0x28]
02c7d058: call   0x181010a20                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
02c7d05d: mov    r8, qword ptr [rsp + 0x38]
02c7d062: test   r8, r8
02c7d065: je     0x182c7d0a2
02c7d067: mov    rdx, qword ptr [rip + 0x101f162]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
02c7d06e: mov    ecx, 1
02c7d073: mov    edi, dword ptr [rsp + 0x30]
02c7d077: call   0x182c44210
02c7d07c: cmp    rax, rsi
02c7d07f: jle    0x182c7d030
02c7d081: mov    eax, edi
02c7d083: add    rsp, 0x88
02c7d08a: pop    rdi
02c7d08b: pop    rsi
02c7d08c: pop    rbp
02c7d08d: pop    rbx
02c7d08e: ret    
02c7d08f: mov    edi, 0xffffffff
02c7d094: mov    eax, edi
02c7d096: add    rsp, 0x88
02c7d09d: pop    rdi
02c7d09e: pop    rsi
02c7d09f: pop    rbp
02c7d0a0: pop    rbx
02c7d0a1: ret    
02c7d0a2: call   0x182f60c50
02c7d0a7: int3   
02c7d0a8: int3   
