001f0830: movd   xmm0, dword ptr [rcx + 0x24]
001f0835: cvtdq2pd xmm0, xmm0
001f0839: divsd  xmm0, qword ptr [rip + 0x2e74487]        ; [0x3064cc8] dbl=1582.0 q=0x4098b80000000000
001f0841: ret    
001f0842: int3   
