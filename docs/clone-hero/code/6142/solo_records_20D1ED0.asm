; function 0x20d1ed0 ʲʽʶʳʺʹʳˀʾʴʹ$$ʸʷʲʵʸʾʾʼʿʼʶ next method starts at 0x20d2440 bytes 0x570
020d1ed0: mov    qword ptr [rsp + 0x10], rdx
020d1ed5: push   rbx
020d1ed6: push   rsi
020d1ed7: push   rdi
020d1ed8: push   r12
020d1eda: push   r13
020d1edc: push   r14
020d1ede: push   r15
020d1ee0: sub    rsp, 0xd0
020d1ee7: mov    r14, rdx
020d1eea: mov    rbx, rcx
020d1eed: cmp    byte ptr [rip + 0x1e47420], 0            ; [0x3f19314] (bss)
020d1ef4: jne    0x1820d1fb5
020d1efa: lea    rcx, [rip + 0x1bee8a7]                   ; [0x3cc07a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.Dispose()
020d1f01: call   0x182f609b0
020d1f06: lea    rcx, [rip + 0x1bee95b]                   ; [0x3cc0868] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.MoveNext()
020d1f0d: call   0x182f609b0
020d1f12: lea    rcx, [rip + 0x1beea0f]                   ; [0x3cc0928] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.get_Current()
020d1f19: call   0x182f609b0
020d1f1e: lea    rcx, [rip + 0x1c2bdab]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020d1f25: call   0x182f609b0
020d1f2a: lea    rcx, [rip + 0x1bfb9c7]                   ; [0x3ccd8f8] meta:System.Collections.Generic.IEnumerable<ValueTuple<int, ʽʿʸʸʾʶʶʾʶʹʲ>>_TypeInfo
020d1f31: call   0x182f609b0
020d1f36: lea    rcx, [rip + 0x1c01e33]                   ; [0x3cd3d70] meta:System.Collections.Generic.IEnumerator<ValueTuple<int, ʽʿʸʸʾʶʶʾʶʹʲ>>_TypeInfo
020d1f3d: call   0x182f609b0
020d1f42: lea    rcx, [rip + 0x1c2d2b7]                   ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
020d1f49: call   0x182f609b0
020d1f4e: lea    rcx, [rip + 0x1c0624b]                   ; [0x3cd81a0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.Add()
020d1f55: call   0x182f609b0
020d1f5a: lea    rcx, [rip + 0x1bffadf]                   ; [0x3cd1a40] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.GetEnumerator()
020d1f61: call   0x182f609b0
020d1f66: lea    rcx, [rip + 0x1c06173]                   ; [0x3cd80e0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>..ctor()
020d1f6d: call   0x182f609b0
020d1f72: lea    rcx, [rip + 0x1c05267]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d1f79: call   0x182f609b0
020d1f7e: lea    rcx, [rip + 0x1c0531b]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d1f85: call   0x182f609b0
020d1f8a: lea    rcx, [rip + 0x1c2402f]                   ; [0x3cf5fc0] meta:System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>_TypeInfo
020d1f91: call   0x182f609b0
020d1f96: lea    rcx, [rip + 0x1bc6803]                   ; [0x3c987a0] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʶʽˀʾˀʻʺʿʲʸʳ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d1f9d: call   0x182f609b0
020d1fa2: lea    rcx, [rip + 0x1bcc117]                   ; [0x3c9e0c0] meta:ʿʶˁˀˁʻʲʷˀʼʲ_TypeInfo
020d1fa9: call   0x182f609b0
020d1fae: mov    byte ptr [rip + 0x1e4735f], 1            ; [0x3f19314] (bss)
020d1fb5: xorps  xmm0, xmm0
020d1fb8: xor    eax, eax
020d1fba: movups xmmword ptr [rsp + 0x80], xmm0
020d1fc2: mov    qword ptr [rsp + 0x90], rax
020d1fca: mov    qword ptr [rsp + 0x38], rax
020d1fcf: test   rbx, rbx
020d1fd2: je     0x1820d2404
020d1fd8: movzx  eax, byte ptr [rbx + 0x58]
020d1fdc: mov    rbx, qword ptr [rbx + 0x78]
020d1fe0: cmp    al, 6
020d1fe2: jne    0x1820d1feb
020d1fe4: mov    edi, 1
020d1fe9: jmp    0x1820d1ff3
020d1feb: xor    edi, edi
020d1fed: cmp    al, 9
020d1fef: sete   dil
020d1ff3: mov    dword ptr [rsp + 0x128], edi
020d1ffa: mov    rcx, qword ptr [rip + 0x1c23fbf]         ; [0x3cf5fc0] meta:System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>_TypeInfo
020d2001: call   0x182f60c00
020d2006: mov    rsi, rax
020d2009: mov    qword ptr [rsp + 0x40], rax
020d200e: mov    rdx, qword ptr [rip + 0x1c060cb]         ; [0x3cd80e0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>..ctor()
020d2015: mov    rcx, rax
020d2018: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d201d: test   r14, r14
020d2020: je     0x1820d2404
020d2026: cmp    dword ptr [r14 + 0x18], 0
020d202b: je     0x1820d23ee
020d2031: mov    r8, qword ptr [rip + 0x1c05268]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2038: xor    edx, edx
020d203a: mov    rcx, r14
020d203d: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2042: test   rax, rax
020d2045: je     0x1820d2404
020d204b: mov    r15, qword ptr [rax + 0x40]
020d204f: mov    qword ptr [rsp + 0x48], r15
020d2054: mov    edx, dword ptr [r14 + 0x18]
020d2058: dec    edx
020d205a: mov    r8, qword ptr [rip + 0x1c0523f]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020d2061: mov    rcx, r14
020d2064: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d2069: test   rax, rax
020d206c: je     0x1820d2404
020d2072: mov    qword ptr [rsp + 0x58], rsi
020d2077: test   rbx, rbx
020d207a: je     0x1820d2404
020d2080: mov    byte ptr [rsp + 0x110], dil
020d2088: xor    r12d, r12d
020d208b: mov    dword ptr [rsp + 0x30], r12d
020d2090: mov    qword ptr [rsp + 0x98], r15
020d2098: mov    rdi, qword ptr [rax + 0x48]
020d209c: add    rdi, qword ptr [rax + 0x40]
020d20a0: mov    qword ptr [rsp + 0x50], rdi
020d20a5: mov    r8, qword ptr [rip + 0x1bff994]          ; [0x3cd1a40] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.GetEnumerator()
020d20ac: mov    rdx, rbx
020d20af: lea    rcx, [rsp + 0xa0]
020d20b7: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d20bc: movups xmm0, xmmword ptr [rsp + 0xa0]
020d20c4: movups xmmword ptr [rsp + 0x80], xmm0
020d20cc: movsd  xmm1, qword ptr [rsp + 0xb0]
020d20d5: movsd  qword ptr [rsp + 0x90], xmm1
020d20de: mov    qword ptr [rsp + 0x60], r12
020d20e3: lea    rax, [rsp + 0x80]
020d20eb: mov    qword ptr [rsp + 0x68], rax
020d20f0: mov    rdx, qword ptr [rip + 0x1bee771]         ; [0x3cc0868] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.MoveNext()
020d20f7: lea    rcx, [rsp + 0x80]
020d20ff: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d2104: test   al, al
020d2106: je     0x1820d23bb
020d210c: mov    rdx, qword ptr [rsp + 0x90]
020d2114: test   rdx, rdx
020d2117: je     0x1820d2434
020d211d: mov    r8, qword ptr [rdx + 0x18]
020d2121: mov    rax, qword ptr [rdx + 0x10]
020d2125: add    r8, rax
020d2128: cmp    r8, r15
020d212b: jl     0x1820d20f0
020d212d: cmp    rax, rdi
020d2130: jg     0x1820d20f0
020d2132: mov    rax, qword ptr [rip + 0x1bc6667]         ; [0x3c987a0] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʶʽˀʾˀʻʺʿʲʸʳ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d2139: mov    qword ptr [rsp + 0x20], rax
020d213e: mov    r9d, r12d
020d2141: mov    rdx, qword ptr [rdx + 0x10]
020d2145: mov    rcx, r14
020d2148: call   0x1805dde30                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ʶʽˀʾˀʻʺʿʲʸʳ<object>
020d214d: test   rax, rax
020d2150: je     0x1820d242f
020d2156: xor    r14d, r14d
020d2159: xor    r15b, r15b
020d215c: xor    r13d, r13d
020d215f: xor    ecx, ecx
020d2161: mov    r8, rax
020d2164: mov    rdx, qword ptr [rip + 0x1bfb78d]         ; [0x3ccd8f8] meta:System.Collections.Generic.IEnumerable<ValueTuple<int, ʽʿʸʸʾʶʶʾʶʹʲ>>_TypeInfo
020d216b: call   0x182c44210
020d2170: mov    qword ptr [rsp + 0x38], rax
020d2175: mov    qword ptr [rsp + 0x70], r13
020d217a: lea    rbx, [rsp + 0x38]
020d217f: mov    qword ptr [rsp + 0x78], rbx
020d2184: nop    dword ptr [rax]
020d2188: nop    dword ptr [rax + rax]
020d2190: mov    r8, qword ptr [rsp + 0x38]
020d2195: test   r8, r8
020d2198: je     0x1820d2424
020d219e: xor    ecx, ecx
020d21a0: mov    rdx, qword ptr [rip + 0x1c2d059]         ; [0x3cff200] meta:System.Collections.IEnumerator_TypeInfo
020d21a7: call   0x182c44210
020d21ac: test   al, al
020d21ae: je     0x1820d2321
020d21b4: mov    r9, qword ptr [rsp + 0x38]
020d21b9: test   r9, r9
020d21bc: je     0x1820d241f
020d21c2: xor    edx, edx
020d21c4: mov    r8, qword ptr [rip + 0x1c01ba5]          ; [0x3cd3d70] meta:System.Collections.Generic.IEnumerator<ValueTuple<int, ʽʿʸʸʾʶʶʾʶʹʲ>>_TypeInfo
020d21cb: lea    rcx, [rsp + 0xa0]
020d21d3: call   0x182c455f0
020d21d8: mov    rdi, qword ptr [rax + 8]
020d21dc: test   rdi, rdi
020d21df: je     0x1820d241a
020d21e5: mov    esi, dword ptr [rax]
020d21e7: xor    edx, edx
020d21e9: mov    rcx, rdi
020d21ec: call   0x18214b630                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʽʻʻʶˁʼʾˀʽʽ
020d21f1: test   al, al
020d21f3: je     0x1820d2202
020d21f5: test   r15b, r15b
020d21f8: jne    0x1820d2207
020d21fa: mov    r15b, 1
020d21fd: mov    r13, rdi
020d2200: jmp    0x1820d2207
020d2202: test   r15b, r15b
020d2205: je     0x1820d2190
020d2207: mov    r12d, esi
020d220a: mov    dword ptr [rsp + 0x30], esi
020d220e: xor    edx, edx
020d2210: mov    rcx, rdi
020d2213: call   0x18214bcc0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿˀʼʷʲʸʶʾˀʷʹ
020d2218: lea    esi, [r14 + 1]
020d221c: test   al, al
020d221e: cmovne esi, r14d
020d2222: cmp    byte ptr [rsp + 0x128], 0
020d222a: je     0x1820d2249
020d222c: xor    edx, edx
020d222e: mov    rcx, rdi
020d2231: call   0x18214ba20                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽˀʺˁʵʸʲʴʺʽʼ
020d2236: cmp    eax, 1
020d2239: je     0x1820d2249
020d223b: mov    r14d, esi
020d223e: cmp    qword ptr [rdi + 0x18], 0
020d2243: jne    0x1820d2190
020d2249: xor    edx, edx
020d224b: mov    rcx, rdi
020d224e: call   0x18214b7d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʸʹʲʳʻʾʺʷʲˁʶ
020d2253: mov    r14d, esi
020d2256: test   al, al
020d2258: je     0x1820d2190
020d225e: mov    rcx, qword ptr [rip + 0x1bcbe5b]         ; [0x3c9e0c0] meta:ʿʶˁˀˁʻʲʷˀʼʲ_TypeInfo
020d2265: call   0x182f60c00
020d226a: mov    r14, rax
020d226d: xor    edx, edx
020d226f: mov    rcx, rax
020d2272: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d2277: test   r13, r13
020d227a: je     0x1820d2415
020d2280: test   r14, r14
020d2283: je     0x1820d2410
020d2289: mov    rax, qword ptr [r13 + 0x28]
020d228d: mov    qword ptr [r14 + 0x18], rax
020d2291: movss  xmm0, dword ptr [rdi + 0x38]
020d2296: cvtps2pd xmm0, xmm0
020d2299: addsd  xmm0, qword ptr [rdi + 0x28]
020d229e: subsd  xmm0, qword ptr [r13 + 0x28]
020d22a4: cvtpd2ps xmm0, xmm0
020d22a8: movss  dword ptr [r14 + 0x20], xmm0
020d22ae: mov    dword ptr [r14 + 0x10], esi
020d22b2: mov    r8, qword ptr [r13 + 0x40]
020d22b6: mov    qword ptr [r14 + 0x28], r8
020d22ba: mov    rdx, qword ptr [rdi + 0x48]
020d22be: sub    rdx, qword ptr [r13 + 0x40]
020d22c2: add    rdx, qword ptr [rdi + 0x40]
020d22c6: mov    qword ptr [r14 + 0x38], rdx
020d22ca: lea    rax, [rdx + r8]
020d22ce: mov    qword ptr [r14 + 0x30], rax
020d22d2: mov    rsi, qword ptr [rsp + 0x40]
020d22d7: test   rsi, rsi
020d22da: je     0x1820d240a
020d22e0: mov    r8, qword ptr [rip + 0x1c05eb9]          ; [0x3cd81a0] metamethod:Method$System.Collections.Generic.List<ʿʶˁˀˁʻʲʷˀʼʲ>.Add()
020d22e7: mov    rdx, r14
020d22ea: mov    rcx, rsi
020d22ed: call   0x182c425f0
020d22f2: nop    
020d22f3: cmp    qword ptr [rbx], 0
020d22f7: je     0x1820d230a
020d22f9: xor    ecx, ecx
020d22fb: mov    r8, qword ptr [rbx]
020d22fe: mov    rdx, qword ptr [rip + 0x1c2b9cb]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020d2305: call   0x182c42550
020d230a: mov    r14, qword ptr [rsp + 0x118]
020d2312: mov    rdi, qword ptr [rsp + 0x50]
020d2317: mov    r15, qword ptr [rsp + 0x48]
020d231c: jmp    0x1820d20f0
020d2321: cmp    qword ptr [rbx], 0
020d2325: je     0x1820d2338
020d2327: xor    ecx, ecx
020d2329: mov    r8, qword ptr [rbx]
020d232c: mov    rdx, qword ptr [rip + 0x1c2b99d]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020d2333: call   0x182c42550
020d2338: mov    rsi, qword ptr [rsp + 0x40]
020d233d: mov    r14, qword ptr [rsp + 0x118]
020d2345: mov    rdi, qword ptr [rsp + 0x50]
020d234a: mov    r15, qword ptr [rsp + 0x48]
020d234f: jmp    0x1820d20f0
020d2354: mov    r8, qword ptr [rsp + 0x78]
020d2359: cmp    qword ptr [r8], 0
020d235d: je     0x1820d2370
020d235f: xor    ecx, ecx
020d2361: mov    r8, qword ptr [r8]
020d2364: mov    rdx, qword ptr [rip + 0x1c2b965]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
020d236b: call   0x182c42550
020d2370: mov    rcx, qword ptr [rsp + 0x70]
020d2375: test   rcx, rcx
020d2378: jne    0x1820d242a
020d237e: movzx  eax, byte ptr [rsp + 0x110]
020d2386: mov    dword ptr [rsp + 0x128], eax
020d238d: mov    rsi, qword ptr [rsp + 0x58]
020d2392: mov    qword ptr [rsp + 0x40], rsi
020d2397: mov    r12d, dword ptr [rsp + 0x30]
020d239c: mov    r15, qword ptr [rsp + 0x98]
020d23a4: mov    qword ptr [rsp + 0x48], r15
020d23a9: mov    rdi, qword ptr [rsp + 0x50]
020d23ae: mov    r14, qword ptr [rsp + 0x118]
020d23b6: jmp    0x1820d20f0
020d23bb: mov    rdx, qword ptr [rip + 0x1bee3e6]         ; [0x3cc07a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.Dispose()
020d23c2: mov    rcx, qword ptr [rsp + 0x68]
020d23c7: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d23cc: jmp    0x1820d23ee
020d23ce: mov    rdx, qword ptr [rip + 0x1bee3d3]         ; [0x3cc07a8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʸʵʵʾʿˀʺʽʲʾˁ>.Dispose()
020d23d5: mov    rcx, qword ptr [rsp + 0x68]
020d23da: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d23df: mov    rcx, qword ptr [rsp + 0x60]
020d23e4: test   rcx, rcx
020d23e7: jne    0x1820d243a
020d23e9: mov    rsi, qword ptr [rsp + 0x58]
020d23ee: mov    rax, rsi
020d23f1: add    rsp, 0xd0
020d23f8: pop    r15
020d23fa: pop    r14
020d23fc: pop    r13
020d23fe: pop    r12
020d2400: pop    rdi
020d2401: pop    rsi
020d2402: pop    rbx
020d2403: ret    
020d2404: call   0x182f60c50
020d2409: nop    
020d240a: call   0x182f60c50
020d240f: nop    
020d2410: call   0x182f60c50
020d2415: call   0x182f60c50
020d241a: call   0x182f60c50
020d241f: call   0x182f60c50
020d2424: call   0x182f60c50
020d2429: nop    
020d242a: call   0x182f60ce0
020d242f: call   0x182f60c50
020d2434: call   0x182f60c50
020d2439: nop    
020d243a: call   0x182f60ce0
020d243f: int3   
