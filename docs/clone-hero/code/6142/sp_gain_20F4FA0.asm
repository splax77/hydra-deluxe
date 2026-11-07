020f4fa0: mov    qword ptr [rsp + 8], rbx
020f4fa5: mov    qword ptr [rsp + 0x10], rsi
020f4faa: push   rdi
020f4fab: sub    rsp, 0x30
020f4faf: cmp    byte ptr [rip + 0x1e243f7], 0            ; [0x3f193ad] (bss)
020f4fb6: movzx  esi, r8b
020f4fba: mov    rdi, rdx
020f4fbd: mov    rbx, rcx
020f4fc0: jne    0x1820f4fd5
020f4fc2: lea    rcx, [rip + 0x1c19cd7]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f4fc9: call   0x182f609b0
020f4fce: mov    byte ptr [rip + 0x1e243d8], 1            ; [0x3f193ad] (bss)
020f4fd5: cmp    dword ptr [rbx + 0x190], 2
020f4fdc: je     0x1820f509f
020f4fe2: cmp    dword ptr [rbx + 0x190], 1
020f4fe9: je     0x1820f509f
020f4fef: test   rdi, rdi
020f4ff2: js     0x1820f50af
020f4ff8: mov    rdx, qword ptr [rbx + 0x68]
020f4ffc: add    qword ptr [rbx + 0x80], rdi
020f5003: add    rdx, rdi
020f5006: mov    qword ptr [rbx + 0x68], rdx
020f500a: test   sil, sil
020f500d: jne    0x1820f503a
020f500f: cmp    byte ptr [rbx + 0x42], sil
020f5013: jne    0x1820f505a
020f5015: mov    rax, qword ptr [rbx + 0x1b8]
020f501c: cmp    qword ptr [rbx + 0x60], rax
020f5020: jge    0x1820f505a
020f5022: cmp    rdx, rax
020f5025: jl     0x1820f505a
020f5027: mov    r8, qword ptr [rbx + 0x1a0]
020f502e: test   r8, r8
020f5031: je     0x1820f505a
020f5033: mov    ecx, 0xd
020f5038: jmp    0x1820f504e
020f503a: inc    dword ptr [rbx + 0x4c]
020f503d: mov    r8, qword ptr [rbx + 0x1a0]
020f5044: test   r8, r8
020f5047: je     0x1820f505a
020f5049: mov    ecx, 1
020f504e: mov    rdx, qword ptr [rip + 0x1c19c4b]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f5055: call   0x182c42550
020f505a: mov    r8, qword ptr [rbx + 0x1a0]
020f5061: mov    byte ptr [rbx + 0x130], 1
020f5068: test   r8, r8
020f506b: je     0x1820f509f
020f506d: movzx  eax, byte ptr [rbx + 0x42]
020f5071: xorps  xmm3, xmm3
020f5074: cvtsi2ss xmm3, qword ptr [rbx + 0x68]
020f507a: mov    rdx, qword ptr [rip + 0x1c19c1f]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f5081: mov    ecx, 0xf
020f5086: mov    byte ptr [rsp + 0x20], al
020f508a: xorps  xmm0, xmm0
020f508d: cvtsi2ss xmm0, qword ptr [rbx + 0x1b0]
020f5096: divss  xmm3, xmm0
020f509a: call   0x182efc240
020f509f: mov    rbx, qword ptr [rsp + 0x40]
020f50a4: mov    rsi, qword ptr [rsp + 0x48]
020f50a9: add    rsp, 0x30
020f50ad: pop    rdi
020f50ae: ret    
020f50af: lea    rcx, [rip + 0x1bc1592]                   ; [0x3cb6648] meta:System.ArgumentOutOfRangeException_TypeInfo
020f50b6: call   0x182f609d0
020f50bb: mov    rcx, rax
020f50be: call   0x182f60c00
020f50c3: lea    rcx, [rip + 0x1bf82f6]                   ; [0x3ced3c0] str:'Star power gain amount must be greater than 0'
020f50ca: mov    rdi, rax
020f50cd: call   0x182f609d0
020f50d2: lea    rcx, [rip + 0x1ba8aef]                   ; [0x3c9dbc8] str:'amount'
020f50d9: mov    rbx, rax
020f50dc: call   0x182f609d0
020f50e1: xor    r9d, r9d
020f50e4: mov    r8, rbx
020f50e7: mov    rdx, rax
020f50ea: mov    rcx, rdi
020f50ed: call   0x181a8bdd0                              ; System.ArgumentOutOfRangeException$$.ctor
020f50f2: lea    rcx, [rip + 0x1ba7477]                   ; [0x3c9c570] metamethod:Method$ʿʶʴˀʴʾʵʵʶʳʲ.ʸʸʳʽʲʻʿʸʳʸʽ()
020f50f9: call   0x182f609d0
020f50fe: mov    rdx, rax
020f5101: mov    rcx, rdi
020f5104: call   0x182f60c10
020f5109: int3   
020f510a: int3   
