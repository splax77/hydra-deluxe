020f43a0: mov    qword ptr [rsp + 8], rbx
020f43a5: mov    qword ptr [rsp + 0x10], rsi
020f43aa: push   rdi
020f43ab: sub    rsp, 0x40
020f43af: cmp    byte ptr [rip + 0x1e25008], 0            ; [0x3f193be] (bss)
020f43b6: mov    rdi, rcx
020f43b9: movaps xmmword ptr [rsp + 0x30], xmm6
020f43be: movaps xmm6, xmm1
020f43c1: jne    0x1820f43d6
020f43c3: lea    rcx, [rip + 0x1c1a00e]                   ; [0x3d0e3d8] metamethod:Method$System.Collections.Generic.List<double>.Add()
020f43ca: call   0x182f609b0
020f43cf: mov    byte ptr [rip + 0x1e24fe8], 1            ; [0x3f193be] (bss)
020f43d6: cmp    byte ptr [rip + 0x1e24fec], 0            ; [0x3f193c9] (bss)
020f43dd: jne    0x1820f43fe
020f43df: lea    rcx, [rip + 0x1bb5cfa]                   ; [0x3caa0e0] metamethod:Method$System.Span<double>.get_Length()
020f43e6: call   0x182f609b0
020f43eb: lea    rcx, [rip + 0x1bfce7e]                   ; [0x3cf1270] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<double>()
020f43f2: call   0x182f609b0
020f43f7: mov    byte ptr [rip + 0x1e24fcb], 1            ; [0x3f193c9] (bss)
020f43fe: mov    rbx, qword ptr [rip + 0x1bfce6b]         ; [0x3cf1270] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<double>()
020f4405: mov    rsi, qword ptr [rdi + 0x180]
020f440c: cmp    qword ptr [rbx + 0x38], 0
020f4411: jne    0x1820f441b
020f4413: mov    rcx, rbx
020f4416: call   0x182f657d0
020f441b: mov    r8, qword ptr [rbx + 0x38]
020f441f: lea    rcx, [rsp + 0x20]
020f4424: mov    rdx, rsi
020f4427: mov    r8, qword ptr [r8 + 8]
020f442b: call   0x180462bf0                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>
020f4430: mov    edx, dword ptr [rsp + 0x28]
020f4434: xor    ecx, ecx
020f4436: mov    r8, qword ptr [rsp + 0x20]
020f443b: test   edx, edx
020f443d: jle    0x1820f445c
020f443f: nop    
020f4440: cmp    ecx, edx
020f4442: jae    0x1820f44cd
020f4448: movsd  xmm0, qword ptr [r8 + rcx*8]
020f444e: ucomisd xmm0, xmm6
020f4452: jp     0x1820f4456
020f4454: je     0x1820f449c
020f4456: inc    ecx
020f4458: cmp    ecx, edx
020f445a: jl     0x1820f4442
020f445c: mov    rcx, qword ptr [rdi + 0x180]
020f4463: test   rcx, rcx
020f4466: je     0x1820f44c7
020f4468: mov    r9, qword ptr [rip + 0x1c19f69]          ; [0x3d0e3d8] metamethod:Method$System.Collections.Generic.List<double>.Add()
020f446f: inc    dword ptr [rcx + 0x1c]
020f4472: mov    rdx, qword ptr [rcx + 0x10]
020f4476: test   rdx, rdx
020f4479: je     0x1820f44c7
020f447b: movsxd r8, dword ptr [rcx + 0x18]
020f447f: cmp    r8d, dword ptr [rdx + 0x18]
020f4483: jb     0x1820f44b1
020f4485: mov    rax, qword ptr [r9 + 0x20]
020f4489: movaps xmm1, xmm6
020f448c: mov    r8, qword ptr [rax + 0xc0]
020f4493: mov    r8, qword ptr [r8 + 0x70]
020f4497: call   0x180746ea0                              ; System.Collections.Generic.List<double>$$AddWithResize
020f449c: mov    rbx, qword ptr [rsp + 0x50]
020f44a1: mov    rsi, qword ptr [rsp + 0x58]
020f44a6: movaps xmm6, xmmword ptr [rsp + 0x30]
020f44ab: add    rsp, 0x40
020f44af: pop    rdi
020f44b0: ret    
020f44b1: lea    eax, [r8 + 1]
020f44b5: mov    dword ptr [rcx + 0x18], eax
020f44b8: cmp    r8d, dword ptr [rdx + 0x18]
020f44bc: jae    0x1820f44cd
020f44be: movsd  qword ptr [rdx + r8*8 + 0x20], xmm6
020f44c5: jmp    0x1820f449c
020f44c7: call   0x182f60c50
020f44cc: int3   
020f44cd: call   0x182f60c40
020f44d2: int3   
020f44d3: int3   
