00377fc0: push   rbx
00377fc2: sub    rsp, 0x20
00377fc6: cmp    byte ptr [rip + 0x3b96148], 0            ; [0x3f0e115] (bss)
00377fcd: jne    0x180377ffa
00377fcf: lea    rcx, [rip + 0x393f17a]                   ; [0x3cb7150] meta:System.IO.Path_TypeInfo
00377fd6: call   0x182f609b0
00377fdb: lea    rcx, [rip + 0x391c29e]                   ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
00377fe2: call   0x182f609b0
00377fe7: lea    rcx, [rip + 0x3935132]                   ; [0x3cad120] str:'settings.ini'
00377fee: call   0x182f609b0
00377ff3: mov    byte ptr [rip + 0x3b9611b], 1            ; [0x3f0e115] (bss)
00377ffa: mov    rcx, qword ptr [rip + 0x391c27f]         ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
00378001: cmp    dword ptr [rcx + 0xe0], 0
00378008: jne    0x18037800f
0037800a: call   0x182f60cf0
0037800f: xor    ecx, ecx
00378011: call   0x180307b40                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʴˁʿʸʷˁʷʳʴʻʷ
00378016: mov    rcx, qword ptr [rip + 0x393f133]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
0037801d: mov    rbx, rax
00378020: cmp    dword ptr [rcx + 0xe0], 0
00378027: jne    0x18037802e
00378029: call   0x182f60cf0
0037802e: mov    rdx, qword ptr [rip + 0x39350eb]         ; [0x3cad120] str:'settings.ini'
00378035: xor    r8d, r8d
00378038: mov    rcx, rbx
0037803b: add    rsp, 0x20
0037803f: pop    rbx
00378040: jmp    0x181b1eee0                              ; System.IO.Path$$Combine
00378045: int3   
