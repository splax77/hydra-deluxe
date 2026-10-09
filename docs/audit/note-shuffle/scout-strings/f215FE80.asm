0215fe80: cmp    byte ptr [rcx + 0x58], 6
0215fe84: jne    0x18215fe89
0215fe86: mov    al, 1
0215fe88: ret    
0215fe89: cmp    byte ptr [rcx + 0x58], 9
0215fe8d: sete   al
0215fe90: ret    
0215fe91: int3   
