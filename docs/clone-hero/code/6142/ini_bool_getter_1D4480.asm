001d4480: mov    qword ptr [rsp + 8], rbx
001d4485: push   rbp
001d4486: push   rsi
001d4487: push   rdi
001d4488: sub    rsp, 0x30
001d448c: cmp    byte ptr [rip + 0x3d38738], 0            ; [0x3f0cbcb] (bss)
001d4493: movzx  ebx, r9b
001d4497: mov    rdi, r8
001d449a: mov    rsi, rdx
001d449d: mov    rbp, rcx
001d44a0: jne    0x1801d44c1
001d44a2: lea    rcx, [rip + 0x3b24b87]                   ; [0x3cf9030] str:'false'
001d44a9: call   0x182f609b0
001d44ae: lea    rcx, [rip + 0x3b0a7c3]                   ; [0x3cdec78] str:'true'
001d44b5: call   0x182f609b0
001d44ba: mov    byte ptr [rip + 0x3d3870a], 1            ; [0x3f0cbcb] (bss)
001d44c1: xor    eax, eax
001d44c3: lea    r9, [rsp + 0x60]
001d44c8: mov    r8, rdi
001d44cb: mov    qword ptr [rsp + 0x60], rax
001d44d0: mov    rdx, rsi
001d44d3: mov    dword ptr [rsp + 0x58], eax
001d44d7: mov    rcx, rbp
001d44da: mov    qword ptr [rsp + 0x20], rax
001d44df: call   0x1801bd150
001d44e4: test   al, al
001d44e6: je     0x1801d457b
001d44ec: mov    rcx, qword ptr [rsp + 0x60]
001d44f1: lea    rdx, [rsp + 0x58]
001d44f6: xor    r8d, r8d
001d44f9: call   0x181b4f710                              ; System.Int32$$TryParse
001d44fe: test   al, al
001d4500: jne    0x1801d4566
001d4502: mov    rcx, qword ptr [rsp + 0x60]
001d4507: test   rcx, rcx
001d450a: je     0x1801d458b
001d450c: mov    rdx, qword ptr [rip + 0x3b0a765]         ; [0x3cdec78] str:'true'
001d4513: xor    r9d, r9d
001d4516: mov    r8d, 5
001d451c: call   0x181981640                              ; System.String$$Equals
001d4521: test   al, al
001d4523: jne    0x1801d4557
001d4525: mov    rcx, qword ptr [rsp + 0x60]
001d452a: test   rcx, rcx
001d452d: je     0x1801d458b
001d452f: mov    rdx, qword ptr [rip + 0x3b24afa]         ; [0x3cf9030] str:'false'
001d4536: xor    r9d, r9d
001d4539: mov    r8d, 5
001d453f: call   0x181981640                              ; System.String$$Equals
001d4544: test   al, al
001d4546: je     0x1801d457b
001d4548: xor    al, al
001d454a: mov    rbx, qword ptr [rsp + 0x50]
001d454f: add    rsp, 0x30
001d4553: pop    rdi
001d4554: pop    rsi
001d4555: pop    rbp
001d4556: ret    
001d4557: mov    al, 1
001d4559: mov    rbx, qword ptr [rsp + 0x50]
001d455e: add    rsp, 0x30
001d4562: pop    rdi
001d4563: pop    rsi
001d4564: pop    rbp
001d4565: ret    
001d4566: cmp    dword ptr [rsp + 0x58], 0
001d456b: seta   al
001d456e: mov    rbx, qword ptr [rsp + 0x50]
001d4573: add    rsp, 0x30
001d4577: pop    rdi
001d4578: pop    rsi
001d4579: pop    rbp
001d457a: ret    
001d457b: movzx  eax, bl
001d457e: mov    rbx, qword ptr [rsp + 0x50]
001d4583: add    rsp, 0x30
001d4587: pop    rdi
001d4588: pop    rsi
001d4589: pop    rbp
001d458a: ret    
001d458b: call   0x182f60c50
001d4590: int3   
001d4591: int3   
