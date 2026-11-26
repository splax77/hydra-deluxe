00fe5b10: mov    qword ptr [rsp + 0x10], rbx
00fe5b15: mov    qword ptr [rsp + 0x18], rbp
00fe5b1a: push   rsi
00fe5b1b: push   rdi
00fe5b1c: push   r14
00fe5b1e: sub    rsp, 0x20
00fe5b22: cmp    byte ptr [rip + 0x2f2b15b], 0            ; [0x3f10c84] (bss)
00fe5b29: mov    rdi, rdx
00fe5b2c: mov    rbx, rcx
00fe5b2f: jne    0x180fe5b44
00fe5b31: lea    rcx, [rip + 0x2cad440]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
00fe5b38: call   0x182f609b0
00fe5b3d: mov    byte ptr [rip + 0x2f2b140], 1            ; [0x3f10c84] (bss)
00fe5b44: mov    rcx, qword ptr [rip + 0x2cad42d]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
00fe5b4b: mov    r14d, dword ptr [rbx + 0x10]
00fe5b4f: mov    esi, dword ptr [rbx + 0x14]
00fe5b52: mov    ebp, dword ptr [rbx + 0x18]
00fe5b55: cmp    dword ptr [rcx + 0xe0], 0
00fe5b5c: jne    0x180fe5b63
00fe5b5e: call   0x182f60cf0
00fe5b63: mov    rax, qword ptr [rdi + 0x20]
00fe5b67: mov    dword ptr [rsp + 0x40], r14d
00fe5b6c: mov    rcx, qword ptr [rax + 0xc0]
00fe5b73: mov    rdi, qword ptr [rcx + 0x30]
00fe5b77: cmp    qword ptr [rdi + 0x38], 0
00fe5b7c: jne    0x180fe5b86
00fe5b7e: mov    rcx, rdi
00fe5b81: call   0x182f657d0
00fe5b86: mov    r8, qword ptr [rdi + 0x38]
00fe5b8a: lea    rcx, [rsp + 0x40]
00fe5b8f: mov    edx, esi
00fe5b91: mov    r8, qword ptr [r8 + 0x10]
00fe5b95: call   0x181b4e780                              ; System.Int32$$CompareTo
00fe5b9a: test   eax, eax
00fe5b9c: js     0x180fe5bbe
00fe5b9e: mov    r8, qword ptr [rdi + 0x38]
00fe5ba2: lea    rcx, [rsp + 0x40]
00fe5ba7: mov    edx, ebp
00fe5ba9: mov    r8, qword ptr [r8 + 0x10]
00fe5bad: call   0x181b4e780                              ; System.Int32$$CompareTo
00fe5bb2: test   eax, eax
00fe5bb4: cmovle ebp, dword ptr [rsp + 0x40]
00fe5bb9: mov    dword ptr [rbx + 0x10], ebp
00fe5bbc: jmp    0x180fe5bc1
00fe5bbe: mov    dword ptr [rbx + 0x10], esi
00fe5bc1: mov    rbx, qword ptr [rsp + 0x48]
00fe5bc6: mov    rbp, qword ptr [rsp + 0x50]
00fe5bcb: add    rsp, 0x20
00fe5bcf: pop    r14
00fe5bd1: pop    rdi
00fe5bd2: pop    rsi
00fe5bd3: ret    
00fe5bd4: int3   
