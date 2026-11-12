00330f30: push   rbx
00330f32: sub    rsp, 0x20
00330f36: cmp    byte ptr [rip + 0x3bdcdfc], 0            ; [0x3f0dd39] (bss)
00330f3d: mov    rbx, rcx
00330f40: jne    0x180330f55
00330f42: lea    rcx, [rip + 0x39de0ff]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
00330f49: call   0x182f609b0
00330f4e: mov    byte ptr [rip + 0x3bdcde4], 1            ; [0x3f0dd39] (bss)
00330f55: cmp    dword ptr [rbx + 0x120], 4
00330f5c: je     0x180330f96
00330f5e: xor    edx, edx
00330f60: mov    rcx, rbx
00330f63: call   0x1801d6ae0                              ; BaseMenu$$ˁʳʼʶʽʶʿʷʻʵʻ
00330f68: cmp    byte ptr [rbx + 0x245], 0
00330f6f: je     0x180330f96
00330f71: mov    rax, qword ptr [rip + 0x39de0d0]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
00330f78: mov    rcx, qword ptr [rax + 0xb8]
00330f7f: mov    rcx, qword ptr [rcx]
00330f82: test   rcx, rcx
00330f85: je     0x180330f9c
00330f87: xor    r8d, r8d
00330f8a: mov    dl, 2
00330f8c: add    rsp, 0x20
00330f90: pop    rbx
00330f91: jmp    0x180092910                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʿʹˀʿʺʺʲʺʸʵʼ
00330f96: add    rsp, 0x20
00330f9a: pop    rbx
00330f9b: ret    
00330f9c: call   0x182f60c50
00330fa1: int3   
00330fa2: int3   
