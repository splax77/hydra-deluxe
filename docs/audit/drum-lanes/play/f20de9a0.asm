020de9a0: push   rbx
020de9a2: sub    rsp, 0x20
020de9a6: cmp    byte ptr [rcx + 0xf0], 0
020de9ad: mov    rbx, rcx
020de9b0: jne    0x1820dea91
020de9b6: movzx  eax, word ptr [rcx + 0x2c4]
020de9bd: and    ax, dx
020de9c0: cmp    ax, dx
020de9c3: je     0x1820dea5a
020de9c9: cmp    word ptr [rcx + 0x2c4], 0
020de9d1: je     0x1820de9ff
020de9d3: cmp    byte ptr [rcx + 0x2c8], 0
020de9da: je     0x1820de9ff
020de9dc: cmp    byte ptr [rcx + 0x2f0], 0
020de9e3: jne    0x1820de9ff
020de9e5: test   dl, 1
020de9e8: je     0x1820de9ff
020de9ea: xor    eax, eax
020de9ec: mov    byte ptr [rcx + 0x2f0], 1
020de9f3: mov    dword ptr [rcx + 0x2e8], eax
020de9f9: add    rsp, 0x20
020de9fd: pop    rbx
020de9fe: ret    
020de9ff: cmp    byte ptr [rcx + 0x308], 0
020dea06: je     0x1820dea1e
020dea08: movzx  eax, byte ptr [rcx + 0x152]
020dea0f: and    al, 0x82
020dea11: cmp    al, 0x82
020dea13: jne    0x1820dea1e
020dea15: test   dx, dx
020dea18: je     0x1820dea1e
020dea1a: test   al, dl
020dea1c: jne    0x1820dea91
020dea1e: xor    eax, eax
020dea20: xor    r8d, r8d
020dea23: mov    dword ptr [rcx + 0x2ec], eax
020dea29: mov    dl, 1
020dea2b: mov    dword ptr [rcx + 0x2c4], eax
020dea31: mov    byte ptr [rcx + 0x2c8], al
020dea37: mov    word ptr [rcx + 0x2f0], ax
020dea3e: mov    rax, qword ptr [rcx]
020dea41: mov    r9, qword ptr [rax + 0x280]
020dea48: call   qword ptr [rax + 0x278]
020dea4e: inc    dword ptr [rbx + 0xb0]
020dea54: add    rsp, 0x20
020dea58: pop    rbx
020dea59: ret    
020dea5a: mov    rax, qword ptr [rcx + 0x100]
020dea61: not    dx
020dea64: and    word ptr [rcx + 0x2c6], dx
020dea6b: mov    qword ptr [rcx + 0x2d0], rax
020dea72: jne    0x1820dea89
020dea74: mov    qword ptr [rcx + 0x2e0], rax
020dea7b: movzx  eax, word ptr [rcx + 0x2c4]
020dea82: mov    word ptr [rcx + 0x2c6], ax
020dea89: xor    eax, eax
020dea8b: mov    dword ptr [rcx + 0x2e8], eax
020dea91: add    rsp, 0x20
020dea95: pop    rbx
020dea96: ret    
020dea97: int3   
