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
