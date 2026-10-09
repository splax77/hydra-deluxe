0215dbd0: mov    qword ptr [rsp + 0x18], r8
0215dbd5: push   rbp
0215dbd6: push   rbx
0215dbd7: push   rdi
0215dbd8: lea    rbp, [rsp - 0x47]
0215dbdd: sub    rsp, 0xa0
0215dbe4: cmp    byte ptr [rip + 0x1dbbb63], 0            ; [0x3f1974e] (bss)
0215dbeb: movzx  edi, dl
0215dbee: mov    rbx, rcx
0215dbf1: jne    0x18215dc4e
0215dbf3: lea    rcx, [rip + 0x1b74986]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215dbfa: call   0x182f609b0
0215dbff: lea    rcx, [rip + 0x1b4dc82]                   ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215dc06: call   0x182f609b0
0215dc0b: lea    rcx, [rip + 0x1b4d6b6]                   ; [0x3cab2c8] metamethod:Method$System.Span<ʸʻˁʴʿʶʶʳʸʶʳ>.get_Length()
0215dc12: call   0x182f609b0
0215dc17: lea    rcx, [rip + 0x1b4ddda]                   ; [0x3cab9f8] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.op_Implicit()
0215dc1e: call   0x182f609b0
0215dc23: lea    rcx, [rip + 0x1b3c8f6]                   ; [0x3c9a520] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʾˁʲʻʹʽʶʹʶʲʹ<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215dc2a: call   0x182f609b0
0215dc2f: lea    rcx, [rip + 0x1b93c3a]                   ; [0x3cf1870] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>()
0215dc36: call   0x182f609b0
0215dc3b: lea    rcx, [rip + 0x1b93f2e]                   ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215dc42: call   0x182f609b0
0215dc47: mov    byte ptr [rip + 0x1dbbb00], 1            ; [0x3f1974e] (bss)
0215dc4e: cmp    byte ptr [rip + 0x1dbbadd], 0            ; [0x3f19732] (bss)
0215dc55: mov    qword ptr [rbp + 0x67], 0
0215dc5d: jne    0x18215dc72
0215dc5f: lea    rcx, [rip + 0x1b47c22]                   ; [0x3ca5888] metamethod:Method$System.Collections.Generic.Dictionary<ʻˀʳʸʺʵʼʺʹˁʽ, List<ʸʻˁʴʿʶʶʳʸʶʳ>>.TryGetValue()
0215dc66: call   0x182f609b0
0215dc6b: mov    byte ptr [rip + 0x1dbbac0], 1            ; [0x3f19732] (bss)
0215dc72: mov    rcx, qword ptr [rbx + 0x98]
0215dc79: mov    qword ptr [rsp + 0xd8], rsi
0215dc81: mov    qword ptr [rsp + 0x98], r12
0215dc89: mov    qword ptr [rsp + 0x90], r13
0215dc91: mov    qword ptr [rsp + 0x88], r14
0215dc99: mov    qword ptr [rsp + 0x80], r15
0215dca1: test   rcx, rcx
0215dca4: je     0x18215dee8
0215dcaa: mov    r9, qword ptr [rip + 0x1b47bd7]          ; [0x3ca5888] metamethod:Method$System.Collections.Generic.Dictionary<ʻˀʳʸʺʵʼʺʹˁʽ, List<ʸʻˁʴʿʶʶʳʸʶʳ>>.TryGetValue()
0215dcb1: lea    r8, [rbp + 0x67]
0215dcb5: movzx  edx, dil
0215dcb9: call   0x18137df70                              ; System.Collections.Generic.Dictionary<ByteEnum, object>$$TryGetValue
0215dcbe: test   al, al
0215dcc0: je     0x18215deb5
0215dcc6: mov    rdi, qword ptr [rbx + 0x60]
0215dcca: mov    rbx, qword ptr [rip + 0x1b93e9f]         ; [0x3cf1b70] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215dcd1: cmp    qword ptr [rbx + 0x38], 0
0215dcd6: jne    0x18215dce0
0215dcd8: mov    rcx, rbx
0215dcdb: call   0x182f657d0
0215dce0: mov    r8, qword ptr [rbx + 0x38]
0215dce4: lea    rcx, [rbp - 0x39]
0215dce8: mov    rdx, rdi
0215dceb: mov    r8, qword ptr [r8 + 8]
0215dcef: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0215dcf4: mov    rbx, qword ptr [rip + 0x1b93b75]         ; [0x3cf1870] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>()
0215dcfb: mov    r13, qword ptr [rbp - 0x39]
0215dcff: mov    r12d, dword ptr [rbp - 0x31]
0215dd03: mov    rdi, qword ptr [rbp + 0x67]
0215dd07: cmp    qword ptr [rbx + 0x38], 0
0215dd0c: jne    0x18215dd16
0215dd0e: mov    rcx, rbx
0215dd11: call   0x182f657d0
0215dd16: mov    r8, qword ptr [rbx + 0x38]
0215dd1a: lea    rcx, [rbp + 7]
0215dd1e: mov    rdx, rdi
0215dd21: mov    r8, qword ptr [r8 + 8]
0215dd25: call   0x180462bf0                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>
0215dd2a: mov    ecx, dword ptr [rbp + 0xf]
0215dd2d: xor    eax, eax
0215dd2f: mov    dword ptr [rbp + 0x6f], eax
0215dd32: test   ecx, ecx
0215dd34: jle    0x18215deb5
0215dd3a: mov    edi, dword ptr [rbp - 0x2d]
0215dd3d: nop    dword ptr [rax]
0215dd40: cmp    eax, ecx
0215dd42: jae    0x18215deee
0215dd48: mov    r15d, eax
0215dd4b: shl    r15, 5
0215dd4f: add    r15, qword ptr [rbp + 7]
0215dd53: mov    r14, qword ptr [r15]
0215dd56: mov    rbx, qword ptr [r15 + 8]
0215dd5a: xor    edx, edx
0215dd5c: mov    rcx, r15
0215dd5f: call   0x18213e310                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$ʳʼʹʾʶʷʾˁʽʿʾ
0215dd64: mov    r8, qword ptr [rip + 0x1b4dc8d]          ; [0x3cab9f8] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.op_Implicit()
0215dd6b: lea    rsi, [rbx - 1]
0215dd6f: test   rax, rax
0215dd72: mov    qword ptr [rbp - 0x19], r13
0215dd76: lea    rdx, [rbp - 0x19]
0215dd7a: mov    dword ptr [rbp - 0x11], r12d
0215dd7e: lea    rcx, [rbp + 0x17]
0215dd82: mov    dword ptr [rbp - 0xd], edi
0215dd85: cmove  rsi, rbx
0215dd89: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
0215dd8e: movaps xmm0, xmmword ptr [rbp + 0x17]
0215dd92: lea    rcx, [rbp - 0x39]
0215dd96: mov    r9, qword ptr [rip + 0x1b3c783]          ; [0x3c9a520] metamethod:Method$ʽˁʷʳʸʶʳʵʸʾʿ.ʾˁʲʻʹʽʶʹʶʲʹ<ʾʳʿʽʸʸʷʾʼʴʿ>()
0215dd9d: mov    r8, rsi
0215dda0: mov    rdx, r14
0215dda3: movdqa xmmword ptr [rbp - 0x39], xmm0
0215dda8: call   0x1805fad90                              ; ʽˁʷʳʸʶʳʵʸʾʿ$$ʾˁʲʻʹʽʶʹʶʲʹ<object>
0215ddad: mov    rbx, rax
0215ddb0: cmp    eax, -1
0215ddb3: je     0x18215dea2
0215ddb9: mov    rsi, rax
0215ddbc: shr    rsi, 0x20
0215ddc0: cmp    esi, -1
0215ddc3: je     0x18215dea2
0215ddc9: mov    r14, qword ptr [rbp + 0x77]
0215ddcd: test   r14, r14
0215ddd0: jne    0x18215de30
0215ddd2: cmp    ebx, esi
0215ddd4: jg     0x18215dea2
0215ddda: nop    word ptr [rax + rax]
0215dde0: cmp    ebx, r12d
0215dde3: jae    0x18215deee
0215dde9: movsxd rax, ebx
0215ddec: mov    rax, qword ptr [r13 + rax*8]
0215ddf1: test   rax, rax
0215ddf4: je     0x18215dee8
0215ddfa: mov    rcx, qword ptr [rax + 0x38]
0215ddfe: test   rcx, rcx
0215de01: je     0x18215dee8
0215de07: movups xmm0, xmmword ptr [r15]
0215de0b: mov    r8, qword ptr [rip + 0x1b7476e]          ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215de12: lea    rdx, [rbp - 0x39]
0215de16: movups xmm1, xmmword ptr [r15 + 0x10]
0215de1b: movaps xmmword ptr [rbp - 0x39], xmm0
0215de1f: movaps xmmword ptr [rbp - 0x29], xmm1
0215de23: call   0x182c8dc80
0215de28: inc    ebx
0215de2a: cmp    ebx, esi
0215de2c: jle    0x18215dde0
0215de2e: jmp    0x18215dea2
0215de30: mov    rax, qword ptr [rip + 0x1b4da51]         ; [0x3cab888] metamethod:Method$System.Span<ʾʳʿʽʸʸʷʾʼʴʿ>.Slice()
0215de37: sub    esi, ebx
0215de39: inc    esi
0215de3b: mov    qword ptr [rbp - 0x39], rax
0215de3f: cmp    ebx, r12d
0215de42: ja     0x18215de4d
0215de44: mov    eax, r12d
0215de47: sub    eax, ebx
0215de49: cmp    esi, eax
0215de4b: jbe    0x18215de54
0215de4d: xor    ecx, ecx
0215de4f: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
0215de54: mov    rcx, qword ptr [rbp - 0x39]
0215de58: movsxd rax, ebx
0215de5b: mov    dword ptr [rbp + 3], 0
0215de62: mov    rcx, qword ptr [rcx + 0x20]
0215de66: lea    rbx, [rax*8]
0215de6e: add    rbx, r13
0215de71: test   byte ptr [rcx + 0x135], 1
0215de78: jne    0x18215de7f
0215de7a: call   0x182f65750
0215de7f: mov    r9, qword ptr [r14 + 0x28]
0215de83: lea    rdx, [rbp - 0x39]
0215de87: mov    rcx, qword ptr [r14 + 0x40]
0215de8b: mov    r8, r15
0215de8e: mov    qword ptr [rbp - 9], rbx
0215de92: mov    dword ptr [rbp - 1], esi
0215de95: movaps xmm0, xmmword ptr [rbp - 9]
0215de99: movdqa xmmword ptr [rbp - 0x39], xmm0
0215de9e: call   qword ptr [r14 + 0x18]
0215dea2: mov    eax, dword ptr [rbp + 0x6f]
0215dea5: mov    ecx, dword ptr [rbp + 0xf]
0215dea8: inc    eax
0215deaa: mov    dword ptr [rbp + 0x6f], eax
0215dead: cmp    eax, ecx
0215deaf: jl     0x18215dd42
0215deb5: mov    r15, qword ptr [rsp + 0x80]
0215debd: mov    r14, qword ptr [rsp + 0x88]
0215dec5: mov    r13, qword ptr [rsp + 0x90]
0215decd: mov    r12, qword ptr [rsp + 0x98]
0215ded5: mov    rsi, qword ptr [rsp + 0xd8]
0215dedd: add    rsp, 0xa0
0215dee4: pop    rdi
0215dee5: pop    rbx
0215dee6: pop    rbp
0215dee7: ret    
0215dee8: call   0x182f60c50
0215deed: int3   
0215deee: call   0x182f60c40
0215def3: int3   
0215def4: int3   
