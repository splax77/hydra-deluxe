020f4b60: push   rbx
020f4b62: push   rbp
020f4b63: push   rsi
020f4b64: push   rdi
020f4b65: push   r14
020f4b67: sub    rsp, 0x30
020f4b6b: cmp    byte ptr [rip + 0x1e24850], 0            ; [0x3f193c2] (bss)
020f4b72: movzx  ebp, r8b
020f4b76: movzx  r14d, dl
020f4b7a: mov    rbx, rcx
020f4b7d: jne    0x1820f4b92
020f4b7f: lea    rcx, [rip + 0x1c1a11a]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f4b86: call   0x182f609b0
020f4b8b: mov    byte ptr [rip + 0x1e24830], 1            ; [0x3f193c2] (bss)
020f4b92: mov    esi, dword ptr [rbx + 0x88]
020f4b98: xor    dil, dil
020f4b9b: cmp    byte ptr [rbx + 0x46], dil
020f4b9f: je     0x1820f4bb8
020f4ba1: mov    rax, qword ptr [rbx]
020f4ba4: mov    rcx, rbx
020f4ba7: mov    rdx, qword ptr [rax + 0x2a0]
020f4bae: call   qword ptr [rax + 0x298]
020f4bb4: mov    byte ptr [rbx + 0x46], dil
020f4bb8: cmp    dword ptr [rbx + 0x190], 1
020f4bbf: je     0x1820f4bf8
020f4bc1: mov    rax, qword ptr [rbx]
020f4bc4: mov    rcx, rbx
020f4bc7: mov    dword ptr [rbx + 0x88], 0
020f4bd1: mov    rdx, qword ptr [rax + 0x1e0]
020f4bd8: call   qword ptr [rax + 0x1d8]
020f4bde: xor    r8d, r8d
020f4be1: xor    edx, edx
020f4be3: mov    rcx, rbx
020f4be6: call   0x1820f4e30                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʷʸˁʾʿʹʿʽʲˀʾ
020f4beb: cmp    byte ptr [rbx + 0x44], dil
020f4bef: jne    0x1820f4bf8
020f4bf1: mov    byte ptr [rbx + 0x44], 1
020f4bf5: mov    dil, 1
020f4bf8: mov    rbx, qword ptr [rbx + 0x1a0]
020f4bff: test   rbx, rbx
020f4c02: je     0x1820f4c73
020f4c04: mov    r10, qword ptr [rbx]
020f4c07: xor    eax, eax
020f4c09: mov    r9, qword ptr [rip + 0x1c1a090]          ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f4c10: movzx  edx, word ptr [r10 + 0x12e]
020f4c18: cmp    ax, dx
020f4c1b: jae    0x1820f4c44
020f4c1d: mov    r8, qword ptr [r10 + 0xb0]
020f4c24: xor    ecx, ecx
020f4c26: nop    word ptr [rax + rax]
020f4c30: movzx  eax, cx
020f4c33: add    rax, rax
020f4c36: cmp    qword ptr [r8 + rax*8], r9
020f4c3a: je     0x1820f4c7e
020f4c3c: inc    cx
020f4c3f: cmp    cx, dx
020f4c42: jb     0x1820f4c30
020f4c44: mov    r8d, 0xc
020f4c4a: mov    rdx, r9
020f4c4d: mov    rcx, rbx
020f4c50: call   0x182f65070
020f4c55: mov    rdx, qword ptr [rax + 8]
020f4c59: mov    r9d, esi
020f4c5c: mov    qword ptr [rsp + 0x28], rdx
020f4c61: movzx  r8d, bpl
020f4c65: movzx  edx, r14b
020f4c69: mov    byte ptr [rsp + 0x20], dil
020f4c6e: mov    rcx, rbx
020f4c71: call   qword ptr [rax]
020f4c73: add    rsp, 0x30
020f4c77: pop    r14
020f4c79: pop    rdi
020f4c7a: pop    rsi
020f4c7b: pop    rbp
020f4c7c: pop    rbx
020f4c7d: ret    
020f4c7e: movzx  ecx, cx
020f4c81: add    rcx, rcx
020f4c84: mov    eax, dword ptr [r8 + rcx*8 + 8]
020f4c89: add    eax, 0xc
020f4c8c: cdqe   
020f4c8e: shl    rax, 4
020f4c92: add    rax, 0x138
020f4c98: add    rax, r10
020f4c9b: jmp    0x1820f4c55
020f4c9d: int3   
