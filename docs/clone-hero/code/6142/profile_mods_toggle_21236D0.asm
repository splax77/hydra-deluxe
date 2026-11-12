021236d0: mov    qword ptr [rsp + 0x18], rbp
021236d5: mov    qword ptr [rsp + 0x20], rsi
021236da: push   r14
021236dc: sub    rsp, 0x20
021236e0: mov    ebp, 0x100000
021236e5: mov    r14d, edx
021236e8: mov    rsi, rcx
021236eb: cmp    edx, ebp
021236ed: je     0x182123762
021236ef: test   dword ptr [rcx + 0x14], ebp
021236f2: jbe    0x1821236fc
021236f4: cmp    edx, 0x20000
021236fa: je     0x182123762
021236fc: cmp    r14d, 1
02123700: je     0x18212374e
02123702: mov    qword ptr [rsp + 0x30], rbx
02123707: mov    qword ptr [rsp + 0x38], rdi
0212370c: cmp    dword ptr [rcx + 0x14], 1
02123710: jne    0x182123719
02123712: mov    dword ptr [rcx + 0x14], 0
02123719: mov    ebx, dword ptr [rcx + 0x14]
0212371c: mov    edi, r14d
0212371f: and    edi, dword ptr [rcx + 0x14]
02123722: xor    r8d, r8d
02123725: call   0x182122ef0                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʴʵʶʹʿʻʲʷʸʻ
0212372a: not    eax
0212372c: and    eax, ebx
0212372e: mov    rbx, qword ptr [rsp + 0x30]
02123733: test   edi, edi
02123735: mov    dword ptr [rsi + 0x14], eax
02123738: mov    rdi, qword ptr [rsp + 0x38]
0212373d: jne    0x182123745
0212373f: or     eax, r14d
02123742: mov    dword ptr [rsi + 0x14], eax
02123745: cmp    dword ptr [rsi + 0x14], 0
02123749: jne    0x182123762
0212374b: mov    rcx, rsi
0212374e: xor    edx, edx
02123750: call   0x182123150                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʴʶʾʶˀʺʾʲʸʻ
02123755: test   al, al
02123757: mov    ecx, 1
0212375c: cmove  ebp, ecx
0212375f: mov    dword ptr [rsi + 0x14], ebp
02123762: mov    rbp, qword ptr [rsp + 0x40]
02123767: mov    rsi, qword ptr [rsp + 0x48]
0212376c: add    rsp, 0x20
02123770: pop    r14
02123772: ret    
02123773: int3   
