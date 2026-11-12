0214db20: push   rbx
0214db22: push   rbp
0214db23: push   rsi
0214db24: push   rdi
0214db25: sub    rsp, 0x28
0214db29: cmp    byte ptr [rip + 0x1dcbad8], 0            ; [0x3f19608] (bss)
0214db30: mov    ebp, r8d
0214db33: mov    rsi, rdx
0214db36: mov    rdi, rcx
0214db39: jne    0x18214db4e
0214db3b: lea    rcx, [rip + 0x1b89c9e]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0214db42: call   0x182f609b0
0214db47: mov    byte ptr [rip + 0x1dcbaba], 1            ; [0x3f19608] (bss)
0214db4e: mov    ebx, dword ptr [rdi + 0x2c]
0214db51: cmp    ebx, dword ptr [rdi + 0x30]
0214db54: jg     0x18214db88
0214db56: cmp    ebx, dword ptr [rdi + 0x28]
0214db59: je     0x18214db81
0214db5b: test   rsi, rsi
0214db5e: je     0x18214db93
0214db60: mov    rcx, qword ptr [rsi + 0x60]
0214db64: test   rcx, rcx
0214db67: je     0x18214db93
0214db69: mov    r8, qword ptr [rip + 0x1b89c70]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0214db70: mov    edx, ebx
0214db72: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0214db77: test   rax, rax
0214db7a: je     0x18214db93
0214db7c: cmp    dword ptr [rax + 0x20], ebp
0214db7f: je     0x18214db8a
0214db81: inc    ebx
0214db83: cmp    ebx, dword ptr [rdi + 0x30]
0214db86: jle    0x18214db56
0214db88: xor    eax, eax
0214db8a: add    rsp, 0x28
0214db8e: pop    rdi
0214db8f: pop    rsi
0214db90: pop    rbp
0214db91: pop    rbx
0214db92: ret    
0214db93: call   0x182f60c50
0214db98: int3   
0214db99: int3   
