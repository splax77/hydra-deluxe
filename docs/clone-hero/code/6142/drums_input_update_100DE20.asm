0100de20: mov    qword ptr [rsp + 0x10], rbx
0100de25: mov    qword ptr [rsp + 0x18], rsi
0100de2a: push   rdi
0100de2b: sub    rsp, 0x30
0100de2f: mov    eax, dword ptr [rcx + 0x2ac]
0100de35: mov    rsi, rdx
0100de38: movsd  xmm0, qword ptr [rcx + 0x2a4]
0100de40: mov    rbx, rcx
0100de43: movsd  qword ptr [rcx + 0x298], xmm0
0100de4b: mov    dword ptr [rcx + 0x2a0], eax
0100de51: xor    eax, eax
0100de53: mov    qword ptr [rcx + 0x2a4], rax
0100de5a: mov    dword ptr [rcx + 0x2ac], eax
0100de60: mov    rdi, qword ptr [rcx + 0x2b0]
0100de67: test   rdi, rdi
0100de6a: je     0x18100df72
0100de70: mov    rax, qword ptr [rdx + 0x20]
0100de74: mov    qword ptr [rsp + 0x40], rbp
0100de79: movaps xmmword ptr [rsp + 0x20], xmm6
0100de7e: movsd  xmm6, qword ptr [rcx + 0x100]
0100de86: mov    rcx, qword ptr [rax + 0xc0]
0100de8d: mov    rdx, qword ptr [rcx + 0x28]
0100de91: test   byte ptr [rdx + 0x135], 1
0100de98: jne    0x18100dea5
0100de9a: mov    rcx, rdx
0100de9d: call   0x182f65750
0100dea2: mov    rdx, rax
0100dea5: mov    r10, qword ptr [rdi]
0100dea8: xor    ecx, ecx
0100deaa: movzx  r8d, word ptr [r10 + 0x12e]
0100deb2: cmp    cx, r8w
0100deb6: jae    0x18100ded9
0100deb8: mov    r9, qword ptr [r10 + 0xb0]
0100debf: nop    
0100dec0: movzx  eax, cx
0100dec3: add    rax, rax
0100dec6: cmp    qword ptr [r9 + rax*8], rdx
0100deca: je     0x18100df55
0100ded0: inc    cx
0100ded3: cmp    cx, r8w
0100ded7: jb     0x18100dec0
0100ded9: xor    r8d, r8d
0100dedc: mov    rcx, rdi
0100dedf: call   0x182f65070
0100dee4: mov    r9, qword ptr [rax + 8]
0100dee8: lea    r8, [rbx + 0x2a4]
0100deef: movaps xmm1, xmm6
0100def2: mov    rcx, rdi
0100def5: call   qword ptr [rax]
0100def7: mov    rax, qword ptr [rbx]
0100defa: mov    rcx, rbx
0100defd: mov    rdx, qword ptr [rax + 0x2d0]
0100df04: call   qword ptr [rax + 0x2c8]
0100df0a: movaps xmm6, xmmword ptr [rsp + 0x20]
0100df0f: mov    rbp, qword ptr [rsp + 0x40]
0100df14: test   al, al
0100df16: je     0x18100df32
0100df18: mov    rax, qword ptr [rsi + 0x20]
0100df1c: mov    rcx, rbx
0100df1f: mov    rdx, qword ptr [rax + 0xc0]
0100df26: mov    rdx, qword ptr [rdx + 0xa0]
0100df2d: call   0x18100e6e0                              ; ʿʺʸʺʼʴʸʼʻʳʼ<ʾʸʸʻʷʸˁʾˁʽʸ>$$ʻʺʺʾʹʴʸʳʼʲʳ
0100df32: mov    rax, qword ptr [rbx]
0100df35: mov    rcx, rbx
0100df38: mov    rdx, qword ptr [rax + 0x2e0]
0100df3f: mov    rbx, qword ptr [rsp + 0x48]
0100df44: mov    rsi, qword ptr [rsp + 0x50]
0100df49: add    rsp, 0x30
0100df4d: pop    rdi
0100df4e: jmp    qword ptr [rax + 0x2d8]
0100df55: movzx  ecx, cx
0100df58: add    rcx, rcx
0100df5b: movsxd rax, dword ptr [r9 + rcx*8 + 8]
0100df60: shl    rax, 4
0100df64: add    rax, 0x138
0100df6a: add    rax, r10
0100df6d: jmp    0x18100dee4
0100df72: call   0x182f60c50
0100df77: int3   
0100df78: int3   
