020d1d20: sub    rsp, 0x28
020d1d24: test   rdx, rdx
020d1d27: je     0x1820d1d48
020d1d29: movsd  xmm1, qword ptr [rdx + 0x40]
020d1d2e: mulsd  xmm1, qword ptr [rip + 0xf93e9a]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
020d1d36: mulsd  xmm0, xmm2
020d1d3a: mulsd  xmm0, qword ptr [rdx + 0x30]
020d1d3f: divsd  xmm0, xmm1
020d1d43: add    rsp, 0x28
020d1d47: ret    
020d1d48: call   0x182f60c50
020d1d4d: int3   
020d1d4e: int3   
