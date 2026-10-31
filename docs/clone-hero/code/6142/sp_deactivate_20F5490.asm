020f5490: push   rbx
020f5492: sub    rsp, 0x20
020f5496: cmp    byte ptr [rip + 0x1e23f2e], 0            ; [0x3f193cb] (bss)
020f549d: mov    rbx, rcx
020f54a0: jne    0x1820f54b5
020f54a2: lea    rcx, [rip + 0x1c197f7]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f54a9: call   0x182f609b0
020f54ae: mov    byte ptr [rip + 0x1e23f16], 1            ; [0x3f193cb] (bss)
020f54b5: mov    eax, dword ptr [rbx + 0x48]
020f54b8: xor    ecx, ecx
020f54ba: mov    r8, qword ptr [rbx + 0x1a0]
020f54c1: cdq    
020f54c2: sub    eax, edx
020f54c4: mov    byte ptr [rbx + 0x42], 0
020f54c8: sar    eax, 1
020f54ca: mov    dword ptr [rbx + 0x48], eax
020f54cd: mov    qword ptr [rbx + 0x68], rcx
020f54d1: movsd  xmm0, qword ptr [rbx + 0x100]
020f54d9: subsd  xmm0, qword ptr [rbx + 0x110]
020f54e1: addsd  xmm0, qword ptr [rbx + 0xb8]
020f54e9: movsd  qword ptr [rbx + 0xb8], xmm0
020f54f1: mov    rax, qword ptr [rbx + 0x100]
020f54f8: mov    qword ptr [rbx + 0x128], rax
020f54ff: mov    qword ptr [rbx + 0x138], rcx
020f5506: mov    qword ptr [rbx + 0x140], rcx
020f550d: mov    qword ptr [rbx + 0x120], rcx
020f5514: test   r8, r8
020f5517: je     0x1820f552f
020f5519: mov    rdx, qword ptr [rip + 0x1c19780]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f5520: mov    ecx, 0xe
020f5525: add    rsp, 0x20
020f5529: pop    rbx
020f552a: jmp    0x182c42550
020f552f: add    rsp, 0x20
020f5533: pop    rbx
020f5534: ret    
020f5535: int3   
