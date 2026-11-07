00337e10: push   rbx
00337e12: push   rsi
00337e13: push   rdi
00337e14: push   r14
00337e16: sub    rsp, 0x48
00337e1a: movaps xmmword ptr [rsp + 0x30], xmm6
00337e1f: mov    r14d, edx
00337e22: mov    rbx, rcx
00337e25: cmp    byte ptr [rip + 0x3bd61a8], 0            ; [0x3f0dfd4] (bss)
00337e2c: jne    0x180337e59
00337e2e: lea    rcx, [rip + 0x39686cb]                   ; [0x3ca0500] meta:MIDIInputManager_TypeInfo
00337e35: call   0x182f609b0
00337e3a: lea    rcx, [rip + 0x396a927]                   ; [0x3ca2768] meta:System.Math_TypeInfo
00337e41: call   0x182f609b0
00337e46: lea    rcx, [rip + 0x397ae1b]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
00337e4d: call   0x182f609b0
00337e52: mov    byte ptr [rip + 0x3bd617b], 1            ; [0x3f0dfd4] (bss)
00337e59: mov    rax, qword ptr [rip + 0x39686a0]         ; [0x3ca0500] meta:MIDIInputManager_TypeInfo
00337e60: mov    rcx, qword ptr [rax + 0xb8]
00337e67: mov    rdi, qword ptr [rcx]
00337e6a: mov    rcx, qword ptr [rip + 0x397adf7]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
00337e71: cmp    dword ptr [rcx + 0xe0], 0
00337e78: jne    0x180337e7f
00337e7a: call   0x182f60cf0
00337e7f: xor    r8d, r8d
00337e82: xor    edx, edx
00337e84: mov    rcx, rdi
00337e87: call   0x18287fed0                              ; UnityEngine.Object$$op_Inequality
00337e8c: test   al, al
00337e8e: jne    0x180337e95
00337e90: xor    dil, dil
00337e93: jmp    0x180337ec1
00337e95: mov    rax, qword ptr [rip + 0x3968664]         ; [0x3ca0500] meta:MIDIInputManager_TypeInfo
00337e9c: mov    rcx, qword ptr [rax + 0xb8]
00337ea3: mov    rcx, qword ptr [rcx]
00337ea6: test   rcx, rcx
00337ea9: je     0x180338017
00337eaf: xor    r8d, r8d
00337eb2: mov    rdx, qword ptr [rbx + 0x88]
00337eb9: call   0x18021f790                              ; MIDIInputManager$$ʺʷʵʾʺʻʷʺʿʽʽ
00337ebe: movzx  edi, al
00337ec1: mov    rcx, qword ptr [rbx + 0x88]
00337ec8: test   rcx, rcx
00337ecb: je     0x180338012
00337ed1: mov    rdx, qword ptr [rcx + 0x18]
00337ed5: test   rdx, rdx
00337ed8: je     0x18033800d
00337ede: mov    rcx, qword ptr [rdx + 0x50]
00337ee2: test   rcx, rcx
00337ee5: je     0x180338008
00337eeb: xor    r9d, r9d
00337eee: mov    edx, 0x14
00337ef3: mov    r8d, 6
00337ef9: call   0x181f2dcf0                              ; Rewired.Player.ControllerHelper$$ContainsController
00337efe: movzx  esi, al
00337f01: mov    rcx, qword ptr [rbx + 0x88]
00337f08: test   rcx, rcx
00337f0b: je     0x180338003
00337f11: mov    rcx, qword ptr [rcx + 0x18]
00337f15: test   rcx, rcx
00337f18: je     0x180337ffe
00337f1e: xor    r8d, r8d
00337f21: mov    edx, r14d
00337f24: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
00337f29: test   al, al
00337f2b: je     0x180337fde
00337f31: test   dil, dil
00337f34: jne    0x180337f3f
00337f36: test   sil, sil
00337f39: je     0x180337fde
00337f3f: mov    rax, qword ptr [rbx + 0x88]
00337f46: test   rax, rax
00337f49: je     0x180337ff9
00337f4f: mov    rcx, qword ptr [rax + 0x18]
00337f53: test   rcx, rcx
00337f56: je     0x180337ff3
00337f5c: xor    r8d, r8d
00337f5f: mov    edx, r14d
00337f62: call   0x181f389b0                              ; Rewired.Player$$GetAxis
00337f67: movaps xmm6, xmm0
00337f6a: mov    rcx, qword ptr [rip + 0x396a7f7]         ; [0x3ca2768] meta:System.Math_TypeInfo
00337f71: cmp    dword ptr [rcx + 0xe0], 0
00337f78: jne    0x180337f7f
00337f7a: call   0x182f60cf0
00337f7f: subss  xmm6, dword ptr [rip + 0x2d2c42d]        ; [0x30643b4] flt=0.5 i=1056964608
00337f87: addss  xmm6, xmm6
00337f8b: cmp    byte ptr [rip + 0x3bd6041], 0            ; [0x3f0dfd3] (bss)
00337f92: jne    0x180337fa7
00337f94: lea    rcx, [rip + 0x396a7cd]                   ; [0x3ca2768] meta:System.Math_TypeInfo
00337f9b: call   0x182f609b0
00337fa0: mov    byte ptr [rip + 0x3bd602c], 1            ; [0x3f0dfd3] (bss)
00337fa7: mov    rcx, qword ptr [rip + 0x396a7ba]         ; [0x3ca2768] meta:System.Math_TypeInfo
00337fae: cmp    dword ptr [rcx + 0xe0], 0
00337fb5: jne    0x180337fbc
00337fb7: call   0x182f60cf0
00337fbc: mulss  xmm6, dword ptr [rip + 0x2d2ceb4]        ; [0x3064e78] flt=127.0 i=1123942400
00337fc4: xorps  xmm0, xmm0
00337fc7: cvtss2sd xmm0, xmm6
00337fcb: xor    r9d, r9d
00337fce: xor    r8d, r8d
00337fd1: xor    edx, edx
00337fd3: call   0x181b520a0                              ; System.Math$$Round
00337fd8: cvttsd2si eax, xmm0
00337fdc: jmp    0x180337fe4
00337fde: mov    al, 0xff
00337fe0: jmp    0x180337fe4
00337fe2: mov    al, 0xff
00337fe4: movaps xmm6, xmmword ptr [rsp + 0x30]
00337fe9: add    rsp, 0x48
00337fed: pop    r14
00337fef: pop    rdi
00337ff0: pop    rsi
00337ff1: pop    rbx
00337ff2: ret    
00337ff3: call   0x182f60c50
00337ff8: nop    
00337ff9: call   0x182f60c50
00337ffe: call   0x182f60c50
00338003: call   0x182f60c50
00338008: call   0x182f60c50
0033800d: call   0x182f60c50
00338012: call   0x182f60c50
00338017: call   0x182f60c50
0033801c: int3   
