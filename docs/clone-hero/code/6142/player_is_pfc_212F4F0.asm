0212f4f0: mov    eax, dword ptr [rcx + 0x44]
0212f4f3: cmp    dword ptr [rcx + 0x20], eax
0212f4f6: jne    0x18212f509
0212f4f8: cmp    dword ptr [rcx + 0x20], 0
0212f4fc: jle    0x18212f509
0212f4fe: cmp    byte ptr [rcx + 0xa8], 0
0212f505: sete   al
0212f508: ret    
0212f509: xor    al, al
0212f50b: ret    
0212f50c: int3   
