0214bcd0: mov    qword ptr [rsp + 8], rbx
0214bcd5: push   rbp
0214bcd6: mov    rbp, rsp
0214bcd9: sub    rsp, 0x50
0214bcdd: xorps  xmm0, xmm0
0214bce0: mov    qword ptr [rbp - 0x18], rcx
0214bce4: xor    eax, eax
0214bce6: mov    rdx, rcx
0214bce9: lea    rcx, [rbp - 0x18]
0214bced: mov    qword ptr [rbp - 0x20], rax
0214bcf1: movups xmmword ptr [rbp - 0x30], xmm0
0214bcf5: xor    ebx, ebx
0214bcf7: movdqu xmmword ptr [rbp - 0x10], xmm0
0214bcfc: call   0x182f5fc00
0214bd01: mov    rdx, qword ptr [rbp - 0x18]
0214bd05: lea    rcx, [rbp - 0x10]
0214bd09: mov    qword ptr [rbp - 0x10], rdx
0214bd0d: call   0x182f5fc00
0214bd12: movups xmm0, xmmword ptr [rbp - 0x18]
0214bd16: mov    dword ptr [rbp - 8], ebx
0214bd19: movsd  xmm1, qword ptr [rbp - 8]
0214bd1e: movups xmmword ptr [rbp - 0x30], xmm0
0214bd22: mov    rax, qword ptr [rbp - 0x28]
0214bd26: movsd  qword ptr [rbp - 0x20], xmm1
0214bd2b: nop    dword ptr [rax + rax]
0214bd30: mov    edx, dword ptr [rbp - 0x20]
0214bd33: test   edx, edx
0214bd35: je     0x18214bd4a
0214bd37: cmp    edx, 1
0214bd3a: je     0x18214bd81
0214bd3c: movzx  eax, bx
0214bd3f: mov    rbx, qword ptr [rsp + 0x60]
0214bd44: add    rsp, 0x50
0214bd48: pop    rbp
0214bd49: ret    
0214bd4a: test   rax, rax
0214bd4d: je     0x18214bd65
0214bd4f: cmp    qword ptr [rax + 0x18], 0
0214bd54: je     0x18214bd65
0214bd56: test   rax, rax
0214bd59: je     0x18214bddf
0214bd5f: mov    rdx, qword ptr [rax + 0x18]
0214bd63: jmp    0x18214bd96
0214bd65: mov    rdx, qword ptr [rbp - 0x30]
0214bd69: lea    rcx, [rbp - 0x28]
0214bd6d: mov    qword ptr [rbp - 0x28], rdx
0214bd71: mov    dword ptr [rbp - 0x20], 1
0214bd78: call   0x182f5fc00
0214bd7d: mov    rax, qword ptr [rbp - 0x28]
0214bd81: test   rax, rax
0214bd84: je     0x18214bda9
0214bd86: cmp    qword ptr [rax + 0x10], 0
0214bd8b: je     0x18214bda9
0214bd8d: test   rax, rax
0214bd90: je     0x18214bddf
0214bd92: mov    rdx, qword ptr [rax + 0x10]
0214bd96: lea    rcx, [rbp - 0x28]
0214bd9a: mov    qword ptr [rbp - 0x28], rdx
0214bd9e: call   0x182f5fc00
0214bda3: mov    rax, qword ptr [rbp - 0x28]
0214bda7: jmp    0x18214bdce
0214bda9: mov    rdx, qword ptr [rbp - 0x30]
0214bdad: lea    rcx, [rbp - 0x28]
0214bdb1: mov    qword ptr [rbp - 0x28], rdx
0214bdb5: mov    dword ptr [rbp - 0x20], 2
0214bdbc: call   0x182f5fc00
0214bdc1: mov    rax, qword ptr [rbp - 0x28]
0214bdc5: test   rax, rax
0214bdc8: je     0x18214bd3c
0214bdce: test   rax, rax
0214bdd1: je     0x18214bddf
0214bdd3: or     bx, word ptr [rax + 0x80]
0214bdda: jmp    0x18214bd30
0214bddf: call   0x182f60c50
0214bde4: int3   
0214bde5: int3   
