020eff60: mov    qword ptr [rsp + 0x10], rbx
020eff65: push   rdi
020eff66: sub    rsp, 0x20
020eff6a: cmp    byte ptr [rip + 0x1e29402], 0            ; [0x3f19373] (bss)
020eff71: mov    rdi, rcx
020eff74: jne    0x1820effd1
020eff76: lea    rcx, [rip + 0x1be70e3]                   ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020eff7d: call   0x182f609b0
020eff82: lea    rcx, [rip + 0x1be7257]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020eff89: call   0x182f609b0
020eff8e: lea    rcx, [rip + 0x1be730b]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020eff95: call   0x182f609b0
020eff9a: lea    rcx, [rip + 0x1be73bf]                   ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020effa1: call   0x182f609b0
020effa6: lea    rcx, [rip + 0x1c103ab]                   ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020effad: call   0x182f609b0
020effb2: lea    rcx, [rip + 0x1be9677]                   ; [0x3cd9630] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ʼʴʺʳʳʷˁʴʸʷʿ()
020effb9: call   0x182f609b0
020effbe: lea    rcx, [rip + 0x1c15523]                   ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020effc5: call   0x182f609b0
020effca: mov    byte ptr [rip + 0x1e293a2], 1            ; [0x3f19373] (bss)
020effd1: xor    ebx, ebx
020effd3: mov    qword ptr [rsp + 0x30], rsi
020effd8: xor    eax, eax
020effda: test   rdi, rdi
020effdd: je     0x1820f01d6
020effe3: cmp    eax, dword ptr [rdi + 0x18]
020effe6: jge    0x1820f011b
020effec: mov    r8, qword ptr [rip + 0x1be72ad]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020efff3: mov    edx, ebx
020efff5: mov    rcx, rdi
020efff8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020efffd: test   rax, rax
020f0000: je     0x1820f01d6
020f0006: xor    edx, edx
020f0008: mov    rcx, rax
020f000b: call   0x18214b850                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʺʸˁʽʹʿˁʶʵʴʽ
020f0010: test   al, al
020f0012: je     0x1820f0112
020f0018: mov    r8, qword ptr [rip + 0x1be7281]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f001f: mov    edx, ebx
020f0021: mov    rcx, rdi
020f0024: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0029: test   rax, rax
020f002c: je     0x1820f01d6
020f0032: cmp    qword ptr [rax + 0x10], 0
020f0037: je     0x1820f008b
020f0039: mov    r8, qword ptr [rip + 0x1be7260]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0040: mov    edx, ebx
020f0042: mov    rcx, rdi
020f0045: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f004a: test   rax, rax
020f004d: je     0x1820f01d6
020f0053: mov    r8, qword ptr [rip + 0x1be7246]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f005a: mov    edx, ebx
020f005c: mov    rsi, qword ptr [rax + 0x10]
020f0060: mov    rcx, rdi
020f0063: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f0068: test   rax, rax
020f006b: je     0x1820f01d6
020f0071: test   rsi, rsi
020f0074: je     0x1820f01d6
020f007a: mov    rdx, qword ptr [rax + 0x18]
020f007e: lea    rcx, [rsi + 0x18]
020f0082: mov    qword ptr [rsi + 0x18], rdx
020f0086: call   0x182f5fc00
020f008b: mov    r8, qword ptr [rip + 0x1be720e]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f0092: mov    edx, ebx
020f0094: mov    rcx, rdi
020f0097: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f009c: test   rax, rax
020f009f: je     0x1820f01d6
020f00a5: cmp    qword ptr [rax + 0x18], 0
020f00aa: je     0x1820f00fe
020f00ac: mov    r8, qword ptr [rip + 0x1be71ed]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f00b3: mov    edx, ebx
020f00b5: mov    rcx, rdi
020f00b8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f00bd: test   rax, rax
020f00c0: je     0x1820f01d6
020f00c6: mov    r8, qword ptr [rip + 0x1be71d3]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f00cd: mov    edx, ebx
020f00cf: mov    rsi, qword ptr [rax + 0x18]
020f00d3: mov    rcx, rdi
020f00d6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f00db: test   rax, rax
020f00de: je     0x1820f01d6
020f00e4: test   rsi, rsi
020f00e7: je     0x1820f01d6
020f00ed: mov    rdx, qword ptr [rax + 0x10]
020f00f1: lea    rcx, [rsi + 0x10]
020f00f5: mov    qword ptr [rsi + 0x10], rdx
020f00f9: call   0x182f5fc00
020f00fe: mov    r9, qword ptr [rip + 0x1be725b]          ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020f0105: xor    r8d, r8d
020f0108: mov    edx, ebx
020f010a: mov    rcx, rdi
020f010d: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
020f0112: inc    ebx
020f0114: mov    eax, ebx
020f0116: jmp    0x1820effe3
020f011b: mov    rcx, qword ptr [rip + 0x1c153c6]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0122: cmp    dword ptr [rcx + 0xe0], 0
020f0129: jne    0x1820f0137
020f012b: call   0x182f60cf0
020f0130: mov    rcx, qword ptr [rip + 0x1c153b1]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0137: mov    rax, qword ptr [rcx + 0xb8]
020f013e: mov    rsi, qword ptr [rax + 8]
020f0142: test   rsi, rsi
020f0145: jne    0x1820f01b5
020f0147: cmp    dword ptr [rcx + 0xe0], esi
020f014d: jne    0x1820f015b
020f014f: call   0x182f60cf0
020f0154: mov    rcx, qword ptr [rip + 0x1c1538d]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f015b: mov    rax, qword ptr [rcx + 0xb8]
020f0162: mov    rcx, qword ptr [rip + 0x1c101ef]         ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020f0169: mov    rbx, qword ptr [rax]
020f016c: call   0x182f60c00
020f0171: mov    r8, qword ptr [rip + 0x1be94b8]          ; [0x3cd9630] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ʼʴʺʳʳʷˁʴʸʷʿ()
020f0178: xor    r9d, r9d
020f017b: mov    rdx, rbx
020f017e: mov    rcx, rax
020f0181: mov    rsi, rax
020f0184: call   0x1808ae670                              ; Grpc.Core.VerifyPeerCallback$$.ctor
020f0189: mov    rax, qword ptr [rip + 0x1c15358]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f0190: mov    rdx, rsi
020f0193: mov    rcx, qword ptr [rax + 0xb8]
020f019a: mov    qword ptr [rcx + 8], rsi
020f019e: mov    rax, qword ptr [rip + 0x1c15343]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f01a5: mov    rcx, qword ptr [rax + 0xb8]
020f01ac: add    rcx, 8
020f01b0: call   0x182f5fc00
020f01b5: mov    r8, qword ptr [rip + 0x1be6ea4]          ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020f01bc: mov    rdx, rsi
020f01bf: mov    rcx, rdi
020f01c2: mov    rsi, qword ptr [rsp + 0x30]
020f01c7: mov    rbx, qword ptr [rsp + 0x38]
020f01cc: add    rsp, 0x20
020f01d0: pop    rdi
020f01d1: jmp    0x1807747e0                              ; System.Collections.Generic.List<object>$$RemoveAll
020f01d6: call   0x182f60c50
020f01db: int3   
020f01dc: int3   
