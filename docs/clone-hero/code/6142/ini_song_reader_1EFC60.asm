; function 0x1efc60 ʵʴʺˁʵʽʾʹʸʹˀ$$ʺʻʸʽʿʵʴʾʵʴʲ
001efc60: push   rbp
001efc62: push   rsi
001efc63: push   rdi
001efc64: mov    rbp, rsp
001efc67: sub    rsp, 0x30
001efc6b: cmp    byte ptr [rip + 0x3d1cede], 0            ; [0x3f0cb50] (bss)
001efc72: mov    rdi, rdx
001efc75: mov    rsi, rcx
001efc78: jne    0x1801efd15
001efc7e: lea    rcx, [rip + 0x3b060bb]                   ; [0x3cf5d40] str:'video_start_time'
001efc85: call   0x182f609b0
001efc8a: lea    rcx, [rip + 0x3adc89f]                   ; [0x3ccc530] str:'sustain_cutoff_threshold'
001efc91: call   0x182f609b0
001efc96: lea    rcx, [rip + 0x3afecab]                   ; [0x3cee948] str:'eighthnote_hopo'
001efc9d: call   0x182f609b0
001efca2: lea    rcx, [rip + 0x3ae313f]                   ; [0x3cd2de8] str:'loading_phrase'
001efca9: call   0x182f609b0
001efcae: lea    rcx, [rip + 0x3aed5c3]                   ; [0x3cdd278] str:'delay'
001efcb5: call   0x182f609b0
001efcba: lea    rcx, [rip + 0x3b0d6f7]                   ; [0x3cfd3b8] str:'five_lane_drums'
001efcc1: call   0x182f609b0
001efcc6: lea    rcx, [rip + 0x3ac9033]                   ; [0x3cb8d00] str:'song'
001efccd: call   0x182f609b0
001efcd2: lea    rcx, [rip + 0x3ad2bf7]                   ; [0x3cc28d0] str:'chord_snap_threshold'
001efcd9: call   0x182f609b0
001efcde: lea    rcx, [rip + 0x3af9a23]                   ; [0x3ce9708] str:'multiplier_note'
001efce5: call   0x182f609b0
001efcea: lea    rcx, [rip + 0x3aa80bf]                   ; [0x3c97db0] str:'pro_drums'
001efcf1: call   0x182f609b0
001efcf6: lea    rcx, [rip + 0x3ac4173]                   ; [0x3cb3e70] str:''
001efcfd: call   0x182f609b0
001efd02: lea    rcx, [rip + 0x3ab8427]                   ; [0x3ca8130] str:'hopo_frequency'
001efd09: call   0x182f609b0
001efd0e: mov    byte ptr [rip + 0x3d1ce3b], 1            ; [0x3f0cb50] (bss)
001efd15: test   rdi, rdi
001efd18: je     0x1801eff83
001efd1e: mov    r8, qword ptr [rip + 0x3aed553]          ; [0x3cdd278] str:'delay'
001efd25: xor    r9d, r9d
001efd28: mov    rdx, qword ptr [rip + 0x3ac8fd1]         ; [0x3cb8d00] str:'song'
001efd2f: mov    rcx, rdi
001efd32: mov    qword ptr [rsp + 0x50], rbx
001efd37: mov    qword ptr [rsp + 0x60], r14
001efd3c: xor    r14d, r14d
001efd3f: mov    qword ptr [rsp + 0x20], r14
001efd44: call   0x1821647a0                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʲʿʿʺʶʻʿʻʵʺʹ
001efd49: mov    dword ptr [rsi + 0x24], eax
001efd4c: lea    r9, [rbp + 0x38]
001efd50: mov    r8, qword ptr [rip + 0x3ab83d9]          ; [0x3ca8130] str:'hopo_frequency'
001efd57: mov    rcx, rdi
001efd5a: mov    rdx, qword ptr [rip + 0x3ac8f9f]         ; [0x3cb8d00] str:'song'
001efd61: mov    qword ptr [rbp + 0x38], r14
001efd65: mov    dword ptr [rbp + 0x28], r14d
001efd69: mov    qword ptr [rsp + 0x20], r14
001efd6e: call   0x1801bd150
001efd73: test   al, al
001efd75: je     0x1801efd9f
001efd77: mov    rcx, qword ptr [rbp + 0x38]
001efd7b: lea    rdx, [rbp + 0x28]
001efd7f: xor    r8d, r8d
001efd82: call   0x181b4f710                              ; System.Int32$$TryParse
001efd87: test   al, al
001efd89: je     0x1801efd9f
001efd8b: mov    eax, dword ptr [rbp + 0x28]
001efd8e: test   eax, eax
001efd90: js     0x1801efd9b
001efd92: mov    ecx, 0xffff
001efd97: cmp    eax, ecx
001efd99: jg     0x1801efda8
001efd9b: mov    ecx, eax
001efd9d: jmp    0x1801efda2
001efd9f: mov    ecx, r14d
001efda2: test   ecx, ecx
001efda4: cmovs  ecx, r14d
001efda8: mov    dword ptr [rsi + 0x18], ecx
001efdab: xor    r9d, r9d
001efdae: mov    r8, qword ptr [rip + 0x3b0d603]          ; [0x3cfd3b8] str:'five_lane_drums'
001efdb5: mov    rcx, rdi
001efdb8: mov    rdx, qword ptr [rip + 0x3ac8f41]         ; [0x3cb8d00] str:'song'
001efdbf: mov    qword ptr [rsp + 0x20], r14
001efdc4: call   0x1801d4480                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʸʺˁʳʹʹˁʶˁʸʵ
001efdc9: mov    byte ptr [rsi + 0x38], al
001efdcc: xor    r9d, r9d
001efdcf: mov    r8, qword ptr [rip + 0x3aa7fda]          ; [0x3c97db0] str:'pro_drums'
001efdd6: mov    rcx, rdi
001efdd9: mov    rdx, qword ptr [rip + 0x3ac8f20]         ; [0x3cb8d00] str:'song'
001efde0: mov    qword ptr [rsp + 0x20], r14
001efde5: call   0x1801d4480                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʸʺˁʳʹʹˁʶˁʸʵ
001efdea: mov    byte ptr [rsi + 0x39], al
001efded: xor    r9d, r9d
001efdf0: mov    r8, qword ptr [rip + 0x3afeb51]          ; [0x3cee948] str:'eighthnote_hopo'
001efdf7: mov    rcx, rdi
001efdfa: mov    rdx, qword ptr [rip + 0x3ac8eff]         ; [0x3cb8d00] str:'song'
001efe01: mov    qword ptr [rsp + 0x20], r14
001efe06: call   0x1801d4480                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʸʺˁʳʹʹˁʶˁʸʵ
001efe0b: mov    byte ptr [rsi + 0x10], al
001efe0e: lea    r9, [rbp + 0x38]
001efe12: mov    r8, qword ptr [rip + 0x3af98ef]          ; [0x3ce9708] str:'multiplier_note'
001efe19: mov    rcx, rdi
001efe1c: mov    rdx, qword ptr [rip + 0x3ac8edd]         ; [0x3cb8d00] str:'song'
001efe23: mov    qword ptr [rbp + 0x38], r14
001efe27: mov    dword ptr [rbp + 0x28], r14d
001efe2b: mov    qword ptr [rsp + 0x20], r14
001efe30: call   0x1801bd150
001efe35: test   al, al
001efe37: je     0x1801efe50
001efe39: mov    rcx, qword ptr [rbp + 0x38]
001efe3d: lea    rdx, [rbp + 0x28]
001efe41: xor    r8d, r8d
001efe44: call   0x181b4f710                              ; System.Int32$$TryParse
001efe49: test   al, al
001efe4b: mov    eax, dword ptr [rbp + 0x28]
001efe4e: jne    0x1801efe53
001efe50: mov    eax, r14d
001efe53: mov    dword ptr [rsi + 0x14], eax
001efe56: lea    r9, [rbp + 0x38]
001efe5a: mov    r8, qword ptr [rip + 0x3b05edf]          ; [0x3cf5d40] str:'video_start_time'
001efe61: mov    rcx, rdi
001efe64: mov    rdx, qword ptr [rip + 0x3ac8e95]         ; [0x3cb8d00] str:'song'
001efe6b: mov    qword ptr [rbp + 0x38], r14
001efe6f: mov    dword ptr [rbp + 0x28], r14d
001efe73: mov    qword ptr [rsp + 0x20], r14
001efe78: call   0x1801bd150
001efe7d: test   al, al
001efe7f: je     0x1801efe98
001efe81: mov    rcx, qword ptr [rbp + 0x38]
001efe85: lea    rdx, [rbp + 0x28]
001efe89: xor    r8d, r8d
001efe8c: call   0x181b4f710                              ; System.Int32$$TryParse
001efe91: test   al, al
001efe93: mov    eax, dword ptr [rbp + 0x28]
001efe96: jne    0x1801efe9b
001efe98: mov    eax, r14d
001efe9b: mov    dword ptr [rsi + 0x28], eax
001efe9e: lea    r9, [rbp + 0x38]
001efea2: mov    r8, qword ptr [rip + 0x3adc687]          ; [0x3ccc530] str:'sustain_cutoff_threshold'
001efea9: mov    rcx, rdi
001efeac: mov    rdx, qword ptr [rip + 0x3ac8e4d]         ; [0x3cb8d00] str:'song'
001efeb3: mov    qword ptr [rbp + 0x38], r14
001efeb7: mov    dword ptr [rbp + 0x28], r14d
001efebb: mov    qword ptr [rsp + 0x20], r14
001efec0: call   0x1801bd150
001efec5: test   al, al
001efec7: je     0x1801efee0
001efec9: mov    rcx, qword ptr [rbp + 0x38]
001efecd: lea    rdx, [rbp + 0x28]
001efed1: xor    r8d, r8d
001efed4: call   0x181b4f710                              ; System.Int32$$TryParse
001efed9: test   al, al
001efedb: mov    eax, dword ptr [rbp + 0x28]
001efede: jne    0x1801efee5
001efee0: mov    eax, 0xffffffff
001efee5: mov    dword ptr [rsi + 0x1c], eax
001efee8: lea    r9, [rbp + 0x38]
001efeec: mov    r8, qword ptr [rip + 0x3ad29dd]          ; [0x3cc28d0] str:'chord_snap_threshold'
001efef3: mov    rcx, rdi
001efef6: mov    rdx, qword ptr [rip + 0x3ac8e03]         ; [0x3cb8d00] str:'song'
001efefd: mov    qword ptr [rbp + 0x38], r14
001eff01: mov    dword ptr [rbp + 0x28], r14d
001eff05: mov    qword ptr [rsp + 0x20], r14
001eff0a: call   0x1801bd150
001eff0f: test   al, al
001eff11: je     0x1801eff2a
001eff13: mov    rcx, qword ptr [rbp + 0x38]
001eff17: lea    rdx, [rbp + 0x28]
001eff1b: xor    r8d, r8d
001eff1e: call   0x181b4f710                              ; System.Int32$$TryParse
001eff23: test   al, al
001eff25: mov    eax, dword ptr [rbp + 0x28]
001eff28: jne    0x1801eff2d
001eff2a: mov    eax, r14d
001eff2d: mov    dword ptr [rsi + 0x20], eax
001eff30: lea    r9, [rbp + 0x28]
001eff34: mov    r8, qword ptr [rip + 0x3ae2ead]          ; [0x3cd2de8] str:'loading_phrase'
001eff3b: mov    rcx, rdi
001eff3e: mov    rdx, qword ptr [rip + 0x3ac8dbb]         ; [0x3cb8d00] str:'song'
001eff45: mov    rbx, qword ptr [rip + 0x3ac3f24]         ; [0x3cb3e70] str:''
001eff4c: mov    qword ptr [rbp + 0x28], r14
001eff50: mov    qword ptr [rsp + 0x20], r14
001eff55: call   0x1801bd150
001eff5a: mov    rdx, qword ptr [rbp + 0x28]
001eff5e: lea    rcx, [rsi + 0x40]
001eff62: test   al, al
001eff64: cmove  rdx, rbx
001eff68: mov    qword ptr [rsi + 0x40], rdx
001eff6c: call   0x182f5fc00
001eff71: mov    r14, qword ptr [rsp + 0x60]
001eff76: mov    rbx, qword ptr [rsp + 0x50]
001eff7b: add    rsp, 0x30
001eff7f: pop    rdi
001eff80: pop    rsi
001eff81: pop    rbp
001eff82: ret    
001eff83: call   0x182f60c50
001eff88: int3   
001eff89: int3   
