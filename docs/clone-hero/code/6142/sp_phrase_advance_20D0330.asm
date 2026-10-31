020d0330: push   rbx
020d0332: sub    rsp, 0x20
020d0336: cmp    byte ptr [rip + 0x1e48fd3], 0            ; [0x3f19310] (bss)
020d033d: mov    rbx, rcx
020d0340: jne    0x1820d0361
020d0342: lea    rcx, [rip + 0x1bff6b7]                   ; [0x3ccfa00] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Count()
020d0349: call   0x182f609b0
020d034e: lea    rcx, [rip + 0x1bff76b]                   ; [0x3ccfac0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Item()
020d0355: call   0x182f609b0
020d035a: mov    byte ptr [rip + 0x1e48faf], 1            ; [0x3f19310] (bss)
020d0361: mov    rax, qword ptr [rbx + 8]
020d0365: test   rax, rax
020d0368: je     0x1820d03a0
020d036a: mov    eax, dword ptr [rax + 0x18]
020d036d: dec    eax
020d036f: cmp    dword ptr [rbx], eax
020d0371: jg     0x1820d039a
020d0373: mov    r8, qword ptr [rip + 0x1bff746]          ; [0x3ccfac0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Item()
020d037a: mov    edx, dword ptr [rbx]
020d037c: mov    rcx, qword ptr [rbx + 8]
020d0380: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d0385: test   rax, rax
020d0388: je     0x1820d03a0
020d038a: mov    rcx, qword ptr [rax + 0x10]
020d038e: mov    qword ptr [rbx + 0x10], rcx
020d0392: add    rcx, qword ptr [rax + 0x18]
020d0396: mov    qword ptr [rbx + 0x18], rcx
020d039a: add    rsp, 0x20
020d039e: pop    rbx
020d039f: ret    
020d03a0: call   0x182f60c50
020d03a5: int3   
020d03a6: int3   
