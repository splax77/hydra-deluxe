020d2440: push   rbp
020d2442: push   rbx
020d2443: push   r15
020d2445: lea    rbp, [rsp - 0x47]
020d244a: sub    rsp, 0x90
020d2451: cmp    byte ptr [rip + 0x1e46eab], 0            ; [0x3f19303] (bss)
020d2458: mov    rbx, rdx
020d245b: mov    r15, rcx
020d245e: jne    0x1820d247f
020d2460: lea    rcx, [rip + 0x1c04d79]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d2467: call   0x182f609b0
020d246c: lea    rcx, [rip + 0x1c04e2d]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2473: call   0x182f609b0
020d2478: mov    byte ptr [rip + 0x1e46e84], 1            ; [0x3f19303] (bss)
020d247f: xorps  xmm0, xmm0
020d2482: mov    qword ptr [rbp + 0x1f], 0
020d248a: xor    eax, eax
020d248c: mov    qword ptr [rbp + 0x27], rbx
020d2490: xorps  xmm1, xmm1
020d2493: mov    qword ptr [rbp - 0x19], rax
020d2497: mov    rdx, rbx
020d249a: mov    qword ptr [rbp - 1], rax
020d249e: lea    rcx, [rbp + 0x27]
020d24a2: movdqu xmmword ptr [rbp + 0x2f], xmm0
020d24a7: movups xmmword ptr [rbp - 0x29], xmm1
020d24ab: movups xmmword ptr [rbp - 0x11], xmm0
020d24af: call   0x182f5fc00
020d24b4: xor    edx, edx
020d24b6: cmp    byte ptr [rip + 0x1e46e54], dl           ; [0x3f19310] (bss)
020d24bc: mov    dword ptr [rbp + 0x1f], edx
020d24bf: mov    qword ptr [rbp + 0x37], rdx
020d24c3: mov    qword ptr [rbp + 0x2f], rdx
020d24c7: jne    0x1820d24eb
020d24c9: lea    rcx, [rip + 0x1bfd530]                   ; [0x3ccfa00] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Count()
020d24d0: call   0x182f609b0
020d24d5: lea    rcx, [rip + 0x1bfd5e4]                   ; [0x3ccfac0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Item()
020d24dc: call   0x182f609b0
020d24e1: mov    edx, dword ptr [rbp + 0x1f]
020d24e4: mov    byte ptr [rip + 0x1e46e25], 1            ; [0x3f19310] (bss)
020d24eb: mov    rcx, qword ptr [rbp + 0x27]
020d24ef: mov    qword ptr [rsp + 0xb0], rsi
020d24f7: mov    qword ptr [rsp + 0xb8], rdi
020d24ff: mov    qword ptr [rsp + 0xc0], r14
020d2507: test   rcx, rcx
020d250a: je     0x1820d2694
020d2510: mov    eax, dword ptr [rcx + 0x18]
020d2513: dec    eax
020d2515: cmp    edx, eax
020d2517: jg     0x1820d253e
020d2519: mov    r8, qword ptr [rip + 0x1bfd5a0]          ; [0x3ccfac0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Item()
020d2520: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2525: test   rax, rax
020d2528: je     0x1820d2694
020d252e: mov    rcx, qword ptr [rax + 0x10]
020d2532: mov    qword ptr [rbp + 0x2f], rcx
020d2536: add    rcx, qword ptr [rax + 0x18]
020d253a: mov    qword ptr [rbp + 0x37], rcx
020d253e: test   r15, r15
020d2541: je     0x1820d2694
020d2547: mov    r14d, dword ptr [r15 + 0x18]
020d254b: xor    edi, edi
020d254d: test   r14d, r14d
020d2550: jle    0x1820d2670
020d2556: nop    word ptr [rax + rax]
020d2560: mov    r8, qword ptr [rip + 0x1c04d39]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2567: mov    edx, edi
020d2569: mov    rcx, r15
020d256c: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2571: mov    rbx, rax
020d2574: test   rax, rax
020d2577: je     0x1820d2694
020d257d: mov    rcx, qword ptr [rbp + 0x2f]
020d2581: cmp    qword ptr [rax + 0x40], rcx
020d2585: jl     0x1820d2665
020d258b: lea    ecx, [rdi + 1]
020d258e: cmp    r14d, ecx
020d2591: jle    0x1820d25aa
020d2593: mov    r8, qword ptr [rip + 0x1c04d06]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d259a: lea    edx, [rdi + 1]
020d259d: mov    rcx, r15
020d25a0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d25a5: mov    rsi, rax
020d25a8: jmp    0x1820d25ac
020d25aa: xor    esi, esi
020d25ac: mov    rcx, qword ptr [rbp + 0x37]
020d25b0: cmp    qword ptr [rbx + 0x40], rcx
020d25b4: jl     0x1820d25c8
020d25b6: inc    dword ptr [rbp + 0x1f]
020d25b9: lea    rcx, [rbp + 0x1f]
020d25bd: xor    edx, edx
020d25bf: call   0x1820d0330                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʴˁʽʳʾʿˁʽʼʸʺ
020d25c4: mov    rcx, qword ptr [rbp + 0x37]
020d25c8: mov    rax, qword ptr [rbp + 0x2f]
020d25cc: cmp    qword ptr [rbx + 0x40], rax
020d25d0: jl     0x1820d2665
020d25d6: cmp    qword ptr [rbx + 0x40], rcx
020d25da: jge    0x1820d2665
020d25e0: or     dword ptr [rbx + 0x7c], 0x80
020d25e7: test   rsi, rsi
020d25ea: je     0x1820d25f6
020d25ec: mov    rax, qword ptr [rbp + 0x37]
020d25f0: cmp    qword ptr [rsi + 0x40], rax
020d25f4: jl     0x1820d2665
020d25f6: xor    r8d, r8d
020d25f9: lea    rcx, [rbp + 7]
020d25fd: mov    rdx, rbx
020d2600: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d2605: xor    r8d, r8d
020d2608: lea    rdx, [rbp - 0x11]
020d260c: lea    rcx, [rbp + 7]
020d2610: movups xmm0, xmmword ptr [rax]
020d2613: movups xmmword ptr [rbp - 0x11], xmm0
020d2617: movsd  xmm1, qword ptr [rax + 0x10]
020d261c: movsd  qword ptr [rbp - 1], xmm1
020d2621: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d2626: xor    edx, edx
020d2628: lea    rcx, [rbp - 0x29]
020d262c: movups xmm0, xmmword ptr [rax]
020d262f: movups xmmword ptr [rbp - 0x29], xmm0
020d2633: movsd  xmm1, qword ptr [rax + 0x10]
020d2638: movsd  qword ptr [rbp - 0x19], xmm1
020d263d: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d2642: test   al, al
020d2644: je     0x1820d2665
020d2646: mov    rax, qword ptr [rbp - 0x21]
020d264a: test   rax, rax
020d264d: je     0x1820d2694
020d264f: or     dword ptr [rax + 0x7c], 0x100
020d2656: lea    rcx, [rbp - 0x29]
020d265a: xor    edx, edx
020d265c: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d2661: test   al, al
020d2663: jne    0x1820d2646
020d2665: inc    edi
020d2667: cmp    edi, r14d
020d266a: jl     0x1820d2560
020d2670: mov    r14, qword ptr [rsp + 0xc0]
020d2678: mov    rdi, qword ptr [rsp + 0xb8]
020d2680: mov    rsi, qword ptr [rsp + 0xb0]
020d2688: add    rsp, 0x90
020d268f: pop    r15
020d2691: pop    rbx
020d2692: pop    rbp
020d2693: ret    
020d2694: call   0x182f60c50
020d2699: int3   
020d269a: int3   
