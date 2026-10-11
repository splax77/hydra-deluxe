020de5b0: push   rbx
020de5b2: sub    rsp, 0x20
020de5b6: movsd  xmm0, qword ptr [rcx + 0x100]
020de5be: mov    rbx, rcx
020de5c1: subsd  xmm0, qword ptr [rcx + 0x2d8]
020de5c9: comisd xmm0, xmmword ptr [rip + 0xf859df]       ; [0x3063fb0] xmm dbl=(0.5, 14.40000342991617) flt=(0.0, 1.75, 1.7777777910232544, 2.700000047683716)
020de5d1: jbe    0x1820de605
020de5d3: cmp    word ptr [rcx + 0x2c4], 0
020de5db: je     0x1820de605
020de5dd: cmp    byte ptr [rcx + 0x2f1], 0
020de5e4: je     0x1820de605
020de5e6: xor    eax, eax
020de5e8: mov    dword ptr [rcx + 0x2ec], 0
020de5f2: mov    dword ptr [rcx + 0x2c4], eax
020de5f8: mov    byte ptr [rcx + 0x2c8], al
020de5fe: mov    word ptr [rcx + 0x2f0], ax
020de605: movsd  xmm0, qword ptr [rcx + 0x100]
020de60d: subsd  xmm0, qword ptr [rcx + 0x2e0]
020de615: comisd xmm0, xmmword ptr [rip + 0x10620cb]      ; [0x31406e8] xmm dbl=(0.75, 12.0) flt=(0.0, 1.8125, 0.0, 2.625)
020de61d: jbe    0x1820de66f
020de61f: cmp    word ptr [rcx + 0x2c4], 0
020de627: je     0x1820de66f
020de629: cmp    byte ptr [rcx + 0x2c8], 0
020de630: je     0x1820de66f
020de632: cmp    byte ptr [rcx + 0x2f1], 0
020de639: jne    0x1820de66f
020de63b: mov    rax, qword ptr [rcx]
020de63e: xor    r8d, r8d
020de641: mov    dl, 1
020de643: mov    r9, qword ptr [rax + 0x280]
020de64a: call   qword ptr [rax + 0x278]
020de650: xor    eax, eax
020de652: mov    dword ptr [rbx + 0x2ec], 0
020de65c: mov    dword ptr [rbx + 0x2c4], eax
020de662: mov    byte ptr [rbx + 0x2c8], al
020de668: mov    word ptr [rbx + 0x2f0], ax
020de66f: add    rsp, 0x20
020de673: pop    rbx
020de674: ret    
020de675: int3   
