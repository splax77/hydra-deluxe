0215cc80: mov    qword ptr [rsp + 8], rbx
0215cc85: push   rdi
0215cc86: sub    rsp, 0x40
0215cc8a: cmp    byte ptr [rip + 0x1dbcac2], 0            ; [0x3f19753] (bss)
0215cc91: mov    rbx, rdx
0215cc94: mov    rdi, rcx
0215cc97: jne    0x18215ccac
0215cc99: lea    rcx, [rip + 0x1b758e0]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215cca0: call   0x182f609b0
0215cca5: mov    byte ptr [rip + 0x1dbcaa7], 1            ; [0x3f19753] (bss)
0215ccac: movzx  edx, byte ptr [rbx + 0x18]
0215ccb0: xor    r8d, r8d
0215ccb3: mov    rcx, rdi
0215ccb6: call   0x182160210                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʽʿʳʳʲʾʷʹʲʴ
0215ccbb: mov    r9, rax
0215ccbe: test   rax, rax
0215ccc1: je     0x18215cd40
0215ccc3: movups xmm0, xmmword ptr [rbx]
0215ccc6: mov    rax, qword ptr [rip + 0x1b758b3]         ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
0215cccd: movups xmm1, xmmword ptr [rbx + 0x10]
0215ccd1: inc    dword ptr [r9 + 0x1c]
0215ccd5: mov    rdx, qword ptr [r9 + 0x10]
0215ccd9: test   rdx, rdx
0215ccdc: je     0x18215cd40
0215ccde: movsxd rcx, dword ptr [r9 + 0x18]
0215cce2: cmp    ecx, dword ptr [rdx + 0x18]
0215cce5: jb     0x18215cd18
0215cce7: mov    rax, qword ptr [rax + 0x20]
0215cceb: lea    rdx, [rsp + 0x20]
0215ccf0: mov    rcx, r9
0215ccf3: movaps xmmword ptr [rsp + 0x20], xmm0
0215ccf8: movaps xmmword ptr [rsp + 0x30], xmm1
0215ccfd: mov    r8, qword ptr [rax + 0xc0]
0215cd04: mov    r8, qword ptr [r8 + 0x70]
0215cd08: call   0x180719fe0                              ; System.Collections.Generic.List<UIRStylePainter.RepeatRectUV>$$AddWithResize
0215cd0d: mov    rbx, qword ptr [rsp + 0x50]
0215cd12: add    rsp, 0x40
0215cd16: pop    rdi
0215cd17: ret    
0215cd18: lea    eax, [rcx + 1]
0215cd1b: mov    dword ptr [r9 + 0x18], eax
0215cd1f: cmp    ecx, dword ptr [rdx + 0x18]
0215cd22: jae    0x18215cd46
0215cd24: mov    rbx, qword ptr [rsp + 0x50]
0215cd29: lea    rax, [rcx + 1]
0215cd2d: shl    rax, 5
0215cd31: movups xmmword ptr [rax + rdx], xmm0
0215cd35: movups xmmword ptr [rax + rdx + 0x10], xmm1
0215cd3a: add    rsp, 0x40
0215cd3e: pop    rdi
0215cd3f: ret    
0215cd40: call   0x182f60c50
0215cd45: int3   
0215cd46: call   0x182f60c40
0215cd4b: int3   
0215cd4c: int3   
