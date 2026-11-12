020d4a70: sub    rsp, 0x28
020d4a74: test   rdx, rdx
020d4a77: je     0x1820d4a94
020d4a79: mulsd  xmm0, qword ptr [rip + 0xf9114f]         ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
020d4a81: mulsd  xmm2, qword ptr [rdx + 0x30]
020d4a86: mulsd  xmm0, qword ptr [rdx + 0x40]
020d4a8b: divsd  xmm0, xmm2
020d4a8f: add    rsp, 0x28
020d4a93: ret    
020d4a94: call   0x182f60c50
020d4a99: int3   
020d4a9a: int3   
