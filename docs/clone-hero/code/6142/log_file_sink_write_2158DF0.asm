02158df0: mov    qword ptr [rsp + 0x18], rbx
02158df5: mov    qword ptr [rsp + 0x20], rsi
02158dfa: push   rdi
02158dfb: sub    rsp, 0x40
02158dff: cmp    byte ptr [rip + 0x1dc08e3], 0            ; [0x3f196e9] (bss)
02158e06: mov    rdi, r8
02158e09: mov    rsi, rdx
02158e0c: mov    rbx, rcx
02158e0f: jne    0x182158e54
02158e11: lea    rcx, [rip + 0x1b56c80]                   ; [0x3cafa98] metamethod:Method$System.ArraySegment<char>.get_Array()
02158e18: call   0x182f609b0
02158e1d: lea    rcx, [rip + 0x1b56d2c]                   ; [0x3cafb50] metamethod:Method$System.ArraySegment<char>.get_Count()
02158e24: call   0x182f609b0
02158e29: lea    rcx, [rip + 0x1b56dd8]                   ; [0x3cafc08] metamethod:Method$System.ArraySegment<char>.get_Offset()
02158e30: call   0x182f609b0
02158e35: lea    rcx, [rip + 0x1bb0c2c]                   ; [0x3d09a68] meta:System.ArraySegment<char>_TypeInfo
02158e3c: call   0x182f609b0
02158e41: lea    rcx, [rip + 0x1b957d0]                   ; [0x3cee618] meta:Cysharp.Text.Utf16ValueStringBuilder_TypeInfo
02158e48: call   0x182f609b0
02158e4d: mov    byte ptr [rip + 0x1dc0895], 1            ; [0x3f196e9] (bss)
02158e54: cmp    qword ptr [rbx + 0x10], 0
02158e59: je     0x182158f45
02158e5f: cmp    qword ptr [rbx + 0x18], 0
02158e64: je     0x182158f45
02158e6a: mov    qword ptr [rsp + 0x50], rbp
02158e6f: mov    qword ptr [rsp + 0x58], r14
02158e74: mov    rcx, qword ptr [rbx + 0x30]
02158e78: xor    edx, edx
02158e7a: call   0x181983d80                              ; System.String$$IsNullOrEmpty
02158e7f: test   al, al
02158e81: jne    0x182158eb3
02158e83: test   rdi, rdi
02158e86: je     0x182158f55
02158e8c: mov    rcx, qword ptr [rdi + 0x40]
02158e90: test   rcx, rcx
02158e93: je     0x182158f55
02158e99: mov    rdx, qword ptr [rbx + 0x30]
02158e9d: xor    r9d, r9d
02158ea0: mov    r8d, 5
02158ea6: call   0x181981640                              ; System.String$$Equals
02158eab: test   al, al
02158ead: je     0x182158f3b
02158eb3: mov    rcx, qword ptr [rip + 0x1b9575e]         ; [0x3cee618] meta:Cysharp.Text.Utf16ValueStringBuilder_TypeInfo
02158eba: cmp    dword ptr [rcx + 0xe0], 0
02158ec1: jne    0x182158ec8
02158ec3: call   0x182f60cf0
02158ec8: xor    r8d, r8d
02158ecb: lea    rcx, [rsp + 0x30]
02158ed0: mov    rdx, rsi
02158ed3: call   0x182be61d0                              ; Cysharp.Text.Utf16ValueStringBuilder$$AsArraySegment
02158ed8: mov    rcx, qword ptr [rip + 0x1bb0b89]         ; [0x3d09a68] meta:System.ArraySegment<char>_TypeInfo
02158edf: mov    rdi, qword ptr [rbx + 0x18]
02158ee3: mov    rsi, qword ptr [rax]
02158ee6: cmp    dword ptr [rcx + 0xe0], 0
02158eed: mov    ebp, dword ptr [rax + 8]
02158ef0: mov    r14d, dword ptr [rax + 0xc]
02158ef4: jne    0x182158efb
02158ef6: call   0x182f60cf0
02158efb: test   rdi, rdi
02158efe: je     0x182158f55
02158f00: mov    r10, qword ptr [rdi]
02158f03: mov    r9d, r14d
02158f06: mov    r8d, ebp
02158f09: mov    rdx, rsi
02158f0c: mov    rcx, rdi
02158f0f: mov    rax, qword ptr [r10 + 0x2d0]
02158f16: mov    qword ptr [rsp + 0x20], rax
02158f1b: call   qword ptr [r10 + 0x2c8]
02158f22: mov    rcx, qword ptr [rbx + 0x18]
02158f26: test   rcx, rcx
02158f29: je     0x182158f55
02158f2b: mov    rax, qword ptr [rcx]
02158f2e: mov    rdx, qword ptr [rax + 0x200]
02158f35: call   qword ptr [rax + 0x1f8]
02158f3b: mov    rbp, qword ptr [rsp + 0x50]
02158f40: mov    r14, qword ptr [rsp + 0x58]
02158f45: mov    rbx, qword ptr [rsp + 0x60]
02158f4a: mov    rsi, qword ptr [rsp + 0x68]
02158f4f: add    rsp, 0x40
02158f53: pop    rdi
02158f54: ret    
02158f55: call   0x182f60c50
02158f5a: int3   
02158f5b: int3   
