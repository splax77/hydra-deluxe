002ba4f0: mov    qword ptr [rsp + 0x10], rbx
002ba4f5: mov    qword ptr [rsp + 0x18], rsi
002ba4fa: push   rdi
002ba4fb: sub    rsp, 0x40
002ba4ff: cmp    byte ptr [rip + 0x3c53391], 0            ; [0x3f0d897] (bss)
002ba506: mov    rdi, r8
002ba509: mov    rbx, rdx
002ba50c: mov    rsi, rcx
002ba50f: jne    0x1802ba524
002ba511: lea    rcx, [rip + 0x39e8250]                   ; [0x3ca2768] meta:System.Math_TypeInfo
002ba518: call   0x182f609b0
002ba51d: mov    byte ptr [rip + 0x3c53373], 1            ; [0x3f0d897] (bss)
002ba524: cmp    byte ptr [rbx], 0xf
002ba527: xorps  xmm0, xmm0
002ba52a: movups xmmword ptr [rsp + 0x20], xmm0
002ba52f: movups xmmword ptr [rsp + 0x30], xmm0
002ba534: je     0x1802ba650
002ba53a: cmp    byte ptr [rdi], 0xf
002ba53d: je     0x1802ba647
002ba543: mov    qword ptr [rsp + 0x50], rbp
002ba548: mov    rcx, qword ptr [rip + 0x39e8219]         ; [0x3ca2768] meta:System.Math_TypeInfo
002ba54f: movzx  eax, byte ptr [rbx]
002ba552: mov    byte ptr [rsp + 0x20], al
002ba556: cmp    dword ptr [rcx + 0xe0], 0
002ba55d: jne    0x1802ba564
002ba55f: call   0x182f60cf0
002ba564: movzx  edx, byte ptr [rdi + 0x14]
002ba568: xor    r8d, r8d
002ba56b: movzx  ecx, byte ptr [rbx + 0x14]
002ba56f: call   0x181b51d80                              ; Rewired.Utils.MathTools$$MaxMagnitude
002ba574: cmp    byte ptr [rbx + 8], 0
002ba578: mov    ebp, 1
002ba57d: mov    byte ptr [rsp + 0x34], al
002ba581: mov    eax, ebp
002ba583: jne    0x1802ba589
002ba585: movzx  eax, byte ptr [rdi + 8]
002ba589: mov    rcx, qword ptr [rip + 0x39e81d8]         ; [0x3ca2768] meta:System.Math_TypeInfo
002ba590: test   eax, eax
002ba592: setne  byte ptr [rsp + 0x28]
002ba597: cmp    dword ptr [rcx + 0xe0], 0
002ba59e: jne    0x1802ba5a5
002ba5a0: call   0x182f60cf0
002ba5a5: movzx  edx, byte ptr [rdi + 0xc]
002ba5a9: xor    r8d, r8d
002ba5ac: movzx  ecx, byte ptr [rbx + 0xc]
002ba5b0: call   0x181b51d80                              ; Rewired.Utils.MathTools$$MaxMagnitude
002ba5b5: mov    edx, dword ptr [rdi + 4]
002ba5b8: xor    r8d, r8d
002ba5bb: mov    ecx, dword ptr [rbx + 4]
002ba5be: mov    byte ptr [rsp + 0x2c], al
002ba5c2: call   0x181b51d90                              ; Rewired.Utils.MathTools$$Max
002ba5c7: mov    dword ptr [rsp + 0x24], eax
002ba5cb: mov    eax, dword ptr [rdi + 4]
002ba5ce: mov    rcx, rbx
002ba5d1: cmp    dword ptr [rbx + 4], eax
002ba5d4: cmovle rcx, rdi
002ba5d8: movzx  eax, byte ptr [rcx + 1]
002ba5dc: mov    byte ptr [rsp + 0x21], al
002ba5e0: mov    eax, dword ptr [rdi + 4]
002ba5e3: mov    rcx, rbx
002ba5e6: cmp    dword ptr [rbx + 4], eax
002ba5e9: cmovle rcx, rdi
002ba5ed: mov    eax, dword ptr [rcx + 0x10]
002ba5f0: mov    dword ptr [rsp + 0x30], eax
002ba5f4: mov    eax, dword ptr [rdi + 4]
002ba5f7: mov    rcx, rbx
002ba5fa: cmp    dword ptr [rbx + 4], eax
002ba5fd: cmovle rcx, rdi
002ba601: cmp    byte ptr [rbx + 0x15], 0
002ba605: movzx  eax, word ptr [rcx + 0xa]
002ba609: mov    word ptr [rsp + 0x2a], ax
002ba60e: jne    0x1802ba614
002ba610: movzx  ebp, byte ptr [rdi + 0x15]
002ba614: test   ebp, ebp
002ba616: setne  byte ptr [rsp + 0x35]
002ba61b: mov    eax, dword ptr [rdi + 0x18]
002ba61e: mov    rcx, rbx
002ba621: cmp    dword ptr [rbx + 0x18], eax
002ba624: movups xmm0, xmmword ptr [rsp + 0x20]
002ba629: mov    rbp, qword ptr [rsp + 0x50]
002ba62e: cmovle rcx, rdi
002ba632: mov    eax, dword ptr [rcx + 0x18]
002ba635: mov    dword ptr [rsp + 0x38], eax
002ba639: mov    eax, dword ptr [rbx + 0x1c]
002ba63c: mov    dword ptr [rsp + 0x3c], eax
002ba640: movups xmm1, xmmword ptr [rsp + 0x30]
002ba645: jmp    0x1802ba657
002ba647: movups xmm0, xmmword ptr [rbx]
002ba64a: movups xmm1, xmmword ptr [rbx + 0x10]
002ba64e: jmp    0x1802ba657
002ba650: movups xmm0, xmmword ptr [rdi]
002ba653: movups xmm1, xmmword ptr [rdi + 0x10]
002ba657: mov    rbx, qword ptr [rsp + 0x58]
002ba65c: mov    rax, rsi
002ba65f: movups xmmword ptr [rsi], xmm0
002ba662: movups xmmword ptr [rsi + 0x10], xmm1
002ba666: mov    rsi, qword ptr [rsp + 0x60]
002ba66b: add    rsp, 0x40
002ba66f: pop    rdi
002ba670: ret    
002ba671: int3   
