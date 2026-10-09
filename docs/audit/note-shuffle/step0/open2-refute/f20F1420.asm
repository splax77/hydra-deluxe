020f1420: mov    qword ptr [rsp + 0x18], rbx
020f1425: push   rdi
020f1426: sub    rsp, 0x20
020f142a: cmp    byte ptr [rip + 0x1e27f49], 0            ; [0x3f1937a] (bss)
020f1431: mov    rdi, rcx
020f1434: jne    0x1820f1455
020f1436: lea    rcx, [rip + 0x1be5da3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f143d: call   0x182f609b0
020f1442: lea    rcx, [rip + 0x1be5e57]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1449: call   0x182f609b0
020f144e: mov    byte ptr [rip + 0x1e27f25], 1            ; [0x3f1937a] (bss)
020f1455: xor    ebx, ebx
020f1457: mov    qword ptr [rsp + 0x30], rbp
020f145c: xor    eax, eax
020f145e: mov    qword ptr [rsp + 0x38], rsi
020f1463: test   rdi, rdi
020f1466: je     0x1820f1532
020f146c: nop    dword ptr [rax]
020f1470: cmp    eax, dword ptr [rdi + 0x18]
020f1473: jge    0x1820f151d
020f1479: mov    r8, qword ptr [rip + 0x1be5e20]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1480: mov    edx, ebx
020f1482: mov    rcx, rdi
020f1485: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f148a: test   rax, rax
020f148d: je     0x1820f1532
020f1493: xor    edx, edx
020f1495: mov    rcx, rax
020f1498: call   0x18214b780                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʷʶʳˀˀʸʿʷʲʷʷ
020f149d: test   al, al
020f149f: je     0x1820f1514
020f14a1: mov    r8, qword ptr [rip + 0x1be5df8]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14a8: mov    edx, ebx
020f14aa: mov    rcx, rdi
020f14ad: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f14b2: test   rax, rax
020f14b5: je     0x1820f1532
020f14b7: movzx  eax, word ptr [rax + 0x80]
020f14be: cmp    ax, 2
020f14c2: jne    0x1820f14e9
020f14c4: mov    r8, qword ptr [rip + 0x1be5dd5]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14cb: mov    edx, ebx
020f14cd: mov    rcx, rdi
020f14d0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f14d5: test   rax, rax
020f14d8: je     0x1820f1532
020f14da: inc    ebx
020f14dc: mov    word ptr [rax + 0x80], 0x20
020f14e5: mov    eax, ebx
020f14e7: jmp    0x1820f1470
020f14e9: cmp    ax, 4
020f14ed: je     0x1820f14f5
020f14ef: cmp    ax, 0x20
020f14f3: jne    0x1820f1514
020f14f5: mov    r8, qword ptr [rip + 0x1be5da4]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14fc: mov    edx, ebx
020f14fe: mov    rcx, rdi
020f1501: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1506: test   rax, rax
020f1509: je     0x1820f1532
020f150b: mov    word ptr [rax + 0x80], 2
020f1514: inc    ebx
020f1516: mov    eax, ebx
020f1518: jmp    0x1820f1470
020f151d: mov    rsi, qword ptr [rsp + 0x38]
020f1522: mov    rbp, qword ptr [rsp + 0x30]
020f1527: mov    rbx, qword ptr [rsp + 0x40]
020f152c: add    rsp, 0x20
020f1530: pop    rdi
020f1531: ret    
020f1532: call   0x182f60c50
020f1537: int3   
020f1538: int3   
