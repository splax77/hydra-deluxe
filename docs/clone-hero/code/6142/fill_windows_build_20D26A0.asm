020d26a0: mov    qword ptr [rsp + 8], rbx
020d26a5: mov    qword ptr [rsp + 0x18], rsi
020d26aa: push   rdi
020d26ab: push   r12
020d26ad: push   r13
020d26af: push   r14
020d26b1: push   r15
020d26b3: sub    rsp, 0x60
020d26b7: mov    rbx, rdx
020d26ba: mov    r15, rcx
020d26bd: cmp    byte ptr [rip + 0x1e46c46], 0            ; [0x3f1930a] (bss)
020d26c4: jne    0x1820d2739
020d26c6: lea    rcx, [rip + 0x1bec823]                   ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020d26cd: call   0x182f609b0
020d26d2: lea    rcx, [rip + 0x1bec8d7]                   ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
020d26d9: call   0x182f609b0
020d26de: lea    rcx, [rip + 0x1bec98b]                   ; [0x3cbf070] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Current()
020d26e5: call   0x182f609b0
020d26ea: lea    rcx, [rip + 0x1c024cf]                   ; [0x3cd4bc0] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.Add()
020d26f1: call   0x182f609b0
020d26f6: lea    rcx, [rip + 0x1bfaccb]                   ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
020d26fd: call   0x182f609b0
020d2702: lea    rcx, [rip + 0x1c023f7]                   ; [0x3cd4b00] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d2709: call   0x182f609b0
020d270e: lea    rcx, [rip + 0x1c0232b]                   ; [0x3cd4a40] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d2715: call   0x182f609b0
020d271a: lea    rcx, [rip + 0x1c2312f]                   ; [0x3cf5850] meta:System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>_TypeInfo
020d2721: call   0x182f609b0
020d2726: lea    rcx, [rip + 0x1bc8593]                   ; [0x3c9acc0] meta:ʼʶʷʷʴʶʺʽʴʼʽ_TypeInfo
020d272d: call   0x182f609b0
020d2732: mov    byte ptr [rip + 0x1e46bd1], 1            ; [0x3f1930a] (bss)
020d2739: xorps  xmm0, xmm0
020d273c: xor    eax, eax
020d273e: movups xmmword ptr [rsp + 0x40], xmm0
020d2743: mov    qword ptr [rsp + 0x50], rax
020d2748: mov    rcx, qword ptr [rip + 0x1c23101]         ; [0x3cf5850] meta:System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>_TypeInfo
020d274f: call   0x182f60c00
020d2754: mov    rsi, rax
020d2757: mov    r8, qword ptr [rip + 0x1c023a2]          ; [0x3cd4b00] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d275e: mov    edx, 1
020d2763: mov    rcx, rax
020d2766: call   0x180706bc0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$.ctor
020d276b: test   rbx, rbx
020d276e: je     0x1820d290a
020d2774: mov    rdx, qword ptr [rbx + 0x88]
020d277b: test   rdx, rdx
020d277e: je     0x1820d290a
020d2784: mov    qword ptr [rsp + 0x98], rsi
020d278c: mov    r8, qword ptr [rip + 0x1bfac35]          ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
020d2793: lea    rcx, [rsp + 0x28]
020d2798: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d279d: movups xmm0, xmmword ptr [rsp + 0x28]
020d27a2: movups xmmword ptr [rsp + 0x40], xmm0
020d27a7: movsd  xmm1, qword ptr [rsp + 0x38]
020d27ad: movsd  qword ptr [rsp + 0x50], xmm1
020d27b3: mov    qword ptr [rsp + 0x28], 0
020d27bc: lea    rbx, [rsp + 0x40]
020d27c1: mov    qword ptr [rsp + 0x30], rbx
020d27c6: nop    word ptr [rax + rax]
020d27d0: mov    rdx, qword ptr [rip + 0x1bec7d9]         ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
020d27d7: lea    rcx, [rsp + 0x40]
020d27dc: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d27e1: test   al, al
020d27e3: je     0x1820d28b9
020d27e9: mov    rdi, qword ptr [rsp + 0x50]
020d27ee: test   rdi, rdi
020d27f1: je     0x1820d2920
020d27f7: cmp    byte ptr [rdi + 0x20], 0
020d27fb: je     0x1820d27d0
020d27fd: mov    r13, qword ptr [rdi + 0x10]
020d2801: mov    r12, qword ptr [rdi + 0x18]
020d2805: mov    rcx, qword ptr [rip + 0x1bc84b4]         ; [0x3c9acc0] meta:ʼʶʷʷʴʶʺʽʴʼʽ_TypeInfo
020d280c: call   0x182f60c00
020d2811: mov    r14, rax
020d2814: xor    edx, edx
020d2816: mov    rcx, rax
020d2819: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d281e: test   r15, r15
020d2821: je     0x1820d291b
020d2827: xor    r8d, r8d
020d282a: mov    rdx, qword ptr [rdi + 0x10]
020d282e: mov    rcx, r15
020d2831: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d2836: test   r14, r14
020d2839: je     0x1820d2916
020d283f: movsd  qword ptr [r14 + 0x10], xmm0
020d2845: lea    r8, [r12 + r13]
020d2849: xor    r9d, r9d
020d284c: mov    rdx, qword ptr [rdi + 0x10]
020d2850: mov    rcx, r15
020d2853: call   0x1820cfce0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴʸˁʳˀʽʼʺʸʾˁ
020d2858: cvtsd2ss xmm0, xmm0
020d285c: movss  dword ptr [r14 + 0x18], xmm0
020d2862: mov    rax, qword ptr [rdi + 0x10]
020d2866: mov    qword ptr [r14 + 0x20], rax
020d286a: movsxd rax, dword ptr [rdi + 0x18]
020d286e: mov    qword ptr [r14 + 0x28], rax
020d2872: test   rsi, rsi
020d2875: jne    0x1820d28a2
020d2877: mov    rcx, qword ptr [rip + 0x1c22fd2]         ; [0x3cf5850] meta:System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>_TypeInfo
020d287e: call   0x182f60c00
020d2883: mov    rsi, rax
020d2886: mov    rdx, qword ptr [rip + 0x1c021b3]         ; [0x3cd4a40] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d288d: mov    rcx, rax
020d2890: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d2895: mov    qword ptr [rsp + 0x98], rsi
020d289d: test   rsi, rsi
020d28a0: je     0x1820d2910
020d28a2: mov    r8, qword ptr [rip + 0x1c02317]          ; [0x3cd4bc0] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>.Add()
020d28a9: mov    rdx, r14
020d28ac: mov    rcx, rsi
020d28af: call   0x182c425f0
020d28b4: jmp    0x1820d27d0
020d28b9: mov    rdx, qword ptr [rip + 0x1bec630]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020d28c0: mov    rcx, rbx
020d28c3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d28c8: jmp    0x1820d28ed
020d28ca: mov    rdx, qword ptr [rip + 0x1bec61f]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020d28d1: mov    rcx, qword ptr [rsp + 0x30]
020d28d6: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d28db: mov    rcx, qword ptr [rsp + 0x28]
020d28e0: test   rcx, rcx
020d28e3: jne    0x1820d2926
020d28e5: mov    rsi, qword ptr [rsp + 0x98]
020d28ed: mov    rax, rsi
020d28f0: lea    r11, [rsp + 0x60]
020d28f5: mov    rbx, qword ptr [r11 + 0x30]
020d28f9: mov    rsi, qword ptr [r11 + 0x40]
020d28fd: mov    rsp, r11
020d2900: pop    r15
020d2902: pop    r14
020d2904: pop    r13
020d2906: pop    r12
020d2908: pop    rdi
020d2909: ret    
020d290a: call   0x182f60c50
020d290f: nop    
020d2910: call   0x182f60c50
020d2915: nop    
020d2916: call   0x182f60c50
020d291b: call   0x182f60c50
020d2920: call   0x182f60c50
020d2925: nop    
020d2926: call   0x182f60ce0
020d292b: int3   
020d292c: int3   
