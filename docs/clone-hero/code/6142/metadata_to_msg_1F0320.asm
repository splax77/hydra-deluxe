001f0320: sub    rsp, 0x28
001f0324: test   rdx, rdx
001f0327: je     0x1801f036c
001f0329: movzx  eax, byte ptr [rcx + 0x10]
001f032d: mov    byte ptr [rdx + 0x10], al
001f0330: movzx  eax, byte ptr [rcx + 0x38]
001f0334: mov    byte ptr [rdx + 0x11], al
001f0337: movzx  eax, byte ptr [rcx + 0x39]
001f033b: mov    byte ptr [rdx + 0x12], al
001f033e: movzx  eax, byte ptr [rcx + 0x14]
001f0342: mov    byte ptr [rdx + 0x13], al
001f0345: movzx  eax, word ptr [rcx + 0x18]
001f0349: mov    word ptr [rdx + 0x14], ax
001f034d: mov    eax, dword ptr [rcx + 0x24]
001f0350: mov    dword ptr [rdx + 0x20], eax
001f0353: mov    rax, qword ptr [rcx + 0x30]
001f0357: mov    qword ptr [rdx + 0x18], rax
001f035b: mov    eax, dword ptr [rcx + 0x1c]
001f035e: mov    dword ptr [rdx + 0x24], eax
001f0361: mov    eax, dword ptr [rcx + 0x20]
001f0364: mov    dword ptr [rdx + 0x28], eax
001f0367: add    rsp, 0x28
001f036b: ret    
001f036c: call   0x182f60c50
001f0371: int3   
001f0372: int3   
