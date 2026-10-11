020f5540: mov    qword ptr [rsp + 0x10], rbx
020f5545: push   rsi
020f5546: sub    rsp, 0x20
020f554a: xor    eax, eax
020f554c: mov    rbx, rcx
020f554f: cmp    byte ptr [rip + 0x1e23e65], al           ; [0x3f193ba] (bss)
020f5555: mov    qword ptr [rcx + 0x8c], rax
020f555c: jne    0x1820f557d
020f555e: lea    rcx, [rip + 0x1bdf883]                   ; [0x3cd4de8] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.get_Count()
020f5565: call   0x182f609b0
020f556a: lea    rcx, [rip + 0x1bdf92f]                   ; [0x3cd4ea0] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.get_Item()
020f5571: call   0x182f609b0
020f5576: mov    byte ptr [rip + 0x1e23e3d], 1            ; [0x3f193ba] (bss)
020f557d: mov    rsi, qword ptr [rbx + 0x1f8]
020f5584: mov    qword ptr [rsp + 0x30], rdi
020f5589: test   rsi, rsi
020f558c: je     0x1820f55f6
020f558e: mov    edi, dword ptr [rbx + 0x294]
020f5594: cmp    edi, dword ptr [rsi + 0x18]
020f5597: jge    0x1820f55f6
020f5599: movzx  eax, byte ptr [rbx + 0xf0]
020f55a0: mov    edx, edi
020f55a2: mov    byte ptr [rbx + 0xf1], al
020f55a8: mov    rcx, rsi
020f55ab: mov    r8, qword ptr [rip + 0x1bdf8ee]          ; [0x3cd4ea0] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.get_Item()
020f55b2: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f55b7: test   rax, rax
020f55ba: je     0x1820f563a
020f55bc: mov    rcx, qword ptr [rax + 0x20]
020f55c0: cmp    qword ptr [rbx + 0x108], rcx
020f55c7: jl     0x1820f55da
020f55c9: mov    rcx, qword ptr [rax + 0x28]
020f55cd: add    rcx, qword ptr [rax + 0x20]
020f55d1: cmp    qword ptr [rbx + 0x108], rcx
020f55d8: jl     0x1820f5631
020f55da: cmp    byte ptr [rbx + 0xf0], 0
020f55e1: jne    0x1820f5623
020f55e3: mov    rax, qword ptr [rax + 0x20]
020f55e7: cmp    qword ptr [rbx + 0x108], rax
020f55ee: jge    0x1820f562a
020f55f0: mov    dword ptr [rbx + 0x294], edi
020f55f6: xor    edx, edx
020f55f8: mov    rcx, rbx
020f55fb: call   0x1820f32e0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʲʴʼʻʺʶʳʸˀʵʻ
020f5600: mov    rax, qword ptr [rbx]
020f5603: mov    rcx, rbx
020f5606: mov    rdx, qword ptr [rax + 0x210]
020f560d: mov    rdi, qword ptr [rsp + 0x30]
020f5612: mov    rbx, qword ptr [rsp + 0x38]
020f5617: add    rsp, 0x20
020f561b: pop    rsi
020f561c: jmp    qword ptr [rax + 0x208]
020f5623: mov    byte ptr [rbx + 0xf0], 0
020f562a: inc    edi
020f562c: jmp    0x1820f5594
020f5631: mov    byte ptr [rbx + 0xf0], 1
020f5638: jmp    0x1820f55f6
020f563a: call   0x182f60c50
020f563f: int3   
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
