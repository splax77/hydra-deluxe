=== 214d2c0
0214d2c0: mov    eax, dword ptr [rcx + 0x24]
0214d2c3: shr    eax, 3
0214d2c6: and    al, 1
0214d2c8: ret    
0214d2c9: int3   
=== 214dca0
0214dca0: mov    eax, dword ptr [rcx + 0x24]
0214dca3: shr    eax, 5
0214dca6: and    al, 1
0214dca8: ret    
0214dca9: int3   
=== 214d580
0214d580: mov    eax, dword ptr [rcx + 0x30]
0214d583: cmp    dword ptr [rcx + 0x2c], eax
0214d586: setne  al
0214d589: ret    
0214d58a: int3   
=== 214dc90
0214dc90: mov    eax, dword ptr [rcx + 0x24]
0214dc93: shr    eax, 0xa
0214dc96: and    al, 1
0214dc98: ret    
0214dc99: int3   
=== 214c8b0
0214c8b0: mov    eax, dword ptr [rcx + 0x24]
0214c8b3: shr    eax, 9
0214c8b6: and    al, 1
0214c8b8: ret    
0214c8b9: int3   
=== 214c5d0
0214c5d0: mov    eax, dword ptr [rcx + 0x24]
0214c5d3: shr    eax, 4
0214c5d6: and    al, 1
0214c5d8: ret    
0214c5d9: int3   
=== 214cdc0
0214cdc0: mov    eax, dword ptr [rcx + 0x24]
0214cdc3: shr    eax, 7
0214cdc6: and    al, 1
0214cdc8: ret    
0214cdc9: int3   
=== 214c880
0214c880: mov    eax, dword ptr [rcx + 0x24]
0214c883: shr    eax, 6
0214c886: and    al, 1
0214c888: ret    
0214c889: int3   
=== 214cdb0
0214cdb0: mov    eax, dword ptr [rcx + 0x24]
0214cdb3: shr    eax, 8
0214cdb6: and    al, 1
0214cdb8: ret    
0214cdb9: int3   
=== 20e7880
020e7880: test   byte ptr [rcx + 0x24], 2
020e7884: seta   al
020e7887: ret    
020e7888: int3   
=== 20e7900
020e7900: movzx  eax, byte ptr [rcx + 0x24]
020e7904: and    al, 1
020e7906: ret    
020e7907: int3   
=== 20e78f0
020e78f0: test   byte ptr [rcx + 0x24], 4
020e78f4: seta   al
020e78f7: ret    
020e78f8: int3   
