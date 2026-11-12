; function 0x20f62c0 ʿʶʴˀʴʾʵʵʶʳʲ$$ʽˀʺʺʺʷʶʸʶʲʺ next method starts at 0x20f6390 bytes 0xd0
020f62c0: push   rbx
020f62c2: sub    rsp, 0x20
020f62c6: cmp    byte ptr [rip + 0x1e230f9], 0            ; [0x3f193c6] (bss)
020f62cd: mov    rbx, rcx
020f62d0: jne    0x1820f62f1
020f62d2: lea    rcx, [rip + 0x1be2107]                   ; [0x3cd83e0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.get_Item()
020f62d9: call   0x182f609b0
020f62de: lea    rcx, [rip + 0x1c189bb]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f62e5: call   0x182f609b0
020f62ea: mov    byte ptr [rip + 0x1e230d5], 1            ; [0x3f193c6] (bss)
020f62f1: cmp    byte ptr [rbx + 0x43], 0
020f62f5: jne    0x1820f637d
020f62fb: mov    rcx, qword ptr [rbx + 0x200]
020f6302: test   rcx, rcx
020f6305: je     0x1820f6383
020f6307: mov    r8, qword ptr [rip + 0x1be20d2]          ; [0x3cd83e0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.get_Item()
020f630e: mov    edx, dword ptr [rbx + 0x160]
020f6314: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f6319: test   rax, rax
020f631c: je     0x1820f6383
020f631e: mov    eax, dword ptr [rax + 0x10]
020f6321: mov    dword ptr [rbx + 0x16c], eax
020f6327: mov    rcx, qword ptr [rbx + 0x200]
020f632e: test   rcx, rcx
020f6331: je     0x1820f6383
020f6333: mov    r8, qword ptr [rip + 0x1be20a6]          ; [0x3cd83e0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.get_Item()
020f633a: mov    edx, dword ptr [rbx + 0x160]
020f6340: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f6345: test   rax, rax
020f6348: je     0x1820f6383
020f634a: xor    edx, edx
020f634c: mov    rcx, rax
020f634f: call   0x18210dae0                              ; ʿʶˁˀˁʻʲʷˀʼʲ$$ˁʼʼʴʼʺʾʼʺʷʴ
020f6354: mov    r8, qword ptr [rbx + 0x1a0]
020f635b: movsd  qword ptr [rbx + 0x170], xmm0
020f6363: test   r8, r8
020f6366: je     0x1820f6379
020f6368: mov    rdx, qword ptr [rip + 0x1c18931]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f636f: mov    ecx, 2
020f6374: call   0x182c42550
020f6379: mov    byte ptr [rbx + 0x43], 1
020f637d: add    rsp, 0x20
020f6381: pop    rbx
020f6382: ret    
020f6383: call   0x182f60c50
020f6388: int3   
020f6389: int3   
