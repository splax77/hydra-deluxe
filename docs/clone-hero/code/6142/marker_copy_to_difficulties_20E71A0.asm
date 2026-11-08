020e71a0: push   rbx
020e71a2: push   rbp
020e71a3: push   rsi
020e71a4: push   rdi
020e71a5: push   r14
020e71a7: sub    rsp, 0x80
020e71ae: cmp    byte ptr [rip + 0x1e320fa], 0            ; [0x3f192af] (bss)
020e71b5: mov    rsi, r8
020e71b8: movzx  ebp, dl
020e71bb: mov    rdi, rcx
020e71be: jne    0x1820e71d3
020e71c0: lea    rcx, [rip + 0x1bb8dd1]                   ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020e71c7: call   0x182f609b0
020e71cc: mov    byte ptr [rip + 0x1e320dc], 1            ; [0x3f192af] (bss)
020e71d3: xor    r14d, r14d
020e71d6: mov    ebx, r14d
020e71d9: nop    dword ptr [rax]
020e71e0: mov    rax, qword ptr [rip + 0x1bb8db1]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020e71e7: cmp    dword ptr [rax + 0xe0], r14d
020e71ee: jne    0x1820e71ff
020e71f0: mov    rcx, rax
020e71f3: call   0x182f60cf0
020e71f8: mov    rax, qword ptr [rip + 0x1bb8d99]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020e71ff: mov    rcx, qword ptr [rax + 0xb8]
020e7206: cmp    ebx, dword ptr [rcx]
020e7208: jge    0x1820e729e
020e720e: mov    eax, dword ptr [rsi + 0x14]
020e7211: lea    rcx, [rsp + 0x40]
020e7216: movzx  r9d, byte ptr [rsi + 0x18]
020e721b: xorps  xmm0, xmm0
020e721e: mov    r8, qword ptr [rsi + 8]
020e7222: mov    rdx, qword ptr [rsi]
020e7225: mov    qword ptr [rsp + 0x30], r14
020e722a: mov    dword ptr [rsp + 0x28], eax
020e722e: movups xmmword ptr [rsp + 0x40], xmm0
020e7233: mov    byte ptr [rsp + 0x20], bl
020e7237: movups xmmword ptr [rsp + 0x50], xmm0
020e723c: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
020e7241: mov    rcx, qword ptr [rip + 0x1bb8d50]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020e7248: movups xmm0, xmmword ptr [rsp + 0x40]
020e724d: movups xmm1, xmmword ptr [rsp + 0x50]
020e7252: movups xmmword ptr [rsp + 0x60], xmm0
020e7257: movups xmmword ptr [rsp + 0x70], xmm1
020e725c: cmp    dword ptr [rcx + 0xe0], r14d
020e7263: jne    0x1820e726a
020e7265: call   0x182f60cf0
020e726a: test   rdi, rdi
020e726d: je     0x1820e72ac
020e726f: movzx  r8d, bl
020e7273: xor    r9d, r9d
020e7276: movzx  edx, bpl
020e727a: mov    rcx, rdi
020e727d: call   0x1820cfd40
020e7282: test   rax, rax
020e7285: je     0x1820e72ac
020e7287: xor    r8d, r8d
020e728a: lea    rdx, [rsp + 0x60]
020e728f: mov    rcx, rax
020e7292: call   0x1820ceb90
020e7297: inc    ebx
020e7299: jmp    0x1820e71e0
020e729e: add    rsp, 0x80
020e72a5: pop    r14
020e72a7: pop    rdi
020e72a8: pop    rsi
020e72a9: pop    rbp
020e72aa: pop    rbx
020e72ab: ret    
020e72ac: call   0x182f60c50
020e72b1: int3   
020e72b2: int3   
