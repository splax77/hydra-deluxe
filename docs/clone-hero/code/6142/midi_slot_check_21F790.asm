0021f790: mov    qword ptr [rsp + 8], rbx
0021f795: mov    qword ptr [rsp + 0x10], rsi
0021f79a: push   rdi
0021f79b: sub    rsp, 0x20
0021f79f: cmp    byte ptr [rip + 0x3ced6d1], 0            ; [0x3f0ce77] (bss)
0021f7a6: mov    rsi, rdx
0021f7a9: mov    rdi, rcx
0021f7ac: jne    0x18021f7cd
0021f7ae: lea    rcx, [rip + 0x3ac46db]                   ; [0x3ce3e90] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Count()
0021f7b5: call   0x182f609b0
0021f7ba: lea    rcx, [rip + 0x3ac478f]                   ; [0x3ce3f50] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Item()
0021f7c1: call   0x182f609b0
0021f7c6: mov    byte ptr [rip + 0x3ced6aa], 1            ; [0x3f0ce77] (bss)
0021f7cd: mov    rax, qword ptr [rdi + 0x38]
0021f7d1: xor    ebx, ebx
0021f7d3: xor    ecx, ecx
0021f7d5: test   rax, rax
0021f7d8: je     0x18021f83a
0021f7da: nop    word ptr [rax + rax]
0021f7e0: cmp    ecx, dword ptr [rax + 0x18]
0021f7e3: jge    0x18021f828
0021f7e5: mov    rcx, qword ptr [rdi + 0x38]
0021f7e9: test   rcx, rcx
0021f7ec: je     0x18021f83a
0021f7ee: mov    r8, qword ptr [rip + 0x3ac475b]          ; [0x3ce3f50] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Item()
0021f7f5: mov    edx, ebx
0021f7f7: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0021f7fc: test   rax, rax
0021f7ff: je     0x18021f83a
0021f801: cmp    qword ptr [rax + 0x10], rsi
0021f805: je     0x18021f816
0021f807: mov    rax, qword ptr [rdi + 0x38]
0021f80b: inc    ebx
0021f80d: mov    ecx, ebx
0021f80f: test   rax, rax
0021f812: je     0x18021f83a
0021f814: jmp    0x18021f7e0
0021f816: mov    al, 1
0021f818: mov    rbx, qword ptr [rsp + 0x30]
0021f81d: mov    rsi, qword ptr [rsp + 0x38]
0021f822: add    rsp, 0x20
0021f826: pop    rdi
0021f827: ret    
0021f828: mov    rbx, qword ptr [rsp + 0x30]
0021f82d: xor    al, al
0021f82f: mov    rsi, qword ptr [rsp + 0x38]
0021f834: add    rsp, 0x20
0021f838: pop    rdi
0021f839: ret    
0021f83a: call   0x182f60c50
0021f83f: int3   
