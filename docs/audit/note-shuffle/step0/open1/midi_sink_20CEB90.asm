020ceb90: mov    qword ptr [rsp + 8], rbx
020ceb95: push   rdi
020ceb96: sub    rsp, 0x40
020ceb9a: cmp    byte ptr [rip + 0x1e4a782], 0            ; [0x3f19323] (bss)
020ceba1: mov    rbx, rdx
020ceba4: mov    rdi, rcx
020ceba7: jne    0x1820cebbc
020ceba9: lea    rcx, [rip + 0x1c039d0]                   ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
020cebb0: call   0x182f609b0
020cebb5: mov    byte ptr [rip + 0x1e4a767], 1            ; [0x3f19323] (bss)
020cebbc: movzx  edx, byte ptr [rbx + 0x18]
020cebc0: xor    r8d, r8d
020cebc3: mov    rcx, rdi
020cebc6: call   0x182160210                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ˁʽʿʳʳʲʾʷʹʲʴ
020cebcb: mov    r9, rax
020cebce: test   rax, rax
020cebd1: je     0x1820cec50
020cebd3: movups xmm0, xmmword ptr [rbx]
020cebd6: mov    rax, qword ptr [rip + 0x1c039a3]         ; [0x3cd2580] metamethod:Method$System.Collections.Generic.List<ʸʻˁʴʿʶʶʳʸʶʳ>.Add()
020cebdd: movups xmm1, xmmword ptr [rbx + 0x10]
020cebe1: inc    dword ptr [r9 + 0x1c]
020cebe5: mov    rdx, qword ptr [r9 + 0x10]
020cebe9: test   rdx, rdx
020cebec: je     0x1820cec50
020cebee: movsxd rcx, dword ptr [r9 + 0x18]
020cebf2: cmp    ecx, dword ptr [rdx + 0x18]
020cebf5: jb     0x1820cec28
020cebf7: mov    rax, qword ptr [rax + 0x20]
020cebfb: lea    rdx, [rsp + 0x20]
020cec00: mov    rcx, r9
020cec03: movaps xmmword ptr [rsp + 0x20], xmm0
020cec08: movaps xmmword ptr [rsp + 0x30], xmm1
020cec0d: mov    r8, qword ptr [rax + 0xc0]
020cec14: mov    r8, qword ptr [r8 + 0x70]
020cec18: call   0x180719fe0                              ; System.Collections.Generic.List<UIRStylePainter.RepeatRectUV>$$AddWithResize
020cec1d: mov    rbx, qword ptr [rsp + 0x50]
020cec22: add    rsp, 0x40
020cec26: pop    rdi
020cec27: ret    
020cec28: lea    eax, [rcx + 1]
020cec2b: mov    dword ptr [r9 + 0x18], eax
020cec2f: cmp    ecx, dword ptr [rdx + 0x18]
020cec32: jae    0x1820cec56
020cec34: mov    rbx, qword ptr [rsp + 0x50]
020cec39: lea    rax, [rcx + 1]
020cec3d: shl    rax, 5
020cec41: movups xmmword ptr [rax + rdx], xmm0
020cec45: movups xmmword ptr [rax + rdx + 0x10], xmm1
020cec4a: add    rsp, 0x40
020cec4e: pop    rdi
020cec4f: ret    
020cec50: call   0x182f60c50
020cec55: int3   
020cec56: call   0x182f60c40
020cec5b: int3   
020cec5c: int3   
