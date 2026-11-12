00333790: push   rbp
00333792: push   rsi
00333793: push   rdi
00333794: push   r12
00333796: push   r13
00333798: push   r14
0033379a: push   r15
0033379c: sub    rsp, 0x40
003337a0: lea    rbp, [rsp + 0x20]
003337a5: cmp    byte ptr [rip + 0x3bda5ff], 0            ; [0x3f0ddab] (bss)
003337ac: mov    qword ptr [rbp + 0x60], rbx
003337b0: jne    0x1803337e9
003337b2: lea    rcx, [rip + 0x39d73df]                   ; [0x3d0ab98] metamethod:Method$System.Runtime.InteropServices.MemoryMarshal.Cast<byte, uint>()
003337b9: call   0x182f609b0
003337be: lea    rcx, [rip + 0x3975443]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
003337c5: call   0x182f609b0
003337ca: lea    rcx, [rip + 0x396251f]                   ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003337d1: call   0x182f609b0
003337d6: lea    rcx, [rip + 0x3974cf3]                   ; [0x3ca84d0] meta:uint[]_TypeInfo
003337dd: call   0x182f609b0
003337e2: mov    byte ptr [rip + 0x3bda5c2], 1            ; [0x3f0ddab] (bss)
003337e9: mov    rax, 0xfffffffffffffff0
003337f0: call   0x18302b960
003337f5: sub    rsp, rax
003337f8: xor    edx, edx
003337fa: mov    r8, 0xffffffffffffffef
00333801: lea    r15, [rsp + 0x20]
00333806: mov    rcx, r15
00333809: call   0x18305d9d0
0033380e: xorps  xmm0, xmm0
00333811: movups xmmword ptr [rbp], xmm0
00333815: xor    ecx, ecx
00333817: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0033381c: mov    eax, dword ptr [rbp + 0xc]
0033381f: lea    rcx, [rbp + 0x10]
00333823: xor    edx, edx
00333825: mov    dword ptr [rbp + 0x1c], eax
00333828: mov    qword ptr [rbp + 0x10], r15
0033382c: mov    dword ptr [rbp + 0x18], 0xffffffcd
00333833: call   0x18215afe0                              ; ʶʸʾʵʶʶʼʲʸˀʽ$$ˁʶʿʹʲʶʺʴʴʷʹ
00333838: mov    rcx, qword ptr [rip + 0x39d7359]         ; [0x3d0ab98] metamethod:Method$System.Runtime.InteropServices.MemoryMarshal.Cast<byte, uint>()
0033383f: cmp    qword ptr [rcx + 0x38], 0
00333844: jne    0x18033384b
00333846: call   0x182f657d0
0033384b: mov    rcx, qword ptr [rip + 0x3974c7e]         ; [0x3ca84d0] meta:uint[]_TypeInfo
00333852: mov    edx, 0x6f
00333857: call   0x182f5fc80
0033385c: mov    rbx, rax
0033385f: test   rax, rax
00333862: je     0x180333a51
00333868: cmp    dword ptr [rax + 0x18], 0
0033386c: jbe    0x180333a57
00333872: mov    eax, dword ptr [r15 + 4]
00333876: mov    dword ptr [rbx + 0x20], eax
00333879: mov    esi, dword ptr [r15 + 4]
0033387d: mov    dword ptr [rbx + 0x20], esi
00333880: mov    edi, dword ptr [r15]
00333883: mov    dword ptr [rbx + 0x20], edi
00333886: cmp    dword ptr [rbx + 0x18], 1
0033388a: mov    dword ptr [rbp + 0x70], eax
0033388d: jbe    0x180333a57
00333893: mov    r15d, dword ptr [r15 + 0x18]
00333897: xor    r14d, r14d
0033389a: mov    dword ptr [rbx + 0x24], r15d
0033389e: nop    
003338a0: mov    rax, qword ptr [rip + 0x3962449]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003338a7: cmp    dword ptr [rax + 0xe0], 0
003338ae: jne    0x1803338bf
003338b0: mov    rcx, rax
003338b3: call   0x182f60cf0
003338b8: mov    rax, qword ptr [rip + 0x3962431]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003338bf: cmp    byte ptr [rip + 0x3bda4ff], 0            ; [0x3f0ddc5] (bss)
003338c6: jne    0x1803338e2
003338c8: lea    rcx, [rip + 0x3962421]                   ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003338cf: call   0x182f609b0
003338d4: mov    rax, qword ptr [rip + 0x3962415]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003338db: mov    byte ptr [rip + 0x3bda4e3], 1            ; [0x3f0ddc5] (bss)
003338e2: cmp    dword ptr [rax + 0xe0], 0
003338e9: jne    0x1803338fa
003338eb: mov    rcx, rax
003338ee: call   0x182f60cf0
003338f3: mov    rax, qword ptr [rip + 0x39623f6]         ; [0x3c95cf0] meta:ʷʺʹʷʺʹʺˁʵʴʼ_TypeInfo
003338fa: mov    rcx, qword ptr [rax + 0xb8]
00333901: mov    r9, qword ptr [rcx]
00333904: test   r9, r9
00333907: je     0x180333a51
0033390d: mov    ecx, r15d
00333910: mov    edx, r15d
00333913: shl    ecx, 0x18
00333916: shr    edx, 8
00333919: or     edx, ecx
0033391b: movzx  ecx, dl
0033391e: mov    qword ptr [rbp + 0x10], rcx
00333922: cmp    ecx, dword ptr [r9 + 0x18]
00333926: jae    0x180333a57
0033392c: mov    rcx, qword ptr [rax + 0xb8]
00333933: mov    r12, qword ptr [rcx]
00333936: mov    ecx, edx
00333938: shr    ecx, 8
0033393b: movzx  r11d, cl
0033393f: cmp    r11d, dword ptr [r12 + 0x18]
00333944: jae    0x180333a57
0033394a: mov    rcx, qword ptr [rax + 0xb8]
00333951: mov    r8, qword ptr [rcx]
00333954: mov    ecx, edx
00333956: shr    ecx, 0x10
00333959: movzx  r13d, cl
0033395d: mov    qword ptr [rbp], r8
00333961: cmp    r13d, dword ptr [r8 + 0x18]
00333965: jae    0x180333a57
0033396b: mov    rcx, qword ptr [rax + 0xb8]
00333972: shr    edx, 0x18
00333975: mov    rcx, qword ptr [rcx]
00333978: mov    qword ptr [rbp + 0x78], rcx
0033397c: cmp    edx, dword ptr [rcx + 0x18]
0033397f: jae    0x180333a57
00333985: mov    rcx, qword ptr [rax + 0xb8]
0033398c: mov    r10, qword ptr [rcx + 8]
00333990: test   r10, r10
00333993: je     0x180333a51
00333999: cmp    r14d, dword ptr [r10 + 0x18]
0033399d: jae    0x180333a57
003339a3: lea    r8d, [r14 + r14*2]
003339a7: cmp    r8d, dword ptr [rbx + 0x18]
003339ab: jae    0x180333a57
003339b1: mov    rcx, qword ptr [rbp + 0x78]
003339b5: mov    rax, qword ptr [rbp + 0x10]
003339b9: movzx  edx, byte ptr [rdx + rcx + 0x20]
003339be: movzx  eax, byte ptr [rax + r9 + 0x20]
003339c4: mov    rcx, qword ptr [rbp]
003339c8: shl    edx, 8
003339cb: movzx  ecx, byte ptr [r13 + rcx + 0x20]
003339d1: or     edx, ecx
003339d3: movzx  ecx, byte ptr [r11 + r12 + 0x20]
003339d9: shl    edx, 8
003339dc: or     edx, ecx
003339de: movzx  ecx, byte ptr [r14 + r10 + 0x20]
003339e4: shl    edx, 8
003339e7: or     edx, eax
003339e9: movsxd rax, r8d
003339ec: xor    edx, ecx
003339ee: xor    edx, dword ptr [rbp + 0x70]
003339f1: mov    dword ptr [rbp + 0x70], edx
003339f4: mov    dword ptr [rbx + rax*4 + 0x20], edx
003339f8: cmp    r8d, dword ptr [rbx + 0x18]
003339fc: jae    0x180333a57
003339fe: movsxd rax, r8d
00333a01: xor    esi, edx
00333a03: mov    dword ptr [rbx + rax*4 + 0x20], esi
00333a07: lea    eax, [r8 + 8]
00333a0b: cmp    eax, dword ptr [rbx + 0x18]
00333a0e: jae    0x180333a57
00333a10: movsxd rax, r8d
00333a13: xor    edi, esi
00333a15: mov    dword ptr [rbx + rax*4 + 0x40], edi
00333a19: lea    eax, [r8 + 2]
00333a1d: cmp    eax, dword ptr [rbx + 0x18]
00333a20: jae    0x180333a57
00333a22: xor    r15d, edi
00333a25: movsxd rax, r8d
00333a28: inc    r14d
00333a2b: mov    dword ptr [rbx + rax*4 + 0x28], r15d
00333a30: cmp    r14d, 0x50
00333a34: jle    0x1803338a0
00333a3a: mov    rax, rbx
00333a3d: mov    rbx, qword ptr [rbp + 0x60]
00333a41: lea    rsp, [rbp + 0x20]
00333a45: pop    r15
00333a47: pop    r14
00333a49: pop    r13
00333a4b: pop    r12
00333a4d: pop    rdi
00333a4e: pop    rsi
00333a4f: pop    rbp
00333a50: ret    
00333a51: call   0x182f60c50
00333a56: int3   
00333a57: call   0x182f60c40
00333a5c: int3   
00333a5d: int3   
