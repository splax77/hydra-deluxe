02112db0: mov    qword ptr [rsp + 8], rcx
02112db5: push   rbx
02112db6: push   rsi
02112db7: push   rdi
02112db8: push   r12
02112dba: push   r13
02112dbc: push   r14
02112dbe: push   r15
02112dc0: sub    rsp, 0xd0
02112dc7: mov    r13, rcx
02112dca: cmp    byte ptr [rip + 0x1e06747], 0            ; [0x3f19518] (bss)
02112dd1: jne    0x182112e6e
02112dd7: lea    rcx, [rip + 0x1bd16ea]                   ; [0x3ce44c8] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>.GetEnumerator()
02112dde: call   0x182f609b0
02112de3: lea    rcx, [rip + 0x1bd304e]                   ; [0x3ce5e38] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.ContainsKey()
02112dea: call   0x182f609b0
02112def: lea    rcx, [rip + 0x1bd4d7a]                   ; [0x3ce7b70] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.GetEnumerator()
02112df6: call   0x182f609b0
02112dfb: lea    rcx, [rip + 0x1beaece]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
02112e02: call   0x182f609b0
02112e07: lea    rcx, [rip + 0x1bc0d22]                   ; [0x3cd3b30] meta:System.Collections.Generic.IEnumerator<KeyValuePair<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>>_TypeInfo
02112e0e: call   0x182f609b0
02112e13: lea    rcx, [rip + 0x1bc5136]                   ; [0x3cd7f50] meta:System.Collections.Generic.IEnumerator<ˀʸʲˁʽʴʶʶʷʴʾ>_TypeInfo
02112e1a: call   0x182f609b0
02112e1f: lea    rcx, [rip + 0x1bec3da]                   ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
02112e26: call   0x182f609b0
02112e2b: lea    rcx, [rip + 0x1bdcf2e]                   ; [0x3cefd60] metamethod:Method$System.Collections.Generic.KeyValuePair<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.get_Key()
02112e32: call   0x182f609b0
02112e37: lea    rcx, [rip + 0x1bdcfe2]                   ; [0x3cefe20] metamethod:Method$System.Collections.Generic.KeyValuePair<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.get_Value()
02112e3e: call   0x182f609b0
02112e43: lea    rcx, [rip + 0x1bfb11e]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02112e4a: call   0x182f609b0
02112e4f: lea    rcx, [rip + 0x1b8c8ea]                   ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
02112e56: call   0x182f609b0
02112e5b: lea    rcx, [rip + 0x1ba100e]                   ; [0x3cb3e70] str:''
02112e62: call   0x182f609b0
02112e67: mov    byte ptr [rip + 0x1e066aa], 1            ; [0x3f19518] (bss)
02112e6e: xor    r12d, r12d
02112e71: mov    qword ptr [rsp + 0x120], r12
02112e79: mov    qword ptr [rsp + 0x128], r12
02112e81: mov    r15, qword ptr [rip + 0x1ba0fe8]         ; [0x3cb3e70] str:''
02112e88: mov    qword ptr [rsp + 0x38], r15
02112e8d: mov    rax, qword ptr [rip + 0x1bfb0d4]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02112e94: cmp    dword ptr [rax + 0xe0], r12d
02112e9b: jne    0x182112eac
02112e9d: mov    rcx, rax
02112ea0: call   0x182f60cf0
02112ea5: mov    rax, qword ptr [rip + 0x1bfb0bc]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02112eac: mov    rax, qword ptr [rax + 0xb8]
02112eb3: mov    rcx, qword ptr [rax]
02112eb6: test   rcx, rcx
02112eb9: je     0x1821132c5
02112ebf: mov    qword ptr [rsp + 0x68], r13
02112ec4: mov    r14, r12
02112ec7: mov    qword ptr [rsp + 0x30], r12
02112ecc: mov    rdx, qword ptr [rip + 0x1bd4c9d]         ; [0x3ce7b70] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>.GetEnumerator()
02112ed3: call   0x1812f83a0                              ; System.Collections.Concurrent.ConcurrentDictionary<ˁʼʹʿʴʳˀʻʾʶʷ, object>$$GetEnumerator
02112ed8: mov    qword ptr [rsp + 0x120], rax
02112ee0: mov    qword ptr [rsp + 0x58], r12
02112ee5: lea    rax, [rsp + 0x120]
02112eed: mov    qword ptr [rsp + 0x60], rax
02112ef2: mov    r8, qword ptr [rsp + 0x120]
02112efa: test   r8, r8
02112efd: je     0x1821132f0
02112f03: xor    ecx, ecx
02112f05: mov    rdx, qword ptr [rip + 0x1bec2f4]         ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
02112f0c: call   0x182c44210
02112f11: test   al, al
02112f13: je     0x182113226
02112f19: mov    rbx, qword ptr [rsp + 0x120]
02112f21: test   rbx, rbx
02112f24: je     0x1821132eb
02112f2a: mov    r9, qword ptr [rip + 0x1bc0bff]          ; [0x3cd3b30] meta:System.Collections.Generic.IEnumerator<KeyValuePair<ˁʼʹʿʴʳˀʻʾʶʷ, ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>>>_TypeInfo
02112f31: mov    r10, qword ptr [rbx]
02112f34: movzx  edx, word ptr [r10 + 0x12e]
02112f3c: cmp    r12w, dx
02112f40: jae    0x182112f68
02112f42: movzx  ecx, r12w
02112f46: mov    r8, qword ptr [r10 + 0xb0]
02112f4d: nop    dword ptr [rax]
02112f50: movzx  eax, cx
02112f53: add    rax, rax
02112f56: cmp    qword ptr [r8 + rax*8], r9
02112f5a: je     0x182113128
02112f60: inc    cx
02112f63: cmp    cx, dx
02112f66: jb     0x182112f50
02112f68: xor    r8d, r8d
02112f6b: mov    rdx, r9
02112f6e: mov    rcx, rbx
02112f71: call   0x182f65070
02112f76: mov    r8, qword ptr [rax + 8]
02112f7a: mov    rdx, rbx
02112f7d: lea    rcx, [rsp + 0xb8]
02112f85: call   qword ptr [rax]
02112f87: mov    rsi, qword ptr [rax]
02112f8a: mov    rdi, qword ptr [rax + 8]
02112f8e: mov    qword ptr [rsp + 0x40], rdi
02112f93: mov    rbx, qword ptr [rax + 0x10]
02112f97: xor    r8d, r8d
02112f9a: mov    rdx, qword ptr [rip + 0x1ba0ecf]         ; [0x3cb3e70] str:''
02112fa1: mov    rcx, rsi
02112fa4: call   0x181981570                              ; System.String$$op_Equality
02112fa9: test   al, al
02112fab: jne    0x182112ef2
02112fb1: test   rbx, rbx
02112fb4: je     0x1821132e6
02112fba: mov    qword ptr [rsp + 0x78], rsi
02112fbf: mov    qword ptr [rsp + 0x70], rdi
02112fc4: mov    rdx, qword ptr [rip + 0x1bd14fd]         ; [0x3ce44c8] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʲˁʽʴʶʶʷʴʾ>.GetEnumerator()
02112fcb: mov    rcx, rbx
02112fce: call   0x1812f5a60                              ; System.Collections.Concurrent.ConcurrentBag<object>$$GetEnumerator
02112fd3: mov    qword ptr [rsp + 0x128], rax
02112fdb: mov    qword ptr [rsp + 0x48], r12
02112fe0: lea    rbx, [rsp + 0x128]
02112fe8: mov    qword ptr [rsp + 0x50], rbx
02112fed: nop    dword ptr [rax]
02112ff0: mov    r8, qword ptr [rsp + 0x128]
02112ff8: test   r8, r8
02112ffb: je     0x1821132db
02113001: xor    ecx, ecx
02113003: mov    rdx, qword ptr [rip + 0x1bec1f6]         ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
0211300a: call   0x182c44210
0211300f: test   al, al
02113011: je     0x1821131c6
02113017: mov    r8, qword ptr [rsp + 0x128]
0211301f: test   r8, r8
02113022: je     0x1821132d6
02113028: xor    ecx, ecx
0211302a: mov    rdx, qword ptr [rip + 0x1bc4f1f]         ; [0x3cd7f50] meta:System.Collections.Generic.IEnumerator<ˀʸʲˁʽʴʶʶʷʴʾ>_TypeInfo
02113031: call   0x182c44210
02113036: mov    rdi, rax
02113039: mov    qword ptr [rsp + 0x80], rax
02113041: test   r13, r13
02113044: je     0x182113058
02113046: xor    r8d, r8d
02113049: mov    rdx, r13
0211304c: mov    rcx, rsi
0211304f: call   0x181989ec0                              ; System.String$$op_Inequality
02113054: test   al, al
02113056: jne    0x182112ff0
02113058: test   rdi, rdi
0211305b: je     0x1821132d1
02113061: mov    ecx, 4
02113066: mov    r8, rdi
02113069: mov    rdx, qword ptr [rip + 0x1b8c6d0]         ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
02113070: call   0x182c44210
02113075: movzx  eax, al
02113078: cmp    qword ptr [rsp + 0x68], 0
0211307e: mov    ecx, 1
02113083: cmova  eax, ecx
02113086: test   al, al
02113088: je     0x182112ff0
0211308e: mov    rax, qword ptr [rip + 0x1bfaed3]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02113095: cmp    dword ptr [rax + 0xe0], 0
0211309c: jne    0x1821130ad
0211309e: mov    rcx, rax
021130a1: call   0x182f60cf0
021130a6: mov    rax, qword ptr [rip + 0x1bfaebb]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
021130ad: mov    rax, qword ptr [rax + 0xb8]
021130b4: mov    rcx, qword ptr [rax + 8]
021130b8: test   rcx, rcx
021130bb: je     0x1821132cb
021130c1: mov    r8, qword ptr [rip + 0x1bd2d70]          ; [0x3ce5e38] metamethod:Method$System.Collections.Concurrent.ConcurrentDictionary<string, string>.ContainsKey()
021130c8: mov    rdx, rsi
021130cb: call   0x1812feb00                              ; System.Collections.Concurrent.ConcurrentDictionary<object, object>$$ContainsKey
021130d0: test   al, al
021130d2: je     0x182112ff0
021130d8: xor    r8d, r8d
021130db: mov    rdx, rsi
021130de: mov    rcx, r15
021130e1: call   0x181989ec0                              ; System.String$$op_Inequality
021130e6: test   al, al
021130e8: je     0x1821131a0
021130ee: test   r14, r14
021130f1: je     0x182113171
021130f3: mov    rcx, qword ptr [rip + 0x1bfae6e]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
021130fa: cmp    dword ptr [rcx + 0xe0], 0
02113101: jne    0x182113108
02113103: call   0x182f60cf0
02113108: xor    edx, edx
0211310a: mov    rcx, r15
0211310d: call   0x182115760                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˁʹʲʵʽʲʲʸʻʳˀ
02113112: test   rax, rax
02113115: je     0x182113126
02113117: xor    r8d, r8d
0211311a: mov    rdx, rax
0211311d: mov    rcx, r14
02113120: call   0x182165c50                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʸʷʽʺʼʻʿʾʸʾʸ
02113125: nop    
02113126: jmp    0x18211316c
02113128: movzx  ecx, cx
0211312b: add    rcx, rcx
0211312e: movsxd rax, dword ptr [r8 + rcx*8 + 8]
02113133: shl    rax, 4
02113137: add    rax, 0x138
0211313d: add    rax, r10
02113140: jmp    0x182112f76
02113145: xor    r12d, r12d
02113148: mov    r13, qword ptr [rsp + 0x110]
02113150: mov    rbx, qword ptr [rsp + 0x50]
02113155: mov    rax, qword ptr [rsp + 0x70]
0211315a: mov    qword ptr [rsp + 0x40], rax
0211315f: mov    rsi, qword ptr [rsp + 0x78]
02113164: mov    rdi, qword ptr [rsp + 0x80]
0211316c: mov    qword ptr [rsp + 0x30], r12
02113171: mov    r15, rsi
02113174: mov    qword ptr [rsp + 0x38], rsi
02113179: mov    rcx, qword ptr [rip + 0x1bfade8]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02113180: cmp    dword ptr [rcx + 0xe0], 0
02113187: jne    0x18211318e
02113189: call   0x182f60cf0
0211318e: xor    edx, edx
02113190: mov    rcx, rsi
02113193: call   0x182114600                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ʽʽʵʶʸʹʸʽʼʳʹ
02113198: mov    r14, rax
0211319b: mov    qword ptr [rsp + 0x30], rax
021131a0: mov    ecx, 2
021131a5: mov    rax, qword ptr [rsp + 0x40]
021131aa: mov    qword ptr [rsp + 0x20], rax
021131af: mov    r9, r14
021131b2: mov    r8, rdi
021131b5: mov    rdx, qword ptr [rip + 0x1b8c584]         ; [0x3c9f740] meta:ˀʸʲˁʽʴʶʶʷʴʾ_TypeInfo
021131bc: call   0x182c44a20
021131c1: jmp    0x182112ff0
021131c6: cmp    qword ptr [rbx], 0
021131ca: je     0x1821131dd
021131cc: xor    ecx, ecx
021131ce: mov    r8, qword ptr [rbx]
021131d1: mov    rdx, qword ptr [rip + 0x1beaaf8]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
021131d8: call   0x182c42550
021131dd: jmp    0x182112ef2
021131e2: mov    r8, qword ptr [rsp + 0x50]
021131e7: cmp    qword ptr [r8], 0
021131eb: je     0x1821131fe
021131ed: xor    ecx, ecx
021131ef: mov    r8, qword ptr [r8]
021131f2: mov    rdx, qword ptr [rip + 0x1beaad7]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
021131f9: call   0x182c42550
021131fe: mov    rcx, qword ptr [rsp + 0x48]
02113203: test   rcx, rcx
02113206: jne    0x1821132e1
0211320c: xor    r12d, r12d
0211320f: mov    r13, qword ptr [rsp + 0x110]
02113217: mov    r14, qword ptr [rsp + 0x30]
0211321c: mov    r15, qword ptr [rsp + 0x38]
02113221: jmp    0x182112ef2
02113226: mov    r8, qword ptr [rsp + 0x60]
0211322b: cmp    qword ptr [r8], 0
0211322f: je     0x182113242
02113231: xor    ecx, ecx
02113233: mov    r8, qword ptr [r8]
02113236: mov    rdx, qword ptr [rip + 0x1beaa93]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
0211323d: call   0x182c42550
02113242: jmp    0x182113278
02113244: mov    r8, qword ptr [rsp + 0x60]
02113249: cmp    qword ptr [r8], 0
0211324d: je     0x182113260
0211324f: xor    ecx, ecx
02113251: mov    r8, qword ptr [r8]
02113254: mov    rdx, qword ptr [rip + 0x1beaa75]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
0211325b: call   0x182c42550
02113260: mov    rcx, qword ptr [rsp + 0x58]
02113265: test   rcx, rcx
02113268: jne    0x1821132f6
0211326e: mov    r14, qword ptr [rsp + 0x30]
02113273: mov    r15, qword ptr [rsp + 0x38]
02113278: test   r14, r14
0211327b: je     0x1821132b2
0211327d: mov    rcx, qword ptr [rip + 0x1bface4]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
02113284: cmp    dword ptr [rcx + 0xe0], 0
0211328b: jne    0x182113292
0211328d: call   0x182f60cf0
02113292: xor    edx, edx
02113294: mov    rcx, r15
02113297: call   0x182115760                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˁʹʲʵʽʲʲʸʻʳˀ
0211329c: test   rax, rax
0211329f: je     0x1821132b0
021132a1: xor    r8d, r8d
021132a4: mov    rdx, rax
021132a7: mov    rcx, r14
021132aa: call   0x182165c50                              ; ʼʿʺʿʹʳʹʶʹʹʼ$$ʸʷʽʺʼʻʿʾʸʾʸ
021132af: nop    
021132b0: jmp    0x1821132b2
021132b2: add    rsp, 0xd0
021132b9: pop    r15
021132bb: pop    r14
021132bd: pop    r13
021132bf: pop    r12
021132c1: pop    rdi
021132c2: pop    rsi
021132c3: pop    rbx
021132c4: ret    
021132c5: call   0x182f60c50
021132ca: nop    
021132cb: call   0x182f60c50
021132d0: nop    
021132d1: call   0x182f60c50
021132d6: call   0x182f60c50
021132db: call   0x182f60c50
021132e0: nop    
021132e1: call   0x182f60ce0
021132e6: call   0x182f60c50
021132eb: call   0x182f60c50
021132f0: call   0x182f60c50
021132f5: nop    
021132f6: call   0x182f60ce0
021132fb: int3   
021132fc: int3   
