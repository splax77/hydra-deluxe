02154180: mov    qword ptr [rsp + 8], rbx
02154185: mov    qword ptr [rsp + 0x20], rsi
0215418a: mov    byte ptr [rsp + 0x18], r8b
0215418f: push   rdi
02154190: push   r12
02154192: push   r13
02154194: push   r14
02154196: push   r15
02154198: sub    rsp, 0x90
0215419f: mov    rbx, rdx
021541a2: mov    rdi, rcx
021541a5: cmp    byte ptr [rip + 0x1dc54f2], 0            ; [0x3f1969e] (bss)
021541ac: jne    0x182154209
021541ae: lea    rcx, [rip + 0x1b6bcfb]                   ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021541b5: call   0x182f609b0
021541ba: lea    rcx, [rip + 0x1b6bdaf]                   ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
021541c1: call   0x182f609b0
021541c6: lea    rcx, [rip + 0x1b6be63]                   ; [0x3cc0030] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Current()
021541cd: call   0x182f609b0
021541d2: lea    rcx, [rip + 0x1b7bb27]                   ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
021541d9: call   0x182f609b0
021541de: lea    rcx, [rip + 0x1b4e60b]                   ; [0x3ca27f0] metamethod:Method$System.ReadOnlySpan<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Length()
021541e5: call   0x182f609b0
021541ea: lea    rcx, [rip + 0x1b4014f]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021541f1: call   0x182f609b0
021541f6: lea    rcx, [rip + 0x1b4089b]                   ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
021541fd: call   0x182f609b0
02154202: mov    byte ptr [rip + 0x1dc5495], 1            ; [0x3f1969e] (bss)
02154209: xorps  xmm0, xmm0
0215420c: movups xmmword ptr [rsp + 0x60], xmm0
02154211: xorps  xmm1, xmm1
02154214: xor    eax, eax
02154216: movups xmmword ptr [rsp + 0x70], xmm1
0215421b: mov    qword ptr [rsp + 0x80], rax
02154223: mov    rax, qword ptr [rbx]
02154226: test   rax, rax
02154229: je     0x1821544b0
0215422f: test   rdi, rdi
02154232: je     0x1821544b0
02154238: mov    r13, qword ptr [rax + 0x30]
0215423c: mov    qword ptr [rsp + 0x38], r13
02154241: xor    r12b, r12b
02154244: movzx  r14d, byte ptr [rax + 0x11]
02154249: mov    byte ptr [rsp + 0xc8], r14b
02154251: movzx  r15d, byte ptr [rax + 0x10]
02154256: mov    byte ptr [rsp + 0x30], r15b
0215425b: mov    r8, qword ptr [rip + 0x1b7ba9e]          ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
02154262: mov    rdx, rdi
02154265: lea    rcx, [rsp + 0x48]
0215426a: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0215426f: movups xmm0, xmmword ptr [rsp + 0x48]
02154274: movups xmmword ptr [rsp + 0x70], xmm0
02154279: movsd  xmm1, qword ptr [rsp + 0x58]
0215427f: movsd  qword ptr [rsp + 0x80], xmm1
02154288: xor    esi, esi
0215428a: mov    qword ptr [rsp + 0x48], rsi
0215428f: lea    rbx, [rsp + 0x70]
02154294: mov    qword ptr [rsp + 0x50], rbx
02154299: nop    dword ptr [rax]
021542a0: mov    rdx, qword ptr [rip + 0x1b6bcc9]         ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
021542a7: lea    rcx, [rsp + 0x70]
021542ac: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
021542b1: test   al, al
021542b3: je     0x1821543fa
021542b9: mov    rcx, qword ptr [rsp + 0x80]
021542c1: test   rcx, rcx
021542c4: je     0x1821542a0
021542c6: mov    rdi, rsi
021542c9: mov    rax, qword ptr [rip + 0x1b407c8]         ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
021542d0: cmp    qword ptr [rcx], rax
021542d3: cmove  rdi, rcx
021542d7: test   rdi, rdi
021542da: je     0x1821542a0
021542dc: cmp    qword ptr [rdi + 0x30], 0
021542e1: je     0x1821544c1
021542e7: test   r15b, r15b
021542ea: jne    0x18215430d
021542ec: xor    edx, edx
021542ee: movzx  ecx, byte ptr [rdi + 0x28]
021542f2: call   0x18210d780                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʳʺʾʶʷʻʷʴʴʶˀ
021542f7: movzx  r15d, r15b
021542fb: test   al, al
021542fd: mov    eax, 1
02154302: cmovne r15d, eax
02154306: mov    byte ptr [rsp + 0x30], r15b
0215430b: jmp    0x182154312
0215430d: mov    eax, 1
02154312: test   r14b, r14b
02154315: jne    0x182154331
02154317: cmp    byte ptr [rdi + 0x28], 0x6e
0215431b: jb     0x182154331
0215431d: movzx  r14d, r14b
02154321: cmp    byte ptr [rdi + 0x28], 0x70
02154325: cmovbe r14d, eax
02154329: mov    byte ptr [rsp + 0xc8], r14b
02154331: movzx  ecx, byte ptr [rdi + 0x28]
02154335: xor    edx, edx
02154337: call   0x18210d9c0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʼʴʷʽˀʺʻʹʿʺʲ
0215433c: cmp    al, 0xff
0215433e: je     0x1821542a0
02154344: mov    ecx, eax
02154346: and    ecx, 0x1f
02154349: mov    edi, 1
0215434e: shl    dil, cl
02154351: movzx  ecx, dil
02154355: and    cl, r12b
02154358: cmp    cl, dil
0215435b: je     0x1821542a0
02154361: test   r13, r13
02154364: je     0x1821544bc
0215436a: xor    r9d, r9d
0215436d: movzx  r8d, al
02154371: movzx  edx, byte ptr [rsp + 0xd0]
02154379: mov    rcx, r13
0215437c: call   0x1820cfd40
02154381: test   rax, rax
02154384: je     0x1821544b6
0215438a: mov    qword ptr [rsp + 0x28], rsi
0215438f: mov    dword ptr [rsp + 0x20], esi
02154393: xor    r9d, r9d
02154396: xor    r8d, r8d
02154399: xor    edx, edx
0215439b: mov    rcx, rax
0215439e: call   0x1820cec60
021543a3: mov    rdx, qword ptr [rip + 0x1b3ff96]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021543aa: cmp    dword ptr [rdx + 0xe0], 0
021543b1: jne    0x1821543c2
021543b3: mov    rcx, rdx
021543b6: call   0x182f60cf0
021543bb: mov    rdx, qword ptr [rip + 0x1b3ff7e]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021543c2: or     r12b, dil
021543c5: mov    rax, qword ptr [rdx + 0xb8]
021543cc: movzx  ecx, r12b
021543d0: and    ecx, dword ptr [rax + 8]
021543d3: cmp    ecx, dword ptr [rax + 8]
021543d6: jne    0x1821542a0
021543dc: movzx  eax, r15b
021543e0: or     al, r14b
021543e3: je     0x1821542a0
021543e9: mov    rdx, qword ptr [rip + 0x1b6bac0]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021543f0: mov    rcx, rbx
021543f3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021543f8: jmp    0x182154445
021543fa: mov    rdx, qword ptr [rip + 0x1b6baaf]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
02154401: mov    rcx, rbx
02154404: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02154409: jmp    0x182154440
0215440b: mov    rdx, qword ptr [rip + 0x1b6ba9e]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
02154412: mov    rcx, qword ptr [rsp + 0x50]
02154417: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0215441c: mov    rcx, qword ptr [rsp + 0x48]
02154421: test   rcx, rcx
02154424: jne    0x1821544c7
0215442a: xor    esi, esi
0215442c: mov    r13, qword ptr [rsp + 0x38]
02154431: movzx  r14d, byte ptr [rsp + 0xc8]
0215443a: movzx  r15d, byte ptr [rsp + 0x30]
02154440: test   r13, r13
02154443: je     0x1821544b0
02154445: xor    r9d, r9d
02154448: lea    r8, [rsp + 0x60]
0215444d: movzx  edx, byte ptr [rsp + 0xd0]
02154455: mov    rcx, r13
02154458: call   0x182152b20
0215445d: test   al, al
0215445f: je     0x182154493
02154461: mov    r8, qword ptr [rsp + 0x60]
02154466: mov    edx, dword ptr [rsp + 0x68]
0215446a: test   edx, edx
0215446c: jle    0x182154493
0215446e: nop    
02154470: cmp    esi, edx
02154472: jae    0x1821544cd
02154474: mov    eax, esi
02154476: mov    rcx, qword ptr [r8 + rax*8]
0215447a: test   rcx, rcx
0215447d: je     0x18215448d
0215447f: mov    byte ptr [rcx + 0xa1], r15b
02154486: mov    byte ptr [rcx + 0xa2], r14b
0215448d: inc    esi
0215448f: cmp    esi, edx
02154491: jl     0x182154472
02154493: lea    r11, [rsp + 0x90]
0215449b: mov    rbx, qword ptr [r11 + 0x30]
0215449f: mov    rsi, qword ptr [r11 + 0x48]
021544a3: mov    rsp, r11
021544a6: pop    r15
021544a8: pop    r14
021544aa: pop    r13
021544ac: pop    r12
021544ae: pop    rdi
021544af: ret    
021544b0: call   0x182f60c50
021544b5: nop    
021544b6: call   0x182f60c50
021544bb: nop    
021544bc: call   0x182f60c50
021544c1: call   0x182f60c50
021544c6: nop    
021544c7: call   0x182f60ce0
021544cc: int3   
021544cd: call   0x182f60c40
021544d2: int3   
021544d3: int3   
