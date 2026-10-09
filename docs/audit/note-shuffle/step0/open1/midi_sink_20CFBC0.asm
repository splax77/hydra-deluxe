020cfbc0: push   rbx
020cfbc2: push   rbp
020cfbc3: push   rsi
020cfbc4: push   rdi
020cfbc5: push   r14
020cfbc7: sub    rsp, 0x80
020cfbce: cmp    byte ptr [rip + 0x1e4974f], 0            ; [0x3f19324] (bss)
020cfbd5: mov    rsi, r8
020cfbd8: movzx  ebp, dl
020cfbdb: mov    rdi, rcx
020cfbde: jne    0x1820cfbf3
020cfbe0: lea    rcx, [rip + 0x1bd03b1]                   ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020cfbe7: call   0x182f609b0
020cfbec: mov    byte ptr [rip + 0x1e49731], 1            ; [0x3f19324] (bss)
020cfbf3: xor    r14d, r14d
020cfbf6: mov    ebx, r14d
020cfbf9: nop    dword ptr [rax]
020cfc00: mov    rax, qword ptr [rip + 0x1bd0391]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020cfc07: cmp    dword ptr [rax + 0xe0], r14d
020cfc0e: jne    0x1820cfc1f
020cfc10: mov    rcx, rax
020cfc13: call   0x182f60cf0
020cfc18: mov    rax, qword ptr [rip + 0x1bd0379]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020cfc1f: mov    rcx, qword ptr [rax + 0xb8]
020cfc26: cmp    ebx, dword ptr [rcx]
020cfc28: jge    0x1820cfcbe
020cfc2e: mov    eax, dword ptr [rsi + 0x14]
020cfc31: lea    rcx, [rsp + 0x40]
020cfc36: movzx  r9d, byte ptr [rsi + 0x18]
020cfc3b: xorps  xmm0, xmm0
020cfc3e: mov    r8, qword ptr [rsi + 8]
020cfc42: mov    rdx, qword ptr [rsi]
020cfc45: mov    qword ptr [rsp + 0x30], r14
020cfc4a: mov    dword ptr [rsp + 0x28], eax
020cfc4e: movups xmmword ptr [rsp + 0x40], xmm0
020cfc53: mov    byte ptr [rsp + 0x20], bl
020cfc57: movups xmmword ptr [rsp + 0x50], xmm0
020cfc5c: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
020cfc61: mov    rcx, qword ptr [rip + 0x1bd0330]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
020cfc68: movups xmm0, xmmword ptr [rsp + 0x40]
020cfc6d: movups xmm1, xmmword ptr [rsp + 0x50]
020cfc72: movups xmmword ptr [rsp + 0x60], xmm0
020cfc77: movups xmmword ptr [rsp + 0x70], xmm1
020cfc7c: cmp    dword ptr [rcx + 0xe0], r14d
020cfc83: jne    0x1820cfc8a
020cfc85: call   0x182f60cf0
020cfc8a: test   rdi, rdi
020cfc8d: je     0x1820cfccc
020cfc8f: movzx  r8d, bl
020cfc93: xor    r9d, r9d
020cfc96: movzx  edx, bpl
020cfc9a: mov    rcx, rdi
020cfc9d: call   0x1820cfd40
020cfca2: test   rax, rax
020cfca5: je     0x1820cfccc
020cfca7: xor    r8d, r8d
020cfcaa: lea    rdx, [rsp + 0x60]
020cfcaf: mov    rcx, rax
020cfcb2: call   0x1820ceb90
020cfcb7: inc    ebx
020cfcb9: jmp    0x1820cfc00
020cfcbe: add    rsp, 0x80
020cfcc5: pop    r14
020cfcc7: pop    rdi
020cfcc8: pop    rsi
020cfcc9: pop    rbp
020cfcca: pop    rbx
020cfccb: ret    
020cfccc: call   0x182f60c50
020cfcd1: int3   
020cfcd2: int3   
