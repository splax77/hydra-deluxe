00112490: mov    qword ptr [rsp + 8], rbx
00112495: mov    qword ptr [rsp + 0x10], rsi
0011249a: push   rdi
0011249b: sub    rsp, 0x20
0011249f: cmp    byte ptr [rip + 0x3df9f1e], 0            ; [0x3f0c3c4] (bss)
001124a6: mov    rsi, r8
001124a9: mov    rdi, rdx
001124ac: mov    rbx, rcx
001124af: jne    0x1801124c4
001124b1: lea    rcx, [rip + 0x3b85e68]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
001124b8: call   0x182f609b0
001124bd: mov    byte ptr [rip + 0x3df9f00], 1            ; [0x3f0c3c4] (bss)
001124c4: test   rdi, rdi
001124c7: je     0x1801127af
001124cd: mov    rax, qword ptr [rdi + 0x10]
001124d1: test   rax, rax
001124d4: je     0x1801127af
001124da: movzx  eax, byte ptr [rax + 0x10]
001124de: mov    byte ptr [rbx + 0x10], al
001124e1: mov    rax, qword ptr [rdi + 0x10]
001124e5: test   rax, rax
001124e8: je     0x1801127af
001124ee: movzx  eax, byte ptr [rax + 0x11]
001124f2: mov    byte ptr [rbx + 0x11], al
001124f5: mov    rax, qword ptr [rdi + 0x10]
001124f9: test   rax, rax
001124fc: je     0x1801127af
00112502: mov    rdx, qword ptr [rax + 0x18]
00112506: test   rdx, rdx
00112509: je     0x1801127af
0011250f: mov    rdx, qword ptr [rdx + 0x38]
00112513: lea    rcx, [rbx + 0x18]
00112517: mov    qword ptr [rbx + 0x18], rdx
0011251b: call   0x182f5fc00
00112520: mov    rax, qword ptr [rdi + 0x10]
00112524: test   rax, rax
00112527: je     0x1801127af
0011252d: mov    eax, dword ptr [rax + 0x14]
00112530: mov    dword ptr [rbx + 0x20], eax
00112533: mov    rax, qword ptr [rdi + 0x10]
00112537: test   rax, rax
0011253a: je     0x1801127af
00112540: movzx  eax, byte ptr [rax + 0xb1]
00112547: mov    byte ptr [rbx + 0x24], al
0011254a: mov    rax, qword ptr [rdi + 0x10]
0011254e: test   rax, rax
00112551: je     0x1801127af
00112557: mov    rcx, qword ptr [rax + 0x38]
0011255b: test   rcx, rcx
0011255e: je     0x1801127af
00112564: xor    edx, edx
00112566: call   0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
0011256b: mov    byte ptr [rbx + 0x25], al
0011256e: mov    rax, qword ptr [rip + 0x3b85dab]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00112575: mov    rcx, qword ptr [rax + 0xb8]
0011257c: mov    rcx, qword ptr [rcx + 0xe8]
00112583: test   rcx, rcx
00112586: je     0x1801127af
0011258c: xor    edx, edx
0011258e: call   0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
00112593: mov    byte ptr [rbx + 0x26], al
00112596: mov    rax, qword ptr [rip + 0x3b85d83]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0011259d: mov    rcx, qword ptr [rax + 0xb8]
001125a4: mov    rcx, qword ptr [rcx + 0x20]
001125a8: test   rcx, rcx
001125ab: je     0x1801127af
001125b1: xor    edx, edx
001125b3: call   0x18210c1f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʽʺʲʴʼʿˁʼʻˁʴ
001125b8: movss  dword ptr [rbx + 0x28], xmm0
001125bd: mov    rax, qword ptr [rip + 0x3b85d5c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
001125c4: mov    rcx, qword ptr [rax + 0xb8]
001125cb: mov    rcx, qword ptr [rcx + 0x18]
001125cf: test   rcx, rcx
001125d2: je     0x1801127af
001125d8: xor    edx, edx
001125da: call   0x18210c1f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʽʺʲʴʼʿˁʼʻˁʴ
001125df: xor    edx, edx
001125e1: movss  dword ptr [rbx + 0x2c], xmm0
001125e6: mov    rcx, rdi
001125e9: call   0x180337ce0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʾˀʴˁʿˁˀʹˀʴʷ
001125ee: cmp    al, 1
001125f0: je     0x18011262d
001125f2: xor    edx, edx
001125f4: mov    rcx, rdi
001125f7: call   0x180337ce0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʾˀʴˁʿˁˀʹˀʴʷ
001125fc: test   al, al
001125fe: je     0x180112627
00112600: xor    edx, edx
00112602: mov    rcx, rdi
00112605: call   0x180337ce0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʾˀʴˁʿˁˀʹˀʴʷ
0011260a: cmp    al, 2
0011260c: je     0x180112621
0011260e: xor    edx, edx
00112610: mov    rcx, rdi
00112613: call   0x180337ce0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʾˀʴˁʿˁˀʹˀʴʷ
00112618: cmp    al, 3
0011261a: jne    0x180112631
0011261c: mov    byte ptr [rbx + 0x30], al
0011261f: jmp    0x180112631
00112621: mov    byte ptr [rbx + 0x30], 2
00112625: jmp    0x180112631
00112627: mov    byte ptr [rbx + 0x30], 0
0011262b: jmp    0x180112631
0011262d: mov    byte ptr [rbx + 0x30], 1
00112631: test   rsi, rsi
00112634: je     0x1801127af
0011263a: xor    edx, edx
0011263c: mov    rcx, rsi
0011263f: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
00112644: mov    dword ptr [rbx + 0x34], eax
00112647: xor    edx, edx
00112649: mov    eax, dword ptr [rsi + 0x10]
0011264c: mov    rcx, rsi
0011264f: mov    dword ptr [rbx + 0x38], eax
00112652: mov    eax, dword ptr [rsi + 0x18]
00112655: mov    dword ptr [rbx + 0x3c], eax
00112658: mov    eax, dword ptr [rsi + 0x2c]
0011265b: mov    dword ptr [rbx + 0x40], eax
0011265e: mov    eax, dword ptr [rsi + 0x30]
00112661: mov    dword ptr [rbx + 0x44], eax
00112664: mov    eax, dword ptr [rsi + 0x38]
00112667: mov    dword ptr [rbx + 0x48], eax
0011266a: mov    eax, dword ptr [rsi + 0x34]
0011266d: mov    dword ptr [rbx + 0x4c], eax
00112670: mov    eax, dword ptr [rsi + 0x3c]
00112673: mov    dword ptr [rbx + 0x50], eax
00112676: mov    eax, dword ptr [rsi + 0x1c]
00112679: mov    dword ptr [rbx + 0x54], eax
0011267c: mov    eax, dword ptr [rsi + 0x40]
0011267f: mov    dword ptr [rbx + 0x58], eax
00112682: mov    eax, dword ptr [rsi + 0x44]
00112685: mov    dword ptr [rbx + 0x5c], eax
00112688: mov    eax, dword ptr [rsi + 0x20]
0011268b: mov    dword ptr [rbx + 0x60], eax
0011268e: mov    eax, dword ptr [rsi + 0x24]
00112691: mov    dword ptr [rbx + 0x64], eax
00112694: mov    eax, dword ptr [rsi + 0x28]
00112697: mov    dword ptr [rbx + 0x68], eax
0011269a: mov    eax, dword ptr [rsi + 0x48]
0011269d: mov    dword ptr [rbx + 0x6c], eax
001126a0: mov    eax, dword ptr [rsi + 0x4c]
001126a3: mov    dword ptr [rbx + 0x70], eax
001126a6: mov    eax, dword ptr [rsi + 0x68]
001126a9: mov    dword ptr [rbx + 0x74], eax
001126ac: movsd  xmm0, qword ptr [rsi + 0x70]
001126b1: cvtpd2ps xmm0, xmm0
001126b5: movss  dword ptr [rbx + 0x78], xmm0
001126ba: mov    rax, qword ptr [rsi + 0x58]
001126be: mov    qword ptr [rbx + 0x80], rax
001126c5: mov    rax, qword ptr [rsi + 0x60]
001126c9: mov    qword ptr [rbx + 0x88], rax
001126d0: mov    eax, dword ptr [rsi + 0x94]
001126d6: mov    dword ptr [rbx + 0x90], eax
001126dc: call   0x182130570                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʼʴʺˁʾʲʳʷʽʾʵ
001126e1: movss  dword ptr [rbx + 0x94], xmm0
001126e9: mov    rcx, rsi
001126ec: mov    eax, dword ptr [rsi + 0x8c]
001126f2: mov    dword ptr [rbx + 0x98], eax
001126f8: mov    eax, dword ptr [rsi + 0x90]
001126fe: mov    dword ptr [rbx + 0x9c], eax
00112704: mov    eax, dword ptr [rsi + 0x78]
00112707: mov    dword ptr [rbx + 0xa0], eax
0011270d: mov    eax, dword ptr [rsi + 0x84]
00112713: mov    dword ptr [rbx + 0xa4], eax
00112719: mov    eax, dword ptr [rsi + 0x88]
0011271f: mov    dword ptr [rbx + 0xa8], eax
00112725: mov    eax, dword ptr [rsi + 0x7c]
00112728: mov    dword ptr [rbx + 0xac], eax
0011272e: mov    eax, dword ptr [rsi + 0x80]
00112734: mov    dword ptr [rbx + 0xb0], eax
0011273a: mov    eax, dword ptr [rbx + 0x5c]
0011273d: cmp    dword ptr [rbx + 0x60], eax
00112740: sete   al
00112743: xor    edx, edx
00112745: mov    byte ptr [rbx + 0xb4], al
0011274b: call   0x18212f4f0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʴʻʽʵʳˀʿʵʻʲ
00112750: mov    byte ptr [rbx + 0xb7], al
00112756: xor    edx, edx
00112758: movzx  eax, byte ptr [rsi + 0xa9]
0011275f: mov    rcx, rdi
00112762: mov    byte ptr [rbx + 0xb5], al
00112768: call   0x18013f8b0                              ; ʳʼʽʼˀʴʽʵʵʳʵ$$ˀʵʲʲʻʸʸʽʹʽˁ
0011276d: mov    byte ptr [rbx + 0xb6], al
00112773: lea    rcx, [rbx + 0xc0]
0011277a: mov    rdx, qword ptr [rsi + 0xa0]
00112781: mov    qword ptr [rbx + 0xc0], rdx
00112788: call   0x182f5fc00
0011278d: mov    eax, dword ptr [rsi + 0x1c]
00112790: mov    dword ptr [rbx + 0x54], eax
00112793: mov    eax, dword ptr [rsi + 0x98]
00112799: mov    rsi, qword ptr [rsp + 0x38]
0011279e: mov    dword ptr [rbx + 0xb8], eax
001127a4: mov    rbx, qword ptr [rsp + 0x30]
001127a9: add    rsp, 0x20
001127ad: pop    rdi
001127ae: ret    
001127af: call   0x182f60c50
001127b4: int3   
001127b5: int3   
