020f4950: mov    rax, rsp
020f4953: mov    qword ptr [rax + 8], rbx
020f4957: mov    qword ptr [rax + 0x10], rsi
020f495b: push   rdi
020f495c: sub    rsp, 0x80
020f4963: movzx  esi, byte ptr [rcx + 0x290]
020f496a: mov    r8b, 1
020f496d: movaps xmmword ptr [rax - 0x18], xmm6
020f4971: mov    rbx, rcx
020f4974: movaps xmmword ptr [rax - 0x28], xmm7
020f4978: movaps xmmword ptr [rax - 0x38], xmm8
020f497d: movaps xmm8, xmm1
020f4981: movaps xmmword ptr [rax - 0x48], xmm9
020f4986: movaps xmmword ptr [rax - 0x58], xmm10
020f498b: mov    rax, qword ptr [rcx]
020f498e: mov    r9, qword ptr [rax + 0x1c0]
020f4995: call   qword ptr [rax + 0x1b8]
020f499b: cmp    byte ptr [rbx + 0x290], 0
020f49a2: je     0x1820f4a58
020f49a8: movsd  xmm9, qword ptr [rip + 0xf718cf]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f49b1: xor    edi, edi
020f49b3: movups xmm10, xmmword ptr [rbx + 0x100]
020f49bb: movaps xmm7, xmm9
020f49bf: nop    
020f49c0: xor    r9d, r9d
020f49c3: movaps xmm2, xmm8
020f49c7: movaps xmm1, xmm7
020f49ca: mov    rcx, rbx
020f49cd: call   0x1820f6ff0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʿˁʿʶʴʸʵʶʿʲˁ
020f49d2: ucomisd xmm0, xmm9
020f49d7: movaps xmm6, xmm0
020f49da: jp     0x1820f49de
020f49dc: je     0x1820f4a4e
020f49de: comisd xmm7, xmm6
020f49e2: jae    0x1820f4a4e
020f49e4: mov    rcx, qword ptr [rbx + 0x230]
020f49eb: test   rcx, rcx
020f49ee: je     0x1820f4b3e
020f49f4: xor    r8d, r8d
020f49f7: movaps xmm1, xmm6
020f49fa: movaps xmm7, xmm6
020f49fd: call   0x18212bcc0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʽʸʴʷʷʼʵʳʻʾ
020f4a02: xorps  xmm0, xmm0
020f4a05: lea    rcx, [rsp + 0x20]
020f4a0a: xor    r9d, r9d
020f4a0d: mov    r8, rax
020f4a10: movaps xmm1, xmm6
020f4a13: movups xmmword ptr [rsp + 0x20], xmm0
020f4a18: call   0x182117120                              ; ʴʻʽʻʲʾʸʵʺʶʲ$$.ctor
020f4a1d: movups xmm0, xmmword ptr [rsp + 0x20]
020f4a22: mov    rax, qword ptr [rbx]
020f4a25: xor    r8d, r8d
020f4a28: movaps xmm1, xmm8
020f4a2c: mov    rcx, rbx
020f4a2f: movups xmmword ptr [rbx + 0x100], xmm0
020f4a36: mov    r9, qword ptr [rax + 0x1c0]
020f4a3d: call   qword ptr [rax + 0x1b8]
020f4a43: inc    edi
020f4a45: cmp    edi, 3
020f4a48: jl     0x1820f49c0
020f4a4e: movups xmmword ptr [rbx + 0x100], xmm10
020f4a56: jmp    0x1820f4a6f
020f4a58: movsd  xmm1, qword ptr [rip + 0xf71820]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f4a60: xor    r9d, r9d
020f4a63: movaps xmm2, xmm8
020f4a67: mov    rcx, rbx
020f4a6a: call   0x1820f6ff0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʿˁʿʶʴʸʵʶʿʲˁ
020f4a6f: mov    byte ptr [rbx + 0x290], sil
020f4a76: mov    rcx, qword ptr [rbx + 0x230]
020f4a7d: test   rcx, rcx
020f4a80: je     0x1820f4b38
020f4a86: xor    r8d, r8d
020f4a89: movaps xmm1, xmm8
020f4a8d: call   0x18212bcc0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʽʸʴʷʷʼʵʳʻʾ
020f4a92: xorps  xmm0, xmm0
020f4a95: lea    rcx, [rsp + 0x20]
020f4a9a: xor    r9d, r9d
020f4a9d: mov    r8, rax
020f4aa0: movaps xmm1, xmm8
020f4aa4: mov    rdi, rax
020f4aa7: movups xmmword ptr [rsp + 0x20], xmm0
020f4aac: call   0x182117120                              ; ʴʻʽʻʲʾʸʵʺʶʲ$$.ctor
020f4ab1: movups xmm0, xmmword ptr [rsp + 0x20]
020f4ab6: mov    rcx, qword ptr [rbx + 0x1d8]
020f4abd: movups xmmword ptr [rbx + 0x100], xmm0
020f4ac4: test   rcx, rcx
020f4ac7: je     0x1820f4b38
020f4ac9: xor    r9d, r9d
020f4acc: movaps xmm2, xmm8
020f4ad0: mov    rdx, rdi
020f4ad3: call   0x1820dc060                              ; ʹʷʽʶʶʸʻʿˁʾʵ$$ʽʾʿʴʳʵʻʻʿʶʼ
020f4ad8: mov    r8, qword ptr [rbx + 0x148]
020f4adf: xor    r9d, r9d
020f4ae2: mov    rdx, qword ptr [rbx + 0x108]
020f4ae9: mov    rcx, qword ptr [rbx + 0x208]
020f4af0: call   0x1820d4aa0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʽʿʽʹʿʲʹʴʸʹʳ
020f4af5: lea    rcx, [rbx + 0x148]
020f4afc: mov    qword ptr [rbx + 0x148], rax
020f4b03: mov    rdx, rax
020f4b06: movaps xmm6, xmmword ptr [rsp + 0x70]
020f4b0b: lea    r11, [rsp + 0x80]
020f4b13: mov    rbx, qword ptr [r11 + 0x10]
020f4b17: mov    rsi, qword ptr [r11 + 0x18]
020f4b1b: movaps xmm8, xmmword ptr [r11 - 0x30]
020f4b20: movaps xmm9, xmmword ptr [r11 - 0x40]
020f4b25: movaps xmm10, xmmword ptr [r11 - 0x50]
020f4b2a: movaps xmm7, xmmword ptr [rsp + 0x60]
020f4b2f: mov    rsp, r11
020f4b32: pop    rdi
020f4b33: jmp    0x182f5fc00
020f4b38: call   0x182f60c50
020f4b3d: int3   
020f4b3e: call   0x182f60c50
020f4b43: int3   
020f4b44: int3   
