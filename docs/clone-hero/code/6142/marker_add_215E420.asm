0215e420: mov    qword ptr [rsp + 8], rbx
0215e425: push   rdi
0215e426: sub    rsp, 0x40
0215e42a: cmp    byte ptr [rip + 0x1dbb31b], 0            ; [0x3f1974c] (bss)
0215e431: mov    rbx, rdx
0215e434: mov    rdi, rcx
0215e437: jne    0x18215e44c
0215e439: lea    rcx, [rip + 0x1b74140]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215e440: call   0x182f609b0
0215e445: mov    byte ptr [rip + 0x1dbb300], 1            ; [0x3f1974c] (bss)
0215e44c: movzx  edx, byte ptr [rbx + 0x18]
0215e450: xor    r8d, r8d
0215e453: mov    rcx, rdi
0215e456: call   0x182160210                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʽʿʳʳʲʾʷʹʲʴ
0215e45b: mov    r9, rax
0215e45e: test   rax, rax
0215e461: je     0x18215e4e0
0215e463: movups xmm0, xmmword ptr [rbx]
0215e466: mov    rax, qword ptr [rip + 0x1b74113]         ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215e46d: movups xmm1, xmmword ptr [rbx + 0x10]
0215e471: inc    dword ptr [r9 + 0x1c]
0215e475: mov    rdx, qword ptr [r9 + 0x10]
0215e479: test   rdx, rdx
0215e47c: je     0x18215e4e0
0215e47e: movsxd rcx, dword ptr [r9 + 0x18]
0215e482: cmp    ecx, dword ptr [rdx + 0x18]
0215e485: jb     0x18215e4b8
0215e487: mov    rax, qword ptr [rax + 0x20]
0215e48b: lea    rdx, [rsp + 0x20]
0215e490: mov    rcx, r9
0215e493: movaps xmmword ptr [rsp + 0x20], xmm0
0215e498: movaps xmmword ptr [rsp + 0x30], xmm1
0215e49d: mov    r8, qword ptr [rax + 0xc0]
0215e4a4: mov    r8, qword ptr [r8 + 0x70]
0215e4a8: call   0x180719fe0                              ; System.Collections.Generic.List<UIRStylePainter.RepeatRectUV>$$AddWithResize
0215e4ad: mov    rbx, qword ptr [rsp + 0x50]
0215e4b2: add    rsp, 0x40
0215e4b6: pop    rdi
0215e4b7: ret    
0215e4b8: lea    eax, [rcx + 1]
0215e4bb: mov    dword ptr [r9 + 0x18], eax
0215e4bf: cmp    ecx, dword ptr [rdx + 0x18]
0215e4c2: jae    0x18215e4e6
0215e4c4: mov    rbx, qword ptr [rsp + 0x50]
0215e4c9: lea    rax, [rcx + 1]
0215e4cd: shl    rax, 5
0215e4d1: movups xmmword ptr [rax + rdx], xmm0
0215e4d5: movups xmmword ptr [rax + rdx + 0x10], xmm1
0215e4da: add    rsp, 0x40
0215e4de: pop    rdi
0215e4df: ret    
0215e4e0: call   0x182f60c50
0215e4e5: int3   
0215e4e6: call   0x182f60c40
0215e4eb: int3   
0215e4ec: int3   
