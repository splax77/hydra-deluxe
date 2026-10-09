003bd710: mov    rax, rsp
003bd713: mov    qword ptr [rax + 8], rcx
003bd717: push   rbx
003bd718: push   rsi
003bd719: push   rdi
003bd71a: push   r12
003bd71c: push   r13
003bd71e: push   r14
003bd720: push   r15
003bd722: sub    rsp, 0x130
003bd729: movaps xmmword ptr [rax - 0x48], xmm6
003bd72d: movaps xmmword ptr [rax - 0x58], xmm7
003bd731: mov    rdi, rcx
003bd734: cmp    byte ptr [rip + 0x3b50cb5], 0            ; [0x3f0e3f0] (bss)
003bd73b: jne    0x1803bd7e4
003bd741: lea    rcx, [rip + 0x390ccd8]                   ; [0x3cca420] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.AwaitUnsafeOnCompleted<TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>, ʲʻʳʶʲʼʸʴʾʲʳ.ˁʸˁʵʽʵʾʹʲʶʼ>()
003bd748: call   0x182f609b0
003bd74d: lea    rcx, [rip + 0x390cfcc]                   ; [0x3cca720] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.SetResult()
003bd754: call   0x182f609b0
003bd759: lea    rcx, [rip + 0x391abf0]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
003bd760: call   0x182f609b0
003bd765: lea    rcx, [rip + 0x3932124]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
003bd76c: call   0x182f609b0
003bd771: lea    rcx, [rip + 0x38fcf40]                   ; [0x3cba6b8] metamethod:Method$System.Runtime.CompilerServices.TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.GetResult()
003bd778: call   0x182f609b0
003bd77d: lea    rcx, [rip + 0x38fcfec]                   ; [0x3cba770] metamethod:Method$System.Runtime.CompilerServices.TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.get_IsCompleted()
003bd784: call   0x182f609b0
003bd789: lea    rcx, [rip + 0x3904598]                   ; [0x3cc1d28] metamethod:Method$System.Threading.Tasks.Task<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.GetAwaiter()
003bd790: call   0x182f609b0
003bd795: lea    rcx, [rip + 0x3950abc]                   ; [0x3d0e258] meta:ʲʻʳʶʲʼʸʴʾʲʳ_TypeInfo
003bd79c: call   0x182f609b0
003bd7a1: lea    rcx, [rip + 0x392cab8]                   ; [0x3cea260] metamethod:Method$System.ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>..ctor()
003bd7a8: call   0x182f609b0
003bd7ad: lea    rcx, [rip + 0x38fd4e4]                   ; [0x3cbac98] str:'TrackHash: '
003bd7b4: call   0x182f609b0
003bd7b9: lea    rcx, [rip + 0x39242f0]                   ; [0x3ce1ab0] str:'SongHash: '
003bd7c0: call   0x182f609b0
003bd7c5: lea    rcx, [rip + 0x3903674]                   ; [0x3cc0e40] str:'Failed to compute leaderboard hashes.'
003bd7cc: call   0x182f609b0
003bd7d1: lea    rcx, [rip + 0x394c8e0]                   ; [0x3d0a0b8] str:'TempoMapHash: '
003bd7d8: call   0x182f609b0
003bd7dd: mov    byte ptr [rip + 0x3b50c0c], 1            ; [0x3f0e3f0] (bss)
003bd7e4: xor    ebx, ebx
003bd7e6: mov    qword ptr [rsp + 0xb8], rbx
003bd7ee: mov    qword ptr [rsp + 0xb0], rbx
003bd7f6: mov    qword ptr [rsp + 0x90], rbx
003bd7fe: mov    qword ptr [rsp + 0x70], rbx
003bd803: mov    dword ptr [rsp + 0xa8], ebx
003bd80a: mov    eax, dword ptr [rdi]
003bd80c: test   eax, eax
003bd80e: jne    0x1803bd82a
003bd810: mov    rax, qword ptr [rdi + 0x50]
003bd814: mov    qword ptr [rsp + 0x70], rax
003bd819: xor    eax, eax
003bd81b: mov    qword ptr [rdi + 0x50], rax
003bd81f: mov    dword ptr [rdi], 0xffffffff
003bd825: jmp    0x1803bda26
003bd82a: mov    rax, qword ptr [rip + 0x393205f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
003bd831: cmp    dword ptr [rax + 0xe0], 0
003bd838: jne    0x1803bd849
003bd83a: mov    rcx, rax
003bd83d: call   0x182f60cf0
003bd842: mov    rax, qword ptr [rip + 0x3932047]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
003bd849: mov    rax, qword ptr [rax + 0xb8]
003bd850: mov    rcx, qword ptr [rax + 8]
003bd854: test   rcx, rcx
003bd857: je     0x1803bddab
003bd85d: mov    qword ptr [rsp + 0x30], rbx
003bd862: lea    rax, [rsp + 0x90]
003bd86a: mov    qword ptr [rsp + 0x28], rax
003bd86f: lea    rax, [rsp + 0xb0]
003bd877: mov    qword ptr [rsp + 0x20], rax
003bd87c: lea    r9, [rsp + 0xb8]
003bd884: mov    r8, qword ptr [rdi + 0x38]
003bd888: mov    rdx, qword ptr [rdi + 0x30]
003bd88c: call   0x180235ad0                              ; GlobalVariables$$ʺʵʸʸʽʶʾˀʶʺʺ
003bd891: test   al, al
003bd893: je     0x1803bdb6a
003bd899: xor    r8d, r8d
003bd89c: mov    rdx, qword ptr [rsp + 0xb0]
003bd8a4: mov    rcx, qword ptr [rip + 0x394c80d]         ; [0x3d0a0b8] str:'TempoMapHash: '
003bd8ab: call   0x18197f200                              ; System.String$$Concat
003bd8b0: mov    rsi, rax
003bd8b3: mov    rcx, qword ptr [rip + 0x391aa96]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
003bd8ba: cmp    dword ptr [rcx + 0xe0], 0
003bd8c1: jne    0x1803bd8c8
003bd8c3: call   0x182f60cf0
003bd8c8: xor    edx, edx
003bd8ca: mov    rcx, rsi
003bd8cd: call   0x18285fd00                              ; UnityEngine.Debug$$Log
003bd8d2: xor    r8d, r8d
003bd8d5: mov    rdx, qword ptr [rsp + 0x90]
003bd8dd: mov    rcx, qword ptr [rip + 0x38fd3b4]         ; [0x3cbac98] str:'TrackHash: '
003bd8e4: call   0x18197f200                              ; System.String$$Concat
003bd8e9: xor    edx, edx
003bd8eb: mov    rcx, rax
003bd8ee: call   0x18285fd00                              ; UnityEngine.Debug$$Log
003bd8f3: xor    r8d, r8d
003bd8f6: mov    rdx, qword ptr [rsp + 0xb8]
003bd8fe: mov    rcx, qword ptr [rip + 0x39241ab]         ; [0x3ce1ab0] str:'SongHash: '
003bd905: call   0x18197f200                              ; System.String$$Concat
003bd90a: xor    edx, edx
003bd90c: mov    rcx, rax
003bd90f: call   0x18285fd00                              ; UnityEngine.Debug$$Log
003bd914: mov    r12, qword ptr [rdi + 0x40]
003bd918: mov    r13, qword ptr [rsp + 0x90]
003bd920: xor    ecx, ecx
003bd922: call   0x180139dd0                              ; LeaderboardsOnlineManager$$ˀʸʲʼˁʶʹʾʾʲʺ
003bd927: test   rax, rax
003bd92a: je     0x1803bdda6
003bd930: mov    rax, qword ptr [rax + 0x18]
003bd934: mov    qword ptr [rsp + 0xe0], rax
003bd93c: mov    eax, dword ptr [rdi + 0x48]
003bd93f: mov    dword ptr [rsp + 0x188], eax
003bd946: mov    eax, dword ptr [rdi + 0x4c]
003bd949: mov    dword ptr [rsp + 0x180], eax
003bd950: mov    rsi, qword ptr [rdi + 0x38]
003bd954: mov    rcx, qword ptr [rip + 0x39508fd]         ; [0x3d0e258] meta:ʲʻʳʶʲʼʸʴʾʲʳ_TypeInfo
003bd95b: cmp    dword ptr [rcx + 0xe0], 0
003bd962: jne    0x1803bd969
003bd964: call   0x182f60cf0
003bd969: xor    edx, edx
003bd96b: mov    rcx, rsi
003bd96e: call   0x1803b2e30                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʾʻʶʻʵʵʳʹʶʾʽ
003bd973: mov    r15, rax
003bd976: xor    edx, edx
003bd978: mov    rcx, qword ptr [rdi + 0x38]
003bd97c: call   0x1803b02b0                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʴʽʽʵʻʺʺʷʳʲʿ
003bd981: mov    r14, rax
003bd984: xor    edx, edx
003bd986: mov    rcx, qword ptr [rdi + 0x38]
003bd98a: call   0x1803b3570                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʿʵʿʲʹʹʽʶʼʾˀ
003bd98f: mov    rsi, rax
003bd992: xor    edx, edx
003bd994: mov    rcx, qword ptr [rdi + 0x38]
003bd998: call   0x1803b1000                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʸʴʻʾʽʻʽʵʿʺʿ
003bd99d: mov    qword ptr [rsp + 0x60], rbx
003bd9a2: mov    qword ptr [rsp + 0x58], r15
003bd9a7: mov    ecx, dword ptr [rsp + 0x180]
003bd9ae: mov    dword ptr [rsp + 0x50], ecx
003bd9b2: mov    ecx, dword ptr [rsp + 0x188]
003bd9b9: mov    dword ptr [rsp + 0x48], ecx
003bd9bd: mov    qword ptr [rsp + 0x40], rsi
003bd9c2: mov    qword ptr [rsp + 0x38], rax
003bd9c7: mov    qword ptr [rsp + 0x30], r14
003bd9cc: mov    rax, qword ptr [rsp + 0xe0]
003bd9d4: mov    qword ptr [rsp + 0x28], rax
003bd9d9: mov    qword ptr [rsp + 0x20], r13
003bd9de: mov    edx, 9
003bd9e3: mov    r9d, 2
003bd9e9: mov    r8d, r9d
003bd9ec: mov    rcx, r12
003bd9ef: call   0x1803b27a0                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʼˁʺʵʵˀˁʵʾʽʶ
003bd9f4: test   rax, rax
003bd9f7: je     0x1803bdda1
003bd9fd: mov    rdx, qword ptr [rip + 0x3904324]         ; [0x3cc1d28] metamethod:Method$System.Threading.Tasks.Task<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.GetAwaiter()
003bda04: mov    rcx, rax
003bda07: call   0x180a415e0                              ; System.Threading.Tasks.Task<__Il2CppFullySharedGenericType>$$GetAwaiter
003bda0c: mov    qword ptr [rsp + 0x70], rax
003bda11: mov    rdx, qword ptr [rip + 0x38fcd58]         ; [0x3cba770] metamethod:Method$System.Runtime.CompilerServices.TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.get_IsCompleted()
003bda18: lea    rcx, [rsp + 0x70]
003bda1d: call   0x1809ef900                              ; System.Runtime.CompilerServices.ConfiguredTaskAwaitable.ConfiguredTaskAwaiter<__Il2CppFullySharedGenericType>$$get_IsCompleted
003bda22: test   al, al
003bda24: je     0x1803bda93
003bda26: mov    r8, qword ptr [rip + 0x38fcc8b]          ; [0x3cba6b8] metamethod:Method$System.Runtime.CompilerServices.TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.GetResult()
003bda2d: lea    rdx, [rsp + 0x70]
003bda32: lea    rcx, [rsp + 0xc0]
003bda3a: call   0x1809eeef0                              ; System.Runtime.CompilerServices.ConfiguredTaskAwaitable.ConfiguredTaskAwaiter<Status>$$GetResult
003bda3f: xorps  xmm0, xmm0
003bda42: xor    eax, eax
003bda44: movups xmmword ptr [rsp + 0x78], xmm0
003bda49: mov    qword ptr [rsp + 0x88], rax
003bda51: mov    rax, qword ptr [rip + 0x392c808]         ; [0x3cea260] metamethod:Method$System.ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>..ctor()
003bda58: mov    qword ptr [rsp + 0x20], rax
003bda5d: mov    r9, qword ptr [rsp + 0xd0]
003bda65: movzx  r8d, byte ptr [rsp + 0xc8]
003bda6e: mov    rdx, qword ptr [rsp + 0xc0]
003bda76: lea    rcx, [rsp + 0x78]
003bda7b: call   0x180cfc7a0                              ; System.ValueTuple<object, bool, object>$$.ctor
003bda80: movups xmm6, xmmword ptr [rsp + 0x78]
003bda85: movsd  xmm7, qword ptr [rsp + 0x88]
003bda8e: jmp    0x1803bdc53
003bda93: mov    dword ptr [rdi], ebx
003bda95: mov    rax, qword ptr [rsp + 0x70]
003bda9a: mov    qword ptr [rdi + 0x50], rax
003bda9e: lea    rcx, [rdi + 0x50]
003bdaa2: xor    edx, edx
003bdaa4: call   0x182f5fc00
003bdaa9: mov    rbx, qword ptr [rip + 0x390c970]         ; [0x3cca420] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.AwaitUnsafeOnCompleted<TaskAwaiter<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>, ʲʻʳʶʲʼʸʴʾʲʳ.ˁʸˁʵʽʵʾʹʲʶʼ>()
003bdab0: cmp    qword ptr [rbx + 0x38], 0
003bdab5: jne    0x1803bdabf
003bdab7: mov    rcx, rbx
003bdaba: call   0x182f657d0
003bdabf: cmp    qword ptr [rdi + 8], 0
003bdac4: jne    0x1803bdb04
003bdac6: mov    rax, qword ptr [rbx + 0x38]
003bdaca: mov    rax, qword ptr [rax + 0x10]
003bdace: test   byte ptr [rax + 0x135], 1
003bdad5: jne    0x1803bdadf
003bdad7: mov    rcx, rax
003bdada: call   0x182f65750
003bdadf: cmp    dword ptr [rax + 0xe0], 0
003bdae6: jne    0x1803bdaf0
003bdae8: mov    rcx, rax
003bdaeb: call   0x182f60cf0
003bdaf0: mov    r8, qword ptr [rbx + 0x38]
003bdaf4: mov    r8, qword ptr [r8 + 8]
003bdaf8: lea    rdx, [rdi + 8]
003bdafc: mov    rcx, rdi
003bdaff: call   0x1810f9dc0                              ; Cysharp.Threading.Tasks.CompilerServices.AsyncUniTask<ʲʻʳʶʲʼʸʴʾʲʳ.ˁʸˁʵʽʵʾʹʲʶʼ, ValueTuple<object, bool, object>>$$SetStateMachine
003bdb04: mov    rdi, qword ptr [rdi + 8]
003bdb08: test   rdi, rdi
003bdb0b: je     0x1803bdd9b
003bdb11: mov    rax, qword ptr [rbx + 0x20]
003bdb15: test   byte ptr [rax + 0x135], 1
003bdb1c: jne    0x1803bdb26
003bdb1e: mov    rcx, rax
003bdb21: call   0x182f65750
003bdb26: mov    rax, qword ptr [rax + 0xc0]
003bdb2d: mov    rax, qword ptr [rax + 8]
003bdb31: test   byte ptr [rax + 0x135], 1
003bdb38: jne    0x1803bdb42
003bdb3a: mov    rcx, rax
003bdb3d: call   0x182f65750
003bdb42: xor    ecx, ecx
003bdb44: mov    r8, rdi
003bdb47: mov    rdx, rax
003bdb4a: call   0x182c44210
003bdb4f: mov    r8, qword ptr [rbx + 0x38]
003bdb53: mov    r8, qword ptr [r8 + 0x28]
003bdb57: mov    rdx, rax
003bdb5a: lea    rcx, [rsp + 0x70]
003bdb5f: call   0x1809ef8e0                              ; System.Runtime.CompilerServices.TaskAwaiter<__Il2CppFullySharedGenericType>$$UnsafeOnCompleted
003bdb64: nop    
003bdb65: jmp    0x1803bdd04
003bdb6a: xorps  xmm0, xmm0
003bdb6d: xor    eax, eax
003bdb6f: movups xmmword ptr [rsp + 0x78], xmm0
003bdb74: mov    qword ptr [rsp + 0x88], rax
003bdb7c: mov    rax, qword ptr [rip + 0x392c6dd]         ; [0x3cea260] metamethod:Method$System.ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>..ctor()
003bdb83: mov    qword ptr [rsp + 0x20], rax
003bdb88: mov    r9, qword ptr [rip + 0x39032b1]          ; [0x3cc0e40] str:'Failed to compute leaderboard hashes.'
003bdb8f: xor    r8d, r8d
003bdb92: xor    edx, edx
003bdb94: lea    rcx, [rsp + 0x78]
003bdb99: call   0x180cfc7a0                              ; System.ValueTuple<object, bool, object>$$.ctor
003bdb9e: movups xmm6, xmmword ptr [rsp + 0x78]
003bdba3: movsd  xmm7, qword ptr [rsp + 0x88]
003bdbac: jmp    0x1803bdc53
003bdbb1: lea    rcx, [rip + 0x391a798]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
003bdbb8: call   0x182f609d0
003bdbbd: cmp    dword ptr [rax + 0xe0], 0
003bdbc4: jne    0x1803bdbce
003bdbc6: mov    rcx, rax
003bdbc9: call   0x182f60cf0
003bdbce: xor    edx, edx
003bdbd0: mov    rbx, qword ptr [rsp + 0x180]
003bdbd8: mov    rcx, rbx
003bdbdb: call   0x18285f4c0                              ; UnityEngine.Debug$$LogException
003bdbe0: test   rbx, rbx
003bdbe3: je     0x1803bddb1
003bdbe9: mov    rax, qword ptr [rbx]
003bdbec: mov    rdx, qword ptr [rax + 0x190]
003bdbf3: mov    rcx, rbx
003bdbf6: call   qword ptr [rax + 0x188]
003bdbfc: mov    rbx, rax
003bdbff: xorps  xmm0, xmm0
003bdc02: xor    eax, eax
003bdc04: movups xmmword ptr [rsp + 0x78], xmm0
003bdc09: mov    qword ptr [rsp + 0x88], rax
003bdc11: lea    rcx, [rip + 0x392c648]                   ; [0x3cea260] metamethod:Method$System.ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>..ctor()
003bdc18: call   0x182f609d0
003bdc1d: mov    qword ptr [rsp + 0x20], rax
003bdc22: mov    r9, rbx
003bdc25: xor    r8d, r8d
003bdc28: xor    edx, edx
003bdc2a: lea    rcx, [rsp + 0x78]
003bdc2f: call   0x180cfc7a0                              ; System.ValueTuple<object, bool, object>$$.ctor
003bdc34: movups xmm6, xmmword ptr [rsp + 0x78]
003bdc39: movsd  xmm7, qword ptr [rsp + 0x88]
003bdc42: xor    ebx, ebx
003bdc44: mov    dword ptr [rsp + 0xa8], ebx
003bdc4b: mov    rdi, qword ptr [rsp + 0x170]
003bdc53: mov    dword ptr [rdi], 0xfffffffe
003bdc59: cmp    qword ptr [rdi + 8], 0
003bdc5e: je     0x1803bdd46
003bdc64: mov    rdi, qword ptr [rdi + 8]
003bdc68: mov    rax, qword ptr [rip + 0x390cab1]         ; [0x3cca720] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.SetResult()
003bdc6f: mov    rax, qword ptr [rax + 0x20]
003bdc73: test   byte ptr [rax + 0x135], 1
003bdc7a: jne    0x1803bdc84
003bdc7c: mov    rcx, rax
003bdc7f: call   0x182f65750
003bdc84: mov    rax, qword ptr [rax + 0xc0]
003bdc8b: mov    rdx, qword ptr [rax + 8]
003bdc8f: test   byte ptr [rdx + 0x135], 1
003bdc96: jne    0x1803bdca3
003bdc98: mov    rcx, rdx
003bdc9b: call   0x182f65750
003bdca0: mov    rdx, rax
003bdca3: mov    r9, qword ptr [rdi]
003bdca6: movzx  ecx, word ptr [r9 + 0x12e]
003bdcae: cmp    bx, cx
003bdcb1: jae    0x1803bdcd4
003bdcb3: mov    r8, qword ptr [r9 + 0xb0]
003bdcba: nop    word ptr [rax + rax]
003bdcc0: movzx  eax, bx
003bdcc3: add    rax, rax
003bdcc6: cmp    qword ptr [r8 + rax*8], rdx
003bdcca: je     0x1803bdd27
003bdccc: inc    bx
003bdccf: cmp    bx, cx
003bdcd2: jb     0x1803bdcc0
003bdcd4: mov    r8d, 2
003bdcda: mov    rcx, rdi
003bdcdd: call   0x182f65070
003bdce2: movaps xmmword ptr [rsp + 0xc0], xmm6
003bdcea: movsd  qword ptr [rsp + 0xd0], xmm7
003bdcf3: mov    r8, qword ptr [rax + 8]
003bdcf7: lea    rdx, [rsp + 0xc0]
003bdcff: mov    rcx, rdi
003bdd02: call   qword ptr [rax]
003bdd04: movaps xmm6, xmmword ptr [rsp + 0x120]
003bdd0c: movaps xmm7, xmmword ptr [rsp + 0x110]
003bdd14: add    rsp, 0x130
003bdd1b: pop    r15
003bdd1d: pop    r14
003bdd1f: pop    r13
003bdd21: pop    r12
003bdd23: pop    rdi
003bdd24: pop    rsi
003bdd25: pop    rbx
003bdd26: ret    
003bdd27: movzx  ecx, bx
003bdd2a: add    rcx, rcx
003bdd2d: mov    eax, dword ptr [r8 + rcx*8 + 8]
003bdd32: add    eax, 2
003bdd35: cdqe   
003bdd37: shl    rax, 4
003bdd3b: add    rax, 0x138
003bdd41: add    rax, r9
003bdd44: jmp    0x1803bdce2
003bdd46: movups xmmword ptr [rdi + 0x18], xmm6
003bdd4a: movsd  qword ptr [rdi + 0x28], xmm7
003bdd4f: lea    rcx, [rdi + 0x18]
003bdd53: xor    edx, edx
003bdd55: call   0x182f5fc00
003bdd5a: jmp    0x1803bdd04
003bdd5c: mov    rbx, qword ptr [rsp + 0x170]
003bdd64: mov    dword ptr [rbx], 0xfffffffe
003bdd6a: lea    rcx, [rip + 0x390c8ef]                   ; [0x3cca660] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<ValueTuple<ʲʻʳʶʲʼʸʴʾʲʳ.ScoreResponse, bool, string>>.SetException()
003bdd71: call   0x182f609d0
003bdd76: mov    r8, rax
003bdd79: mov    eax, dword ptr [rsp + 0xa8]
003bdd80: dec    eax
003bdd82: movsxd rdx, eax
003bdd85: lea    rcx, [rbx + 8]
003bdd89: mov    rdx, qword ptr [rsp + rdx*8 + 0x98]
003bdd91: call   0x182c49c00
003bdd96: jmp    0x1803bdd04
003bdd9b: call   0x182f60c50
003bdda0: nop    
003bdda1: call   0x182f60c50
003bdda6: call   0x182f60c50
003bddab: call   0x182f60c50
003bddb0: nop    
003bddb1: call   0x182f60c50
003bddb6: int3   
003bddb7: int3   
