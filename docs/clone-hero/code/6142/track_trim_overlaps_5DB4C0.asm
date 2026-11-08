005db4c0: mov    qword ptr [rsp + 0x18], r8
005db4c5: mov    qword ptr [rsp + 8], rcx
005db4ca: push   rbp
005db4cb: push   rsi
005db4cc: push   rdi
005db4cd: push   r12
005db4cf: push   r13
005db4d1: push   r14
005db4d3: push   r15
005db4d5: sub    rsp, 0x40
005db4d9: lea    rbp, [rsp + 0x20]
005db4de: cmp    qword ptr [r8 + 0x38], 0
005db4e3: mov    r13, rdx
005db4e6: mov    qword ptr [rbp + 0x68], rbx
005db4ea: mov    rbx, r8
005db4ed: jne    0x1805db53a
005db4ef: lea    rcx, [rip + 0x371a952]                   ; [0x3cf5e48] meta:System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>_TypeInfo
005db4f6: call   0x182f609b0
005db4fb: lea    rcx, [rip + 0x36c7266]                   ; [0x3ca2768] meta:System.Math_TypeInfo
005db502: call   0x182f609b0
005db507: lea    rcx, [rip + 0x36cd142]                   ; [0x3ca8650] metamethod:Method$System.Span<ValueTuple<int, long>>..ctor()
005db50e: call   0x182f609b0
005db513: lea    rcx, [rip + 0x36be55e]                   ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db51a: call   0x182f609b0
005db51f: lea    rcx, [rip + 0x370c7d2]                   ; [0x3ce7cf8] metamethod:Method$System.ValueTuple<int, long>..ctor()
005db526: call   0x182f609b0
005db52b: cmp    qword ptr [rbx + 0x38], 0
005db530: jne    0x1805db53a
005db532: mov    rcx, rbx
005db535: call   0x182f657d0
005db53a: mov    eax, dword ptr [rsp]
005db53d: xorps  xmm0, xmm0
005db540: movups xmmword ptr [rbp + 0x10], xmm0
005db544: mov    eax, 0x200
005db549: sub    rsp, rax
005db54c: lea    r12, [rsp + 0x20]
005db551: mov    eax, dword ptr [r12]
005db555: mov    qword ptr [rbp + 0x78], r12
005db559: xor    edx, edx
005db55b: mov    r8d, 0x200
005db561: mov    rcx, r12
005db564: call   0x18305d9d0
005db569: xor    esi, esi
005db56b: xor    eax, eax
005db56d: test   r13, r13
005db570: je     0x1805db767
005db576: cmp    eax, dword ptr [r13 + 0x18]
005db57a: jge    0x1805db719
005db580: mov    r8, qword ptr [rbx + 0x38]
005db584: mov    edx, esi
005db586: mov    rcx, r13
005db589: mov    r8, qword ptr [r8 + 8]
005db58d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db592: mov    rbx, rax
005db595: test   rax, rax
005db598: je     0x1805db767
005db59e: mov    r10, qword ptr [rax]
005db5a1: mov    rdi, qword ptr [rax + 0x18]
005db5a5: mov    r15, qword ptr [rax + 0x10]
005db5a9: xor    eax, eax
005db5ab: mov    r9, qword ptr [rip + 0x36be4c6]          ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db5b2: movzx  edx, word ptr [r10 + 0x12e]
005db5ba: cmp    ax, dx
005db5bd: jae    0x1805db5e8
005db5bf: mov    r8, qword ptr [r10 + 0xb0]
005db5c6: nop    word ptr [rax + rax]
005db5d0: movzx  ecx, ax
005db5d3: add    rcx, rcx
005db5d6: cmp    qword ptr [r8 + rcx*8], r9
005db5da: je     0x1805db6bf
005db5e0: inc    ax
005db5e3: cmp    ax, dx
005db5e6: jb     0x1805db5d0
005db5e8: xor    r8d, r8d
005db5eb: mov    rdx, r9
005db5ee: mov    rcx, rbx
005db5f1: call   0x182f65070
005db5f6: mov    rdx, qword ptr [rax + 8]
005db5fa: mov    rcx, rbx
005db5fd: call   qword ptr [rax]
005db5ff: movsxd r14, eax
005db602: cmp    r14d, 0x20
005db606: jae    0x1805db773
005db60c: add    r15, rdi
005db60f: mov    rcx, r14
005db612: mov    rdi, qword ptr [rbx + 0x10]
005db616: add    rcx, rcx
005db619: mov    r12, qword ptr [r12 + rcx*8 + 8]
005db61e: cmp    rdi, r12
005db621: jge    0x1805db6dc
005db627: mov    rdx, qword ptr [rbp + 0x78]
005db62b: mov    r8, qword ptr [rbp + 0x70]
005db62f: mov    edx, dword ptr [rdx + rcx*8]
005db632: mov    rcx, r13
005db635: mov    r8, qword ptr [r8 + 0x38]
005db639: mov    r8, qword ptr [r8 + 8]
005db63d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db642: test   rax, rax
005db645: je     0x1805db76d
005db64b: sub    rdi, qword ptr [rax + 0x10]
005db64f: mov    qword ptr [rax + 0x18], rdi
005db653: mov    rcx, qword ptr [rip + 0x36c710e]         ; [0x3ca2768] meta:System.Math_TypeInfo
005db65a: cmp    dword ptr [rcx + 0xe0], 0
005db661: jne    0x1805db668
005db663: call   0x182f60cf0
005db668: xor    r8d, r8d
005db66b: mov    rdx, r12
005db66e: mov    rcx, r15
005db671: call   0x181b51e40                              ; Rewired.Utils.MathTools$$Max
005db676: mov    r15, rax
005db679: lea    rcx, [rbp]
005db67d: sub    rax, qword ptr [rbx + 0x10]
005db681: xorps  xmm0, xmm0
005db684: mov    qword ptr [rbx + 0x18], rax
005db688: mov    r8, r15
005db68b: mov    r9, qword ptr [rip + 0x370c666]          ; [0x3ce7cf8] metamethod:Method$System.ValueTuple<int, long>..ctor()
005db692: mov    edx, esi
005db694: movups xmmword ptr [rbp], xmm0
005db698: call   0x180ce1550                              ; System.Collections.Generic.KeyValuePair<int, long>$$.ctor
005db69d: mov    rdi, qword ptr [rbp + 0x78]
005db6a1: mov    rcx, r14
005db6a4: mov    r12, qword ptr [rbp + 8]
005db6a8: add    rcx, rcx
005db6ab: mov    edx, dword ptr [rbp]
005db6ae: mov    eax, dword ptr [rbp + 4]
005db6b1: mov    dword ptr [rdi + rcx*8], edx
005db6b4: mov    dword ptr [rdi + rcx*8 + 4], eax
005db6b8: mov    qword ptr [rdi + rcx*8 + 8], r12
005db6bd: jmp    0x1805db6e0
005db6bf: movzx  ecx, ax
005db6c2: add    rcx, rcx
005db6c5: movsxd rax, dword ptr [r8 + rcx*8 + 8]
005db6ca: shl    rax, 4
005db6ce: add    rax, 0x138
005db6d4: add    rax, r10
005db6d7: jmp    0x1805db5f6
005db6dc: mov    rdi, qword ptr [rbp + 0x78]
005db6e0: cmp    r15, r12
005db6e3: jle    0x1805db708
005db6e5: mov    r9, qword ptr [rip + 0x370c60c]          ; [0x3ce7cf8] metamethod:Method$System.ValueTuple<int, long>..ctor()
005db6ec: lea    rcx, [rbp + 0x10]
005db6f0: mov    r8, r15
005db6f3: mov    edx, esi
005db6f5: call   0x180ce1550                              ; System.Collections.Generic.KeyValuePair<int, long>$$.ctor
005db6fa: movups xmm0, xmmword ptr [rbp + 0x10]
005db6fe: mov    rax, r14
005db701: add    rax, rax
005db704: movups xmmword ptr [rdi + rax*8], xmm0
005db708: mov    rbx, qword ptr [rbp + 0x70]
005db70c: inc    esi
005db70e: mov    r12, qword ptr [rbp + 0x78]
005db712: mov    eax, esi
005db714: jmp    0x1805db576
005db719: mov    rdx, qword ptr [rip + 0x371a728]         ; [0x3cf5e48] meta:System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>_TypeInfo
005db720: mov    r8, qword ptr [r13]
005db724: movzx  eax, byte ptr [rdx + 0x130]
005db72b: cmp    byte ptr [r8 + 0x130], al
005db732: jb     0x1805db753
005db734: movzx  ecx, al
005db737: mov    rax, qword ptr [r8 + 0xc8]
005db73e: cmp    qword ptr [rax + rcx*8 - 8], rdx
005db743: jne    0x1805db753
005db745: mov    rcx, qword ptr [rbp + 0x60]
005db749: xor    r8d, r8d
005db74c: xor    edx, edx
005db74e: call   0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
005db753: mov    rbx, qword ptr [rbp + 0x68]
005db757: lea    rsp, [rbp + 0x20]
005db75b: pop    r15
005db75d: pop    r14
005db75f: pop    r13
005db761: pop    r12
005db763: pop    rdi
005db764: pop    rsi
005db765: pop    rbp
005db766: ret    
005db767: call   0x182f60c50
005db76c: int3   
005db76d: call   0x182f60c50
005db772: int3   
005db773: call   0x182f60c40
005db778: int3   
005db779: int3   
