0214d240: push   rbx
0214d242: push   rbp
0214d243: push   rsi
0214d244: push   rdi
0214d245: sub    rsp, 0x28
0214d249: cmp    byte ptr [rip + 0x1dcc3bf], 0            ; [0x3f1960f] (bss)
0214d250: mov    rdi, r9
0214d253: mov    esi, r8d
0214d256: mov    rbp, rdx
0214d259: mov    rbx, rcx
0214d25c: jne    0x18214d271
0214d25e: lea    rcx, [rip + 0x1b8549b]                   ; [0x3cd2700] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Clear()
0214d265: call   0x182f609b0
0214d26a: mov    byte ptr [rip + 0x1dcc39e], 1            ; [0x3f1960f] (bss)
0214d271: mov    rcx, qword ptr [rbx + 0x38]
0214d275: xor    edx, edx
0214d277: mov    eax, dword ptr [rsp + 0x70]
0214d27b: mov    qword ptr [rbx + 0x10], rbp
0214d27f: mov    qword ptr [rbx + 0x18], rdi
0214d283: mov    dword ptr [rbx + 0x24], eax
0214d286: mov    dword ptr [rbx + 0x20], esi
0214d289: mov    word ptr [rbx + 0x34], 0
0214d28f: mov    byte ptr [rbx + 0x36], 0
0214d293: mov    qword ptr [rbx + 0x28], rdx
0214d297: mov    dword ptr [rbx + 0x30], edx
0214d29a: test   rcx, rcx
0214d29d: je     0x18214d2b1
0214d29f: inc    dword ptr [rcx + 0x1c]
0214d2a2: mov    dword ptr [rcx + 0x18], edx
0214d2a5: mov    rax, rbx
0214d2a8: add    rsp, 0x28
0214d2ac: pop    rdi
0214d2ad: pop    rsi
0214d2ae: pop    rbp
0214d2af: pop    rbx
0214d2b0: ret    
0214d2b1: call   0x182f60c50
0214d2b6: int3   
0214d2b7: int3   
