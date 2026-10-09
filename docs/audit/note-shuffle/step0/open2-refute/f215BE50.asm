0215be50: push   rsi
0215be52: push   rdi
0215be53: sub    rsp, 0x28
0215be57: cmp    byte ptr [rip + 0x1dbd8ce], 0            ; [0x3f1972c] (bss)
0215be5e: mov    edi, edx
0215be60: mov    rsi, rcx
0215be63: jne    0x18215be84
0215be65: lea    rcx, [rip + 0x1b7b8b4]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
0215be6c: call   0x182f609b0
0215be71: lea    rcx, [rip + 0x1b7b968]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215be78: call   0x182f609b0
0215be7d: mov    byte ptr [rip + 0x1dbd8a8], 1            ; [0x3f1972c] (bss)
0215be84: mov    rax, qword ptr [rsi + 0x60]
0215be88: mov    qword ptr [rsp + 0x40], rbx
0215be8d: mov    qword ptr [rsp + 0x48], rbp
0215be92: mov    qword ptr [rsp + 0x50], r13
0215be97: mov    qword ptr [rsp + 0x58], r14
0215be9c: mov    qword ptr [rsp + 0x20], r15
0215bea1: test   rax, rax
0215bea4: je     0x18215c030
0215beaa: cmp    dword ptr [rax + 0x18], 0
0215beae: je     0x18215c010
0215beb4: mov    r8, qword ptr [rip + 0x1b7b925]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215bebb: mov    edx, edi
0215bebd: mov    rcx, rax
0215bec0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215bec5: test   rax, rax
0215bec8: je     0x18215c030
0215bece: mov    rcx, qword ptr [rsi + 0x60]
0215bed2: test   rcx, rcx
0215bed5: je     0x18215c030
0215bedb: mov    r8, qword ptr [rip + 0x1b7b8fe]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215bee2: mov    edx, edi
0215bee4: mov    rbx, qword ptr [rax + 0x10]
0215bee8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215beed: test   rax, rax
0215bef0: je     0x18215c030
0215bef6: cmp    byte ptr [rsi + 0x58], 6
0215befa: mov    rbp, qword ptr [rax + 0x18]
0215befe: jne    0x18215bf05
0215bf00: mov    r14b, 1
0215bf03: jmp    0x18215bf0d
0215bf05: cmp    byte ptr [rsi + 0x58], 9
0215bf09: sete   r14b
0215bf0d: mov    rax, qword ptr [rsi + 0x60]
0215bf11: mov    r13d, edi
0215bf14: mov    edx, edi
0215bf16: test   rax, rax
0215bf19: je     0x18215c030
0215bf1f: nop    
0215bf20: mov    rcx, qword ptr [rsi + 0x60]
0215bf24: cmp    edx, dword ptr [rax + 0x18]
0215bf27: jge    0x18215bfd1
0215bf2d: test   rcx, rcx
0215bf30: je     0x18215c030
0215bf36: mov    r8, qword ptr [rip + 0x1b7b8a3]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215bf3d: mov    edx, r13d
0215bf40: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215bf45: mov    r15, rax
0215bf48: test   rax, rax
0215bf4b: je     0x18215bfbd
0215bf4d: mov    dword ptr [rax + 0x28], r13d
0215bf51: cmp    qword ptr [rax + 0x10], rbx
0215bf55: je     0x18215bfb4
0215bf57: lea    ebp, [r13 - 1]
0215bf5b: mov    ebx, edi
0215bf5d: cmp    edi, ebp
0215bf5f: jg     0x18215bf91
0215bf61: mov    rcx, qword ptr [rsi + 0x60]
0215bf65: test   rcx, rcx
0215bf68: je     0x18215c030
0215bf6e: mov    r8, qword ptr [rip + 0x1b7b86b]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215bf75: mov    edx, ebx
0215bf77: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215bf7c: test   rax, rax
0215bf7f: je     0x18215bf8b
0215bf81: mov    dword ptr [rax + 0x2c], edi
0215bf84: mov    dword ptr [rax + 0x30], ebp
0215bf87: mov    byte ptr [rax + 0x34], r14b
0215bf8b: inc    ebx
0215bf8d: cmp    ebx, ebp
0215bf8f: jle    0x18215bf61
0215bf91: cmp    byte ptr [rsi + 0x58], 6
0215bf95: mov    rbx, qword ptr [r15 + 0x10]
0215bf99: mov    rbp, qword ptr [r15 + 0x18]
0215bf9d: jne    0x18215bfa7
0215bf9f: mov    r14b, 1
0215bfa2: mov    edi, r13d
0215bfa5: jmp    0x18215bfbd
0215bfa7: cmp    byte ptr [rsi + 0x58], 9
0215bfab: mov    edi, r13d
0215bfae: sete   r14b
0215bfb2: jmp    0x18215bfbd
0215bfb4: cmp    qword ptr [rax + 0x18], rbp
0215bfb8: je     0x18215bfbd
0215bfba: mov    r14b, 1
0215bfbd: mov    rax, qword ptr [rsi + 0x60]
0215bfc1: inc    r13d
0215bfc4: mov    edx, r13d
0215bfc7: test   rax, rax
0215bfca: je     0x18215c030
0215bfcc: jmp    0x18215bf20
0215bfd1: test   rcx, rcx
0215bfd4: je     0x18215c030
0215bfd6: mov    ebp, dword ptr [rcx + 0x18]
0215bfd9: mov    ebx, edi
0215bfdb: dec    ebp
0215bfdd: cmp    edi, ebp
0215bfdf: jg     0x18215c010
0215bfe1: mov    rcx, qword ptr [rsi + 0x60]
0215bfe5: test   rcx, rcx
0215bfe8: je     0x18215c030
0215bfea: mov    r8, qword ptr [rip + 0x1b7b7ef]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0215bff1: mov    edx, ebx
0215bff3: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0215bff8: test   rax, rax
0215bffb: je     0x18215c00a
0215bffd: mov    dword ptr [rax + 0x28], ebx
0215c000: mov    dword ptr [rax + 0x2c], edi
0215c003: mov    dword ptr [rax + 0x30], ebp
0215c006: mov    byte ptr [rax + 0x34], r14b
0215c00a: inc    ebx
0215c00c: cmp    ebx, ebp
0215c00e: jle    0x18215bfe1
0215c010: mov    r15, qword ptr [rsp + 0x20]
0215c015: mov    r14, qword ptr [rsp + 0x58]
0215c01a: mov    r13, qword ptr [rsp + 0x50]
0215c01f: mov    rbp, qword ptr [rsp + 0x48]
0215c024: mov    rbx, qword ptr [rsp + 0x40]
0215c029: add    rsp, 0x28
0215c02d: pop    rdi
0215c02e: pop    rsi
0215c02f: ret    
0215c030: call   0x182f60c50
0215c035: int3   
0215c036: int3   
