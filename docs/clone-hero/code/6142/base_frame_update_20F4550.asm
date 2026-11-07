020f4550: mov    qword ptr [rsp + 8], rbx
020f4555: push   rdi
020f4556: sub    rsp, 0x30
020f455a: cmp    byte ptr [rip + 0x1e24e4e], 0            ; [0x3f193af] (bss)
020f4561: movzx  edi, r8b
020f4565: movaps xmmword ptr [rsp + 0x20], xmm6
020f456a: mov    rbx, rcx
020f456d: movaps xmm6, xmm1
020f4570: jne    0x1820f4585
020f4572: lea    rcx, [rip + 0x1c19fd7]                   ; [0x3d0e550] metamethod:Method$System.Collections.Generic.List<double>.Clear()
020f4579: call   0x182f609b0
020f457e: mov    byte ptr [rip + 0x1e24e2a], 1            ; [0x3f193af] (bss)
020f4585: xor    r8d, r8d
020f4588: mov    byte ptr [rbx + 0x290], dil
020f458f: movaps xmm1, xmm6
020f4592: mov    rcx, rbx
020f4595: call   0x1820f4950                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʶʸʻˁʶˁʹʳʳʻʶ
020f459a: mov    rax, qword ptr [rbx]
020f459d: mov    rcx, rbx
020f45a0: mov    rdx, qword ptr [rax + 0x1d0]
020f45a7: call   qword ptr [rax + 0x1c8]
020f45ad: cmp    byte ptr [rbx + 0x290], 0
020f45b4: je     0x1820f45e0
020f45b6: mov    rax, qword ptr [rbx + 0x288]
020f45bd: test   rax, rax
020f45c0: je     0x1820f45cd
020f45c2: mov    rdx, qword ptr [rax + 0x28]
020f45c6: mov    rcx, qword ptr [rax + 0x40]
020f45ca: call   qword ptr [rax + 0x18]
020f45cd: mov    rax, qword ptr [rbx]
020f45d0: mov    rcx, rbx
020f45d3: mov    rdx, qword ptr [rax + 0x290]
020f45da: call   qword ptr [rax + 0x288]
020f45e0: mov    rax, qword ptr [rbx]
020f45e3: mov    rcx, rbx
020f45e6: mov    rdx, qword ptr [rax + 0x230]
020f45ed: call   qword ptr [rax + 0x228]
020f45f3: mov    rax, qword ptr [rbx]
020f45f6: mov    rcx, rbx
020f45f9: mov    rdx, qword ptr [rax + 0x260]
020f4600: call   qword ptr [rax + 0x258]
020f4606: mov    rax, qword ptr [rbx]
020f4609: mov    rcx, rbx
020f460c: mov    rdx, qword ptr [rax + 0x200]
020f4613: call   qword ptr [rax + 0x1f8]
020f4619: cmp    byte ptr [rbx + 0x290], 0
020f4620: je     0x1820f4638
020f4622: mov    rcx, qword ptr [rbx + 0x180]
020f4629: test   rcx, rcx
020f462c: je     0x1820f4648
020f462e: inc    dword ptr [rcx + 0x1c]
020f4631: mov    dword ptr [rcx + 0x18], 0
020f4638: mov    rbx, qword ptr [rsp + 0x40]
020f463d: movaps xmm6, xmmword ptr [rsp + 0x20]
020f4642: add    rsp, 0x30
020f4646: pop    rdi
020f4647: ret    
020f4648: call   0x182f60c50
020f464d: int3   
020f464e: int3   
