0215bd50: push   rbx
0215bd52: push   rbp
0215bd53: push   rsi
0215bd54: push   rdi
0215bd55: sub    rsp, 0x38
0215bd59: cmp    byte ptr [rip + 0x1dbd9f2], 0            ; [0x3f19752] (bss)
0215bd60: movzx  edi, r9b
0215bd64: mov    rsi, r8
0215bd67: mov    rbp, rdx
0215bd6a: mov    rbx, rcx
0215bd6d: jne    0x18215bd8e
0215bd6f: lea    rcx, [rip + 0x1b71412]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0215bd76: call   0x182f609b0
0215bd7b: lea    rcx, [rip + 0x1b9c2de]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215bd82: call   0x182f609b0
0215bd87: mov    byte ptr [rip + 0x1dbd9c4], 1            ; [0x3f19752] (bss)
0215bd8e: mov    rcx, qword ptr [rbx + 0x30]
0215bd92: test   rcx, rcx
0215bd95: je     0x18215be40
0215bd9b: mov    rdx, qword ptr [rip + 0x1b9c2be]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
0215bda2: mov    rbx, qword ptr [rbx + 0x88]
0215bda9: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
0215bdae: test   rax, rax
0215bdb1: je     0x18215be40
0215bdb7: movzx  r9d, dil
0215bdbb: mov    qword ptr [rsp + 0x20], 0
0215bdc4: mov    r8, rsi
0215bdc7: mov    rdx, rbp
0215bdca: mov    rcx, rax
0215bdcd: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
0215bdd2: mov    r9, rax
0215bdd5: test   rbx, rbx
0215bdd8: je     0x18215be40
0215bdda: mov    rax, qword ptr [rip + 0x1b713a7]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
0215bde1: inc    dword ptr [rbx + 0x1c]
0215bde4: mov    rcx, qword ptr [rbx + 0x10]
0215bde8: test   rcx, rcx
0215bdeb: je     0x18215be40
0215bded: movsxd rdx, dword ptr [rbx + 0x18]
0215bdf1: cmp    edx, dword ptr [rcx + 0x18]
0215bdf4: jb     0x18215be18
0215bdf6: mov    rax, qword ptr [rax + 0x20]
0215bdfa: mov    rdx, r9
0215bdfd: mov    rcx, rbx
0215be00: mov    r8, qword ptr [rax + 0xc0]
0215be07: mov    r8, qword ptr [r8 + 0x70]
0215be0b: add    rsp, 0x38
0215be0f: pop    rdi
0215be10: pop    rsi
0215be11: pop    rbp
0215be12: pop    rbx
0215be13: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0215be18: lea    eax, [rdx + 1]
0215be1b: mov    dword ptr [rbx + 0x18], eax
0215be1e: cmp    edx, dword ptr [rcx + 0x18]
0215be21: jae    0x18215be46
0215be23: mov    qword ptr [rcx + rdx*8 + 0x20], r9
0215be28: lea    rcx, [rcx + rdx*8]
0215be2c: add    rcx, 0x20
0215be30: mov    rdx, r9
0215be33: add    rsp, 0x38
0215be37: pop    rdi
0215be38: pop    rsi
0215be39: pop    rbp
0215be3a: pop    rbx
0215be3b: jmp    0x182f5fc00
0215be40: call   0x182f60c50
0215be45: int3   
0215be46: call   0x182f60c40
0215be4b: int3   
0215be4c: int3   
