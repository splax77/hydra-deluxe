020df3e0: mov    qword ptr [rsp + 8], rbx
020df3e5: mov    qword ptr [rsp + 0x10], rsi
020df3ea: push   rdi
020df3eb: sub    rsp, 0x20
020df3ef: cmp    byte ptr [rip + 0x1e39ec6], 0            ; [0x3f192bc] (bss)
020df3f6: mov    rdi, rcx
020df3f9: jne    0x1820df41a
020df3fb: lea    rcx, [rip + 0x1bef63e]                   ; [0x3ccea40] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Count()
020df402: call   0x182f609b0
020df407: lea    rcx, [rip + 0x1bef6f2]                   ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020df40e: call   0x182f609b0
020df413: mov    byte ptr [rip + 0x1e39ea2], 1            ; [0x3f192bc] (bss)
020df41a: mov    rsi, qword ptr [rdi + 0x300]
020df421: xor    edx, edx
020df423: add    rsi, qword ptr [rdi + 0x108]
020df42a: mov    rcx, rdi
020df42d: mov    ebx, dword ptr [rdi + 0x2f8]
020df433: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020df438: test   rax, rax
020df43b: je     0x1820df4ad
020df43d: nop    dword ptr [rax]
020df440: cmp    ebx, dword ptr [rax + 0x18]
020df443: jge    0x1820df49d
020df445: xor    edx, edx
020df447: mov    rcx, rdi
020df44a: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020df44f: test   rax, rax
020df452: je     0x1820df4ad
020df454: mov    r8, qword ptr [rip + 0x1bef6a5]          ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020df45b: mov    edx, ebx
020df45d: mov    rcx, rax
020df460: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020df465: test   rax, rax
020df468: je     0x1820df4ad
020df46a: cmp    qword ptr [rax + 0x28], rsi
020df46e: jg     0x1820df497
020df470: mov    rcx, qword ptr [rax + 0x20]
020df474: mov    byte ptr [rax + 0x38], 0
020df478: test   rcx, rcx
020df47b: je     0x1820df4ad
020df47d: and    dword ptr [rcx + 0x7c], 0xfffff7ff
020df484: xor    edx, edx
020df486: mov    rcx, rdi
020df489: inc    ebx
020df48b: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020df490: test   rax, rax
020df493: je     0x1820df4ad
020df495: jmp    0x1820df440
020df497: mov    dword ptr [rdi + 0x2f8], ebx
020df49d: mov    rbx, qword ptr [rsp + 0x30]
020df4a2: mov    rsi, qword ptr [rsp + 0x38]
020df4a7: add    rsp, 0x20
020df4ab: pop    rdi
020df4ac: ret    
020df4ad: call   0x182f60c50
020df4b2: int3   
020df4b3: int3   
