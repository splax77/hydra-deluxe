020f5640: push   rbx
020f5642: sub    rsp, 0x20
020f5646: cmp    byte ptr [rip + 0x1e23d6a], 0            ; [0x3f193b7] (bss)
020f564d: mov    rbx, rcx
020f5650: jne    0x1820f5665
020f5652: lea    rcx, [rip + 0x1c19647]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f5659: call   0x182f609b0
020f565e: mov    byte ptr [rip + 0x1e23d52], 1            ; [0x3f193b7] (bss)
020f5665: mov    rcx, qword ptr [rbx + 0x148]
020f566c: mov    byte ptr [rbx + 0x42], 1
020f5670: mov    rax, qword ptr [rbx + 0x100]
020f5677: mov    qword ptr [rbx + 0x110], rax
020f567e: test   rcx, rcx
020f5681: je     0x1820f5729
020f5687: mov    rax, qword ptr [rbx + 0x230]
020f568e: test   rax, rax
020f5691: je     0x1820f5729
020f5697: movsd  xmm0, qword ptr [rbx + 0x100]
020f569f: xor    r9d, r9d
020f56a2: subsd  xmm0, qword ptr [rcx + 0x18]
020f56a7: movsd  xmm2, qword ptr [rax + 0xb0]
020f56af: mov    rdx, rcx
020f56b2: call   0x1820d1d20                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʷʾʻʿʵʻʸʵʺʹʺ
020f56b7: mov    rax, qword ptr [rbx + 0x148]
020f56be: test   rax, rax
020f56c1: je     0x1820f5729
020f56c3: addsd  xmm0, qword ptr [rax + 0x38]
020f56c8: movsd  xmm2, qword ptr [rbx + 0x110]
020f56d0: xor    r9d, r9d
020f56d3: mov    rdx, qword ptr [rbx + 0x68]
020f56d7: mov    rcx, rbx
020f56da: cvttsd2si rax, xmm0
020f56df: mov    qword ptr [rbx + 0x118], rax
020f56e6: call   0x1820f37e0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʳʼʽʽʸʽʺʲʼʹʻ
020f56eb: mov    eax, dword ptr [rbx + 0x48]
020f56ee: inc    dword ptr [rbx + 0xb4]
020f56f4: add    eax, eax
020f56f6: mov    r8, qword ptr [rbx + 0x1a0]
020f56fd: movsd  qword ptr [rbx + 0x120], xmm0
020f5705: mov    dword ptr [rbx + 0x48], eax
020f5708: test   r8, r8
020f570b: je     0x1820f5723
020f570d: mov    rdx, qword ptr [rip + 0x1c1958c]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f5714: mov    ecx, 0xb
020f5719: add    rsp, 0x20
020f571d: pop    rbx
020f571e: jmp    0x182c42550
020f5723: add    rsp, 0x20
020f5727: pop    rbx
020f5728: ret    
020f5729: call   0x182f60c50
020f572e: int3   
020f572f: int3   
