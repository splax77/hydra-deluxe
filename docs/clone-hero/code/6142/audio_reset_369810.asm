00369810: mov    qword ptr [rsp + 0x18], rbx
00369815: push   rdi
00369816: sub    rsp, 0x20
0036981a: cmp    byte ptr [rip + 0x3ba4896], 0            ; [0x3f0e0b7] (bss)
00369821: movzx  edi, dl
00369824: mov    rbx, rcx
00369827: jne    0x180369878
00369829: lea    rcx, [rip + 0x3952628]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
00369830: call   0x182f609b0
00369835: lea    rcx, [rip + 0x3994494]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
0036983c: call   0x182f609b0
00369841: lea    rcx, [rip + 0x3970998]                   ; [0x3cda1e0] metamethod:Method$System.Collections.Generic.List<BassAudioManager.ʻˁʾʷʺʺʻʿʳʺʳ>.Clear()
00369848: call   0x182f609b0
0036984d: lea    rcx, [rip + 0x39286d4]                   ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
00369854: call   0x182f609b0
00369859: lea    rcx, [rip + 0x392c318]                   ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00369860: call   0x182f609b0
00369865: lea    rcx, [rip + 0x395c614]                   ; [0x3cc5e80] str:'Failed to stop channel'
0036986c: call   0x182f609b0
00369871: mov    byte ptr [rip + 0x3ba483f], 1            ; [0x3f0e0b7] (bss)
00369878: cmp    byte ptr [rbx + 0x28], 0
0036987c: je     0x180369bf3
00369882: cmp    byte ptr [rbx + 0x88], 0
00369889: je     0x1803698a2
0036988b: test   dil, dil
0036988e: jne    0x1803698a2
00369890: mov    byte ptr [rbx + 0xa8], 1
00369897: mov    rbx, qword ptr [rsp + 0x40]
0036989c: add    rsp, 0x20
003698a0: pop    rdi
003698a1: ret    
003698a2: cmp    byte ptr [rbx + 0x140], 0
003698a9: mov    qword ptr [rsp + 0x30], rbp
003698ae: mov    qword ptr [rsp + 0x38], rsi
003698b3: je     0x180369912
003698b5: xor    edx, edx
003698b7: mov    rcx, rbx
003698ba: call   0x1803658c0                              ; BassAudioManager$$ʳʿʴʳʺʿʴʿʳʵʻ
003698bf: test   al, al
003698c1: jne    0x180369912
003698c3: call   0x182f60ba0
003698c8: mov    rcx, qword ptr [rbx + 0x160]
003698cf: mov    dword ptr [rbx + 0x144], 1
003698d9: test   rcx, rcx
003698dc: je     0x180369bfe
003698e2: xor    edx, edx
003698e4: call   0x181bc88c0                              ; System.Threading.EventWaitHandle$$Set
003698e9: cmp    byte ptr [rbx + 0x168], 0
003698f0: jne    0x180369912
003698f2: mov    rcx, qword ptr [rbx + 0x158]
003698f9: test   rcx, rcx
003698fc: je     0x180369bfe
00369902: mov    rax, qword ptr [rcx]
00369905: mov    rdx, qword ptr [rax + 0x1e0]
0036990c: call   qword ptr [rax + 0x1d8]
00369912: xor    ebp, ebp
00369914: cmp    byte ptr [rbx + 0xc8], bpl
0036991b: je     0x18036994d
0036991d: mov    rcx, qword ptr [rbx + 0xe0]
00369924: test   rcx, rcx
00369927: je     0x180369bfe
0036992d: mov    edx, dword ptr [rbx + 0x58]
00369930: xor    r8d, r8d
00369933: call   0x18038ffe0                              ; ʼʳʾʺʻˀʻʸʵʾʶ$$ʸʿʴˁʾʵʵʷʾʼʿ
00369938: mov    qword ptr [rbx + 0xd0], rbp
0036993f: mov    qword ptr [rbx + 0xd8], rbp
00369946: mov    word ptr [rbx + 0xc8], bp
0036994d: mov    rcx, qword ptr [rbx + 0x98]
00369954: mov    word ptr [rbx + 0x28], bp
00369958: mov    byte ptr [rbx + 0x3c], bpl
0036995c: mov    word ptr [rbx + 0x38], bp
00369960: mov    byte ptr [rbx + 0x3a], bpl
00369964: mov    byte ptr [rbx + 0x78], bpl
00369968: mov    qword ptr [rbx + 0x90], rbp
0036996f: mov    qword ptr [rbx + 0x40], rbp
00369973: mov    qword ptr [rbx + 0x48], rbp
00369977: test   rcx, rcx
0036997a: je     0x180369bfe
00369980: mov    r8d, dword ptr [rcx + 0x18]
00369984: inc    dword ptr [rcx + 0x1c]
00369987: mov    dword ptr [rcx + 0x18], ebp
0036998a: test   r8d, r8d
0036998d: jle    0x18036999d
0036998f: mov    rcx, qword ptr [rcx + 0x10]
00369993: xor    r9d, r9d
00369996: xor    edx, edx
00369998: call   0x181b642b0                              ; System.Array$$Clear
0036999d: xor    edx, edx
0036999f: mov    rcx, rbx
003699a2: call   0x180369390                              ; BassAudioManager$$ʹˀʻʷʵʲʷʼʶʹʷ
003699a7: cmp    byte ptr [rbx + 0x198], bpl
003699ae: je     0x1803699e9
003699b0: mov    rcx, qword ptr [rbx + 0x190]
003699b7: test   rcx, rcx
003699ba: je     0x180369bfe
003699c0: mov    edx, dword ptr [rbx + 0x58]
003699c3: xor    r8d, r8d
003699c6: call   0x18038ffe0                              ; ʼʳʾʺʻˀʻʸʵʾʶ$$ʸʿʴˁʾʵʵʷʾʼʿ
003699cb: mov    rcx, qword ptr [rbx + 0x190]
003699d2: test   rcx, rcx
003699d5: je     0x180369bfe
003699db: xor    edx, edx
003699dd: call   0x18038f5f0                              ; ʼʳʾʺʻˀʻʸʵʾʶ$$Dispose
003699e2: mov    byte ptr [rbx + 0x198], bpl
003699e9: lea    rdx, [rbx + 0x58]
003699ed: xor    r8d, r8d
003699f0: mov    rcx, rbx
003699f3: call   0x1803691d0                              ; BassAudioManager$$ʹʿʹʿʽʶʼʼʿʶʵ
003699f8: mov    rax, qword ptr [rbx + 0x20]
003699fc: mov    edi, ebp
003699fe: mov    ecx, ebp
00369a00: test   rax, rax
00369a03: je     0x180369bfe
00369a09: nop    dword ptr [rax]
00369a10: cmp    ecx, dword ptr [rax + 0x18]
00369a13: jge    0x180369b91
00369a19: mov    rcx, qword ptr [rbx + 0x20]
00369a1d: test   rcx, rcx
00369a20: je     0x180369bfe
00369a26: cmp    edi, dword ptr [rcx + 0x18]
00369a29: jae    0x180369c04
00369a2f: movsxd rax, edi
00369a32: cmp    qword ptr [rcx + rax*8 + 0x20], rbp
00369a37: je     0x180369b7f
00369a3d: mov    rdx, qword ptr [rip + 0x39284e4]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
00369a44: movsxd rax, edi
00369a47: mov    r8, qword ptr [rcx + rax*8 + 0x20]
00369a4c: mov    ecx, 4
00369a51: call   0x182c44210
00369a56: test   al, al
00369a58: je     0x180369b7f
00369a5e: mov    rcx, qword ptr [rbx + 0x20]
00369a62: test   rcx, rcx
00369a65: je     0x180369bfe
00369a6b: cmp    edi, dword ptr [rcx + 0x18]
00369a6e: jae    0x180369c04
00369a74: movsxd rax, edi
00369a77: mov    r8, qword ptr [rcx + rax*8 + 0x20]
00369a7c: test   r8, r8
00369a7f: je     0x180369bfe
00369a85: mov    rdx, qword ptr [rip + 0x392849c]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
00369a8c: xor    ecx, ecx
00369a8e: call   0x182c44210
00369a93: mov    rcx, qword ptr [rip + 0x392c0de]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00369a9a: mov    esi, eax
00369a9c: cmp    dword ptr [rcx + 0xe0], ebp
00369aa2: jne    0x180369aa9
00369aa4: call   0x182f60cf0
00369aa9: xor    edx, edx
00369aab: mov    ecx, esi
00369aad: call   0x18024e220                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˀʻʹʷʹʷˀʺʼʼʳ
00369ab2: mov    rcx, qword ptr [rip + 0x395239f]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
00369ab9: movzx  esi, al
00369abc: cmp    dword ptr [rcx + 0xe0], ebp
00369ac2: jne    0x180369ac9
00369ac4: call   0x182f60cf0
00369ac9: mov    rdx, qword ptr [rip + 0x395c3b0]         ; [0x3cc5e80] str:'Failed to stop channel'
00369ad0: xor    r8d, r8d
00369ad3: movzx  ecx, sil
00369ad7: call   0x18036d890                              ; BassAudioManager$$ˀʷʹʴʴʸʿʻʷʳʷ
00369adc: mov    rcx, qword ptr [rbx + 0x20]
00369ae0: test   rcx, rcx
00369ae3: je     0x180369bfe
00369ae9: cmp    edi, dword ptr [rcx + 0x18]
00369aec: jae    0x180369c04
00369af2: movsxd rax, edi
00369af5: mov    r8, qword ptr [rcx + rax*8 + 0x20]
00369afa: test   r8, r8
00369afd: je     0x180369bfe
00369b03: mov    rdx, qword ptr [rip + 0x392841e]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
00369b0a: mov    ecx, 3
00369b0f: call   0x182c42550
00369b14: cmp    byte ptr [rbx + 0x5c], bpl
00369b18: jne    0x180369b4f
00369b1a: mov    rcx, qword ptr [rbx + 0x20]
00369b1e: test   rcx, rcx
00369b21: je     0x180369bfe
00369b27: cmp    edi, dword ptr [rcx + 0x18]
00369b2a: jae    0x180369c04
00369b30: movsxd rax, edi
00369b33: mov    r8, qword ptr [rcx + rax*8 + 0x20]
00369b38: test   r8, r8
00369b3b: je     0x180369bfe
00369b41: mov    rdx, qword ptr [rip + 0x3994188]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00369b48: xor    ecx, ecx
00369b4a: call   0x182c42550
00369b4f: mov    rcx, qword ptr [rbx + 0x20]
00369b53: test   rcx, rcx
00369b56: je     0x180369bfe
00369b5c: cmp    edi, dword ptr [rcx + 0x18]
00369b5f: jae    0x180369c04
00369b65: movsxd rax, edi
00369b68: xor    edx, edx
00369b6a: mov    qword ptr [rcx + rax*8 + 0x20], rbp
00369b6f: movsxd rax, edi
00369b72: lea    rcx, [rcx + rax*8]
00369b76: add    rcx, 0x20
00369b7a: call   0x182f5fc00
00369b7f: mov    rax, qword ptr [rbx + 0x20]
00369b83: inc    edi
00369b85: mov    ecx, edi
00369b87: test   rax, rax
00369b8a: je     0x180369bfe
00369b8c: jmp    0x180369a10
00369b91: cmp    byte ptr [rbx + 0x140], bpl
00369b98: je     0x180369be5
00369b9a: cmp    qword ptr [rbx + 0x138], rbp
00369ba1: je     0x180369bd2
00369ba3: xor    ecx, ecx
00369ba5: call   0x181bed500                              ; System.Threading.Thread$$get_CurrentThread
00369baa: test   rax, rax
00369bad: je     0x180369bfe
00369baf: xor    edx, edx
00369bb1: mov    rcx, rax
00369bb4: call   0x181bec830                              ; System.Threading.Thread$$GetHashCode
00369bb9: mov    rcx, qword ptr [rbx + 0x138]
00369bc0: mov    edi, eax
00369bc2: test   rcx, rcx
00369bc5: je     0x180369bfe
00369bc7: xor    edx, edx
00369bc9: call   0x181bec830                              ; System.Threading.Thread$$GetHashCode
00369bce: cmp    edi, eax
00369bd0: je     0x180369be5
00369bd2: mov    rcx, qword ptr [rbx + 0x150]
00369bd9: test   rcx, rcx
00369bdc: je     0x180369bfe
00369bde: xor    edx, edx
00369be0: call   0x181bc88c0                              ; System.Threading.EventWaitHandle$$Set
00369be5: mov    rsi, qword ptr [rsp + 0x38]
00369bea: mov    byte ptr [rbx + 0x5c], bpl
00369bee: mov    rbp, qword ptr [rsp + 0x30]
00369bf3: mov    rbx, qword ptr [rsp + 0x40]
00369bf8: add    rsp, 0x20
00369bfc: pop    rdi
00369bfd: ret    
00369bfe: call   0x182f60c50
00369c03: int3   
00369c04: call   0x182f60c40
00369c09: int3   
00369c0a: int3   
