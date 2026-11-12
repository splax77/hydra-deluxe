020ceec0: push   rbx
020ceec2: push   rbp
020ceec3: push   rsi
020ceec4: push   rdi
020ceec5: sub    rsp, 0x38
020ceec9: cmp    byte ptr [rip + 0x1e4a450], 0            ; [0x3f19320] (bss)
020ceed0: movzx  ebx, r9b
020ceed4: mov    rdi, r8
020ceed7: mov    rbp, rdx
020ceeda: mov    rsi, rcx
020ceedd: jne    0x1820cef22
020ceedf: lea    rcx, [rip + 0x1bff31a]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
020ceee6: call   0x182f609b0
020ceeeb: lea    rcx, [rip + 0x1bff54e]                   ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
020ceef2: call   0x182f609b0
020ceef7: lea    rcx, [rip + 0x1bff602]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
020ceefe: call   0x182f609b0
020cef03: lea    rcx, [rip + 0x1bff6b6]                   ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020cef0a: call   0x182f609b0
020cef0f: lea    rcx, [rip + 0x1c2972a]                   ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cef16: call   0x182f609b0
020cef1b: mov    byte ptr [rip + 0x1e4a3fe], 1            ; [0x3f19320] (bss)
020cef22: mov    rcx, qword ptr [rsi + 0x38]
020cef26: test   rcx, rcx
020cef29: je     0x1820cf0a2
020cef2f: mov    rdx, qword ptr [rip + 0x1c2970a]         ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cef36: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cef3b: test   rax, rax
020cef3e: je     0x1820cf0a2
020cef44: movzx  ecx, byte ptr [rsp + 0x80]
020cef4c: movzx  r9d, bl
020cef50: mov    qword ptr [rsp + 0x28], 0
020cef59: mov    r8, rdi
020cef5c: mov    byte ptr [rsp + 0x20], cl
020cef60: mov    rdx, rbp
020cef63: mov    rcx, rax
020cef66: call   0x182116390                              ; ʳʷʸʽʳʹʶˁʺʳʿ$$ʾʵʺʼʳʾʴˀʷʷʳ
020cef6b: mov    rdi, rax
020cef6e: test   rax, rax
020cef71: je     0x1820cf0a2
020cef77: cmp    byte ptr [rax + 0x21], 0
020cef7b: jne    0x1820cf030
020cef81: mov    rbx, qword ptr [rsi + 0x80]
020cef88: test   rbx, rbx
020cef8b: je     0x1820cf0a2
020cef91: mov    ebx, dword ptr [rbx + 0x18]
020cef94: sub    ebx, 1
020cef97: js     0x1820cf030
020cef9d: mov    r8, qword ptr [rip + 0x1bff61c]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020cefa4: mov    edx, ebx
020cefa6: mov    rcx, qword ptr [rsi + 0x80]
020cefad: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cefb2: test   rax, rax
020cefb5: je     0x1820cf0a2
020cefbb: mov    rcx, qword ptr [rdi + 0x10]
020cefbf: cmp    qword ptr [rax + 0x10], rcx
020cefc3: jne    0x1820cf030
020cefc5: cmp    byte ptr [rax + 0x21], 0
020cefc9: je     0x1820cf030
020cefcb: test   ebx, ebx
020cefcd: je     0x1820cf007
020cefcf: nop    
020cefd0: mov    rcx, qword ptr [rsi + 0x80]
020cefd7: test   rcx, rcx
020cefda: je     0x1820cf0a2
020cefe0: mov    r8, qword ptr [rip + 0x1bff5d9]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020cefe7: lea    edx, [rbx - 1]
020cefea: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cefef: test   rax, rax
020ceff2: je     0x1820cf0a2
020ceff8: mov    rcx, qword ptr [rdi + 0x10]
020ceffc: cmp    qword ptr [rax + 0x10], rcx
020cf000: jne    0x1820cf007
020cf002: sub    ebx, 1
020cf005: jne    0x1820cefd0
020cf007: mov    rcx, qword ptr [rsi + 0x80]
020cf00e: test   rcx, rcx
020cf011: je     0x1820cf0a2
020cf017: mov    r9, qword ptr [rip + 0x1bff422]          ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
020cf01e: mov    r8, rdi
020cf021: mov    edx, ebx
020cf023: add    rsp, 0x38
020cf027: pop    rdi
020cf028: pop    rsi
020cf029: pop    rbp
020cf02a: pop    rbx
020cf02b: jmp    0x1807746c0                              ; System.Collections.Generic.List<object>$$Insert
020cf030: mov    rcx, qword ptr [rsi + 0x80]
020cf037: test   rcx, rcx
020cf03a: je     0x1820cf0a2
020cf03c: mov    r9, qword ptr [rip + 0x1bff1bd]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
020cf043: inc    dword ptr [rcx + 0x1c]
020cf046: mov    rdx, qword ptr [rcx + 0x10]
020cf04a: test   rdx, rdx
020cf04d: je     0x1820cf0a2
020cf04f: movsxd r8, dword ptr [rcx + 0x18]
020cf053: cmp    r8d, dword ptr [rdx + 0x18]
020cf057: jb     0x1820cf078
020cf059: mov    rax, qword ptr [r9 + 0x20]
020cf05d: mov    rdx, rdi
020cf060: mov    r8, qword ptr [rax + 0xc0]
020cf067: mov    r8, qword ptr [r8 + 0x70]
020cf06b: add    rsp, 0x38
020cf06f: pop    rdi
020cf070: pop    rsi
020cf071: pop    rbp
020cf072: pop    rbx
020cf073: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020cf078: lea    eax, [r8 + 1]
020cf07c: mov    dword ptr [rcx + 0x18], eax
020cf07f: cmp    r8d, dword ptr [rdx + 0x18]
020cf083: jae    0x1820cf0a8
020cf085: mov    qword ptr [rdx + r8*8 + 0x20], rdi
020cf08a: add    rdx, 0x20
020cf08e: lea    rcx, [rdx + r8*8]
020cf092: mov    rdx, rdi
020cf095: add    rsp, 0x38
020cf099: pop    rdi
020cf09a: pop    rsi
020cf09b: pop    rbp
020cf09c: pop    rbx
020cf09d: jmp    0x182f5fc00
020cf0a2: call   0x182f60c50
020cf0a7: int3   
020cf0a8: call   0x182f60c40
020cf0ad: int3   
020cf0ae: int3   
