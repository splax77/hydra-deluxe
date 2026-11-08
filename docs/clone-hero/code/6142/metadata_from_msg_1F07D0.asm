001f07d0: sub    rsp, 0x28
001f07d4: test   rdx, rdx
001f07d7: je     0x1801f081f
001f07d9: movzx  eax, byte ptr [rdx + 0x10]
001f07dd: mov    byte ptr [rcx + 0x10], al
001f07e0: movzx  eax, byte ptr [rdx + 0x11]
001f07e4: mov    byte ptr [rcx + 0x38], al
001f07e7: movzx  eax, byte ptr [rdx + 0x12]
001f07eb: mov    byte ptr [rcx + 0x39], al
001f07ee: movzx  eax, byte ptr [rdx + 0x13]
001f07f2: mov    dword ptr [rcx + 0x14], eax
001f07f5: movzx  eax, word ptr [rdx + 0x14]
001f07f9: mov    dword ptr [rcx + 0x18], eax
001f07fc: mov    eax, dword ptr [rdx + 0x20]
001f07ff: mov    dword ptr [rcx + 0x24], eax
001f0802: mov    rax, qword ptr [rdx + 0x18]
001f0806: mov    qword ptr [rcx + 0x30], rax
001f080a: mov    eax, dword ptr [rdx + 0x24]
001f080d: mov    dword ptr [rcx + 0x1c], eax
001f0810: mov    eax, dword ptr [rdx + 0x28]
001f0813: mov    dword ptr [rcx + 0x20], eax
001f0816: mov    byte ptr [rcx + 0x50], 1
001f081a: add    rsp, 0x28
001f081e: ret    
001f081f: call   0x182f60c50
001f0824: int3   
001f0825: int3   
