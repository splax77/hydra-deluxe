020cf1b0: mov    qword ptr [rsp + 8], rbx
020cf1b5: mov    qword ptr [rsp + 0x10], rsi
020cf1ba: push   rdi
020cf1bb: sub    rsp, 0x20
020cf1bf: cmp    byte ptr [rip + 0x1e4a15b], 0            ; [0x3f19321] (bss)
020cf1c6: mov    rdi, r8
020cf1c9: mov    rsi, rdx
020cf1cc: mov    rbx, rcx
020cf1cf: jne    0x1820cf1f0
020cf1d1: lea    rcx, [rip + 0x1c02628]                   ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
020cf1d8: call   0x182f609b0
020cf1dd: lea    rcx, [rip + 0x1c2a82c]                   ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cf1e4: call   0x182f609b0
020cf1e9: mov    byte ptr [rip + 0x1e4a131], 1            ; [0x3f19321] (bss)
020cf1f0: mov    rcx, qword ptr [rbx + 0x40]
020cf1f4: test   rcx, rcx
020cf1f7: je     0x1820cf2a3
020cf1fd: mov    rdx, qword ptr [rip + 0x1c2a80c]         ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cf204: mov    rbx, qword ptr [rbx + 0x78]
020cf208: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cf20d: test   rax, rax
020cf210: je     0x1820cf2a3
020cf216: xor    r9d, r9d
020cf219: mov    r8, rdi
020cf21c: mov    rdx, rsi
020cf21f: mov    rcx, rax
020cf222: call   0x1820d5e60                              ; ʸʵʵʾʿˀʺʽʲʾˁ$$ʹʳʾˁʺʾʹˁʻʹʲ
020cf227: mov    r9, rax
020cf22a: test   rbx, rbx
020cf22d: je     0x1820cf2a3
020cf22f: mov    rax, qword ptr [rip + 0x1c025ca]         ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
020cf236: inc    dword ptr [rbx + 0x1c]
020cf239: mov    rcx, qword ptr [rbx + 0x10]
020cf23d: test   rcx, rcx
020cf240: je     0x1820cf2a3
020cf242: movsxd rdx, dword ptr [rbx + 0x18]
020cf246: cmp    edx, dword ptr [rcx + 0x18]
020cf249: jb     0x1820cf274
020cf24b: mov    rax, qword ptr [rax + 0x20]
020cf24f: mov    rdx, r9
020cf252: mov    rcx, rbx
020cf255: mov    r8, qword ptr [rax + 0xc0]
020cf25c: mov    r8, qword ptr [r8 + 0x70]
020cf260: mov    rbx, qword ptr [rsp + 0x30]
020cf265: mov    rsi, qword ptr [rsp + 0x38]
020cf26a: add    rsp, 0x20
020cf26e: pop    rdi
020cf26f: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020cf274: lea    eax, [rdx + 1]
020cf277: mov    dword ptr [rbx + 0x18], eax
020cf27a: cmp    edx, dword ptr [rcx + 0x18]
020cf27d: jae    0x1820cf2a9
020cf27f: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020cf284: lea    rcx, [rcx + rdx*8]
020cf288: add    rcx, 0x20
020cf28c: mov    rdx, r9
020cf28f: mov    rbx, qword ptr [rsp + 0x30]
020cf294: mov    rsi, qword ptr [rsp + 0x38]
020cf299: add    rsp, 0x20
020cf29d: pop    rdi
020cf29e: jmp    0x182f5fc00
020cf2a3: call   0x182f60c50
020cf2a8: int3   
020cf2a9: call   0x182f60c40
020cf2ae: int3   
020cf2af: int3   
