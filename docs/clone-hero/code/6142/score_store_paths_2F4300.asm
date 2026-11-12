002f4300: push   rbx
002f4302: sub    rsp, 0x20
002f4306: cmp    byte ptr [rip + 0x3c19706], 0            ; [0x3f0da13] (bss)
002f430d: jne    0x1802f435e
002f430f: lea    rcx, [rip + 0x39c2e3a]                   ; [0x3cb7150] meta:System.IO.Path_TypeInfo
002f4316: call   0x182f609b0
002f431b: lea    rcx, [rip + 0x399ff5e]                   ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
002f4322: call   0x182f609b0
002f4327: lea    rcx, [rip + 0x39a9a12]                   ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f432e: call   0x182f609b0
002f4333: lea    rcx, [rip + 0x39b43f6]                   ; [0x3ca8730] str:'scoredata_backup.bin'
002f433a: call   0x182f609b0
002f433f: lea    rcx, [rip + 0x39b4332]                   ; [0x3ca8678] str:'scoredata.bin'
002f4346: call   0x182f609b0
002f434b: lea    rcx, [rip + 0x39b454e]                   ; [0x3ca88a0] str:'scores.bin'
002f4352: call   0x182f609b0
002f4357: mov    byte ptr [rip + 0x3c196b5], 1            ; [0x3f0da13] (bss)
002f435e: mov    rcx, qword ptr [rip + 0x399ff1b]         ; [0x3c94280] meta:ʵˁʼʾʽʶʵʹʴʸʻ_TypeInfo
002f4365: cmp    dword ptr [rcx + 0xe0], 0
002f436c: jne    0x1802f4373
002f436e: call   0x182f60cf0
002f4373: xor    ecx, ecx
002f4375: call   0x180307590                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʳʸʿʸʿʻʷʴʷʻʽ
002f437a: mov    rcx, qword ptr [rip + 0x39c2dcf]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
002f4381: mov    rbx, rax
002f4384: cmp    dword ptr [rcx + 0xe0], 0
002f438b: jne    0x1802f4392
002f438d: call   0x182f60cf0
002f4392: mov    rdx, qword ptr [rip + 0x39b42df]         ; [0x3ca8678] str:'scoredata.bin'
002f4399: xor    r8d, r8d
002f439c: mov    rcx, rbx
002f439f: call   0x181b1eee0                              ; System.IO.Path$$Combine
002f43a4: mov    rcx, qword ptr [rip + 0x39a9995]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f43ab: mov    rbx, rax
002f43ae: cmp    dword ptr [rcx + 0xe0], 0
002f43b5: jne    0x1802f43c3
002f43b7: call   0x182f60cf0
002f43bc: mov    rcx, qword ptr [rip + 0x39a997d]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f43c3: mov    rcx, qword ptr [rcx + 0xb8]
002f43ca: mov    rdx, rbx
002f43cd: mov    qword ptr [rcx], rbx
002f43d0: mov    rcx, qword ptr [rip + 0x39a9969]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f43d7: mov    rcx, qword ptr [rcx + 0xb8]
002f43de: call   0x182f5fc00
002f43e3: xor    ecx, ecx
002f43e5: call   0x180307590                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʳʸʿʸʿʻʷʴʷʻʽ
002f43ea: mov    rdx, qword ptr [rip + 0x39b44af]         ; [0x3ca88a0] str:'scores.bin'
002f43f1: xor    r8d, r8d
002f43f4: mov    rcx, rax
002f43f7: call   0x181b1eee0                              ; System.IO.Path$$Combine
002f43fc: mov    rcx, qword ptr [rip + 0x39a993d]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f4403: mov    rdx, qword ptr [rcx + 0xb8]
002f440a: mov    qword ptr [rdx + 8], rax
002f440e: mov    rdx, rax
002f4411: mov    rcx, qword ptr [rip + 0x39a9928]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f4418: mov    rcx, qword ptr [rcx + 0xb8]
002f441f: add    rcx, 8
002f4423: call   0x182f5fc00
002f4428: xor    ecx, ecx
002f442a: call   0x180307590                              ; ʵˁʼʾʽʶʵʹʴʸʻ$$ʳʸʿʸʿʻʷʴʷʻʽ
002f442f: mov    rdx, qword ptr [rip + 0x39b42fa]         ; [0x3ca8730] str:'scoredata_backup.bin'
002f4436: xor    r8d, r8d
002f4439: mov    rcx, rax
002f443c: call   0x181b1eee0                              ; System.IO.Path$$Combine
002f4441: mov    rcx, qword ptr [rip + 0x39a98f8]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f4448: mov    rdx, qword ptr [rcx + 0xb8]
002f444f: mov    qword ptr [rdx + 0x10], rax
002f4453: mov    rdx, rax
002f4456: mov    rcx, qword ptr [rip + 0x39a98e3]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
002f445d: mov    rcx, qword ptr [rcx + 0xb8]
002f4464: add    rcx, 0x10
002f4468: call   0x182f5fc00
002f446d: xor    ecx, ecx
002f446f: add    rsp, 0x20
002f4473: pop    rbx
002f4474: jmp    0x1801564a0                              ; ʿʺʸʿˀʼʴʾʷʳʵ$$ʳʶʿʳʾˁʴʸʷʾʴ
002f4479: int3   
