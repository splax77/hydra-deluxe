001eff90: mov    qword ptr [rsp + 8], rbx
001eff95: push   rdi
001eff96: sub    rsp, 0x20
001eff9a: cmp    byte ptr [rip + 0x3d1cbb5], 0            ; [0x3f0cb56] (bss)
001effa1: mov    rbx, rdx
001effa4: mov    rdi, rcx
001effa7: jne    0x1801f0038
001effad: lea    rcx, [rip + 0x3b05d8c]                   ; [0x3cf5d40] str:'video_start_time'
001effb4: call   0x182f609b0
001effb9: lea    rcx, [rip + 0x3adc570]                   ; [0x3ccc530] str:'sustain_cutoff_threshold'
001effc0: call   0x182f609b0
001effc5: lea    rcx, [rip + 0x3afe97c]                   ; [0x3cee948] str:'eighthnote_hopo'
001effcc: call   0x182f609b0
001effd1: lea    rcx, [rip + 0x3ae2e10]                   ; [0x3cd2de8] str:'loading_phrase'
001effd8: call   0x182f609b0
001effdd: lea    rcx, [rip + 0x3aed294]                   ; [0x3cdd278] str:'delay'
001effe4: call   0x182f609b0
001effe9: lea    rcx, [rip + 0x3b0d3c8]                   ; [0x3cfd3b8] str:'five_lane_drums'
001efff0: call   0x182f609b0
001efff5: lea    rcx, [rip + 0x3ad28d4]                   ; [0x3cc28d0] str:'chord_snap_threshold'
001efffc: call   0x182f609b0
001f0001: lea    rcx, [rip + 0x3af9700]                   ; [0x3ce9708] str:'multiplier_note'
001f0008: call   0x182f609b0
001f000d: lea    rcx, [rip + 0x3aa7d9c]                   ; [0x3c97db0] str:'pro_drums'
001f0014: call   0x182f609b0
001f0019: lea    rcx, [rip + 0x3ac3e50]                   ; [0x3cb3e70] str:''
001f0020: call   0x182f609b0
001f0025: lea    rcx, [rip + 0x3ab8104]                   ; [0x3ca8130] str:'hopo_frequency'
001f002c: call   0x182f609b0
001f0031: mov    byte ptr [rip + 0x3d1cb1e], 1            ; [0x3f0cb56] (bss)
001f0038: test   rbx, rbx
001f003b: je     0x1801f014f
001f0041: mov    rdx, qword ptr [rip + 0x3aed230]         ; [0x3cdd278] str:'delay'
001f0048: xor    r9d, r9d
001f004b: xor    r8d, r8d
001f004e: mov    rcx, rbx
001f0051: call   0x180262940                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʲʿʿʺʶʻʿʻʵʺʹ
001f0056: mov    dword ptr [rdi + 0x24], eax
001f0059: xor    r9d, r9d
001f005c: mov    rdx, qword ptr [rip + 0x3ab80cd]         ; [0x3ca8130] str:'hopo_frequency'
001f0063: xor    r8d, r8d
001f0066: mov    rcx, rbx
001f0069: call   0x1802659d0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʾʻʶʾʷʸʺʽʹʾʶ
001f006e: mov    dword ptr [rdi + 0x18], eax
001f0071: xor    r9d, r9d
001f0074: mov    rdx, qword ptr [rip + 0x3b0d33d]         ; [0x3cfd3b8] str:'five_lane_drums'
001f007b: xor    r8d, r8d
001f007e: mov    rcx, rbx
001f0081: call   0x1802642c0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʸʺˁʳʹʹˁʶˁʸʵ
001f0086: mov    byte ptr [rdi + 0x38], al
001f0089: xor    r9d, r9d
001f008c: mov    rdx, qword ptr [rip + 0x3aa7d1d]         ; [0x3c97db0] str:'pro_drums'
001f0093: xor    r8d, r8d
001f0096: mov    rcx, rbx
001f0099: call   0x1802642c0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʸʺˁʳʹʹˁʶˁʸʵ
001f009e: mov    byte ptr [rdi + 0x39], al
001f00a1: xor    r9d, r9d
001f00a4: mov    rdx, qword ptr [rip + 0x3afe89d]         ; [0x3cee948] str:'eighthnote_hopo'
001f00ab: xor    r8d, r8d
001f00ae: mov    rcx, rbx
001f00b1: call   0x1802642c0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʸʺˁʳʹʹˁʶˁʸʵ
001f00b6: mov    byte ptr [rdi + 0x10], al
001f00b9: xor    r9d, r9d
001f00bc: mov    rdx, qword ptr [rip + 0x3af9645]         ; [0x3ce9708] str:'multiplier_note'
001f00c3: xor    r8d, r8d
001f00c6: mov    rcx, rbx
001f00c9: call   0x1802659d0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʾʻʶʾʷʸʺʽʹʾʶ
001f00ce: mov    dword ptr [rdi + 0x14], eax
001f00d1: xor    r9d, r9d
001f00d4: mov    rdx, qword ptr [rip + 0x3b05c65]         ; [0x3cf5d40] str:'video_start_time'
001f00db: xor    r8d, r8d
001f00de: mov    rcx, rbx
001f00e1: call   0x1802659d0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʾʻʶʾʷʸʺʽʹʾʶ
001f00e6: mov    dword ptr [rdi + 0x28], eax
001f00e9: xor    r9d, r9d
001f00ec: mov    rdx, qword ptr [rip + 0x3adc43d]         ; [0x3ccc530] str:'sustain_cutoff_threshold'
001f00f3: mov    r8d, 0xffffffff
001f00f9: mov    rcx, rbx
001f00fc: call   0x1802659d0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʾʻʶʾʷʸʺʽʹʾʶ
001f0101: mov    dword ptr [rdi + 0x1c], eax
001f0104: xor    r9d, r9d
001f0107: mov    rdx, qword ptr [rip + 0x3ad27c2]         ; [0x3cc28d0] str:'chord_snap_threshold'
001f010e: xor    r8d, r8d
001f0111: mov    rcx, rbx
001f0114: call   0x1802659d0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʾʻʶʾʷʸʺʽʹʾʶ
001f0119: mov    dword ptr [rdi + 0x20], eax
001f011c: xor    r9d, r9d
001f011f: mov    r8, qword ptr [rip + 0x3ac3d4a]          ; [0x3cb3e70] str:''
001f0126: mov    rcx, rbx
001f0129: mov    rdx, qword ptr [rip + 0x3ae2cb8]         ; [0x3cd2de8] str:'loading_phrase'
001f0130: call   0x180262c80                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʳʵʻʾˁʷʲˀˀʻʶ
001f0135: lea    rcx, [rdi + 0x40]
001f0139: mov    qword ptr [rdi + 0x40], rax
001f013d: mov    rdx, rax
001f0140: mov    rbx, qword ptr [rsp + 0x30]
001f0145: add    rsp, 0x20
001f0149: pop    rdi
001f014a: jmp    0x182f5fc00
001f014f: call   0x182f60c50
001f0154: int3   
001f0155: int3   
