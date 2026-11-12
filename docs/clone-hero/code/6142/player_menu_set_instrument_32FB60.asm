0032fb60: mov    qword ptr [rsp + 8], rbx
0032fb65: push   rdi
0032fb66: sub    rsp, 0x20
0032fb6a: cmp    byte ptr [rip + 0x3bde1c9], 0            ; [0x3f0dd3a] (bss)
0032fb71: mov    rbx, rcx
0032fb74: jne    0x18032fc1d
0032fb7a: lea    rcx, [rip + 0x39cfd7f]                   ; [0x3cff900] str:'6 Fret Lead Guitar'
0032fb81: call   0x182f609b0
0032fb86: lea    rcx, [rip + 0x398a7eb]                   ; [0x3cba378] str:'Keys'
0032fb8d: call   0x182f609b0
0032fb92: lea    rcx, [rip + 0x39cfbef]                   ; [0x3cff788] str:'6 Fret Guitar Coop'
0032fb99: call   0x182f609b0
0032fb9e: lea    rcx, [rip + 0x39cfca3]                   ; [0x3cff848] str:'6 Fret Keys'
0032fba5: call   0x182f609b0
0032fbaa: lea    rcx, [rip + 0x3978b37]                   ; [0x3ca86e8] str:'Pro Drums'
0032fbb1: call   0x182f609b0
0032fbb6: lea    rcx, [rip + 0x398e51b]                   ; [0x3cbe0d8] str:'Lead Guitar'
0032fbbd: call   0x182f609b0
0032fbc2: lea    rcx, [rip + 0x39b9f77]                   ; [0x3ce9b40] str:'No Part'
0032fbc9: call   0x182f609b0
0032fbce: lea    rcx, [rip + 0x39cfea3]                   ; [0x3cffa78] str:'6 Fret Rhythm Guitar'
0032fbd5: call   0x182f609b0
0032fbda: lea    rcx, [rip + 0x398745f]                   ; [0x3cb7040] str:'Bass Guitar'
0032fbe1: call   0x182f609b0
0032fbe6: lea    rcx, [rip + 0x39714e3]                   ; [0x3ca10d0] str:'Drums'
0032fbed: call   0x182f609b0
0032fbf2: lea    rcx, [rip + 0x39cfa17]                   ; [0x3cff610] str:'6 Fret Bass Guitar'
0032fbf9: call   0x182f609b0
0032fbfe: lea    rcx, [rip + 0x39bedb3]                   ; [0x3cee9b8] str:'Guitar Coop'
0032fc05: call   0x182f609b0
0032fc0a: lea    rcx, [rip + 0x39933b7]                   ; [0x3cc2fc8] str:'Rhythm Guitar'
0032fc11: call   0x182f609b0
0032fc16: mov    byte ptr [rip + 0x3bde11d], 1            ; [0x3f0dd3a] (bss)
0032fc1d: mov    rdi, qword ptr [rbx + 0x218]
0032fc24: xor    edx, edx
0032fc26: mov    rcx, rbx
0032fc29: call   0x1801d5c00                              ; BaseMenu$$ʹʴʹˀʲʾʽʲʹˀʴ
0032fc2e: test   rdi, rdi
0032fc31: je     0x18032ff58
0032fc37: cmp    eax, dword ptr [rdi + 0x18]
0032fc3a: jae    0x18032ff5e
0032fc40: cdqe   
0032fc42: xor    edx, edx
0032fc44: mov    rdi, qword ptr [rdi + rax*8 + 0x20]
0032fc49: mov    rcx, rdi
0032fc4c: call   0x180103fe0                              ; <PrivateImplementationDetails>$$ComputeStringHash
0032fc51: cmp    eax, 0x6ed73ea1
0032fc56: jbe    0x18032fdb5
0032fc5c: cmp    eax, 0x78d7ccc0
0032fc61: jbe    0x18032fd2a
0032fc67: cmp    eax, 0x879475bc
0032fc6c: jbe    0x18032fccc
0032fc6e: cmp    eax, 0x9a6c749a
0032fc73: jne    0x18032fc9b
0032fc75: mov    rdx, qword ptr [rip + 0x39cfc84]         ; [0x3cff900] str:'6 Fret Lead Guitar'
0032fc7c: xor    r8d, r8d
0032fc7f: mov    rcx, rdi
0032fc82: call   0x181981570                              ; System.String$$op_Equality
0032fc87: test   al, al
0032fc89: je     0x18032feb4
0032fc8f: mov    byte ptr [rbx + 0x240], 4
0032fc96: jmp    0x18032feb4
0032fc9b: cmp    eax, 0xec4b77b1
0032fca0: jne    0x18032feb4
0032fca6: mov    rdx, qword ptr [rip + 0x39cfadb]         ; [0x3cff788] str:'6 Fret Guitar Coop'
0032fcad: xor    r8d, r8d
0032fcb0: mov    rcx, rdi
0032fcb3: call   0x181981570                              ; System.String$$op_Equality
0032fcb8: test   al, al
0032fcba: je     0x18032feb4
0032fcc0: mov    byte ptr [rbx + 0x240], 0xb
0032fcc7: jmp    0x18032feb4
0032fccc: cmp    eax, 0x85ac7c5a
0032fcd1: jne    0x18032fcf9
0032fcd3: mov    rdx, qword ptr [rip + 0x39713f6]         ; [0x3ca10d0] str:'Drums'
0032fcda: xor    r8d, r8d
0032fcdd: mov    rcx, rdi
0032fce0: call   0x181981570                              ; System.String$$op_Equality
0032fce5: test   al, al
0032fce7: je     0x18032feb4
0032fced: mov    byte ptr [rbx + 0x240], 6
0032fcf4: jmp    0x18032feb4
0032fcf9: cmp    eax, 0x879475bc
0032fcfe: jne    0x18032feb4
0032fd04: mov    rdx, qword ptr [rip + 0x39cfd6d]         ; [0x3cffa78] str:'6 Fret Rhythm Guitar'
0032fd0b: xor    r8d, r8d
0032fd0e: mov    rcx, rdi
0032fd11: call   0x181981570                              ; System.String$$op_Equality
0032fd16: test   al, al
0032fd18: je     0x18032feb4
0032fd1e: mov    byte ptr [rbx + 0x240], 0xa
0032fd25: jmp    0x18032feb4
0032fd2a: cmp    eax, 0x734b95f0
0032fd2f: jne    0x18032fd57
0032fd31: mov    rdx, qword ptr [rip + 0x39bec80]         ; [0x3cee9b8] str:'Guitar Coop'
0032fd38: xor    r8d, r8d
0032fd3b: mov    rcx, rdi
0032fd3e: call   0x181981570                              ; System.String$$op_Equality
0032fd43: test   al, al
0032fd45: je     0x18032feb4
0032fd4b: mov    byte ptr [rbx + 0x240], 3
0032fd52: jmp    0x18032feb4
0032fd57: cmp    eax, 0x78a98107
0032fd5c: jne    0x18032fd84
0032fd5e: mov    rdx, qword ptr [rip + 0x39b9ddb]         ; [0x3ce9b40] str:'No Part'
0032fd65: xor    r8d, r8d
0032fd68: mov    rcx, rdi
0032fd6b: call   0x181981570                              ; System.String$$op_Equality
0032fd70: test   al, al
0032fd72: je     0x18032feb4
0032fd78: mov    byte ptr [rbx + 0x240], 0xf
0032fd7f: jmp    0x18032feb4
0032fd84: cmp    eax, 0x78d7ccc0
0032fd89: jne    0x18032feb4
0032fd8f: mov    rdx, qword ptr [rip + 0x39872aa]         ; [0x3cb7040] str:'Bass Guitar'
0032fd96: xor    r8d, r8d
0032fd99: mov    rcx, rdi
0032fd9c: call   0x181981570                              ; System.String$$op_Equality
0032fda1: test   al, al
0032fda3: je     0x18032feb4
0032fda9: mov    byte ptr [rbx + 0x240], 1
0032fdb0: jmp    0x18032feb4
0032fdb5: cmp    eax, 0x602b32ed
0032fdba: jbe    0x18032fe44
0032fdc0: cmp    eax, 0x62379537
0032fdc5: jne    0x18032fded
0032fdc7: mov    rdx, qword ptr [rip + 0x398e30a]         ; [0x3cbe0d8] str:'Lead Guitar'
0032fdce: xor    r8d, r8d
0032fdd1: mov    rcx, rdi
0032fdd4: call   0x181981570                              ; System.String$$op_Equality
0032fdd9: test   al, al
0032fddb: je     0x18032feb4
0032fde1: mov    byte ptr [rbx + 0x240], 0
0032fde8: jmp    0x18032feb4
0032fded: cmp    eax, 0x6d91b636
0032fdf2: jne    0x18032fe1a
0032fdf4: mov    rdx, qword ptr [rip + 0x39cfa4d]         ; [0x3cff848] str:'6 Fret Keys'
0032fdfb: xor    r8d, r8d
0032fdfe: mov    rcx, rdi
0032fe01: call   0x181981570                              ; System.String$$op_Equality
0032fe06: test   al, al
0032fe08: je     0x18032feb4
0032fe0e: mov    byte ptr [rbx + 0x240], 0xe
0032fe15: jmp    0x18032feb4
0032fe1a: cmp    eax, 0x6ed73ea1
0032fe1f: jne    0x18032feb4
0032fe25: mov    rdx, qword ptr [rip + 0x39788bc]         ; [0x3ca86e8] str:'Pro Drums'
0032fe2c: xor    r8d, r8d
0032fe2f: mov    rcx, rdi
0032fe32: call   0x181981570                              ; System.String$$op_Equality
0032fe37: test   al, al
0032fe39: je     0x18032feb4
0032fe3b: mov    byte ptr [rbx + 0x240], 9
0032fe42: jmp    0x18032feb4
0032fe44: cmp    eax, 0x58f195
0032fe49: jne    0x18032fe6a
0032fe4b: mov    rdx, qword ptr [rip + 0x39cf7be]         ; [0x3cff610] str:'6 Fret Bass Guitar'
0032fe52: xor    r8d, r8d
0032fe55: mov    rcx, rdi
0032fe58: call   0x181981570                              ; System.String$$op_Equality
0032fe5d: test   al, al
0032fe5f: je     0x18032feb4
0032fe61: mov    byte ptr [rbx + 0x240], 5
0032fe68: jmp    0x18032feb4
0032fe6a: cmp    eax, 0x54cf5575
0032fe6f: jne    0x18032fe90
0032fe71: mov    rdx, qword ptr [rip + 0x3993150]         ; [0x3cc2fc8] str:'Rhythm Guitar'
0032fe78: xor    r8d, r8d
0032fe7b: mov    rcx, rdi
0032fe7e: call   0x181981570                              ; System.String$$op_Equality
0032fe83: test   al, al
0032fe85: je     0x18032feb4
0032fe87: mov    byte ptr [rbx + 0x240], 2
0032fe8e: jmp    0x18032feb4
0032fe90: cmp    eax, 0x602b32ed
0032fe95: jne    0x18032feb4
0032fe97: mov    rdx, qword ptr [rip + 0x398a4da]         ; [0x3cba378] str:'Keys'
0032fe9e: xor    r8d, r8d
0032fea1: mov    rcx, rdi
0032fea4: call   0x181981570                              ; System.String$$op_Equality
0032fea9: test   al, al
0032feab: je     0x18032feb4
0032fead: mov    byte ptr [rbx + 0x240], 7
0032feb4: mov    rax, qword ptr [rbx + 0x20]
0032feb8: test   rax, rax
0032febb: je     0x18032ff58
0032fec1: mov    rcx, qword ptr [rax + 0x10]
0032fec5: test   rcx, rcx
0032fec8: je     0x18032ff58
0032fece: movzx  eax, byte ptr [rbx + 0x240]
0032fed5: mov    byte ptr [rcx + 0x10], al
0032fed8: mov    rax, qword ptr [rbx + 0x20]
0032fedc: test   rax, rax
0032fedf: je     0x18032ff58
0032fee1: mov    rcx, qword ptr [rax + 0x10]
0032fee5: test   rcx, rcx
0032fee8: je     0x18032ff58
0032feea: xor    edx, edx
0032feec: call   0x182123320                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʾˁʺʴʻʵʳʾʲʺ
0032fef1: cmp    al, 2
0032fef3: jne    0x18032fef9
0032fef5: mov    al, 1
0032fef7: jmp    0x18032fefe
0032fef9: cmp    al, 3
0032fefb: sete   al
0032fefe: mov    byte ptr [rbx + 0x242], al
0032ff04: mov    rax, qword ptr [rbx + 0x20]
0032ff08: test   rax, rax
0032ff0b: je     0x18032ff58
0032ff0d: mov    rcx, qword ptr [rax + 0x10]
0032ff11: test   rcx, rcx
0032ff14: je     0x18032ff58
0032ff16: xor    edx, edx
0032ff18: call   0x182123150                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʴʶʾʶˀʺʾʲʸʻ
0032ff1d: cmp    byte ptr [rbx + 0x240], 0xf
0032ff24: mov    byte ptr [rbx + 0x243], al
0032ff2a: jne    0x18032ff35
0032ff2c: mov    word ptr [rbx + 0x1f0], 0
0032ff35: movzx  eax, byte ptr [rbx + 0x1f0]
0032ff3c: mov    rcx, rbx
0032ff3f: neg    al
0032ff41: sbb    edx, edx
0032ff43: xor    r8d, r8d
0032ff46: and    edx, 2
0032ff49: mov    rbx, qword ptr [rsp + 0x30]
0032ff4e: add    rsp, 0x20
0032ff52: pop    rdi
0032ff53: jmp    0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032ff58: call   0x182f60c50
0032ff5d: int3   
0032ff5e: call   0x182f60c40
0032ff63: int3   
0032ff64: int3   
