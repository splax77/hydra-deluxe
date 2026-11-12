001eb800: mov    qword ptr [rsp + 8], rbx
001eb805: push   rdi
001eb806: sub    rsp, 0x30
001eb80a: mov    rdi, rcx
001eb80d: cmp    byte ptr [rip + 0x3d21312], 0            ; [0x3f0cb26] (bss)
001eb814: jne    0x1801eb859
001eb816: lea    rcx, [rip + 0x3acb933]                   ; [0x3cb7150] meta:System.IO.Path_TypeInfo
001eb81d: call   0x182f609b0
001eb822: lea    rcx, [rip + 0x3b2250f]                   ; [0x3d0dd38] meta:ʲʴʹʶʻʵʴʸʿʲʶ_TypeInfo
001eb829: call   0x182f609b0
001eb82e: lea    rcx, [rip + 0x3aa8a4b]                   ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
001eb835: call   0x182f609b0
001eb83a: lea    rcx, [rip + 0x3ac1997]                   ; [0x3cad1d8] str:'settings_schema.json'
001eb841: call   0x182f609b0
001eb846: lea    rcx, [rip + 0x3aca9e3]                   ; [0x3cb6230] str:'Threshold'
001eb84d: call   0x182f609b0
001eb852: mov    byte ptr [rip + 0x3d212cd], 1            ; [0x3f0cb26] (bss)
001eb859: mov    rcx, qword ptr [rip + 0x3aa8a20]         ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
001eb860: cmp    dword ptr [rcx + 0xe0], 0
001eb867: jne    0x1801eb86e
001eb869: call   0x182f60cf0
001eb86e: xor    ecx, ecx
001eb870: call   0x180307590                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʳʸʿʸʿʻʷʴʷʻʽ
001eb875: mov    rbx, rax
001eb878: mov    rcx, qword ptr [rip + 0x3acb8d1]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
001eb87f: cmp    dword ptr [rcx + 0xe0], 0
001eb886: jne    0x1801eb88d
001eb888: call   0x182f60cf0
001eb88d: xor    r8d, r8d
001eb890: mov    rdx, qword ptr [rip + 0x3aca999]         ; [0x3cb6230] str:'Threshold'
001eb897: mov    rcx, rbx
001eb89a: call   0x181b1eee0                              ; System.IO.Path$$Combine
001eb89f: nop    
001eb8a0: xor    edx, edx
001eb8a2: mov    rcx, rax
001eb8a5: call   0x181b1bd50                              ; System.IO.File$$ReadAllBytes
001eb8aa: xor    r9d, r9d
001eb8ad: mov    r8, qword ptr [rip + 0x3ac1924]          ; [0x3cad1d8] str:'settings_schema.json'
001eb8b4: mov    rdx, rax
001eb8b7: mov    rcx, rdi
001eb8ba: call   0x1801ecc50                              ; ʲʴʹʶʻʵʴʸʿʲʶ$$ˁʳʹʷʹʸʻʸʺʹʴ
001eb8bf: mov    rbx, rax
001eb8c2: mov    rax, qword ptr [rip + 0x3b2246f]         ; [0x3d0dd38] meta:ʲʴʹʶʻʵʴʸʿʲʶ_TypeInfo
001eb8c9: mov    rcx, qword ptr [rax + 0xb8]
