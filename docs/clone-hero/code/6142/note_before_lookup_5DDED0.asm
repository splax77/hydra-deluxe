005dded0: push   rbx
005dded2: push   rbp
005dded3: push   rsi
005dded4: push   rdi
005dded5: push   r14
005dded7: sub    rsp, 0x80
005ddede: mov    rdi, qword ptr [rsp + 0xd0]
005ddee6: mov    ebx, r8d
005ddee9: movzx  ebp, r9b
005ddeed: mov    rsi, rdx
005ddef0: mov    r14, rcx
005ddef3: cmp    qword ptr [rdi + 0x38], 0
005ddef8: jne    0x1805ddf15
005ddefa: lea    rcx, [rip + 0x36be2cf]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddf01: call   0x182f609b0
005ddf06: cmp    qword ptr [rdi + 0x38], 0
005ddf0b: jne    0x1805ddf15
005ddf0d: mov    rcx, rdi
005ddf10: call   0x182f657d0
005ddf15: xor    eax, eax
005ddf17: xorps  xmm0, xmm0
005ddf1a: mov    qword ptr [rsp + 0x40], rax
005ddf1f: xorps  xmm1, xmm1
005ddf22: mov    qword ptr [rsp + 0x58], rax
005ddf27: xor    r9d, r9d
005ddf2a: mov    rax, qword ptr [rdi + 0x38]
005ddf2e: mov    r8d, ebx
005ddf31: movups xmmword ptr [rsp + 0x30], xmm0
005ddf36: mov    rdx, r14
005ddf39: movups xmmword ptr [rsp + 0x48], xmm1
005ddf3e: mov    rcx, qword ptr [rax + 8]
005ddf42: mov    qword ptr [rsp + 0x20], rcx
005ddf47: lea    rcx, [rsp + 0x60]
005ddf4c: call   0x1805fb640                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʸʶʼʵʾʺʹʲˁʴˁ<object>
005ddf51: movups xmm0, xmmword ptr [rsp + 0x60]
005ddf56: mov    r8, qword ptr [rdi + 0x38]
005ddf5a: lea    rdx, [rsp + 0x48]
005ddf5f: movsd  xmm1, qword ptr [rsp + 0x70]
005ddf65: lea    rcx, [rsp + 0x60]
005ddf6a: movups xmmword ptr [rsp + 0x48], xmm0
005ddf6f: movsd  qword ptr [rsp + 0x58], xmm1
005ddf75: mov    r8, qword ptr [r8 + 0x18]
005ddf79: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
005ddf7e: movups xmm0, xmmword ptr [rsp + 0x60]
005ddf83: movsd  xmm1, qword ptr [rsp + 0x70]
005ddf89: movups xmmword ptr [rsp + 0x30], xmm0
005ddf8e: movsd  qword ptr [rsp + 0x40], xmm1
005ddf94: nop    dword ptr [rax]
005ddf98: nop    dword ptr [rax + rax]
005ddfa0: mov    rdx, qword ptr [rdi + 0x38]
005ddfa4: lea    rcx, [rsp + 0x30]
005ddfa9: mov    rdx, qword ptr [rdx + 0x40]
005ddfad: call   0x180fec570                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʻʳʴʻʺʹʲʾʷʽʹ<__Il2CppFullySharedGenericType>$$ʿʵʷʻʵʻʷʵʶʺʼ
005ddfb2: test   al, al
005ddfb4: je     0x1805de01a
005ddfb6: mov    rdx, qword ptr [rdi + 0x38]
005ddfba: lea    rcx, [rsp + 0x30]
005ddfbf: mov    rdx, qword ptr [rdx + 0x28]
005ddfc3: call   0x180fd06f0                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʽʼʵʶʳʹʻʵˁʳˀ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
005ddfc8: mov    rbx, qword ptr [rax]
005ddfcb: test   rbx, rbx
005ddfce: je     0x1805de02a
005ddfd0: mov    rdx, qword ptr [rip + 0x36be1f9]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddfd7: mov    ecx, 1
005ddfdc: mov    r8, rbx
005ddfdf: call   0x182c44210
005ddfe4: cmp    rax, rsi
005ddfe7: jl     0x1805de009
005ddfe9: mov    rdx, qword ptr [rip + 0x36be1e0]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005ddff0: mov    ecx, 1
005ddff5: mov    r8, rbx
005ddff8: call   0x182c44210
005ddffd: xor    edx, edx
005ddfff: cmp    rax, rsi
005de002: cmovle edx, ebp
005de005: test   edx, edx
005de007: je     0x1805ddfa0
005de009: mov    rax, rbx
005de00c: add    rsp, 0x80
005de013: pop    r14
005de015: pop    rdi
005de016: pop    rsi
005de017: pop    rbp
005de018: pop    rbx
005de019: ret    
005de01a: xor    eax, eax
005de01c: add    rsp, 0x80
005de023: pop    r14
005de025: pop    rdi
005de026: pop    rsi
005de027: pop    rbp
005de028: pop    rbx
005de029: ret    
005de02a: call   0x182f60c50
005de02f: int3   
