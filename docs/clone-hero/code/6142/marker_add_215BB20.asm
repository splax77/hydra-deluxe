0215bb20: mov    qword ptr [rsp + 8], rbx
0215bb25: push   rdi
0215bb26: sub    rsp, 0x40
0215bb2a: cmp    byte ptr [rip + 0x1dbdc14], 0            ; [0x3f19745] (bss)
0215bb31: mov    rbx, rdx
0215bb34: mov    rdi, rcx
0215bb37: jne    0x18215bb4c
0215bb39: lea    rcx, [rip + 0x1b76a40]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215bb40: call   0x182f609b0
0215bb45: mov    byte ptr [rip + 0x1dbdbf9], 1            ; [0x3f19745] (bss)
0215bb4c: movzx  edx, byte ptr [rbx + 0x18]
0215bb50: xor    r8d, r8d
0215bb53: mov    rcx, rdi
0215bb56: call   0x182160210                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʽʿʳʳʲʾʷʹʲʴ
0215bb5b: mov    r9, rax
0215bb5e: test   rax, rax
0215bb61: je     0x18215bbe0
0215bb63: movups xmm0, xmmword ptr [rbx]
0215bb66: mov    rax, qword ptr [rip + 0x1b76a13]         ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215bb6d: movups xmm1, xmmword ptr [rbx + 0x10]
0215bb71: inc    dword ptr [r9 + 0x1c]
0215bb75: mov    rdx, qword ptr [r9 + 0x10]
0215bb79: test   rdx, rdx
0215bb7c: je     0x18215bbe0
0215bb7e: movsxd rcx, dword ptr [r9 + 0x18]
0215bb82: cmp    ecx, dword ptr [rdx + 0x18]
0215bb85: jb     0x18215bbb8
0215bb87: mov    rax, qword ptr [rax + 0x20]
0215bb8b: lea    rdx, [rsp + 0x20]
0215bb90: mov    rcx, r9
0215bb93: movaps xmmword ptr [rsp + 0x20], xmm0
0215bb98: movaps xmmword ptr [rsp + 0x30], xmm1
0215bb9d: mov    r8, qword ptr [rax + 0xc0]
0215bba4: mov    r8, qword ptr [r8 + 0x70]
0215bba8: call   0x180719fe0                              ; System.Collections.Generic.List<UIRStylePainter.RepeatRectUV>$$AddWithResize
0215bbad: mov    rbx, qword ptr [rsp + 0x50]
0215bbb2: add    rsp, 0x40
0215bbb6: pop    rdi
0215bbb7: ret    
0215bbb8: lea    eax, [rcx + 1]
0215bbbb: mov    dword ptr [r9 + 0x18], eax
0215bbbf: cmp    ecx, dword ptr [rdx + 0x18]
0215bbc2: jae    0x18215bbe6
0215bbc4: mov    rbx, qword ptr [rsp + 0x50]
0215bbc9: lea    rax, [rcx + 1]
0215bbcd: shl    rax, 5
0215bbd1: movups xmmword ptr [rax + rdx], xmm0
0215bbd5: movups xmmword ptr [rax + rdx + 0x10], xmm1
0215bbda: add    rsp, 0x40
0215bbde: pop    rdi
0215bbdf: ret    
0215bbe0: call   0x182f60c50
0215bbe5: int3   
0215bbe6: call   0x182f60c40
0215bbeb: int3   
0215bbec: int3   
