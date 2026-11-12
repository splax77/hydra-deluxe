00395c10: mov    qword ptr [rsp + 0x20], rsi
00395c15: push   r14
00395c17: sub    rsp, 0x20
00395c1b: cmp    byte ptr [rip + 0x3b78753], 0            ; [0x3f0e375] (bss)
00395c22: mov    rsi, rdx
00395c25: mov    r14, rcx
00395c28: jne    0x180395cdd
00395c2e: lea    rcx, [rip + 0x39680e3]                   ; [0x3cfdd18] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.Add()
00395c35: call   0x182f609b0
00395c3a: lea    rcx, [rip + 0x396818f]                   ; [0x3cfddd0] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.get_Count()
00395c41: call   0x182f609b0
00395c46: lea    rcx, [rip + 0x395526b]                   ; [0x3ceaeb8] str:'-i'
00395c4d: call   0x182f609b0
00395c52: lea    rcx, [rip + 0x3951f77]                   ; [0x3ce7bd0] str:'--instrumentNames'
00395c59: call   0x182f609b0
00395c5e: lea    rcx, [rip + 0x395262b]                   ; [0x3ce8290] str:'--profile'
00395c65: call   0x182f609b0
00395c6a: lea    rcx, [rip + 0x3952a9f]                   ; [0x3ce8710] str:'--song'
00395c71: call   0x182f609b0
00395c76: lea    rcx, [rip + 0x39538bb]                   ; [0x3ce9538] str:'--versus'
00395c7d: call   0x182f609b0
00395c82: lea    rcx, [rip + 0x3952547]                   ; [0x3ce81d0] str:'--player'
00395c89: call   0x182f609b0
00395c8e: lea    rcx, [rip + 0x3956423]                   ; [0x3cec0b8] str:'-v'
00395c95: call   0x182f609b0
00395c9a: lea    rcx, [rip + 0x39555d7]                   ; [0x3ceb278] str:'-r'
00395ca1: call   0x182f609b0
00395ca6: lea    rcx, [rip + 0x395550b]                   ; [0x3ceb1b8] str:'-p'
00395cad: call   0x182f609b0
00395cb2: lea    rcx, [rip + 0x3952697]                   ; [0x3ce8350] str:'--replay'
00395cb9: call   0x182f609b0
00395cbe: lea    rcx, [rip + 0x3955673]                   ; [0x3ceb338] str:'-s'
00395cc5: call   0x182f609b0
00395cca: lea    rcx, [rip + 0x395147f]                   ; [0x3ce7150] str:'--analysis'
00395cd1: call   0x182f609b0
00395cd6: mov    byte ptr [rip + 0x3b78698], 1            ; [0x3f0e375] (bss)
00395cdd: mov    qword ptr [rsp + 0x30], rbx
00395ce2: mov    qword ptr [rsp + 0x38], rbp
00395ce7: mov    qword ptr [rsp + 0x40], rdi
00395cec: test   rsi, rsi
00395cef: je     0x180396064
00395cf5: xor    bpl, bpl
00395cf8: xor    edi, edi
00395cfa: xor    eax, eax
00395cfc: nop    dword ptr [rax]
00395d00: cmp    eax, dword ptr [rsi + 0x18]
00395d03: jge    0x180396045
00395d09: cmp    edi, dword ptr [rsi + 0x18]
00395d0c: jae    0x18039606a
00395d12: movsxd rax, edi
00395d15: xor    edx, edx
00395d17: mov    rbx, qword ptr [rsi + rax*8 + 0x20]
00395d1c: mov    rcx, rbx
00395d1f: call   0x180103fe0                              ; <PrivateImplementationDetails>$$ComputeStringHash
00395d24: cmp    eax, 0x59cd76ca
00395d29: jbe    0x180395e6e
00395d2f: cmp    eax, 0x7b1f9520
00395d34: jbe    0x180395dae
00395d36: cmp    eax, 0xa267116d
00395d3b: jne    0x180395d5d
00395d3d: mov    rdx, qword ptr [rip + 0x39537f4]         ; [0x3ce9538] str:'--versus'
00395d44: xor    r8d, r8d
00395d47: mov    rcx, rbx
00395d4a: call   0x181981570                              ; System.String$$op_Equality
00395d4f: test   al, al
00395d51: jne    0x180395f28
00395d57: inc    edi
00395d59: mov    eax, edi
00395d5b: jmp    0x180395d00
00395d5d: cmp    eax, 0xae127c7e
00395d62: jne    0x180395d80
00395d64: mov    rdx, qword ptr [rip + 0x3951e65]         ; [0x3ce7bd0] str:'--instrumentNames'
00395d6b: xor    r8d, r8d
00395d6e: mov    rcx, rbx
00395d71: call   0x181981570                              ; System.String$$op_Equality
00395d76: test   al, al
00395d78: jne    0x180395dcf
00395d7a: inc    edi
00395d7c: mov    eax, edi
00395d7e: jmp    0x180395d00
00395d80: cmp    eax, 0xff28926e
00395d85: jne    0x18039603c
00395d8b: mov    rdx, qword ptr [rip + 0x395297e]         ; [0x3ce8710] str:'--song'
00395d92: xor    r8d, r8d
00395d95: mov    rcx, rbx
00395d98: call   0x181981570                              ; System.String$$op_Equality
00395d9d: test   al, al
00395d9f: jne    0x180395ec4
00395da5: inc    edi
00395da7: mov    eax, edi
00395da9: jmp    0x180395d00
00395dae: cmp    eax, 0x5ccd7b83
00395db3: jne    0x180395de0
00395db5: mov    rdx, qword ptr [rip + 0x39550fc]         ; [0x3ceaeb8] str:'-i'
00395dbc: xor    r8d, r8d
00395dbf: mov    rcx, rbx
00395dc2: call   0x181981570                              ; System.String$$op_Equality
00395dc7: test   al, al
00395dc9: je     0x18039603c
00395dcf: mov    byte ptr [r14 + 0x48], 1
00395dd4: inc    edi
00395dd6: mov    bpl, 1
00395dd9: mov    eax, edi
00395ddb: jmp    0x180395d00
00395de0: cmp    eax, 0x760a3fee
00395de5: jne    0x180395e0a
00395de7: mov    rdx, qword ptr [rip + 0x39523e2]         ; [0x3ce81d0] str:'--player'
00395dee: xor    r8d, r8d
00395df1: mov    rcx, rbx
00395df4: call   0x181981570                              ; System.String$$op_Equality
00395df9: test   al, al
00395dfb: jne    0x180395f8c
00395e01: inc    edi
00395e03: mov    eax, edi
00395e05: jmp    0x180395d00
00395e0a: cmp    eax, 0x7b1f9520
00395e0f: jne    0x18039603c
00395e15: mov    rdx, qword ptr [rip + 0x3952474]         ; [0x3ce8290] str:'--profile'
00395e1c: xor    r8d, r8d
00395e1f: mov    rcx, rbx
00395e22: call   0x181981570                              ; System.String$$op_Equality
00395e27: test   al, al
00395e29: je     0x18039603c
00395e2f: inc    edi
00395e31: cmp    edi, dword ptr [rsi + 0x18]
00395e34: jae    0x18039606a
00395e3a: movsxd rax, edi
00395e3d: mov    rcx, qword ptr [rsi + rax*8 + 0x20]
00395e42: test   rcx, rcx
00395e45: je     0x180396064
00395e4b: xor    edx, edx
00395e4d: call   0x181989b70                              ; System.String$$Trim
00395e52: lea    rcx, [r14 + 0x30]
00395e56: mov    qword ptr [r14 + 0x30], rax
00395e5a: mov    rdx, rax
00395e5d: call   0x182f5fc00
00395e62: inc    edi
00395e64: mov    bpl, 1
00395e67: mov    eax, edi
00395e69: jmp    0x180395d00
00395e6e: cmp    eax, 0x54b2d278
00395e73: jbe    0x180395f39
00395e79: cmp    eax, 0x55cd707e
00395e7e: jne    0x180395ea3
00395e80: mov    rdx, qword ptr [rip + 0x39553f1]         ; [0x3ceb278] str:'-r'
00395e87: xor    r8d, r8d
00395e8a: mov    rcx, rbx
00395e8d: call   0x181981570                              ; System.String$$op_Equality
00395e92: test   al, al
00395e94: jne    0x18039600e
00395e9a: inc    edi
00395e9c: mov    eax, edi
00395e9e: jmp    0x180395d00
00395ea3: cmp    eax, 0x56cd7211
00395ea8: jne    0x180395f03
00395eaa: mov    rdx, qword ptr [rip + 0x3955487]         ; [0x3ceb338] str:'-s'
00395eb1: xor    r8d, r8d
00395eb4: mov    rcx, rbx
00395eb7: call   0x181981570                              ; System.String$$op_Equality
00395ebc: test   al, al
00395ebe: je     0x18039603c
00395ec4: inc    edi
00395ec6: cmp    edi, dword ptr [rsi + 0x18]
00395ec9: jae    0x18039606a
00395ecf: movsxd rax, edi
00395ed2: mov    rcx, qword ptr [rsi + rax*8 + 0x20]
00395ed7: test   rcx, rcx
00395eda: je     0x180396064
00395ee0: xor    edx, edx
00395ee2: call   0x181989b70                              ; System.String$$Trim
00395ee7: lea    rcx, [r14 + 0x28]
00395eeb: mov    qword ptr [r14 + 0x28], rax
00395eef: mov    rdx, rax
00395ef2: call   0x182f5fc00
00395ef7: inc    edi
00395ef9: mov    bpl, 1
00395efc: mov    eax, edi
00395efe: jmp    0x180395d00
00395f03: cmp    eax, 0x59cd76ca
00395f08: jne    0x18039603c
00395f0e: mov    rdx, qword ptr [rip + 0x39561a3]         ; [0x3cec0b8] str:'-v'
00395f15: xor    r8d, r8d
00395f18: mov    rcx, rbx
00395f1b: call   0x181981570                              ; System.String$$op_Equality
00395f20: test   al, al
00395f22: je     0x18039603c
00395f28: mov    byte ptr [r14 + 0x49], 1
00395f2d: inc    edi
00395f2f: mov    bpl, 1
00395f32: mov    eax, edi
00395f34: jmp    0x180395d00
00395f39: cmp    eax, 0x142582b1
00395f3e: jne    0x180395f6b
00395f40: mov    rdx, qword ptr [rip + 0x3951209]         ; [0x3ce7150] str:'--analysis'
00395f47: xor    r8d, r8d
00395f4a: mov    rcx, rbx
00395f4d: call   0x181981570                              ; System.String$$op_Equality
00395f52: test   al, al
00395f54: je     0x18039603c
00395f5a: mov    byte ptr [r14 + 0x58], 1
00395f5f: inc    edi
00395f61: mov    bpl, 1
00395f64: mov    eax, edi
00395f66: jmp    0x180395d00
00395f6b: cmp    eax, 0x53cd6d58
00395f70: jne    0x180395ff1
00395f72: mov    rdx, qword ptr [rip + 0x395523f]         ; [0x3ceb1b8] str:'-p'
00395f79: xor    r8d, r8d
00395f7c: mov    rcx, rbx
00395f7f: call   0x181981570                              ; System.String$$op_Equality
00395f84: test   al, al
00395f86: je     0x18039603c
00395f8c: inc    edi
00395f8e: cmp    edi, dword ptr [rsi + 0x18]
00395f91: jae    0x18039606a
00395f97: movsxd rax, edi
00395f9a: mov    rcx, qword ptr [rsi + rax*8 + 0x20]
00395f9f: test   rcx, rcx
00395fa2: je     0x180396064
00395fa8: xor    edx, edx
00395faa: call   0x181989b70                              ; System.String$$Trim
00395faf: mov    rcx, qword ptr [r14 + 0x20]
00395fb3: test   rcx, rcx
00395fb6: je     0x180396064
00395fbc: cmp    dword ptr [rcx + 0x18], 4
00395fc0: jg     0x180396039
00395fc2: mov    rbx, rcx
00395fc5: xor    r8d, r8d
00395fc8: mov    rcx, r14
00395fcb: mov    rdx, rax
00395fce: call   0x180396110                              ; CliGameLauncher$$ʴʾʹʶʺˁʼʳʼʾʹ
00395fd3: mov    r8, qword ptr [rip + 0x3967d3e]          ; [0x3cfdd18] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.Add()
00395fda: movzx  edx, ax
00395fdd: mov    rcx, rbx
00395fe0: call   0x182c62d70
00395fe5: inc    edi
00395fe7: mov    bpl, 1
00395fea: mov    eax, edi
00395fec: jmp    0x180395d00
00395ff1: cmp    eax, 0x54b2d278
00395ff6: jne    0x18039603c
00395ff8: mov    rdx, qword ptr [rip + 0x3952351]         ; [0x3ce8350] str:'--replay'
00395fff: xor    r8d, r8d
00396002: mov    rcx, rbx
00396005: call   0x181981570                              ; System.String$$op_Equality
0039600a: test   al, al
0039600c: je     0x18039603c
0039600e: inc    edi
00396010: cmp    edi, dword ptr [rsi + 0x18]
00396013: jae    0x18039606a
00396015: movsxd rax, edi
00396018: mov    rcx, qword ptr [rsi + rax*8 + 0x20]
0039601d: test   rcx, rcx
00396020: je     0x180396064
00396022: xor    edx, edx
00396024: call   0x181989b70                              ; System.String$$Trim
00396029: lea    rcx, [r14 + 0x50]
0039602d: mov    qword ptr [r14 + 0x50], rax
00396031: mov    rdx, rax
00396034: call   0x182f5fc00
00396039: mov    bpl, 1
0039603c: inc    edi
0039603e: mov    eax, edi
00396040: jmp    0x180395d00
00396045: mov    rdi, qword ptr [rsp + 0x40]
0039604a: movzx  eax, bpl
0039604e: mov    rbp, qword ptr [rsp + 0x38]
00396053: mov    rbx, qword ptr [rsp + 0x30]
00396058: mov    rsi, qword ptr [rsp + 0x48]
0039605d: add    rsp, 0x20
00396061: pop    r14
00396063: ret    
00396064: call   0x182f60c50
00396069: int3   
0039606a: call   0x182f60c40
0039606f: int3   
