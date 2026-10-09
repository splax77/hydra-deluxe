0215e020: push   rbx
0215e022: push   rbp
0215e023: push   rsi
0215e024: push   rdi
0215e025: push   r14
0215e027: push   r15
0215e029: sub    rsp, 0x28
0215e02d: cmp    byte ptr [rip + 0x1dbb70c], 0            ; [0x3f19740] (bss)
0215e034: mov    r15, rdx
0215e037: jne    0x18215e04c
0215e039: lea    rcx, [rip + 0x1b4d900]                   ; [0x3cab940] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Length()
0215e040: call   0x182f609b0
0215e045: mov    byte ptr [rip + 0x1dbb6f4], 1            ; [0x3f19740] (bss)
0215e04c: mov    r14d, dword ptr [r15 + 8]
0215e050: cmp    r14d, 1
0215e054: jle    0x18215e104
0215e05a: mov    ebp, dword ptr [r15 + 8]
0215e05e: xor    r10b, r10b
0215e061: mov    ebx, 1
0215e066: nop    word ptr [rax + rax]
0215e070: lea    eax, [rbx - 1]
0215e073: cmp    eax, ebp
0215e075: jae    0x18215e117
0215e07b: cmp    ebx, ebp
0215e07d: jae    0x18215e117
0215e083: mov    rax, qword ptr [r15]
0215e086: mov    ecx, ebx
0215e088: mov    rdx, qword ptr [rax + rcx*8 - 8]
0215e08d: lea    r9, [rax + rcx*8]
0215e091: test   rdx, rdx
0215e094: je     0x18215e111
0215e096: mov    ecx, ebx
0215e098: lea    rsi, [rax + rcx*8]
0215e09c: mov    rax, qword ptr [rax + rcx*8]
0215e0a0: test   rax, rax
0215e0a3: je     0x18215e111
0215e0a5: mov    eax, dword ptr [rax + 0x20]
0215e0a8: cmp    dword ptr [rdx + 0x20], eax
0215e0ab: jne    0x18215e0b8
0215e0ad: mov    rax, qword ptr [rsi]
0215e0b0: mov    ecx, dword ptr [rax + 0x24]
0215e0b3: cmp    dword ptr [rdx + 0x24], ecx
0215e0b6: jg     0x18215e0c7
0215e0b8: mov    rax, qword ptr [rsi]
0215e0bb: mov    r8, rdx
0215e0be: mov    edx, dword ptr [rax + 0x20]
0215e0c1: cmp    dword ptr [r8 + 0x20], edx
0215e0c5: jle    0x18215e0ec
0215e0c7: mov    rdx, qword ptr [rsi]
0215e0ca: lea    rcx, [r9 - 8]
0215e0ce: mov    rdi, qword ptr [r9 - 8]
0215e0d2: mov    qword ptr [r9 - 8], rdx
0215e0d6: call   0x182f5fc00
0215e0db: mov    rdx, rdi
0215e0de: mov    qword ptr [rsi], rdi
0215e0e1: mov    rcx, rsi
0215e0e4: call   0x182f5fc00
0215e0e9: mov    r10b, 1
0215e0ec: inc    ebx
0215e0ee: cmp    ebx, r14d
0215e0f1: jl     0x18215e070
0215e0f7: test   r10b, r10b
0215e0fa: je     0x18215e104
0215e0fc: dec    r14d
0215e0ff: jmp    0x18215e050
0215e104: add    rsp, 0x28
0215e108: pop    r15
0215e10a: pop    r14
0215e10c: pop    rdi
0215e10d: pop    rsi
0215e10e: pop    rbp
0215e10f: pop    rbx
0215e110: ret    
0215e111: call   0x182f60c50
0215e116: int3   
0215e117: call   0x182f60c40
0215e11c: int3   
0215e11d: int3   
