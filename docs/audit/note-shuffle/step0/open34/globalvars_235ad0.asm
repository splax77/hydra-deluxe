00235ad0: mov    rax, rsp
00235ad3: push   rbx
00235ad4: push   rsi
00235ad5: push   rdi
00235ad6: push   r12
00235ad8: push   r13
00235ada: push   r14
00235adc: push   r15
00235ade: sub    rsp, 0x240
00235ae5: movaps xmmword ptr [rax - 0x48], xmm6
00235ae9: movaps xmmword ptr [rax - 0x58], xmm7
00235aed: movaps xmmword ptr [rax - 0x68], xmm8
00235af2: movaps xmmword ptr [rax - 0x78], xmm9
00235af7: mov    r15, r9
00235afa: mov    rdi, r8
00235afd: mov    rbx, rdx
00235b00: mov    rsi, rcx
00235b03: cmp    byte ptr [rip + 0x3cd7a86], 0            ; [0x3f0d590] (bss)
00235b0a: jne    0x180235b4f
00235b0c: lea    rcx, [rip + 0x3aa283d]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00235b13: call   0x182f609b0
00235b18: lea    rcx, [rip + 0x3ac0e81]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
00235b1f: call   0x182f609b0
00235b24: lea    rcx, [rip + 0x3a680ad]                   ; [0x3c9dbd8] meta:ʾˁʸˁˁʻʿˁˁʲʽ_TypeInfo
00235b2b: call   0x182f609b0
00235b30: lea    rcx, [rip + 0x3a8db89]                   ; [0x3cc36c0] str:'Failed to load song object for '
00235b37: call   0x182f609b0
00235b3c: lea    rcx, [rip + 0x3a862bd]                   ; [0x3cbbe00] str:' - '
00235b43: call   0x182f609b0
00235b48: mov    byte ptr [rip + 0x3cd7a41], 1            ; [0x3f0d590] (bss)
00235b4f: xor    r14d, r14d
00235b52: mov    qword ptr [r15], r14
00235b55: xor    edx, edx
00235b57: mov    rcx, r15
00235b5a: call   0x182f5fc00
00235b5f: mov    r13, qword ptr [rsp + 0x2a0]
00235b67: mov    qword ptr [r13], r14
00235b6b: xor    edx, edx
00235b6d: mov    rcx, r13
00235b70: call   0x182f5fc00
00235b75: mov    r12, qword ptr [rsp + 0x2a8]
00235b7d: mov    qword ptr [r12], r14
00235b81: xor    edx, edx
00235b83: mov    rcx, r12
00235b86: call   0x182f5fc00
00235b8b: nop    
00235b8c: mov    qword ptr [rsi + 0x28], rbx
00235b90: lea    rcx, [rsi + 0x28]
00235b94: mov    rdx, rbx
00235b97: call   0x182f5fc00
00235b9c: mov    rcx, qword ptr [rsi + 0x30]
00235ba0: test   rcx, rcx
00235ba3: je     0x180235f1c
00235ba9: xor    r8d, r8d
00235bac: mov    rdx, rbx
00235baf: call   0x1801f0160                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʻʸʳʶˁʷʵʴʻʾʹ
00235bb4: test   rbx, rbx
00235bb7: je     0x180235f17
00235bbd: mov    qword ptr [rsp + 0x20], r14
00235bc2: xor    r9d, r9d
00235bc5: xor    r8d, r8d
00235bc8: xor    edx, edx
00235bca: mov    rcx, rbx
00235bcd: call   0x18035c110                              ; SongEntry$$ʿʲʷʹʿʳʾʳʷˀʾ
00235bd2: mov    rsi, rax
00235bd5: test   rax, rax
00235bd8: je     0x180235da8
00235bde: test   rdi, rdi
00235be1: je     0x180235f08
00235be7: mov    rcx, qword ptr [rdi + 0x10]
00235beb: test   rcx, rcx
00235bee: je     0x180235f03
00235bf4: movzx  ebx, byte ptr [rcx + 0x10]
00235bf8: movzx  eax, byte ptr [rcx + 0x11]
00235bfc: mov    byte ptr [rsp + 0x2a0], al
00235c03: xor    edx, edx
00235c05: mov    rcx, rdi
00235c08: call   0x180337ce0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʾˀʴˁʿˁˀʹˀʴʷ
00235c0d: movzx  edx, al
00235c10: mov    r9, qword ptr [rdi + 0x10]
00235c14: test   r9, r9
00235c17: je     0x180235efe
00235c1d: mov    rcx, qword ptr [r9 + 0x38]
00235c21: test   rcx, rcx
00235c24: je     0x180235ef9
00235c2a: mov    r9d, dword ptr [r9 + 0x14]
00235c2e: mov    ecx, dword ptr [rcx + 0x10]
00235c31: xor    eax, eax
00235c33: mov    qword ptr [rsp + 0x40], rax
00235c38: mov    dword ptr [rsp + 0x48], eax
00235c3c: test   ecx, ecx
00235c3e: setne  al
00235c41: mov    qword ptr [rsp + 0x30], r14
00235c46: mov    byte ptr [rsp + 0x28], al
00235c4a: mov    dword ptr [rsp + 0x20], r9d
00235c4f: movzx  r9d, dl
00235c53: movzx  r8d, byte ptr [rsp + 0x2a0]
00235c5c: movzx  edx, bl
00235c5f: lea    rcx, [rsp + 0x40]
00235c64: call   0x182164780                              ; ʼʹʿʳʾʷʲʹʹˁʽ$$.ctor
00235c69: movsd  xmm0, qword ptr [rsp + 0x40]
00235c6f: movsd  qword ptr [rsp + 0x80], xmm0
00235c78: mov    eax, dword ptr [rsp + 0x48]
00235c7c: mov    dword ptr [rsp + 0x88], eax
00235c83: mov    qword ptr [rsp + 0x20], r14
00235c88: mov    r9b, 1
00235c8b: mov    r8, rsi
00235c8e: lea    rdx, [rsp + 0x80]
00235c96: lea    rcx, [rsp + 0x160]
00235c9e: call   0x1820d2ea0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʹʹʾʴʳʶʼʵʳʼʼ
00235ca3: movups xmm6, xmmword ptr [rax + 0x40]
00235ca7: movups xmm9, xmmword ptr [rax + 0x50]
00235cac: mov    rdx, qword ptr [rax + 0x50]
00235cb0: test   rdx, rdx
00235cb3: je     0x180235ef4
00235cb9: xor    r8d, r8d
00235cbc: lea    rcx, [rsp + 0x50]
00235cc1: call   0x18212c9e0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʷʻʿʽʲʾˀˀʸʹʾ
00235cc6: movups xmm7, xmmword ptr [rax]
00235cc9: movups xmm8, xmmword ptr [rax + 0x10]
00235cce: mov    rcx, qword ptr [rip + 0x3a67f03]         ; [0x3c9dbd8] meta:ʾˁʸˁˁʻʿˁˁʲʽ_TypeInfo
00235cd5: cmp    dword ptr [rcx + 0xe0], r14d
00235cdc: jne    0x180235ce3
00235cde: call   0x182f60cf0
00235ce3: movaps xmmword ptr [rsp + 0x50], xmm7
00235ce8: movaps xmmword ptr [rsp + 0x60], xmm8
00235cee: xor    edx, edx
00235cf0: lea    rcx, [rsp + 0x50]
00235cf5: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
00235cfa: mov    qword ptr [r15], rax
00235cfd: mov    rdx, rax
00235d00: mov    rcx, r15
00235d03: call   0x182f5fc00
00235d08: movq   rdx, xmm9
00235d0d: test   rdx, rdx
00235d10: je     0x180235eef
00235d16: xor    r8d, r8d
00235d19: lea    rcx, [rsp + 0x140]
00235d21: call   0x18212da90                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʽʻʴʶʺˁʳʸʸʳʿ
00235d26: movups xmm0, xmmword ptr [rax]
00235d29: movaps xmmword ptr [rsp + 0x50], xmm0
00235d2e: movups xmm1, xmmword ptr [rax + 0x10]
00235d32: movaps xmmword ptr [rsp + 0x60], xmm1
00235d37: xor    edx, edx
00235d39: lea    rcx, [rsp + 0x50]
00235d3e: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
00235d43: mov    qword ptr [r13], rax
00235d47: mov    rdx, rax
00235d4a: mov    rcx, r13
00235d4d: call   0x182f5fc00
00235d52: psrldq xmm6, 8
00235d57: movq   rdx, xmm6
00235d5c: test   rdx, rdx
00235d5f: je     0x180235ee9
00235d65: xor    r8d, r8d
00235d68: lea    rcx, [rsp + 0x140]
00235d70: call   0x18215f280                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʽʳʸʿʷʽʼˁʸʻʼ
00235d75: movups xmm0, xmmword ptr [rax]
00235d78: movaps xmmword ptr [rsp + 0x50], xmm0
00235d7d: movups xmm1, xmmword ptr [rax + 0x10]
00235d81: movaps xmmword ptr [rsp + 0x60], xmm1
00235d86: xor    edx, edx
00235d88: lea    rcx, [rsp + 0x50]
00235d8d: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
00235d92: mov    qword ptr [r12], rax
00235d96: mov    rdx, rax
00235d99: mov    rcx, r12
00235d9c: call   0x182f5fc00
00235da1: mov    al, 1
00235da3: jmp    0x180235eb8
00235da8: xor    edx, edx
00235daa: mov    rcx, rbx
00235dad: call   0x1803600c0                              ; SongEntry$$get_Artist
00235db2: test   rax, rax
00235db5: je     0x180235f12
00235dbb: mov    rdi, qword ptr [rax + 0x10]
00235dbf: xor    edx, edx
00235dc1: mov    rcx, rbx
00235dc4: call   0x18035a5f0                              ; SongEntry$$ʷˁʼˁʴʸʾʻʿʿʿ
00235dc9: test   rax, rax
00235dcc: je     0x180235f0d
00235dd2: mov    qword ptr [rsp + 0x20], r14
00235dd7: mov    r9, qword ptr [rax + 0x10]
00235ddb: mov    r8, qword ptr [rip + 0x3a8601e]          ; [0x3cbbe00] str:' - '
00235de2: mov    rdx, rdi
00235de5: mov    rcx, qword ptr [rip + 0x3a8d8d4]         ; [0x3cc36c0] str:'Failed to load song object for '
00235dec: call   0x18197e8f0                              ; System.String$$Concat
00235df1: mov    rbx, rax
00235df4: mov    rcx, qword ptr [rip + 0x3aa2555]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00235dfb: cmp    dword ptr [rcx + 0xe0], 0
00235e02: jne    0x180235e09
00235e04: call   0x182f60cf0
00235e09: xor    edx, edx
00235e0b: mov    rcx, rbx
00235e0e: call   0x18285f400                              ; UnityEngine.Debug$$LogError
00235e13: xor    al, al
00235e15: jmp    0x180235eb8
00235e1a: mov    rbx, qword ptr [rsp + 0x2a0]
00235e22: test   rbx, rbx
00235e25: je     0x180235ee3
00235e2b: mov    rax, qword ptr [rbx]
00235e2e: mov    rdx, qword ptr [rax + 0x190]
00235e35: mov    rcx, rbx
00235e38: call   qword ptr [rax + 0x188]
00235e3e: mov    rsi, rax
00235e41: mov    r8, qword ptr [rbx]
00235e44: mov    rdx, qword ptr [r8 + 0x1d0]
00235e4b: mov    rcx, rbx
00235e4e: call   qword ptr [r8 + 0x1c8]
00235e55: mov    rdi, rax
00235e58: lea    rcx, [rip + 0x3a815f1]                   ; [0x3cb7450] str:'\n'
00235e5f: call   0x182f609d0
00235e64: mov    rbx, rax
00235e67: lea    rcx, [rip + 0x3a7ba7a]                   ; [0x3cb18e8] str:'Error loading song into memory: '
00235e6e: call   0x182f609d0
00235e73: xor    r14d, r14d
00235e76: mov    qword ptr [rsp + 0x20], r14
00235e7b: mov    r9, rdi
00235e7e: mov    r8, rbx
00235e81: mov    rdx, rsi
00235e84: mov    rcx, rax
00235e87: call   0x18197e8f0                              ; System.String$$Concat
00235e8c: mov    rbx, rax
00235e8f: lea    rcx, [rip + 0x3aa24ba]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00235e96: call   0x182f609d0
00235e9b: cmp    dword ptr [rax + 0xe0], r14d
00235ea2: jne    0x180235eac
00235ea4: mov    rcx, rax
00235ea7: call   0x182f60cf0
00235eac: xor    edx, edx
00235eae: mov    rcx, rbx
00235eb1: call   0x18285f400                              ; UnityEngine.Debug$$LogError
00235eb6: xor    al, al
00235eb8: lea    r11, [rsp + 0x240]
00235ec0: movaps xmm6, xmmword ptr [r11 - 0x10]
00235ec5: movaps xmm7, xmmword ptr [r11 - 0x20]
00235eca: movaps xmm8, xmmword ptr [r11 - 0x30]
00235ecf: movaps xmm9, xmmword ptr [r11 - 0x40]
00235ed4: mov    rsp, r11
00235ed7: pop    r15
00235ed9: pop    r14
00235edb: pop    r13
00235edd: pop    r12
00235edf: pop    rdi
00235ee0: pop    rsi
00235ee1: pop    rbx
00235ee2: ret    
00235ee3: call   0x182f60c50
00235ee8: nop    
00235ee9: call   0x182f60c50
00235eee: nop    
00235eef: call   0x182f60c50
00235ef4: call   0x182f60c50
00235ef9: call   0x182f60c50
00235efe: call   0x182f60c50
00235f03: call   0x182f60c50
00235f08: call   0x182f60c50
00235f0d: call   0x182f60c50
00235f12: call   0x182f60c50
00235f17: call   0x182f60c50
00235f1c: call   0x182f60c50
00235f21: int3   
00235f22: int3   
