0213e0c0: mov    qword ptr [rsp + 0x10], rbx
0213e0c5: mov    qword ptr [rsp + 0x18], rbp
0213e0ca: push   rdi
0213e0cb: sub    rsp, 0x50
0213e0cf: movups xmm0, xmmword ptr [rcx]
0213e0d2: mov    rbp, r8
0213e0d5: lea    rcx, [rsp + 0x40]
0213e0da: mov    rbx, r9
0213e0dd: lea    r8, [r9 + 8]
0213e0e1: mov    rdi, rdx
0213e0e4: movaps xmmword ptr [rsp + 0x30], xmm0
0213e0e9: xor    r9d, r9d
0213e0ec: lea    rdx, [rsp + 0x30]
0213e0f1: call   0x18213db70                              ; ʷʹʸʲʶʳʾˁʾʷʶ$$ʸʾʺʵˁʳʵʵʺʸʹ
0213e0f6: cmp    byte ptr [rbx + 0xc], 0
0213e0fa: jne    0x18213e20f
0213e100: mov    ecx, dword ptr [rax]
0213e102: mov    qword ptr [rsp + 0x60], rsi
0213e107: mov    rsi, qword ptr [rax + 8]
0213e10b: cmp    ecx, 2
0213e10e: jne    0x18213e1d2
0213e114: test   rdi, rdi
0213e117: je     0x18213e2fe
0213e11d: cmp    byte ptr [rip + 0x1ddb1f8], 0            ; [0x3f1931c] (bss)
0213e124: jne    0x18213e145
0213e126: lea    rcx, [rip + 0x1b915d3]                   ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
0213e12d: call   0x182f609b0
0213e132: lea    rcx, [rip + 0x1bbae8f]                   ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213e139: call   0x182f609b0
0213e13e: mov    byte ptr [rip + 0x1ddb1d7], 1            ; [0x3f1931c] (bss)
0213e145: mov    rcx, qword ptr [rdi + 0x48]
0213e149: test   rcx, rcx
0213e14c: je     0x18213e2fe
0213e152: mov    rdx, qword ptr [rip + 0x1bbae6f]         ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213e159: mov    rbx, qword ptr [rdi + 0x70]
0213e15d: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0213e162: test   rax, rax
0213e165: je     0x18213e2fe
0213e16b: xor    r9d, r9d
0213e16e: mov    r8, rsi
0213e171: mov    rdx, rbp
0213e174: mov    rcx, rax
0213e177: call   0x1820d5e60                              ; ʸʵʵʾʿˀʺʽʲʾˁ$$ʹʳʾˁʺʾʹˁʻʹʲ
0213e17c: mov    r9, rax
0213e17f: test   rbx, rbx
0213e182: je     0x18213e2fe
0213e188: mov    rax, qword ptr [rip + 0x1b91571]         ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
0213e18f: inc    dword ptr [rbx + 0x1c]
0213e192: mov    rcx, qword ptr [rbx + 0x10]
0213e196: test   rcx, rcx
0213e199: je     0x18213e2fe
0213e19f: movsxd rdx, dword ptr [rbx + 0x18]
0213e1a3: cmp    edx, dword ptr [rcx + 0x18]
0213e1a6: jae    0x18213e2cf
0213e1ac: lea    eax, [rdx + 1]
0213e1af: mov    dword ptr [rbx + 0x18], eax
0213e1b2: cmp    edx, dword ptr [rcx + 0x18]
0213e1b5: jae    0x18213e304
0213e1bb: mov    qword ptr [rcx + rdx*8 + 0x20], r9
0213e1c0: lea    rcx, [rcx + rdx*8]
0213e1c4: add    rcx, 0x20
0213e1c8: mov    rdx, r9
0213e1cb: call   0x182f5fc00
0213e1d0: jmp    0x18213e20a
0213e1d2: add    ecx, -0x40
0213e1d5: je     0x18213e23b
0213e1d7: sub    ecx, 1
0213e1da: je     0x18213e21f
0213e1dc: cmp    ecx, 1
0213e1df: jne    0x18213e20a
0213e1e1: test   rdi, rdi
0213e1e4: je     0x18213e2fe
0213e1ea: mov    qword ptr [rsp + 0x28], 0
0213e1f3: movzx  r9d, cl
0213e1f7: mov    byte ptr [rsp + 0x20], 0xff
0213e1fc: mov    r8, rsi
0213e1ff: mov    rdx, rbp
0213e202: mov    rcx, rdi
0213e205: call   0x1820ceec0
0213e20a: mov    rsi, qword ptr [rsp + 0x60]
0213e20f: mov    rbx, qword ptr [rsp + 0x68]
0213e214: mov    rbp, qword ptr [rsp + 0x70]
0213e219: add    rsp, 0x50
0213e21d: pop    rdi
0213e21e: ret    
0213e21f: test   rdi, rdi
0213e222: je     0x18213e2fe
0213e228: mov    qword ptr [rsp + 0x28], 0
0213e231: xor    r9d, r9d
0213e234: mov    byte ptr [rsp + 0x20], 0xff
0213e239: jmp    0x18213e1fc
0213e23b: test   rdi, rdi
0213e23e: je     0x18213e2fe
0213e244: cmp    byte ptr [rip + 0x1ddb0d7], 0            ; [0x3f19322] (bss)
0213e24b: jne    0x18213e26c
0213e24d: lea    rcx, [rip + 0x1b8ef34]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0213e254: call   0x182f609b0
0213e259: lea    rcx, [rip + 0x1bb9e00]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213e260: call   0x182f609b0
0213e265: mov    byte ptr [rip + 0x1ddb0b6], 1            ; [0x3f19322] (bss)
0213e26c: mov    rcx, qword ptr [rdi + 0x30]
0213e270: test   rcx, rcx
0213e273: je     0x18213e2fe
0213e279: mov    rdx, qword ptr [rip + 0x1bb9de0]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0213e280: mov    rbx, qword ptr [rdi + 0x88]
0213e287: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0213e28c: test   rax, rax
0213e28f: je     0x18213e2fe
0213e291: xor    r9d, r9d
0213e294: mov    qword ptr [rsp + 0x20], 0
0213e29d: mov    r8, rsi
0213e2a0: mov    rdx, rbp
0213e2a3: mov    rcx, rax
0213e2a6: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
0213e2ab: mov    r9, rax
0213e2ae: test   rbx, rbx
0213e2b1: je     0x18213e2fe
0213e2b3: mov    rax, qword ptr [rip + 0x1b8eece]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0213e2ba: inc    dword ptr [rbx + 0x1c]
0213e2bd: mov    rcx, qword ptr [rbx + 0x10]
0213e2c1: test   rcx, rcx
0213e2c4: je     0x18213e2fe
0213e2c6: movsxd rdx, dword ptr [rbx + 0x18]
0213e2ca: cmp    edx, dword ptr [rcx + 0x18]
0213e2cd: jb     0x18213e2ee
0213e2cf: mov    rax, qword ptr [rax + 0x20]
0213e2d3: mov    rdx, r9
0213e2d6: mov    rcx, rbx
0213e2d9: mov    r8, qword ptr [rax + 0xc0]
0213e2e0: mov    r8, qword ptr [r8 + 0x70]
0213e2e4: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0213e2e9: jmp    0x18213e20a
0213e2ee: lea    eax, [rdx + 1]
0213e2f1: mov    dword ptr [rbx + 0x18], eax
0213e2f4: cmp    edx, dword ptr [rcx + 0x18]
0213e2f7: jae    0x18213e304
0213e2f9: jmp    0x18213e1bb
0213e2fe: call   0x182f60c50
0213e303: int3   
0213e304: call   0x182f60c40
0213e309: int3   
0213e30a: int3   
