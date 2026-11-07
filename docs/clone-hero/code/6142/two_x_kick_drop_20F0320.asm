020f0320: push   rbx
020f0322: push   rdi
020f0323: sub    rsp, 0x28
020f0327: cmp    byte ptr [rip + 0x1e2904e], 0            ; [0x3f1937c] (bss)
020f032e: mov    rdi, rcx
020f0331: jne    0x1820f0352
020f0333: lea    rcx, [rip + 0x1be6ea6]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f033a: call   0x182f609b0
020f033f: lea    rcx, [rip + 0x1be6f5a]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0346: call   0x182f609b0
020f034b: mov    byte ptr [rip + 0x1e2902a], 1            ; [0x3f1937c] (bss)
020f0352: mov    qword ptr [rsp + 0x40], rbp
020f0357: xor    ebx, ebx
020f0359: mov    qword ptr [rsp + 0x48], rsi
020f035e: xor    eax, eax
020f0360: mov    qword ptr [rsp + 0x50], r14
020f0365: mov    qword ptr [rsp + 0x20], r15
020f036a: test   rdi, rdi
020f036d: je     0x1820f0510
020f0373: cmp    eax, dword ptr [rdi + 0x18]
020f0376: jge    0x1820f04f5
020f037c: mov    r8, qword ptr [rip + 0x1be6f1d]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0383: mov    edx, ebx
020f0385: mov    rcx, rdi
020f0388: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f038d: test   rax, rax
020f0390: je     0x1820f0510
020f0396: movzx  eax, word ptr [rax + 0x80]
020f039d: cmp    ax, 4
020f03a1: jne    0x1820f044f
020f03a7: mov    r8, qword ptr [rip + 0x1be6ef2]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f03ae: mov    edx, ebx
020f03b0: mov    rcx, rdi
020f03b3: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f03b8: test   rax, rax
020f03bb: je     0x1820f0510
020f03c1: xor    edx, edx
020f03c3: mov    rcx, rax
020f03c6: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f03cb: test   al, 8
020f03cd: jne    0x1820f0423
020f03cf: mov    r8, qword ptr [rip + 0x1be6eca]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f03d6: mov    edx, ebx
020f03d8: mov    rcx, rdi
020f03db: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f03e0: test   rax, rax
020f03e3: je     0x1820f0510
020f03e9: xor    edx, edx
020f03eb: mov    rcx, rax
020f03ee: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f03f3: test   al, 0x40
020f03f5: jne    0x1820f0423
020f03f7: mov    r8, qword ptr [rip + 0x1be6ea2]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f03fe: mov    edx, ebx
020f0400: mov    rcx, rdi
020f0403: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0408: test   rax, rax
020f040b: je     0x1820f0510
020f0411: inc    ebx
020f0413: mov    word ptr [rax + 0x80], 8
020f041c: mov    eax, ebx
020f041e: jmp    0x1820f0373
020f0423: mov    r8, qword ptr [rip + 0x1be6e76]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f042a: mov    edx, ebx
020f042c: mov    rcx, rdi
020f042f: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0434: test   rax, rax
020f0437: je     0x1820f0510
020f043d: inc    ebx
020f043f: mov    word ptr [rax + 0x80], 2
020f0448: mov    eax, ebx
020f044a: jmp    0x1820f0373
020f044f: cmp    ax, 0x40
020f0453: jne    0x1820f04ec
020f0459: mov    r8, qword ptr [rip + 0x1be6e40]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0460: mov    edx, ebx
020f0462: mov    rcx, rdi
020f0465: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f046a: test   rax, rax
020f046d: je     0x1820f0510
020f0473: xor    edx, edx
020f0475: mov    rcx, rax
020f0478: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f047d: test   al, 0x10
020f047f: jne    0x1820f04cd
020f0481: mov    r8, qword ptr [rip + 0x1be6e18]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0488: mov    edx, ebx
020f048a: mov    rcx, rdi
020f048d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0492: test   rax, rax
020f0495: je     0x1820f0510
020f0497: xor    edx, edx
020f0499: mov    rcx, rax
020f049c: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020f04a1: test   al, al
020f04a3: js     0x1820f04cd
020f04a5: mov    r8, qword ptr [rip + 0x1be6df4]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f04ac: mov    edx, ebx
020f04ae: mov    rcx, rdi
020f04b1: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f04b6: test   rax, rax
020f04b9: je     0x1820f0510
020f04bb: inc    ebx
020f04bd: mov    word ptr [rax + 0x80], 0x80
020f04c6: mov    eax, ebx
020f04c8: jmp    0x1820f0373
020f04cd: mov    r8, qword ptr [rip + 0x1be6dcc]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f04d4: mov    edx, ebx
020f04d6: mov    rcx, rdi
020f04d9: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f04de: test   rax, rax
020f04e1: je     0x1820f0510
020f04e3: mov    word ptr [rax + 0x80], 0x20
020f04ec: inc    ebx
020f04ee: mov    eax, ebx
020f04f0: jmp    0x1820f0373
020f04f5: mov    r15, qword ptr [rsp + 0x20]
020f04fa: mov    r14, qword ptr [rsp + 0x50]
020f04ff: mov    rsi, qword ptr [rsp + 0x48]
020f0504: mov    rbp, qword ptr [rsp + 0x40]
020f0509: add    rsp, 0x28
020f050d: pop    rdi
020f050e: pop    rbx
020f050f: ret    
020f0510: call   0x182f60c50
020f0515: int3   
020f0516: int3   
