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
