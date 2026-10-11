020d5310: mov    qword ptr [rsp + 0x10], rsi
020d5315: push   rdi
020d5316: sub    rsp, 0x70
020d531a: cmp    byte ptr [rip + 0x1e43fe5], 0            ; [0x3f19306] (bss)
020d5321: mov    esi, edx
020d5323: mov    rdi, rcx
020d5326: jne    0x1820d533b
020d5328: lea    rcx, [rip + 0x1bc3519]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d532f: call   0x182f609b0
020d5334: mov    byte ptr [rip + 0x1e43fcb], 1            ; [0x3f19306] (bss)
020d533b: xor    eax, eax
020d533d: mov    qword ptr [rsp + 0x80], rbx
020d5345: mov    qword ptr [rsp + 0x30], rax
020d534a: xorps  xmm0, xmm0
020d534d: mov    qword ptr [rsp + 0x48], rax
020d5352: xorps  xmm1, xmm1
020d5355: movups xmmword ptr [rsp + 0x20], xmm0
020d535a: movups xmmword ptr [rsp + 0x38], xmm1
020d535f: test   rdi, rdi
020d5362: je     0x1820d5423
020d5368: xor    edx, edx
020d536a: mov    rcx, rdi
020d536d: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020d5372: mov    rcx, qword ptr [rip + 0x1bc34cf]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d5379: movzx  ebx, ax
020d537c: cmp    dword ptr [rcx + 0xe0], 0
020d5383: jne    0x1820d538a
020d5385: call   0x182f60cf0
020d538a: and    si, bx
020d538d: lea    rcx, [rsp + 0x50]
020d5392: movzx  ebx, si
020d5395: xor    r8d, r8d
020d5398: neg    bx
020d539b: mov    rdx, rdi
020d539e: and    bx, si
020d53a1: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d53a6: xor    r8d, r8d
020d53a9: lea    rdx, [rsp + 0x38]
020d53ae: lea    rcx, [rsp + 0x50]
020d53b3: movups xmm0, xmmword ptr [rax]
020d53b6: movups xmmword ptr [rsp + 0x38], xmm0
020d53bb: movsd  xmm1, qword ptr [rax + 0x10]
020d53c0: movsd  qword ptr [rsp + 0x48], xmm1
020d53c6: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d53cb: movups xmm0, xmmword ptr [rax]
020d53ce: movups xmmword ptr [rsp + 0x20], xmm0
020d53d3: movsd  xmm1, qword ptr [rax + 0x10]
020d53d8: movsd  qword ptr [rsp + 0x30], xmm1
020d53de: nop    
020d53e0: xor    edx, edx
020d53e2: lea    rcx, [rsp + 0x20]
020d53e7: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d53ec: test   al, al
020d53ee: je     0x1820d541f
020d53f0: mov    rax, qword ptr [rsp + 0x28]
020d53f5: test   rax, rax
020d53f8: je     0x1820d5423
020d53fa: movzx  ecx, word ptr [rax + 0x80]
020d5401: and    cx, bx
020d5404: cmp    cx, bx
020d5407: jne    0x1820d53e0
020d5409: mov    rbx, qword ptr [rsp + 0x80]
020d5411: mov    rsi, qword ptr [rsp + 0x88]
020d5419: add    rsp, 0x70
020d541d: pop    rdi
020d541e: ret    
020d541f: xor    eax, eax
020d5421: jmp    0x1820d5409
020d5423: call   0x182f60c50
020d5428: int3   
020d5429: int3   
