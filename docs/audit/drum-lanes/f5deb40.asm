005deb40: push   rbx
005deb42: push   rbp
005deb43: push   rsi
005deb44: push   rdi
005deb45: sub    rsp, 0x88
005deb4c: cmp    qword ptr [r9 + 0x38], 0
005deb51: mov    rbx, r9
005deb54: mov    edi, r8d
005deb57: mov    rsi, rdx
005deb5a: mov    rbp, rcx
005deb5d: jne    0x1805deb7a
005deb5f: lea    rcx, [rip + 0x36bd66a]                   ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005deb66: call   0x182f609b0
005deb6b: cmp    qword ptr [rbx + 0x38], 0
005deb70: jne    0x1805deb7a
005deb72: mov    rcx, rbx
005deb75: call   0x182f657d0
005deb7a: xor    eax, eax
005deb7c: xorps  xmm0, xmm0
005deb7f: mov    qword ptr [rsp + 0x58], rax
005deb84: xorps  xmm1, xmm1
005deb87: mov    qword ptr [rsp + 0x70], rax
005deb8c: mov    r9d, 0x7fffffff
005deb92: mov    rax, qword ptr [rbx + 0x38]
005deb96: mov    r8d, edi
005deb99: movups xmmword ptr [rsp + 0x48], xmm0
005deb9e: mov    rdx, rbp
005deba1: movups xmmword ptr [rsp + 0x60], xmm1
005deba6: mov    rcx, qword ptr [rax + 8]
005debaa: mov    qword ptr [rsp + 0x20], rcx
005debaf: lea    rcx, [rsp + 0x30]
005debb4: call   0x1805fbc10                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʿʶʻʾʺʴʴʼʹʸʶ<object>
005debb9: movups xmm0, xmmword ptr [rsp + 0x30]
005debbe: mov    r8, qword ptr [rbx + 0x38]
005debc2: lea    rdx, [rsp + 0x60]
005debc7: movsd  xmm1, qword ptr [rsp + 0x40]
005debcd: lea    rcx, [rsp + 0x30]
005debd2: movups xmmword ptr [rsp + 0x60], xmm0
005debd7: movsd  qword ptr [rsp + 0x70], xmm1
005debdd: mov    r8, qword ptr [r8 + 0x18]
005debe1: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
005debe6: movups xmm0, xmmword ptr [rsp + 0x30]
005debeb: movsd  xmm1, qword ptr [rsp + 0x40]
005debf1: movups xmmword ptr [rsp + 0x48], xmm0
005debf6: movsd  qword ptr [rsp + 0x58], xmm1
005debfc: nop    dword ptr [rax]
005dec00: mov    rdx, qword ptr [rbx + 0x38]
005dec04: lea    rcx, [rsp + 0x48]
005dec09: mov    rdx, qword ptr [rdx + 0x40]
005dec0d: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
005dec12: test   al, al
005dec14: je     0x1805dec60
005dec16: mov    r8, qword ptr [rbx + 0x38]
005dec1a: lea    rdx, [rsp + 0x48]
005dec1f: lea    rcx, [rsp + 0x30]
005dec24: mov    r8, qword ptr [r8 + 0x28]
005dec28: call   0x181010a20                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
005dec2d: mov    r8, qword ptr [rsp + 0x38]
005dec32: test   r8, r8
005dec35: je     0x1805dec71
005dec37: mov    rdx, qword ptr [rip + 0x36bd592]         ; [0x3c9c1d0] meta:ʽʶˁʷʸʲʳʲʹʴˁ_TypeInfo
005dec3e: mov    ecx, 1
005dec43: mov    edi, dword ptr [rsp + 0x30]
005dec47: call   0x182c44210
005dec4c: cmp    rax, rsi
005dec4f: jle    0x1805dec00
005dec51: lea    eax, [rdi - 1]
005dec54: add    rsp, 0x88
005dec5b: pop    rdi
005dec5c: pop    rsi
005dec5d: pop    rbp
005dec5e: pop    rbx
005dec5f: ret    
005dec60: mov    eax, 0xffffffff
005dec65: add    rsp, 0x88
005dec6c: pop    rdi
005dec6d: pop    rsi
005dec6e: pop    rbp
005dec6f: pop    rbx
005dec70: ret    
005dec71: call   0x182f60c50
005dec76: int3   
005dec77: int3   
