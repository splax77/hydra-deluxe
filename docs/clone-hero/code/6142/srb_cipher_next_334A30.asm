00334a30: mov    qword ptr [rsp + 0x18], rbx
00334a35: mov    qword ptr [rsp + 0x20], rbp
00334a3a: push   r14
00334a3c: sub    rsp, 0x30
00334a40: cmp    byte ptr [rip + 0x3bd9362], 0            ; [0x3f0dda9] (bss)
00334a47: mov    rbp, r8
00334a4a: mov    r14, rdx
00334a4d: mov    rbx, rcx
00334a50: jne    0x180334a65
00334a52: lea    rcx, [rip + 0x3961297]                   ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334a59: call   0x182f609b0
00334a5e: mov    byte ptr [rip + 0x3bd9344], 1            ; [0x3f0dda9] (bss)
00334a65: cmp    qword ptr [rbx + 0x10], 0
00334a6a: je     0x180334b50
00334a70: mov    qword ptr [rsp + 0x40], rsi
00334a75: mov    qword ptr [rsp + 0x48], rdi
00334a7a: mov    rax, qword ptr [rbx + 0x10]
00334a7e: mov    ebx, 1
00334a83: cmp    dword ptr [rax + 0x18], 0
00334a87: je     0x180334a98
00334a89: cmp    dword ptr [rax + 0x18], ebx
00334a8c: jbe    0x180334b64
00334a92: lea    rsi, [rax + 0x24]
00334a96: jmp    0x180334a9b
00334a98: mov    rsi, rbx
00334a9b: mov    rcx, qword ptr [rip + 0x396124e]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334aa2: cmp    dword ptr [rcx + 0xe0], 0
00334aa9: jne    0x180334ab7
00334aab: call   0x182f60cf0
00334ab0: mov    rcx, qword ptr [rip + 0x3961239]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334ab7: mov    rax, qword ptr [rcx + 0xb8]
00334abe: mov    rdx, qword ptr [rax + 0x10]
00334ac2: test   rdx, rdx
00334ac5: je     0x180334adc
00334ac7: cmp    dword ptr [rdx + 0x18], 0
00334acb: je     0x180334adc
00334acd: cmp    dword ptr [rdx + 0x18], ebx
00334ad0: jbe    0x180334b64
00334ad6: lea    rdi, [rdx + 0x24]
00334ada: jmp    0x180334ade
00334adc: xor    edi, edi
00334ade: cmp    dword ptr [rcx + 0xe0], 0
00334ae5: jne    0x180334af3
00334ae7: call   0x182f60cf0
00334aec: mov    rcx, qword ptr [rip + 0x39611fd]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334af3: mov    rax, qword ptr [rcx + 0xb8]
00334afa: mov    rcx, qword ptr [rax]
00334afd: test   rcx, rcx
00334b00: je     0x180334b11
00334b02: cmp    dword ptr [rcx + 0x18], 0
00334b06: je     0x180334b11
00334b08: cmp    dword ptr [rcx + 0x18], ebx
00334b0b: jbe    0x180334b64
00334b0d: lea    rbx, [rcx + 0x21]
00334b11: mov    qword ptr [rsp + 0x28], 0
00334b1a: mov    r9, rdi
00334b1d: mov    r8, rsi
00334b20: mov    qword ptr [rsp + 0x20], rbx
00334b25: mov    rdx, rbp
00334b28: mov    rcx, r14
00334b2b: call   0x180336e50                              ; ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ$$ʴʳʽʵˀʶˁʶˁʵˀ
00334b30: mov    rsi, qword ptr [rsp + 0x40]
00334b35: mov    eax, 0xffffffcc
00334b3a: mov    rdi, qword ptr [rsp + 0x48]
00334b3f: mov    rbx, qword ptr [rsp + 0x50]
00334b44: mov    rbp, qword ptr [rsp + 0x58]
00334b49: add    rsp, 0x30
00334b4d: pop    r14
00334b4f: ret    
00334b50: mov    rcx, qword ptr [rip + 0x3961199]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
00334b57: call   0x182c41230
00334b5c: xor    ecx, ecx
00334b5e: call   0x180334bd0                              ; ʷʺʹʷʺʹʺˁʵʴʼ$$ˀʽʶʶʽʳʵʺʶʵʶ
00334b63: int3   
00334b64: call   0x182f60c40
00334b69: int3   
00334b6a: int3   
