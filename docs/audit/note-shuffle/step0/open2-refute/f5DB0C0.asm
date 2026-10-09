005db0c0: mov    qword ptr [rsp + 8], rcx
005db0c5: push   rdi
005db0c6: push   r15
005db0c8: sub    rsp, 0x38
005db0cc: cmp    qword ptr [r8 + 0x38], 0
005db0d1: mov    r15, r8
005db0d4: mov    rdi, rdx
005db0d7: jne    0x1805db100
005db0d9: lea    rcx, [rip + 0x371ad68]                   ; [0x3cf5e48] meta:System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>_TypeInfo
005db0e0: call   0x182f609b0
005db0e5: lea    rcx, [rip + 0x36be98c]                   ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db0ec: call   0x182f609b0
005db0f1: cmp    qword ptr [r15 + 0x38], 0
005db0f6: jne    0x1805db100
005db0f8: mov    rcx, r15
005db0fb: call   0x182f657d0
005db100: mov    qword ptr [rsp + 0x58], rbx
005db105: mov    qword ptr [rsp + 0x60], rbp
005db10a: mov    qword ptr [rsp + 0x68], rsi
005db10f: mov    qword ptr [rsp + 0x30], r12
005db114: mov    qword ptr [rsp + 0x28], r13
005db119: mov    qword ptr [rsp + 0x20], r14
005db11e: test   rdi, rdi
005db121: je     0x1805db310
005db127: xor    r13d, r13d
005db12a: xor    eax, eax
005db12c: nop    dword ptr [rax]
005db130: cmp    eax, dword ptr [rdi + 0x18]
005db133: jge    0x1805db246
005db139: mov    r8, qword ptr [r15 + 0x38]
005db13d: mov    edx, r13d
005db140: mov    rcx, rdi
005db143: mov    r8, qword ptr [r8 + 8]
005db147: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db14c: mov    rbp, rax
005db14f: test   rax, rax
005db152: je     0x1805db23b
005db158: mov    rdx, qword ptr [rip + 0x36be919]         ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db15f: xor    ecx, ecx
005db161: mov    r8, rax
005db164: call   0x182c44210
005db169: mov    rbp, qword ptr [rbp + 0x10]
005db16d: lea    esi, [r13 + 1]
005db171: mov    r12d, eax
005db174: mov    r14d, r13d
005db177: cmp    esi, dword ptr [rdi + 0x18]
005db17a: jge    0x1805db1e0
005db17c: mov    r8, qword ptr [r15 + 0x38]
005db180: mov    edx, esi
005db182: mov    rcx, rdi
005db185: mov    r8, qword ptr [r8 + 8]
005db189: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db18e: mov    rbx, rax
005db191: test   rax, rax
005db194: je     0x1805db1dc
005db196: cmp    qword ptr [rax + 0x10], rbp
005db19a: jg     0x1805db1e0
005db19c: jne    0x1805db1dc
005db19e: mov    rdx, qword ptr [rip + 0x36be8d3]         ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db1a5: xor    ecx, ecx
005db1a7: mov    r8, rax
005db1aa: call   0x182c44210
005db1af: cmp    eax, r12d
005db1b2: jne    0x1805db1dc
005db1b4: mov    r8, qword ptr [r15 + 0x38]
005db1b8: mov    edx, r14d
005db1bb: mov    rbx, qword ptr [rbx + 0x18]
005db1bf: mov    rcx, rdi
005db1c2: mov    r8, qword ptr [r8 + 8]
005db1c6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db1cb: test   rax, rax
005db1ce: je     0x1805db310
005db1d4: cmp    rbx, qword ptr [rax + 0x18]
005db1d8: cmovg  r14d, esi
005db1dc: inc    esi
005db1de: jmp    0x1805db177
005db1e0: mov    ebx, r13d
005db1e3: cmp    ebx, dword ptr [rdi + 0x18]
005db1e6: jge    0x1805db23b
005db1e8: cmp    ebx, r14d
005db1eb: je     0x1805db237
005db1ed: mov    r8, qword ptr [r15 + 0x38]
005db1f1: mov    edx, ebx
005db1f3: mov    rcx, rdi
005db1f6: mov    r8, qword ptr [r8 + 8]
005db1fa: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db1ff: test   rax, rax
005db202: je     0x1805db237
005db204: cmp    qword ptr [rax + 0x10], rbp
005db208: jg     0x1805db23b
005db20a: jne    0x1805db237
005db20c: mov    rdx, qword ptr [rip + 0x36be865]         ; [0x3c99a78] meta:ʻʵʷʳˀʴʴʴʵʺʷ_TypeInfo
005db213: xor    ecx, ecx
005db215: mov    r8, rax
005db218: call   0x182c44210
005db21d: cmp    eax, r12d
005db220: jne    0x1805db237
005db222: mov    r9, qword ptr [r15 + 0x38]
005db226: xor    r8d, r8d
005db229: mov    edx, ebx
005db22b: mov    rcx, rdi
005db22e: mov    r9, qword ptr [r9 + 0x20]
005db232: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
005db237: inc    ebx
005db239: jmp    0x1805db1e3
005db23b: inc    r13d
005db23e: mov    eax, r13d
005db241: jmp    0x1805db130
005db246: mov    rdx, qword ptr [rip + 0x371abfb]         ; [0x3cf5e48] meta:System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>_TypeInfo
005db24d: mov    r8, qword ptr [rdi]
005db250: movzx  eax, byte ptr [rdx + 0x130]
005db257: cmp    byte ptr [r8 + 0x130], al
005db25e: jb     0x1805db278
005db260: movzx  ecx, al
005db263: mov    rax, qword ptr [r8 + 0xc8]
005db26a: cmp    qword ptr [rax + rcx*8 - 8], rdx
005db26f: jne    0x1805db278
005db271: mov    eax, 1
005db276: jmp    0x1805db27a
005db278: xor    eax, eax
005db27a: xor    ebp, ebp
005db27c: mov    esi, 0xffffffff
005db281: test   eax, eax
005db283: cmovne rbp, rdi
005db287: xor    ebx, ebx
005db289: xor    eax, eax
005db28b: nop    dword ptr [rax + rax]
005db290: cmp    eax, dword ptr [rdi + 0x18]
005db293: jge    0x1805db2d1
005db295: mov    r8, qword ptr [r15 + 0x38]
005db299: mov    edx, ebx
005db29b: mov    rcx, rdi
005db29e: mov    r8, qword ptr [r8 + 8]
005db2a2: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
005db2a7: test   rax, rax
005db2aa: jne    0x1805db2cb
005db2ac: mov    r8, qword ptr [r15 + 0x38]
005db2b0: mov    edx, ebx
005db2b2: mov    rcx, rdi
005db2b5: mov    r8, qword ptr [r8 + 0x28]
005db2b9: call   0x180774920                              ; System.Collections.Generic.List<object>$$RemoveAt
005db2be: test   rbp, rbp
005db2c1: je     0x1805db2c9
005db2c3: cmp    esi, -1
005db2c6: cmove  esi, ebx
005db2c9: dec    ebx
005db2cb: inc    ebx
005db2cd: mov    eax, ebx
005db2cf: jmp    0x1805db290
005db2d1: test   rbp, rbp
005db2d4: je     0x1805db2ea
005db2d6: cmp    esi, -1
005db2d9: je     0x1805db2ea
005db2db: mov    rcx, qword ptr [rsp + 0x50]
005db2e0: xor    r8d, r8d
005db2e3: mov    edx, esi
005db2e5: call   0x18215ea80                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʻʺˁˀʴʶʹʿˀʿʴ
005db2ea: mov    r14, qword ptr [rsp + 0x20]
005db2ef: mov    r13, qword ptr [rsp + 0x28]
005db2f4: mov    r12, qword ptr [rsp + 0x30]
005db2f9: mov    rsi, qword ptr [rsp + 0x68]
005db2fe: mov    rbp, qword ptr [rsp + 0x60]
005db303: mov    rbx, qword ptr [rsp + 0x58]
005db308: add    rsp, 0x38
005db30c: pop    r15
005db30e: pop    rdi
005db30f: ret    
005db310: call   0x182f60c50
005db315: int3   
005db316: int3   
