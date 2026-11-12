00164940: mov    ecx, dword ptr [rax + 0xb8]
00164946: add    rcx, 0x20
0016494a: call   0x182f5fc00
0016494f: mov    rcx, qword ptr [rip + 0x3b2f92a]         ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
00164956: cmp    dword ptr [rcx + 0xe0], 0
0016495d: jne    0x180164964
0016495f: call   0x182f60cf0
00164964: xor    ecx, ecx
00164966: call   0x180307b40                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʴˁʿʸʷˁʷʳʴʻʷ
0016496b: mov    rcx, qword ptr [rip + 0x3b527de]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
00164972: mov    rbx, rax
00164975: cmp    dword ptr [rcx + 0xe0], 0
0016497c: jne    0x180164983
0016497e: call   0x182f60cf0
00164983: mov    rdx, qword ptr [rip + 0x3b4884e]         ; [0x3cad1d8] str:'settings_schema.json'
0016498a: xor    r8d, r8d
0016498d: mov    rcx, rbx
00164990: call   0x181b1eee0                              ; System.IO.Path$$Combine
00164995: cmp    byte ptr [rip + 0x3da7e24], 0            ; [0x3f0c7c0] (bss)
0016499c: mov    rbx, rax
0016499f: jne    0x1801649b4
001649a1: lea    rcx, [rip + 0x3ba95c0]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001649a8: call   0x182f609b0
001649ad: mov    byte ptr [rip + 0x3da7e0c], 1            ; [0x3f0c7c0] (bss)
001649b4: mov    rax, qword ptr [rip + 0x3ba95ad]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001649bb: cmp    dword ptr [rax + 0xe0], 0
001649c2: jne    0x1801649d3
001649c4: mov    rcx, rax
001649c7: call   0x182f60cf0
001649cc: mov    rax, qword ptr [rip + 0x3ba9595]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001649d3: mov    rax, qword ptr [rax + 0xb8]
001649da: mov    rdx, rbx
001649dd: mov    qword ptr [rsp + 0x30], rdi
001649e2: mov    qword ptr [rax + 0x28], rbx
001649e6: mov    rax, qword ptr [rip + 0x3ba957b]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
001649ed: mov    rcx, qword ptr [rax + 0xb8]
001649f4: add    rcx, 0x28
001649f8: call   0x182f5fc00
