000d9b90: mov    qword ptr [rsp + 0x10], rbx
000d9b95: push   rdi
000d9b96: sub    rsp, 0x50
000d9b9a: cmp    byte ptr [rip + 0x3e3261f], 0            ; [0x3f0c1c0] (bss)
000d9ba1: mov    rdi, rcx
000d9ba4: jne    0x1800d9bd1
000d9ba6: lea    rcx, [rip + 0x3be22ab]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d9bad: call   0x182f609b0
000d9bb2: lea    rcx, [rip + 0x3c15cd7]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9bb9: call   0x182f609b0
000d9bbe: lea    rcx, [rip + 0x3c11813]                   ; [0x3ceb3d8] meta:Cysharp.Threading.Tasks.UniTask_TypeInfo
000d9bc5: call   0x182f609b0
000d9bca: mov    byte ptr [rip + 0x3e325ef], 1            ; [0x3f0c1c0] (bss)
000d9bd1: mov    ecx, dword ptr [rdi + 0x10]
000d9bd4: mov    rbx, qword ptr [rdi + 0x20]
000d9bd8: mov    qword ptr [rsp + 0x60], rsi
000d9bdd: movaps xmmword ptr [rsp + 0x40], xmm6
000d9be2: test   ecx, ecx
000d9be4: je     0x1800d9c0e
000d9be6: sub    ecx, 1
000d9be9: je     0x1800d9c02
000d9beb: xor    al, al
000d9bed: cmp    ecx, 1
000d9bf0: jne    0x1800d9def
000d9bf6: mov    dword ptr [rdi + 0x10], 0xffffffff
000d9bfd: jmp    0x1800d9def
000d9c02: mov    dword ptr [rdi + 0x10], 0xffffffff
000d9c09: jmp    0x1800d9d78
000d9c0e: mov    dword ptr [rdi + 0x10], 0xffffffff
000d9c15: test   rbx, rbx
000d9c18: je     0x1800d9ea3
000d9c1e: xorps  xmm6, xmm6
000d9c21: comisd xmm6, xmmword ptr [rbx + 0x70]
000d9c26: ja     0x1800d9e92
000d9c2c: mov    rax, qword ptr [rip + 0x3c15c5d]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9c33: cmp    dword ptr [rax + 0xe0], 0
000d9c3a: jne    0x1800d9c4b
000d9c3c: mov    rcx, rax
000d9c3f: call   0x182f60cf0
000d9c44: mov    rax, qword ptr [rip + 0x3c15c45]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9c4b: mov    rax, qword ptr [rax + 0xb8]
000d9c52: mov    rcx, qword ptr [rax + 8]
000d9c56: test   rcx, rcx
000d9c59: je     0x1800d9ea3
000d9c5f: cmp    byte ptr [rcx + 0x72], 0
000d9c63: jne    0x1800d9e92
000d9c69: movsd  xmm0, qword ptr [rbx + 0x88]
000d9c71: comisd xmm0, xmmword ptr [rbx + 0x70]
000d9c76: ja     0x1800d9e92
000d9c7c: movsd  xmm1, qword ptr [rip + 0x2f8a5b4]        ; [0x3064238] dbl=1.5 q=0x3ff8000000000000
000d9c84: movabs rax, 0x3ff8000000000000
000d9c8e: mov    qword ptr [rdi + 0x28], rax
000d9c92: movsd  xmm0, qword ptr [rbx + 0x118]
000d9c9a: subsd  xmm0, xmm1
000d9c9e: comisd xmm6, xmm0
000d9ca2: ja     0x1800d9e92
000d9ca8: movsd  xmm0, qword ptr [rbx + 0x118]
000d9cb0: subsd  xmm0, xmm1
000d9cb4: movsd  xmm1, qword ptr [rbx + 0x70]
000d9cb9: movsd  qword ptr [rbx + 0x118], xmm0
000d9cc1: subsd  xmm1, qword ptr [rdi + 0x28]
000d9cc6: movsd  qword ptr [rdi + 0x30], xmm1
000d9ccb: mov    rax, qword ptr [rip + 0x3be2186]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d9cd2: cmp    dword ptr [rax + 0xe0], 0
000d9cd9: jne    0x1800d9cea
000d9cdb: mov    rcx, rax
000d9cde: call   0x182f60cf0
000d9ce3: mov    rax, qword ptr [rip + 0x3be216e]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d9cea: mov    rcx, qword ptr [rbx + 0xb0]
000d9cf1: test   rcx, rcx
000d9cf4: je     0x1800d9ea3
000d9cfa: mov    rax, qword ptr [rax + 0xb8]
000d9d01: xor    r8d, r8d
000d9d04: movsd  xmm1, qword ptr [rbx + 0x118]
000d9d0c: mov    rsi, qword ptr [rax + 8]
000d9d10: call   0x18016c460                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʺʷʵʵˀʴʻʽʼʽʺ
000d9d15: test   rsi, rsi
000d9d18: je     0x1800d9ea3
000d9d1e: xor    r8d, r8d
000d9d21: movaps xmm1, xmm0
000d9d24: mov    rcx, rsi
000d9d27: call   0x180368d10                              ; BassAudioManager$$ʹʻˀʳʿʸʾʿʳʼʵ
000d9d2c: movss  xmm1, dword ptr [rip + 0x2f8a088]        ; [0x3063dbc] flt=1.0 i=1065353216
000d9d34: xor    r8d, r8d
000d9d37: mov    ecx, 0xc
000d9d3c: call   0x1801a6ef0                              ; ʹˁʾʼʼʹʸʹʿʺʽ$$ʾˁʶʵʺʸʽʾˀʸʸ
000d9d41: mov    rax, qword ptr [rbx + 0x38]
000d9d45: test   rax, rax
000d9d48: je     0x1800d9ea3
000d9d4e: cmp    byte ptr [rax + 0x60], 0
000d9d52: je     0x1800d9d71
000d9d54: cmp    byte ptr [rax + 0x29], 0
000d9d58: jne    0x1800d9d71
000d9d5a: movsd  xmm1, qword ptr [rbx + 0x118]
000d9d62: xor    r8d, r8d
000d9d65: cvtpd2ps xmm1, xmm1
000d9d69: mov    rcx, rax
000d9d6c: call   0x180184370                              ; SongBackground$$ʻʿʹʿʾʲʳʵʾʽʷ
000d9d71: mov    dword ptr [rdi + 0x38], 0
000d9d78: movss  xmm1, dword ptr [rdi + 0x38]
000d9d7d: movsd  xmm0, qword ptr [rdi + 0x28]
000d9d82: cvtps2pd xmm1, xmm1
000d9d85: comisd xmm0, xmm1
000d9d89: jbe    0x1800d9e04
000d9d8b: xor    ecx, ecx
000d9d8d: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
000d9d92: test   rbx, rbx
000d9d95: je     0x1800d9ea3
000d9d9b: movsd  xmm2, qword ptr [rbx + 0x70]
000d9da0: addss  xmm0, xmm0
000d9da4: xorps  xmm1, xmm1
000d9da7: lea    rcx, [rdi + 0x18]
000d9dab: xorps  xmm3, xmm3
000d9dae: xor    edx, edx
000d9db0: cvtss2sd xmm1, xmm0
000d9db4: cvtss2sd xmm3, xmm0
000d9db8: subsd  xmm2, xmm1
000d9dbc: movsd  xmm1, qword ptr [rbx + 0x78]
000d9dc1: subsd  xmm1, xmm3
000d9dc5: movsd  qword ptr [rbx + 0x70], xmm2
000d9dca: movsd  qword ptr [rbx + 0x78], xmm1
000d9dcf: addss  xmm0, dword ptr [rdi + 0x38]
000d9dd4: mov    qword ptr [rdi + 0x18], 0
000d9ddc: movss  dword ptr [rdi + 0x38], xmm0
000d9de1: call   0x182f5fc00
000d9de6: mov    dword ptr [rdi + 0x10], 1
000d9ded: mov    al, 1
000d9def: movaps xmm6, xmmword ptr [rsp + 0x40]
000d9df4: mov    rsi, qword ptr [rsp + 0x60]
000d9df9: mov    rbx, qword ptr [rsp + 0x68]
000d9dfe: add    rsp, 0x50
000d9e02: pop    rdi
000d9e03: ret    
000d9e04: test   rbx, rbx
000d9e07: je     0x1800d9ea9
000d9e0d: mov    rcx, qword ptr [rbx + 0xb0]
000d9e14: mov    rax, qword ptr [rdi + 0x30]
000d9e18: mov    qword ptr [rbx + 0x70], rax
000d9e1c: test   rcx, rcx
000d9e1f: je     0x1800d9ea9
000d9e25: movsd  xmm1, qword ptr [rbx + 0x118]
000d9e2d: xor    r8d, r8d
000d9e30: call   0x18016c540                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʽˀˀʹˁʵʼʴʶʿʼ
000d9e35: mov    rcx, qword ptr [rbx + 0xb0]
000d9e3c: test   rcx, rcx
000d9e3f: je     0x1800d9ea9
000d9e41: xor    edx, edx
000d9e43: call   0x18016c490                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʳʶʿʹˁʴʻʺʶʹ
000d9e48: xor    r8d, r8d
000d9e4b: movsd  qword ptr [rbx + 0x78], xmm0
000d9e50: mov    rdx, rbx
000d9e53: lea    rcx, [rsp + 0x30]
000d9e58: call   0x1800bea90                              ; GameManager$$ʽʷʲʾʷʽˁʾˁʹʷ
000d9e5d: mov    rcx, qword ptr [rip + 0x3c11574]         ; [0x3ceb3d8] meta:Cysharp.Threading.Tasks.UniTask_TypeInfo
000d9e64: lea    rdx, [rsp + 0x20]
000d9e69: movups xmm0, xmmword ptr [rax]
000d9e6c: movups xmmword ptr [rsp + 0x20], xmm0
000d9e71: call   0x182f5fbe0
000d9e76: lea    rcx, [rdi + 0x18]
000d9e7a: mov    qword ptr [rdi + 0x18], rax
000d9e7e: mov    rdx, rax
000d9e81: call   0x182f5fc00
000d9e86: mov    dword ptr [rdi + 0x10], 2
000d9e8d: jmp    0x1800d9ded
000d9e92: xor    edx, edx
000d9e94: mov    rcx, rbx
000d9e97: call   0x1800c04c0                              ; GameManager$$ˁʵʽʴʶʺʾʳʺʿʹ
000d9e9c: xor    al, al
000d9e9e: jmp    0x1800d9def
000d9ea3: call   0x182f60c50
000d9ea8: int3   
000d9ea9: call   0x182f60c50
000d9eae: int3   
000d9eaf: int3   
