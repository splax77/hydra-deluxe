020deaa0: mov    qword ptr [rsp + 0x20], rbx
020deaa5: push   rbp
020deaa6: push   rdi
020deaa7: push   r12
020deaa9: lea    rbp, [rsp - 0x47]
020deaae: sub    rsp, 0xc0
020deab5: cmp    byte ptr [rip + 0x1e3a7fa], 0            ; [0x3f192b6] (bss)
020deabc: mov    rdi, rcx
020deabf: jne    0x1820deb04
020deac1: lea    rcx, [rip + 0x1bccc50]                   ; [0x3cab718] metamethod:Method$System.Span<ʽʿʸʸʾʶʶʾʶʹʲ>.op_Implicit()
020deac8: call   0x182f609b0
020deacd: lea    rcx, [rip + 0x1c17bdc]                   ; [0x3cf66b0] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʶʼʵʲʸʳʻʴʴʹʹ()
020dead4: call   0x182f609b0
020dead9: lea    rcx, [rip + 0x1c17c88]                   ; [0x3cf6768] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʼˀʸʹʴʻʾʶʼʷˁ()
020deae0: call   0x182f609b0
020deae5: lea    rcx, [rip + 0x1c17d3c]                   ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020deaec: call   0x182f609b0
020deaf1: lea    rcx, [rip + 0x1bbc4b0]                   ; [0x3c9afa8] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʹʻʻˁʳʿʹʲʻʷʴ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020deaf8: call   0x182f609b0
020deafd: mov    byte ptr [rip + 0x1e3a7b2], 1            ; [0x3f192b6] (bss)
020deb04: xor    eax, eax
020deb06: mov    qword ptr [rsp + 0xe0], rsi
020deb0e: xor    r12d, r12d
020deb11: mov    qword ptr [rsp + 0xe8], r14
020deb19: xorps  xmm0, xmm0
020deb1c: xorps  xmm1, xmm1
020deb1f: mov    ebx, r12d
020deb22: mov    qword ptr [rsp + 0xf0], r15
020deb2a: movups xmmword ptr [rbp - 0x19], xmm0
020deb2e: mov    qword ptr [rbp - 9], rax
020deb32: movups xmmword ptr [rbp + 0x17], xmm1
020deb36: mov    qword ptr [rbp + 0x27], rax
020deb3a: movups xmmword ptr [rbp - 0x31], xmm0
020deb3e: mov    qword ptr [rbp - 0x21], rax
020deb42: movups xmmword ptr [rbp + 0x2f], xmm1
020deb46: mov    qword ptr [rbp + 0x3f], rax
020deb4a: cmp    dword ptr [rdi + 0x8c], eax
020deb50: jle    0x1820dee31
020deb56: mov    r8, qword ptr [rip + 0x1bccbbb]          ; [0x3cab718] metamethod:Method$System.Span<ʽʿʸʸʾʶʶʾʶʹʲ>.op_Implicit()
020deb5d: lea    rcx, [rbp + 7]
020deb61: mov    rdx, qword ptr [rdi + 0x58]
020deb65: mov    r14d, r12d
020deb68: call   0x1809cf5d0                              ; System.Span<object>$$op_Implicit
020deb6d: mov    r9d, dword ptr [rdi + 0x8c]
020deb74: lea    rdx, [rbp + 7]
020deb78: movaps xmm0, xmmword ptr [rbp + 7]
020deb7c: lea    rcx, [rbp - 0x49]
020deb80: mov    rax, qword ptr [rip + 0x1bbc421]         ; [0x3c9afa8] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʹʻʻˁʳʿʹʲʻʷʴ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020deb87: dec    r9d
020deb8a: xor    r8d, r8d
020deb8d: movdqa xmmword ptr [rbp + 7], xmm0
020deb92: mov    qword ptr [rsp + 0x20], rax
020deb97: call   0x1805fb930                              ; ʿʲˁʽʹʺʶʳˁʾʼ$$ʹʻʻˁʳʿʹʲʻʷʴ<object>
020deb9c: movups xmm0, xmmword ptr [rbp - 0x49]
020deba0: lea    rdx, [rbp + 0x17]
020deba4: mov    r8, qword ptr [rip + 0x1c17bbd]          ; [0x3cf6768] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʼˀʸʹʴʻʾʶʼʷˁ()
020debab: movsd  xmm1, qword ptr [rbp - 0x39]
020debb0: lea    rcx, [rbp - 0x49]
020debb4: movups xmmword ptr [rbp + 0x17], xmm0
020debb8: movsd  qword ptr [rbp + 0x27], xmm1
020debbd: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020debc2: movups xmm0, xmmword ptr [rbp - 0x49]
020debc6: lea    rcx, [rbp - 0x19]
020debca: mov    rdx, qword ptr [rip + 0x1c17c57]         ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020debd1: movsd  xmm1, qword ptr [rbp - 0x39]
020debd6: movups xmmword ptr [rbp - 0x19], xmm0
020debda: movsd  qword ptr [rbp - 9], xmm1
020debdf: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
020debe4: test   al, al
020debe6: je     0x1820dee31
020debec: nop    dword ptr [rax]
020debf0: mov    rdx, qword ptr [rip + 0x1c17ab9]         ; [0x3cf66b0] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʶʼʵʲʸʳʻʴʴʹʹ()
020debf7: lea    rcx, [rbp - 0x19]
020debfb: call   0x180fd06f0                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ʽʼʵʶʳʹʻʵˁʳˀ<object>$$ʶʼʵʲʸʳʻʴʴʹʹ
020dec00: mov    rdx, qword ptr [rax]
020dec03: test   rdx, rdx
020dec06: je     0x1820ded6b
020dec0c: xor    r8d, r8d
020dec0f: lea    rcx, [rbp - 0x49]
020dec13: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020dec18: xor    r8d, r8d
020dec1b: lea    rdx, [rbp + 0x2f]
020dec1f: lea    rcx, [rbp - 0x49]
020dec23: movups xmm0, xmmword ptr [rax]
020dec26: movups xmmword ptr [rbp + 0x2f], xmm0
020dec2a: movsd  xmm1, qword ptr [rax + 0x10]
020dec2f: movsd  qword ptr [rbp + 0x3f], xmm1
020dec34: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020dec39: xor    edx, edx
020dec3b: lea    rcx, [rbp - 0x31]
020dec3f: movups xmm0, xmmword ptr [rax]
020dec42: movups xmmword ptr [rbp - 0x31], xmm0
020dec46: movsd  xmm1, qword ptr [rax + 0x10]
020dec4b: movsd  qword ptr [rbp - 0x21], xmm1
020dec50: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020dec55: test   al, al
020dec57: je     0x1820ded6b
020dec5d: nop    dword ptr [rax]
020dec60: mov    rsi, qword ptr [rbp - 0x29]
020dec64: test   rsi, rsi
020dec67: je     0x1820dee81
020dec6d: xor    edx, edx
020dec6f: mov    rcx, rsi
020dec72: call   0x18214b6d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵʹʶʽʺʽʿʸʶʳʹ
020dec77: test   al, al
020dec79: jne    0x1820ded58
020dec7f: test   dword ptr [rsi + 0x7c], 0x800
020dec86: cmovne r14, rsi
020dec8a: movzx  eax, bx
020dec8d: xor    ax, word ptr [rdi + 0x152]
020dec94: and    ax, word ptr [rsi + 0x80]
020dec9b: test   word ptr [rdi + 0x152], ax
020deca2: je     0x1820ded58
020deca8: or     bx, word ptr [rsi + 0x80]
020decaf: cmp    dword ptr [rdi + 0x190], 1
020decb6: movzx  r15d, bx
020decba: je     0x1820decea
020decbc: xor    edx, edx
020decbe: mov    rcx, rdi
020decc1: call   0x1820f57f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʿʺʺʽʲʿʶˀʸʷ
020decc6: xor    edx, edx
020decc8: mov    rcx, rsi
020deccb: movzx  ebx, al
020decce: call   0x18214bb50                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʾʿʺʺʿʸʶʴʾʺʶ
020decd3: test   bl, al
020decd5: je     0x1820decea
020decd7: mov    rax, qword ptr [rdi]
020decda: mov    rcx, rdi
020decdd: mov    rdx, qword ptr [rax + 0x220]
020dece4: call   qword ptr [rax + 0x218]
020decea: test   dword ptr [rsi + 0x7c], 0xc000
020decf1: je     0x1820ded3e
020decf3: mov    qword ptr [rdi + 0x2e8], r12
020decfa: movzx  ecx, word ptr [rsi + 0x80]
020ded01: mov    rax, qword ptr [rdi + 0x100]
020ded08: not    cx
020ded0b: and    word ptr [rdi + 0x2c6], cx
020ded12: mov    qword ptr [rdi + 0x2d0], rax
020ded19: jne    0x1820ded30
020ded1b: mov    qword ptr [rdi + 0x2e0], rax
020ded22: movzx  eax, word ptr [rdi + 0x2c4]
020ded29: mov    word ptr [rdi + 0x2c6], ax
020ded30: xor    r8d, r8d
020ded33: mov    rdx, rsi
020ded36: mov    rcx, rdi
020ded39: call   0x1820de420                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʼʶʵˀʳʳʾʳʹʺʹ
020ded3e: mov    rax, qword ptr [rdi]
020ded41: mov    rdx, rsi
020ded44: mov    rcx, rdi
020ded47: movzx  ebx, r15w
020ded4b: mov    r8, qword ptr [rax + 0x1f0]
020ded52: call   qword ptr [rax + 0x1e8]
020ded58: xor    edx, edx
020ded5a: lea    rcx, [rbp - 0x31]
020ded5e: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020ded63: test   al, al
020ded65: jne    0x1820dec60
020ded6b: mov    rdx, qword ptr [rip + 0x1c17ab6]         ; [0x3cf6828] metamethod:Method$ʿʲˁʽʹʺʶʳˁʾʼ.ʶʵʸʾʼʺʽʳʿʵʿ<ʽʿʸʸʾʶʶʾʶʹʲ>.ʿʵʷʻʵʻʷʵʶʺʼ()
020ded72: lea    rcx, [rbp - 0x19]
020ded76: call   0x180fd0660                              ; ʿʲˁʽʹʺʶʳˁʾʼ.ˀʺʲʿʾʶʸʺʶʽʽ<__Il2CppFullySharedGenericType>$$ʷʽʸˁʴʵʴʴʵʷʲ
020ded7b: test   al, al
020ded7d: jne    0x1820debf0
020ded83: test   r14, r14
020ded86: je     0x1820dee31
020ded8c: xor    edx, edx
020ded8e: mov    rcx, r14
020ded91: call   0x18214b610                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʲʺʾʽʽʹʳʳʼʹ
020ded96: test   al, al
020ded98: jne    0x1820dee31
020ded9e: xor    edx, edx
020deda0: mov    rcx, rdi
020deda3: call   0x1820f57f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʿʺʺʽʲʿʶˀʸʷ
020deda8: test   al, al
020dedaa: je     0x1820dee31
020dedb0: cmp    word ptr [rdi + 0x152], bx
020dedb7: je     0x1820dee55
020dedbd: cmp    byte ptr [rdi + 0x40], r12b
020dedc1: je     0x1820dee31
020dedc3: test   dword ptr [r14 + 0x7c], 0xc000
020dedcb: je     0x1820dee19
020dedcd: mov    qword ptr [rdi + 0x2e8], r12
020dedd4: movzx  ecx, word ptr [r14 + 0x80]
020deddc: mov    rax, qword ptr [rdi + 0x100]
020dede3: not    cx
020dede6: and    word ptr [rdi + 0x2c6], cx
020deded: mov    qword ptr [rdi + 0x2d0], rax
020dedf4: jne    0x1820dee0b
020dedf6: mov    qword ptr [rdi + 0x2e0], rax
020dedfd: movzx  eax, word ptr [rdi + 0x2c4]
020dee04: mov    word ptr [rdi + 0x2c6], ax
020dee0b: xor    r8d, r8d
020dee0e: mov    rdx, r14
020dee11: mov    rcx, rdi
020dee14: call   0x1820de420                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʼʶʵˀʳʳʾʳʹʺʹ
020dee19: mov    rax, qword ptr [rdi]
020dee1c: mov    rdx, r14
020dee1f: mov    rcx, rdi
020dee22: mov    r8, qword ptr [rax + 0x1f0]
020dee29: call   qword ptr [rax + 0x1e8]
020dee2f: jmp    0x1820dee55
020dee31: cmp    word ptr [rdi + 0x152], bx
020dee38: je     0x1820dee55
020dee3a: cmp    byte ptr [rdi + 0x40], r12b
020dee3e: je     0x1820dee55
020dee40: xor    bx, word ptr [rdi + 0x152]
020dee47: xor    r8d, r8d
020dee4a: movzx  edx, bx
020dee4d: mov    rcx, rdi
020dee50: call   0x1820de9a0                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʾˁʻʽʺʷʵʹʹʽʺ
020dee55: mov    r15, qword ptr [rsp + 0xf0]
020dee5d: mov    r14, qword ptr [rsp + 0xe8]
020dee65: mov    rsi, qword ptr [rsp + 0xe0]
020dee6d: mov    rbx, qword ptr [rsp + 0xf8]
020dee75: add    rsp, 0xc0
020dee7c: pop    r12
020dee7e: pop    rdi
020dee7f: pop    rbp
020dee80: ret    
020dee81: call   0x182f60c50
020dee86: int3   
020dee87: int3   
