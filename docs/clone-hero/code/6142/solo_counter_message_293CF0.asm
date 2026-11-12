00293cf0: sub    rsp, 0x28
00293cf4: cmp    byte ptr [rip + 0x3c79b16], 0            ; [0x3f0d811] (bss)
00293cfb: jne    0x180293d98
00293d01: lea    rcx, [rip + 0x3a3e230]                   ; [0x3cd1f38] meta:SoloCounter_TypeInfo
00293d08: call   0x182f609b0
00293d0d: lea    rcx, [rip + 0x3a13174]                   ; [0x3ca6e88] meta:string[]_TypeInfo
00293d14: call   0x182f609b0
00293d19: lea    rcx, [rip + 0x3a1eb58]                   ; [0x3cb2878] str:'Awesome Choke!'
00293d20: call   0x182f609b0
00293d25: lea    rcx, [rip + 0x3a041b4]                   ; [0x3c97ee0] str:'Perfect Solo!'
00293d2c: call   0x182f609b0
00293d31: lea    rcx, [rip + 0x3a476f8]                   ; [0x3cdb430] str:'Great Solo!'
00293d38: call   0x182f609b0
00293d3d: lea    rcx, [rip + 0x3a1ebec]                   ; [0x3cb2930] str:'Awesome Solo!'
00293d44: call   0x182f609b0
00293d49: lea    rcx, [rip + 0x3a43530]                   ; [0x3cd7280] str:'Messy Solo...'
00293d50: call   0x182f609b0
00293d55: lea    rcx, [rip + 0x3a5966c]                   ; [0x3ced3c8] str:'nice'
00293d5c: call   0x182f609b0
00293d61: lea    rcx, [rip + 0x3a4b288]                   ; [0x3cdeff0] str:'Solid Solo!'
00293d68: call   0x182f609b0
00293d6d: lea    rcx, [rip + 0x3a6747c]                   ; [0x3cfb1f0] str:'Okay Solo...'
00293d74: call   0x182f609b0
00293d79: lea    rcx, [rip + 0x3a46db0]                   ; [0x3cdab30] str:'Good Solo!'
00293d80: call   0x182f609b0
00293d85: lea    rcx, [rip + 0x3a233bc]                   ; [0x3cb7148] str:'soloPointsText'
00293d8c: call   0x182f609b0
00293d91: mov    byte ptr [rip + 0x3c79a79], 1            ; [0x3f0d811] (bss)
00293d98: mov    rax, qword ptr [rip + 0x3a3e199]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
00293d9f: mov    rcx, qword ptr [rax + 0xb8]
00293da6: cmp    qword ptr [rcx], 0
00293daa: jne    0x180294057
00293db0: mov    qword ptr [rsp + 0x20], rbx
00293db5: mov    rcx, qword ptr [rip + 0x3a130cc]         ; [0x3ca6e88] meta:string[]_TypeInfo
00293dbc: mov    edx, 0xa
00293dc1: call   0x182f5fc80
00293dc6: xor    ecx, ecx
00293dc8: mov    rbx, rax
00293dcb: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293dd0: test   rax, rax
00293dd3: je     0x18029405c
00293dd9: mov    rdx, qword ptr [rip + 0x3a04100]         ; [0x3c97ee0] str:'Perfect Solo!'
00293de0: xor    r8d, r8d
00293de3: mov    rcx, rax
00293de6: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293deb: test   rbx, rbx
00293dee: je     0x18029405c
00293df4: cmp    dword ptr [rbx + 0x18], 0
00293df8: jbe    0x180294062
00293dfe: lea    rcx, [rbx + 0x20]
00293e02: mov    qword ptr [rbx + 0x20], rax
00293e06: mov    rdx, rax
00293e09: call   0x182f5fc00
00293e0e: xor    ecx, ecx
00293e10: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293e15: test   rax, rax
00293e18: je     0x18029405c
00293e1e: mov    rdx, qword ptr [rip + 0x3a1eb0b]         ; [0x3cb2930] str:'Awesome Solo!'
00293e25: xor    r8d, r8d
00293e28: mov    rcx, rax
00293e2b: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293e30: cmp    dword ptr [rbx + 0x18], 1
00293e34: jbe    0x180294062
00293e3a: lea    rcx, [rbx + 0x28]
00293e3e: mov    qword ptr [rbx + 0x28], rax
00293e42: mov    rdx, rax
00293e45: call   0x182f5fc00
00293e4a: xor    ecx, ecx
00293e4c: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293e51: test   rax, rax
00293e54: je     0x18029405c
00293e5a: mov    rdx, qword ptr [rip + 0x3a475cf]         ; [0x3cdb430] str:'Great Solo!'
00293e61: xor    r8d, r8d
00293e64: mov    rcx, rax
00293e67: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293e6c: cmp    dword ptr [rbx + 0x18], 2
00293e70: jbe    0x180294062
00293e76: lea    rcx, [rbx + 0x30]
00293e7a: mov    qword ptr [rbx + 0x30], rax
00293e7e: mov    rdx, rax
00293e81: call   0x182f5fc00
00293e86: xor    ecx, ecx
00293e88: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293e8d: test   rax, rax
00293e90: je     0x18029405c
00293e96: mov    rdx, qword ptr [rip + 0x3a46c93]         ; [0x3cdab30] str:'Good Solo!'
00293e9d: xor    r8d, r8d
00293ea0: mov    rcx, rax
00293ea3: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293ea8: cmp    dword ptr [rbx + 0x18], 3
00293eac: jbe    0x180294062
00293eb2: lea    rcx, [rbx + 0x38]
00293eb6: mov    qword ptr [rbx + 0x38], rax
00293eba: mov    rdx, rax
00293ebd: call   0x182f5fc00
00293ec2: xor    ecx, ecx
00293ec4: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293ec9: test   rax, rax
00293ecc: je     0x18029405c
00293ed2: mov    rdx, qword ptr [rip + 0x3a4b117]         ; [0x3cdeff0] str:'Solid Solo!'
00293ed9: xor    r8d, r8d
00293edc: mov    rcx, rax
00293edf: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293ee4: cmp    dword ptr [rbx + 0x18], 4
00293ee8: jbe    0x180294062
00293eee: lea    rcx, [rbx + 0x40]
00293ef2: mov    qword ptr [rbx + 0x40], rax
00293ef6: mov    rdx, rax
00293ef9: call   0x182f5fc00
00293efe: xor    ecx, ecx
00293f00: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293f05: test   rax, rax
00293f08: je     0x18029405c
00293f0e: mov    rdx, qword ptr [rip + 0x3a672db]         ; [0x3cfb1f0] str:'Okay Solo...'
00293f15: xor    r8d, r8d
00293f18: mov    rcx, rax
00293f1b: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293f20: cmp    dword ptr [rbx + 0x18], 5
00293f24: jbe    0x180294062
00293f2a: lea    rcx, [rbx + 0x48]
00293f2e: mov    qword ptr [rbx + 0x48], rax
00293f32: mov    rdx, rax
00293f35: call   0x182f5fc00
00293f3a: xor    ecx, ecx
00293f3c: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293f41: test   rax, rax
00293f44: je     0x18029405c
00293f4a: mov    rdx, qword ptr [rip + 0x3a4332f]         ; [0x3cd7280] str:'Messy Solo...'
00293f51: xor    r8d, r8d
00293f54: mov    rcx, rax
00293f57: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293f5c: cmp    dword ptr [rbx + 0x18], 6
00293f60: jbe    0x180294062
00293f66: lea    rcx, [rbx + 0x50]
00293f6a: mov    qword ptr [rbx + 0x50], rax
00293f6e: mov    rdx, rax
00293f71: call   0x182f5fc00
00293f76: xor    ecx, ecx
00293f78: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293f7d: test   rax, rax
00293f80: je     0x18029405c
00293f86: mov    rdx, qword ptr [rip + 0x3a1e8eb]         ; [0x3cb2878] str:'Awesome Choke!'
00293f8d: xor    r8d, r8d
00293f90: mov    rcx, rax
00293f93: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293f98: cmp    dword ptr [rbx + 0x18], 7
00293f9c: jbe    0x180294062
00293fa2: lea    rcx, [rbx + 0x58]
00293fa6: mov    qword ptr [rbx + 0x58], rax
00293faa: mov    rdx, rax
00293fad: call   0x182f5fc00
00293fb2: xor    ecx, ecx
00293fb4: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293fb9: test   rax, rax
00293fbc: je     0x18029405c
00293fc2: mov    rdx, qword ptr [rip + 0x3a2317f]         ; [0x3cb7148] str:'soloPointsText'
00293fc9: xor    r8d, r8d
00293fcc: mov    rcx, rax
00293fcf: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00293fd4: cmp    dword ptr [rbx + 0x18], 8
00293fd8: jbe    0x180294062
00293fde: lea    rcx, [rbx + 0x60]
00293fe2: mov    qword ptr [rbx + 0x60], rax
00293fe6: mov    rdx, rax
00293fe9: call   0x182f5fc00
00293fee: xor    ecx, ecx
00293ff0: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00293ff5: test   rax, rax
00293ff8: je     0x18029405c
00293ffa: mov    rdx, qword ptr [rip + 0x3a593c7]         ; [0x3ced3c8] str:'nice'
00294001: xor    r8d, r8d
00294004: mov    rcx, rax
00294007: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
0029400c: cmp    dword ptr [rbx + 0x18], 9
00294010: jbe    0x180294062
00294012: lea    rcx, [rbx + 0x68]
00294016: mov    qword ptr [rbx + 0x68], rax
0029401a: mov    rdx, rax
0029401d: call   0x182f5fc00
00294022: mov    rax, qword ptr [rip + 0x3a3df0f]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
00294029: mov    rdx, rbx
0029402c: mov    rcx, qword ptr [rax + 0xb8]
00294033: mov    qword ptr [rcx], rbx
00294036: mov    rcx, qword ptr [rip + 0x3a3defb]         ; [0x3cd1f38] meta:SoloCounter_TypeInfo
0029403d: mov    rcx, qword ptr [rcx + 0xb8]
00294044: mov    rbx, qword ptr [rsp + 0x20]
00294049: add    rsp, 0x28
0029404d: jmp    0x182f5fc00
00294052: mov    rbx, qword ptr [rsp + 0x20]
00294057: add    rsp, 0x28
0029405b: ret    
0029405c: call   0x182f60c50
00294061: int3   
00294062: call   0x182f60c40
00294067: int3   
00294068: int3   
