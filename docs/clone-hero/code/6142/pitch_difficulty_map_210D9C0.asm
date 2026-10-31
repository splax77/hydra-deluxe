0210d9c0: lea    eax, [rcx - 0x3a]
0210d9c3: cmp    eax, 8
0210d9c6: jbe    0x18210d9f7
0210d9c8: lea    eax, [rcx - 0x46]
0210d9cb: cmp    eax, 8
0210d9ce: jbe    0x18210d9f4
0210d9d0: lea    eax, [rcx - 0x52]
0210d9d3: cmp    eax, 8
0210d9d6: jbe    0x18210d9f1
0210d9d8: cmp    ecx, 0x5e
0210d9db: jl     0x18210d9ee
0210d9dd: cmp    ecx, 0x66
0210d9e0: mov    eax, 3
0210d9e5: mov    edx, 0xffffffff
0210d9ea: cmovg  eax, edx
0210d9ed: ret    
0210d9ee: mov    al, 0xff
0210d9f0: ret    
0210d9f1: mov    al, 2
0210d9f3: ret    
0210d9f4: mov    al, 1
0210d9f6: ret    
0210d9f7: xor    al, al
0210d9f9: ret    
0210d9fa: int3   
