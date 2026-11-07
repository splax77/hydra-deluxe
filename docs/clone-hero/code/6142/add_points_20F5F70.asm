020f5f70: cmp    dword ptr [rcx + 0x190], 1
020f5f77: mov    r9d, edx
020f5f7a: mov    r8, rcx
020f5f7d: je     0x1820f5fe2
020f5f7f: test   edx, edx
020f5f81: jle    0x1820f5fe2
020f5f83: mov    r10d, dword ptr [r8 + 0xc8]
020f5f8a: mov    eax, edx
020f5f8c: imul   eax, dword ptr [rcx + 0x48]
020f5f90: add    dword ptr [rcx + 0x94], eax
020f5f96: mov    ecx, dword ptr [rcx + 0xc4]
020f5f9c: mov    eax, dword ptr [r8 + 0x48]
020f5fa0: add    ecx, edx
020f5fa2: cmp    byte ptr [r8 + 0x42], 0
020f5fa7: mov    dword ptr [r8 + 0xc4], ecx
020f5fae: jne    0x1820f5fc1
020f5fb0: dec    eax
020f5fb2: imul   eax, r9d
020f5fb6: add    eax, r10d
020f5fb9: mov    dword ptr [r8 + 0xc8], eax
020f5fc0: ret    
020f5fc1: cdq    
020f5fc2: sub    eax, edx
020f5fc4: sar    eax, 1
020f5fc6: lea    ecx, [rax - 1]
020f5fc9: imul   eax, r9d
020f5fcd: imul   ecx, r9d
020f5fd1: add    ecx, r10d
020f5fd4: add    dword ptr [r8 + 0xcc], eax
020f5fdb: mov    dword ptr [r8 + 0xc8], ecx
020f5fe2: ret    
020f5fe3: int3   
