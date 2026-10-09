0215c040: push   rbx
0215c042: push   rbp
0215c043: push   rsi
0215c044: push   rdi
0215c045: push   r13
0215c047: push   r14
0215c049: sub    rsp, 0x48
0215c04d: cmp    byte ptr [rip + 0x1dbd6db], 0            ; [0x3f1972f] (bss)
0215c054: mov    r13, rcx
0215c057: jne    0x18215c0a8
0215c059: lea    rcx, [rip + 0x1b7b6c0]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215c060: call   0x182f609b0
0215c065: lea    rcx, [rip + 0x1b7b774]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c06c: call   0x182f609b0
0215c071: lea    rcx, [rip + 0x1b4f810]                   ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c078: call   0x182f609b0
0215c07d: lea    rcx, [rip + 0x1b4f74c]                   ; [0x3cab7d0] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c084: call   0x182f609b0
0215c089: lea    rcx, [rip + 0x1b4f8b0]                   ; [0x3cab940] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Length()
0215c090: call   0x182f609b0
0215c095: lea    rcx, [rip + 0x1b95ad4]                   ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215c09c: call   0x182f609b0
0215c0a1: mov    byte ptr [rip + 0x1dbd687], 1            ; [0x3f1972f] (bss)
0215c0a8: mov    rbx, qword ptr [rip + 0x1b95ac1]         ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215c0af: mov    rdi, qword ptr [r13 + 0x60]
0215c0b3: cmp    qword ptr [rbx + 0x38], 0
0215c0b8: jne    0x18215c0c2
0215c0ba: mov    rcx, rbx
0215c0bd: call   0x182f657d0
0215c0c2: mov    r8, qword ptr [rbx + 0x38]
0215c0c6: lea    rcx, [rsp + 0x20]
0215c0cb: mov    rdx, rdi
0215c0ce: mov    qword ptr [rsp + 0x80], r15
0215c0d6: mov    r8, qword ptr [r8 + 8]
0215c0da: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0215c0df: mov    rax, qword ptr [r13 + 0x60]
0215c0e3: xor    ecx, ecx
0215c0e5: mov    ebp, dword ptr [rsp + 0x28]
0215c0e9: xor    esi, esi
0215c0eb: xor    r14d, r14d
0215c0ee: test   rax, rax
0215c0f1: je     0x18215c30e
0215c0f7: mov    rbx, qword ptr [rsp + 0x20]
0215c0fc: nop    dword ptr [rax]
0215c100: cmp    ecx, dword ptr [rax + 0x18]
0215c103: jge    0x18215c1e0
0215c109: mov    rcx, qword ptr [r13 + 0x60]
0215c10d: test   rcx, rcx
0215c110: je     0x18215c30e
0215c116: mov    r8, qword ptr [rip + 0x1b7b6c3]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c11d: mov    edx, r14d
0215c120: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c125: mov    rcx, qword ptr [r13 + 0x60]
0215c129: mov    rdi, rax
0215c12c: test   rcx, rcx
0215c12f: je     0x18215c30e
0215c135: mov    r8, qword ptr [rip + 0x1b7b6a4]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215c13c: mov    edx, esi
0215c13e: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215c143: test   rax, rax
0215c146: je     0x18215c30e
0215c14c: test   rdi, rdi
0215c14f: je     0x18215c30e
0215c155: mov    rcx, qword ptr [rdi + 0x10]
0215c159: cmp    qword ptr [rax + 0x10], rcx
0215c15d: je     0x18215c1c8
0215c15f: mov    r15, qword ptr [rip + 0x1b4f722]         ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c166: mov    edi, r14d
0215c169: sub    edi, esi
0215c16b: cmp    esi, ebp
0215c16d: ja     0x18215c177
0215c16f: mov    eax, ebp
0215c171: sub    eax, esi
0215c173: cmp    edi, eax
0215c175: jbe    0x18215c17e
0215c177: xor    ecx, ecx
0215c179: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c17e: mov    rcx, qword ptr [r15 + 0x20]
0215c182: xorps  xmm0, xmm0
0215c185: movups xmmword ptr [rsp + 0x30], xmm0
0215c18a: test   byte ptr [rcx + 0x135], 1
0215c191: jne    0x18215c198
0215c193: call   0x182f65750
0215c198: cmp    edi, 1
0215c19b: jle    0x18215c1c5
0215c19d: movsxd rax, esi
0215c1a0: lea    rdx, [rsp + 0x20]
0215c1a5: xor    r8d, r8d
0215c1a8: mov    dword ptr [rsp + 0x28], edi
0215c1ac: lea    rcx, [rbx + rax*8]
0215c1b0: mov    eax, dword ptr [rsp + 0x3c]
0215c1b4: mov    qword ptr [rsp + 0x20], rcx
0215c1b9: mov    rcx, r13
0215c1bc: mov    dword ptr [rsp + 0x2c], eax
0215c1c0: call   0x18215e020                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʷʻʻʷʲʺˀʷˁʴʹ
0215c1c5: mov    esi, r14d
0215c1c8: mov    rax, qword ptr [r13 + 0x60]
0215c1cc: inc    r14d
0215c1cf: mov    ecx, r14d
0215c1d2: test   rax, rax
0215c1d5: je     0x18215c30e
0215c1db: jmp    0x18215c100
0215c1e0: mov    rdi, qword ptr [rip + 0x1b4f5e9]         ; [0x3cab7d0] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215c1e7: cmp    esi, ebp
0215c1e9: jbe    0x18215c1f2
0215c1eb: xor    ecx, ecx
0215c1ed: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215c1f2: mov    rcx, qword ptr [rdi + 0x20]
0215c1f6: test   byte ptr [rcx + 0x135], 1
0215c1fd: jne    0x18215c204
0215c1ff: call   0x182f65750
0215c204: sub    ebp, esi
0215c206: cmp    ebp, 1
0215c209: jle    0x18215c2ed
0215c20f: cmp    byte ptr [rip + 0x1dbd52a], 0            ; [0x3f19740] (bss)
0215c216: movsxd rax, esi
0215c219: lea    r15, [rbx + rax*8]
0215c21d: jne    0x18215c232
0215c21f: lea    rcx, [rip + 0x1b4f71a]                   ; [0x3cab940] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Length()
0215c226: call   0x182f609b0
0215c22b: mov    byte ptr [rip + 0x1dbd50e], 1            ; [0x3f19740] (bss)
0215c232: mov    r14d, ebp
0215c235: xor    r10b, r10b
0215c238: mov    ebx, 1
0215c23d: cmp    r14d, ebx
0215c240: jle    0x18215c2ed
0215c246: nop    word ptr [rax + rax]
0215c250: lea    eax, [rbx - 1]
0215c253: cmp    eax, ebp
0215c255: jae    0x18215c314
0215c25b: cmp    ebx, ebp
0215c25d: jae    0x18215c314
0215c263: mov    eax, ebx
0215c265: mov    rcx, qword ptr [r15 + rax*8 - 8]
0215c26a: lea    r9, [r15 + rax*8]
0215c26e: test   rcx, rcx
0215c271: je     0x18215c30e
0215c277: mov    eax, ebx
0215c279: lea    rsi, [r15 + rax*8]
0215c27d: mov    rax, qword ptr [r15 + rax*8]
0215c281: test   rax, rax
0215c284: je     0x18215c30e
0215c28a: mov    eax, dword ptr [rax + 0x20]
0215c28d: cmp    dword ptr [rcx + 0x20], eax
0215c290: jne    0x18215c2a0
0215c292: mov    rax, qword ptr [rsi]
0215c295: mov    rdx, rcx
0215c298: mov    ecx, dword ptr [rax + 0x24]
0215c29b: cmp    dword ptr [rdx + 0x24], ecx
0215c29e: jg     0x18215c2b0
0215c2a0: mov    rax, qword ptr [rsi]
0215c2a3: mov    r8, qword ptr [r9 - 8]
0215c2a7: mov    edx, dword ptr [rax + 0x20]
0215c2aa: cmp    dword ptr [r8 + 0x20], edx
0215c2ae: jle    0x18215c2d5
0215c2b0: mov    rdx, qword ptr [rsi]
0215c2b3: lea    rcx, [r9 - 8]
0215c2b7: mov    rdi, qword ptr [r9 - 8]
0215c2bb: mov    qword ptr [r9 - 8], rdx
0215c2bf: call   0x182f5fc00
0215c2c4: mov    rdx, rdi
0215c2c7: mov    qword ptr [rsi], rdi
0215c2ca: mov    rcx, rsi
0215c2cd: call   0x182f5fc00
0215c2d2: mov    r10b, 1
0215c2d5: inc    ebx
0215c2d7: cmp    ebx, r14d
0215c2da: jl     0x18215c250
0215c2e0: test   r10b, r10b
0215c2e3: je     0x18215c2ed
0215c2e5: dec    r14d
0215c2e8: jmp    0x18215c235
0215c2ed: xor    r8d, r8d
0215c2f0: xor    edx, edx
0215c2f2: mov    rcx, r13
0215c2f5: mov    r15, qword ptr [rsp + 0x80]
0215c2fd: add    rsp, 0x48
0215c301: pop    r14
0215c303: pop    r13
0215c305: pop    rdi
0215c306: pop    rsi
0215c307: pop    rbp
0215c308: pop    rbx
0215c309: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
0215c30e: call   0x182f60c50
0215c313: int3   
0215c314: call   0x182f60c40
0215c319: int3   
0215c31a: int3   
