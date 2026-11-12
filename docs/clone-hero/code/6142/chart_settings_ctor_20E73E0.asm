020e73e0: mov    qword ptr [rsp + 8], rbx
020e73e5: mov    qword ptr [rsp + 0x10], rsi
020e73ea: push   rdi
020e73eb: sub    rsp, 0x70
020e73ef: cmp    byte ptr [rip + 0x1e31f40], 0            ; [0x3f19336] (bss)
020e73f6: movzx  ebx, r8b
020e73fa: movaps xmmword ptr [rsp + 0x60], xmm6
020e73ff: movzx  esi, dl
020e7402: movaps xmm6, xmm3
020e7405: mov    rdi, rcx
020e7408: jne    0x1820e7429
020e740a: lea    rcx, [rip + 0x1bb9c5f]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020e7411: call   0x182f609b0
020e7416: lea    rcx, [rip + 0x1bcca53]                   ; [0x3cb3e70] str:''
020e741d: call   0x182f609b0
020e7422: mov    byte ptr [rip + 0x1e31f0d], 1            ; [0x3f19336] (bss)
020e7429: xor    edx, edx
020e742b: mov    rcx, rdi
020e742e: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020e7433: mov    eax, dword ptr [rsp + 0xa0]
020e743a: mov    dword ptr [rdi + 0x18], eax
020e743d: movzx  eax, byte ptr [rsp + 0xa8]
020e7445: mov    byte ptr [rdi + 0x1c], al
020e7448: mov    eax, dword ptr [rsp + 0xb0]
020e744f: mov    dword ptr [rdi + 0x20], eax
020e7452: mov    eax, dword ptr [rsp + 0xb8]
020e7459: mov    dword ptr [rdi + 0x24], eax
020e745c: mov    eax, dword ptr [rsp + 0xc0]
020e7463: mov    dword ptr [rdi + 0x28], eax
020e7466: movzx  eax, byte ptr [rsp + 0xc8]
020e746e: mov    byte ptr [rdi + 0x2c], al
020e7471: movss  dword ptr [rdi + 0x14], xmm6
020e7476: mov    byte ptr [rdi + 0x10], sil
020e747a: mov    byte ptr [rdi + 0x11], bl
020e747d: mov    rcx, qword ptr [rip + 0x1bb9bec]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020e7484: call   0x182f60c00
020e7489: mov    rdx, qword ptr [rip + 0x1bcc9e0]         ; [0x3cb3e70] str:''
020e7490: mov    rbx, rax
020e7493: xor    eax, eax
020e7495: mov    r9, rdx
020e7498: mov    qword ptr [rsp + 0x50], rax
020e749d: mov    r8, rdi
020e74a0: mov    dword ptr [rsp + 0x48], eax
020e74a4: mov    rcx, rbx
020e74a7: mov    qword ptr [rsp + 0x40], rdx
020e74ac: mov    qword ptr [rsp + 0x38], rdx
020e74b1: mov    qword ptr [rsp + 0x30], rdx
020e74b6: mov    qword ptr [rsp + 0x28], rdx
020e74bb: mov    qword ptr [rsp + 0x20], rdx
020e74c0: call   0x18212e990                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$.ctor
020e74c5: lea    rcx, [rdi + 0x30]
020e74c9: mov    qword ptr [rdi + 0x30], rbx
020e74cd: mov    rdx, rbx
020e74d0: movaps xmm6, xmmword ptr [rsp + 0x60]
020e74d5: lea    r11, [rsp + 0x70]
020e74da: mov    rbx, qword ptr [r11 + 0x10]
020e74de: mov    rsi, qword ptr [r11 + 0x18]
020e74e2: mov    rsp, r11
020e74e5: pop    rdi
020e74e6: jmp    0x182f5fc00
020e74eb: int3   
