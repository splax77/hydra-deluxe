020d12c0: mov    rax, rsp
020d12c3: mov    qword ptr [rax + 8], rbx
020d12c7: mov    qword ptr [rax + 0x18], rsi
020d12cb: mov    qword ptr [rax + 0x10], rdx
020d12cf: push   rdi
020d12d0: push   r12
020d12d2: push   r13
020d12d4: push   r14
020d12d6: push   r15
020d12d8: sub    rsp, 0xb0
020d12df: movaps xmmword ptr [rax - 0x38], xmm6
020d12e3: movaps xmmword ptr [rax - 0x48], xmm7
020d12e7: mov    rdi, rdx
020d12ea: mov    r14, rcx
020d12ed: cmp    byte ptr [rip + 0x1e4801e], 0            ; [0x3f19312] (bss)
020d12f4: jne    0x1820d13b5
020d12fa: lea    rcx, [rip + 0x1bee2af]                   ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020d1301: call   0x182f609b0
020d1306: lea    rcx, [rip + 0x1bee363]                   ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
020d130d: call   0x182f609b0
020d1312: lea    rcx, [rip + 0x1bee417]                   ; [0x3cbf730] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Current()
020d1319: call   0x182f609b0
020d131e: lea    rcx, [rip + 0x1c066fb]                   ; [0x3cd7a20] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>.Add()
020d1325: call   0x182f609b0
020d132a: lea    rcx, [rip + 0x1bfd04f]                   ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
020d1331: call   0x182f609b0
020d1336: lea    rcx, [rip + 0x1c06623]                   ; [0x3cd7960] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>..ctor()
020d133d: call   0x182f609b0
020d1342: lea    rcx, [rip + 0x1c05e97]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d1349: call   0x182f609b0
020d134e: lea    rcx, [rip + 0x1bfd1ab]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
020d1355: call   0x182f609b0
020d135a: lea    rcx, [rip + 0x1c05f3f]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1361: call   0x182f609b0
020d1366: lea    rcx, [rip + 0x1c24b9b]                   ; [0x3cf5f08] meta:System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>_TypeInfo
020d136d: call   0x182f609b0
020d1372: lea    rcx, [rip + 0x1bc1bff]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020d1379: call   0x182f609b0
020d137e: lea    rcx, [rip + 0x1bc770b]                   ; [0x3c98a90] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ˁʴʵʺʷʲʲʾʻʿʿ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d1385: call   0x182f609b0
020d138a: lea    rcx, [rip + 0x1bc77bf]                   ; [0x3c98b50] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ˁˁʳʳˁʸʲʾʴʶʹ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d1391: call   0x182f609b0
020d1396: lea    rcx, [rip + 0x1bcc4b3]                   ; [0x3c9d850] meta:ʾʻʹʶʹʺʸʺʺʶʹ_TypeInfo
020d139d: call   0x182f609b0
020d13a2: lea    rcx, [rip + 0x1bcfcc7]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020d13a9: call   0x182f609b0
020d13ae: mov    byte ptr [rip + 0x1e47f5d], 1            ; [0x3f19312] (bss)
020d13b5: xorps  xmm0, xmm0
020d13b8: xor    eax, eax
020d13ba: movups xmmword ptr [rsp + 0x58], xmm0
020d13bf: mov    qword ptr [rsp + 0x68], rax
020d13c4: test   rdi, rdi
020d13c7: je     0x1820d188d
020d13cd: mov    rbx, qword ptr [rdi + 0x80]
020d13d4: test   rbx, rbx
020d13d7: je     0x1820d188d
020d13dd: mov    ebx, dword ptr [rbx + 0x18]
020d13e0: mov    rcx, qword ptr [rip + 0x1c24b21]         ; [0x3cf5f08] meta:System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>_TypeInfo
020d13e7: call   0x182f60c00
020d13ec: mov    rsi, rax
020d13ef: mov    qword ptr [rsp + 0x30], rax
020d13f4: mov    r8, qword ptr [rip + 0x1c06565]          ; [0x3cd7960] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>..ctor()
020d13fb: mov    edx, ebx
020d13fd: mov    rcx, rax
020d1400: call   0x180706bc0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$.ctor
020d1405: test   r14, r14
020d1408: je     0x1820d188d
020d140e: cmp    dword ptr [r14 + 0x18], 0
020d1413: je     0x1820d1863
020d1419: mov    rax, qword ptr [rdi + 0x50]
020d141d: test   rax, rax
020d1420: je     0x1820d188d
020d1426: mov    qword ptr [rsp + 0x78], rsi
020d142b: movsd  xmm6, qword ptr [rax + 0xb8]
020d1433: mov    rax, qword ptr [rip + 0x1bcfc36]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020d143a: cmp    dword ptr [rax + 0xe0], 0
020d1441: jne    0x1820d1452
020d1443: mov    rcx, rax
020d1446: call   0x182f60cf0
020d144b: mov    rax, qword ptr [rip + 0x1bcfc1e]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020d1452: mov    rax, qword ptr [rax + 0xb8]
020d1459: movsd  xmm7, qword ptr [rax + 0x68]
020d145e: mov    rcx, qword ptr [rip + 0x1bc1b13]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020d1465: cmp    dword ptr [rcx + 0xe0], 0
020d146c: jne    0x1820d1473
020d146e: call   0x182f60cf0
020d1473: mulsd  xmm6, qword ptr [rip + 0x106f275]        ; [0x31406f0] dbl=12.0 q=0x4028000000000000
020d147b: divsd  xmm6, xmm7
020d147f: cmp    byte ptr [rip + 0x1e47ea1], 0            ; [0x3f19327] (bss)
020d1486: jne    0x1820d149b
020d1488: lea    rcx, [rip + 0x1bd12d9]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020d148f: call   0x182f609b0
020d1494: mov    byte ptr [rip + 0x1e47e8c], 1            ; [0x3f19327] (bss)
020d149b: mov    rcx, qword ptr [rip + 0x1bd12c6]         ; [0x3ca2768] meta:System.Math_TypeInfo
020d14a2: cmp    dword ptr [rcx + 0xe0], 0
020d14a9: jne    0x1820d14b0
020d14ab: call   0x182f60cf0
020d14b0: movaps xmm0, xmm6
020d14b3: call   0x182c49a60
020d14b8: mov    rdx, qword ptr [rdi + 0x80]
020d14bf: test   rdx, rdx
020d14c2: je     0x1820d188d
020d14c8: cvttsd2si r12, xmm0
020d14cd: mov    qword ptr [rsp + 0xf8], r12
020d14d5: mov    r8, qword ptr [rip + 0x1bfcea4]          ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
020d14dc: lea    rcx, [rsp + 0x40]
020d14e1: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d14e6: movups xmm0, xmmword ptr [rsp + 0x40]
020d14eb: movups xmmword ptr [rsp + 0x58], xmm0
020d14f0: movsd  xmm1, qword ptr [rsp + 0x50]
020d14f6: movsd  qword ptr [rsp + 0x68], xmm1
020d14fc: mov    qword ptr [rsp + 0x40], 0
020d1505: lea    rbx, [rsp + 0x58]
020d150a: mov    qword ptr [rsp + 0x48], rbx
020d150f: jmp    0x1820d151e
020d1511: mov    r12, qword ptr [rsp + 0xf8]
020d1519: mov    rsi, qword ptr [rsp + 0x30]
020d151e: mov    rdx, qword ptr [rip + 0x1bee14b]         ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
020d1525: lea    rcx, [rsp + 0x58]
020d152a: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d152f: test   al, al
020d1531: je     0x1820d1832
020d1537: mov    r13, qword ptr [rsp + 0x68]
020d153c: test   r13, r13
020d153f: je     0x1820d18d5
020d1545: mov    rdi, qword ptr [r13 + 0x10]
020d1549: mov    rax, qword ptr [rip + 0x1bc7540]         ; [0x3c98a90] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ˁʴʵʺʷʲʲʾʻʿʿ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d1550: mov    r8d, 0xfe
020d1556: lea    rdx, [rdi - 1]
020d155a: mov    qword ptr [rsp + 0x20], rax
020d155f: xor    r9d, r9d
020d1562: mov    rcx, r14
020d1565: call   0x1805de790                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ˁʴʵʺʷʲʲʾʻʿʿ<object>
020d156a: mov    esi, eax
020d156c: cmp    eax, -1
020d156f: je     0x1820d1519
020d1571: mov    r8, qword ptr [rip + 0x1c05d28]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1578: mov    edx, eax
020d157a: mov    rcx, r14
020d157d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1582: mov    r15, rax
020d1585: mov    qword ptr [rsp + 0x70], rax
020d158a: test   rax, rax
020d158d: je     0x1820d18d0
020d1593: lea    rax, [rdi + r12]
020d1597: cmp    qword ptr [r15 + 0x40], rax
020d159b: jg     0x1820d1519
020d15a1: xor    r8d, r8d
020d15a4: mov    edx, 0xfe
020d15a9: mov    rcx, r15
020d15ac: call   0x1820d5310                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ˁʻʲʸʼʾʺʳʷʷʶ
020d15b1: test   rax, rax
020d15b4: je     0x1820d18cb
020d15ba: movzx  r12d, word ptr [rax + 0x80]
020d15c2: cmp    byte ptr [r13 + 0x21], 0
020d15c7: je     0x1820d1623
020d15c9: mov    r8d, 0xfe
020d15cf: mov    rax, qword ptr [rip + 0x1bc74ba]         ; [0x3c98a90] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ˁʴʵʺʷʲʲʾʻʿʿ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d15d6: mov    qword ptr [rsp + 0x20], rax
020d15db: mov    r9d, esi
020d15de: mov    rdx, qword ptr [r15 + 0x40]
020d15e2: mov    rcx, r14
020d15e5: call   0x1805de790                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ˁʴʵʺʷʲʲʾʻʿʿ<object>
020d15ea: mov    r8, qword ptr [rip + 0x1c05caf]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d15f1: mov    edx, eax
020d15f3: mov    rcx, r14
020d15f6: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d15fb: mov    qword ptr [rsp + 0x38], rax
020d1600: xor    r8d, r8d
020d1603: mov    edx, 0xfe
020d1608: mov    rcx, rax
020d160b: call   0x1820d51d0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ˀʶˀʾʴʳʹʶˁʹʹ
020d1610: test   rax, rax
020d1613: je     0x1820d1893
020d1619: or     r12w, word ptr [rax + 0x80]
020d1621: jmp    0x1820d162a
020d1623: xor    edx, edx
020d1625: mov    qword ptr [rsp + 0x38], rdx
020d162a: mov    rcx, qword ptr [r13 + 0x18]
020d162e: mov    rdx, qword ptr [r13 + 0x10]
020d1632: inc    rcx
020d1635: add    rdx, rcx
020d1638: mov    r9, qword ptr [rip + 0x1bc7511]          ; [0x3c98b50] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ˁˁʳʳˁʸʲʾʴʶʹ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d163f: mov    r8d, esi
020d1642: mov    rcx, r14
020d1645: call   0x1805deb40                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ˁˁʳʳˁʸʲʾʴʶʹ<object>
020d164a: mov    r15d, eax
020d164d: cmp    eax, -1
020d1650: jne    0x1820d1659
020d1652: mov    r15d, dword ptr [r14 + 0x18]
020d1656: dec    r15d
020d1659: cmp    esi, r15d
020d165c: je     0x1820d1511
020d1662: mov    edi, esi
020d1664: nop    dword ptr [rax]
020d1668: nop    dword ptr [rax + rax]
020d1670: mov    r8, qword ptr [rip + 0x1c05c29]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1677: mov    rcx, r14
020d167a: cmp    edi, r15d
020d167d: jg     0x1820d16e8
020d167f: mov    edx, edi
020d1681: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1686: test   rax, rax
020d1689: je     0x1820d18a3
020d168f: test   word ptr [rax + 0x80], r12w
020d1697: je     0x1820d16e4
020d1699: cmp    byte ptr [r13 + 0x21], 0
020d169e: je     0x1820d16c1
020d16a0: mov    r8, qword ptr [rip + 0x1c05bf9]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d16a7: mov    edx, edi
020d16a9: mov    rcx, r14
020d16ac: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d16b1: test   rax, rax
020d16b4: je     0x1820d1899
020d16ba: or     dword ptr [rax + 0x7c], 0x8000
020d16c1: mov    r8, qword ptr [rip + 0x1c05bd8]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d16c8: mov    edx, edi
020d16ca: mov    rcx, r14
020d16cd: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d16d2: test   rax, rax
020d16d5: je     0x1820d189e
020d16db: or     dword ptr [rax + 0x7c], 0x4000
020d16e2: mov    esi, edi
020d16e4: inc    edi
020d16e6: jmp    0x1820d1670
020d16e8: mov    edx, esi
020d16ea: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d16ef: test   rax, rax
020d16f2: je     0x1820d18c6
020d16f8: or     dword ptr [rax + 0x7c], 0x10000
020d16ff: mov    rcx, qword ptr [rip + 0x1bcc14a]         ; [0x3c9d850] meta:ʾʻʹʶʹʺʸʺʺʶʹ_TypeInfo
020d1706: call   0x182f60c00
020d170b: mov    rdi, rax
020d170e: xor    edx, edx
020d1710: mov    rcx, rax
020d1713: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d1718: mov    r15, qword ptr [rsp + 0xe8]
020d1720: mov    rcx, qword ptr [r15 + 0x50]
020d1724: test   rcx, rcx
020d1727: je     0x1820d18c1
020d172d: mov    rdx, qword ptr [r13 + 0x10]
020d1731: sub    rdx, qword ptr [rsp + 0xf8]
020d1739: xor    r8d, r8d
020d173c: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d1741: test   rdi, rdi
020d1744: je     0x1820d18bc
020d174a: movsd  qword ptr [rdi + 0x10], xmm0
020d174f: mov    r15, qword ptr [r15 + 0x50]
020d1753: mov    r12, qword ptr [r13 + 0x10]
020d1757: mov    r8, qword ptr [rip + 0x1c05b42]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d175e: mov    edx, esi
020d1760: mov    rcx, r14
020d1763: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d1768: test   rax, rax
020d176b: je     0x1820d18b7
020d1771: test   r15, r15
020d1774: je     0x1820d18b2
020d177a: mov    r8, qword ptr [rax + 0x40]
020d177e: add    r8, qword ptr [rsp + 0xf8]
020d1786: sub    r12, qword ptr [rsp + 0xf8]
020d178e: xor    r9d, r9d
020d1791: mov    rdx, r12
020d1794: mov    rcx, r15
020d1797: call   0x1820cfce0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴʸˁʳˀʽʼʺʸʾˁ
020d179c: cvtsd2ss xmm0, xmm0
020d17a0: movss  dword ptr [rdi + 0x18], xmm0
020d17a5: mov    rdx, qword ptr [rsp + 0x70]
020d17aa: mov    qword ptr [rdi + 0x20], rdx
020d17ae: lea    rcx, [rdi + 0x20]
020d17b2: call   0x182f5fc00
020d17b7: mov    rdx, qword ptr [rsp + 0x38]
020d17bc: mov    qword ptr [rdi + 0x28], rdx
020d17c0: lea    rcx, [rdi + 0x28]
020d17c4: call   0x182f5fc00
020d17c9: mov    rax, qword ptr [r13 + 0x10]
020d17cd: mov    qword ptr [rdi + 0x30], rax
020d17d1: mov    r8, qword ptr [rip + 0x1c05ac8]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d17d8: mov    edx, esi
020d17da: mov    rcx, r14
020d17dd: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d17e2: test   rax, rax
020d17e5: je     0x1820d18ad
020d17eb: mov    eax, dword ptr [rax + 0x40]
020d17ee: sub    eax, dword ptr [r13 + 0x10]
020d17f2: mov    r12, qword ptr [rsp + 0xf8]
020d17fa: lea    eax, [rax + r12*2]
020d17fe: movsxd rcx, eax
020d1801: mov    qword ptr [rdi + 0x38], rcx
020d1805: movzx  eax, byte ptr [r13 + 0x21]
020d180a: mov    byte ptr [rdi + 0x40], al
020d180d: mov    rsi, qword ptr [rsp + 0x30]
020d1812: test   rsi, rsi
020d1815: je     0x1820d18a8
020d181b: mov    r8, qword ptr [rip + 0x1c061fe]          ; [0x3cd7a20] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>.Add()
020d1822: mov    rdx, rdi
020d1825: mov    rcx, rsi
020d1828: call   0x182c425f0
020d182d: jmp    0x1820d151e
020d1832: mov    rdx, qword ptr [rip + 0x1bedd77]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020d1839: mov    rcx, rbx
020d183c: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d1841: jmp    0x1820d1863
020d1843: mov    rdx, qword ptr [rip + 0x1bedd66]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
020d184a: mov    rcx, qword ptr [rsp + 0x48]
020d184f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d1854: mov    rcx, qword ptr [rsp + 0x40]
020d1859: test   rcx, rcx
020d185c: jne    0x1820d18db
020d185e: mov    rsi, qword ptr [rsp + 0x78]
020d1863: mov    rax, rsi
020d1866: lea    r11, [rsp + 0xb0]
020d186e: mov    rbx, qword ptr [r11 + 0x30]
020d1872: mov    rsi, qword ptr [r11 + 0x40]
020d1876: movaps xmm6, xmmword ptr [r11 - 0x10]
020d187b: movaps xmm7, xmmword ptr [r11 - 0x20]
020d1880: mov    rsp, r11
020d1883: pop    r15
020d1885: pop    r14
020d1887: pop    r13
020d1889: pop    r12
020d188b: pop    rdi
020d188c: ret    
020d188d: call   0x182f60c50
020d1892: nop    
020d1893: call   0x182f60c50
020d1898: nop    
020d1899: call   0x182f60c50
020d189e: call   0x182f60c50
020d18a3: call   0x182f60c50
020d18a8: call   0x182f60c50
020d18ad: call   0x182f60c50
020d18b2: call   0x182f60c50
020d18b7: call   0x182f60c50
020d18bc: call   0x182f60c50
020d18c1: call   0x182f60c50
020d18c6: call   0x182f60c50
020d18cb: call   0x182f60c50
020d18d0: call   0x182f60c50
020d18d5: call   0x182f60c50
020d18da: nop    
020d18db: call   0x182f60ce0
020d18e0: int3   
020d18e1: int3   
