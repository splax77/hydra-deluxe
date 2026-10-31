0215afe0: mov    qword ptr [rsp + 0x10], rbx
0215afe5: mov    qword ptr [rsp + 0x18], rsi
0215afea: push   rdi
0215afeb: sub    rsp, 0x30
0215afef: mov    rsi, rcx
0215aff2: movabs rbx, 0x60749cf3495cdcbb
0215affc: movabs rdi, 0x14ac2dca74a88953
0215b006: xor    r9d, r9d
0215b009: nop    dword ptr [rax]
0215b010: mov    ecx, r9d
0215b013: mov    edx, 0xf
0215b018: shl    rdx, cl
0215b01b: mov    r8d, 0xf
0215b021: and    rdx, rdi
0215b024: shl    r8, cl
0215b027: and    r8, rbx
0215b02a: mov    eax, 0xf
0215b02f: shl    rax, cl
0215b032: inc    r9d
0215b035: xor    rax, rbx
0215b038: mov    rbx, rdx
0215b03b: or     rbx, rax
0215b03e: mov    eax, 0xf
0215b043: shl    rax, cl
0215b046: xor    rdi, rax
0215b049: or     rdi, r8
0215b04c: mov    eax, edi
0215b04e: xor    eax, ebx
0215b050: and    eax, 0x3f
0215b053: cmp    r9d, eax
0215b056: jle    0x18215b010
0215b058: cmp    byte ptr [rip + 0x1dbe60d], 0            ; [0x3f1966c] (bss)
0215b05f: jne    0x18215b074
0215b061: lea    rcx, [rip + 0x1ba94f0]                   ; [0x3d04558] metamethod:Method$ʶʸʾʵʶʶʼʲʸˀʽ.ʲʾʶˀʵʽʵʸʷʽʼ<ulong>()
0215b068: call   0x182f609b0
0215b06d: mov    byte ptr [rip + 0x1dbe5f8], 1            ; [0x3f1966c] (bss)
0215b074: movups xmm0, xmmword ptr [rsi]
0215b077: mov    r9, qword ptr [rip + 0x1ba94da]          ; [0x3d04558] metamethod:Method$ʶʸʾʵʶʶʼʲʸˀʽ.ʲʾʶˀʵʽʵʸʷʽʼ<ulong>()
0215b07e: lea    r8, [rsp + 0x40]
0215b083: mov    rdx, rbx
0215b086: mov    dword ptr [rsp + 0x40], 0
0215b08e: lea    rcx, [rsp + 0x20]
0215b093: movaps xmmword ptr [rsp + 0x20], xmm0
0215b098: call   0x1805d94b0                              ; ʶʸʾʵʶʶʼʲʸˀʽ$$ʲʾʶˀʵʽʵʸʷʽʼ<ulong>
0215b09d: movups xmm0, xmmword ptr [rsi]
0215b0a0: mov    r9, qword ptr [rip + 0x1ba94b1]          ; [0x3d04558] metamethod:Method$ʶʸʾʵʶʶʼʲʸˀʽ.ʲʾʶˀʵʽʵʸʷʽʼ<ulong>()
0215b0a7: lea    r8, [rsp + 0x40]
0215b0ac: mov    rdx, rdi
0215b0af: lea    rcx, [rsp + 0x20]
0215b0b4: movaps xmmword ptr [rsp + 0x20], xmm0
0215b0b9: call   0x1805d94b0                              ; ʶʸʾʵʶʶʼʲʸˀʽ$$ʲʾʶˀʵʽʵʸʷʽʼ<ulong>
0215b0be: mov    rbx, qword ptr [rsp + 0x48]
0215b0c3: mov    rsi, qword ptr [rsp + 0x50]
0215b0c8: add    rsp, 0x30
0215b0cc: pop    rdi
0215b0cd: ret    
0215b0ce: int3   
