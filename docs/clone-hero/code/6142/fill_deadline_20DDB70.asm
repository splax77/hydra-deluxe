020ddb70: push   rbx
020ddb72: push   rsi
020ddb73: push   rdi
020ddb74: push   r14
020ddb76: sub    rsp, 0x28
020ddb7a: mov    rax, qword ptr [rcx + 0x1b8]
020ddb81: mov    r14, rdx
020ddb84: mov    rbx, rcx
020ddb87: cmp    qword ptr [rcx + 0x68], rax
020ddb8b: jge    0x1820ddc53
020ddb91: mov    r8, qword ptr [rcx + 0x68]
020ddb95: add    r8, rdx
020ddb98: cmp    r8, rax
020ddb9b: jl     0x1820ddc53
020ddba1: cmp    byte ptr [rcx + 0x42], 0
020ddba5: jne    0x1820ddc53
020ddbab: cmp    byte ptr [rip + 0x1e3b70a], 0            ; [0x3f192bc] (bss)
020ddbb2: jne    0x1820ddbd3
020ddbb4: lea    rcx, [rip + 0x1bf0e85]                   ; [0x3ccea40] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Count()
020ddbbb: call   0x182f609b0
020ddbc0: lea    rcx, [rip + 0x1bf0f39]                   ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020ddbc7: call   0x182f609b0
020ddbcc: mov    byte ptr [rip + 0x1e3b6e9], 1            ; [0x3f192bc] (bss)
020ddbd3: mov    rsi, qword ptr [rbx + 0x300]
020ddbda: xor    edx, edx
020ddbdc: add    rsi, qword ptr [rbx + 0x108]
020ddbe3: mov    rcx, rbx
020ddbe6: mov    edi, dword ptr [rbx + 0x2f8]
020ddbec: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020ddbf1: test   rax, rax
020ddbf4: je     0x1820ddc6d
020ddbf6: cmp    edi, dword ptr [rax + 0x18]
020ddbf9: jge    0x1820ddc53
020ddbfb: xor    edx, edx
020ddbfd: mov    rcx, rbx
020ddc00: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020ddc05: test   rax, rax
020ddc08: je     0x1820ddc6d
020ddc0a: mov    r8, qword ptr [rip + 0x1bf0eef]          ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
020ddc11: mov    edx, edi
020ddc13: mov    rcx, rax
020ddc16: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020ddc1b: test   rax, rax
020ddc1e: je     0x1820ddc6d
020ddc20: cmp    qword ptr [rax + 0x28], rsi
020ddc24: jg     0x1820ddc4d
020ddc26: mov    rcx, qword ptr [rax + 0x20]
020ddc2a: mov    byte ptr [rax + 0x38], 0
020ddc2e: test   rcx, rcx
020ddc31: je     0x1820ddc6d
020ddc33: and    dword ptr [rcx + 0x7c], 0xfffff7ff
020ddc3a: xor    edx, edx
020ddc3c: mov    rcx, rbx
020ddc3f: inc    edi
020ddc41: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
020ddc46: test   rax, rax
020ddc49: je     0x1820ddc6d
020ddc4b: jmp    0x1820ddbf6
020ddc4d: mov    dword ptr [rbx + 0x2f8], edi
020ddc53: xor    r9d, r9d
020ddc56: mov    r8b, 1
020ddc59: mov    rdx, r14
020ddc5c: mov    rcx, rbx
020ddc5f: add    rsp, 0x28
020ddc63: pop    r14
020ddc65: pop    rdi
020ddc66: pop    rsi
020ddc67: pop    rbx
020ddc68: jmp    0x1820f4fa0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʸʸʳʽʲʻʿʸʳʸʽ
020ddc6d: call   0x182f60c50
020ddc72: int3   
020ddc73: int3   
