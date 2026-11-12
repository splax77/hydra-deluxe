00365f10: push   rbx
00365f12: sub    rsp, 0x20
00365f16: cmp    byte ptr [rip + 0x3ba81ad], 0            ; [0x3f0e0ca] (bss)
00365f1d: mov    rbx, rcx
00365f20: jne    0x180365f65
00365f22: lea    rcx, [rip + 0x3972427]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00365f29: call   0x182f609b0
00365f2e: lea    rcx, [rip + 0x392fc43]                   ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00365f35: call   0x182f609b0
00365f3a: lea    rcx, [rip + 0x3932847]                   ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
00365f41: call   0x182f609b0
00365f46: lea    rcx, [rip + 0x395cc33]                   ; [0x3cc2b80] str:'Failed to get audio position {0}'
00365f4d: call   0x182f609b0
00365f52: lea    rcx, [rip + 0x395b067]                   ; [0x3cc0fc0] str:'Failed to convert audio position {0}'
00365f59: call   0x182f609b0
00365f5e: mov    byte ptr [rip + 0x3ba8165], 1            ; [0x3f0e0ca] (bss)
00365f65: cmp    byte ptr [rbx + 0x28], 0
00365f69: jne    0x180365f74
00365f6b: xorps  xmm0, xmm0
00365f6e: add    rsp, 0x20
00365f72: pop    rbx
00365f73: ret    
00365f74: mov    rcx, qword ptr [rip + 0x392fbfd]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00365f7b: mov    qword ptr [rsp + 0x40], rdi
00365f80: mov    edi, dword ptr [rbx + 0x58]
00365f83: cmp    dword ptr [rcx + 0xe0], 0
00365f8a: jne    0x180365f91
00365f8c: call   0x182f60cf0
00365f91: xor    r8d, r8d
00365f94: xor    edx, edx
00365f96: mov    ecx, edi
00365f98: call   0x180247cb0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ʷʺʽʲʵʵʹˀʹˁʷ
00365f9d: mov    rcx, qword ptr [rip + 0x392fbd4]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00365fa4: mov    rdi, rax
00365fa7: cmp    rax, -1
00365fab: je     0x180366032
00365fb1: mov    qword ptr [rsp + 0x38], rsi
00365fb6: cmp    dword ptr [rcx + 0xe0], 0
00365fbd: mov    esi, dword ptr [rbx + 0x58]
00365fc0: jne    0x180365fc7
00365fc2: call   0x182f60cf0
00365fc7: xor    r8d, r8d
00365fca: mov    rdx, rdi
00365fcd: mov    ecx, esi
00365fcf: call   0x18024dc30                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˀʵʺʼʾʷˀˀʿˁʿ
00365fd4: ucomisd xmm0, qword ptr [rip + 0x2d002a4]       ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
00365fdc: mov    rsi, qword ptr [rsp + 0x38]
00365fe1: jp     0x18036601f
00365fe3: jne    0x18036601f
00365fe5: mov    rcx, qword ptr [rip + 0x392fb8c]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00365fec: cmp    dword ptr [rcx + 0xe0], 0
00365ff3: jne    0x180365ffa
00365ff5: call   0x182f60cf0
00365ffa: xor    ecx, ecx
00365ffc: call   0x18024edf0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˁʼʿʶʳˀʼʾʳˁʲ
00366001: mov    rcx, qword ptr [rip + 0x3932780]         ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
00366008: lea    rdx, [rsp + 0x30]
0036600d: mov    dword ptr [rsp + 0x30], eax
00366011: call   0x182f5fbe0
00366016: mov    rcx, qword ptr [rip + 0x395afa3]         ; [0x3cc0fc0] str:'Failed to convert audio position {0}'
0036601d: jmp    0x180366063
0036601f: addsd  xmm0, qword ptr [rbx + 0x90]
00366027: mov    rdi, qword ptr [rsp + 0x40]
0036602c: add    rsp, 0x20
00366030: pop    rbx
00366031: ret    
00366032: cmp    dword ptr [rcx + 0xe0], 0
00366039: jne    0x180366040
0036603b: call   0x182f60cf0
00366040: xor    ecx, ecx
00366042: call   0x18024edf0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˁʼʿʶʳˀʼʾʳˁʲ
00366047: mov    rcx, qword ptr [rip + 0x393273a]         ; [0x3c98788] meta:ʹʾʻˀʹʸʹʴʹʶʳ_TypeInfo
0036604e: lea    rdx, [rsp + 0x30]
00366053: mov    dword ptr [rsp + 0x30], eax
00366057: call   0x182f5fbe0
0036605c: mov    rcx, qword ptr [rip + 0x395cb1d]         ; [0x3cc2b80] str:'Failed to get audio position {0}'
00366063: xor    r8d, r8d
00366066: mov    rdx, rax
00366069: call   0x181982590                              ; System.String$$Format
0036606e: mov    rcx, qword ptr [rip + 0x39722db]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00366075: mov    rbx, rax
00366078: cmp    dword ptr [rcx + 0xe0], 0
0036607f: jne    0x180366086
00366081: call   0x182f60cf0
00366086: xor    edx, edx
00366088: mov    rcx, rbx
0036608b: call   0x18285fb70                              ; UnityEngine.Debug$$LogWarning
00366090: mov    rdi, qword ptr [rsp + 0x40]
00366095: xorps  xmm0, xmm0
00366098: add    rsp, 0x20
0036609c: pop    rbx
0036609d: ret    
0036609e: int3   
