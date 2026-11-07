0215b7c0: push   rbx
0215b7c2: push   rbp
0215b7c3: push   rsi
0215b7c4: push   rdi
0215b7c5: sub    rsp, 0x38
0215b7c9: cmp    byte ptr [rip + 0x1dbdf84], 0            ; [0x3f19754] (bss)
0215b7d0: movzx  edi, r9b
0215b7d4: mov    rsi, r8
0215b7d7: mov    rbp, rdx
0215b7da: mov    rbx, rcx
0215b7dd: jne    0x18215b7fe
0215b7df: lea    rcx, [rip + 0x1b719a2]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0215b7e6: call   0x182f609b0
0215b7eb: lea    rcx, [rip + 0x1b9c86e]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215b7f2: call   0x182f609b0
0215b7f7: mov    byte ptr [rip + 0x1dbdf56], 1            ; [0x3f19754] (bss)
0215b7fe: mov    rcx, qword ptr [rbx + 0x30]
0215b802: test   rcx, rcx
0215b805: je     0x18215b8b0
0215b80b: mov    rdx, qword ptr [rip + 0x1b9c84e]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215b812: mov    rbx, qword ptr [rbx + 0x88]
0215b819: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0215b81e: test   rax, rax
0215b821: je     0x18215b8b0
0215b827: movzx  r9d, dil
0215b82b: mov    qword ptr [rsp + 0x20], 0
0215b834: mov    r8, rsi
0215b837: mov    rdx, rbp
0215b83a: mov    rcx, rax
0215b83d: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
0215b842: mov    r9, rax
0215b845: test   rbx, rbx
0215b848: je     0x18215b8b0
0215b84a: mov    rax, qword ptr [rip + 0x1b71937]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0215b851: inc    dword ptr [rbx + 0x1c]
0215b854: mov    rcx, qword ptr [rbx + 0x10]
0215b858: test   rcx, rcx
0215b85b: je     0x18215b8b0
0215b85d: movsxd rdx, dword ptr [rbx + 0x18]
0215b861: cmp    edx, dword ptr [rcx + 0x18]
0215b864: jb     0x18215b888
0215b866: mov    rax, qword ptr [rax + 0x20]
0215b86a: mov    rdx, r9
0215b86d: mov    rcx, rbx
0215b870: mov    r8, qword ptr [rax + 0xc0]
0215b877: mov    r8, qword ptr [r8 + 0x70]
0215b87b: add    rsp, 0x38
0215b87f: pop    rdi
0215b880: pop    rsi
0215b881: pop    rbp
0215b882: pop    rbx
0215b883: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0215b888: lea    eax, [rdx + 1]
0215b88b: mov    dword ptr [rbx + 0x18], eax
0215b88e: cmp    edx, dword ptr [rcx + 0x18]
0215b891: jae    0x18215b8b6
0215b893: mov    qword ptr [rcx + rdx*8 + 0x20], r9
0215b898: lea    rcx, [rcx + rdx*8]
0215b89c: add    rcx, 0x20
0215b8a0: mov    rdx, r9
0215b8a3: add    rsp, 0x38
0215b8a7: pop    rdi
0215b8a8: pop    rsi
0215b8a9: pop    rbp
0215b8aa: pop    rbx
0215b8ab: jmp    0x182f5fc00
0215b8b0: call   0x182f60c50
0215b8b5: int3   
0215b8b6: call   0x182f60c40
0215b8bb: int3   
0215b8bc: int3   
