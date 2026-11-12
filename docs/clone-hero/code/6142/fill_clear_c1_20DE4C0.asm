020de4c0: mov    qword ptr [rsp + 8], rbx
020de4c5: mov    qword ptr [rsp + 0x10], rsi
020de4ca: push   rdi
020de4cb: sub    rsp, 0x20
020de4cf: cmp    byte ptr [rip + 0x1e3ade1], 0            ; [0x3f192b7] (bss)
020de4d6: mov    rbx, rcx
020de4d9: jne    0x1820de4fa
020de4db: lea    rcx, [rip + 0x1bf055e]                   ; [0x3ccea40] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Count()
020de4e2: call   0x182f609b0
020de4e7: lea    rcx, [rip + 0x1bf0612]                   ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020de4ee: call   0x182f609b0
020de4f3: mov    byte ptr [rip + 0x1e3adbd], 1            ; [0x3f192b7] (bss)
020de4fa: mov    rsi, qword ptr [rbx + 0x300]
020de501: xor    edx, edx
020de503: add    rsi, qword ptr [rbx + 0x108]
020de50a: mov    rcx, rbx
020de50d: mov    edi, dword ptr [rbx + 0x2f8]
020de513: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020de518: test   rax, rax
020de51b: je     0x1820de58b
020de51d: nop    dword ptr [rax]
020de520: cmp    edi, dword ptr [rax + 0x18]
020de523: jge    0x1820de57b
020de525: xor    edx, edx
020de527: mov    rcx, rbx
020de52a: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020de52f: test   rax, rax
020de532: je     0x1820de58b
020de534: mov    r8, qword ptr [rip + 0x1bf05c5]          ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020de53b: mov    edx, edi
020de53d: mov    rcx, rax
020de540: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020de545: test   rax, rax
020de548: je     0x1820de58b
020de54a: cmp    qword ptr [rax + 0x28], rsi
020de54e: jg     0x1820de575
020de550: mov    rcx, qword ptr [rax + 0x20]
020de554: mov    byte ptr [rax + 0x38], 0
020de558: test   rcx, rcx
020de55b: je     0x1820de58b
020de55d: and    dword ptr [rcx + 0x7c], 0xc1
020de564: xor    edx, edx
020de566: mov    rcx, rbx
020de569: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020de56e: test   rax, rax
020de571: je     0x1820de58b
020de573: jmp    0x1820de520
020de575: mov    dword ptr [rbx + 0x2f8], edi
020de57b: mov    rbx, qword ptr [rsp + 0x30]
020de580: mov    rsi, qword ptr [rsp + 0x38]
020de585: add    rsp, 0x20
020de589: pop    rdi
020de58a: ret    
020de58b: call   0x182f60c50
020de590: int3   
020de591: int3   
