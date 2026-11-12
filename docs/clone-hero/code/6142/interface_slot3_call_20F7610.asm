020f7610: push   rbx
020f7612: sub    rsp, 0x50
020f7616: cmp    byte ptr [rip + 0x1e21d9c], 0            ; [0x3f193b9] (bss)
020f761d: mov    rbx, rcx
020f7620: jne    0x1820f7641
020f7622: lea    rcx, [rip + 0x1be0cf7]                   ; [0x3cd8320] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.get_Count()
020f7629: call   0x182f609b0
020f762e: lea    rcx, [rip + 0x1c1766b]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f7635: call   0x182f609b0
020f763a: mov    byte ptr [rip + 0x1e21d78], 1            ; [0x3f193b9] (bss)
020f7641: cmp    byte ptr [rbx + 0x43], 0
020f7645: je     0x1820f77f8
020f764b: mov    qword ptr [rsp + 0x68], rsi
020f7650: mov    qword ptr [rsp + 0x70], rdi
020f7655: mov    qword ptr [rsp + 0x40], r15
020f765a: cmp    byte ptr [rip + 0x1e21d53], 0            ; [0x3f193b4] (bss)
020f7661: mov    byte ptr [rbx + 0x43], 0
020f7665: jne    0x1820f767a
020f7667: lea    rcx, [rip + 0x1bab0fa]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f766e: call   0x182f609b0
020f7673: mov    byte ptr [rip + 0x1e21d3a], 1            ; [0x3f193b4] (bss)
020f767a: xor    r15d, r15d
020f767d: mov    qword ptr [rsp + 0x60], rbp
020f7682: cmp    dword ptr [rbx + 0x164], 0x32
020f7689: jl     0x1820f76ed
020f768b: mov    rcx, qword ptr [rip + 0x1bab0d6]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f7692: mov    edi, dword ptr [rbx + 0x168]
020f7698: mov    ebp, dword ptr [rbx + 0x16c]
020f769e: cmp    dword ptr [rcx + 0xe0], r15d
020f76a5: jne    0x1820f76ac
020f76a7: call   0x182f60cf0
020f76ac: xorps  xmm0, xmm0
020f76af: xorps  xmm1, xmm1
020f76b2: cvtsi2sd xmm0, edi
020f76b6: cvtsi2sd xmm1, ebp
020f76ba: divsd  xmm0, xmm1
020f76be: xorps  xmm1, xmm1
020f76c1: subsd  xmm0, qword ptr [rip + 0xf6c8e7]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
020f76c9: cvtsi2sd xmm1, edi
020f76cd: addsd  xmm0, xmm0
020f76d1: mulsd  xmm0, xmm1
020f76d5: mulsd  xmm0, qword ptr [rip + 0xf6cce3]         ; [0x30643c0] dbl=10.0 q=0x4024000000000000
020f76dd: call   0x182c49a60
020f76e2: cvttsd2si eax, xmm0
020f76e6: lea    esi, [rax + rax*4]
020f76e9: add    esi, esi
020f76eb: jmp    0x1820f76f0
020f76ed: mov    esi, r15d
020f76f0: add    dword ptr [rbx + 0xa0], esi
020f76f6: mov    rdi, qword ptr [rbx + 0x1a0]
020f76fd: test   rdi, rdi
020f7700: je     0x1820f77b0
020f7706: mov    qword ptr [rsp + 0x48], r14
020f770b: movaps xmmword ptr [rsp + 0x30], xmm6
020f7710: mov    eax, dword ptr [rbx + 0x16c]
020f7716: mov    r10, qword ptr [rdi]
020f7719: dec    eax
020f771b: cmp    dword ptr [rbx + 0x168], eax
020f7721: mov    ebp, dword ptr [rbx + 0x164]
020f7727: movsd  xmm6, qword ptr [rbx + 0x170]
020f772f: sete   r14b
020f7733: movzx  edx, word ptr [r10 + 0x12e]
020f773b: mov    r9, qword ptr [rip + 0x1c1755e]          ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f7742: cmp    r15w, dx
020f7746: jae    0x1820f7778
020f7748: mov    r8, qword ptr [r10 + 0xb0]
020f774f: mov    ecx, r15d
020f7752: nop    dword ptr [rax]
020f7756: nop    word ptr [rax + rax]
020f7760: movzx  eax, cx
020f7763: add    rax, rax
020f7766: cmp    qword ptr [r8 + rax*8], r9
020f776a: je     0x1820f77fe
020f7770: inc    cx
020f7773: cmp    cx, dx
020f7776: jb     0x1820f7760
020f7778: mov    r8d, 3
020f777e: mov    rdx, r9
020f7781: mov    rcx, rdi
020f7784: call   0x182f65070
020f7789: mov    r10, qword ptr [rax + 8]
020f778d: movzx  r9d, r14b
020f7791: mov    qword ptr [rsp + 0x28], r10
020f7796: mov    r8d, ebp
020f7799: mov    edx, esi
020f779b: movsd  qword ptr [rsp + 0x20], xmm6
020f77a1: mov    rcx, rdi
020f77a4: call   qword ptr [rax]
020f77a6: movaps xmm6, xmmword ptr [rsp + 0x30]
020f77ab: mov    r14, qword ptr [rsp + 0x48]
020f77b0: mov    rax, qword ptr [rbx + 0x200]
020f77b7: mov    rdi, qword ptr [rsp + 0x70]
020f77bc: mov    rsi, qword ptr [rsp + 0x68]
020f77c1: mov    rbp, qword ptr [rsp + 0x60]
020f77c6: test   rax, rax
020f77c9: je     0x1820f7820
020f77cb: mov    eax, dword ptr [rax + 0x18]
020f77ce: dec    eax
020f77d0: cmp    dword ptr [rbx + 0x160], eax
020f77d6: jge    0x1820f77de
020f77d8: inc    dword ptr [rbx + 0x160]
020f77de: mov    qword ptr [rbx + 0x164], r15
020f77e5: mov    dword ptr [rbx + 0x16c], r15d
020f77ec: mov    qword ptr [rbx + 0x170], r15
020f77f3: mov    r15, qword ptr [rsp + 0x40]
020f77f8: add    rsp, 0x50
020f77fc: pop    rbx
020f77fd: ret    
020f77fe: movzx  ecx, cx
020f7801: add    rcx, rcx
020f7804: mov    eax, dword ptr [r8 + rcx*8 + 8]
020f7809: add    eax, 3
020f780c: cdqe   
020f780e: shl    rax, 4
020f7812: add    rax, 0x138
020f7818: add    rax, r10
020f781b: jmp    0x1820f7789
020f7820: call   0x182f60c50
020f7825: int3   
020f7826: int3   
