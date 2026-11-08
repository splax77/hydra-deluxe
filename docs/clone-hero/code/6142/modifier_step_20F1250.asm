020f1250: mov    qword ptr [rsp + 0x10], rsi
020f1255: push   rdi
020f1256: sub    rsp, 0x20
020f125a: cmp    byte ptr [rip + 0x1e2810b], 0            ; [0x3f1936c] (bss)
020f1261: mov    rsi, rcx
020f1264: jne    0x1820f12c1
020f1266: lea    rcx, [rip + 0x1be5df3]                   ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020f126d: call   0x182f609b0
020f1272: lea    rcx, [rip + 0x1be5f67]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f1279: call   0x182f609b0
020f127e: lea    rcx, [rip + 0x1be601b]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1285: call   0x182f609b0
020f128a: lea    rcx, [rip + 0x1be60cf]                   ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020f1291: call   0x182f609b0
020f1296: lea    rcx, [rip + 0x1c0f0bb]                   ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020f129d: call   0x182f609b0
020f12a2: lea    rcx, [rip + 0x1be8447]                   ; [0x3cd96f0] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ˁʼʲʽʲˁʸʳʼʹʵ()
020f12a9: call   0x182f609b0
020f12ae: lea    rcx, [rip + 0x1c14233]                   ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f12b5: call   0x182f609b0
020f12ba: mov    byte ptr [rip + 0x1e280ab], 1            ; [0x3f1936c] (bss)
020f12c1: xor    edi, edi
020f12c3: mov    qword ptr [rsp + 0x30], rbx
020f12c8: xor    eax, eax
020f12ca: test   rsi, rsi
020f12cd: je     0x1820f140c
020f12d3: cmp    eax, dword ptr [rsi + 0x18]
020f12d6: jge    0x1820f1351
020f12d8: mov    r8, qword ptr [rip + 0x1be5fc1]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f12df: mov    edx, edi
020f12e1: mov    rcx, rsi
020f12e4: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f12e9: mov    rbx, rax
020f12ec: test   rax, rax
020f12ef: je     0x1820f140c
020f12f5: cmp    word ptr [rax + 0x80], 1
020f12fd: jne    0x1820f134b
020f12ff: cmp    qword ptr [rax + 0x10], 0
020f1304: je     0x1820f131b
020f1306: mov    rcx, qword ptr [rax + 0x10]
020f130a: mov    rdx, qword ptr [rax + 0x18]
020f130e: mov    qword ptr [rcx + 0x18], rdx
020f1312: add    rcx, 0x18
020f1316: call   0x182f5fc00
020f131b: cmp    qword ptr [rbx + 0x18], 0
020f1320: je     0x1820f1337
020f1322: mov    rcx, qword ptr [rbx + 0x18]
020f1326: mov    rdx, qword ptr [rbx + 0x10]
020f132a: mov    qword ptr [rcx + 0x10], rdx
020f132e: add    rcx, 0x10
020f1332: call   0x182f5fc00
020f1337: mov    r9, qword ptr [rip + 0x1be6022]          ; [0x3cd7360] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.set_Item()
020f133e: xor    r8d, r8d
020f1341: mov    edx, edi
020f1343: mov    rcx, rsi
020f1346: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
020f134b: inc    edi
020f134d: mov    eax, edi
020f134f: jmp    0x1820f12d3
020f1351: mov    rcx, qword ptr [rip + 0x1c14190]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f1358: cmp    dword ptr [rcx + 0xe0], 0
020f135f: jne    0x1820f136d
020f1361: call   0x182f60cf0
020f1366: mov    rcx, qword ptr [rip + 0x1c1417b]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f136d: mov    rax, qword ptr [rcx + 0xb8]
020f1374: mov    rdi, qword ptr [rax + 0x10]
020f1378: test   rdi, rdi
020f137b: jne    0x1820f13eb
020f137d: cmp    dword ptr [rcx + 0xe0], edi
020f1383: jne    0x1820f1391
020f1385: call   0x182f60cf0
020f138a: mov    rcx, qword ptr [rip + 0x1c14157]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f1391: mov    rax, qword ptr [rcx + 0xb8]
020f1398: mov    rcx, qword ptr [rip + 0x1c0efb9]         ; [0x3d00358] meta:System.Predicate<ʽʿʸʸʾʶʶʾʶʹʲ>_TypeInfo
020f139f: mov    rbx, qword ptr [rax]
020f13a2: call   0x182f60c00
020f13a7: mov    r8, qword ptr [rip + 0x1be8342]          ; [0x3cd96f0] metamethod:Method$ʽʶʸʽʳʼʵʸʺʸʳ.<>c.ˁʼʲʽʲˁʸʳʼʹʵ()
020f13ae: xor    r9d, r9d
020f13b1: mov    rdx, rbx
020f13b4: mov    rcx, rax
020f13b7: mov    rdi, rax
020f13ba: call   0x1808ae670                              ; Grpc.Core.VerifyPeerCallback$$.ctor
020f13bf: mov    rax, qword ptr [rip + 0x1c14122]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f13c6: mov    rdx, rdi
020f13c9: mov    rcx, qword ptr [rax + 0xb8]
020f13d0: mov    qword ptr [rcx + 0x10], rdi
020f13d4: mov    rax, qword ptr [rip + 0x1c1410d]         ; [0x3d054e8] meta:ʽʶʸʽʳʼʵʸʺʸʳ.<>c_TypeInfo
020f13db: mov    rcx, qword ptr [rax + 0xb8]
020f13e2: add    rcx, 0x10
020f13e6: call   0x182f5fc00
020f13eb: mov    r8, qword ptr [rip + 0x1be5c6e]          ; [0x3cd7060] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.RemoveAll()
020f13f2: mov    rdx, rdi
020f13f5: mov    rcx, rsi
020f13f8: mov    rbx, qword ptr [rsp + 0x30]
020f13fd: mov    rsi, qword ptr [rsp + 0x38]
020f1402: add    rsp, 0x20
020f1406: pop    rdi
020f1407: jmp    0x1807747e0                              ; System.Collections.Generic.List<object>$$RemoveAll
020f140c: call   0x182f60c50
020f1411: int3   
020f1412: int3   
