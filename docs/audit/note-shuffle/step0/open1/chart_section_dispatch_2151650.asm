02151650: push   rbx
02151652: push   rbp
02151653: push   rsi
02151654: push   rdi
02151655: push   r14
02151657: push   r15
02151659: sub    rsp, 0x48
0215165d: mov    r14, qword ptr [r9]
02151660: mov    r15, r9
02151663: mov    qword ptr [rsp + 0x98], 0
0215166f: mov    edi, edx
02151671: mov    rsi, rcx
02151674: test   r14, r14
02151677: je     0x182151824
0215167d: movups xmm0, xmmword ptr [r8]
02151681: mov    r14, qword ptr [r14 + 0x30]
02151685: lea    rcx, [rsp + 0x30]
0215168a: xor    edx, edx
0215168c: movaps xmmword ptr [rsp + 0x30], xmm0
02151691: call   0x182151330
02151696: movzx  ebp, ax
02151699: movzx  ebx, ax
0215169c: shr    bp, 8
021516a0: movsx  eax, bpl
021516a4: sub    eax, 0xc
021516a7: cmp    eax, 1
021516aa: jbe    0x1821517ff
021516b0: cmp    bpl, 0xf
021516b4: je     0x1821517ff
021516ba: cmp    bl, 0xff
021516bd: je     0x1821517d9
021516c3: test   r14, r14
021516c6: je     0x182151824
021516cc: cmp    byte ptr [rip + 0x1dbccd5], 0            ; [0x3f0e3a8] (bss)
021516d3: jne    0x1821516e8
021516d5: lea    rcx, [rip + 0x1b7fd64]                   ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
021516dc: call   0x182f609b0
021516e1: mov    byte ptr [rip + 0x1dbccc0], 1            ; [0x3f0e3a8] (bss)
021516e8: xor    r9d, r9d
021516eb: movzx  r8d, bpl
021516ef: movzx  edx, bl
021516f2: mov    rcx, r14
021516f5: call   0x1803931c0
021516fa: cmp    eax, -1
021516fd: je     0x182151774
021516ff: mov    rcx, qword ptr [r14 + 0xd0]
02151706: test   rcx, rcx
02151709: je     0x182151824
0215170f: mov    r8, qword ptr [rip + 0x1b7fd2a]          ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
02151716: mov    edx, eax
02151718: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215171d: mov    rdx, rax
02151720: mov    qword ptr [rsp + 0x98], rax
02151728: lea    rcx, [rsp + 0x98]
02151730: call   0x182f5fc00
02151735: mov    rax, qword ptr [rsp + 0x98]
0215173d: test   rax, rax
02151740: je     0x18215178f
02151742: cmp    byte ptr [rax + 0xa0], 0
02151749: je     0x18215178f
0215174b: mov    rdx, qword ptr [rsi]
0215174e: mov    ecx, dword ptr [rsi + 8]
02151751: cmp    edi, ecx
02151753: jae    0x18215182a
02151759: movsxd rax, edi
0215175c: inc    edi
0215175e: cmp    word ptr [rdx + rax*2], 0x7d
02151763: jne    0x182151751
02151765: mov    eax, edi
02151767: add    rsp, 0x48
0215176b: pop    r15
0215176d: pop    r14
0215176f: pop    rdi
02151770: pop    rsi
02151771: pop    rbp
02151772: pop    rbx
02151773: ret    
02151774: xor    edx, edx
02151776: mov    qword ptr [rsp + 0x98], 0
02151782: lea    rcx, [rsp + 0x98]
0215178a: call   0x182f5fc00
0215178f: xor    r9d, r9d
02151792: movzx  r8d, bl
02151796: movzx  edx, bpl
0215179a: mov    rcx, r14
0215179d: call   0x1820cfd40
021517a2: movups xmm0, xmmword ptr [rsi]
021517a5: mov    qword ptr [rsp + 0x28], 0
021517ae: lea    rdx, [rsp + 0x30]
021517b3: movzx  r9d, bpl
021517b7: mov    qword ptr [rsp + 0x20], r15
021517bc: mov    r8d, edi
021517bf: movaps xmmword ptr [rsp + 0x30], xmm0
021517c4: mov    rcx, rax
021517c7: call   0x18213dd40                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʺʼˁʳˁˀʻʿʵˀʴ
021517cc: add    rsp, 0x48
021517d0: pop    r15
021517d2: pop    r14
021517d4: pop    rdi
021517d5: pop    rsi
021517d6: pop    rbp
021517d7: pop    rbx
021517d8: ret    
021517d9: mov    rdx, qword ptr [rsi]
021517dc: mov    ecx, dword ptr [rsi + 8]
021517df: nop    
021517e0: cmp    edi, ecx
021517e2: jae    0x18215182a
021517e4: movsxd rax, edi
021517e7: inc    edi
021517e9: cmp    word ptr [rdx + rax*2], 0x7d
021517ee: jne    0x1821517e0
021517f0: mov    eax, edi
021517f2: add    rsp, 0x48
021517f6: pop    r15
021517f8: pop    r14
021517fa: pop    rdi
021517fb: pop    rsi
021517fc: pop    rbp
021517fd: pop    rbx
021517fe: ret    
021517ff: mov    rdx, qword ptr [rsi]
02151802: mov    ecx, dword ptr [rsi + 8]
02151805: cmp    edi, ecx
02151807: jae    0x18215182a
02151809: movsxd rax, edi
0215180c: inc    edi
0215180e: cmp    word ptr [rdx + rax*2], 0x7d
02151813: jne    0x182151805
02151815: mov    eax, edi
02151817: add    rsp, 0x48
0215181b: pop    r15
0215181d: pop    r14
0215181f: pop    rdi
02151820: pop    rsi
02151821: pop    rbp
02151822: pop    rbx
02151823: ret    
02151824: call   0x182f60c50
02151829: int3   
0215182a: call   0x182f60c40
0215182f: int3   
