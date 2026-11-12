001f0550: mov    qword ptr [rsp + 8], rbx
001f0555: push   rdi
001f0556: sub    rsp, 0x20
001f055a: xor    eax, eax
001f055c: mov    dword ptr [rcx + 0x1c], 0xffffffff
001f0563: mov    qword ptr [rcx + 0x24], rax
001f0567: mov    rdi, rdx
001f056a: mov    qword ptr [rcx + 0x14], rax
001f056e: mov    rbx, rcx
001f0571: mov    word ptr [rcx + 0x38], ax
001f0575: mov    byte ptr [rcx + 0x10], al
001f0578: mov    dword ptr [rcx + 0x20], eax
001f057b: test   rdx, rdx
001f057e: je     0x1801f05aa
001f0580: mov    rdx, qword ptr [rdx + 0x48]
001f0584: mov    qword ptr [rcx + 0x40], rdx
001f0588: add    rcx, 0x40
001f058c: call   0x182f5fc00
001f0591: movd   xmm0, dword ptr [rdi + 0x78]
001f0596: cvtdq2pd xmm0, xmm0
001f059a: movsd  qword ptr [rbx + 0x30], xmm0
001f059f: mov    rbx, qword ptr [rsp + 0x30]
001f05a4: add    rsp, 0x20
001f05a8: pop    rdi
001f05a9: ret    
001f05aa: call   0x182f60c50
001f05af: int3   
