02128f10: push   rbx
02128f12: push   rsi
02128f13: push   rdi
02128f14: push   r12
02128f16: push   r14
02128f18: push   r15
02128f1a: sub    rsp, 0x58
02128f1e: mov    r15d, r9d
02128f21: mov    rsi, r8
02128f24: mov    r14, rdx
02128f27: mov    edi, ecx
02128f29: cmp    byte ptr [rip + 0x1df0560], 0            ; [0x3f19490] (bss)
02128f30: jne    0x182128f75
02128f32: lea    rcx, [rip + 0x1bbb7cf]                   ; [0x3ce4708] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʸʵʳʻʸʵˁʺʵ>.TryTake()
02128f39: call   0x182f609b0
02128f3e: lea    rcx, [rip + 0x1bae5cb]                   ; [0x3cd7510] meta:System.DateTime_TypeInfo
02128f45: call   0x182f609b0
02128f4a: lea    rcx, [rip + 0x1b6f29f]                   ; [0x3c981f0] metamethod:Method$System.Collections.Generic.Queue<ˀʸʸʵʳʻʸʵˁʺʵ>.Enqueue()
02128f51: call   0x182f609b0
02128f56: lea    rcx, [rip + 0x1b75923]                   ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02128f5d: call   0x182f609b0
02128f62: lea    rcx, [rip + 0x1b76887]                   ; [0x3c9f7f0] meta:ˀʸʸʵʳʻʸʵˁʺʵ_TypeInfo
02128f69: call   0x182f609b0
02128f6e: mov    byte ptr [rip + 0x1df051b], 1            ; [0x3f19490] (bss)
02128f75: xor    r12d, r12d
02128f78: mov    qword ptr [rsp + 0x20], r12
02128f7d: mov    qword ptr [rsp + 0x28], r12
02128f82: mov    byte ptr [rsp + 0x90], r12b
02128f8a: mov    rcx, qword ptr [rip + 0x1b758ef]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02128f91: cmp    dword ptr [rcx + 0xe0], r12d
02128f98: jne    0x182128fa6
02128f9a: call   0x182f60cf0
02128f9f: mov    rcx, qword ptr [rip + 0x1b758da]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02128fa6: mov    rax, qword ptr [rcx + 0xb8]
02128fad: cmp    edi, dword ptr [rax + 0x38]
02128fb0: jg     0x18212925b
02128fb6: cmp    dword ptr [rcx + 0xe0], r12d
02128fbd: jne    0x182128fcb
02128fbf: call   0x182f60cf0
02128fc4: mov    rcx, qword ptr [rip + 0x1b758b5]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02128fcb: mov    rax, qword ptr [rcx + 0xb8]
02128fd2: mov    rcx, qword ptr [rax + 0x20]
02128fd6: test   rcx, rcx
02128fd9: je     0x18212928f
02128fdf: mov    r8, qword ptr [rip + 0x1bbb722]          ; [0x3ce4708] metamethod:Method$System.Collections.Concurrent.ConcurrentBag<ˀʸʸʵʳʻʸʵˁʺʵ>.TryTake()
02128fe6: lea    rdx, [rsp + 0x20]
02128feb: call   0x1812f7040                              ; System.Collections.Concurrent.ConcurrentBag<object>$$TryTake
02128ff0: test   al, al
02128ff2: jne    0x182129014
02128ff4: mov    rcx, qword ptr [rip + 0x1b767f5]         ; [0x3c9f7f0] meta:ˀʸʸʵʳʻʸʵˁʺʵ_TypeInfo
02128ffb: call   0x182f60c00
02129000: mov    rbx, rax
02129003: xor    edx, edx
02129005: mov    rcx, rax
02129008: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
0212900d: mov    qword ptr [rsp + 0x20], rbx
02129012: jmp    0x182129019
02129014: mov    rbx, qword ptr [rsp + 0x20]
02129019: mov    rcx, qword ptr [rip + 0x1bae4f0]         ; [0x3cd7510] meta:System.DateTime_TypeInfo
02129020: cmp    dword ptr [rcx + 0xe0], r12d
02129027: jne    0x18212902e
02129029: call   0x182f60cf0
0212902e: xor    ecx, ecx
02129030: call   0x181b46f50                              ; System.DateTime$$get_UtcNow
02129035: test   rbx, rbx
02129038: je     0x18212928f
0212903e: mov    qword ptr [rbx + 0x18], rax
02129042: mov    rax, qword ptr [rsp + 0x20]
02129047: test   rax, rax
0212904a: je     0x18212928f
02129050: mov    dword ptr [rax + 0x10], edi
02129053: mov    rcx, qword ptr [rsp + 0x20]
02129058: test   rcx, rcx
0212905b: je     0x18212928f
02129061: mov    qword ptr [rcx + 0x28], r14
02129065: add    rcx, 0x28
02129069: mov    rdx, r14
0212906c: call   0x182f5fc00
02129071: mov    rcx, qword ptr [rsp + 0x20]
02129076: test   rcx, rcx
02129079: je     0x18212928f
0212907f: mov    qword ptr [rcx + 0x30], rsi
02129083: add    rcx, 0x30
02129087: mov    rdx, rsi
0212908a: call   0x182f5fc00
0212908f: mov    rax, qword ptr [rsp + 0x20]
02129094: test   rax, rax
02129097: je     0x18212928f
0212909d: mov    dword ptr [rax + 0x38], r15d
021290a1: mov    rcx, qword ptr [rsp + 0x20]
021290a6: test   rcx, rcx
021290a9: je     0x18212928f
021290af: mov    rdx, qword ptr [rsp + 0xb0]
021290b7: mov    qword ptr [rcx + 0x40], rdx
021290bb: add    rcx, 0x40
021290bf: call   0x182f5fc00
021290c4: mov    rcx, qword ptr [rsp + 0x20]
021290c9: test   rcx, rcx
021290cc: je     0x18212928f
021290d2: mov    rdx, qword ptr [rsp + 0xb8]
021290da: mov    qword ptr [rcx + 0x48], rdx
021290de: add    rcx, 0x48
021290e2: call   0x182f5fc00
021290e7: mov    rcx, qword ptr [rsp + 0x20]
021290ec: test   rcx, rcx
021290ef: je     0x18212928f
021290f5: mov    qword ptr [rcx + 0x50], r12
021290f9: add    rcx, 0x50
021290fd: xor    edx, edx
021290ff: call   0x182f5fc00
02129104: mov    rax, qword ptr [rip + 0x1b75775]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
0212910b: cmp    dword ptr [rax + 0xe0], r12d
02129112: jne    0x182129123
02129114: mov    rcx, rax
02129117: call   0x182f60cf0
0212911c: mov    rax, qword ptr [rip + 0x1b7575d]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02129123: mov    rax, qword ptr [rax + 0xb8]
0212912a: mov    rcx, qword ptr [rax + 0x18]
0212912e: mov    qword ptr [rsp + 0x28], rcx
02129133: mov    byte ptr [rsp + 0x90], r12b
0212913b: mov    qword ptr [rsp + 0x38], r12
02129140: lea    rax, [rsp + 0x90]
02129148: mov    qword ptr [rsp + 0x40], rax
0212914d: lea    rax, [rsp + 0x28]
02129152: mov    qword ptr [rsp + 0x48], rax
02129157: xor    r8d, r8d
0212915a: lea    rdx, [rsp + 0x90]
02129162: mov    rcx, qword ptr [rsp + 0x28]
02129167: call   0x181bd9120                              ; System.Threading.Monitor$$Enter
0212916c: mov    rax, qword ptr [rip + 0x1b7570d]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02129173: cmp    dword ptr [rax + 0xe0], 0
0212917a: jne    0x18212918b
0212917c: mov    rcx, rax
0212917f: call   0x182f60cf0
02129184: mov    rax, qword ptr [rip + 0x1b756f5]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
0212918b: mov    rax, qword ptr [rax + 0xb8]
02129192: mov    rcx, qword ptr [rax + 0x18]
02129196: test   rcx, rcx
02129199: je     0x182129295
0212919f: mov    r8, qword ptr [rip + 0x1b6f04a]          ; [0x3c981f0] metamethod:Method$System.Collections.Generic.Queue<ˀʸʸʵʳʻʸʵˁʺʵ>.Enqueue()
021291a6: mov    rdx, qword ptr [rsp + 0x20]
021291ab: call   0x1808ba490                              ; System.Collections.Generic.Queue<object>$$Enqueue
021291b0: nop    
021291b1: cmp    byte ptr [rsp + 0x90], 0
021291b9: je     0x1821291c7
021291bb: xor    edx, edx
021291bd: mov    rcx, qword ptr [rsp + 0x28]
021291c2: call   0x181bd91b0                              ; System.Threading.Monitor$$Exit
021291c7: jmp    0x1821291ed
021291c9: cmp    byte ptr [rsp + 0x90], 0
021291d1: je     0x1821291df
021291d3: xor    edx, edx
021291d5: mov    rcx, qword ptr [rsp + 0x28]
021291da: call   0x181bd91b0                              ; System.Threading.Monitor$$Exit
021291df: mov    rcx, qword ptr [rsp + 0x38]
021291e4: test   rcx, rcx
021291e7: jne    0x18212929b
021291ed: mov    rdx, qword ptr [rip + 0x1b7568c]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
021291f4: cmp    dword ptr [rdx + 0xe0], 0
021291fb: jne    0x18212920c
021291fd: mov    rcx, rdx
02129200: call   0x182f60cf0
02129205: mov    rdx, qword ptr [rip + 0x1b75674]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
0212920c: mov    rax, qword ptr [rdx + 0xb8]
02129213: movzx  ecx, byte ptr [rsp + 0xc0]
0212921b: mov    r8d, 1
02129221: cmp    qword ptr [rax], 0
02129225: cmove  ecx, r8d
02129229: test   ecx, ecx
0212922b: jne    0x182129269
0212922d: cmp    dword ptr [rdx + 0xe0], ecx
02129233: jne    0x182129244
02129235: mov    rcx, rdx
02129238: call   0x182f60cf0
0212923d: mov    rdx, qword ptr [rip + 0x1b7563c]         ; [0x3c9e880] meta:ʿʻʾʿʳʷʿʷʼʽʲ_TypeInfo
02129244: mov    rax, qword ptr [rdx + 0xb8]
0212924b: mov    rcx, qword ptr [rax + 0x28]
0212924f: test   rcx, rcx
02129252: je     0x18212928f
02129254: xor    edx, edx
02129256: call   0x181bc88c0                              ; System.Threading.EventWaitHandle$$Set
0212925b: add    rsp, 0x58
0212925f: pop    r15
02129261: pop    r14
02129263: pop    r12
02129265: pop    rdi
02129266: pop    rsi
02129267: pop    rbx
02129268: ret    
02129269: cmp    dword ptr [rdx + 0xe0], 0
02129270: jne    0x18212927a
02129272: mov    rcx, rdx
02129275: call   0x182f60cf0
0212927a: xor    ecx, ecx
0212927c: call   0x1805f2230
02129281: add    rsp, 0x58
02129285: pop    r15
02129287: pop    r14
02129289: pop    r12
0212928b: pop    rdi
0212928c: pop    rsi
0212928d: pop    rbx
0212928e: ret    
0212928f: call   0x182f60c50
02129294: nop    
02129295: call   0x182f60c50
0212929a: nop    
0212929b: call   0x182f60ce0
021292a0: int3   
021292a1: int3   
