005dd8d0: push   rbx
005dd8d2: push   rbp
005dd8d3: push   rsi
005dd8d4: push   rdi
005dd8d5: sub    rsp, 0x88
005dd8dc: cmp    qword ptr [r9 + 0x38], 0
005dd8e1: mov    rdi, r9
005dd8e4: mov    ebx, r8d
005dd8e7: mov    rsi, rdx
005dd8ea: mov    rbp, rcx
005dd8ed: jne    0x1805dd90a
005dd8ef: lea    rcx, [rip + 0x36be8da]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005dd8f6: call   0x182f609b0
005dd8fb: cmp    qword ptr [rdi + 0x38], 0
005dd900: jne    0x1805dd90a
005dd902: mov    rcx, rdi
005dd905: call   0x182f657d0
005dd90a: xor    eax, eax
005dd90c: xorps  xmm0, xmm0
005dd90f: mov    qword ptr [rsp + 0x40], rax
005dd914: xorps  xmm1, xmm1
005dd917: mov    qword ptr [rsp + 0x58], rax
005dd91c: mov    r9d, 0x7fffffff
005dd922: mov    rax, qword ptr [rdi + 0x38]
005dd926: mov    r8d, ebx
005dd929: movups xmmword ptr [rsp + 0x30], xmm0
005dd92e: mov    rdx, rbp
005dd931: movups xmmword ptr [rsp + 0x48], xmm1
005dd936: mov    rcx, qword ptr [rax + 8]
005dd93a: mov    qword ptr [rsp + 0x20], rcx
005dd93f: lea    rcx, [rsp + 0x60]
005dd944: call   0x1805fbc10                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʿʶʻʾʺʴʴʼʹʸʶ<object>
005dd949: movups xmm0, xmmword ptr [rsp + 0x60]
005dd94e: mov    r8, qword ptr [rdi + 0x38]
005dd952: lea    rdx, [rsp + 0x48]
005dd957: movsd  xmm1, qword ptr [rsp + 0x70]
005dd95d: lea    rcx, [rsp + 0x60]
005dd962: movups xmmword ptr [rsp + 0x48], xmm0
005dd967: movsd  qword ptr [rsp + 0x58], xmm1
005dd96d: mov    r8, qword ptr [r8 + 0x18]
005dd971: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
005dd976: movups xmm0, xmmword ptr [rsp + 0x60]
005dd97b: movsd  xmm1, qword ptr [rsp + 0x70]
005dd981: movups xmmword ptr [rsp + 0x30], xmm0
005dd986: movsd  qword ptr [rsp + 0x40], xmm1
005dd98c: nop    dword ptr [rax]
005dd990: mov    rdx, qword ptr [rdi + 0x38]
005dd994: lea    rcx, [rsp + 0x30]
005dd999: mov    rdx, qword ptr [rdx + 0x40]
005dd99d: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
005dd9a2: test   al, al
005dd9a4: je     0x1805dd9e8
005dd9a6: mov    rdx, qword ptr [rdi + 0x38]
005dd9aa: lea    rcx, [rsp + 0x30]
005dd9af: mov    rdx, qword ptr [rdx + 0x28]
005dd9b3: call   0x180fd06f0                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʽʼʵʶʳʹʻʵˁʳˀ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
005dd9b8: mov    rbx, qword ptr [rax]
005dd9bb: test   rbx, rbx
005dd9be: je     0x1805dd9f6
005dd9c0: mov    rdx, qword ptr [rip + 0x36be809]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005dd9c7: mov    ecx, 1
005dd9cc: mov    r8, rbx
005dd9cf: call   0x182c44210
005dd9d4: cmp    rax, rsi
005dd9d7: jle    0x1805dd990
005dd9d9: mov    rax, rbx
005dd9dc: add    rsp, 0x88
005dd9e3: pop    rdi
005dd9e4: pop    rsi
005dd9e5: pop    rbp
005dd9e6: pop    rbx
005dd9e7: ret    
005dd9e8: xor    eax, eax
005dd9ea: add    rsp, 0x88
005dd9f1: pop    rdi
005dd9f2: pop    rsi
005dd9f3: pop    rbp
005dd9f4: pop    rbx
005dd9f5: ret    
005dd9f6: call   0x182f60c50
005dd9fb: int3   
005dd9fc: int3   
