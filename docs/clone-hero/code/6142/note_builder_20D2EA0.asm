020d2ea0: mov    byte ptr [rsp + 0x20], r9b
020d2ea5: mov    qword ptr [rsp + 0x18], r8
020d2eaa: mov    qword ptr [rsp + 0x10], rdx
020d2eaf: mov    qword ptr [rsp + 8], rcx
020d2eb4: push   rsi
020d2eb5: push   r14
020d2eb7: push   r15
020d2eb9: sub    rsp, 0x140
020d2ec0: cmp    byte ptr [rip + 0x1e4644e], 0            ; [0x3f19315] (bss)
020d2ec7: mov    r15, r8
020d2eca: mov    r14, rdx
020d2ecd: jne    0x1820d2f5e
020d2ed3: lea    rcx, [rip + 0x1c049c6]                   ; [0x3cd78a0] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>..ctor()
020d2eda: call   0x182f609b0
020d2edf: lea    rcx, [rip + 0x1c01b5a]                   ; [0x3cd4a40] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d2ee6: call   0x182f609b0
020d2eeb: lea    rcx, [rip + 0x1bfb78e]                   ; [0x3cce680] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020d2ef2: call   0x182f609b0
020d2ef7: lea    rcx, [rip + 0x1c042e2]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020d2efe: call   0x182f609b0
020d2f03: lea    rcx, [rip + 0x1bfcaf6]                   ; [0x3ccfa00] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Count()
020d2f0a: call   0x182f609b0
020d2f0f: lea    rcx, [rip + 0x1bfa572]                   ; [0x3ccd488] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Count()
020d2f16: call   0x182f609b0
020d2f1b: lea    rcx, [rip + 0x1c22fe6]                   ; [0x3cf5f08] meta:System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>_TypeInfo
020d2f22: call   0x182f609b0
020d2f27: lea    rcx, [rip + 0x1c217e2]                   ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020d2f2e: call   0x182f609b0
020d2f33: lea    rcx, [rip + 0x1c22916]                   ; [0x3cf5850] meta:System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>_TypeInfo
020d2f3a: call   0x182f609b0
020d2f3f: lea    rcx, [rip + 0x1bc50ea]                   ; [0x3c98030] meta:ʹʷʽʶʶʸʻʿˁʾʵ_TypeInfo
020d2f46: call   0x182f609b0
020d2f4b: lea    rcx, [rip + 0x1c1eb5e]                   ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d2f52: call   0x182f609b0
020d2f57: mov    byte ptr [rip + 0x1e463b7], 1            ; [0x3f19315] (bss)
020d2f5e: movzx  esi, byte ptr [r14]
020d2f62: mov    eax, 6
020d2f67: mov    qword ptr [rsp + 0x138], rbx
020d2f6f: cmp    sil, 9
020d2f73: mov    qword ptr [rsp + 0x130], rbp
020d2f7b: mov    edx, esi
020d2f7d: mov    qword ptr [rsp + 0x128], rdi
020d2f85: cmove  edx, eax
020d2f88: mov    qword ptr [rsp + 0x120], r12
020d2f90: mov    qword ptr [rsp + 0x118], r13
020d2f98: test   r15, r15
020d2f9b: je     0x1820d3690
020d2fa1: movzx  eax, byte ptr [r14 + 1]
020d2fa6: xor    r9d, r9d
020d2fa9: movzx  r12d, byte ptr [r14 + 2]
020d2fae: movzx  r8d, al
020d2fb2: mov    rcx, r15
020d2fb5: mov    byte ptr [rsp + 0xb0], al
020d2fbc: call   0x1820cfd40
020d2fc1: movzx  r9d, r12b
020d2fc5: mov    qword ptr [rsp + 0xf0], rax
020d2fcd: movzx  r8d, sil
020d2fd1: mov    qword ptr [rsp + 0x20], 0
020d2fda: mov    rdx, rax
020d2fdd: mov    rcx, r15
020d2fe0: mov    rbp, rax
020d2fe3: call   0x1820d4be0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʿʹʿʸʹʽʾʻʸʵʵ
020d2fe8: mov    rcx, qword ptr [rip + 0x1bc5041]         ; [0x3c98030] meta:ʹʷʽʶʶʸʻʿˁʾʵ_TypeInfo
020d2fef: mov    rdi, rax
020d2ff2: mov    qword ptr [rsp + 0x100], rax
020d2ffa: call   0x182f60c00
020d2fff: cmp    byte ptr [rip + 0x1e462e5], 0            ; [0x3f192eb] (bss)
020d3006: mov    r13, rax
020d3009: mov    qword ptr [rsp + 0xb8], rax
020d3011: jne    0x1820d3062
020d3013: lea    rcx, [rip + 0x1bcd686]                   ; [0x3ca06a0] metamethod:Method$System.Collections.Generic.List<long>..ctor()
020d301a: call   0x182f609b0
020d301f: lea    rcx, [rip + 0x1bfff1a]                   ; [0x3cd2f40] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>..ctor()
020d3026: call   0x182f609b0
020d302b: lea    rcx, [rip + 0x1c3b22e]                   ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020d3032: call   0x182f609b0
020d3037: lea    rcx, [rip + 0x1c151aa]                   ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020d303e: call   0x182f609b0
020d3043: lea    rcx, [rip + 0x1c22506]                   ; [0x3cf5550] meta:System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>_TypeInfo
020d304a: call   0x182f609b0
020d304f: lea    rcx, [rip + 0x1c1823a]                   ; [0x3ceb290] meta:System.Collections.Generic.List<long>_TypeInfo
020d3056: call   0x182f609b0
020d305b: mov    byte ptr [rip + 0x1e46289], 1            ; [0x3f192eb] (bss)
020d3062: mov    rcx, qword ptr [rip + 0x1c1517f]         ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020d3069: call   0x182f60c00
020d306e: mov    rdx, qword ptr [rip + 0x1c3b1eb]         ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020d3075: mov    rcx, rax
020d3078: mov    rbx, rax
020d307b: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d3080: lea    rcx, [r13 + 0x10]
020d3084: mov    qword ptr [r13 + 0x10], rbx
020d3088: mov    rdx, rbx
020d308b: call   0x182f5fc00
020d3090: mov    rcx, qword ptr [rip + 0x1c224b9]         ; [0x3cf5550] meta:System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>_TypeInfo
020d3097: call   0x182f60c00
020d309c: mov    rdx, qword ptr [rip + 0x1bffe9d]         ; [0x3cd2f40] metamethod:Method$System.Collections.Generic.List<ʹʺʾˀʻʺʼʷʼʹʻ>..ctor()
020d30a3: mov    rcx, rax
020d30a6: mov    rbx, rax
020d30a9: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d30ae: lea    rcx, [r13 + 0x18]
020d30b2: mov    qword ptr [r13 + 0x18], rbx
020d30b6: mov    rdx, rbx
020d30b9: call   0x182f5fc00
020d30be: mov    rcx, qword ptr [rip + 0x1c181cb]         ; [0x3ceb290] meta:System.Collections.Generic.List<long>_TypeInfo
020d30c5: call   0x182f60c00
020d30ca: mov    rdx, qword ptr [rip + 0x1bcd5cf]         ; [0x3ca06a0] metamethod:Method$System.Collections.Generic.List<long>..ctor()
020d30d1: mov    rcx, rax
020d30d4: mov    rbx, rax
020d30d7: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d30dc: lea    rcx, [r13 + 0x20]
020d30e0: mov    qword ptr [r13 + 0x20], rbx
020d30e4: mov    rdx, rbx
020d30e7: call   0x182f5fc00
020d30ec: mov    rcx, qword ptr [rip + 0x1c150f5]         ; [0x3ce81e8] meta:System.Collections.Generic.List<double>_TypeInfo
020d30f3: call   0x182f60c00
020d30f8: mov    rdx, qword ptr [rip + 0x1c3b161]         ; [0x3d0e260] metamethod:Method$System.Collections.Generic.List<double>..ctor()
020d30ff: mov    rcx, rax
020d3102: mov    rbx, rax
020d3105: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d310a: lea    rcx, [r13 + 0x28]
020d310e: mov    qword ptr [r13 + 0x28], rbx
020d3112: mov    rdx, rbx
020d3115: call   0x182f5fc00
020d311a: xor    edx, edx
020d311c: mov    rcx, r13
020d311f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d3124: xor    r8d, r8d
020d3127: mov    rdx, r15
020d312a: mov    rcx, r13
020d312d: call   0x1820daca0                              ; ʹʷʽʶʶʸʻʿˁʾʵ$$ʴˀʲʸʷʶˁˀʺʶʲ
020d3132: test   rdi, rdi
020d3135: je     0x1820d3690
020d313b: xor    r8d, r8d
020d313e: xor    r13d, r13d
020d3141: mov    qword ptr [rsp + 0xc8], r8
020d3149: mov    qword ptr [rsp + 0xc0], r13
020d3151: cmp    dword ptr [rdi + 0x18], r8d
020d3155: je     0x1820d33b9
020d315b: mov    ebx, dword ptr [r14 + 4]
020d315f: cmp    r12b, 1
020d3163: jne    0x1820d3175
020d3165: cmp    byte ptr [r14 + 8], r8b
020d3169: je     0x1820d3175
020d316b: xor    edx, edx
020d316d: mov    rcx, rdi
020d3170: call   0x1820f0520                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʸʼʶʹʽʼʼʹʳʹˀ
020d3175: xor    r8d, r8d
020d3178: mov    rdx, rbp
020d317b: mov    rcx, rdi
020d317e: call   0x1820d12c0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʶʳʶʹʷʻʾʹʾʼʶ
020d3183: mov    qword ptr [rsp + 0xc8], rax
020d318b: mov    r14, rax
020d318e: cmp    sil, 6
020d3192: je     0x1820d32ad
020d3198: cmp    sil, 9
020d319c: je     0x1820d32ad
020d31a2: test   bl, 0x40
020d31a5: jbe    0x1820d31b6
020d31a7: xor    r8d, r8d
020d31aa: movzx  edx, sil
020d31ae: mov    rcx, rdi
020d31b1: call   0x1820f0c30                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʾʿʺʶʼʵʽʽʻʾʹ
020d31b6: test   bl, 0x20
020d31b9: jbe    0x1820d31ca
020d31bb: xor    r8d, r8d
020d31be: movzx  edx, sil
020d31c2: mov    rcx, rdi
020d31c5: call   0x1820f1900                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ˁʽʽʿʺʶʿʷʹʹʲ
020d31ca: test   ebx, 0x40000
020d31d0: jbe    0x1820d31e1
020d31d2: xor    r8d, r8d
020d31d5: movzx  edx, r12b
020d31d9: mov    rcx, rdi
020d31dc: call   0x1820ef7c0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʷʴʽʾʲʻʳʺʷʼʷ
020d31e1: test   bl, 0x10
020d31e4: ja     0x1820d3299
020d31ea: test   bl, 8
020d31ed: ja     0x1820d3285
020d31f3: test   bl, 4
020d31f6: ja     0x1820d3274
020d31f8: test   bl, 2
020d31fb: ja     0x1820d3263
020d31fd: test   ebx, 0x80
020d3203: jbe    0x1820d3214
020d3205: xor    r8d, r8d
020d3208: movzx  edx, sil
020d320c: mov    rcx, rdi
020d320f: call   0x1820f08a0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʺˁʹʵʵʴʾʽʵʼʼ
020d3214: mov    r12, qword ptr [rsp + 0xb8]
020d321c: cmp    byte ptr [rsp + 0x178], 0
020d3224: je     0x1820d3250
020d3226: test   rbp, rbp
020d3229: je     0x1820d3690
020d322f: mov    rax, qword ptr [rbp + 0x70]
020d3233: test   rax, rax
020d3236: je     0x1820d3690
020d323c: cmp    dword ptr [rax + 0x18], 0
020d3240: jle    0x1820d3250
020d3242: xor    r8d, r8d
020d3245: mov    rdx, rax
020d3248: mov    rcx, rdi
020d324b: call   0x1820d2440                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʸʹʵˁʵʼʾʸʾˀʺ
020d3250: xor    r8d, r8d
020d3253: mov    rdx, rbp
020d3256: mov    rcx, rdi
020d3259: call   0x1820d18f0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʷʴʹʷʷʶˁʷʷʿʸ
020d325e: jmp    0x1820d33c4
020d3263: xor    r8d, r8d
020d3266: movzx  edx, sil
020d326a: mov    rcx, rdi
020d326d: call   0x1820f09f0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʻʸʸʷʵʾʾʻʵʵʳ
020d3272: jmp    0x1820d3214
020d3274: xor    r8d, r8d
020d3277: movzx  edx, sil
020d327b: mov    rcx, rdi
020d327e: call   0x1820f17c0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ˀʶʳʵʾʿʹʿʴʻʳ
020d3283: jmp    0x1820d3214
020d3285: xor    r8d, r8d
020d3288: movzx  edx, sil
020d328c: mov    rcx, rdi
020d328f: call   0x1820f01e0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʸʹʹʷʽʾʺʾʼʻʶ
020d3294: jmp    0x1820d3214
020d3299: xor    r8d, r8d
020d329c: movzx  edx, sil
020d32a0: mov    rcx, rdi
020d32a3: call   0x1820f1540                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʿʷʳʻˁʿʷʺˁʼʵ
020d32a8: jmp    0x1820d3214
020d32ad: xor    edx, edx
020d32af: mov    rcx, rdi
020d32b2: call   0x1820ef450                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʴʻʷʽʺʹʽʲʲʿʾ
020d32b7: test   bl, 2
020d32ba: ja     0x1820d32cb
020d32bc: xor    r8d, r8d
020d32bf: movzx  edx, sil
020d32c3: mov    rcx, rdi
020d32c6: call   0x1820eff60                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʸʸʶʴʲʳʷʻʿʵʿ
020d32cb: cmp    r12b, 3
020d32cf: jne    0x1820d32e0
020d32d1: xor    r8d, r8d
020d32d4: movzx  edx, sil
020d32d8: mov    rcx, rdi
020d32db: call   0x1820f0320                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʸʼʴʸʷʺʲʵʻʹʻ
020d32e0: xor    edx, edx
020d32e2: mov    rcx, rdi
020d32e5: cmp    sil, 9
020d32e9: jne    0x1820d32f2
020d32eb: call   0x1820f1420                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʿʳʲʻʴʷʳˀʳʼʷ
020d32f0: jmp    0x1820d3306
020d32f2: call   0x1820f16e0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʿʾʲʽʻʹʺʶˀʹʽ
020d32f7: xor    r8d, r8d
020d32fa: movzx  edx, sil
020d32fe: mov    rcx, rdi
020d3301: call   0x1820efee0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʷʽʶʽˁʸʻʵʲʴʶ
020d3306: test   bl, 0x40
020d3309: jbe    0x1820d331a
020d330b: xor    r8d, r8d
020d330e: movzx  edx, sil
020d3312: mov    rcx, rdi
020d3315: call   0x1820f0c30                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʾʿʺʶʼʵʽʽʻʾʹ
020d331a: test   bl, 0x20
020d331d: jbe    0x1820d332e
020d331f: xor    r8d, r8d
020d3322: movzx  edx, sil
020d3326: mov    rcx, rdi
020d3329: call   0x1820f1900                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ˁʽʽʿʺʶʿʷʹʹʲ
020d332e: test   bl, 4
020d3331: jbe    0x1820d3342
020d3333: xor    r8d, r8d
020d3336: movzx  edx, sil
020d333a: mov    rcx, rdi
020d333d: call   0x1820f1250                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʾˀˁʸʷʹʵʶʳʲʼ
020d3342: test   bl, 8
020d3345: jbe    0x1820d3356
020d3347: xor    r8d, r8d
020d334a: movzx  edx, sil
020d334e: mov    rcx, rdi
020d3351: call   0x1820f06a0                              ; ʽʶʸʽʳʼʵʸʺʸʳ$$ʹˀʵʸʿˁʸʶʴˀʲ
020d3356: test   rbp, rbp
020d3359: je     0x1820d3690
020d335f: mov    rax, qword ptr [rbp + 0x88]
020d3366: test   rax, rax
020d3369: je     0x1820d3690
020d336f: xor    r9d, r9d
020d3372: mov    rcx, rdi
020d3375: cmp    dword ptr [rax + 0x18], r13d
020d3379: je     0x1820d3396
020d337b: mov    r8, rbp
020d337e: mov    rdx, r15
020d3381: call   0x1820cff60                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʳʷˁʸʵʾʼʷˀʲˁ
020d3386: mov    r13, rax
020d3389: mov    qword ptr [rsp + 0xc0], rax
020d3391: jmp    0x1820d3214
020d3396: mov    r12, qword ptr [rsp + 0xb8]
020d339e: mov    rdx, rbp
020d33a1: mov    r8, r12
020d33a4: call   0x1820d2b30                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʸˀʽʸʽʳʳʸʼʾʿ
020d33a9: mov    r13, rax
020d33ac: mov    qword ptr [rsp + 0xc0], rax
020d33b4: jmp    0x1820d321c
020d33b9: mov    r12, qword ptr [rsp + 0xb8]
020d33c1: mov    r14, r8
020d33c4: mov    rbx, qword ptr [rip + 0x1c1e6e5]         ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d33cb: cmp    qword ptr [rbx + 0x38], 0
020d33d0: jne    0x1820d33da
020d33d2: mov    rcx, rbx
020d33d5: call   0x182f657d0
020d33da: mov    r8, qword ptr [rbx + 0x38]
020d33de: lea    rcx, [rsp + 0xd8]
020d33e6: mov    rdx, rdi
020d33e9: mov    r8, qword ptr [r8 + 8]
020d33ed: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
020d33f2: cmp    byte ptr [rip + 0x1e45f18], 0            ; [0x3f19311] (bss)
020d33f9: mov    rsi, qword ptr [rsp + 0xd8]
020d3401: mov    ebx, dword ptr [rsp + 0xe0]
020d3408: jne    0x1820d341d
020d340a: lea    rcx, [rip + 0x1bd824f]                   ; [0x3cab660] metamethod:Method$System.Span<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Length()
020d3411: call   0x182f609b0
020d3416: mov    byte ptr [rip + 0x1e45ef4], 1            ; [0x3f19311] (bss)
020d341d: xor    ecx, ecx
020d341f: test   ebx, ebx
020d3421: jle    0x1820d3441
020d3423: cmp    ecx, ebx
020d3425: jae    0x1820d3696
020d342b: mov    rdx, qword ptr [rsi + rcx*8]
020d342f: test   rdx, rdx
020d3432: je     0x1820d3690
020d3438: mov    dword ptr [rdx + 0x20], ecx
020d343b: inc    ecx
020d343d: cmp    ecx, ebx
020d343f: jl     0x1820d3425
020d3441: xor    edx, edx
020d3443: mov    rcx, r15
020d3446: call   0x1820d3f30                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʼʴʷʼʻʵʸʷʵʷˀ
020d344b: xor    r8d, r8d
020d344e: mov    qword ptr [rsp + 0xf8], rax
020d3456: mov    rdx, rbp
020d3459: mov    rcx, r15
020d345c: call   0x1820d26a0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʸʹʸʴʷˁʿʶʾʽʶ
020d3461: mov    qword ptr [rsp + 0xd0], rax
020d3469: mov    rbx, rax
020d346c: test   rax, rax
020d346f: jne    0x1820d3497
020d3471: mov    rcx, qword ptr [rip + 0x1c223d8]         ; [0x3cf5850] meta:System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>_TypeInfo
020d3478: call   0x182f60c00
020d347d: mov    rdx, qword ptr [rip + 0x1c015bc]         ; [0x3cd4a40] metamethod:Method$System.Collections.Generic.List<ʼʶʷʷʴʶʺʽʴʼʽ>..ctor()
020d3484: mov    rcx, rax
020d3487: mov    rbx, rax
020d348a: mov    qword ptr [rsp + 0xd0], rax
020d3492: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d3497: test   r14, r14
020d349a: jne    0x1820d34bf
020d349c: mov    rcx, qword ptr [rip + 0x1c22a65]         ; [0x3cf5f08] meta:System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>_TypeInfo
020d34a3: call   0x182f60c00
020d34a8: mov    rdx, qword ptr [rip + 0x1c043f1]         ; [0x3cd78a0] metamethod:Method$System.Collections.Generic.List<ʾʻʹʶʹʺʸʺʺʶʹ>..ctor()
020d34af: mov    rcx, rax
020d34b2: mov    qword ptr [rsp + 0xc8], rax
020d34ba: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d34bf: test   r13, r13
020d34c2: jne    0x1820d34e7
020d34c4: mov    rcx, qword ptr [rip + 0x1c21245]         ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020d34cb: call   0x182f60c00
020d34d0: mov    rdx, qword ptr [rip + 0x1bfb1a9]         ; [0x3cce680] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020d34d7: mov    rcx, rax
020d34da: mov    qword ptr [rsp + 0xc0], rax
020d34e2: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d34e7: xor    r8d, r8d
020d34ea: mov    rdx, rdi
020d34ed: mov    rcx, rbp
020d34f0: call   0x1820d1ed0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʸʷʲʵʸʾʾʼʿʼʶ
020d34f5: mov    r9, r12
020d34f8: mov    qword ptr [rsp + 0xd8], rax
020d3500: mov    r8, rbx
020d3503: mov    qword ptr [rsp + 0x20], 0
020d350c: mov    rdx, rdi
020d350f: mov    rcx, r15
020d3512: call   0x1820d4440                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʼʽʾˁʼʻʹʷʺˁʻ
020d3517: mov    r13, qword ptr [r15 + 0x70]
020d351b: xor    edx, edx
020d351d: mov    r12, qword ptr [r15 + 0x78]
020d3521: mov    r8d, 0xa0
020d3527: mov    rcx, qword ptr [rsp + 0x160]
020d352f: mov    r15, qword ptr [r15 + 0x98]
020d3536: mov    qword ptr [rsp + 0xe8], rax
020d353e: mov    rax, qword ptr [rsp + 0x170]
020d3546: mov    r14, qword ptr [rax + 0x80]
020d354d: mov    rbp, qword ptr [rax + 0x88]
020d3554: mov    rsi, qword ptr [rax + 0x90]
020d355b: mov    rdi, qword ptr [rax + 0xa0]
020d3562: movsxd rbx, dword ptr [rax + 0xa8]
020d3569: call   0x18305d9d0
020d356e: mov    rax, qword ptr [rsp + 0xb8]
020d3576: mov    rcx, qword ptr [rsp + 0x170]
020d357e: mov    r9, qword ptr [rsp + 0xc0]
020d3586: add    rcx, 0xc8
020d358d: mov    r8, qword ptr [rsp + 0xc8]
020d3595: mov    rdx, qword ptr [rsp + 0x100]
020d359d: mov    qword ptr [rsp + 0xa8], 0
020d35a9: mov    qword ptr [rsp + 0xa0], rcx
020d35b1: mov    rcx, qword ptr [rsp + 0x168]
020d35b9: mov    qword ptr [rsp + 0x98], rbx
020d35c1: mov    qword ptr [rsp + 0x90], rdi
020d35c9: mov    qword ptr [rsp + 0x88], rsi
020d35d1: mov    qword ptr [rsp + 0x80], rbp
020d35d9: mov    qword ptr [rsp + 0x78], r14
020d35de: mov    qword ptr [rsp + 0x70], r15
020d35e3: mov    qword ptr [rsp + 0x68], r12
020d35e8: mov    qword ptr [rsp + 0x60], r13
020d35ed: mov    qword ptr [rsp + 0x58], rax
020d35f2: mov    rax, qword ptr [rsp + 0xe8]
020d35fa: mov    qword ptr [rsp + 0x50], rax
020d35ff: mov    rax, qword ptr [rsp + 0xf0]
020d3607: mov    qword ptr [rsp + 0x48], rax
020d360c: movzx  eax, byte ptr [rsp + 0xb0]
020d3614: mov    byte ptr [rsp + 0x40], al
020d3618: movzx  eax, byte ptr [rcx]
020d361b: mov    rcx, qword ptr [rsp + 0x160]
020d3623: mov    byte ptr [rsp + 0x38], al
020d3627: mov    rax, qword ptr [rsp + 0xf8]
020d362f: mov    qword ptr [rsp + 0x30], rax
020d3634: mov    rax, qword ptr [rsp + 0xd0]
020d363c: mov    qword ptr [rsp + 0x28], rax
020d3641: mov    rax, qword ptr [rsp + 0xd8]
020d3649: mov    qword ptr [rsp + 0x20], rax
020d364e: call   0x1820e8d30                              ; ʲʽʶʳʺʹʳˀʾʴʹ.ʸʲʻʼʴʵʺʲʺˀʸ$$.ctor
020d3653: mov    rax, qword ptr [rsp + 0x160]
020d365b: mov    r13, qword ptr [rsp + 0x118]
020d3663: mov    r12, qword ptr [rsp + 0x120]
020d366b: mov    rdi, qword ptr [rsp + 0x128]
020d3673: mov    rbp, qword ptr [rsp + 0x130]
020d367b: mov    rbx, qword ptr [rsp + 0x138]
020d3683: add    rsp, 0x140
020d368a: pop    r15
020d368c: pop    r14
020d368e: pop    rsi
020d368f: ret    
020d3690: call   0x182f60c50
020d3695: int3   
020d3696: call   0x182f60c40
020d369b: int3   
020d369c: int3   
