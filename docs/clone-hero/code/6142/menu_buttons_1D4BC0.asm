001d4bc0: mov    qword ptr [rsp + 0x18], rbx
001d4bc5: push   rdi
001d4bc6: sub    rsp, 0x20
001d4bca: cmp    byte ptr [rip + 0x3d37f76], 0            ; [0x3f0cb47] (bss)
001d4bd1: mov    rdi, rdx
001d4bd4: mov    rbx, rcx
001d4bd7: jne    0x1801d4bec
001d4bd9: lea    rcx, [rip + 0x3af4278]                   ; [0x3cc8e58] meta:ColorProfileEditor_TypeInfo
001d4be0: call   0x182f609b0
001d4be5: mov    byte ptr [rip + 0x3d37f5b], 1            ; [0x3f0cb47] (bss)
001d4bec: mov    qword ptr [rsp + 0x30], rbp
001d4bf1: mov    qword ptr [rsp + 0x38], rsi
001d4bf6: test   rdi, rdi
001d4bf9: je     0x1801d5247
001d4bff: xor    edx, edx
001d4c01: mov    rcx, rdi
001d4c04: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
001d4c09: test   al, al
001d4c0b: je     0x1801d5232
001d4c11: cmp    byte ptr [rdi + 0x2c], 0
001d4c15: je     0x1801d4c24
001d4c17: cmp    byte ptr [rbx + 0x9c], 0
001d4c1e: je     0x1801d5232
001d4c24: cmp    byte ptr [rbx + 0x9b], 0
001d4c2b: jne    0x1801d5232
001d4c31: cmp    qword ptr [rdi + 0x10], 0
001d4c36: je     0x1801d4c49
001d4c38: mov    rax, qword ptr [rdi + 0x10]
001d4c3c: cmp    byte ptr [rax + 0xb1], 0
001d4c43: jne    0x1801d5232
001d4c49: mov    rcx, qword ptr [rip + 0x3af4208]         ; [0x3cc8e58] meta:ColorProfileEditor_TypeInfo
001d4c50: cmp    dword ptr [rcx + 0xe0], 0
001d4c57: jne    0x1801d4c5e
001d4c59: call   0x182f60cf0
001d4c5e: xor    ecx, ecx
001d4c60: call   0x1800a4590                              ; ColorProfileEditor$$ʿʳʶʵʻʷʾʿˁʺʿ
001d4c65: test   al, al
001d4c67: jne    0x1801d5232
001d4c6d: xor    ecx, ecx
001d4c6f: call   0x180101220                              ; SongPathsManager$$ʿʳʶʵʻʷʾʿˁʺʿ
001d4c74: test   al, al
001d4c76: jne    0x1801d5232
001d4c7c: xor    ecx, ecx
001d4c7e: call   0x180192120                              ; ContentPathsManager$$ʿʳʶʵʻʷʾʿˁʺʿ
001d4c83: test   al, al
001d4c85: jne    0x1801d5232
001d4c8b: mov    rcx, qword ptr [rdi + 0x18]
001d4c8f: test   rcx, rcx
001d4c92: je     0x1801d5247
001d4c98: xor    r8d, r8d
001d4c9b: mov    edx, 0x20
001d4ca0: call   0x181f39e30                              ; Rewired.Player$$GetButtonRepeating
001d4ca5: test   al, al
001d4ca7: jne    0x1801d4cdc
001d4ca9: mov    rcx, qword ptr [rdi + 0x18]
001d4cad: test   rcx, rcx
001d4cb0: je     0x1801d5247
001d4cb6: xor    r8d, r8d
001d4cb9: mov    edx, 0x1f
001d4cbe: call   0x181f39e30                              ; Rewired.Player$$GetButtonRepeating
001d4cc3: test   al, al
001d4cc5: je     0x1801d4cef
001d4cc7: mov    rax, qword ptr [rbx]
001d4cca: mov    rcx, rbx
001d4ccd: mov    rdx, qword ptr [rax + 0x310]
001d4cd4: call   qword ptr [rax + 0x308]
001d4cda: jmp    0x1801d4cef
001d4cdc: mov    rax, qword ptr [rbx]
001d4cdf: mov    rcx, rbx
001d4ce2: mov    rdx, qword ptr [rax + 0x260]
001d4ce9: call   qword ptr [rax + 0x258]
001d4cef: mov    rcx, qword ptr [rdi + 0x18]
001d4cf3: test   rcx, rcx
001d4cf6: je     0x1801d5247
001d4cfc: xor    r8d, r8d
001d4cff: xor    edx, edx
001d4d01: call   0x181f3b730                              ; Rewired.Player$$GetButton
001d4d06: movzx  ebp, al
001d4d09: cmp    qword ptr [rdi + 0x10], 0
001d4d0e: mov    sil, 1
001d4d11: je     0x1801d4d21
001d4d13: mov    rcx, qword ptr [rdi + 0x10]
001d4d17: xor    edx, edx
001d4d19: call   0x1800d7f20                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʸʴʳʺʿʶʷʾʷʿʿ
001d4d1e: movzx  esi, al
001d4d21: mov    rcx, qword ptr [rdi + 0x18]
001d4d25: test   rcx, rcx
001d4d28: je     0x1801d5247
001d4d2e: xor    r8d, r8d
001d4d31: xor    edx, edx
001d4d33: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4d38: test   al, al
001d4d3a: jne    0x1801d510c
001d4d40: mov    rcx, qword ptr [rdi + 0x18]
001d4d44: test   rcx, rcx
001d4d47: je     0x1801d5247
001d4d4d: movss  xmm2, dword ptr [rbx + 0xd8]
001d4d55: xor    r9d, r9d
001d4d58: xor    edx, edx
001d4d5a: call   0x181f3ac00                              ; Rewired.Player$$GetButtonTimedPressDown
001d4d5f: test   al, al
001d4d61: jne    0x1801d50f7
001d4d67: mov    rcx, qword ptr [rdi + 0x18]
001d4d6b: test   rcx, rcx
001d4d6e: je     0x1801d5247
001d4d74: movss  xmm2, dword ptr [rbx + 0xd8]
001d4d7c: xor    r9d, r9d
001d4d7f: mov    edx, 1
001d4d84: call   0x181f3ac00                              ; Rewired.Player$$GetButtonTimedPressDown
001d4d89: test   al, al
001d4d8b: jne    0x1801d50e2
001d4d91: mov    rcx, qword ptr [rdi + 0x18]
001d4d95: test   rcx, rcx
001d4d98: je     0x1801d5247
001d4d9e: xor    r8d, r8d
001d4da1: xor    edx, edx
001d4da3: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d4da8: test   al, al
001d4daa: jne    0x1801d50cd
001d4db0: mov    rcx, qword ptr [rdi + 0x18]
001d4db4: test   rcx, rcx
001d4db7: je     0x1801d5247
001d4dbd: xor    r8d, r8d
001d4dc0: mov    edx, 1
001d4dc5: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4dca: test   al, al
001d4dcc: jne    0x1801d50b8
001d4dd2: mov    rcx, qword ptr [rdi + 0x18]
001d4dd6: test   rcx, rcx
001d4dd9: je     0x1801d5247
001d4ddf: xor    r8d, r8d
001d4de2: mov    edx, 2
001d4de7: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4dec: test   al, al
001d4dee: jne    0x1801d508f
001d4df4: mov    rcx, qword ptr [rdi + 0x18]
001d4df8: test   rcx, rcx
001d4dfb: je     0x1801d5247
001d4e01: xor    r8d, r8d
001d4e04: mov    edx, 2
001d4e09: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d4e0e: test   al, al
001d4e10: jne    0x1801d5060
001d4e16: test   sil, sil
001d4e19: jne    0x1801d4e59
001d4e1b: mov    rcx, qword ptr [rdi + 0x18]
001d4e1f: test   rcx, rcx
001d4e22: je     0x1801d5247
001d4e28: movss  xmm2, dword ptr [rbx + 0xd8]
001d4e30: xor    r9d, r9d
001d4e33: mov    edx, 2
001d4e38: call   0x181f3ac00                              ; Rewired.Player$$GetButtonTimedPressDown
001d4e3d: test   al, al
001d4e3f: je     0x1801d4e59
001d4e41: mov    rax, qword ptr [rbx]
001d4e44: mov    rcx, rbx
001d4e47: mov    rdx, qword ptr [rax + 0x200]
001d4e4e: call   qword ptr [rax + 0x1f8]
001d4e54: jmp    0x1801d511f
001d4e59: mov    rcx, qword ptr [rdi + 0x18]
001d4e5d: test   rcx, rcx
001d4e60: je     0x1801d5247
001d4e66: xor    r8d, r8d
001d4e69: mov    edx, 3
001d4e6e: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4e73: test   al, al
001d4e75: jne    0x1801d5031
001d4e7b: mov    rcx, qword ptr [rdi + 0x18]
001d4e7f: test   rcx, rcx
001d4e82: je     0x1801d5247
001d4e88: xor    r8d, r8d
001d4e8b: mov    edx, 3
001d4e90: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d4e95: test   al, al
001d4e97: jne    0x1801d5002
001d4e9d: test   sil, sil
001d4ea0: jne    0x1801d4ee0
001d4ea2: mov    rcx, qword ptr [rdi + 0x18]
001d4ea6: test   rcx, rcx
001d4ea9: je     0x1801d5247
001d4eaf: movss  xmm2, dword ptr [rbx + 0xd8]
001d4eb7: xor    r9d, r9d
001d4eba: mov    edx, 3
001d4ebf: call   0x181f3ac00                              ; Rewired.Player$$GetButtonTimedPressDown
001d4ec4: test   al, al
001d4ec6: je     0x1801d4ee0
001d4ec8: mov    rax, qword ptr [rbx]
001d4ecb: mov    rcx, rbx
001d4ece: mov    rdx, qword ptr [rax + 0x4b0]
001d4ed5: call   qword ptr [rax + 0x4a8]
001d4edb: jmp    0x1801d511f
001d4ee0: mov    rcx, qword ptr [rdi + 0x18]
001d4ee4: test   rcx, rcx
001d4ee7: je     0x1801d5247
001d4eed: xor    r8d, r8d
001d4ef0: mov    edx, 4
001d4ef5: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4efa: test   al, al
001d4efc: jne    0x1801d4fea
001d4f02: mov    rcx, qword ptr [rdi + 0x18]
001d4f06: test   rcx, rcx
001d4f09: je     0x1801d5247
001d4f0f: xor    r8d, r8d
001d4f12: mov    edx, 4
001d4f17: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d4f1c: test   al, al
001d4f1e: jne    0x1801d4fd2
001d4f24: mov    rcx, qword ptr [rdi + 0x18]
001d4f28: test   rcx, rcx
001d4f2b: je     0x1801d5247
001d4f31: movss  xmm2, dword ptr [rbx + 0xd8]
001d4f39: xor    r9d, r9d
001d4f3c: mov    edx, 4
001d4f41: call   0x181f3ac00                              ; Rewired.Player$$GetButtonTimedPressDown
001d4f46: test   al, al
001d4f48: jne    0x1801d4fba
001d4f4a: mov    rcx, qword ptr [rdi + 0x18]
001d4f4e: test   rcx, rcx
001d4f51: je     0x1801d5247
001d4f57: xor    r8d, r8d
001d4f5a: mov    edx, 6
001d4f5f: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4f64: test   al, al
001d4f66: jne    0x1801d4fa2
001d4f68: mov    rcx, qword ptr [rdi + 0x18]
001d4f6c: test   rcx, rcx
001d4f6f: je     0x1801d5247
001d4f75: xor    r8d, r8d
001d4f78: mov    edx, 0x10
001d4f7d: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
001d4f82: test   al, al
001d4f84: je     0x1801d511f
001d4f8a: mov    rax, qword ptr [rbx]
001d4f8d: mov    rcx, rbx
001d4f90: mov    rdx, qword ptr [rax + 0x3b0]
001d4f97: call   qword ptr [rax + 0x3a8]
001d4f9d: jmp    0x1801d511f
001d4fa2: mov    rax, qword ptr [rbx]
001d4fa5: mov    rcx, rbx
001d4fa8: mov    rdx, qword ptr [rax + 0x3a0]
001d4faf: call   qword ptr [rax + 0x398]
001d4fb5: jmp    0x1801d511f
001d4fba: mov    rax, qword ptr [rbx]
001d4fbd: mov    rcx, rbx
001d4fc0: mov    rdx, qword ptr [rax + 0x2d0]
001d4fc7: call   qword ptr [rax + 0x2c8]
001d4fcd: jmp    0x1801d511f
001d4fd2: mov    rax, qword ptr [rbx]
001d4fd5: mov    rcx, rbx
001d4fd8: mov    rdx, qword ptr [rax + 0x2a0]
001d4fdf: call   qword ptr [rax + 0x298]
001d4fe5: jmp    0x1801d511f
001d4fea: mov    rax, qword ptr [rbx]
001d4fed: mov    rcx, rbx
001d4ff0: mov    rdx, qword ptr [rax + 0x270]
001d4ff7: call   qword ptr [rax + 0x268]
001d4ffd: jmp    0x1801d511f
001d5002: mov    rax, qword ptr [rbx]
001d5005: mov    rcx, rbx
001d5008: test   sil, sil
001d500b: jne    0x1801d501f
001d500d: mov    rdx, qword ptr [rax + 0x370]
001d5014: call   qword ptr [rax + 0x368]
001d501a: jmp    0x1801d511f
001d501f: mov    rdx, qword ptr [rax + 0x180]
001d5026: call   qword ptr [rax + 0x178]
001d502c: jmp    0x1801d511f
001d5031: mov    rax, qword ptr [rbx]
001d5034: mov    rcx, rbx
001d5037: test   sil, sil
001d503a: jne    0x1801d504e
001d503c: mov    rdx, qword ptr [rax + 0x380]
001d5043: call   qword ptr [rax + 0x378]
001d5049: jmp    0x1801d511f
001d504e: mov    rdx, qword ptr [rax + 0x1e0]
001d5055: call   qword ptr [rax + 0x1d8]
001d505b: jmp    0x1801d511f
001d5060: mov    rax, qword ptr [rbx]
001d5063: mov    rcx, rbx
001d5066: test   sil, sil
001d5069: jne    0x1801d507d
001d506b: mov    rdx, qword ptr [rax + 0x410]
001d5072: call   qword ptr [rax + 0x408]
001d5078: jmp    0x1801d511f
001d507d: mov    rdx, qword ptr [rax + 0x220]
001d5084: call   qword ptr [rax + 0x218]
001d508a: jmp    0x1801d511f
001d508f: mov    rax, qword ptr [rbx]
001d5092: mov    rcx, rbx
001d5095: test   sil, sil
001d5098: jne    0x1801d50a9
001d509a: mov    rdx, qword ptr [rax + 0x280]
001d50a1: call   qword ptr [rax + 0x278]
001d50a7: jmp    0x1801d511f
001d50a9: mov    rdx, qword ptr [rax + 0x290]
001d50b0: call   qword ptr [rax + 0x288]
001d50b6: jmp    0x1801d511f
001d50b8: mov    rax, qword ptr [rbx]
001d50bb: mov    rcx, rbx
001d50be: mov    rdx, qword ptr [rax + 0x470]
001d50c5: call   qword ptr [rax + 0x468]
001d50cb: jmp    0x1801d511f
001d50cd: mov    rax, qword ptr [rbx]
001d50d0: mov    rcx, rbx
001d50d3: mov    rdx, qword ptr [rax + 0x2c0]
001d50da: call   qword ptr [rax + 0x2b8]
001d50e0: jmp    0x1801d511f
001d50e2: mov    rax, qword ptr [rbx]
001d50e5: mov    rcx, rbx
001d50e8: mov    rdx, qword ptr [rax + 0x2e0]
001d50ef: call   qword ptr [rax + 0x2d8]
001d50f5: jmp    0x1801d511f
001d50f7: mov    rax, qword ptr [rbx]
001d50fa: mov    rcx, rbx
001d50fd: mov    rdx, qword ptr [rax + 0x4d0]
001d5104: call   qword ptr [rax + 0x4c8]
001d510a: jmp    0x1801d511f
001d510c: mov    rax, qword ptr [rbx]
001d510f: mov    rcx, rbx
001d5112: mov    rdx, qword ptr [rax + 0x330]
001d5119: call   qword ptr [rax + 0x328]
001d511f: test   bpl, bpl
001d5122: jne    0x1801d5232
001d5128: xor    edx, edx
001d512a: mov    rcx, rdi
001d512d: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
001d5132: test   al, al
001d5134: je     0x1801d5232
001d513a: cmp    qword ptr [rdi + 0x10], 0
001d513f: je     0x1801d5172
001d5141: mov    rcx, qword ptr [rdi + 0x10]
001d5145: xor    edx, edx
001d5147: mov    rcx, qword ptr [rcx + 0x38]
001d514b: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
001d5150: test   al, al
001d5152: je     0x1801d5172
001d5154: mov    rax, qword ptr [rdi + 0x10]
001d5158: test   rax, rax
001d515b: je     0x1801d5247
001d5161: xor    ecx, ecx
001d5163: cmp    byte ptr [rax + 0x10], 6
001d5167: je     0x1801d5174
001d5169: cmp    byte ptr [rax + 0x10], 9
001d516d: setne  cl
001d5170: jmp    0x1801d5174
001d5172: xor    ecx, ecx
001d5174: mov    eax, 5
001d5179: test   ecx, ecx
001d517b: mov    ebp, eax
001d517d: mov    esi, 0xe
001d5182: cmove  ebp, esi
001d5185: mov    rdx, qword ptr [rdi + 0x18]
001d5189: cmove  esi, eax
001d518c: xor    r9d, r9d
001d518f: mov    r8d, ebp
001d5192: mov    rcx, rbx
001d5195: call   0x1801d6bc0                              ; BaseMenu$$ˁʴʾʲʾʴʺʵʵˀˁ
001d519a: test   al, al
001d519c: je     0x1801d51b1
001d519e: mov    rax, qword ptr [rbx]
001d51a1: mov    rcx, rbx
001d51a4: mov    rdx, qword ptr [rax + 0x1e0]
001d51ab: call   qword ptr [rax + 0x1d8]
001d51b1: mov    rcx, qword ptr [rdi + 0x18]
001d51b5: test   rcx, rcx
001d51b8: je     0x1801d5247
001d51be: xor    r8d, r8d
001d51c1: mov    edx, ebp
001d51c3: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d51c8: test   al, al
001d51ca: je     0x1801d51df
001d51cc: mov    rax, qword ptr [rbx]
001d51cf: mov    rcx, rbx
001d51d2: mov    rdx, qword ptr [rax + 0x180]
001d51d9: call   qword ptr [rax + 0x178]
001d51df: mov    rdx, qword ptr [rdi + 0x18]
001d51e3: xor    r9d, r9d
001d51e6: mov    r8d, esi
001d51e9: mov    rcx, rbx
001d51ec: call   0x1801d6bc0                              ; BaseMenu$$ˁʴʾʲʾʴʺʵʵˀˁ
001d51f1: test   al, al
001d51f3: je     0x1801d5208
001d51f5: mov    rax, qword ptr [rbx]
001d51f8: mov    rcx, rbx
001d51fb: mov    rdx, qword ptr [rax + 0x290]
001d5202: call   qword ptr [rax + 0x288]
001d5208: mov    rcx, qword ptr [rdi + 0x18]
001d520c: test   rcx, rcx
001d520f: je     0x1801d5247
001d5211: xor    r8d, r8d
001d5214: mov    edx, esi
001d5216: call   0x181f3b4c0                              ; Rewired.Player$$GetButtonUp
001d521b: test   al, al
001d521d: je     0x1801d5232
001d521f: mov    rax, qword ptr [rbx]
001d5222: mov    rcx, rbx
001d5225: mov    rdx, qword ptr [rax + 0x220]
001d522c: call   qword ptr [rax + 0x218]
001d5232: mov    rsi, qword ptr [rsp + 0x38]
001d5237: mov    rbp, qword ptr [rsp + 0x30]
001d523c: mov    rbx, qword ptr [rsp + 0x40]
001d5241: add    rsp, 0x20
001d5245: pop    rdi
001d5246: ret    
001d5247: call   0x182f60c50
001d524c: int3   
001d524d: int3   
