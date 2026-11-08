005db320: mov    qword ptr [rsp + 0x18], rbx
005db325: push   rbp
005db326: push   r14
005db328: push   r15
005db32a: sub    rsp, 0x60
005db32e: cmp    qword ptr [r9 + 0x38], 0
005db333: mov    rbp, r9
005db336: mov    r15, r8
005db339: mov    r14, rdx
005db33c: jne    0x1805db346
005db33e: mov    rcx, r9
005db341: call   0x182f657d0
005db346: mov    rax, qword ptr [rbp + 0x38]
005db34a: mov    rbx, qword ptr [rax + 8]
005db34e: cmp    qword ptr [rbx + 0x38], 0
005db353: jne    0x1805db35d
005db355: mov    rcx, rbx
005db358: call   0x182f657d0
005db35d: mov    r8, qword ptr [rbx + 0x38]
005db361: lea    rcx, [rsp + 0x20]
005db366: mov    qword ptr [rsp + 0x80], rsi
005db36e: mov    rdx, r15
005db371: mov    qword ptr [rsp + 0x88], rdi
005db379: movaps xmmword ptr [rsp + 0x50], xmm6
005db37e: mov    r8, qword ptr [r8 + 8]
005db382: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
005db387: test   r14, r14
005db38a: je     0x1805db4ad
005db390: mov    esi, dword ptr [r14 + 0x18]
005db394: sub    esi, 1
005db397: movaps xmm6, xmmword ptr [rsp + 0x20]
005db39c: js     0x1805db486
005db3a2: mov    r8, qword ptr [rbp + 0x38]
005db3a6: mov    edx, esi
005db3a8: mov    rcx, r14
005db3ab: mov    r8, qword ptr [r8 + 0x28]
005db3af: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db3b4: mov    rbx, rax
005db3b7: test   rax, rax
005db3ba: je     0x1805db4ad
005db3c0: mov    rdi, qword ptr [rax + 0x10]
005db3c4: xor    edx, edx
005db3c6: mov    rcx, rax
005db3c9: call   0x182109730                              ; ʼʿʷʵʲʶʽʵʿʳʷ$$ʼʺʻʼˀʸʴʻʼˁʽ
005db3ce: xor    edx, edx
005db3d0: mov    rcx, rbx
005db3d3: cmp    rdi, rax
005db3d6: jne    0x1805db3e2
005db3d8: call   0x182109730                              ; ʼʿʷʵʲʶʽʵʿʳʷ$$ʼʺʻʼˀʸʴʻʼˁʽ
005db3dd: mov    rdi, rax
005db3e0: jmp    0x1805db3eb
005db3e2: call   0x182109730                              ; ʼʿʷʵʲʶʽʵʿʳʷ$$ʼʺʻʼˀʸʴʻʼˁʽ
005db3e7: lea    rdi, [rax - 1]
005db3eb: mov    r8, qword ptr [rbp + 0x38]
005db3ef: lea    rdx, [rsp + 0x20]
005db3f4: lea    rcx, [rsp + 0x30]
005db3f9: movdqa xmmword ptr [rsp + 0x20], xmm6
005db3ff: mov    r8, qword ptr [r8 + 0x38]
005db403: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
005db408: mov    r9, qword ptr [rbp + 0x38]
005db40c: lea    rcx, [rsp + 0x20]
005db411: movaps xmm0, xmmword ptr [rsp + 0x30]
005db416: mov    r8, rdi
005db419: mov    rdx, qword ptr [rbx + 0x10]
005db41d: movdqa xmmword ptr [rsp + 0x20], xmm0
005db423: mov    r9, qword ptr [r9 + 0x50]
005db427: call   0x1805fad90                              ; ʽˁʷʳʸʶʳʵʸʾʿ$$ʾˁʲʻʹʽʶʹʶʲʹ<object>
005db42c: cmp    eax, -1
005db42f: jne    0x1805db47d
005db431: shr    rax, 0x20
005db435: cmp    eax, -1
005db438: jne    0x1805db47d
005db43a: mov    r8, qword ptr [rbp + 0x38]
005db43e: mov    edx, esi
005db440: mov    rcx, r14
005db443: mov    r8, qword ptr [r8 + 0x58]
005db447: call   0x180774920                              ; System.Collections.Generic.List<object>$$RemoveAt
005db44c: mov    rax, qword ptr [rbp + 0x38]
005db450: mov    rbx, qword ptr [rax + 8]
005db454: cmp    qword ptr [rbx + 0x38], 0
005db459: jne    0x1805db463
005db45b: mov    rcx, rbx
005db45e: call   0x182f657d0
005db463: mov    r8, qword ptr [rbx + 0x38]
005db467: lea    rcx, [rsp + 0x40]
005db46c: mov    rdx, r15
005db46f: mov    r8, qword ptr [r8 + 8]
005db473: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
005db478: movaps xmm6, xmmword ptr [rsp + 0x40]
005db47d: sub    esi, 1
005db480: jns    0x1805db3a2
005db486: movaps xmm6, xmmword ptr [rsp + 0x50]
005db48b: mov    rdi, qword ptr [rsp + 0x88]
005db493: mov    rsi, qword ptr [rsp + 0x80]
005db49b: mov    rbx, qword ptr [rsp + 0x90]
005db4a3: add    rsp, 0x60
005db4a7: pop    r15
005db4a9: pop    r14
005db4ab: pop    rbp
005db4ac: ret    
005db4ad: call   0x182f60c50
005db4b2: int3   
005db4b3: int3   
