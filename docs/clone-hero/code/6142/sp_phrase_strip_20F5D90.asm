020f5d90: mov    qword ptr [rsp + 8], rbx
020f5d95: push   rbp
020f5d96: lea    rbp, [rsp - 0x57]
020f5d9b: sub    rsp, 0xb0
020f5da2: cmp    byte ptr [rip + 0x1e2361c], 0            ; [0x3f193c5] (bss)
020f5da9: mov    rbx, rcx
020f5dac: jne    0x1820f5de5
020f5dae: lea    rcx, [rip + 0x1c008fb]                   ; [0x3cf66b0] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʶʼʵʲʸʳʻʴʴʹʹ()
020f5db5: call   0x182f609b0
020f5dba: lea    rcx, [rip + 0x1c009a7]                   ; [0x3cf6768] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʼˀʸʹʴʻʾʶʼʷˁ()
020f5dc1: call   0x182f609b0
020f5dc6: lea    rcx, [rip + 0x1c00a5b]                   ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020f5dcd: call   0x182f609b0
020f5dd2: lea    rcx, [rip + 0x1ba5337]                   ; [0x3c9b110] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ˁʴʲʿʽʻʸʼˀʹʴ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f5dd9: call   0x182f609b0
020f5dde: mov    byte ptr [rip + 0x1e235e0], 1            ; [0x3f193c5] (bss)
020f5de5: mov    r8d, dword ptr [rbx + 0x50]
020f5de9: lea    rcx, [rbp - 0x29]
020f5ded: mov    rdx, qword ptr [rbx + 0x1e0]
020f5df4: xor    eax, eax
020f5df6: xorps  xmm0, xmm0
020f5df9: mov    qword ptr [rbp - 1], rax
020f5dfd: xorps  xmm1, xmm1
020f5e00: mov    qword ptr [rbp + 0x2f], rax
020f5e04: mov    qword ptr [rbp + 0x17], rax
020f5e08: mov    r9d, 0x7fffffff
020f5e0e: mov    qword ptr [rbp + 0x47], rax
020f5e12: mov    rax, qword ptr [rip + 0x1ba52f7]         ; [0x3c9b110] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ˁʴʲʿʽʻʸʼˀʹʴ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020f5e19: mov    qword ptr [rsp + 0x20], rax
020f5e1e: movups xmmword ptr [rbp - 0x11], xmm0
020f5e22: movups xmmword ptr [rbp + 0x1f], xmm1
020f5e26: movups xmmword ptr [rbp + 7], xmm0
020f5e2a: movups xmmword ptr [rbp + 0x37], xmm1
020f5e2e: call   0x1805fbc10                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʿʶʻʾʺʴʴʼʹʸʶ<object>
020f5e33: movups xmm0, xmmword ptr [rbp - 0x29]
020f5e37: lea    rdx, [rbp + 0x1f]
020f5e3b: mov    r8, qword ptr [rip + 0x1c00926]          ; [0x3cf6768] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʼˀʸʹʴʻʾʶʼʷˁ()
020f5e42: movsd  xmm1, qword ptr [rbp - 0x19]
020f5e47: lea    rcx, [rbp - 0x29]
020f5e4b: movups xmmword ptr [rbp + 0x1f], xmm0
020f5e4f: movsd  qword ptr [rbp + 0x2f], xmm1
020f5e54: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020f5e59: movups xmm0, xmmword ptr [rbp - 0x29]
020f5e5d: lea    rcx, [rbp - 0x11]
020f5e61: mov    rdx, qword ptr [rip + 0x1c009c0]         ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020f5e68: movsd  xmm1, qword ptr [rbp - 0x19]
020f5e6d: movups xmmword ptr [rbp - 0x11], xmm0
020f5e71: movsd  qword ptr [rbp - 1], xmm1
020f5e76: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
020f5e7b: test   al, al
020f5e7d: je     0x1820f5ed3
020f5e7f: nop    
020f5e80: mov    rdx, qword ptr [rip + 0x1c00829]         ; [0x3cf66b0] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʶʼʵʲʸʳʻʴʴʹʹ()
020f5e87: lea    rcx, [rbp - 0x11]
020f5e8b: call   0x180fd06f0                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʽʼʵʶʳʹʻʵˁʳˀ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
020f5e90: mov    rbx, qword ptr [rax]
020f5e93: test   rbx, rbx
020f5e96: je     0x1820f5f64
020f5e9c: xor    edx, edx
020f5e9e: mov    rcx, rbx
020f5ea1: call   0x18214b5f0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʲˁʹʸʸʺʶʼʽʸʸ
020f5ea6: test   al, al
020f5ea8: jne    0x1820f5ee4
020f5eaa: xor    edx, edx
020f5eac: mov    rcx, rbx
020f5eaf: call   0x18214be20                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʶˀʼʼʾʿʽʺʻʵ
020f5eb4: test   al, al
020f5eb6: je     0x1820f5ebf
020f5eb8: and    dword ptr [rbx + 0x7c], 0xffffff7f
020f5ebf: mov    rdx, qword ptr [rip + 0x1c00962]         ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020f5ec6: lea    rcx, [rbp - 0x11]
020f5eca: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
020f5ecf: test   al, al
020f5ed1: jne    0x1820f5e80
020f5ed3: mov    rbx, qword ptr [rsp + 0xc0]
020f5edb: add    rsp, 0xb0
020f5ee2: pop    rbp
020f5ee3: ret    
020f5ee4: xor    r8d, r8d
020f5ee7: lea    rcx, [rbp - 0x29]
020f5eeb: mov    rdx, rbx
020f5eee: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020f5ef3: xor    r8d, r8d
020f5ef6: lea    rdx, [rbp + 0x37]
020f5efa: lea    rcx, [rbp - 0x29]
020f5efe: movups xmm0, xmmword ptr [rax]
020f5f01: movups xmmword ptr [rbp + 0x37], xmm0
020f5f05: movsd  xmm1, qword ptr [rax + 0x10]
020f5f0a: movsd  qword ptr [rbp + 0x47], xmm1
020f5f0f: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020f5f14: xor    edx, edx
020f5f16: lea    rcx, [rbp + 7]
020f5f1a: movups xmm0, xmmword ptr [rax]
020f5f1d: movups xmmword ptr [rbp + 7], xmm0
020f5f21: movsd  xmm1, qword ptr [rax + 0x10]
020f5f26: movsd  qword ptr [rbp + 0x17], xmm1
020f5f2b: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f5f30: test   al, al
020f5f32: je     0x1820f5ed3
020f5f34: mov    rcx, qword ptr [rbp + 0xf]
020f5f38: test   rcx, rcx
020f5f3b: je     0x1820f5f64
020f5f3d: and    dword ptr [rcx + 0x7c], 0xfffffe7f
020f5f44: xor    edx, edx
020f5f46: lea    rcx, [rbp + 7]
020f5f4a: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f5f4f: test   al, al
020f5f51: jne    0x1820f5f34
020f5f53: mov    rbx, qword ptr [rsp + 0xc0]
020f5f5b: add    rsp, 0xb0
020f5f62: pop    rbp
020f5f63: ret    
020f5f64: call   0x182f60c50
020f5f69: int3   
020f5f6a: int3   
