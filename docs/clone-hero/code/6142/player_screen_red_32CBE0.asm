0032cbe0: push   rbx
0032cbe2: sub    rsp, 0x20
0032cbe6: cmp    byte ptr [rip + 0x3be1148], 0            ; [0x3f0dd35] (bss)
0032cbed: mov    rbx, rcx
0032cbf0: jne    0x18032cc11
0032cbf2: lea    rcx, [rip + 0x3996c1f]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032cbf9: call   0x182f609b0
0032cbfe: lea    rcx, [rip + 0x39e2443]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032cc05: call   0x182f609b0
0032cc0a: mov    byte ptr [rip + 0x3be1124], 1            ; [0x3f0dd35] (bss)
0032cc11: cmp    dword ptr [rbx + 0x120], 4
0032cc18: jne    0x18032cc43
0032cc1a: mov    rax, qword ptr [rip + 0x3996bf7]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032cc21: mov    rcx, qword ptr [rax + 0xb8]
0032cc28: mov    rcx, qword ptr [rcx]
0032cc2b: test   rcx, rcx
0032cc2e: je     0x18032cd0c
0032cc34: xor    edx, edx
0032cc36: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0032cc3b: test   al, al
0032cc3d: jne    0x18032cd06
0032cc43: xor    edx, edx
0032cc45: mov    rcx, rbx
0032cc48: call   0x1801d5250                              ; BaseMenu$$ʳʼʺˀˀʸʲʴʳˁʸ
0032cc4d: mov    ecx, dword ptr [rbx + 0x120]
0032cc53: test   ecx, ecx
0032cc55: je     0x18032cc9c
0032cc57: sub    ecx, 1
0032cc5a: je     0x18032cc84
0032cc5c: sub    ecx, 1
0032cc5f: je     0x18032cc8d
0032cc61: sub    ecx, 1
0032cc64: je     0x18032cc8d
0032cc66: cmp    ecx, 1
0032cc69: jne    0x18032ccd8
0032cc6b: xor    r8d, r8d
0032cc6e: xor    edx, edx
0032cc70: mov    rcx, rbx
0032cc73: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032cc78: xor    edx, edx
0032cc7a: mov    rcx, rbx
0032cc7d: call   0x1803308d0                              ; PlayerSelectionMenu$$ʾʹʼʺʳʽˁʵʳˁˀ
0032cc82: jmp    0x18032ccd8
0032cc84: cmp    byte ptr [rbx + 0x246], 0
0032cc8b: jne    0x18032ccd8
0032cc8d: xor    r8d, r8d
0032cc90: xor    edx, edx
0032cc92: mov    rcx, rbx
0032cc95: call   0x180330820                              ; PlayerSelectionMenu$$ʾʷʲʳʵʷʷʹʳʿʼ
0032cc9a: jmp    0x18032ccd8
0032cc9c: mov    rax, qword ptr [rip + 0x3996b75]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
0032cca3: mov    rcx, qword ptr [rax + 0xb8]
0032ccaa: mov    rcx, qword ptr [rcx]
0032ccad: test   rcx, rcx
0032ccb0: je     0x18032cd0c
0032ccb2: xor    edx, edx
0032ccb4: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
0032ccb9: test   al, al
0032ccbb: jne    0x18032cd06
0032ccbd: cmp    byte ptr [rbx + 0x244], al
0032ccc3: jne    0x18032cd06
0032ccc5: mov    rcx, qword ptr [rbx + 0x1c8]
0032cccc: test   rcx, rcx
0032cccf: je     0x18032cd0c
0032ccd1: xor    edx, edx
0032ccd3: call   0x1800d09f0                              ; PlayerSelectionCanvas$$ˀʵʹʺʳˁʼʸʻʼʶ
0032ccd8: cmp    byte ptr [rbx + 0x245], 0
0032ccdf: je     0x18032cd06
0032cce1: mov    rax, qword ptr [rip + 0x39e2360]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032cce8: mov    rcx, qword ptr [rax + 0xb8]
0032ccef: mov    rcx, qword ptr [rcx]
0032ccf2: test   rcx, rcx
0032ccf5: je     0x18032cd0c
0032ccf7: xor    r8d, r8d
0032ccfa: mov    dl, 1
0032ccfc: add    rsp, 0x20
0032cd00: pop    rbx
0032cd01: jmp    0x180092910                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʿʹˀʿʺʺʲʺʸʵʼ
0032cd06: add    rsp, 0x20
0032cd0a: pop    rbx
0032cd0b: ret    
0032cd0c: call   0x182f60c50
0032cd11: int3   
0032cd12: int3   
