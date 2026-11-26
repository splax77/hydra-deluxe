00160260: push   rdi
00160262: sub    rsp, 0x20
00160266: cmp    byte ptr [rip + 0x3dac511], 0            ; [0x3f0c77e] (bss)
0016026d: mov    rdi, rcx
00160270: jne    0x1801602f1
00160272: lea    rcx, [rip + 0x3b87e2f]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
00160279: call   0x182f609b0
0016027e: lea    rcx, [rip + 0x3b8f60b]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00160285: call   0x182f609b0
0016028a: lea    rcx, [rip + 0x3b5643f]                   ; [0x3cb66d0] metamethod:Method$System.Collections.Generic.List<SongEntry>.get_Item()
00160291: call   0x182f609b0
00160296: lea    rcx, [rip + 0x3b401f3]                   ; [0x3ca0490] metamethod:Method$System.Collections.Generic.List<int>.get_Item()
0016029d: call   0x182f609b0
001602a2: lea    rcx, [rip + 0x3b96927]                   ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
001602a9: call   0x182f609b0
001602ae: lea    rcx, [rip + 0x3b3806b]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
001602b5: call   0x182f609b0
001602ba: lea    rcx, [rip + 0x3b3cdd7]                   ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
001602c1: call   0x182f609b0
001602c6: lea    rcx, [rip + 0x3b59e7b]                   ; [0x3cba148] str:' '
001602cd: call   0x182f609b0
001602d2: lea    rcx, [rip + 0x3b6b757]                   ; [0x3ccba30] str:'Main Menu'
001602d9: call   0x182f609b0
001602de: lea    rcx, [rip + 0x3b8d213]                   ; [0x3ced4f8] str:'...'
001602e5: call   0x182f609b0
001602ea: mov    byte ptr [rip + 0x3dac48d], 1            ; [0x3f0c77e] (bss)
001602f1: mov    rdx, qword ptr [rip + 0x3b8f598]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001602f8: cmp    dword ptr [rdx + 0xe0], 0
001602ff: jne    0x180160310
00160301: mov    rcx, rdx
00160304: call   0x182f60cf0
00160309: mov    rdx, qword ptr [rip + 0x3b8f580]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00160310: mov    rax, qword ptr [rdx + 0xb8]
00160317: mov    qword ptr [rsp + 0x30], rbx
0016031c: mov    rcx, qword ptr [rax + 8]
00160320: test   rcx, rcx
00160323: je     0x180160639
00160329: cmp    byte ptr [rcx + 0x74], 0
0016032d: jne    0x1801603d9
00160333: cmp    dword ptr [rdx + 0xe0], 0
0016033a: mov    rax, qword ptr [rip + 0x3b37fdf]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00160341: mov    rcx, qword ptr [rax + 0xb8]
00160348: mov    rbx, qword ptr [rcx + 0x10]
0016034c: jne    0x18016035d
0016034e: mov    rcx, rdx
00160351: call   0x182f60cf0
00160356: mov    rdx, qword ptr [rip + 0x3b8f533]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0016035d: mov    rax, qword ptr [rdx + 0xb8]
00160364: mov    rdx, qword ptr [rax + 8]
00160368: test   rdx, rdx
0016036b: je     0x180160639
00160371: test   rbx, rbx
00160374: je     0x180160639
0016037a: mov    r8, qword ptr [rip + 0x3b9684f]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00160381: mov    rcx, rbx
00160384: mov    edx, dword ptr [rdx + 0x5c]
00160387: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
0016038c: mov    rax, qword ptr [rip + 0x3b8f4fd]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00160393: mov    rcx, qword ptr [rax + 0xb8]
0016039a: mov    rax, qword ptr [rcx + 8]
0016039e: test   rax, rax
001603a1: je     0x180160639
001603a7: mov    byte ptr [rax + 0x75], 0
001603ab: mov    rax, qword ptr [rip + 0x3b87cf6]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
001603b2: mov    rcx, qword ptr [rax + 0xb8]
001603b9: mov    rcx, qword ptr [rcx]
001603bc: test   rcx, rcx
001603bf: je     0x180160639
001603c5: mov    rdx, qword ptr [rip + 0x3b59d7c]         ; [0x3cba148] str:' '
001603cc: xor    r8d, r8d
001603cf: call   0x180257f50                              ; FadeBehaviour$$ʶˁʽʷʻʾʿʺʾʲʽ
001603d4: jmp    0x18016061c
001603d9: cmp    byte ptr [rdi + 0xa1], 0
001603e0: jne    0x18016052d
001603e6: cmp    dword ptr [rdx + 0xe0], 0
001603ed: jne    0x1801603fe
001603ef: mov    rcx, rdx
001603f2: call   0x182f60cf0
001603f7: mov    rdx, qword ptr [rip + 0x3b8f492]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001603fe: mov    rax, qword ptr [rdx + 0xb8]
00160405: mov    rcx, qword ptr [rax + 8]
00160409: test   rcx, rcx
0016040c: je     0x180160639
00160412: inc    dword ptr [rcx + 0x58]
00160415: mov    rax, qword ptr [rip + 0x3b37f04]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0016041c: mov    rdx, qword ptr [rip + 0x3b3cc75]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00160423: mov    rcx, qword ptr [rax + 0xb8]
0016042a: cmp    dword ptr [rdx + 0xe0], 0
00160431: mov    rbx, qword ptr [rcx + 0x10]
00160435: jne    0x180160446
00160437: mov    rcx, rdx
0016043a: call   0x182f60cf0
0016043f: mov    rdx, qword ptr [rip + 0x3b3cc52]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00160446: mov    rax, qword ptr [rip + 0x3b8f443]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0016044d: mov    rcx, qword ptr [rax + 0xb8]
00160454: mov    r9, qword ptr [rcx + 8]
00160458: test   r9, r9
0016045b: je     0x180160639
00160461: mov    rax, qword ptr [rdx + 0xb8]
00160468: mov    rcx, qword ptr [rax + 0x28]
0016046c: test   rcx, rcx
0016046f: je     0x180160639
00160475: mov    r8, qword ptr [rip + 0x3b40014]          ; [0x3ca0490] metamethod:Method$System.Collections.Generic.List<int>.get_Item()
0016047c: mov    edx, dword ptr [r9 + 0x58]
00160480: call   0x1807452d0                              ; System.Collections.Generic.List<RegexCharClass.SingleRange>$$get_Item
00160485: test   rbx, rbx
00160488: je     0x180160639
0016048e: mov    r8, qword ptr [rip + 0x3b9673b]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00160495: mov    edx, eax
00160497: mov    rcx, rbx
0016049a: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
0016049f: mov    r8, qword ptr [rip + 0x3b8f3ea]          ; [0x3cef890] meta:GlobalVariables_TypeInfo
001604a6: mov    rax, qword ptr [r8 + 0xb8]
001604ad: mov    rdx, qword ptr [rax + 8]
001604b1: test   rdx, rdx
001604b4: je     0x180160639
001604ba: mov    rax, qword ptr [rip + 0x3b3cbd7]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
001604c1: mov    rcx, qword ptr [rax + 0xb8]
001604c8: mov    rcx, qword ptr [rcx + 0x20]
001604cc: test   rcx, rcx
001604cf: je     0x180160639
001604d5: mov    rax, qword ptr [r8 + 0xb8]
001604dc: mov    r8, qword ptr [rip + 0x3b561ed]          ; [0x3cb66d0] metamethod:Method$System.Collections.Generic.List<SongEntry>.get_Item()
001604e3: mov    edx, dword ptr [rdx + 0x58]
001604e6: mov    rbx, qword ptr [rax + 8]
001604ea: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
001604ef: lea    rcx, [rbx + 0x28]
001604f3: mov    qword ptr [rbx + 0x28], rax
001604f7: mov    rdx, rax
001604fa: call   0x182f5fc00
001604ff: mov    rax, qword ptr [rip + 0x3b87ba2]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
00160506: mov    rcx, qword ptr [rax + 0xb8]
0016050d: mov    rcx, qword ptr [rcx]
00160510: test   rcx, rcx
00160513: je     0x180160639
00160519: mov    rdx, qword ptr [rip + 0x3b6b510]         ; [0x3ccba30] str:'Main Menu'
00160520: xor    r8d, r8d
00160523: call   0x180257f50                              ; FadeBehaviour$$ʶˁʽʷʻʾʿʺʾʲʽ
00160528: jmp    0x18016061c
0016052d: cmp    dword ptr [rdx + 0xe0], 0
00160534: jne    0x180160545
00160536: mov    rcx, rdx
00160539: call   0x182f60cf0
0016053e: mov    rdx, qword ptr [rip + 0x3b8f34b]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00160545: mov    rax, qword ptr [rdx + 0xb8]
0016054c: mov    rcx, qword ptr [rax + 8]
00160550: test   rcx, rcx
00160553: je     0x180160639
00160559: mov    byte ptr [rcx + 0x75], 0
0016055d: mov    rcx, qword ptr [rip + 0x3b3cb34]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00160564: cmp    dword ptr [rcx + 0xe0], 0
0016056b: jne    0x180160572
0016056d: call   0x182f60cf0
00160572: xor    ecx, ecx
00160574: call   0x180171ff0                              ; ʾʲʽʻʺʶʺʺʷʿʺ$$ʴʿʽˁˀʼʷʴˀʴˁ
00160579: mov    rax, qword ptr [rip + 0x3b8f310]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00160580: mov    rcx, qword ptr [rax + 0xb8]
00160587: mov    rax, qword ptr [rcx + 8]
0016058b: test   rax, rax
0016058e: je     0x180160639
00160594: mov    byte ptr [rax + 0x74], 1
00160598: mov    rax, qword ptr [rip + 0x3b8f2f1]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
0016059f: mov    rcx, qword ptr [rax + 0xb8]
001605a6: mov    rax, qword ptr [rcx + 8]
001605aa: test   rax, rax
001605ad: je     0x180160639
001605b3: mov    dword ptr [rax + 0x58], 1
001605ba: mov    rax, qword ptr [rip + 0x3b8f2cf]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001605c1: mov    rcx, qword ptr [rax + 0xb8]
001605c8: mov    rdx, qword ptr [rcx + 8]
001605cc: test   rdx, rdx
001605cf: je     0x180160639
001605d1: mov    rax, qword ptr [rip + 0x3b37d48]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
001605d8: mov    rcx, qword ptr [rax + 0xb8]
001605df: mov    rcx, qword ptr [rcx + 0x10]
001605e3: test   rcx, rcx
001605e6: je     0x180160639
001605e8: mov    r8, qword ptr [rip + 0x3b965e1]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
001605ef: mov    edx, dword ptr [rdx + 0x5c]
001605f2: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
001605f7: mov    rax, qword ptr [rip + 0x3b87aaa]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
001605fe: mov    rcx, qword ptr [rax + 0xb8]
00160605: mov    rcx, qword ptr [rcx]
00160608: test   rcx, rcx
0016060b: je     0x180160639
0016060d: mov    rdx, qword ptr [rip + 0x3b8cee4]         ; [0x3ced4f8] str:'...'
00160614: xor    r8d, r8d
00160617: call   0x180257d90                              ; FadeBehaviour$$ʵʻʼˀˀʵʶʽʵʶʺ
0016061c: mov    rcx, qword ptr [rdi + 0xe8]
00160623: test   rcx, rcx
00160626: je     0x180160639
00160628: xor    edx, edx
0016062a: mov    rbx, qword ptr [rsp + 0x30]
0016062f: add    rsp, 0x20
00160633: pop    rdi
00160634: jmp    0x180251d20                              ; ʽʳʿˀʿˁʵʳʾʽʶ$$ʵʼʲʹʽʿʼʴʷʿʷ
00160639: call   0x182f60c50
0016063e: int3   
0016063f: int3   
