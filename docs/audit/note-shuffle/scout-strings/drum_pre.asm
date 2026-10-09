=== 20EF450
020ef450: push   rbp
020ef452: push   rsi
020ef453: sub    rsp, 0x38
020ef457: cmp    byte ptr [rip + 0x1e29f14], 0            ; [0x3f19372] (bss)
020ef45e: mov    rbp, rcx
020ef461: jne    0x1820ef4a6
020ef463: lea    rcx, [rip + 0x1be7b36]                   ; [0x3cd6fa0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.Insert()
020ef46a: call   0x182f609b0
020ef46f: lea    rcx, [rip + 0x1be7d6a]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020ef476: call   0x182f609b0
020ef47b: lea    rcx, [rip + 0x1be7e1e]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef482: call   0x182f609b0
020ef487: lea    rcx, [rip + 0x1ba93ba]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020ef48e: call   0x182f609b0
020ef493: lea    rcx, [rip + 0x1bad65e]                   ; [0x3c9caf8] meta:ʽʿʸʸʾʶʶʾʶʹʲ_TypeInfo
020ef49a: call   0x182f609b0
020ef49f: mov    byte ptr [rip + 0x1e29ecc], 1            ; [0x3f19372] (bss)
020ef4a6: mov    qword ptr [rsp + 0x50], rbx
020ef4ab: xor    esi, esi
020ef4ad: mov    qword ptr [rsp + 0x58], rdi
020ef4b2: xor    eax, eax
020ef4b4: mov    qword ptr [rsp + 0x60], r12
020ef4b9: mov    qword ptr [rsp + 0x30], r13
020ef4be: mov    qword ptr [rsp + 0x28], r14
020ef4c3: mov    qword ptr [rsp + 0x20], r15
020ef4c8: test   rbp, rbp
020ef4cb: je     0x1820ef794
020ef4d1: mov    edi, 0xfffe
020ef4d6: mov    r14d, 0x80
020ef4dc: mov    r15d, 0x40
020ef4e2: mov    r12d, 8
020ef4e8: nop    dword ptr [rax + rax]
020ef4f0: cmp    eax, dword ptr [rbp + 0x18]
020ef4f3: jge    0x1820ef76f
020ef4f9: mov    r8, qword ptr [rip + 0x1be7da0]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef500: mov    edx, esi
020ef502: mov    rcx, rbp
020ef505: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef50a: test   rax, rax
020ef50d: je     0x1820ef794
020ef513: xor    edx, edx
020ef515: mov    rcx, rax
020ef518: call   0x18214b640                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʻʹˁʺʲʲʴʾʻʾ
020ef51d: test   al, al
020ef51f: jne    0x1820ef766
020ef525: mov    r8, qword ptr [rip + 0x1be7d74]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef52c: mov    edx, esi
020ef52e: mov    rcx, rbp
020ef531: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef536: test   rax, rax
020ef539: je     0x1820ef794
020ef53f: xor    edx, edx
020ef541: mov    rcx, rax
020ef544: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020ef549: mov    rcx, qword ptr [rip + 0x1ba92f8]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020ef550: movzx  ebx, ax
020ef553: cmp    dword ptr [rcx + 0xe0], 0
020ef55a: jne    0x1820ef561
020ef55c: call   0x182f60cf0
020ef561: mov    ecx, 0
020ef566: and    bx, di
020ef569: je     0x1820ef583
020ef56b: nop    dword ptr [rax + rax]
020ef570: inc    ecx
020ef572: lea    eax, [rbx - 1]
020ef575: and    bx, ax
020ef578: jne    0x1820ef570
020ef57a: cmp    ecx, 2
020ef57d: jge    0x1820ef766
020ef583: mov    r8, qword ptr [rip + 0x1be7d16]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef58a: mov    edx, esi
020ef58c: mov    rcx, rbp
020ef58f: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef594: test   rax, rax
020ef597: je     0x1820ef794
020ef59d: test   dword ptr [rax + 0x7c], 0x20000
020ef5a4: je     0x1820ef766
020ef5aa: mov    r8, qword ptr [rip + 0x1be7cef]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef5b1: mov    edx, esi
020ef5b3: mov    rcx, rbp
020ef5b6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef5bb: test   rax, rax
020ef5be: je     0x1820ef794
020ef5c4: or     dword ptr [rax + 0x7c], 3
020ef5c8: mov    edx, esi
020ef5ca: mov    r8, qword ptr [rip + 0x1be7ccf]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef5d1: mov    rcx, rbp
020ef5d4: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef5d9: mov    rcx, qword ptr [rip + 0x1bad518]         ; [0x3c9caf8] meta:ʽʿʸʸʾʶʶʾʶʹʲ_TypeInfo
020ef5e0: mov    rbx, rax
020ef5e3: call   0x182f60c00
020ef5e8: xor    r8d, r8d
020ef5eb: mov    rdx, rbx
020ef5ee: mov    rcx, rax
020ef5f1: mov    rdi, rax
020ef5f4: call   0x18214be70                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$.ctor
020ef5f9: mov    r8, qword ptr [rip + 0x1be7ca0]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef600: mov    edx, esi
020ef602: mov    rcx, rbp
020ef605: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef60a: test   rax, rax
020ef60d: je     0x1820ef794
020ef613: movzx  eax, word ptr [rax + 0x80]
020ef61a: cmp    ax, r12w
020ef61e: jbe    0x1820ef6b9
020ef624: cmp    ax, 0x20
020ef628: jbe    0x1820ef672
020ef62a: cmp    ax, r15w
020ef62e: je     0x1820ef65c
020ef630: cmp    ax, r14w
020ef634: jne    0x1820ef761
020ef63a: mov    r8, qword ptr [rip + 0x1be7c5f]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef641: mov    edx, esi
020ef643: mov    rcx, rbp
020ef646: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef64b: test   rax, rax
020ef64e: je     0x1820ef794
020ef654: mov    word ptr [rax + 0x80], r15w
020ef65c: test   rdi, rdi
020ef65f: je     0x1820ef794
020ef665: mov    word ptr [rdi + 0x80], r14w
020ef66d: jmp    0x1820ef704
020ef672: cmp    ax, 0x10
020ef676: jne    0x1820ef69c
020ef678: mov    r8, qword ptr [rip + 0x1be7c21]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef67f: mov    edx, esi
020ef681: mov    rcx, rbp
020ef684: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef689: test   rax, rax
020ef68c: je     0x1820ef794
020ef692: mov    word ptr [rax + 0x80], r12w
020ef69a: jmp    0x1820ef6f2
020ef69c: cmp    ax, 0x20
020ef6a0: jne    0x1820ef761
020ef6a6: test   rdi, rdi
020ef6a9: je     0x1820ef794
020ef6af: mov    word ptr [rdi + 0x80], r15w
020ef6b7: jmp    0x1820ef704
020ef6b9: cmp    ax, 2
020ef6bd: jne    0x1820ef6d3
020ef6bf: test   rdi, rdi
020ef6c2: je     0x1820ef794
020ef6c8: mov    word ptr [rdi + 0x80], 4
020ef6d1: jmp    0x1820ef704
020ef6d3: cmp    ax, 4
020ef6d7: jne    0x1820ef6ec
020ef6d9: test   rdi, rdi
020ef6dc: je     0x1820ef794
020ef6e2: mov    word ptr [rdi + 0x80], r12w
020ef6ea: jmp    0x1820ef704
020ef6ec: cmp    ax, r12w
020ef6f0: jne    0x1820ef761
020ef6f2: test   rdi, rdi
020ef6f5: je     0x1820ef794
020ef6fb: mov    word ptr [rdi + 0x80], 0x10
020ef704: mov    r8, qword ptr [rip + 0x1be7b95]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef70b: mov    edx, esi
020ef70d: mov    rcx, rbp
020ef710: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef715: test   rax, rax
020ef718: je     0x1820ef794
020ef71a: lea    rcx, [rax + 0x18]
020ef71e: mov    qword ptr [rax + 0x18], rdi
020ef722: mov    rdx, rdi
020ef725: call   0x182f5fc00
020ef72a: mov    r8, qword ptr [rip + 0x1be7b6f]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020ef731: mov    edx, esi
020ef733: mov    rcx, rbp
020ef736: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ef73b: lea    rcx, [rdi + 0x10]
020ef73f: mov    qword ptr [rdi + 0x10], rax
020ef743: mov    rdx, rax
020ef746: call   0x182f5fc00
020ef74b: mov    r9, qword ptr [rip + 0x1be784e]          ; [0x3cd6fa0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.Insert()
020ef752: inc    esi
020ef754: mov    edx, esi
020ef756: mov    r8, rdi
020ef759: mov    rcx, rbp
020ef75c: call   0x1807746c0                              ; System.Collections.Generic.List<object>$$Insert
020ef761: mov    edi, 0xfffe
020ef766: inc    esi
020ef768: mov    eax, esi
020ef76a: jmp    0x1820ef4f0
020ef76f: mov    r15, qword ptr [rsp + 0x20]
020ef774: mov    r14, qword ptr [rsp + 0x28]
020ef779: mov    r13, qword ptr [rsp + 0x30]
020ef77e: mov    r12, qword ptr [rsp + 0x60]
020ef783: mov    rdi, qword ptr [rsp + 0x58]
020ef788: mov    rbx, qword ptr [rsp + 0x50]
020ef78d: add    rsp, 0x38
020ef791: pop    rsi
020ef792: pop    rbp
020ef793: ret    
020ef794: call   0x182f60c50
020ef799: int3   
020ef79a: int3   
=== 20EFF60
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
=== 20F16E0
020f16e0: mov    qword ptr [rsp + 8], rbx
020f16e5: push   rdi
020f16e6: sub    rsp, 0x20
020f16ea: cmp    byte ptr [rip + 0x1e27c87], 0            ; [0x3f19378] (bss)
020f16f1: mov    rdi, rcx
020f16f4: jne    0x1820f1715
020f16f6: lea    rcx, [rip + 0x1be5ae3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f16fd: call   0x182f609b0
020f1702: lea    rcx, [rip + 0x1be5b97]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1709: call   0x182f609b0
020f170e: mov    byte ptr [rip + 0x1e27c63], 1            ; [0x3f19378] (bss)
020f1715: xor    ebx, ebx
020f1717: xor    eax, eax
020f1719: test   rdi, rdi
020f171c: je     0x1820f177d
020f171e: nop    
020f1720: cmp    eax, dword ptr [rdi + 0x18]
020f1723: jge    0x1820f1772
020f1725: mov    r8, qword ptr [rip + 0x1be5b74]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f172c: mov    edx, ebx
020f172e: mov    rcx, rdi
020f1731: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1736: test   rax, rax
020f1739: je     0x1820f177d
020f173b: test   byte ptr [rax + 0x80], 0xe0
020f1742: je     0x1820f176c
020f1744: mov    r8, qword ptr [rip + 0x1be5b55]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f174b: mov    edx, ebx
020f174d: mov    rcx, rdi
020f1750: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1755: test   rax, rax
020f1758: je     0x1820f177d
020f175a: movzx  ecx, word ptr [rax + 0x80]
020f1761: shr    cx, 3
020f1765: mov    word ptr [rax + 0x80], cx
020f176c: inc    ebx
020f176e: mov    eax, ebx
020f1770: jmp    0x1820f1720
020f1772: mov    rbx, qword ptr [rsp + 0x30]
020f1777: add    rsp, 0x20
020f177b: pop    rdi
020f177c: ret    
020f177d: call   0x182f60c50
020f1782: int3   
020f1783: int3   
=== 20EFEE0
020efee0: mov    qword ptr [rsp + 8], rbx
020efee5: push   rdi
020efee6: sub    rsp, 0x20
020efeea: cmp    byte ptr [rip + 0x1e29484], 0            ; [0x3f19375] (bss)
020efef1: mov    rdi, rcx
020efef4: jne    0x1820eff15
020efef6: lea    rcx, [rip + 0x1be72e3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020efefd: call   0x182f609b0
020eff02: lea    rcx, [rip + 0x1be7397]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020eff09: call   0x182f609b0
020eff0e: mov    byte ptr [rip + 0x1e29460], 1            ; [0x3f19375] (bss)
020eff15: xor    ebx, ebx
020eff17: xor    eax, eax
020eff19: test   rdi, rdi
020eff1c: je     0x1820eff53
020eff1e: nop    
020eff20: cmp    eax, dword ptr [rdi + 0x18]
020eff23: jge    0x1820eff48
020eff25: mov    r8, qword ptr [rip + 0x1be7374]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020eff2c: mov    edx, ebx
020eff2e: mov    rcx, rdi
020eff31: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020eff36: test   rax, rax
020eff39: je     0x1820eff53
020eff3b: and    dword ptr [rax + 0x7c], 0xfff3ffff
020eff42: inc    ebx
020eff44: mov    eax, ebx
020eff46: jmp    0x1820eff20
020eff48: mov    rbx, qword ptr [rsp + 0x30]
020eff4d: add    rsp, 0x20
020eff51: pop    rdi
020eff52: ret    
020eff53: call   0x182f60c50
020eff58: int3   
020eff59: int3   
=== 20F1420
020f1420: mov    qword ptr [rsp + 0x18], rbx
020f1425: push   rdi
020f1426: sub    rsp, 0x20
020f142a: cmp    byte ptr [rip + 0x1e27f49], 0            ; [0x3f1937a] (bss)
020f1431: mov    rdi, rcx
020f1434: jne    0x1820f1455
020f1436: lea    rcx, [rip + 0x1be5da3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f143d: call   0x182f609b0
020f1442: lea    rcx, [rip + 0x1be5e57]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1449: call   0x182f609b0
020f144e: mov    byte ptr [rip + 0x1e27f25], 1            ; [0x3f1937a] (bss)
020f1455: xor    ebx, ebx
020f1457: mov    qword ptr [rsp + 0x30], rbp
020f145c: xor    eax, eax
020f145e: mov    qword ptr [rsp + 0x38], rsi
020f1463: test   rdi, rdi
020f1466: je     0x1820f1532
020f146c: nop    dword ptr [rax]
020f1470: cmp    eax, dword ptr [rdi + 0x18]
020f1473: jge    0x1820f151d
020f1479: mov    r8, qword ptr [rip + 0x1be5e20]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1480: mov    edx, ebx
020f1482: mov    rcx, rdi
020f1485: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f148a: test   rax, rax
020f148d: je     0x1820f1532
020f1493: xor    edx, edx
020f1495: mov    rcx, rax
020f1498: call   0x18214b780                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʷʶʳˀˀʸʿʷʲʷʷ
020f149d: test   al, al
020f149f: je     0x1820f1514
020f14a1: mov    r8, qword ptr [rip + 0x1be5df8]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14a8: mov    edx, ebx
020f14aa: mov    rcx, rdi
020f14ad: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f14b2: test   rax, rax
020f14b5: je     0x1820f1532
020f14b7: movzx  eax, word ptr [rax + 0x80]
020f14be: cmp    ax, 2
020f14c2: jne    0x1820f14e9
020f14c4: mov    r8, qword ptr [rip + 0x1be5dd5]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14cb: mov    edx, ebx
020f14cd: mov    rcx, rdi
020f14d0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f14d5: test   rax, rax
020f14d8: je     0x1820f1532
020f14da: inc    ebx
020f14dc: mov    word ptr [rax + 0x80], 0x20
020f14e5: mov    eax, ebx
020f14e7: jmp    0x1820f1470
020f14e9: cmp    ax, 4
020f14ed: je     0x1820f14f5
020f14ef: cmp    ax, 0x20
020f14f3: jne    0x1820f1514
020f14f5: mov    r8, qword ptr [rip + 0x1be5da4]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f14fc: mov    edx, ebx
020f14fe: mov    rcx, rdi
020f1501: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1506: test   rax, rax
020f1509: je     0x1820f1532
020f150b: mov    word ptr [rax + 0x80], 2
020f1514: inc    ebx
020f1516: mov    eax, ebx
020f1518: jmp    0x1820f1470
020f151d: mov    rsi, qword ptr [rsp + 0x38]
020f1522: mov    rbp, qword ptr [rsp + 0x30]
020f1527: mov    rbx, qword ptr [rsp + 0x40]
020f152c: add    rsp, 0x20
020f1530: pop    rdi
020f1531: ret    
020f1532: call   0x182f60c50
020f1537: int3   
020f1538: int3   
