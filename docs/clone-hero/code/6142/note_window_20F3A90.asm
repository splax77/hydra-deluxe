020f3a90: mov    qword ptr [rsp + 0x18], rbp
020f3a95: push   rsi
020f3a96: sub    rsp, 0x60
020f3a9a: cmp    byte ptr [rcx + 0x41], 0
020f3a9e: mov    rbp, rdx
020f3aa1: movaps xmmword ptr [rsp + 0x40], xmm7
020f3aa6: mov    rsi, rcx
020f3aa9: movaps xmmword ptr [rsp + 0x30], xmm8
020f3aaf: je     0x1820f3bb5
020f3ab5: cmp    byte ptr [rip + 0x1e25903], 0            ; [0x3f193bf] (bss)
020f3abc: movsd  xmm7, qword ptr [rcx + 0x38]
020f3ac1: movsd  xmm8, qword ptr [rcx + 0x30]
020f3ac7: jne    0x1820f3ae8
020f3ac9: lea    rcx, [rip + 0x1ba4b58]                   ; [0x3c98628] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʵʵʹʶʿˀʹʸʸʸˀ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f3ad0: call   0x182f609b0
020f3ad5: lea    rcx, [rip + 0x1ba4d84]                   ; [0x3c98860] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʷʹʲʿʷʿʲˁʴʷʲ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f3adc: call   0x182f609b0
020f3ae1: mov    byte ptr [rip + 0x1e258d7], 1            ; [0x3f193bf] (bss)
020f3ae8: test   rbp, rbp
020f3aeb: je     0x1820f3bdb
020f3af1: mov    r9, qword ptr [rip + 0x1ba4b30]          ; [0x3c98628] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʵʵʹʶʿˀʹʸʸʸˀ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f3af8: mov    rdx, qword ptr [rbp + 0x40]
020f3afc: mov    rcx, qword ptr [rsi + 0x1e0]
020f3b03: mov    qword ptr [rsp + 0x70], rbx
020f3b08: mov    ebx, dword ptr [rbp + 0x20]
020f3b0b: mov    r8d, ebx
020f3b0e: mov    qword ptr [rsp + 0x78], rdi
020f3b13: movaps xmmword ptr [rsp + 0x50], xmm6
020f3b18: call   0x1805dd8d0                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ʵʵʹʶʿˀʹʸʸʸˀ<object>
020f3b1d: mov    rcx, qword ptr [rip + 0x1ba4d3c]         ; [0x3c98860] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʷʹʲʿʷʿʲˁʴʷʲ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f3b24: xor    r9d, r9d
020f3b27: mov    rdx, qword ptr [rbp + 0x40]
020f3b2b: mov    r8d, ebx
020f3b2e: mov    qword ptr [rsp + 0x20], rcx
020f3b33: mov    rdi, rax
020f3b36: mov    rcx, qword ptr [rsi + 0x1e0]
020f3b3d: call   0x1805dded0                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ʷʹʲʿʷʿʲˁʴʷʲ<object>
020f3b42: movaps xmm3, xmm7
020f3b45: mov    qword ptr [rsp + 0x28], 0
020f3b4e: mov    r8, rdi
020f3b51: movsd  qword ptr [rsp + 0x20], xmm8
020f3b58: mov    rdx, rbp
020f3b5b: mov    rcx, rsi
020f3b5e: mov    rbx, rax
020f3b61: call   0x1820f7210                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ˀʵˁʴʾʶʽʺʼʴʹ
020f3b66: movaps xmm3, xmm7
020f3b69: mov    qword ptr [rsp + 0x28], 0
020f3b72: mov    r8, rbp
020f3b75: movsd  qword ptr [rsp + 0x20], xmm8
020f3b7c: mov    rdx, rbx
020f3b7f: mov    rcx, rsi
020f3b82: movaps xmm6, xmm0
020f3b85: call   0x1820f7210                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ˀʵˁʴʾʶʽʺʼʴʹ
020f3b8a: mov    rax, qword ptr [rsi]
020f3b8d: addsd  xmm0, xmm6
020f3b91: mov    rcx, rsi
020f3b94: mov    r8, qword ptr [rax + 0x270]
020f3b9b: movaps xmm1, xmm0
020f3b9e: call   qword ptr [rax + 0x268]
020f3ba4: movaps xmm6, xmmword ptr [rsp + 0x50]
020f3ba9: mov    rdi, qword ptr [rsp + 0x78]
020f3bae: mov    rbx, qword ptr [rsp + 0x70]
020f3bb3: jmp    0x1820f3bba
020f3bb5: movsd  xmm0, qword ptr [rcx + 0x20]
020f3bba: mulsd  xmm0, qword ptr [rip + 0xf703ee]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
020f3bc2: mov    rbp, qword ptr [rsp + 0x80]
020f3bca: movaps xmm7, xmmword ptr [rsp + 0x40]
020f3bcf: movaps xmm8, xmmword ptr [rsp + 0x30]
020f3bd5: add    rsp, 0x60
020f3bd9: pop    rsi
020f3bda: ret    
020f3bdb: call   0x182f60c50
020f3be0: int3   
020f3be1: int3   
