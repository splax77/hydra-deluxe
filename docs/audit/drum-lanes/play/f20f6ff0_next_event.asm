020f6ff0: mov    qword ptr [rsp + 0x18], rbp
020f6ff5: push   rsi
020f6ff6: sub    rsp, 0x70
020f6ffa: cmp    byte ptr [rip + 0x1e223bf], 0            ; [0x3f193c0] (bss)
020f7001: mov    rbp, rcx
020f7004: movaps xmmword ptr [rsp + 0x40], xmm8
020f700a: movaps xmm8, xmm1
020f700e: movaps xmmword ptr [rsp + 0x30], xmm9
020f7014: movaps xmm9, xmm2
020f7018: jne    0x1820f7051
020f701a: lea    rcx, [rip + 0x1c175e7]                   ; [0x3d0e608] metamethod:Method$System.Collections.Generic.List<double>.RemoveAt()
020f7021: call   0x182f609b0
020f7026: lea    rcx, [rip + 0x1c178cb]                   ; [0x3d0e8f8] metamethod:Method$System.Collections.Generic.List<double>.get_Count()
020f702d: call   0x182f609b0
020f7032: lea    rcx, [rip + 0x1c1797f]                   ; [0x3d0e9b8] metamethod:Method$System.Collections.Generic.List<double>.get_Item()
020f7039: call   0x182f609b0
020f703e: lea    rcx, [rip + 0x1bfa22b]                   ; [0x3cf1270] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<double>()
020f7045: call   0x182f609b0
020f704a: mov    byte ptr [rip + 0x1e2236f], 1            ; [0x3f193c0] (bss)
020f7051: mov    rsi, qword ptr [rbp + 0x180]
020f7058: test   rsi, rsi
020f705b: je     0x1820f7201
020f7061: cmp    dword ptr [rsi + 0x18], 0
020f7065: je     0x1820f71f7
020f706b: mov    qword ptr [rsp + 0x80], rbx
020f7073: mov    qword ptr [rsp + 0x88], rdi
020f707b: mov    rbx, qword ptr [rip + 0x1bfa1ee]         ; [0x3cf1270] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<double>()
020f7082: cmp    qword ptr [rbx + 0x38], 0
020f7087: jne    0x1820f7091
020f7089: mov    rcx, rbx
020f708c: call   0x182f657d0
020f7091: mov    r8, qword ptr [rbx + 0x38]
020f7095: lea    rcx, [rsp + 0x20]
020f709a: mov    rdx, rsi
020f709d: mov    r8, qword ptr [r8 + 8]
020f70a1: call   0x180462bf0                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>
020f70a6: cmp    byte ptr [rip + 0x1e222fc], 0            ; [0x3f193a9] (bss)
020f70ad: mov    rbx, qword ptr [rsp + 0x20]
020f70b2: mov    edi, dword ptr [rsp + 0x28]
020f70b6: jne    0x1820f70cb
020f70b8: lea    rcx, [rip + 0x1bb3021]                   ; [0x3caa0e0] metamethod:Method$System.Span<double>.get_Length()
020f70bf: call   0x182f609b0
020f70c4: mov    byte ptr [rip + 0x1e222de], 1            ; [0x3f193a9] (bss)
020f70cb: mov    r8d, edi
020f70ce: nop    
020f70d0: xor    r9b, r9b
020f70d3: mov    edx, 1
020f70d8: cmp    r8d, edx
020f70db: jle    0x1820f7134
020f70dd: nop    dword ptr [rax]
020f70e0: lea    eax, [rdx - 1]
020f70e3: cmp    eax, edi
020f70e5: jae    0x1820f7207
020f70eb: cmp    edx, edi
020f70ed: jae    0x1820f7207
020f70f3: mov    eax, edx
020f70f5: mov    ecx, edx
020f70f7: movsd  xmm0, qword ptr [rbx + rax*8]
020f70fc: comisd xmm0, xmmword ptr [rbx + rcx*8 - 8]
020f7102: jbe    0x1820f7123
020f7104: mov    eax, edx
020f7106: mov    r9b, 1
020f7109: mov    ecx, edx
020f710b: movsd  xmm0, qword ptr [rbx + rax*8 - 8]
020f7111: mov    eax, edx
020f7113: mov    rax, qword ptr [rbx + rax*8]
020f7117: mov    qword ptr [rbx + rcx*8 - 8], rax
020f711c: mov    eax, edx
020f711e: movsd  qword ptr [rbx + rax*8], xmm0
020f7123: inc    edx
020f7125: cmp    edx, r8d
020f7128: jl     0x1820f70e0
020f712a: test   r9b, r9b
020f712d: je     0x1820f7134
020f712f: dec    r8d
020f7132: jmp    0x1820f70d0
020f7134: mov    ebx, dword ptr [rsi + 0x18]
020f7137: sub    ebx, 1
020f713a: movaps xmmword ptr [rsp + 0x50], xmm7
020f713f: movsd  xmm7, qword ptr [rip + 0xf6f139]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f7147: js     0x1820f71c5
020f714d: movaps xmmword ptr [rsp + 0x60], xmm6
020f7152: mov    r8, qword ptr [rip + 0x1c1785f]          ; [0x3d0e9b8] metamethod:Method$System.Collections.Generic.List<double>.get_Item()
020f7159: mov    edx, ebx
020f715b: mov    rcx, rsi
020f715e: call   0x180754670                              ; System.Collections.Generic.List<double>$$get_Item
020f7163: comisd xmm0, xmm9
020f7168: movaps xmm6, xmm0
020f716b: jae    0x1820f71a6
020f716d: comisd xmm8, xmm0
020f7172: jae    0x1820f71a6
020f7174: comisd xmm0, xmm7
020f7178: jbe    0x1820f717d
020f717a: movaps xmm7, xmm0
020f717d: mov    r8, qword ptr [rip + 0x1c17484]          ; [0x3d0e608] metamethod:Method$System.Collections.Generic.List<double>.RemoveAt()
020f7184: mov    edx, ebx
020f7186: mov    rcx, rsi
020f7189: call   0x1807103c0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$RemoveAt
020f718e: xor    r9d, r9d
020f7191: xor    r8d, r8d
020f7194: movaps xmm1, xmm6
020f7197: mov    rcx, rbp
020f719a: call   0x1820f4550                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʺʴʻˀʺˀˁʶʻʷ
020f719f: sub    ebx, 1
020f71a2: jns    0x1820f7152
020f71a4: jmp    0x1820f71c0
020f71a6: ucomisd xmm6, xmm9
020f71ab: jp     0x1820f71c0
020f71ad: jne    0x1820f71c0
020f71af: mov    r8, qword ptr [rip + 0x1c17452]          ; [0x3d0e608] metamethod:Method$System.Collections.Generic.List<double>.RemoveAt()
020f71b6: mov    edx, ebx
020f71b8: mov    rcx, rsi
020f71bb: call   0x1807103c0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$RemoveAt
020f71c0: movaps xmm6, xmmword ptr [rsp + 0x60]
020f71c5: mov    rbx, qword ptr [rsp + 0x80]
020f71cd: movaps xmm0, xmm7
020f71d0: movaps xmm7, xmmword ptr [rsp + 0x50]
020f71d5: mov    rdi, qword ptr [rsp + 0x88]
020f71dd: mov    rbp, qword ptr [rsp + 0x90]
020f71e5: movaps xmm8, xmmword ptr [rsp + 0x40]
020f71eb: movaps xmm9, xmmword ptr [rsp + 0x30]
020f71f1: add    rsp, 0x70
020f71f5: pop    rsi
020f71f6: ret    
020f71f7: movsd  xmm0, qword ptr [rip + 0xf6f081]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f71ff: jmp    0x1820f71dd
020f7201: call   0x182f60c50
020f7206: int3   
020f7207: call   0x182f60c40
020f720c: int3   
020f720d: int3   
