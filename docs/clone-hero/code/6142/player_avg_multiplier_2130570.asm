02130570: cmp    dword ptr [rcx + 0x14], 0
02130574: jle    0x18213058b
02130576: movd   xmm0, dword ptr [rcx + 0x14]
0213057b: movd   xmm1, dword ptr [rcx + 0x10]
02130580: cvtdq2ps xmm0, xmm0
02130583: cvtdq2ps xmm1, xmm1
02130586: divss  xmm0, xmm1
0213058a: ret    
0213058b: xorps  xmm0, xmm0
0213058e: ret    
0213058f: int3   
