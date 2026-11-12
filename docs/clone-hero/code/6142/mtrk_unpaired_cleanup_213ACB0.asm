0213acb0: mov    qword ptr [rsp + 8], rbx
0213acb5: push   rdi
0213acb6: sub    rsp, 0x20
0213acba: mov    rbx, rcx
0213acbd: mov    rax, qword ptr [rcx]
0213acc0: xor    edi, edi
0213acc2: mov    rcx, qword ptr [rcx + 8]
0213acc6: mov    rax, qword ptr [rax]
0213acc9: mov    rax, qword ptr [rax + 0x18]
0213accd: mov    qword ptr [rcx], rax
0213acd0: mov    rax, qword ptr [rbx + 0x10]
0213acd4: mov    dword ptr [rax], edi
0213acd6: mov    rax, qword ptr [rbx + 0x18]
0213acda: mov    rcx, qword ptr [rbx + 0x10]
0213acde: mov    eax, dword ptr [rax + 8]
0213ace1: cmp    dword ptr [rcx], eax
0213ace3: jge    0x18213ad9d
0213ace9: nop    dword ptr [rax]
0213acf0: mov    rax, qword ptr [rbx + 0x10]
0213acf4: movsxd rcx, dword ptr [rax]
0213acf7: mov    rax, qword ptr [rbx + 0x18]
0213acfb: cmp    ecx, dword ptr [rax + 8]
0213acfe: jae    0x18213afb1
0213ad04: lea    rdx, [rcx + rcx*8]
0213ad08: mov    rcx, qword ptr [rax]
0213ad0b: mov    rax, qword ptr [rbx + 0x20]
0213ad0f: lea    rdx, [rcx + rdx*4]
0213ad13: mov    qword ptr [rax], rdx
0213ad16: mov    rcx, qword ptr [rbx + 0x20]
0213ad1a: mov    r8, qword ptr [rip + 0x1bb853f]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213ad21: mov    rdx, qword ptr [rbx + 0x28]
0213ad25: mov    rcx, qword ptr [rcx]
0213ad28: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213ad2d: test   al, al
0213ad2f: je     0x18213ad80
0213ad31: nop    dword ptr [rax]
0213ad35: nop    word ptr [rax + rax]
0213ad40: mov    rax, qword ptr [rbx + 8]
0213ad44: mov    rcx, qword ptr [rax]
0213ad47: test   rcx, rcx
0213ad4a: je     0x18213afab
0213ad50: mov    rdx, qword ptr [rbx + 0x28]
0213ad54: xor    r8d, r8d
0213ad57: mov    r9, qword ptr [rip + 0x1b952a2]          ; [0x3cd0000] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.set_Item()
0213ad5e: mov    edx, dword ptr [rdx]
0213ad60: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
0213ad65: mov    rcx, qword ptr [rbx + 0x20]
0213ad69: mov    r8, qword ptr [rip + 0x1bb84f0]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213ad70: mov    rdx, qword ptr [rbx + 0x28]
0213ad74: mov    rcx, qword ptr [rcx]
0213ad77: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213ad7c: test   al, al
0213ad7e: jne    0x18213ad40
0213ad80: mov    rax, qword ptr [rbx + 0x10]
0213ad84: mov    ecx, dword ptr [rax]
0213ad86: inc    ecx
0213ad88: mov    dword ptr [rax], ecx
0213ad8a: mov    rax, qword ptr [rbx + 0x18]
0213ad8e: mov    rcx, qword ptr [rbx + 0x10]
0213ad92: mov    eax, dword ptr [rax + 8]
0213ad95: cmp    dword ptr [rcx], eax
0213ad97: jl     0x18213acf0
0213ad9d: mov    rax, qword ptr [rbx + 0x30]
0213ada1: mov    dword ptr [rax], edi
0213ada3: mov    rax, qword ptr [rbx + 0x38]
0213ada7: mov    rcx, qword ptr [rbx + 0x30]
0213adab: mov    eax, dword ptr [rax + 8]
0213adae: cmp    dword ptr [rcx], eax
0213adb0: jge    0x18213ae6d
0213adb6: nop    word ptr [rax + rax]
0213adc0: mov    rax, qword ptr [rbx + 0x30]
0213adc4: movsxd rcx, dword ptr [rax]
0213adc7: mov    rax, qword ptr [rbx + 0x38]
0213adcb: cmp    ecx, dword ptr [rax + 8]
0213adce: jae    0x18213afb1
0213add4: lea    rdx, [rcx + rcx*8]
0213add8: mov    rcx, qword ptr [rax]
0213addb: mov    rax, qword ptr [rbx + 0x40]
0213addf: lea    rdx, [rcx + rdx*4]
0213ade3: mov    qword ptr [rax], rdx
0213ade6: mov    rcx, qword ptr [rbx + 0x40]
0213adea: mov    r8, qword ptr [rip + 0x1bb846f]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213adf1: mov    rdx, qword ptr [rbx + 0x48]
0213adf5: mov    rcx, qword ptr [rcx]
0213adf8: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213adfd: test   al, al
0213adff: je     0x18213ae50
0213ae01: nop    dword ptr [rax]
0213ae05: nop    word ptr [rax + rax]
0213ae10: mov    rax, qword ptr [rbx + 8]
0213ae14: mov    rcx, qword ptr [rax]
0213ae17: test   rcx, rcx
0213ae1a: je     0x18213afab
0213ae20: mov    rdx, qword ptr [rbx + 0x48]
0213ae24: xor    r8d, r8d
0213ae27: mov    r9, qword ptr [rip + 0x1b951d2]          ; [0x3cd0000] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.set_Item()
0213ae2e: mov    edx, dword ptr [rdx]
0213ae30: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
0213ae35: mov    rcx, qword ptr [rbx + 0x40]
0213ae39: mov    r8, qword ptr [rip + 0x1bb8420]          ; [0x3cf3260] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.ʵʼʼʺʷˁʼʹʿˁʷ()
0213ae40: mov    rdx, qword ptr [rbx + 0x48]
0213ae44: mov    rcx, qword ptr [rcx]
0213ae47: call   0x180f9ebe0                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ʵʼʼʺʷˁʼʹʿˁʷ
0213ae4c: test   al, al
0213ae4e: jne    0x18213ae10
0213ae50: mov    rax, qword ptr [rbx + 0x30]
0213ae54: mov    ecx, dword ptr [rax]
0213ae56: inc    ecx
0213ae58: mov    dword ptr [rax], ecx
0213ae5a: mov    rax, qword ptr [rbx + 0x38]
0213ae5e: mov    rcx, qword ptr [rbx + 0x30]
0213ae62: mov    eax, dword ptr [rax + 8]
0213ae65: cmp    dword ptr [rcx], eax
0213ae67: jl     0x18213adc0
0213ae6d: mov    rax, qword ptr [rbx + 8]
0213ae71: mov    rcx, qword ptr [rax]
0213ae74: test   rcx, rcx
0213ae77: je     0x18213afab
0213ae7d: mov    rax, qword ptr [rbx + 0x50]
0213ae81: mov    ecx, dword ptr [rcx + 0x18]
0213ae84: dec    ecx
0213ae86: mov    dword ptr [rax], ecx
0213ae88: mov    rax, qword ptr [rbx + 0x50]
0213ae8c: cmp    dword ptr [rax], edi
0213ae8e: jl     0x18213aeeb
0213ae90: mov    rax, qword ptr [rbx + 8]
0213ae94: mov    rcx, qword ptr [rax]
0213ae97: test   rcx, rcx
0213ae9a: je     0x18213afab
0213aea0: mov    rdx, qword ptr [rbx + 0x50]
0213aea4: mov    r8, qword ptr [rip + 0x1b95095]          ; [0x3ccff40] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Item()
0213aeab: mov    edx, dword ptr [rdx]
0213aead: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0213aeb2: test   rax, rax
0213aeb5: jne    0x18213aed9
0213aeb7: mov    rax, qword ptr [rbx + 8]
0213aebb: mov    rcx, qword ptr [rax]
0213aebe: test   rcx, rcx
0213aec1: je     0x18213afab
0213aec7: mov    rdx, qword ptr [rbx + 0x50]
0213aecb: mov    r8, qword ptr [rip + 0x1b94eee]          ; [0x3ccfdc0] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.RemoveAt()
0213aed2: mov    edx, dword ptr [rdx]
0213aed4: call   0x180774920                              ; System.Collections.Generic.List<object>$$RemoveAt
0213aed9: mov    rax, qword ptr [rbx + 0x50]
0213aedd: mov    ecx, dword ptr [rax]
0213aedf: dec    ecx
0213aee1: mov    dword ptr [rax], ecx
0213aee3: mov    rax, qword ptr [rbx + 0x50]
0213aee7: cmp    dword ptr [rax], edi
0213aee9: jge    0x18213ae90
0213aeeb: mov    rax, qword ptr [rbx + 0x58]
0213aeef: mov    dword ptr [rax], edi
0213aef1: mov    rax, qword ptr [rbx + 0x18]
0213aef5: mov    rdx, qword ptr [rbx + 0x58]
0213aef9: mov    ecx, dword ptr [rax + 8]
0213aefc: cmp    dword ptr [rdx], ecx
0213aefe: jge    0x18213af44
0213af00: mov    rax, qword ptr [rbx + 0x58]
0213af04: movsxd rcx, dword ptr [rax]
0213af07: mov    rax, qword ptr [rbx + 0x18]
0213af0b: cmp    ecx, dword ptr [rax + 8]
0213af0e: jae    0x18213afb1
0213af14: lea    rdx, [rcx + rcx*8]
0213af18: mov    rcx, qword ptr [rax]
0213af1b: lea    rcx, [rcx + rdx*4]
0213af1f: mov    rdx, qword ptr [rip + 0x1bb81ba]         ; [0x3cf30e0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.Dispose()
0213af26: call   0x180f9da60                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ˁˀʺʳʼˀʳʴˁʽʷ
0213af2b: mov    rax, qword ptr [rbx + 0x58]
0213af2f: mov    ecx, dword ptr [rax]
0213af31: inc    ecx
0213af33: mov    dword ptr [rax], ecx
0213af35: mov    rax, qword ptr [rbx + 0x18]
0213af39: mov    rdx, qword ptr [rbx + 0x58]
0213af3d: mov    ecx, dword ptr [rax + 8]
0213af40: cmp    dword ptr [rdx], ecx
0213af42: jl     0x18213af00
0213af44: mov    rax, qword ptr [rbx + 0x60]
0213af48: mov    dword ptr [rax], edi
0213af4a: mov    rax, qword ptr [rbx + 0x38]
0213af4e: mov    rdx, qword ptr [rbx + 0x60]
0213af52: mov    ecx, dword ptr [rax + 8]
0213af55: cmp    dword ptr [rdx], ecx
0213af57: jge    0x18213afa0
0213af59: nop    dword ptr [rax]
0213af60: mov    rax, qword ptr [rbx + 0x60]
0213af64: movsxd rcx, dword ptr [rax]
0213af67: mov    rax, qword ptr [rbx + 0x38]
0213af6b: cmp    ecx, dword ptr [rax + 8]
0213af6e: jae    0x18213afb1
0213af70: lea    rdx, [rcx + rcx*8]
0213af74: mov    rcx, qword ptr [rax]
0213af77: lea    rcx, [rcx + rdx*4]
0213af7b: mov    rdx, qword ptr [rip + 0x1bb815e]         ; [0x3cf30e0] metamethod:Method$ʳʲʿʵʶʻʽʶˁʷʳ<int>.Dispose()
0213af82: call   0x180f9da60                              ; ʳʲʿʵʶʻʽʶˁʷʳ<int>$$ˁˀʺʳʼˀʳʴˁʽʷ
0213af87: mov    rax, qword ptr [rbx + 0x60]
0213af8b: mov    ecx, dword ptr [rax]
0213af8d: inc    ecx
0213af8f: mov    dword ptr [rax], ecx
0213af91: mov    rax, qword ptr [rbx + 0x38]
0213af95: mov    rdx, qword ptr [rbx + 0x60]
0213af99: mov    ecx, dword ptr [rax + 8]
0213af9c: cmp    dword ptr [rdx], ecx
0213af9e: jl     0x18213af60
0213afa0: mov    rbx, qword ptr [rsp + 0x30]
0213afa5: add    rsp, 0x20
0213afa9: pop    rdi
0213afaa: ret    
0213afab: call   0x182f60c50
0213afb0: int3   
0213afb1: call   0x182f60c40
0213afb6: int3   
0213afb7: int3   
