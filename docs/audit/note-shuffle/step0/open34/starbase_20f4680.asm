020f4680: mov    qword ptr [rsp + 0x10], rbx
020f4685: mov    qword ptr [rsp + 0x20], rsi
020f468a: movsd  qword ptr [rsp + 0x18], xmm2
020f4690: push   rdi
020f4691: push   r12
020f4693: push   r13
020f4695: push   r14
020f4697: push   r15
020f4699: sub    rsp, 0x80
020f46a0: movaps xmmword ptr [rsp + 0x70], xmm6
020f46a5: movaps xmm6, xmm2
020f46a8: movzx  r12d, dl
020f46ac: mov    rbx, rcx
020f46af: cmp    byte ptr [rip + 0x1e24cf4], 0            ; [0x3f193aa] (bss)
020f46b6: jne    0x1820f4707
020f46b8: lea    rcx, [rip + 0x1bcdbe9]                   ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f46bf: call   0x182f609b0
020f46c4: lea    rcx, [rip + 0x1bcdc9d]                   ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f46cb: call   0x182f609b0
020f46d0: lea    rcx, [rip + 0x1bcdd51]                   ; [0x3cc2428] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Current()
020f46d7: call   0x182f609b0
020f46dc: lea    rcx, [rip + 0x1be27fd]                   ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f46e3: call   0x182f609b0
020f46e8: lea    rcx, [rip + 0x1bae079]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f46ef: call   0x182f609b0
020f46f4: lea    rcx, [rip + 0x1ba414d]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f46fb: call   0x182f609b0
020f4700: mov    byte ptr [rip + 0x1e24ca3], 1            ; [0x3f193aa] (bss)
020f4707: xorps  xmm0, xmm0
020f470a: xor    eax, eax
020f470c: movups xmmword ptr [rsp + 0x50], xmm0
020f4711: mov    qword ptr [rsp + 0x60], rax
020f4716: mov    rcx, qword ptr [rip + 0x1bae04b]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f471d: cmp    dword ptr [rcx + 0xe0], eax
020f4723: jne    0x1820f472a
020f4725: call   0x182f60cf0
020f472a: movaps xmm0, xmm6
020f472d: divsd  xmm0, qword ptr [rip + 0xf702c3]         ; [0x30649f8] dbl=3.0 q=0x4008000000000000
020f4735: call   0x183031130
020f473a: mov    rdx, qword ptr [rbx + 0x1e0]
020f4741: test   rdx, rdx
020f4744: je     0x1820f4932
020f474a: xor    r15d, r15d
020f474d: mov    dword ptr [rsp + 0xb0], r15d
020f4755: xor    r14d, r14d
020f4758: mov    qword ptr [rsp + 0x20], r14
020f475d: mov    esi, 1
020f4762: xor    r13d, r13d
020f4765: cvttsd2si rax, xmm0
020f476a: mov    qword ptr [rsp + 0x28], rax
020f476f: mov    r8, qword ptr [rip + 0x1be276a]          ; [0x3cd6ee0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.GetEnumerator()
020f4776: lea    rcx, [rsp + 0x38]
020f477b: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020f4780: movups xmm0, xmmword ptr [rsp + 0x38]
020f4785: movups xmmword ptr [rsp + 0x50], xmm0
020f478a: movsd  xmm1, qword ptr [rsp + 0x48]
020f4790: movsd  qword ptr [rsp + 0x60], xmm1
020f4796: mov    qword ptr [rsp + 0x38], r13
020f479b: lea    rbx, [rsp + 0x50]
020f47a0: mov    qword ptr [rsp + 0x40], rbx
020f47a5: nop    word ptr [rax + rax]
020f47b0: mov    rdx, qword ptr [rip + 0x1bcdbb1]         ; [0x3cc2368] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.MoveNext()
020f47b7: lea    rcx, [rsp + 0x50]
020f47bc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020f47c1: test   al, al
020f47c3: je     0x1820f4899
020f47c9: mov    rdi, qword ptr [rsp + 0x60]
020f47ce: test   r12b, r12b
020f47d1: jne    0x1820f4801
020f47d3: test   rdi, rdi
020f47d6: je     0x1820f4938
020f47dc: mov    rax, qword ptr [rsp + 0x28]
020f47e1: cmp    qword ptr [rdi + 0x48], rax
020f47e5: jl     0x1820f47f5
020f47e7: mov    rax, qword ptr [rdi + 0x48]
020f47eb: add    r14, rax
020f47ee: mov    qword ptr [rsp + 0x20], r14
020f47f3: jmp    0x1820f4843
020f47f5: xor    eax, eax
020f47f7: add    r14, rax
020f47fa: mov    qword ptr [rsp + 0x20], r14
020f47ff: jmp    0x1820f4843
020f4801: test   rdi, rdi
020f4804: je     0x1820f493e
020f480a: xor    edx, edx
020f480c: mov    rcx, rdi
020f480f: call   0x18214be30                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʸʷʽʸʸʼʼʲʾʼ
020f4814: neg    al
020f4816: sbb    r13d, r13d
020f4819: and    r13d, 0xf
020f481d: xor    edx, edx
020f481f: mov    rcx, rdi
020f4822: call   0x18214be10                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʴˀʽʳʴʹʻʾʴʵ
020f4827: test   al, al
020f4829: jne    0x1820f483e
020f482b: xor    edx, edx
020f482d: mov    rcx, rdi
020f4830: call   0x18214b6c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʶʲʵʹʸʺʼʶʵʷ
020f4835: test   al, al
020f4837: mov    esi, 1
020f483c: je     0x1820f4843
020f483e: mov    esi, 2
020f4843: cmp    dword ptr [rdi + 0x50], -1
020f4847: jne    0x1820f487c
020f4849: movzx  edi, word ptr [rdi + 0x80]
020f4850: mov    rcx, qword ptr [rip + 0x1ba3ff1]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f4857: cmp    dword ptr [rcx + 0xe0], 0
020f485e: jne    0x1820f4865
020f4860: call   0x182f60cf0
020f4865: xor    ecx, ecx
020f4867: test   di, di
020f486a: je     0x1820f487f
020f486c: nop    dword ptr [rax]
020f4870: inc    ecx
020f4872: lea    eax, [rdi - 1]
020f4875: and    di, ax
020f4878: jne    0x1820f4870
020f487a: jmp    0x1820f487f
020f487c: mov    ecx, dword ptr [rdi + 0x50]
020f487f: lea    eax, [r13 + 0x32]
020f4883: imul   eax, ecx
020f4886: imul   eax, esi
020f4889: add    r15d, eax
020f488c: mov    dword ptr [rsp + 0xb0], r15d
020f4894: jmp    0x1820f47b0
020f4899: mov    rdx, qword ptr [rip + 0x1bcda08]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f48a0: mov    rcx, rbx
020f48a3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f48a8: jmp    0x1820f48db
020f48aa: mov    rdx, qword ptr [rip + 0x1bcd9f7]         ; [0x3cc22a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʽʿʸʸʾʶʶʾʶʹʲ>.Dispose()
020f48b1: mov    rcx, qword ptr [rsp + 0x40]
020f48b6: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020f48bb: mov    rcx, qword ptr [rsp + 0x38]
020f48c0: test   rcx, rcx
020f48c3: jne    0x1820f4944
020f48c5: movsd  xmm6, qword ptr [rsp + 0xc0]
020f48ce: mov    r15d, dword ptr [rsp + 0xb0]
020f48d6: mov    r14, qword ptr [rsp + 0x20]
020f48db: mov    rcx, qword ptr [rip + 0x1bade86]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f48e2: cmp    dword ptr [rcx + 0xe0], 0
020f48e9: jne    0x1820f48f0
020f48eb: call   0x182f60cf0
020f48f0: xorps  xmm0, xmm0
020f48f3: cvtsi2sd xmm0, r14
020f48f8: divsd  xmm6, qword ptr [rip + 0xf6f430]         ; [0x3063d30] dbl=25.0 q=0x4039000000000000
020f4900: divsd  xmm0, xmm6
020f4904: call   0x183031130
020f4909: cvttsd2si eax, xmm0
020f490d: add    eax, r15d
020f4910: lea    r11, [rsp + 0x80]
020f4918: mov    rbx, qword ptr [r11 + 0x38]
020f491c: mov    rsi, qword ptr [r11 + 0x48]
020f4920: movaps xmm6, xmmword ptr [rsp + 0x70]
020f4925: mov    rsp, r11
020f4928: pop    r15
020f492a: pop    r14
020f492c: pop    r13
020f492e: pop    r12
020f4930: pop    rdi
020f4931: ret    
020f4932: call   0x182f60c50
020f4937: nop    
020f4938: call   0x182f60c50
020f493d: nop    
020f493e: call   0x182f60c50
020f4943: nop    
020f4944: call   0x182f60ce0
020f4949: int3   
020f494a: int3   
