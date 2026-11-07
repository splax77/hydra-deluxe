020f52e0: cmp    dword ptr [rcx + 0x88], 0x1e
020f52e7: jg     0x1820f5306
020f52e9: mov    eax, 0x66666667
020f52ee: imul   dword ptr [rcx + 0x88]
020f52f4: sar    edx, 2
020f52f7: mov    r8d, edx
020f52fa: shr    r8d, 0x1f
020f52fe: inc    r8d
020f5301: add    r8d, edx
020f5304: jmp    0x1820f530c
020f5306: mov    r8d, 4
020f530c: mov    dword ptr [rcx + 0x48], r8d
020f5310: cmp    byte ptr [rcx + 0x42], 0
020f5314: je     0x1820f531d
020f5316: add    r8d, r8d
020f5319: mov    dword ptr [rcx + 0x48], r8d
020f531d: ret    
020f531e: int3   
