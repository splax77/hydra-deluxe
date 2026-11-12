0032e190: push   rbx
0032e192: sub    rsp, 0x20
0032e196: cmp    byte ptr [rip + 0x3bdfb94], 0            ; [0x3f0dd31] (bss)
0032e19d: mov    rbx, rcx
0032e1a0: jne    0x18032e1b5
0032e1a2: lea    rcx, [rip + 0x39e0e9f]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032e1a9: call   0x182f609b0
0032e1ae: mov    byte ptr [rip + 0x3bdfb7c], 1            ; [0x3f0dd31] (bss)
0032e1b5: cmp    dword ptr [rbx + 0x120], 4
0032e1bc: je     0x18032e53e
0032e1c2: mov    qword ptr [rsp + 0x30], rdi
0032e1c7: xor    edx, edx
0032e1c9: mov    rcx, rbx
0032e1cc: call   0x1801d5620                              ; BaseMenu$$ʵˁʺʻʲʹˁʻʹʺʳ
0032e1d1: mov    ecx, dword ptr [rbx + 0x120]
0032e1d7: test   ecx, ecx
0032e1d9: je     0x18032e3db
0032e1df: sub    ecx, 1
0032e1e2: je     0x18032e3cc
0032e1e8: sub    ecx, 1
0032e1eb: je     0x18032e264
0032e1ed: cmp    ecx, 1
0032e1f0: jne    0x18032e510
0032e1f6: mov    edi, dword ptr [rbx + 0x40]
0032e1f9: mov    rcx, rbx
0032e1fc: add    edi, dword ptr [rbx + 0x3c]
0032e1ff: je     0x18032e255
0032e201: xor    edx, edx
0032e203: call   0x18032ce60                              ; PlayerSelectionMenu$$ʿʻʺʻʼˁʼˀʻʿʸ
0032e208: test   rax, rax
0032e20b: je     0x18032e544
0032e211: movsxd rdx, edi
0032e214: dec    rdx
0032e217: cmp    edx, dword ptr [rax + 0x18]
0032e21a: jae    0x18032e54a
0032e220: mov    rcx, qword ptr [rbx + 0x20]
0032e224: test   rcx, rcx
0032e227: je     0x18032e544
0032e22d: mov    rcx, qword ptr [rcx + 0x10]
0032e231: test   rcx, rcx
0032e234: je     0x18032e544
0032e23a: mov    edx, dword ptr [rax + rdx*4 + 0x20]
0032e23e: xor    r8d, r8d
0032e241: call   0x1821236d0                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʿʵʹʽˁˀʿʴʼʷʾ
0032e246: xor    edx, edx
0032e248: mov    rcx, rbx
0032e24b: call   0x18032d400                              ; PlayerSelectionMenu$$ʵʲʴʸʶʿʴʴʹʻʼ
0032e250: jmp    0x18032e452
0032e255: xor    r8d, r8d
0032e258: xor    edx, edx
0032e25a: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e25f: jmp    0x18032e510
0032e264: cmp    byte ptr [rip + 0x3bdfacc], 0            ; [0x3f0dd37] (bss)
0032e26b: jne    0x18032e2b0
0032e26d: lea    rcx, [rip + 0x39c7cf4]                   ; [0x3cf5f68] str:'Hard'
0032e274: call   0x182f609b0
0032e279: lea    rcx, [rip + 0x39b62e8]                   ; [0x3ce4568] str:'Change Instrument'
0032e280: call   0x182f609b0
0032e285: lea    rcx, [rip + 0x3979cac]                   ; [0x3ca7f38] str:'Easy'
0032e28c: call   0x182f609b0
0032e291: lea    rcx, [rip + 0x398d9c0]                   ; [0x3cbbc58] str:'Expert'
0032e298: call   0x182f609b0
0032e29d: lea    rcx, [rip + 0x39a47fc]                   ; [0x3cd2aa0] str:'Medium'
0032e2a4: call   0x182f609b0
0032e2a9: mov    byte ptr [rip + 0x3bdfa87], 1            ; [0x3f0dd37] (bss)
0032e2b0: mov    rdi, qword ptr [rbx + 0x218]
0032e2b7: test   rdi, rdi
0032e2ba: je     0x18032e544
0032e2c0: movsxd rax, dword ptr [rbx + 0x40]
0032e2c4: cmp    eax, dword ptr [rdi + 0x18]
0032e2c7: jae    0x18032e54a
0032e2cd: mov    rdi, qword ptr [rdi + rax*8 + 0x20]
0032e2d2: xor    r8d, r8d
0032e2d5: mov    rdx, qword ptr [rip + 0x398d97c]         ; [0x3cbbc58] str:'Expert'
0032e2dc: mov    rcx, rdi
0032e2df: call   0x181981570                              ; System.String$$op_Equality
0032e2e4: test   al, al
0032e2e6: je     0x18032e333
0032e2e8: mov    byte ptr [rbx + 0x241], 3
0032e2ef: mov    rax, qword ptr [rbx + 0x20]
0032e2f3: test   rax, rax
0032e2f6: je     0x18032e544
0032e2fc: mov    rcx, qword ptr [rax + 0x10]
0032e300: test   rcx, rcx
0032e303: je     0x18032e544
0032e309: movzx  eax, byte ptr [rbx + 0x241]
0032e310: mov    byte ptr [rcx + 0x11], al
0032e313: movzx  eax, byte ptr [rbx + 0x1f1]
0032e31a: neg    al
0032e31c: sbb    ecx, ecx
0032e31e: xor    r8d, r8d
0032e321: and    ecx, 3
0032e324: mov    edx, ecx
0032e326: mov    rcx, rbx
0032e329: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e32e: jmp    0x18032e510
0032e333: mov    rdx, qword ptr [rip + 0x39c7c2e]         ; [0x3cf5f68] str:'Hard'
0032e33a: xor    r8d, r8d
0032e33d: mov    rcx, rdi
0032e340: call   0x181981570                              ; System.String$$op_Equality
0032e345: test   al, al
0032e347: je     0x18032e352
0032e349: mov    byte ptr [rbx + 0x241], 2
0032e350: jmp    0x18032e2ef
0032e352: mov    rdx, qword ptr [rip + 0x39a4747]         ; [0x3cd2aa0] str:'Medium'
0032e359: xor    r8d, r8d
0032e35c: mov    rcx, rdi
0032e35f: call   0x181981570                              ; System.String$$op_Equality
0032e364: test   al, al
0032e366: je     0x18032e374
0032e368: mov    byte ptr [rbx + 0x241], 1
0032e36f: jmp    0x18032e2ef
0032e374: mov    rdx, qword ptr [rip + 0x3979bbd]         ; [0x3ca7f38] str:'Easy'
0032e37b: xor    r8d, r8d
0032e37e: mov    rcx, rdi
0032e381: call   0x181981570                              ; System.String$$op_Equality
0032e386: test   al, al
0032e388: je     0x18032e396
0032e38a: mov    byte ptr [rbx + 0x241], 0
0032e391: jmp    0x18032e2ef
0032e396: mov    rdx, qword ptr [rip + 0x39b61cb]         ; [0x3ce4568] str:'Change Instrument'
0032e39d: xor    r8d, r8d
0032e3a0: mov    rcx, rdi
0032e3a3: call   0x181981570                              ; System.String$$op_Equality
0032e3a8: test   al, al
0032e3aa: je     0x18032e2ef
0032e3b0: mov    edx, 1
0032e3b5: mov    byte ptr [rbx + 0x1f0], 1
0032e3bc: xor    r8d, r8d
0032e3bf: mov    rcx, rbx
0032e3c2: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e3c7: jmp    0x18032e510
0032e3cc: xor    edx, edx
0032e3ce: mov    rcx, rbx
0032e3d1: call   0x18032fb60                              ; PlayerSelectionMenu$$ʹʻʽʷˀʿˀʿʸˁʼ
0032e3d6: jmp    0x18032e510
0032e3db: cmp    byte ptr [rip + 0x3bdf950], 0            ; [0x3f0dd32] (bss)
0032e3e2: jne    0x18032e3f7
0032e3e4: lea    rcx, [rip + 0x399542d]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032e3eb: call   0x182f609b0
0032e3f0: mov    byte ptr [rip + 0x3bdf93b], 1            ; [0x3f0dd32] (bss)
0032e3f7: mov    ecx, dword ptr [rbx + 0x40]
0032e3fa: test   ecx, ecx
0032e3fc: je     0x18032e46a
0032e3fe: sub    ecx, 1
0032e401: je     0x18032e43b
0032e403: sub    ecx, 1
0032e406: je     0x18032e426
0032e408: cmp    ecx, 1
0032e40b: jne    0x18032e510
0032e411: xor    r8d, r8d
0032e414: mov    edx, 3
0032e419: mov    rcx, rbx
0032e41c: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e421: jmp    0x18032e510
0032e426: xor    r8d, r8d
0032e429: mov    edx, 2
0032e42e: mov    rcx, rbx
0032e431: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e436: jmp    0x18032e510
0032e43b: xor    r8d, r8d
0032e43e: mov    byte ptr [rbx + 0x246], 0
0032e445: mov    edx, 1
0032e44a: mov    rcx, rbx
0032e44d: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e452: mov    rax, qword ptr [rbx]
0032e455: mov    rcx, rbx
0032e458: mov    rdx, qword ptr [rax + 0x440]
0032e45f: call   qword ptr [rax + 0x438]
0032e465: jmp    0x18032e510
0032e46a: cmp    byte ptr [rbx + 0x245], 0
0032e471: jne    0x18032e498
0032e473: mov    rax, qword ptr [rip + 0x399539e]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032e47a: mov    rcx, qword ptr [rax + 0xb8]
0032e481: mov    rcx, qword ptr [rcx]
0032e484: test   rcx, rcx
0032e487: je     0x18032e544
0032e48d: xor    edx, edx
0032e48f: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0032e494: test   al, al
0032e496: jne    0x18032e506
0032e498: mov    r9, qword ptr [rbx + 0x238]
0032e49f: test   r9, r9
0032e4a2: je     0x18032e544
0032e4a8: mov    r8, qword ptr [rbx + 0x20]
0032e4ac: test   r8, r8
0032e4af: je     0x18032e544
0032e4b5: movsx  ecx, byte ptr [rbx + 0x240]
0032e4bc: mov    edx, 1
0032e4c1: movsx  eax, byte ptr [rbx + 0x241]
0032e4c8: lea    ecx, [rax + rcx*4]
0032e4cb: shl    rdx, cl
0032e4ce: mov    rax, qword ptr [r9 + 0x20]
0032e4d2: mov    rcx, qword ptr [r8 + 0x10]
0032e4d6: and    rax, rdx
0032e4d9: cmp    rax, rdx
0032e4dc: jne    0x18032e4f4
0032e4de: test   rcx, rcx
0032e4e1: je     0x18032e544
0032e4e3: movzx  r8d, byte ptr [rbx + 0x241]
0032e4eb: movzx  edx, byte ptr [rbx + 0x240]
0032e4f2: jmp    0x18032e4fe
0032e4f4: test   rcx, rcx
0032e4f7: je     0x18032e544
0032e4f9: mov    r8b, 3
0032e4fc: mov    dl, 0xf
0032e4fe: xor    r9d, r9d
0032e501: call   0x1800d86d0                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʾʵʽʳʳˁʼʼʾʶʷ
0032e506: xor    edx, edx
0032e508: mov    rcx, rbx
0032e50b: call   0x18032ff70                              ; PlayerSelectionMenu$$ʺʵʲʿʴˁʼʳʺʸˀ
0032e510: cmp    byte ptr [rbx + 0x245], 0
0032e517: je     0x18032e539
0032e519: mov    rax, qword ptr [rip + 0x39e0b28]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032e520: mov    rcx, qword ptr [rax + 0xb8]
0032e527: mov    rcx, qword ptr [rcx]
0032e52a: test   rcx, rcx
0032e52d: je     0x18032e544
0032e52f: xor    r8d, r8d
0032e532: xor    edx, edx
0032e534: call   0x180092910                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʿʹˀʿʺʺʲʺʸʵʼ
0032e539: mov    rdi, qword ptr [rsp + 0x30]
0032e53e: add    rsp, 0x20
0032e542: pop    rbx
0032e543: ret    
0032e544: call   0x182f60c50
0032e549: int3   
0032e54a: call   0x182f60c40
0032e54f: int3   
0032e550: push   rbx
0032e552: sub    rsp, 0x20
0032e556: cmp    byte ptr [rip + 0x3bdf7cc], 0            ; [0x3f0dd29] (bss)
0032e55d: mov    rbx, rcx
0032e560: jne    0x18032e575
0032e562: lea    rcx, [rip + 0x39952af]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032e569: call   0x182f609b0
0032e56e: mov    byte ptr [rip + 0x3bdf7b4], 1            ; [0x3f0dd29] (bss)
0032e575: mov    ecx, dword ptr [rbx + 0x40]
0032e578: test   ecx, ecx
0032e57a: je     0x18032e5eb
0032e57c: sub    ecx, 1
0032e57f: je     0x18032e5bb
0032e581: sub    ecx, 1
0032e584: je     0x18032e5a6
0032e586: cmp    ecx, 1
0032e589: jne    0x18032e5a0
0032e58b: xor    r8d, r8d
0032e58e: mov    edx, 5
0032e593: mov    rcx, rbx
0032e596: add    rsp, 0x20
0032e59a: pop    rbx
0032e59b: jmp    0x180330350                              ; PlayerSelectionMenu$$ʻʺʵˀʺʵʼʲʼʻʽ
0032e5a0: add    rsp, 0x20
0032e5a4: pop    rbx
0032e5a5: ret    
0032e5a6: xor    r8d, r8d
0032e5a9: mov    edx, 6
0032e5ae: mov    rcx, rbx
0032e5b1: add    rsp, 0x20
0032e5b5: pop    rbx
0032e5b6: jmp    0x180330350                              ; PlayerSelectionMenu$$ʻʺʵˀʺʵʼʲʼʻʽ
0032e5bb: xor    r8d, r8d
0032e5be: mov    byte ptr [rbx + 0x246], 0
0032e5c5: mov    edx, 1
0032e5ca: mov    rcx, rbx
0032e5cd: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032e5d2: mov    rax, qword ptr [rbx]
0032e5d5: mov    rcx, rbx
0032e5d8: mov    rdx, qword ptr [rax + 0x440]
0032e5df: add    rsp, 0x20
0032e5e3: pop    rbx
0032e5e4: jmp    qword ptr [rax + 0x438]
0032e5eb: cmp    byte ptr [rbx + 0x245], 0
0032e5f2: jne    0x18032e615
0032e5f4: mov    rax, qword ptr [rip + 0x399521d]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032e5fb: mov    rcx, qword ptr [rax + 0xb8]
0032e602: mov    rcx, qword ptr [rcx]
0032e605: test   rcx, rcx
0032e608: je     0x18032e684
0032e60a: xor    edx, edx
0032e60c: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0032e611: test   al, al
0032e613: jne    0x18032e675
0032e615: mov    rcx, qword ptr [rbx + 0x238]
0032e61c: test   rcx, rcx
0032e61f: je     0x18032e684
0032e621: movzx  r8d, byte ptr [rbx + 0x241]
0032e629: add    rcx, 0x20
0032e62d: movzx  edx, byte ptr [rbx + 0x240]
0032e634: xor    r9d, r9d
0032e637: call   0x180006d50                              ; System.IO.Pipelines.PipeWriterStream$$get_CanWrite
0032e63c: mov    rcx, qword ptr [rbx + 0x20]
0032e640: test   rcx, rcx
0032e643: je     0x18032e684
0032e645: mov    rcx, qword ptr [rcx + 0x10]
0032e649: test   al, al
0032e64b: je     0x18032e663
0032e64d: test   rcx, rcx
0032e650: je     0x18032e684
0032e652: movzx  r8d, byte ptr [rbx + 0x241]
0032e65a: movzx  edx, byte ptr [rbx + 0x240]
0032e661: jmp    0x18032e66d
0032e663: test   rcx, rcx
0032e666: je     0x18032e684
0032e668: xor    r8d, r8d
0032e66b: mov    dl, 0x82
0032e66d: xor    r9d, r9d
0032e670: call   0x1800d86d0                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʾʵʽʳʳˁʼʼʾʶʷ
0032e675: xor    edx, edx
0032e677: mov    rcx, rbx
0032e67a: add    rsp, 0x20
0032e67e: pop    rbx
0032e67f: jmp    0x18032ff70                              ; PlayerSelectionMenu$$ʺʵʲʿʴˁʼʳʺʸˀ
0032e684: call   0x182f60c50
0032e689: int3   
0032e68a: int3   
