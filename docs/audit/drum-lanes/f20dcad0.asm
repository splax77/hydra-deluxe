020dcad0: mov    qword ptr [rsp + 8], rbx
020dcad5: push   rdi
020dcad6: sub    rsp, 0x20
020dcada: mov    rdi, rdx
020dcadd: mov    rbx, rcx
020dcae0: xor    edx, edx
020dcae2: call   0x1820f57f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʿʺʺʽʲʿʶˀʸʷ
020dcae7: test   al, al
020dcae9: je     0x1820dcb06
020dcaeb: test   rdi, rdi
020dcaee: je     0x1820dcc82
020dcaf4: xor    edx, edx
020dcaf6: mov    rcx, rdi
020dcaf9: call   0x18214bb50                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʾʿʺʺʿʸʶʴʾʺʶ
020dcafe: test   al, al
020dcb00: jne    0x1820dcbf7
020dcb06: test   rdi, rdi
020dcb09: je     0x1820dcc82
020dcb0f: movzx  eax, word ptr [rbx + 0x2c4]
020dcb16: test   word ptr [rdi + 0x80], ax
020dcb1d: je     0x1820dcb3f
020dcb1f: cmp    byte ptr [rbx + 0x2c8], 0
020dcb26: mov    eax, 4
020dcb2b: mov    ecx, 2
020dcb30: cmove  eax, ecx
020dcb33: cmp    dword ptr [rbx + 0x2e8], eax
020dcb39: jb     0x1820dcbf1
020dcb3f: test   dword ptr [rdi + 0x7c], 0x8000
020dcb46: je     0x1820dcc18
020dcb4c: cmp    word ptr [rbx + 0x2c4], 0
020dcb54: je     0x1820dcc18
020dcb5a: movzx  eax, word ptr [rbx + 0x2c4]
020dcb61: test   word ptr [rdi + 0x80], ax
020dcb68: jne    0x1820dcc18
020dcb6e: cmp    byte ptr [rbx + 0x2f1], 0
020dcb75: je     0x1820dcb92
020dcb77: xor    eax, eax
020dcb79: mov    dword ptr [rbx + 0x2ec], eax
020dcb7f: mov    dword ptr [rbx + 0x2c4], eax
020dcb85: mov    byte ptr [rbx + 0x2c8], al
020dcb8b: mov    word ptr [rbx + 0x2f0], ax
020dcb92: movzx  eax, word ptr [rbx + 0x2c4]
020dcb99: test   word ptr [rdi + 0x80], ax
020dcba0: jne    0x1820dcbf1
020dcba2: test   dword ptr [rdi + 0x7c], 0x8000
020dcba9: mov    byte ptr [rbx + 0x2f1], 0
020dcbb0: seta   al
020dcbb3: mov    byte ptr [rbx + 0x2c8], al
020dcbb9: movzx  eax, word ptr [rbx + 0x2c4]
020dcbc0: or     ax, word ptr [rdi + 0x80]
020dcbc7: mov    word ptr [rbx + 0x2c4], ax
020dcbce: movzx  eax, word ptr [rbx + 0x2c6]
020dcbd5: or     ax, word ptr [rdi + 0x80]
020dcbdc: mov    word ptr [rbx + 0x2c6], ax
020dcbe3: mov    rax, qword ptr [rbx + 0x100]
020dcbea: mov    qword ptr [rbx + 0x2e0], rax
020dcbf1: inc    dword ptr [rbx + 0x2e8]
020dcbf7: mov    rax, qword ptr [rbx]
020dcbfa: mov    rdx, rdi
020dcbfd: mov    rcx, rbx
020dcc00: mov    r8, qword ptr [rax + 0x1f0]
020dcc07: mov    rbx, qword ptr [rsp + 0x30]
020dcc0c: add    rsp, 0x20
020dcc10: pop    rdi
020dcc11: jmp    qword ptr [rax + 0x1e8]
020dcc18: movzx  eax, word ptr [rbx + 0x2c4]
020dcc1f: test   word ptr [rdi + 0x80], ax
020dcc26: je     0x1820dcc46
020dcc28: movsd  xmm1, qword ptr [rbx + 0x100]
020dcc30: subsd  xmm1, qword ptr [rbx + 0x2d0]
020dcc38: movsd  xmm0, qword ptr [rip + 0x1063a98]        ; [0x31406d8] dbl=0.075 q=0x3fb3333333333333
020dcc40: comisd xmm0, xmm1
020dcc44: ja     0x1820dcbf7
020dcc46: test   byte ptr [rdi + 0x80], 1
020dcc4d: jne    0x1820dcc6a
020dcc4f: xor    eax, eax
020dcc51: mov    dword ptr [rbx + 0x2ec], eax
020dcc57: mov    dword ptr [rbx + 0x2c4], eax
020dcc5d: mov    byte ptr [rbx + 0x2c8], al
020dcc63: mov    word ptr [rbx + 0x2f0], ax
020dcc6a: xor    r8d, r8d
020dcc6d: mov    rdx, rdi
020dcc70: mov    rcx, rbx
020dcc73: mov    rbx, qword ptr [rsp + 0x30]
020dcc78: add    rsp, 0x20
020dcc7c: pop    rdi
020dcc7d: jmp    0x1820f3610                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʳʶˀˁʶʶʽʽʷʼʼ
020dcc82: call   0x182f60c50
020dcc87: int3   
020dcc88: int3   
