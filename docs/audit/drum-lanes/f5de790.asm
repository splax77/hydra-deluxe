005de790: mov    qword ptr [rsp + 0x10], rbx
005de795: mov    qword ptr [rsp + 0x18], rbp
005de79a: push   rdi
005de79b: push   r14
005de79d: push   r15
005de79f: sub    rsp, 0x80
005de7a6: mov    rdi, qword ptr [rsp + 0xc0]
005de7ae: mov    r14d, r9d
005de7b1: movzx  r15d, r8w
005de7b5: mov    rbp, rdx
005de7b8: mov    rbx, rcx
005de7bb: cmp    qword ptr [rdi + 0x38], 0
005de7c0: jne    0x1805de7dd
005de7c2: lea    rcx, [rip + 0x36bda07]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de7c9: call   0x182f609b0
005de7ce: cmp    qword ptr [rdi + 0x38], 0
005de7d3: jne    0x1805de7dd
005de7d5: mov    rcx, rdi
005de7d8: call   0x182f657d0
005de7dd: xor    eax, eax
005de7df: xorps  xmm0, xmm0
005de7e2: mov    qword ptr [rsp + 0x58], rax
005de7e7: xorps  xmm1, xmm1
005de7ea: mov    qword ptr [rsp + 0x70], rax
005de7ef: mov    r9d, 0x7fffffff
005de7f5: mov    rax, qword ptr [rdi + 0x38]
005de7f9: mov    r8d, r14d
005de7fc: movups xmmword ptr [rsp + 0x48], xmm0
005de801: mov    rdx, rbx
005de804: movups xmmword ptr [rsp + 0x60], xmm1
005de809: mov    rcx, qword ptr [rax + 8]
005de80d: mov    qword ptr [rsp + 0x20], rcx
005de812: lea    rcx, [rsp + 0x30]
005de817: call   0x1805fbc10                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʿʶʻʾʺʴʴʼʹʸʶ<object>
005de81c: movups xmm0, xmmword ptr [rsp + 0x30]
005de821: mov    r8, qword ptr [rdi + 0x38]
005de825: lea    rdx, [rsp + 0x60]
005de82a: movsd  xmm1, qword ptr [rsp + 0x40]
005de830: lea    rcx, [rsp + 0x30]
005de835: movups xmmword ptr [rsp + 0x60], xmm0
005de83a: movsd  qword ptr [rsp + 0x70], xmm1
005de840: mov    r8, qword ptr [r8 + 0x18]
005de844: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
005de849: movups xmm0, xmmword ptr [rsp + 0x30]
005de84e: mov    qword ptr [rsp + 0xa0], rsi
005de856: movsd  xmm1, qword ptr [rsp + 0x40]
005de85c: movups xmmword ptr [rsp + 0x48], xmm0
005de861: movsd  qword ptr [rsp + 0x58], xmm1
005de867: nop    word ptr [rax + rax]
005de870: mov    rdx, qword ptr [rdi + 0x38]
005de874: lea    rcx, [rsp + 0x48]
005de879: mov    rdx, qword ptr [rdx + 0x40]
005de87d: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
005de882: test   al, al
005de884: je     0x1805de90c
005de88a: mov    r8, qword ptr [rdi + 0x38]
005de88e: lea    rdx, [rsp + 0x48]
005de893: lea    rcx, [rsp + 0x30]
005de898: mov    r8, qword ptr [r8 + 0x28]
005de89c: call   0x181010a20                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
005de8a1: mov    rbx, qword ptr [rsp + 0x38]
005de8a6: test   rbx, rbx
005de8a9: je     0x1805de932
005de8af: mov    rdx, qword ptr [rip + 0x36bd91a]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de8b6: mov    ecx, 1
005de8bb: mov    esi, dword ptr [rsp + 0x30]
005de8bf: mov    r8, rbx
005de8c2: call   0x182c44210
005de8c7: cmp    rax, rbp
005de8ca: jg     0x1805de8ea
005de8cc: mov    rdx, qword ptr [rip + 0x36bd8fd]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de8d3: mov    ecx, 1
005de8d8: mov    r8, rbx
005de8db: call   0x182c44210
005de8e0: cmp    rax, rbp
005de8e3: jne    0x1805de870
005de8e5: cmp    esi, r14d
005de8e8: je     0x1805de870
005de8ea: mov    rdx, qword ptr [rip + 0x36bd8df]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005de8f1: mov    ecx, 3
005de8f6: mov    r8, rbx
005de8f9: call   0x182c44210
005de8fe: test   r15w, ax
005de902: je     0x1805de870
005de908: mov    eax, esi
005de90a: jmp    0x1805de911
005de90c: mov    eax, 0xffffffff
005de911: mov    rsi, qword ptr [rsp + 0xa0]
005de919: lea    r11, [rsp + 0x80]
005de921: mov    rbx, qword ptr [r11 + 0x28]
005de925: mov    rbp, qword ptr [r11 + 0x30]
005de929: mov    rsp, r11
005de92c: pop    r15
005de92e: pop    r14
005de930: pop    rdi
005de931: ret    
005de932: call   0x182f60c50
005de937: int3   
005de938: int3   
