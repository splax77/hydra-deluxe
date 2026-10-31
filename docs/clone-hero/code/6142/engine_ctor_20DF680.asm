020df680: push   rbp
020df682: push   rbx
020df683: push   rsi
020df684: push   rdi
020df685: lea    rbp, [rsp - 0x28]
020df68a: sub    rsp, 0x128
020df691: cmp    byte ptr [rip + 0x1e39c20], 0            ; [0x3f192b8] (bss)
020df698: mov    rdi, r8
020df69b: mov    rsi, rdx
020df69e: mov    rbx, rcx
020df6a1: jne    0x1820df6b6
020df6a3: lea    rcx, [rip + 0x1c20cfe]                   ; [0x3d003a8] metamethod:Method$ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>..ctor()
020df6aa: call   0x182f609b0
020df6af: mov    byte ptr [rip + 0x1e39c02], 1            ; [0x3f192b8] (bss)
020df6b6: movups xmm0, xmmword ptr [rsi]
020df6b9: mov    r9, qword ptr [rdi + 0x10]
020df6bd: lea    rcx, [rdi + 0x28]
020df6c1: movups xmm1, xmmword ptr [rsi + 0x10]
020df6c5: mov    r8, qword ptr [rdi + 8]
020df6c9: lea    rdx, [rbp - 0x80]
020df6cd: movups xmmword ptr [rbp - 0x80], xmm0
020df6d1: mov    byte ptr [rbx + 0x2f2], 1
020df6d8: movups xmm0, xmmword ptr [rsi + 0x20]
020df6dc: mov    rax, qword ptr [rip + 0x1c20cc5]         ; [0x3d003a8] metamethod:Method$ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>..ctor()
020df6e3: movups xmmword ptr [rbp - 0x70], xmm1
020df6e7: mov    qword ptr [rsp + 0x28], rax
020df6ec: movups xmm1, xmmword ptr [rsi + 0x30]
020df6f0: mov    qword ptr [rsp + 0x20], rcx
020df6f5: mov    rcx, rbx
020df6f8: movups xmmword ptr [rbp - 0x60], xmm0
020df6fc: movups xmm0, xmmword ptr [rsi + 0x40]
020df700: movups xmmword ptr [rbp - 0x50], xmm1
020df704: movups xmm1, xmmword ptr [rsi + 0x50]
020df708: movups xmmword ptr [rbp - 0x40], xmm0
020df70c: movups xmm0, xmmword ptr [rsi + 0x60]
020df710: movups xmmword ptr [rbp - 0x30], xmm1
020df714: movups xmm1, xmmword ptr [rsi + 0x70]
020df718: movups xmmword ptr [rbp - 0x20], xmm0
020df71c: movups xmm0, xmmword ptr [rsi + 0x80]
020df723: movups xmmword ptr [rbp - 0x10], xmm1
020df727: movups xmm1, xmmword ptr [rsi + 0x90]
020df72e: movups xmmword ptr [rbp], xmm0
020df732: movups xmmword ptr [rbp + 0x10], xmm1
020df736: call   0x18100eae0                              ; ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>$$.ctor
020df73b: mov    rdx, qword ptr [rdi]
020df73e: lea    rcx, [rbx + 0x318]
020df745: mov    qword ptr [rbx + 0x318], rdx
020df74c: call   0x182f5fc00
020df751: mov    eax, dword ptr [rdi + 0x18]
020df754: mov    byte ptr [rbx + 0x2f6], al
020df75a: mov    rax, qword ptr [rdi + 0x18]
020df75e: shr    rax, 0x20
020df762: mov    dword ptr [rbx + 0x2c0], eax
020df768: mov    rax, qword ptr [rbx + 0x230]
020df76f: test   rax, rax
020df772: je     0x1820df843
020df778: cvttsd2si rax, qword ptr [rax + 0xb8]
020df781: xor    ecx, ecx
020df783: xor    r8d, r8d
020df786: mov    dword ptr [rbx + 0x2c4], ecx
020df78c: mov    dl, 0xff
020df78e: mov    byte ptr [rbx + 0x2c8], cl
020df794: mov    qword ptr [rbx + 0x2d0], rcx
020df79b: mov    qword ptr [rbx + 0x2d8], rcx
020df7a2: mov    qword ptr [rbx + 0x2e0], rcx
020df7a9: mov    qword ptr [rbx + 0x2e8], rcx
020df7b0: mov    word ptr [rbx + 0x2f0], cx
020df7b7: shl    rax, 2
020df7bb: mov    qword ptr [rbx + 0x300], rax
020df7c2: mov    eax, dword ptr [rdi + 0x20]
020df7c5: mov    qword ptr [rbp + 0x50], rcx
020df7c9: lea    rcx, [rbp + 0x50]
020df7cd: mov    byte ptr [rbx + 0x308], al
020df7d3: call   0x1821770a0                              ; ʾʿʴʹʻʹʸʳʼʲʵ$$.ctor
020df7d8: test   dword ptr [rbx + 0x198], 0x1000
020df7e2: mov    rax, qword ptr [rbp + 0x50]
020df7e6: mov    qword ptr [rbx + 0x309], rax
020df7ed: mov    byte ptr [rbx + 0x41], 1
020df7f1: ja     0x1820df805
020df7f3: movsd  xmm0, qword ptr [rip + 0x1060ecd]        ; [0x31406c8] dbl=0.0375 q=0x3fa3333333333333
020df7fb: movsd  xmm1, qword ptr [rip + 0x1060edd]        ; [0x31406e0] dbl=0.085 q=0x3fb5c28f5c28f5c3
020df803: jmp    0x1820df815
020df805: movsd  xmm0, qword ptr [rip + 0x1060eab]        ; [0x31406b8] dbl=0.025 q=0x3f9999999999999a
020df80d: movsd  xmm1, qword ptr [rip + 0x1060ebb]        ; [0x31406d0] dbl=0.04 q=0x3fa47ae147ae147b
020df815: movsd  qword ptr [rbx + 0x30], xmm1
020df81a: xor    edx, edx
020df81c: movsd  qword ptr [rbx + 0x38], xmm0
020df821: mov    rcx, rbx
020df824: movsd  xmm0, qword ptr [rbx + 0x30]
020df829: addsd  xmm0, xmm0
020df82d: movsd  qword ptr [rbx + 0x20], xmm0
020df832: call   0x1820f74b0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ˁʲʷʹˀʸʷʽʽʷʳ
020df837: add    rsp, 0x128
020df83e: pop    rdi
020df83f: pop    rsi
020df840: pop    rbx
020df841: pop    rbp
020df842: ret    
020df843: call   0x182f60c50
020df848: int3   
020df849: int3   
