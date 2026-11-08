001f0160: mov    qword ptr [rsp + 0x10], rbx
001f0165: push   rdi
001f0166: sub    rsp, 0x30
001f016a: cmp    byte ptr [rip + 0x3d1c9e4], 0            ; [0x3f0cb55] (bss)
001f0171: mov    rdi, rdx
001f0174: mov    rbx, rcx
001f0177: jne    0x1801f018c
001f0179: lea    rcx, [rip + 0x3aab518]                   ; [0x3c9b698] meta:ʼʿʺʿʹʳʹʶʹʹʼ_TypeInfo
001f0180: call   0x182f609b0
001f0185: mov    byte ptr [rip + 0x3d1c9c9], 1            ; [0x3f0cb55] (bss)
001f018c: cmp    qword ptr [rbx + 0x48], rdi
001f0190: je     0x1801f027b
001f0196: mov    qword ptr [rsp + 0x40], rsi
001f019b: lea    rcx, [rbx + 0x48]
001f019f: mov    qword ptr [rbx + 0x48], rdi
001f01a3: mov    rdx, rdi
001f01a6: call   0x182f5fc00
001f01ab: mov    byte ptr [rbx + 0x50], 0
001f01af: test   rdi, rdi
001f01b2: je     0x1801f030f
001f01b8: cmp    byte ptr [rdi + 0xb2], 0
001f01bf: jne    0x1801f02ad
001f01c5: cmp    byte ptr [rdi + 0xb3], 0
001f01cc: jne    0x1801f0286
001f01d2: mov    rcx, qword ptr [rip + 0x3aab4bf]         ; [0x3c9b698] meta:ʼʿʺʿʹʳʹʶʹʹʼ_TypeInfo
001f01d9: call   0x182f60c00
001f01de: xor    edx, edx
001f01e0: mov    rcx, rax
001f01e3: mov    rsi, rax
001f01e6: call   0x18216a330                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$.ctor
001f01eb: xor    edx, edx
001f01ed: mov    rcx, rdi
001f01f0: call   0x180360750                              ; SongEntry$$get_IniPath
001f01f5: xor    edx, edx
001f01f7: mov    rcx, rax
001f01fa: call   0x181b1c090                              ; System.IO.File$$ReadAllText
001f01ff: cmp    byte ptr [rip + 0x3d1bf55], 0            ; [0x3f0c15b] (bss)
001f0206: mov    rdi, rax
001f0209: jne    0x1801f021e
001f020b: lea    rcx, [rip + 0x3ab1396]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
001f0212: call   0x182f609b0
001f0217: mov    byte ptr [rip + 0x3d1bf3d], 1            ; [0x3f0c15b] (bss)
001f021e: test   rdi, rdi
001f0221: je     0x1801f0246
001f0223: xor    ecx, ecx
001f0225: xor    edx, edx
001f0227: mov    dword ptr [rsp + 0x2c], ecx
001f022b: mov    rcx, rdi
001f022e: call   0x1819829d0                              ; System.String$$GetRawStringData
001f0233: mov    qword ptr [rsp + 0x20], rax
001f0238: mov    eax, dword ptr [rdi + 0x10]
001f023b: mov    dword ptr [rsp + 0x28], eax
001f023f: movaps xmm0, xmmword ptr [rsp + 0x20]
001f0244: jmp    0x1801f0249
001f0246: xorps  xmm0, xmm0
001f0249: test   rsi, rsi
001f024c: je     0x1801f030f
001f0252: xor    r8d, r8d
001f0255: movdqa xmmword ptr [rsp + 0x20], xmm0
001f025b: lea    rdx, [rsp + 0x20]
001f0260: mov    rcx, rsi
001f0263: call   0x1821695e0                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ˀʷʳʵʸʼˀʷʵʷʶ
001f0268: xor    r8d, r8d
001f026b: mov    rdx, rsi
001f026e: mov    rcx, rbx
001f0271: call   0x1801efc60                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʺʻʸʽʿʵʴʾʵʴʲ
001f0276: mov    rsi, qword ptr [rsp + 0x40]
001f027b: mov    rbx, qword ptr [rsp + 0x48]
001f0280: add    rsp, 0x30
001f0284: pop    rdi
001f0285: ret    
001f0286: xor    edx, edx
001f0288: mov    rcx, rdi
001f028b: call   0x180360a20                              ; SongEntry$$get_Sng
001f0290: xor    r8d, r8d
001f0293: mov    rdx, rax
001f0296: mov    rcx, rbx
001f0299: mov    rsi, qword ptr [rsp + 0x40]
001f029e: mov    rbx, qword ptr [rsp + 0x48]
001f02a3: add    rsp, 0x30
001f02a7: pop    rdi
001f02a8: jmp    0x1801eff90                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʺʽʻʸʽˁˀʲʺʻʲ
001f02ad: xor    edx, edx
001f02af: mov    rcx, rdi
001f02b2: call   0x180360460                              ; SongEntry$$get_EncSong
001f02b7: test   rax, rax
001f02ba: je     0x1801f030f
001f02bc: mov    rdi, qword ptr [rax + 0x18]
001f02c0: xor    ecx, ecx
001f02c2: mov    qword ptr [rbx + 0x24], rcx
001f02c6: mov    qword ptr [rbx + 0x14], rcx
001f02ca: mov    word ptr [rbx + 0x38], cx
001f02ce: mov    byte ptr [rbx + 0x10], cl
001f02d1: mov    dword ptr [rbx + 0x1c], 0xffffffff
001f02d8: mov    dword ptr [rbx + 0x20], ecx
001f02db: test   rdi, rdi
001f02de: je     0x1801f030f
001f02e0: mov    rdx, qword ptr [rdi + 0x48]
001f02e4: lea    rcx, [rbx + 0x40]
001f02e8: mov    qword ptr [rbx + 0x40], rdx
001f02ec: call   0x182f5fc00
001f02f1: movd   xmm0, dword ptr [rdi + 0x78]
001f02f6: mov    rsi, qword ptr [rsp + 0x40]
001f02fb: cvtdq2pd xmm0, xmm0
001f02ff: movsd  qword ptr [rbx + 0x30], xmm0
001f0304: mov    rbx, qword ptr [rsp + 0x48]
001f0309: add    rsp, 0x30
001f030d: pop    rdi
001f030e: ret    
001f030f: call   0x182f60c50
001f0314: int3   
001f0315: int3   
