020fa8f0: mov    qword ptr [rsp + 8], rbx
020fa8f5: mov    qword ptr [rsp + 0x18], rsi
020fa8fa: push   rdi
020fa8fb: sub    rsp, 0x50
020fa8ff: mov    edi, r8d
020fa902: mov    rsi, rdx
020fa905: mov    rbx, rcx
020fa908: cmp    byte ptr [rip + 0x1e1ea8f], 0            ; [0x3f1939e] (bss)
020fa90f: jne    0x1820fa924
020fa911: lea    rcx, [rip + 0x1c033b8]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020fa918: call   0x182f609b0
020fa91d: mov    byte ptr [rip + 0x1e1ea7a], 1            ; [0x3f1939e] (bss)
020fa924: mov    qword ptr [rsp + 0x68], 0
020fa92d: xor    ecx, ecx
020fa92f: call   0x1819bf800                              ; System.Security.Cryptography.MD5$$Create
020fa934: mov    qword ptr [rsp + 0x68], rax
020fa939: mov    qword ptr [rsp + 0x38], 0
020fa942: lea    rax, [rsp + 0x68]
020fa947: mov    qword ptr [rsp + 0x40], rax
020fa94c: mov    rcx, qword ptr [rsp + 0x68]
020fa951: test   rcx, rcx
020fa954: je     0x1820fa9c8
020fa956: mov    qword ptr [rsp + 0x20], 0
020fa95f: mov    r9d, edi
020fa962: xor    r8d, r8d
020fa965: mov    rdx, rsi
020fa968: call   0x1819be3b0                              ; System.Security.Cryptography.HashAlgorithm$$ComputeHash
020fa96d: xor    r8d, r8d
020fa970: mov    rdx, rax
020fa973: mov    rcx, rbx
020fa976: call   0x1820fa1e0                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʸʶʳʽʺʵˁʷʿʴʻ
020fa97b: nop    
020fa97c: mov    r8, qword ptr [rsp + 0x68]
020fa981: test   r8, r8
020fa984: je     0x1820fa994
020fa986: xor    ecx, ecx
020fa988: mov    rdx, qword ptr [rip + 0x1c03341]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020fa98f: call   0x182c42550
020fa994: jmp    0x1820fa9b8
020fa996: mov    r8, qword ptr [rsp + 0x68]
020fa99b: test   r8, r8
020fa99e: je     0x1820fa9ae
020fa9a0: xor    ecx, ecx
020fa9a2: mov    rdx, qword ptr [rip + 0x1c03327]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020fa9a9: call   0x182c42550
020fa9ae: mov    rcx, qword ptr [rsp + 0x38]
020fa9b3: test   rcx, rcx
020fa9b6: jne    0x1820fa9ce
020fa9b8: mov    rbx, qword ptr [rsp + 0x60]
020fa9bd: mov    rsi, qword ptr [rsp + 0x70]
020fa9c2: add    rsp, 0x50
020fa9c6: pop    rdi
020fa9c7: ret    
020fa9c8: call   0x182f60c50
020fa9cd: nop    
020fa9ce: call   0x182f60ce0
020fa9d3: int3   
020fa9d4: int3   
