0032bf80: push   rdi
0032bf82: sub    rsp, 0x30
0032bf86: cmp    byte ptr [rip + 0x3be1dab], 0            ; [0x3f0dd38] (bss)
0032bf8d: mov    rdi, rcx
0032bf90: jne    0x18032c045
0032bf96: lea    rcx, [rip + 0x399787b]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032bf9d: call   0x182f609b0
0032bfa2: lea    rcx, [rip + 0x39c38e7]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032bfa9: call   0x182f609b0
0032bfae: lea    rcx, [rip + 0x398a71b]                   ; [0x3cb66d0] metamethod:Method$System.Collections.Generic.List<SongEntry>.get_Item()
0032bfb5: call   0x182f609b0
0032bfba: lea    rcx, [rip + 0x399e9df]                   ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
0032bfc1: call   0x182f609b0
0032bfc6: lea    rcx, [rip + 0x39e307b]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032bfcd: call   0x182f609b0
0032bfd2: lea    rcx, [rip + 0x39710bf]                   ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
0032bfd9: call   0x182f609b0
0032bfde: lea    rcx, [rip + 0x39b1c4b]                   ; [0x3cddc30] str:'Modifiers'
0032bfe5: call   0x182f609b0
0032bfea: lea    rcx, [rip + 0x39bbd9f]                   ; [0x3ce7d90] str:'Choose Difficulty'
0032bff1: call   0x182f609b0
0032bff6: lea    rcx, [rip + 0x39ddefb]                   ; [0x3d09ef8] str:'<color=#00FFFF>'
0032bffd: call   0x182f609b0
0032c002: lea    rcx, [rip + 0x397ffc7]                   ; [0x3cabfd0] str:'EndOfSong'
0032c009: call   0x182f609b0
0032c00e: lea    rcx, [rip + 0x398749b]                   ; [0x3cb34b0] str:'Ready'
0032c015: call   0x182f609b0
0032c01a: lea    rcx, [rip + 0x39d91b7]                   ; [0x3d051d8] str:'</color>'
0032c021: call   0x182f609b0
0032c026: lea    rcx, [rip + 0x39bbee3]                   ; [0x3ce7f10] str:'Choose Modifiers'
0032c02d: call   0x182f609b0
0032c032: lea    rcx, [rip + 0x39bbe17]                   ; [0x3ce7e50] str:'Choose Instrument'
0032c039: call   0x182f609b0
0032c03e: mov    byte ptr [rip + 0x3be1cf3], 1            ; [0x3f0dd38] (bss)
0032c045: mov    rcx, qword ptr [rip + 0x39c3844]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c04c: mov    dword ptr [rsp + 0x50], 0
0032c054: cmp    dword ptr [rcx + 0xe0], 0
0032c05b: jne    0x18032c069
0032c05d: call   0x182f60cf0
0032c062: mov    rcx, qword ptr [rip + 0x39c3827]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c069: mov    rax, qword ptr [rcx + 0xb8]
0032c070: mov    qword ptr [rsp + 0x40], rbx
0032c075: mov    rdx, qword ptr [rax + 8]
0032c079: test   rdx, rdx
0032c07c: je     0x18032c7db
0032c082: mov    rdx, qword ptr [rdx + 0x38]
0032c086: test   rdx, rdx
0032c089: je     0x18032c7db
0032c08f: movsxd rax, dword ptr [rdi + 0x130]
0032c096: cmp    eax, dword ptr [rdx + 0x18]
0032c099: jae    0x18032c7e1
0032c09f: mov    r8, qword ptr [rdx + rax*8 + 0x20]
0032c0a4: test   r8, r8
0032c0a7: je     0x18032c7db
0032c0ad: cmp    qword ptr [r8 + 0x18], 0
0032c0b2: jne    0x18032c0ce
0032c0b4: mov    rax, qword ptr [r8 + 0x10]
0032c0b8: test   rax, rax
0032c0bb: je     0x18032c7b4
0032c0c1: cmp    byte ptr [rax + 0xb1], 0
0032c0c8: je     0x18032c7b4
0032c0ce: cmp    dword ptr [rcx + 0xe0], 0
0032c0d5: jne    0x18032c0e3
0032c0d7: call   0x182f60cf0
0032c0dc: mov    rcx, qword ptr [rip + 0x39c37ad]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c0e3: mov    rax, qword ptr [rcx + 0xb8]
0032c0ea: mov    rcx, qword ptr [rax + 8]
0032c0ee: test   rcx, rcx
0032c0f1: je     0x18032c7db
0032c0f7: mov    rcx, qword ptr [rcx + 0x38]
0032c0fb: test   rcx, rcx
0032c0fe: je     0x18032c7db
0032c104: movsxd rax, dword ptr [rdi + 0x130]
0032c10b: cmp    eax, dword ptr [rcx + 0x18]
0032c10e: jae    0x18032c7e1
0032c114: mov    rdx, qword ptr [rcx + rax*8 + 0x20]
0032c119: test   rdx, rdx
0032c11c: je     0x18032c7db
0032c122: cmp    qword ptr [rdx + 0x10], 0
0032c127: je     0x18032c7b4
0032c12d: xor    edx, edx
0032c12f: mov    rcx, rdi
0032c132: call   0x1801d46f0                              ; BaseMenu$$OnEnable
0032c137: mov    rax, qword ptr [rip + 0x39c3752]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c13e: cmp    dword ptr [rax + 0xe0], 0
0032c145: jne    0x18032c156
0032c147: mov    rcx, rax
0032c14a: call   0x182f60cf0
0032c14f: mov    rax, qword ptr [rip + 0x39c373a]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c156: mov    rax, qword ptr [rax + 0xb8]
0032c15d: mov    rdx, qword ptr [rax + 8]
0032c161: test   rdx, rdx
0032c164: je     0x18032c7db
0032c16a: mov    rdx, qword ptr [rdx + 0x38]
0032c16e: test   rdx, rdx
0032c171: je     0x18032c7db
0032c177: movsxd rax, dword ptr [rdi + 0x130]
0032c17e: cmp    eax, dword ptr [rdx + 0x18]
0032c181: jae    0x18032c7e1
0032c187: mov    rdx, qword ptr [rdx + rax*8 + 0x20]
0032c18c: lea    rcx, [rdi + 0x20]
0032c190: mov    qword ptr [rdi + 0x20], rdx
0032c194: call   0x182f5fc00
0032c199: mov    rcx, qword ptr [rdi + 0x20]
0032c19d: xor    edx, edx
0032c19f: mov    rbx, qword ptr [rdi + 0x128]
0032c1a6: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
0032c1ab: movzx  ecx, al
0032c1ae: mov    rax, qword ptr [rdi + 0x20]
0032c1b2: test   rax, rax
0032c1b5: je     0x18032c7db
0032c1bb: mov    rax, qword ptr [rax + 0x10]
0032c1bf: test   cl, cl
0032c1c1: je     0x18032c1f8
0032c1c3: test   rax, rax
0032c1c6: je     0x18032c7db
0032c1cc: mov    rdx, qword ptr [rax + 0x18]
0032c1d0: test   rdx, rdx
0032c1d3: je     0x18032c7db
0032c1d9: mov    r8, qword ptr [rip + 0x39d8ff8]          ; [0x3d051d8] str:'</color>'
0032c1e0: xor    r9d, r9d
0032c1e3: mov    rdx, qword ptr [rdx + 0x30]
0032c1e7: mov    rcx, qword ptr [rip + 0x39ddd0a]         ; [0x3d09ef8] str:'<color=#00FFFF>'
0032c1ee: call   0x18197eef0                              ; System.String$$Concat
0032c1f3: mov    rdx, rax
0032c1f6: jmp    0x18032c212
0032c1f8: test   rax, rax
0032c1fb: je     0x18032c7db
0032c201: mov    rdx, qword ptr [rax + 0x18]
0032c205: test   rdx, rdx
0032c208: je     0x18032c7db
0032c20e: mov    rdx, qword ptr [rdx + 0x38]
0032c212: test   rbx, rbx
0032c215: je     0x18032c7db
0032c21b: mov    rax, qword ptr [rbx]
0032c21e: mov    rcx, rbx
0032c221: mov    r8, qword ptr [rax + 0x560]
0032c228: call   qword ptr [rax + 0x558]
0032c22e: mov    rax, qword ptr [rdi + 0x20]
0032c232: test   rax, rax
0032c235: je     0x18032c7db
0032c23b: mov    rax, qword ptr [rax + 0x10]
0032c23f: test   rax, rax
0032c242: je     0x18032c267
0032c244: cmp    byte ptr [rax + 0xb1], 0
0032c24b: je     0x18032c267
0032c24d: mov    rcx, qword ptr [rdi + 0x128]
0032c254: test   rcx, rcx
0032c257: je     0x18032c7db
0032c25d: movdqa xmm0, xmmword ptr [rip + 0x2d37c3b]      ; [0x3063ea0] xmm dbl=(0.0078125, 0.0078125) flt=(0.0, 1.0, 0.0, 1.0)
0032c265: jmp    0x18032c298
0032c267: mov    rcx, qword ptr [rdi + 0x20]
0032c26b: xor    edx, edx
0032c26d: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
0032c272: mov    rcx, qword ptr [rdi + 0x128]
0032c279: test   rcx, rcx
0032c27c: je     0x18032c7db
0032c282: test   al, al
0032c284: jne    0x18032c290
0032c286: movdqa xmm0, xmmword ptr [rip + 0x2d37c32]      ; [0x3063ec0] xmm dbl=(0.007812501848093234, 0.007812501848093234) flt=(1.0, 1.0, 1.0, 1.0)
0032c28e: jmp    0x18032c298
0032c290: movdqa xmm0, xmmword ptr [rip + 0x2d381f8]      ; [0x3064490] xmm dbl=(0.0078125, 0.007812501848093234) flt=(0.0, 1.0, 1.0, 1.0)
0032c298: mov    rax, qword ptr [rcx]
0032c29b: lea    rdx, [rsp + 0x20]
0032c2a0: movaps xmmword ptr [rsp + 0x20], xmm0
0032c2a5: mov    r8, qword ptr [rax + 0x2b0]
0032c2ac: call   qword ptr [rax + 0x2a8]
0032c2b2: mov    rax, qword ptr [rdi + 0x20]
0032c2b6: test   rax, rax
0032c2b9: je     0x18032c7db
0032c2bf: mov    rcx, qword ptr [rax + 0x10]
0032c2c3: test   rcx, rcx
0032c2c6: je     0x18032c7db
0032c2cc: xor    edx, edx
0032c2ce: call   0x182123320                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʾˁʺʴʻʵʳʾʲʺ
0032c2d3: mov    rcx, qword ptr [rdi + 0x20]
0032c2d7: mov    byte ptr [rdi + 0x1e8], al
0032c2dd: test   rcx, rcx
0032c2e0: je     0x18032c7db
0032c2e6: mov    rcx, qword ptr [rcx + 0x10]
0032c2ea: test   rcx, rcx
0032c2ed: je     0x18032c7db
0032c2f3: cmp    byte ptr [rcx + 0x10], 0xf
0032c2f7: jne    0x18032c30e
0032c2f9: mov    rcx, qword ptr [rdi + 0x20]
0032c2fd: xor    edx, edx
0032c2ff: mov    rbx, qword ptr [rcx + 0x10]
0032c303: movzx  ecx, al
0032c306: call   0x1800d75f0                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʲʸʳʼʷʸˁʶʾʲʷ
0032c30b: mov    byte ptr [rbx + 0x10], al
0032c30e: mov    rax, qword ptr [rdi + 0x20]
0032c312: test   rax, rax
0032c315: je     0x18032c7db
0032c31b: mov    rax, qword ptr [rax + 0x10]
0032c31f: test   rax, rax
0032c322: je     0x18032c7db
0032c328: movzx  eax, byte ptr [rax + 0x10]
0032c32c: mov    byte ptr [rdi + 0x240], al
0032c332: mov    rax, qword ptr [rdi + 0x20]
0032c336: test   rax, rax
0032c339: je     0x18032c7db
0032c33f: mov    rax, qword ptr [rax + 0x10]
0032c343: test   rax, rax
0032c346: je     0x18032c7db
0032c34c: movzx  eax, byte ptr [rax + 0x11]
0032c350: mov    byte ptr [rdi + 0x241], al
0032c356: mov    rcx, qword ptr [rip + 0x399e643]         ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
0032c35d: cmp    dword ptr [rcx + 0xe0], 0
0032c364: jne    0x18032c36b
0032c366: call   0x182f60cf0
0032c36b: xor    ecx, ecx
0032c36d: call   0x18289bbf0                              ; UnityEngine.SceneManagement.SceneManager$$GetActiveScene
0032c372: xor    edx, edx
0032c374: mov    dword ptr [rsp + 0x50], eax
0032c378: lea    rcx, [rsp + 0x50]
0032c37d: call   0x18289c8e0                              ; UnityEngine.SceneManagement.Scene$$get_name
0032c382: mov    rdx, qword ptr [rip + 0x397fc47]         ; [0x3cabfd0] str:'EndOfSong'
0032c389: xor    r8d, r8d
0032c38c: mov    rcx, rax
0032c38f: call   0x181981570                              ; System.String$$op_Equality
0032c394: test   al, al
0032c396: jne    0x18032c39a
0032c398: jmp    0x18032c3d4
0032c39a: mov    rax, qword ptr [rip + 0x39c34ef]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c3a1: cmp    dword ptr [rax + 0xe0], 0
0032c3a8: jne    0x18032c3b9
0032c3aa: mov    rcx, rax
0032c3ad: call   0x182f60cf0
0032c3b2: mov    rax, qword ptr [rip + 0x39c34d7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c3b9: mov    rax, qword ptr [rax + 0xb8]
0032c3c0: mov    rcx, qword ptr [rax + 8]
0032c3c4: test   rcx, rcx
0032c3c7: je     0x18032c7db
0032c3cd: xor    edx, edx
0032c3cf: call   0x180236c80                              ; GlobalVariables$$ʼʿʳˀʹʿʼʺʳʿʲ
0032c3d4: mov    byte ptr [rdi + 0x244], al
0032c3da: mov    rax, qword ptr [rip + 0x3997437]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032c3e1: mov    rcx, qword ptr [rax + 0xb8]
0032c3e8: mov    rcx, qword ptr [rcx]
0032c3eb: test   rcx, rcx
0032c3ee: je     0x18032c7db
0032c3f4: xor    edx, edx
0032c3f6: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0032c3fb: test   al, al
0032c3fd: jne    0x18032c403
0032c3ff: xor    ecx, ecx
0032c401: jmp    0x18032c42c
0032c403: mov    rax, qword ptr [rip + 0x39e2c3e]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032c40a: mov    rcx, qword ptr [rax + 0xb8]
0032c411: mov    rax, qword ptr [rcx]
0032c414: test   rax, rax
0032c417: je     0x18032c7db
0032c41d: mov    eax, dword ptr [rax + 0xc0]
0032c423: cmp    dword ptr [rdi + 0x130], eax
0032c429: sete   cl
0032c42c: mov    byte ptr [rdi + 0x245], cl
0032c432: mov    rax, qword ptr [rdi + 0x20]
0032c436: test   rax, rax
0032c439: je     0x18032c7db
0032c43f: mov    rcx, qword ptr [rax + 0x10]
0032c443: test   rcx, rcx
0032c446: je     0x18032c7db
0032c44c: xor    edx, edx
0032c44e: call   0x182123320                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʾˁʺʴʻʵʳʾʲʺ
0032c453: cmp    al, 2
0032c455: jne    0x18032c45b
0032c457: mov    al, 1
0032c459: jmp    0x18032c460
0032c45b: cmp    al, 3
0032c45d: sete   al
0032c460: mov    byte ptr [rdi + 0x242], al
0032c466: mov    rax, qword ptr [rdi + 0x20]
0032c46a: test   rax, rax
0032c46d: je     0x18032c7db
0032c473: mov    rcx, qword ptr [rax + 0x10]
0032c477: test   rcx, rcx
0032c47a: je     0x18032c7db
0032c480: xor    edx, edx
0032c482: call   0x182123150                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʴʶʾʶˀʺʾʲʸʻ
0032c487: xor    edx, edx
0032c489: mov    byte ptr [rdi + 0x243], al
0032c48f: mov    rcx, rdi
0032c492: call   0x18032d400                              ; PlayerSelectionMenu$$ʵʲʴʸʶʿʴʴʹʻʼ
0032c497: mov    rax, qword ptr [rdi + 0x1a0]
0032c49e: test   rax, rax
0032c4a1: je     0x18032c7db
0032c4a7: cmp    dword ptr [rax + 0x18], 0
0032c4ab: jbe    0x18032c7e1
0032c4b1: mov    rbx, qword ptr [rax + 0x20]
0032c4b5: xor    ecx, ecx
0032c4b7: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
0032c4bc: test   rax, rax
0032c4bf: je     0x18032c7db
0032c4c5: mov    rdx, qword ptr [rip + 0x39b1764]         ; [0x3cddc30] str:'Modifiers'
0032c4cc: xor    r8d, r8d
0032c4cf: mov    rcx, rax
0032c4d2: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0032c4d7: test   rbx, rbx
0032c4da: je     0x18032c7db
0032c4e0: mov    r9, qword ptr [rbx]
0032c4e3: mov    rdx, rax
0032c4e6: mov    rcx, rbx
0032c4e9: mov    r8, qword ptr [r9 + 0x560]
0032c4f0: call   qword ptr [r9 + 0x558]
0032c4f7: mov    rax, qword ptr [rdi + 0x1a0]
0032c4fe: test   rax, rax
0032c501: je     0x18032c7db
0032c507: cmp    dword ptr [rax + 0x18], 1
0032c50b: jbe    0x18032c7e1
0032c511: mov    rbx, qword ptr [rax + 0x28]
0032c515: xor    ecx, ecx
0032c517: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
0032c51c: test   rax, rax
0032c51f: je     0x18032c7db
0032c525: mov    rdx, qword ptr [rip + 0x39bb924]         ; [0x3ce7e50] str:'Choose Instrument'
0032c52c: xor    r8d, r8d
0032c52f: mov    rcx, rax
0032c532: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0032c537: test   rbx, rbx
0032c53a: je     0x18032c7db
0032c540: mov    r9, qword ptr [rbx]
0032c543: mov    rdx, rax
0032c546: mov    rcx, rbx
0032c549: mov    r8, qword ptr [r9 + 0x560]
0032c550: call   qword ptr [r9 + 0x558]
0032c557: mov    rax, qword ptr [rdi + 0x1a0]
0032c55e: test   rax, rax
0032c561: je     0x18032c7db
0032c567: cmp    dword ptr [rax + 0x18], 2
0032c56b: jbe    0x18032c7e1
0032c571: mov    rbx, qword ptr [rax + 0x30]
0032c575: xor    ecx, ecx
0032c577: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
0032c57c: test   rax, rax
0032c57f: je     0x18032c7db
0032c585: mov    rdx, qword ptr [rip + 0x39bb804]         ; [0x3ce7d90] str:'Choose Difficulty'
0032c58c: xor    r8d, r8d
0032c58f: mov    rcx, rax
0032c592: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0032c597: test   rbx, rbx
0032c59a: je     0x18032c7db
0032c5a0: mov    r9, qword ptr [rbx]
0032c5a3: mov    rdx, rax
0032c5a6: mov    rcx, rbx
0032c5a9: mov    r8, qword ptr [r9 + 0x560]
0032c5b0: call   qword ptr [r9 + 0x558]
0032c5b7: mov    rax, qword ptr [rdi + 0x1a0]
0032c5be: test   rax, rax
0032c5c1: je     0x18032c7db
0032c5c7: cmp    dword ptr [rax + 0x18], 3
0032c5cb: jbe    0x18032c7e1
0032c5d1: mov    rbx, qword ptr [rax + 0x38]
0032c5d5: xor    ecx, ecx
0032c5d7: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
0032c5dc: test   rax, rax
0032c5df: je     0x18032c7db
0032c5e5: mov    rdx, qword ptr [rip + 0x39bb924]         ; [0x3ce7f10] str:'Choose Modifiers'
0032c5ec: xor    r8d, r8d
0032c5ef: mov    rcx, rax
0032c5f2: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0032c5f7: test   rbx, rbx
0032c5fa: je     0x18032c7db
0032c600: mov    r9, qword ptr [rbx]
0032c603: mov    rdx, rax
0032c606: mov    rcx, rbx
0032c609: mov    r8, qword ptr [r9 + 0x560]
0032c610: call   qword ptr [r9 + 0x558]
0032c617: cmp    byte ptr [rdi + 0x244], 0
0032c61e: jne    0x18032c659
0032c620: mov    rax, qword ptr [rip + 0x39c3269]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c627: cmp    dword ptr [rax + 0xe0], 0
0032c62e: jne    0x18032c63f
0032c630: mov    rcx, rax
0032c633: call   0x182f60cf0
0032c638: mov    rax, qword ptr [rip + 0x39c3251]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c63f: mov    rax, qword ptr [rax + 0xb8]
0032c646: mov    rax, qword ptr [rax + 8]
0032c64a: test   rax, rax
0032c64d: je     0x18032c7db
0032c653: mov    rax, qword ptr [rax + 0x28]
0032c657: jmp    0x18032c6d3
0032c659: mov    rax, qword ptr [rip + 0x3970a38]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
0032c660: cmp    dword ptr [rax + 0xe0], 0
0032c667: jne    0x18032c678
0032c669: mov    rcx, rax
0032c66c: call   0x182f60cf0
0032c671: mov    rax, qword ptr [rip + 0x3970a20]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
0032c678: mov    rax, qword ptr [rax + 0xb8]
0032c67f: mov    rbx, qword ptr [rax + 0x20]
0032c683: mov    rax, qword ptr [rip + 0x39c3206]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c68a: cmp    dword ptr [rax + 0xe0], 0
0032c691: jne    0x18032c6a2
0032c693: mov    rcx, rax
0032c696: call   0x182f60cf0
0032c69b: mov    rax, qword ptr [rip + 0x39c31ee]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0032c6a2: mov    rax, qword ptr [rax + 0xb8]
0032c6a9: mov    rdx, qword ptr [rax + 8]
0032c6ad: test   rdx, rdx
0032c6b0: je     0x18032c7db
0032c6b6: test   rbx, rbx
0032c6b9: je     0x18032c7db
0032c6bf: mov    edx, dword ptr [rdx + 0x58]
0032c6c2: mov    rcx, rbx
0032c6c5: mov    r8, qword ptr [rip + 0x398a004]          ; [0x3cb66d0] metamethod:Method$System.Collections.Generic.List<SongEntry>.get_Item()
0032c6cc: inc    edx
0032c6ce: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0032c6d3: mov    rcx, rax
0032c6d6: lea    rcx, [rdi + 0x238]
0032c6dd: mov    qword ptr [rdi + 0x238], rax
0032c6e4: mov    rdx, rax
0032c6e7: call   0x182f5fc00
0032c6ec: mov    rax, qword ptr [rdi + 0x20]
0032c6f0: test   rax, rax
0032c6f3: je     0x18032c7db
0032c6f9: mov    rax, qword ptr [rax + 0x10]
0032c6fd: test   rax, rax
0032c700: je     0x18032c7db
0032c706: xor    r8d, r8d
0032c709: mov    rcx, rdi
0032c70c: cmp    byte ptr [rax + 0xb2], r8b
0032c713: je     0x18032c71e
0032c715: xor    edx, edx
0032c717: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032c71c: jmp    0x18032c752
0032c71e: mov    edx, 1
0032c723: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032c728: mov    rax, qword ptr [rdi + 0x20]
0032c72c: mov    word ptr [rdi + 0x1f0], 0x101
0032c735: test   rax, rax
0032c738: je     0x18032c7db
0032c73e: mov    rax, qword ptr [rax + 0x10]
0032c742: test   rax, rax
0032c745: je     0x18032c7db
0032c74b: mov    byte ptr [rax + 0xb2], 1
0032c752: mov    rcx, qword ptr [rdi + 0x160]
0032c759: test   rcx, rcx
0032c75c: je     0x18032c7db
0032c75e: xor    r8d, r8d
0032c761: xor    edx, edx
0032c763: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0032c768: mov    rbx, qword ptr [rdi + 0x168]
0032c76f: xor    ecx, ecx
0032c771: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
0032c776: test   rax, rax
0032c779: je     0x18032c7db
0032c77b: mov    rdx, qword ptr [rip + 0x3986d2e]         ; [0x3cb34b0] str:'Ready'
0032c782: xor    r8d, r8d
0032c785: mov    rcx, rax
0032c788: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0032c78d: test   rbx, rbx
0032c790: je     0x18032c7db
0032c792: mov    r9, qword ptr [rbx]
0032c795: mov    rdx, rax
0032c798: mov    rcx, rbx
0032c79b: mov    r8, qword ptr [r9 + 0x560]
0032c7a2: call   qword ptr [r9 + 0x558]
0032c7a9: mov    rbx, qword ptr [rsp + 0x40]
0032c7ae: add    rsp, 0x30
0032c7b2: pop    rdi
0032c7b3: ret    
0032c7b4: xor    edx, edx
0032c7b6: mov    rcx, rdi
0032c7b9: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
0032c7be: test   rax, rax
0032c7c1: je     0x18032c7db
0032c7c3: xor    r8d, r8d
0032c7c6: xor    edx, edx
0032c7c8: mov    rcx, rax
0032c7cb: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0032c7d0: mov    rbx, qword ptr [rsp + 0x40]
0032c7d5: add    rsp, 0x30
0032c7d9: pop    rdi
0032c7da: ret    
0032c7db: call   0x182f60c50
0032c7e0: int3   
0032c7e1: call   0x182f60c40
0032c7e6: int3   
0032c7e7: int3   
