0214be30: mov    eax, 0xe0
0214be35: test   word ptr [rcx + 0x80], ax
0214be3c: seta   al
0214be3f: ret    
0214be40: test   byte ptr [rcx + 0x7c], 0x20
0214be44: seta   al
0214be47: ret    
0214be48: int3   
