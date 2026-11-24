0115bb30: mov    qword ptr [rsp + 8], rbx
0115bb35: mov    qword ptr [rsp + 0x10], rsi
0115bb3a: push   rdi
0115bb3b: sub    rsp, 0x30
0115bb3f: cmp    byte ptr [rip + 0x2db5d4e], 0            ; [0x3f11894] (bss)
0115bb46: mov    rsi, r8
0115bb49: mov    rdi, rdx
0115bb4c: mov    rbx, rcx
0115bb4f: jne    0x18115bb64
0115bb51: lea    rcx, [rip + 0x2b60300]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0115bb58: call   0x182f609b0
0115bb5d: mov    byte ptr [rip + 0x2db5d30], 1            ; [0x3f11894] (bss)
0115bb64: test   rdi, rdi
0115bb67: je     0x18115bc72
0115bb6d: mov    rcx, qword ptr [rbx + 0x28]
0115bb71: test   rcx, rcx
0115bb74: je     0x18115bc72
0115bb7a: mov    rax, qword ptr [rcx]
0115bb7d: movzx  edx, word ptr [rdi + 0x80]
0115bb84: mov    r8, qword ptr [rax + 0x210]
0115bb8b: call   qword ptr [rax + 0x208]
0115bb91: mov    rax, qword ptr [rbx + 0x120]
0115bb98: test   rax, rax
0115bb9b: je     0x18115bc72
0115bba1: mov    rax, qword ptr [rax + 0x1c8]
0115bba8: cmp    qword ptr [rdi + 0x48], rax
0115bbac: jle    0x18115bc62
0115bbb2: mov    rax, qword ptr [rbx]
0115bbb5: mov    rcx, rbx
0115bbb8: mov    rdx, qword ptr [rax + 0x510]
0115bbbf: call   qword ptr [rax + 0x508]
0115bbc5: test   rax, rax
0115bbc8: je     0x18115bc72
0115bbce: mov    rcx, qword ptr [rsi + 0x20]
0115bbd2: mov    rdx, qword ptr [rcx + 0xc0]
0115bbd9: mov    rcx, rax
0115bbdc: mov    r8, qword ptr [rdx + 0x80]
0115bbe3: mov    rdx, r8
0115bbe6: call   qword ptr [r8]
0115bbe9: test   al, al
0115bbeb: jne    0x18115bc62
0115bbed: mov    rax, qword ptr [rip + 0x2b60264]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0115bbf4: cmp    dword ptr [rax + 0xe0], 0
0115bbfb: jne    0x18115bc0c
0115bbfd: mov    rcx, rax
0115bc00: call   0x182f60cf0
0115bc05: mov    rax, qword ptr [rip + 0x2b6024c]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0115bc0c: mov    rcx, qword ptr [rbx + 0x88]
0115bc13: test   rcx, rcx
0115bc16: je     0x18115bc72
0115bc18: mov    rdx, qword ptr [rcx + 0x10]
0115bc1c: test   rdx, rdx
0115bc1f: je     0x18115bc72
0115bc21: mov    rax, qword ptr [rax + 0xb8]
0115bc28: mov    rcx, qword ptr [rax + 8]
0115bc2c: test   rcx, rcx
0115bc2f: je     0x18115bc72
0115bc31: movss  xmm0, dword ptr [rip + 0x1f08197]        ; [0x3063dd0] flt=-1.0 i=3212836864
0115bc39: xor    r8d, r8d
0115bc3c: mov    r9d, dword ptr [rbx + 0xa0]
0115bc43: movzx  edx, byte ptr [rdx + 0x10]
0115bc47: mov    qword ptr [rsp + 0x28], 0
0115bc50: movss  dword ptr [rsp + 0x20], xmm0
0115bc56: call   0x18036e7a0                              ; BassAudioManager$$ˁʺʳʴʷʳʴʵʽˁʿ
0115bc5b: mov    byte ptr [rbx + 0xd0], 1
0115bc62: mov    rbx, qword ptr [rsp + 0x40]
0115bc67: mov    rsi, qword ptr [rsp + 0x48]
0115bc6c: add    rsp, 0x30
0115bc70: pop    rdi
0115bc71: ret    
0115bc72: call   0x182f60c50
0115bc77: int3   
0115bc78: int3   
