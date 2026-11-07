020f63b0: movsd  xmm0, qword ptr [rcx + 0x100]
020f63b8: comisd xmm0, xmmword ptr [rcx + 0x120]
020f63c0: jb     0x1820f63d9
020f63c2: cmp    byte ptr [rcx + 0x42], 0
020f63c6: je     0x1820f63d9
020f63c8: mov    rax, qword ptr [rcx]
020f63cb: mov    rdx, qword ptr [rax + 0x2b0]
020f63d2: jmp    qword ptr [rax + 0x2a8]
020f63d9: ret    
020f63da: int3   
