001f0770: sub    rsp, 0x28
001f0774: test   rdx, rdx
001f0777: je     0x1801f07bf
001f0779: movzx  eax, byte ptr [rdx + 0x10]
001f077d: mov    byte ptr [rcx + 0x10], al
001f0780: movzx  eax, byte ptr [rdx + 0x11]
001f0784: mov    byte ptr [rcx + 0x38], al
001f0787: movzx  eax, byte ptr [rdx + 0x12]
001f078b: mov    byte ptr [rcx + 0x39], al
001f078e: movzx  eax, byte ptr [rdx + 0x13]
001f0792: mov    dword ptr [rcx + 0x14], eax
001f0795: movzx  eax, word ptr [rdx + 0x14]
001f0799: mov    dword ptr [rcx + 0x18], eax
001f079c: mov    eax, dword ptr [rdx + 0x20]
001f079f: mov    dword ptr [rcx + 0x24], eax
001f07a2: mov    rax, qword ptr [rdx + 0x18]
001f07a6: mov    qword ptr [rcx + 0x30], rax
001f07aa: mov    eax, dword ptr [rdx + 0x24]
001f07ad: mov    dword ptr [rcx + 0x1c], eax
001f07b0: mov    eax, dword ptr [rdx + 0x28]
001f07b3: mov    dword ptr [rcx + 0x20], eax
001f07b6: mov    byte ptr [rcx + 0x50], 0
001f07ba: add    rsp, 0x28
001f07be: ret    
001f07bf: call   0x182f60c50
001f07c4: int3   
001f07c5: int3   
