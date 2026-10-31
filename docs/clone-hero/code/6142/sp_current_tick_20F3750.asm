020f3750: push   rbx
020f3752: sub    rsp, 0x20
020f3756: xorps  xmm0, xmm0
020f3759: mov    rbx, rcx
020f375c: comisd xmm0, xmmword ptr [rcx + 0x100]
020f3764: jbe    0x1820f376e
020f3766: xor    eax, eax
020f3768: add    rsp, 0x20
020f376c: pop    rbx
020f376d: ret    
020f376e: mov    rax, qword ptr [rcx + 0x148]
020f3775: test   rax, rax
020f3778: je     0x1820f37c2
020f377a: mov    rcx, qword ptr [rcx + 0x230]
020f3781: test   rcx, rcx
020f3784: je     0x1820f37c2
020f3786: movsd  xmm0, qword ptr [rbx + 0x100]
020f378e: xor    r9d, r9d
020f3791: subsd  xmm0, qword ptr [rax + 0x18]
020f3796: movsd  xmm2, qword ptr [rcx + 0xb0]
020f379e: mov    rdx, rax
020f37a1: call   0x1820d1d20                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʷʾʻʿʵʻʸʵʺʹʺ
020f37a6: mov    rax, qword ptr [rbx + 0x148]
020f37ad: test   rax, rax
020f37b0: je     0x1820f37c2
020f37b2: addsd  xmm0, qword ptr [rax + 0x38]
020f37b7: cvttsd2si rax, xmm0
020f37bc: add    rsp, 0x20
020f37c0: pop    rbx
020f37c1: ret    
020f37c2: call   0x182f60c50
020f37c7: int3   
020f37c8: int3   
