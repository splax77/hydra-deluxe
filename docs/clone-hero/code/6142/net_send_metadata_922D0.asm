000922d0: push   rbx
000922d2: sub    rsp, 0x30
000922d6: mov    rax, rdx
000922d9: mov    rbx, rcx
000922dc: test   rdx, rdx
000922df: je     0x180092311
000922e1: mov    rdx, qword ptr [rcx + 0x78]
000922e5: xor    r8d, r8d
000922e8: mov    rcx, rax
000922eb: call   0x1801f0320                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʾʽʽʿʲʺʼˀʲʶʾ
000922f0: mov    rdx, qword ptr [rbx + 0x78]
000922f4: xor    r9d, r9d
000922f7: mov    r8b, 2
000922fa: mov    qword ptr [rsp + 0x20], 0
00092303: mov    rcx, rbx
00092306: call   0x1800322b0                              ; ʺʸˁʼˁʶʲʼʻˀʵ$$ˀˁʲʼʽʽʸʺʾʹʺ
0009230b: add    rsp, 0x30
0009230f: pop    rbx
00092310: ret    
00092311: call   0x182f60c50
00092316: int3   
00092317: int3   
