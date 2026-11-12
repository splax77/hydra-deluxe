001d57f0: mov    qword ptr [rsp + 8], rbx
001d57f5: push   rdi
001d57f6: sub    rsp, 0x20
001d57fa: mov    r11d, dword ptr [rcx + 0x40]
001d57fe: xor    edi, edi
001d5800: mov    r10, rcx
001d5803: mov    ebx, dword ptr [rcx + 0x3c]
001d5806: cmp    byte ptr [rcx + 0x9a], dil
001d580d: jne    0x1801d5888
001d580f: mov    r9d, dword ptr [rcx + 0x44]
001d5813: lea    eax, [rbx + r11]
001d5817: dec    r9d
001d581a: add    r9d, dword ptr [rcx + 0x48]
001d581e: cmp    eax, r9d
001d5821: je     0x1801d586d
001d5823: cmp    dword ptr [rcx + 0xdc], edi
001d5829: jle    0x1801d5867
001d582b: cmp    ebx, dword ptr [rcx + 0x44]
001d582e: jge    0x1801d5839
001d5830: cmp    r11d, dword ptr [rcx + 0xdc]
001d5837: jge    0x1801d5861
001d5839: cmp    byte ptr [rcx + 0x9d], dil
001d5840: je     0x1801d5867
001d5842: cmp    r11d, dword ptr [rcx + 0xdc]
001d5849: jl     0x1801d5867
001d584b: mov    eax, dword ptr [rcx + 0x44]
001d584e: cmp    ebx, eax
001d5850: jl     0x1801d5867
001d5852: mov    ecx, dword ptr [rcx + 0xdc]
001d5858: add    ecx, -3
001d585b: add    ecx, eax
001d585d: cmp    ebx, ecx
001d585f: jge    0x1801d5867
001d5861: inc    dword ptr [r10 + 0x3c]
001d5865: jmp    0x1801d5890
001d5867: inc    dword ptr [r10 + 0x40]
001d586b: jmp    0x1801d5890
001d586d: cmp    byte ptr [rcx + 0x2b], dil
001d5871: jne    0x1801d5890
001d5873: cmp    byte ptr [rcx + 0x98], dil
001d587a: je     0x1801d5890
001d587c: mov    qword ptr [rcx + 0x3c], rdi
001d5880: mov    dword ptr [rcx + 0xe4], edi
001d5886: jmp    0x1801d5890
001d5888: cmp    ebx, dword ptr [rcx + 0x44]
001d588b: jge    0x1801d5890
001d588d: inc    dword ptr [rcx + 0x3c]
001d5890: mov    byte ptr [r10 + 0x2e], 1
001d5895: cmp    dword ptr [r10 + 0x40], edi
001d5899: jne    0x1801d58a2
001d589b: mov    dword ptr [r10 + 0xe4], edi
001d58a2: cmp    dword ptr [r10 + 0x40], 7
001d58a7: jl     0x1801d58bd
001d58a9: cmp    dword ptr [r10 + 0x40], r11d
001d58ad: je     0x1801d58bd
001d58af: mov    eax, dword ptr [r10 + 0x40]
001d58b3: sub    eax, 6
001d58b6: mov    dword ptr [r10 + 0xe4], eax
001d58bd: test   dl, dl
001d58bf: je     0x1801d5908
001d58c1: cmp    r11d, dword ptr [r10 + 0x40]
001d58c5: jne    0x1801d58cd
001d58c7: cmp    ebx, dword ptr [r10 + 0x3c]
001d58cb: je     0x1801d5908
001d58cd: cmp    byte ptr [r10 + 0x11c], dil
001d58d4: je     0x1801d58f3
001d58d6: cmp    byte ptr [r10 + 0x11d], dil
001d58dd: je     0x1801d58e9
001d58df: movss  xmm1, dword ptr [rip + 0x2e8f3d9]        ; [0x3064cc0] flt=0.30000001192092896 i=1050253722
001d58e7: jmp    0x1801d58fb
001d58e9: movss  xmm1, dword ptr [rip + 0x2e8eac3]        ; [0x30643b4] flt=0.5 i=1056964608
001d58f1: jmp    0x1801d58fb
001d58f3: movss  xmm1, dword ptr [rip + 0x2e8e4c1]        ; [0x3063dbc] flt=1.0 i=1065353216
001d58fb: xor    r8d, r8d
001d58fe: mov    ecx, 0xf
001d5903: call   0x1801a6ef0                              ; ʹˁʾʼʼʹʸʹʿʺʽ$$ʾˁʶʵʺʸʽʾˀʸʸ
001d5908: mov    rbx, qword ptr [rsp + 0x30]
001d590d: add    rsp, 0x20
001d5911: pop    rdi
001d5912: ret    
001d5913: int3   
