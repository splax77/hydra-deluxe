020ddec0: push   rbx
020ddec2: sub    rsp, 0x20
020ddec6: mov    rbx, rcx
020ddec9: mov    qword ptr [rsp + 0x30], 0
020dded2: lea    rcx, [rsp + 0x30]
020dded7: xor    r8d, r8d
020ddeda: mov    dl, 0xff
020ddedc: call   0x1821770a0                              ; ʾʿʴʹʻʹʸʳʼʲʵ$$.ctor
020ddee1: mov    rax, qword ptr [rsp + 0x30]
020ddee6: xor    edx, edx
020ddee8: mov    qword ptr [rbx + 0x309], rax
020ddeef: mov    rcx, rbx
020ddef2: xor    eax, eax
020ddef4: mov    word ptr [rbx + 0x152], ax
020ddefb: mov    word ptr [rbx + 0x2f4], ax
020ddf02: call   0x1820f5540                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʻʿʻʲʴʷˀʿʽʿ
020ddf07: movsd  xmm0, qword ptr [rbx + 0x100]
020ddf0f: subsd  xmm0, qword ptr [rbx + 0x2d8]
020ddf17: comisd xmm0, xmmword ptr [rip + 0xf86091]       ; [0x3063fb0] xmm dbl=(0.5, 14.40000342991617) flt=(0.0, 1.75, 1.7777777910232544, 2.700000047683716)
020ddf1f: jbe    0x1820ddf53
020ddf21: cmp    word ptr [rbx + 0x2c4], 0
020ddf29: je     0x1820ddf53
020ddf2b: cmp    byte ptr [rbx + 0x2f1], 0
020ddf32: je     0x1820ddf53
020ddf34: xor    eax, eax
020ddf36: mov    dword ptr [rbx + 0x2ec], 0
020ddf40: mov    dword ptr [rbx + 0x2c4], eax
020ddf46: mov    byte ptr [rbx + 0x2c8], al
020ddf4c: mov    word ptr [rbx + 0x2f0], ax
020ddf53: movsd  xmm0, qword ptr [rbx + 0x100]
020ddf5b: subsd  xmm0, qword ptr [rbx + 0x2e0]
020ddf63: comisd xmm0, xmmword ptr [rip + 0x106277d]      ; [0x31406e8] xmm dbl=(0.75, 12.0) flt=(0.0, 1.8125, 0.0, 2.625)
020ddf6b: jbe    0x1820ddfc0
020ddf6d: cmp    word ptr [rbx + 0x2c4], 0
020ddf75: je     0x1820ddfc0
020ddf77: cmp    byte ptr [rbx + 0x2c8], 0
020ddf7e: je     0x1820ddfc0
020ddf80: cmp    byte ptr [rbx + 0x2f1], 0
020ddf87: jne    0x1820ddfc0
020ddf89: mov    rax, qword ptr [rbx]
020ddf8c: xor    r8d, r8d
020ddf8f: mov    dl, 1
020ddf91: mov    rcx, rbx
020ddf94: mov    r9, qword ptr [rax + 0x280]
020ddf9b: call   qword ptr [rax + 0x278]
020ddfa1: xor    eax, eax
020ddfa3: mov    dword ptr [rbx + 0x2ec], 0
020ddfad: mov    dword ptr [rbx + 0x2c4], eax
020ddfb3: mov    byte ptr [rbx + 0x2c8], al
020ddfb9: mov    word ptr [rbx + 0x2f0], ax
020ddfc0: add    rsp, 0x20
020ddfc4: pop    rbx
020ddfc5: ret    
020ddfc6: int3   
