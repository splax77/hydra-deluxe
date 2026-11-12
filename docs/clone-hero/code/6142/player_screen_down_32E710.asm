0032e710: push   rbx
0032e712: sub    rsp, 0x20
0032e716: cmp    byte ptr [rip + 0x3bdf60d], 0            ; [0x3f0dd2a] (bss)
0032e71d: mov    rbx, rcx
0032e720: jne    0x18032e735
0032e722: lea    rcx, [rip + 0x39e091f]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032e729: call   0x182f609b0
0032e72e: mov    byte ptr [rip + 0x3bdf5f5], 1            ; [0x3f0dd2a] (bss)
0032e735: cmp    dword ptr [rbx + 0x120], 4
0032e73c: je     0x18032e776
0032e73e: xor    edx, edx
0032e740: mov    rcx, rbx
0032e743: call   0x1801d5290                              ; BaseMenu$$ʷʺˀʸʾʺʽʴʻʳʶ
0032e748: cmp    byte ptr [rbx + 0x245], 0
0032e74f: je     0x18032e776
0032e751: mov    rax, qword ptr [rip + 0x39e08f0]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
0032e758: mov    rcx, qword ptr [rax + 0xb8]
0032e75f: mov    rcx, qword ptr [rcx]
0032e762: test   rcx, rcx
0032e765: je     0x18032e77c
0032e767: xor    r8d, r8d
0032e76a: mov    dl, 3
0032e76c: add    rsp, 0x20
0032e770: pop    rbx
0032e771: jmp    0x180092910                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʿʹˀʿʺʺʲʺʸʵʼ
0032e776: add    rsp, 0x20
0032e77a: pop    rbx
0032e77b: ret    
0032e77c: call   0x182f60c50
0032e781: int3   
0032e782: int3   
