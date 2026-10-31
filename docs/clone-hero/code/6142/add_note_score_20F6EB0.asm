020f6eb0: mov    qword ptr [rsp + 0x18], rbp
020f6eb5: mov    qword ptr [rsp + 0x20], rsi
020f6eba: push   r14
020f6ebc: sub    rsp, 0x20
020f6ec0: cmp    byte ptr [rip + 0x1e224e5], 0            ; [0x3f193ac] (bss)
020f6ec7: mov    r14d, r8d
020f6eca: mov    rbp, rdx
020f6ecd: mov    rsi, rcx
020f6ed0: jne    0x1820f6ee5
020f6ed2: lea    rcx, [rip + 0x1ba196f]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f6ed9: call   0x182f609b0
020f6ede: mov    byte ptr [rip + 0x1e224c7], 1            ; [0x3f193ac] (bss)
020f6ee5: cmp    dword ptr [rsi + 0x190], 1
020f6eec: je     0x1820f6f93
020f6ef2: test   rbp, rbp
020f6ef5: je     0x1820f6fde
020f6efb: cmp    dword ptr [rbp + 0x50], -1
020f6eff: mov    qword ptr [rsp + 0x30], rbx
020f6f04: mov    qword ptr [rsp + 0x38], rdi
020f6f09: jne    0x1820f6f3c
020f6f0b: mov    rcx, qword ptr [rip + 0x1ba1936]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020f6f12: movzx  edi, word ptr [rbp + 0x80]
020f6f19: cmp    dword ptr [rcx + 0xe0], 0
020f6f20: jne    0x1820f6f27
020f6f22: call   0x182f60cf0
020f6f27: xor    ebx, ebx
020f6f29: test   di, di
020f6f2c: je     0x1820f6f3f
020f6f2e: nop    
020f6f30: inc    ebx
020f6f32: lea    eax, [rdi - 1]
020f6f35: and    di, ax
020f6f38: jne    0x1820f6f30
020f6f3a: jmp    0x1820f6f3f
020f6f3c: mov    ebx, dword ptr [rbp + 0x50]
020f6f3f: mov    r8d, dword ptr [rsi + 0xc8]
020f6f46: imul   ebx, r14d
020f6f4a: add    dword ptr [rsi + 0xc0], ebx
020f6f50: mov    eax, ebx
020f6f52: imul   eax, dword ptr [rsi + 0x48]
020f6f56: add    dword ptr [rsi + 0x94], eax
020f6f5c: cmp    byte ptr [rsi + 0x42], 0
020f6f60: mov    eax, dword ptr [rsi + 0x48]
020f6f63: jne    0x1820f6fa4
020f6f65: lea    edi, [rax - 1]
020f6f68: xor    edx, edx
020f6f6a: imul   edi, ebx
020f6f6d: mov    rcx, rbp
020f6f70: lea    eax, [rdi + r8]
020f6f74: mov    dword ptr [rsi + 0xc8], eax
020f6f7a: call   0x18214bba0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿʹʿʽʸʵʽˁʴˀʿ
020f6f7f: test   al, al
020f6f81: je     0x1820f6f89
020f6f83: sub    dword ptr [rsi + 0xd0], edi
020f6f89: mov    rdi, qword ptr [rsp + 0x38]
020f6f8e: mov    rbx, qword ptr [rsp + 0x30]
020f6f93: mov    rbp, qword ptr [rsp + 0x40]
020f6f98: mov    rsi, qword ptr [rsp + 0x48]
020f6f9d: add    rsp, 0x20
020f6fa1: pop    r14
020f6fa3: ret    
020f6fa4: cdq    
020f6fa5: sub    eax, edx
020f6fa7: xor    edx, edx
020f6fa9: sar    eax, 1
020f6fab: mov    edi, eax
020f6fad: lea    ecx, [rax - 1]
020f6fb0: imul   ecx, ebx
020f6fb3: add    ecx, r8d
020f6fb6: mov    dword ptr [rsi + 0xc8], ecx
020f6fbc: mov    ecx, eax
020f6fbe: imul   ecx, ebx
020f6fc1: add    dword ptr [rsi + 0xcc], ecx
020f6fc7: mov    rcx, rbp
020f6fca: call   0x18214b600                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʳʵˁʽʾʶʲʴʵʾʹ
020f6fcf: test   al, al
020f6fd1: je     0x1820f6f89
020f6fd3: imul   edi, ebx
020f6fd6: add    dword ptr [rsi + 0xd0], edi
020f6fdc: jmp    0x1820f6f89
020f6fde: call   0x182f60c50
020f6fe3: int3   
020f6fe4: int3   
