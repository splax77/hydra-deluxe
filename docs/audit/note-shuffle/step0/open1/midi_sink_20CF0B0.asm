020cf0b0: mov    qword ptr [rsp + 8], rbx
020cf0b5: mov    qword ptr [rsp + 0x10], rsi
020cf0ba: push   rdi
020cf0bb: sub    rsp, 0x20
020cf0bf: cmp    byte ptr [rip + 0x1e4a256], 0            ; [0x3f1931c] (bss)
020cf0c6: mov    rdi, r8
020cf0c9: mov    rsi, rdx
020cf0cc: mov    rbx, rcx
020cf0cf: jne    0x1820cf0f0
020cf0d1: lea    rcx, [rip + 0x1c00628]                   ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
020cf0d8: call   0x182f609b0
020cf0dd: lea    rcx, [rip + 0x1c29ee4]                   ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cf0e4: call   0x182f609b0
020cf0e9: mov    byte ptr [rip + 0x1e4a22c], 1            ; [0x3f1931c] (bss)
020cf0f0: mov    rcx, qword ptr [rbx + 0x48]
020cf0f4: test   rcx, rcx
020cf0f7: je     0x1820cf19a
020cf0fd: mov    rdx, qword ptr [rip + 0x1c29ec4]         ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cf104: mov    rbx, qword ptr [rbx + 0x70]
020cf108: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cf10d: mov    r9, rax
020cf110: test   rax, rax
020cf113: je     0x1820cf19a
020cf119: mov    qword ptr [rax + 0x10], rsi
020cf11d: mov    qword ptr [rax + 0x18], rdi
020cf121: test   rbx, rbx
020cf124: je     0x1820cf19a
020cf126: mov    rax, qword ptr [rip + 0x1c005d3]         ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
020cf12d: inc    dword ptr [rbx + 0x1c]
020cf130: mov    rcx, qword ptr [rbx + 0x10]
020cf134: test   rcx, rcx
020cf137: je     0x1820cf19a
020cf139: movsxd rdx, dword ptr [rbx + 0x18]
020cf13d: cmp    edx, dword ptr [rcx + 0x18]
020cf140: jb     0x1820cf16b
020cf142: mov    rax, qword ptr [rax + 0x20]
020cf146: mov    rdx, r9
020cf149: mov    rcx, rbx
020cf14c: mov    r8, qword ptr [rax + 0xc0]
020cf153: mov    r8, qword ptr [r8 + 0x70]
020cf157: mov    rbx, qword ptr [rsp + 0x30]
020cf15c: mov    rsi, qword ptr [rsp + 0x38]
020cf161: add    rsp, 0x20
020cf165: pop    rdi
020cf166: jmp    0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020cf16b: lea    eax, [rdx + 1]
020cf16e: mov    dword ptr [rbx + 0x18], eax
020cf171: cmp    edx, dword ptr [rcx + 0x18]
020cf174: jae    0x1820cf1a0
020cf176: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020cf17b: lea    rcx, [rcx + rdx*8]
020cf17f: add    rcx, 0x20
020cf183: mov    rdx, r9
020cf186: mov    rbx, qword ptr [rsp + 0x30]
020cf18b: mov    rsi, qword ptr [rsp + 0x38]
020cf190: add    rsp, 0x20
020cf194: pop    rdi
020cf195: jmp    0x182f5fc00
020cf19a: call   0x182f60c50
020cf19f: int3   
020cf1a0: call   0x182f60c40
020cf1a5: int3   
020cf1a6: int3   
