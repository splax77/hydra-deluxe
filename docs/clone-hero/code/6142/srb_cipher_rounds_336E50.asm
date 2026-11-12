00336e50: mov    qword ptr [rsp + 0x20], rbp
00336e55: push   r12
00336e57: push   r14
00336e59: push   r15
00336e5b: sub    rsp, 0x30
00336e5f: cmp    byte ptr [rip + 0x3bd6f50], 0            ; [0x3f0ddb6] (bss)
00336e66: mov    rbp, r9
00336e69: mov    r14, r8
00336e6c: mov    r15, rdx
00336e6f: mov    r12, rcx
00336e72: jne    0x180336e87
00336e74: lea    rcx, [rip + 0x3968d25]                   ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336e7b: call   0x182f609b0
00336e80: mov    byte ptr [rip + 0x3bd6f2f], 1            ; [0x3f0ddb6] (bss)
00336e87: mov    rcx, qword ptr [rip + 0x3968d12]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336e8e: cmp    dword ptr [rcx + 0xe0], 0
00336e95: jne    0x180336e9c
00336e97: call   0x182f60cf0
00336e9c: cmp    byte ptr [rip + 0x3bd6f17], 0            ; [0x3f0ddba] (bss)
00336ea3: jne    0x180336ec4
00336ea5: lea    rcx, [rip + 0x398ac2c]                   ; [0x3cc1ad8] meta:Unity.Burst.BurstCompiler_TypeInfo
00336eac: call   0x182f609b0
00336eb1: lea    rcx, [rip + 0x3968ce8]                   ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336eb8: call   0x182f609b0
00336ebd: mov    byte ptr [rip + 0x3bd6ef6], 1            ; [0x3f0ddba] (bss)
00336ec4: mov    rcx, qword ptr [rip + 0x398ac0d]         ; [0x3cc1ad8] meta:Unity.Burst.BurstCompiler_TypeInfo
00336ecb: cmp    dword ptr [rcx + 0xe0], 0
00336ed2: jne    0x180336ed9
00336ed4: call   0x182f60cf0
00336ed9: xor    ecx, ecx
00336edb: call   0x182764720                              ; Unity.Burst.BurstCompiler$$get_IsEnabled
00336ee0: test   al, al
00336ee2: je     0x180337067
00336ee8: mov    rcx, qword ptr [rip + 0x3968cb1]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336eef: cmp    dword ptr [rcx + 0xe0], 0
00336ef6: jne    0x180336efd
00336ef8: call   0x182f60cf0
00336efd: cmp    byte ptr [rip + 0x3bd6eb5], 0            ; [0x3f0ddb9] (bss)
00336f04: jne    0x180336f19
00336f06: lea    rcx, [rip + 0x3968c93]                   ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336f0d: call   0x182f609b0
00336f12: mov    byte ptr [rip + 0x3bd6ea0], 1            ; [0x3f0ddb9] (bss)
00336f19: mov    rcx, qword ptr [rip + 0x3968c80]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336f20: cmp    dword ptr [rcx + 0xe0], 0
00336f27: jne    0x180336f2e
00336f29: call   0x182f60cf0
00336f2e: cmp    byte ptr [rip + 0x3bd6e83], 0            ; [0x3f0ddb8] (bss)
00336f35: jne    0x180336f6e
00336f37: lea    rcx, [rip + 0x398ab9a]                   ; [0x3cc1ad8] meta:Unity.Burst.BurstCompiler_TypeInfo
00336f3e: call   0x182f609b0
00336f43: lea    rcx, [rip + 0x3968c56]                   ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336f4a: call   0x182f609b0
00336f4f: lea    rcx, [rip + 0x39a171a]                   ; [0x3cd8670] metamethod:Method$ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʴʸʹʴʻʸʸˀʼʸʼ()
00336f56: call   0x182f609b0
00336f5b: lea    rcx, [rip + 0x39c9dfe]                   ; [0x3d00d60] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ˁʹʽʻʲʺʶʲˀʸʴ_var
00336f62: call   0x182f609b0
00336f67: mov    byte ptr [rip + 0x3bd6e4a], 1            ; [0x3f0ddb8] (bss)
00336f6e: mov    rcx, qword ptr [rip + 0x3968c2b]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336f75: cmp    dword ptr [rcx + 0xe0], 0
00336f7c: jne    0x180336f83
00336f7e: call   0x182f60cf0
00336f83: mov    rcx, qword ptr [rip + 0x3968c16]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336f8a: mov    rax, qword ptr [rcx + 0xb8]
00336f91: cmp    qword ptr [rax], 0
00336f95: jne    0x18033701e
00336f9b: cmp    dword ptr [rcx + 0xe0], 0
00336fa2: mov    qword ptr [rsp + 0x50], rbx
00336fa7: mov    qword ptr [rsp + 0x58], rsi
00336fac: mov    qword ptr [rsp + 0x60], rdi
00336fb1: jne    0x180336fb8
00336fb3: call   0x182f60cf0
00336fb8: mov    rax, qword ptr [rip + 0x3968be1]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00336fbf: mov    rbx, qword ptr [rip + 0x39a16aa]         ; [0x3cd8670] metamethod:Method$ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʴʸʹʴʻʸʸˀʼʸʼ()
00336fc6: mov    rdi, qword ptr [rip + 0x39c9d93]         ; [0x3d00d60] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ˁʹʽʻʲʺʶʲˀʸʴ_var
00336fcd: mov    rcx, qword ptr [rax + 0xb8]
00336fd4: mov    rsi, qword ptr [rcx + 8]
00336fd8: mov    rcx, qword ptr [rip + 0x398aaf9]         ; [0x3cc1ad8] meta:Unity.Burst.BurstCompiler_TypeInfo
00336fdf: cmp    dword ptr [rcx + 0xe0], 0
00336fe6: jne    0x180336fed
00336fe8: call   0x182f60cf0
00336fed: xor    r9d, r9d
00336ff0: mov    r8, rdi
00336ff3: mov    rdx, rbx
00336ff6: mov    rcx, rsi
00336ff9: call   0x182763780                              ; Unity.Burst.BurstCompiler$$GetILPPMethodFunctionPointer2
00336ffe: mov    rcx, qword ptr [rip + 0x3968b9b]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00337005: mov    rdi, qword ptr [rsp + 0x60]
0033700a: mov    rsi, qword ptr [rsp + 0x58]
0033700f: mov    rbx, qword ptr [rsp + 0x50]
00337014: mov    rdx, qword ptr [rcx + 0xb8]
0033701b: mov    qword ptr [rdx], rax
0033701e: mov    rcx, qword ptr [rip + 0x3968b7b]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
00337025: cmp    dword ptr [rcx + 0xe0], 0
0033702c: jne    0x180337033
0033702e: call   0x182f60cf0
00337033: mov    rax, qword ptr [rip + 0x3968b66]         ; [0x3c9fba0] meta:ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ.ʼʴʸʴʶʶʼʴˀʸʼ_TypeInfo
0033703a: mov    rcx, qword ptr [rax + 0xb8]
00337041: mov    r10, qword ptr [rcx]
00337044: test   r10, r10
00337047: je     0x180337067
00337049: mov    r9, rbp
0033704c: mov    r8, r14
0033704f: mov    rdx, r15
00337052: mov    rcx, r12
00337055: mov    rbp, qword ptr [rsp + 0x68]
0033705a: add    rsp, 0x30
0033705e: pop    r15
00337060: pop    r14
00337062: pop    r12
00337064: jmp    r10
00337067: mov    r9, rbp
0033706a: mov    qword ptr [rsp + 0x78], 0
00337073: mov    r8, r14
00337076: mov    rdx, r15
00337079: mov    rcx, r12
0033707c: mov    rbp, qword ptr [rsp + 0x68]
00337081: add    rsp, 0x30
00337085: pop    r15
00337087: pop    r14
00337089: pop    r12
0033708b: jmp    0x18032aa80                              ; ʷʺʹʷʺʹʺˁʵʴʼ.ˀʹʺʳʼʶˀˁʻˁʿ$$ʴʸʹʴʻʸʸˀʼʸʼ
00337090: mov    eax, ecx
00337092: shl    ecx, 8
00337095: shr    eax, 0x18
00337098: or     eax, ecx
0033709a: ret    
0033709b: int3   
