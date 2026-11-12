001d6b10: mov    qword ptr [rsp + 8], rcx
001d6b15: push   rdi
001d6b16: sub    rsp, 0x40
001d6b1a: mov    rdi, rdx
001d6b1d: mov    qword ptr [rcx + 0xf8], rdx
001d6b24: mov    rcx, qword ptr [rsp + 0x50]
001d6b29: add    rcx, 0xf8
001d6b30: call   0x182f5fc00
001d6b35: mov    qword ptr [rsp + 0x28], 0
001d6b3e: lea    rax, [rsp + 0x50]
001d6b43: mov    qword ptr [rsp + 0x30], rax
001d6b48: mov    rcx, qword ptr [rsp + 0x50]
001d6b4d: mov    rax, qword ptr [rcx]
001d6b50: mov    r8, qword ptr [rax + 0x350]
001d6b57: mov    rdx, rdi
001d6b5a: call   qword ptr [rax + 0x348]
001d6b60: nop    
001d6b61: mov    rax, qword ptr [rsp + 0x50]
001d6b66: mov    qword ptr [rax + 0xf8], 0
001d6b71: mov    rcx, qword ptr [rsp + 0x50]
001d6b76: add    rcx, 0xf8
001d6b7d: xor    edx, edx
001d6b7f: call   0x182f5fc00
001d6b84: jmp    0x1801d6bb3
001d6b86: mov    rax, qword ptr [rsp + 0x50]
001d6b8b: mov    qword ptr [rax + 0xf8], 0
001d6b96: mov    rcx, qword ptr [rsp + 0x50]
001d6b9b: add    rcx, 0xf8
001d6ba2: xor    edx, edx
001d6ba4: call   0x182f5fc00
001d6ba9: mov    rcx, qword ptr [rsp + 0x28]
001d6bae: test   rcx, rcx
001d6bb1: jne    0x1801d6bb9
001d6bb3: add    rsp, 0x40
001d6bb7: pop    rdi
001d6bb8: ret    
001d6bb9: call   0x182f60ce0
001d6bbe: int3   
001d6bbf: int3   
