0210d990: test   ecx, ecx
0210d992: je     0x18210d9af
0210d994: sub    ecx, 1
0210d997: je     0x18210d9ac
0210d999: sub    ecx, 1
0210d99c: je     0x18210d9a9
0210d99e: cmp    ecx, 1
0210d9a1: je     0x18210d9a6
0210d9a3: mov    al, 0xff
0210d9a5: ret    
0210d9a6: mov    al, 3
0210d9a8: ret    
0210d9a9: mov    al, 2
0210d9ab: ret    
0210d9ac: mov    al, 1
0210d9ae: ret    
0210d9af: xor    al, al
0210d9b1: ret    
0210d9b2: int3   
