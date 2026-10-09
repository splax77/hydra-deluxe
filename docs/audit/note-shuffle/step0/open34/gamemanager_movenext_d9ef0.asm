000d9ef0: mov    rax, rsp
000d9ef3: mov    qword ptr [rax + 8], rcx
000d9ef7: push   rbx
000d9ef8: push   rsi
000d9ef9: push   rdi
000d9efa: push   r12
000d9efc: push   r13
000d9efe: push   r14
000d9f00: push   r15
000d9f02: sub    rsp, 0x270
000d9f09: movaps xmmword ptr [rax - 0x48], xmm6
000d9f0d: movaps xmmword ptr [rax - 0x58], xmm7
000d9f11: movaps xmmword ptr [rax - 0x68], xmm8
000d9f16: movaps xmmword ptr [rax - 0x78], xmm9
000d9f1b: movaps xmmword ptr [rax - 0x88], xmm10
000d9f23: movaps xmmword ptr [rax - 0x98], xmm11
000d9f2b: movaps xmmword ptr [rax - 0xa8], xmm12
000d9f33: movaps xmmword ptr [rax - 0xb8], xmm13
000d9f3b: movaps xmmword ptr [rax - 0xc8], xmm14
000d9f43: movaps xmmword ptr [rax - 0xd8], xmm15
000d9f4b: cmp    byte ptr [rip + 0x3e32270], 0            ; [0x3f0c1c2] (bss)
000d9f52: jne    0x1800da0c3
000d9f58: lea    rcx, [rip + 0x3bf0f41]                   ; [0x3ccaea0] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.AwaitUnsafeOnCompleted<UniTask.Awaiter<ValueTuple<bool, string, string>>, GameManager.ʾʿʷʵʿʷʺˀʿʹˀ>()
000d9f5f: call   0x182f609b0
000d9f64: lea    rcx, [rip + 0x3bf10b5]                   ; [0x3ccb020] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.AwaitUnsafeOnCompleted<UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>, GameManager.ʾʿʷʵʿʷʺˀʿʹˀ>()
000d9f6b: call   0x182f609b0
000d9f70: lea    rcx, [rip + 0x3bf1469]                   ; [0x3ccb3e0] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.SetResult()
000d9f77: call   0x182f609b0
000d9f7c: lea    rcx, [rip + 0x3bf5ff5]                   ; [0x3ccff78] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ValueTuple<bool, string, string>>.GetResult()
000d9f83: call   0x182f609b0
000d9f88: lea    rcx, [rip + 0x3bf7069]                   ; [0x3cd0ff8] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.GetResult()
000d9f8f: call   0x182f609b0
000d9f94: lea    rcx, [rip + 0x3bf711d]                   ; [0x3cd10b8] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.get_IsCompleted()
000d9f9b: call   0x182f609b0
000d9fa0: lea    rcx, [rip + 0x3bf6091]                   ; [0x3cd0038] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ValueTuple<bool, string, string>>.get_IsCompleted()
000d9fa7: call   0x182f609b0
000d9fac: lea    rcx, [rip + 0x3bfe39d]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d9fb3: call   0x182f609b0
000d9fb8: lea    rcx, [rip + 0x3c0e0e9]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000d9fbf: call   0x182f609b0
000d9fc4: lea    rcx, [rip + 0x3c158c5]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9fcb: call   0x182f609b0
000d9fd0: lea    rcx, [rip + 0x3c23cf9]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000d9fd7: call   0x182f609b0
000d9fdc: lea    rcx, [rip + 0x3bc9f4d]                   ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
000d9fe3: call   0x182f609b0
000d9fe8: lea    rcx, [rip + 0x3bd7b11]                   ; [0x3cb1b00] metamethod:Method$UnityEngine.Object.FindObjectOfType<InGameLeaderboardManager>()
000d9fef: call   0x182f609b0
000d9ff4: lea    rcx, [rip + 0x3bd8c6d]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000d9ffb: call   0x182f609b0
000da000: lea    rcx, [rip + 0x3c34251]                   ; [0x3d0e258] meta:ʲʻʳʶʲʼʸʴʾʲʳ_TypeInfo
000da007: call   0x182f609b0
000da00c: lea    rcx, [rip + 0x3bbaeed]                   ; [0x3c94f00] meta:ʶʼʾʻˁʳʲʹˁʿʷ_TypeInfo
000da013: call   0x182f609b0
000da018: lea    rcx, [rip + 0x3c1c981]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
000da01f: call   0x182f609b0
000da024: lea    rcx, [rip + 0x3bbe2f5]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000da02b: call   0x182f609b0
000da030: lea    rcx, [rip + 0x3bbf021]                   ; [0x3c99058] meta:ʺʺʸʶʿʵˀʸʲˀʲ_TypeInfo
000da037: call   0x182f609b0
000da03c: lea    rcx, [rip + 0x3bc3b95]                   ; [0x3c9dbd8] meta:ʾˁʸˁˁʻʿˁˁʲʽ_TypeInfo
000da043: call   0x182f609b0
000da048: lea    rcx, [rip + 0x3bfe3d1]                   ; [0x3cd8420] metamethod:Method$Cysharp.Threading.Tasks.UniTask<ValueTuple<bool, string, string>>.GetAwaiter()
000da04f: call   0x182f609b0
000da054: lea    rcx, [rip + 0x3bffc85]                   ; [0x3cd9ce0] metamethod:Method$Cysharp.Threading.Tasks.UniTask<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.GetAwaiter()
000da05b: call   0x182f609b0
000da060: lea    rcx, [rip + 0x3c136d9]                   ; [0x3ced740] str:'No valid main player found for leaderboard session'
000da067: call   0x182f609b0
000da06c: lea    rcx, [rip + 0x3bf38bd]                   ; [0x3ccd930] str:'leaderboardsNoServersAvailable'
000da073: call   0x182f609b0
000da078: lea    rcx, [rip + 0x3bf3c71]                   ; [0x3ccdcf0] str:'leaderboardsServersFull'
000da07f: call   0x182f609b0
000da084: lea    rcx, [rip + 0x3bebbb5]                   ; [0x3cc5c40] str:'Failed to show leaderboards in game. InGameLeaderboardManager not found.'
000da08b: call   0x182f609b0
000da090: lea    rcx, [rip + 0x3bf1999]                   ; [0x3ccba30] str:'Main Menu'
000da097: call   0x182f609b0
000da09c: lea    rcx, [rip + 0x3c09495]                   ; [0x3ce3538] str:'unableToJoinLeaderboardSession'
000da0a3: call   0x182f609b0
000da0a8: lea    rcx, [rip + 0x3bd9dc1]                   ; [0x3cb3e70] str:''
000da0af: call   0x182f609b0
000da0b4: mov    byte ptr [rip + 0x3e32107], 1            ; [0x3f0c1c2] (bss)
000da0bb: mov    rcx, qword ptr [rsp + 0x2b0]
000da0c3: xor    ebx, ebx
000da0c5: mov    dword ptr [rsp + 0x2c0], ebx
000da0cc: mov    r14d, ebx
000da0cf: mov    dword ptr [rsp + 0xf8], ebx
000da0d6: mov    eax, dword ptr [rcx]
000da0d8: mov    dword ptr [rsp + 0x2c0], eax
000da0df: mov    rdi, qword ptr [rcx + 0x20]
000da0e3: cmp    dword ptr [rsp + 0x2c0], 1
000da0eb: jbe    0x1800da4f7
000da0f1: test   rdi, rdi
000da0f4: je     0x1800db1d3
000da0fa: xor    edx, edx
000da0fc: mov    rcx, rdi
000da0ff: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da104: mov    rsi, rax
000da107: mov    rcx, qword ptr [rip + 0x3bd8b5a]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000da10e: cmp    dword ptr [rcx + 0xe0], ebx
000da114: jne    0x1800da11b
000da116: call   0x182f60cf0
000da11b: xor    r8d, r8d
000da11e: xor    edx, edx
000da120: mov    rcx, rsi
000da123: call   0x18287fd30                              ; UnityEngine.Object$$op_Equality
000da128: test   al, al
000da12a: jne    0x1800da583
000da130: xor    edx, edx
000da132: mov    rcx, rdi
000da135: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da13a: test   rax, rax
000da13d: je     0x1800db1ce
000da143: mov    rax, qword ptr [rax + 0x120]
000da14a: test   rax, rax
000da14d: je     0x1800db1c9
000da153: movups xmm8, xmmword ptr [rax + 0x1e0]
000da15b: movups xmm9, xmmword ptr [rax + 0x1f0]
000da163: movups xmm10, xmmword ptr [rax + 0x200]
000da16b: movups xmm11, xmmword ptr [rax + 0x210]
000da173: movups xmm12, xmmword ptr [rax + 0x220]
000da17b: movups xmm13, xmmword ptr [rax + 0x230]
000da183: movups xmm14, xmmword ptr [rax + 0x240]
000da18b: movups xmm15, xmmword ptr [rax + 0x250]
000da193: movups xmm0, xmmword ptr [rax + 0x260]
000da19a: movups xmmword ptr [rsp + 0x100], xmm0
000da1a2: movups xmm0, xmmword ptr [rax + 0x270]
000da1a9: movups xmmword ptr [rsp + 0x90], xmm0
000da1b1: xor    ecx, ecx
000da1b3: call   0x180139dd0                              ; LeaderboardsOnlineManager$$ˀʸʲʼˁʶʹʾʾʲʺ
000da1b8: test   rax, rax
000da1bb: je     0x1800db1c4
000da1c1: mov    rcx, qword ptr [rax + 0x20]
000da1c5: mov    qword ptr [rsp + 0xc0], rcx
000da1cd: mov    rax, qword ptr [rip + 0x3c156bc]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000da1d4: cmp    dword ptr [rax + 0xe0], 0
000da1db: jne    0x1800da1ec
000da1dd: mov    rcx, rax
000da1e0: call   0x182f60cf0
000da1e5: mov    rax, qword ptr [rip + 0x3c156a4]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000da1ec: mov    rax, qword ptr [rax + 0xb8]
000da1f3: mov    rcx, qword ptr [rax + 8]
000da1f7: test   rcx, rcx
000da1fa: je     0x1800db1bf
000da200: mov    rax, qword ptr [rcx + 0x60]
000da204: mov    qword ptr [rsp + 0x80], rax
000da20c: xor    edx, edx
000da20e: mov    rcx, rdi
000da211: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da216: test   rax, rax
000da219: je     0x1800db1ba
000da21f: mov    rax, qword ptr [rax + 0x88]
000da226: test   rax, rax
000da229: je     0x1800db1b5
000da22f: mov    rcx, qword ptr [rax + 0x10]
000da233: test   rcx, rcx
000da236: je     0x1800db1b0
000da23c: mov    rax, qword ptr [rcx + 0x60]
000da240: test   rax, rax
000da243: je     0x1800db1ab
000da249: mov    ecx, dword ptr [rax + 0x10]
000da24c: mov    dword ptr [rsp + 0x2c8], ecx
000da253: xor    edx, edx
000da255: mov    rcx, rdi
000da258: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da25d: test   rax, rax
000da260: je     0x1800db1a6
000da266: mov    r8, qword ptr [rax]
000da269: mov    rdx, qword ptr [r8 + 0x2a0]
000da270: mov    rcx, rax
000da273: call   qword ptr [r8 + 0x298]
000da27a: mov    r13d, eax
000da27d: xor    edx, edx
000da27f: mov    rcx, rdi
000da282: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da287: test   rax, rax
000da28a: je     0x1800db1a1
000da290: mov    rcx, qword ptr [rax + 0x88]
000da297: test   rcx, rcx
000da29a: je     0x1800db19c
000da2a0: mov    rax, qword ptr [rcx + 0x10]
000da2a4: test   rax, rax
000da2a7: je     0x1800db197
000da2ad: mov    r12d, dword ptr [rax + 0x14]
000da2b1: xor    edx, edx
000da2b3: mov    rcx, rdi
000da2b6: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da2bb: test   rax, rax
000da2be: je     0x1800db192
000da2c4: mov    rax, qword ptr [rax + 0x88]
000da2cb: test   rax, rax
000da2ce: je     0x1800db18d
000da2d4: mov    rcx, qword ptr [rax + 0x10]
000da2d8: test   rcx, rcx
000da2db: je     0x1800db188
000da2e1: mov    rdx, qword ptr [rcx + 0x38]
000da2e5: test   rdx, rdx
000da2e8: je     0x1800db183
000da2ee: mov    rax, qword ptr [rip + 0x3bbe02b]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000da2f5: mov    rcx, qword ptr [rax + 0xb8]
000da2fc: mov    rcx, qword ptr [rcx + 0x218]
000da303: test   rcx, rcx
000da306: je     0x1800db17e
000da30c: mov    r14d, dword ptr [rdx + 0x10]
000da310: xor    edx, edx
000da312: call   0x18210c190                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʺʿʸʼʼˁˁʼˀʵʹ
000da317: movaps xmm7, xmm0
000da31a: mov    rax, qword ptr [rip + 0x3bbdfff]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000da321: mov    rcx, qword ptr [rax + 0xb8]
000da328: mov    rcx, qword ptr [rcx + 0x210]
000da32f: test   rcx, rcx
000da332: je     0x1800db179
000da338: xor    edx, edx
000da33a: call   0x18210c190                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʺʿʸʼʼˁˁʼˀʵʹ
000da33f: movaps xmm6, xmm0
000da342: xor    edx, edx
000da344: mov    rcx, rdi
000da347: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da34c: test   rax, rax
000da34f: je     0x1800db174
000da355: mov    rdx, qword ptr [rax + 0x120]
000da35c: test   rdx, rdx
000da35f: je     0x1800da381
000da361: mov    rcx, rbx
000da364: mov    rax, qword ptr [rip + 0x3bbeced]         ; [0x3c99058] meta:ʺʺʸʶʿʵˀʸʲˀʲ_TypeInfo
000da36b: cmp    qword ptr [rdx], rax
000da36e: cmove  rcx, rdx
000da372: test   rcx, rcx
000da375: je     0x1800da381
000da377: movzx  r15d, byte ptr [rcx + 0x2f6]
000da37f: jmp    0x1800da3a4
000da381: mov    r15d, ebx
000da384: mov    rax, qword ptr [rsp + 0x80]
000da38c: mov    qword ptr [rsp + 0x80], rax
000da394: mov    rax, qword ptr [rsp + 0xc0]
000da39c: mov    qword ptr [rsp + 0xc0], rax
000da3a4: test   r14d, r14d
000da3a7: mov    r14d, dword ptr [rsp + 0x2c8]
000da3af: setne  sil
000da3b3: xor    edx, edx
000da3b5: mov    rcx, rdi
000da3b8: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da3bd: test   rax, rax
000da3c0: je     0x1800db16f
000da3c6: mov    rax, qword ptr [rax + 0x88]
000da3cd: test   rax, rax
000da3d0: je     0x1800db16a
000da3d6: mov    rcx, qword ptr [rax + 0x10]
000da3da: test   rcx, rcx
000da3dd: je     0x1800db165
000da3e3: mov    rax, qword ptr [rcx + 0xd8]
000da3ea: test   rax, rax
000da3ed: je     0x1800db160
000da3f3: movups xmmword ptr [rsp + 0x130], xmm8
000da3fc: movups xmmword ptr [rsp + 0x140], xmm9
000da405: movups xmmword ptr [rsp + 0x150], xmm10
000da40e: movups xmmword ptr [rsp + 0x160], xmm11
000da417: movups xmmword ptr [rsp + 0x170], xmm12
000da420: movups xmmword ptr [rsp + 0x180], xmm13
000da429: movups xmmword ptr [rsp + 0x190], xmm14
000da432: movups xmmword ptr [rsp + 0x1a0], xmm15
000da43b: movups xmm0, xmmword ptr [rsp + 0x100]
000da443: movups xmmword ptr [rsp + 0x1b0], xmm0
000da44b: movups xmm1, xmmword ptr [rsp + 0x90]
000da453: movups xmmword ptr [rsp + 0x1c0], xmm1
000da45b: test   r15d, r15d
000da45e: setne  cl
000da461: mov    qword ptr [rsp + 0x70], rbx
000da466: mov    byte ptr [rsp + 0x68], 3
000da46b: mov    eax, dword ptr [rax + 0x10]
000da46e: mov    dword ptr [rsp + 0x60], eax
000da472: mov    byte ptr [rsp + 0x58], cl
000da476: movsd  qword ptr [rsp + 0x50], xmm6
000da47c: movsd  qword ptr [rsp + 0x48], xmm7
000da482: mov    byte ptr [rsp + 0x40], sil
000da487: mov    dword ptr [rsp + 0x38], r12d
000da48c: mov    qword ptr [rsp + 0x30], rbx
000da491: mov    dword ptr [rsp + 0x28], r13d
000da496: mov    byte ptr [rsp + 0x20], r14b
000da49b: xor    r9d, r9d
000da49e: mov    r8, qword ptr [rsp + 0x80]
000da4a6: mov    rdx, qword ptr [rsp + 0xc0]
000da4ae: lea    rcx, [rsp + 0x130]
000da4b6: call   0x1820f2a00                              ; ʾʾʷʴʶʽʾʴˁʼʶ$$ʺʼʻʼʴʺʶʽʳʸʳ
000da4bb: mov    r14, rax
000da4be: mov    rcx, qword ptr [rip + 0x3bc9a6b]         ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
000da4c5: call   0x182f60c00
000da4ca: mov    rsi, rax
000da4cd: xor    edx, edx
000da4cf: mov    rcx, rax
000da4d2: call   0x181af8fa0                              ; System.IO.MemoryStream$$.ctor
000da4d7: mov    rcx, qword ptr [rsp + 0x2b0]
000da4df: mov    qword ptr [rcx + 0x28], rsi
000da4e3: mov    rcx, qword ptr [rsp + 0x2b0]
000da4eb: add    rcx, 0x28
000da4ef: mov    rdx, rsi
000da4f2: call   0x182f5fc00
000da4f7: mov    qword ptr [rsp + 0x100], rbx
000da4ff: lea    rax, [rsp + 0x2c0]
000da507: mov    qword ptr [rsp + 0x108], rax
000da50f: lea    rax, [rsp + 0x2b0]
000da517: mov    qword ptr [rsp + 0x110], rax
000da51f: mov    eax, dword ptr [rsp + 0x2c0]
000da526: test   eax, eax
000da528: jne    0x1800da5ae
000da52e: mov    rax, qword ptr [rsp + 0x2b0]
000da536: movups xmm6, xmmword ptr [rax + 0x38]
000da53a: movups xmm0, xmmword ptr [rax + 0x48]
000da53e: movups xmmword ptr [rsp + 0xd0], xmm0
000da546: movsd  xmm1, qword ptr [rax + 0x58]
000da54b: movsd  qword ptr [rsp + 0xe0], xmm1
000da554: xorps  xmm0, xmm0
000da557: xor    ecx, ecx
000da559: movups xmmword ptr [rax + 0x38], xmm0
000da55d: movups xmmword ptr [rax + 0x48], xmm0
000da561: mov    qword ptr [rax + 0x58], rcx
000da565: mov    dword ptr [rsp + 0x2c0], 0xffffffff
000da570: mov    rax, qword ptr [rsp + 0x2b0]
000da578: mov    dword ptr [rax], 0xffffffff
000da57e: jmp    0x1800da98f
000da583: mov    rcx, qword ptr [rip + 0x3bfddc6]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000da58a: cmp    dword ptr [rcx + 0xe0], 0
000da591: jne    0x1800da598
000da593: call   0x182f60cf0
000da598: xor    edx, edx
000da59a: mov    rcx, qword ptr [rip + 0x3c1319f]         ; [0x3ced740] str:'No valid main player found for leaderboard session'
000da5a1: call   0x18285f400                              ; UnityEngine.Debug$$LogError
000da5a6: xor    dil, dil
000da5a9: jmp    0x1800daf32
000da5ae: cmp    eax, 1
000da5b1: jne    0x1800da5e4
000da5b3: mov    rax, qword ptr [rsp + 0x2b0]
000da5bb: movups xmm6, xmmword ptr [rax + 0x60]
000da5bf: xorps  xmm0, xmm0
000da5c2: movups xmmword ptr [rax + 0x60], xmm0
000da5c6: mov    dword ptr [rsp + 0x2c0], 0xffffffff
000da5d1: mov    rax, qword ptr [rsp + 0x2b0]
000da5d9: mov    dword ptr [rax], 0xffffffff
000da5df: jmp    0x1800dada6
000da5e4: xor    r8d, r8d
000da5e7: mov    rdx, r14
000da5ea: mov    rcx, qword ptr [rsp + 0x2b0]
000da5f2: mov    rcx, qword ptr [rcx + 0x28]
000da5f6: call   0x1820f2720                              ; ʾʾʷʴʶʽʾʴˁʼʶ$$ʷʷʿʷʹʳʸʿʿʸˁ
000da5fb: mov    rax, qword ptr [rsp + 0x2b0]
000da603: mov    rcx, qword ptr [rax + 0x28]
000da607: test   rcx, rcx
000da60a: je     0x1800db24c
000da610: mov    rax, qword ptr [rcx]
000da613: mov    rdx, qword ptr [rax + 0x420]
000da61a: call   qword ptr [rax + 0x418]
000da620: mov    r14, rax
000da623: test   rdi, rdi
000da626: je     0x1800db247
000da62c: xor    edx, edx
000da62e: mov    rcx, rdi
000da631: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da636: test   rax, rax
000da639: je     0x1800db242
000da63f: mov    rax, qword ptr [rax + 0x120]
000da646: test   rax, rax
000da649: je     0x1800db23d
000da64f: mov    rdx, qword ptr [rax + 0x230]
000da656: test   rdx, rdx
000da659: je     0x1800db238
000da65f: xor    r8d, r8d
000da662: lea    rcx, [rsp + 0x90]
000da66a: call   0x18212c9e0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʷʻʿʽʲʾˀˀʸʹʾ
000da66f: movups xmm6, xmmword ptr [rax]
000da672: movups xmm7, xmmword ptr [rax + 0x10]
000da676: mov    rcx, qword ptr [rip + 0x3bc355b]         ; [0x3c9dbd8] meta:ʾˁʸˁˁʻʿˁˁʲʽ_TypeInfo
000da67d: cmp    dword ptr [rcx + 0xe0], 0
000da684: jne    0x1800da68b
000da686: call   0x182f60cf0
000da68b: movaps xmmword ptr [rsp + 0x90], xmm6
000da693: movaps xmmword ptr [rsp + 0xa0], xmm7
000da69b: xor    edx, edx
000da69d: lea    rcx, [rsp + 0x90]
000da6a5: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
000da6aa: mov    rsi, rax
000da6ad: xor    edx, edx
000da6af: mov    rcx, rdi
000da6b2: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da6b7: test   rax, rax
000da6ba: je     0x1800db233
000da6c0: mov    rax, qword ptr [rax + 0x120]
000da6c7: test   rax, rax
000da6ca: je     0x1800db22e
000da6d0: mov    rdx, qword ptr [rax + 0x230]
000da6d7: test   rdx, rdx
000da6da: je     0x1800db229
000da6e0: xor    r8d, r8d
000da6e3: lea    rcx, [rsp + 0xc0]
000da6eb: call   0x18212da90                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʽʻʴʶʺˁʳʸʸʳʿ
000da6f0: movups xmm0, xmmword ptr [rax]
000da6f3: movaps xmmword ptr [rsp + 0x90], xmm0
000da6fb: movups xmm1, xmmword ptr [rax + 0x10]
000da6ff: movaps xmmword ptr [rsp + 0xa0], xmm1
000da707: xor    edx, edx
000da709: lea    rcx, [rsp + 0x90]
000da711: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
000da716: xor    edx, edx
000da718: mov    rcx, rdi
000da71b: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da720: test   rax, rax
000da723: je     0x1800db224
000da729: mov    rcx, qword ptr [rax + 0x120]
000da730: test   rcx, rcx
000da733: je     0x1800db21f
000da739: xor    edx, edx
000da73b: call   0x1820f5110                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʸʾʻʶʻʶʵʾʾʺˀ
000da740: test   rax, rax
000da743: je     0x1800db21a
000da749: xor    r8d, r8d
000da74c: mov    rdx, rax
000da74f: lea    rcx, [rsp + 0xc0]
000da757: call   0x18215f280                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʽʳʸʿʷʽʼˁʸʻʼ
000da75c: movups xmm0, xmmword ptr [rax]
000da75f: movaps xmmword ptr [rsp + 0x90], xmm0
000da767: movups xmm1, xmmword ptr [rax + 0x10]
000da76b: movaps xmmword ptr [rsp + 0xa0], xmm1
000da773: xor    edx, edx
000da775: lea    rcx, [rsp + 0x90]
000da77d: call   0x18210cfe0                              ; ʾˁʸˁˁʻʿˁˁʲʽ$$ʼʶʷʷʼʿʴʴʺʵʵ
000da782: mov    rcx, qword ptr [rsp + 0x2b0]
000da78a: mov    qword ptr [rcx + 0x30], rax
000da78e: mov    rcx, qword ptr [rsp + 0x2b0]
000da796: add    rcx, 0x30
000da79a: mov    rdx, rax
000da79d: call   0x182f5fc00
000da7a2: xor    edx, edx
000da7a4: mov    rcx, rdi
000da7a7: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000da7ac: test   rax, rax
000da7af: je     0x1800db215
000da7b5: mov    r10, qword ptr [rax]
000da7b8: mov    rcx, qword ptr [r10 + 0x430]
000da7bf: mov    qword ptr [rsp + 0x20], rcx
000da7c4: mov    r9, rsi
000da7c7: mov    r8, r14
000da7ca: mov    rdx, rax
000da7cd: lea    rcx, [rsp + 0x90]
000da7d5: call   qword ptr [r10 + 0x428]
000da7dc: movups xmm6, xmmword ptr [rax]
000da7df: movups xmm7, xmmword ptr [rax + 0x10]
000da7e3: movsd  xmm8, qword ptr [rax + 0x20]
000da7e9: xorps  xmm0, xmm0
000da7ec: xor    eax, eax
000da7ee: movups xmmword ptr [rsp + 0x90], xmm0
000da7f6: movups xmmword ptr [rsp + 0xa0], xmm0
000da7fe: mov    qword ptr [rsp + 0xb0], rax
000da806: mov    rax, qword ptr [rip + 0x3bfdc13]         ; [0x3cd8420] metamethod:Method$Cysharp.Threading.Tasks.UniTask<ValueTuple<bool, string, string>>.GetAwaiter()
000da80d: mov    rcx, qword ptr [rax + 0x20]
000da811: test   byte ptr [rcx + 0x135], 1
000da818: jne    0x1800da81f
000da81a: call   0x182f65750
000da81f: movups xmmword ptr [rsp + 0x90], xmm6
000da827: movups xmmword ptr [rsp + 0xa0], xmm7
000da82f: movsd  qword ptr [rsp + 0xb0], xmm8
000da839: xor    edx, edx
000da83b: lea    rcx, [rsp + 0x90]
000da843: call   0x182f5fc00
000da848: movups xmm6, xmmword ptr [rsp + 0x90]
000da850: movups xmmword ptr [rsp + 0xc0], xmm6
000da858: movups xmm7, xmmword ptr [rsp + 0xa0]
000da860: movups xmmword ptr [rsp + 0xd0], xmm7
000da868: movsd  xmm8, qword ptr [rsp + 0xb0]
000da872: movsd  qword ptr [rsp + 0xe0], xmm8
000da87c: mov    rax, qword ptr [rip + 0x3bf57b5]         ; [0x3cd0038] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ValueTuple<bool, string, string>>.get_IsCompleted()
000da883: mov    rax, qword ptr [rax + 0x20]
000da887: test   byte ptr [rax + 0x135], 1
000da88e: jne    0x1800da898
000da890: mov    rcx, rax
000da893: call   0x182f65750
000da898: movq   rsi, xmm6
000da89d: test   rsi, rsi
000da8a0: je     0x1800da98f
000da8a6: mov    rax, qword ptr [rax + 0xc0]
000da8ad: mov    rcx, qword ptr [rax + 0x18]
000da8b1: mov    rax, qword ptr [rcx + 0x20]
000da8b5: test   byte ptr [rax + 0x135], 1
000da8bc: jne    0x1800da8c6
000da8be: mov    rcx, rax
000da8c1: call   0x182f65750
000da8c6: mov    rax, qword ptr [rax + 0xc0]
000da8cd: mov    rax, qword ptr [rax + 8]
000da8d1: test   byte ptr [rax + 0x135], 1
000da8d8: jne    0x1800da8e2
000da8da: mov    rcx, rax
000da8dd: call   0x182f65750
000da8e2: mov    ecx, 1
000da8e7: movzx  r9d, word ptr [rsp + 0xb0]
000da8f0: mov    r8, rsi
000da8f3: mov    rdx, rax
000da8f6: call   0x182c40ac0
000da8fb: test   eax, eax
000da8fd: jne    0x1800da98f
000da903: mov    dword ptr [rsp + 0x2c0], ebx
000da90a: mov    rax, qword ptr [rsp + 0x2b0]
000da912: mov    dword ptr [rax], ebx
000da914: mov    rax, qword ptr [rsp + 0x2b0]
000da91c: movups xmmword ptr [rax + 0x38], xmm6
000da920: movups xmmword ptr [rax + 0x48], xmm7
000da924: movsd  qword ptr [rax + 0x58], xmm8
000da92a: mov    rcx, qword ptr [rsp + 0x2b0]
000da932: add    rcx, 0x38
000da936: xor    edx, edx
000da938: call   0x182f5fc00
000da93d: mov    r8, qword ptr [rsp + 0x2b0]
000da945: lea    rcx, [r8 + 8]
000da949: mov    r9, qword ptr [rip + 0x3bf0550]          ; [0x3ccaea0] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.AwaitUnsafeOnCompleted<UniTask.Awaiter<ValueTuple<bool, string, string>>, GameManager.ʾʿʷʵʿʷʺˀʿʹˀ>()
000da950: lea    rdx, [rsp + 0xc0]
000da958: call   0x1800ced60
000da95d: nop    
000da95e: cmp    dword ptr [rsp + 0x2c0], 0
000da966: jge    0x1800da98a
000da968: mov    r8, qword ptr [rsp + 0x2b0]
000da970: cmp    qword ptr [r8 + 0x28], 0
000da975: je     0x1800da98a
000da977: xor    ecx, ecx
000da979: mov    r8, qword ptr [r8 + 0x28]
000da97d: mov    rdx, qword ptr [rip + 0x3c2334c]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000da984: call   0x182c42550
000da989: nop    
000da98a: jmp    0x1800db01f
000da98f: movq   rsi, xmm6
000da994: test   rsi, rsi
000da997: je     0x1800daa4c
000da99d: mov    rax, qword ptr [rip + 0x3bf55d4]         ; [0x3ccff78] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ValueTuple<bool, string, string>>.GetResult()
000da9a4: mov    rax, qword ptr [rax + 0x20]
000da9a8: test   byte ptr [rax + 0x135], 1
000da9af: jne    0x1800da9b9
000da9b1: mov    rcx, rax
000da9b4: call   0x182f65750
000da9b9: mov    rax, qword ptr [rax + 0xc0]
000da9c0: mov    rdx, qword ptr [rax + 0x28]
000da9c4: test   byte ptr [rdx + 0x135], 1
000da9cb: jne    0x1800da9d8
000da9cd: mov    rcx, rdx
000da9d0: call   0x182f65750
000da9d5: mov    rdx, rax
000da9d8: mov    r10, qword ptr [rsi]
000da9db: movzx  r8d, word ptr [r10 + 0x12e]
000da9e3: cmp    bx, r8w
000da9e7: jae    0x1800daa08
000da9e9: movzx  ecx, bx
000da9ec: mov    r9, qword ptr [r10 + 0xb0]
000da9f3: movzx  eax, cx
000da9f6: add    rax, rax
000da9f9: cmp    qword ptr [r9 + rax*8], rdx
000da9fd: je     0x1800daa32
000da9ff: inc    cx
000daa02: cmp    cx, r8w
000daa06: jb     0x1800da9f3
000daa08: xor    r8d, r8d
000daa0b: mov    rcx, rsi
000daa0e: call   0x182f65070
000daa13: mov    r9, qword ptr [rax + 8]
000daa17: movzx  r8d, word ptr [rsp + 0xe0]
000daa20: mov    rdx, rsi
000daa23: lea    rcx, [rsp + 0x90]
000daa2b: call   qword ptr [rax]
000daa2d: movzx  eax, byte ptr [rax]
000daa30: jmp    0x1800daa55
000daa32: movzx  ecx, cx
000daa35: add    rcx, rcx
000daa38: movsxd rax, dword ptr [r9 + rcx*8 + 8]
000daa3d: shl    rax, 4
000daa41: add    rax, 0x138
000daa47: add    rax, r10
000daa4a: jmp    0x1800daa13
000daa4c: psrldq xmm6, 8
000daa51: movd   eax, xmm6
000daa55: test   al, al
000daa57: je     0x1800dac25
000daa5d: mov    rcx, qword ptr [rip + 0x3bd8204]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000daa64: cmp    dword ptr [rcx + 0xe0], 0
000daa6b: jne    0x1800daa72
000daa6d: call   0x182f60cf0
000daa72: mov    rcx, qword ptr [rip + 0x3bd7087]         ; [0x3cb1b00] metamethod:Method$UnityEngine.Object.FindObjectOfType<InGameLeaderboardManager>()
000daa79: call   0x1804e6370                              ; UnityEngine.Object$$FindObjectOfType<object>
000daa7e: mov    rsi, rax
000daa81: test   rax, rax
000daa84: jne    0x1800daaad
000daa86: mov    rcx, qword ptr [rip + 0x3bfd8c3]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000daa8d: cmp    dword ptr [rcx + 0xe0], eax
000daa93: jne    0x1800daa9a
000daa95: call   0x182f60cf0
000daa9a: xor    edx, edx
000daa9c: mov    rcx, qword ptr [rip + 0x3beb19d]         ; [0x3cc5c40] str:'Failed to show leaderboards in game. InGameLeaderboardManager not found.'
000daaa3: call   0x18285f400                              ; UnityEngine.Debug$$LogError
000daaa8: jmp    0x1800dabf1
000daaad: xor    ecx, ecx
000daaaf: call   0x180139dd0                              ; LeaderboardsOnlineManager$$ˀʸʲʼˁʶʹʾʾʲʺ
000daab4: test   rax, rax
000daab7: je     0x1800db1f7
000daabd: mov    rcx, qword ptr [rip + 0x3bbd85c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000daac4: mov    rdx, qword ptr [rcx + 0xb8]
000daacb: mov    rcx, qword ptr [rdx + 0x10]
000daacf: test   rcx, rcx
000daad2: je     0x1800db1f2
000daad8: test   rdi, rdi
000daadb: je     0x1800db1ed
000daae1: mov    rdx, qword ptr [rax + 0x20]
000daae5: mov    qword ptr [rsp + 0x80], rdx
000daaed: mov    rax, qword ptr [rsp + 0x2b0]
000daaf5: mov    rax, qword ptr [rax + 0x30]
000daaf9: mov    qword ptr [rsp + 0xc0], rax
000dab01: mov    eax, dword ptr [rcx + 0x10]
000dab04: mov    dword ptr [rsp + 0x2c8], eax
000dab0b: xor    edx, edx
000dab0d: mov    rcx, rdi
000dab10: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000dab15: test   rax, rax
000dab18: je     0x1800db1e8
000dab1e: mov    r14, qword ptr [rax + 0x88]
000dab25: mov    rcx, qword ptr [rip + 0x3c3372c]         ; [0x3d0e258] meta:ʲʻʳʶʲʼʸʴʾʲʳ_TypeInfo
000dab2c: cmp    dword ptr [rcx + 0xe0], 0
000dab33: jne    0x1800dab3a
000dab35: call   0x182f60cf0
000dab3a: xor    edx, edx
000dab3c: mov    rcx, r14
000dab3f: call   0x1803b3570                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʿʵʿʲʹʹʽʶʼʾˀ
000dab44: mov    r13, rax
000dab47: xor    edx, edx
000dab49: mov    rcx, rdi
000dab4c: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000dab51: test   rax, rax
000dab54: je     0x1800db1e3
000dab5a: xor    edx, edx
000dab5c: mov    rcx, qword ptr [rax + 0x88]
000dab63: call   0x1803b1000                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʸʴʻʾʽʻʽʵʿʺʿ
000dab68: mov    r12, rax
000dab6b: xor    edx, edx
000dab6d: mov    rcx, rdi
000dab70: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000dab75: test   rax, rax
000dab78: je     0x1800db1de
000dab7e: mov    r14, qword ptr [rax + 0x120]
000dab85: xor    edx, edx
000dab87: mov    rcx, rdi
000dab8a: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000dab8f: mov    r15, rax
000dab92: xor    edx, edx
000dab94: mov    rcx, rdi
000dab97: call   0x1800b5db0                              ; GameManager$$ʴʻʴʳʲʵʲʾʸʲʾ
000dab9c: test   rax, rax
000dab9f: je     0x1800db1d9
000daba5: xor    edx, edx
000daba7: mov    rcx, qword ptr [rax + 0x88]
000dabae: call   0x1803b2e30                              ; ʲʻʳʶʲʼʸʴʾʲʳ$$ʾʻʶʻʵʵʳʹʶʾʽ
000dabb3: mov    qword ptr [rsp + 0x48], rbx
000dabb8: mov    qword ptr [rsp + 0x40], rax
000dabbd: mov    qword ptr [rsp + 0x38], r15
000dabc2: mov    qword ptr [rsp + 0x30], r14
000dabc7: mov    qword ptr [rsp + 0x28], r12
000dabcc: mov    qword ptr [rsp + 0x20], r13
000dabd1: mov    r9d, dword ptr [rsp + 0x2c8]
000dabd9: mov    r8, qword ptr [rsp + 0xc0]
000dabe1: mov    rdx, qword ptr [rsp + 0x80]
000dabe9: mov    rcx, rsi
000dabec: call   0x1803723b0                              ; InGameLeaderboardManager$$ʾʶʲʷʹʻʾʾʺʸʿ
000dabf1: mov    dil, 1
000dabf4: cmp    dword ptr [rsp + 0x2c0], 0
000dabfc: jge    0x1800dac20
000dabfe: mov    r8, qword ptr [rsp + 0x2b0]
000dac06: cmp    qword ptr [r8 + 0x28], 0
000dac0b: je     0x1800dac20
000dac0d: xor    ecx, ecx
000dac0f: mov    r8, qword ptr [r8 + 0x28]
000dac13: mov    rdx, qword ptr [rip + 0x3c230b6]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000dac1a: call   0x182c42550
000dac1f: nop    
000dac20: jmp    0x1800daf32
000dac25: mov    rcx, qword ptr [rip + 0x3bba2d4]         ; [0x3c94f00] meta:ʶʼʾʻˁʳʲʹˁʿʷ_TypeInfo
000dac2c: cmp    dword ptr [rcx + 0xe0], 0
000dac33: jne    0x1800dac3a
000dac35: call   0x182f60cf0
000dac3a: xor    edx, edx
000dac3c: lea    rcx, [rsp + 0x90]
000dac44: call   0x1802ebed0                              ; ʶʼʾʻˁʳʲʹˁʿʷ$$ʶʳʴʹʴʸʳˁʴʿʸ
000dac49: movups xmm6, xmmword ptr [rax]
000dac4c: xorps  xmm0, xmm0
000dac4f: movups xmmword ptr [rsp + 0xc0], xmm0
000dac57: mov    rax, qword ptr [rip + 0x3bff082]         ; [0x3cd9ce0] metamethod:Method$Cysharp.Threading.Tasks.UniTask<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.GetAwaiter()
000dac5e: mov    rcx, qword ptr [rax + 0x20]
000dac62: test   byte ptr [rcx + 0x135], 1
000dac69: jne    0x1800dac70
000dac6b: call   0x182f65750
000dac70: movdqa xmmword ptr [rsp + 0xc0], xmm6
000dac79: xor    edx, edx
000dac7b: lea    rcx, [rsp + 0xc0]
000dac83: call   0x182f5fc00
000dac88: movaps xmm6, xmmword ptr [rsp + 0xc0]
000dac90: movaps xmmword ptr [rsp + 0x90], xmm6
000dac98: mov    rax, qword ptr [rip + 0x3bf6419]         ; [0x3cd10b8] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.get_IsCompleted()
000dac9f: mov    rax, qword ptr [rax + 0x20]
000daca3: test   byte ptr [rax + 0x135], 1
000dacaa: jne    0x1800dacb4
000dacac: mov    rcx, rax
000dacaf: call   0x182f65750
000dacb4: movq   rdi, xmm6
000dacb9: test   rdi, rdi
000dacbc: je     0x1800dada6
000dacc2: mov    rax, qword ptr [rax + 0xc0]
000dacc9: mov    rcx, qword ptr [rax + 0x18]
000daccd: mov    rax, qword ptr [rcx + 0x20]
000dacd1: test   byte ptr [rax + 0x135], 1
000dacd8: jne    0x1800dace2
000dacda: mov    rcx, rax
000dacdd: call   0x182f65750
000dace2: mov    rax, qword ptr [rax + 0xc0]
000dace9: mov    rax, qword ptr [rax + 8]
000daced: test   byte ptr [rax + 0x135], 1
000dacf4: jne    0x1800dacfe
000dacf6: mov    rcx, rax
000dacf9: call   0x182f65750
000dacfe: mov    ecx, 1
000dad03: pextrw r9d, xmm6, 6
000dad09: mov    r8, rdi
000dad0c: mov    rdx, rax
000dad0f: call   0x182c40ac0
000dad14: test   eax, eax
000dad16: jne    0x1800dada6
000dad1c: mov    dword ptr [rsp + 0x2c0], 1
000dad27: mov    rax, qword ptr [rsp + 0x2b0]
000dad2f: mov    dword ptr [rax], 1
000dad35: mov    rax, qword ptr [rsp + 0x2b0]
000dad3d: movups xmmword ptr [rax + 0x60], xmm6
000dad41: mov    rcx, qword ptr [rsp + 0x2b0]
000dad49: add    rcx, 0x60
000dad4d: xor    edx, edx
000dad4f: call   0x182f5fc00
000dad54: mov    r8, qword ptr [rsp + 0x2b0]
000dad5c: lea    rcx, [r8 + 8]
000dad60: mov    r9, qword ptr [rip + 0x3bf02b9]          ; [0x3ccb020] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.AwaitUnsafeOnCompleted<UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>, GameManager.ʾʿʷʵʿʷʺˀʿʹˀ>()
000dad67: lea    rdx, [rsp + 0x90]
000dad6f: call   0x1800cebc0
000dad74: nop    
000dad75: cmp    dword ptr [rsp + 0x2c0], 0
000dad7d: jge    0x1800dada1
000dad7f: mov    r8, qword ptr [rsp + 0x2b0]
000dad87: cmp    qword ptr [r8 + 0x28], 0
000dad8c: je     0x1800dada1
000dad8e: xor    ecx, ecx
000dad90: mov    r8, qword ptr [r8 + 0x28]
000dad94: mov    rdx, qword ptr [rip + 0x3c22f35]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000dad9b: call   0x182c42550
000dada0: nop    
000dada1: jmp    0x1800db01f
000dada6: movq   rdi, xmm6
000dadab: test   rdi, rdi
000dadae: je     0x1800dadfd
000dadb0: mov    rax, qword ptr [rip + 0x3bf6241]         ; [0x3cd0ff8] metamethod:Method$Cysharp.Threading.Tasks.UniTask.Awaiter<ʶʼʾʻˁʳʲʹˁʿʷ.ʷʶʺʳˀʳʹʻʶʴʸ>.GetResult()
000dadb7: mov    rax, qword ptr [rax + 0x20]
000dadbb: test   byte ptr [rax + 0x135], 1
000dadc2: jne    0x1800dadcc
000dadc4: mov    rcx, rax
000dadc7: call   0x182f65750
000dadcc: mov    rax, qword ptr [rax + 0xc0]
000dadd3: mov    rax, qword ptr [rax + 0x28]
000dadd7: test   byte ptr [rax + 0x135], 1
000dadde: jne    0x1800dade8
000dade0: mov    rcx, rax
000dade3: call   0x182f65750
000dade8: xor    ecx, ecx
000dadea: pextrw r9d, xmm6, 6
000dadf0: mov    r8, rdi
000dadf3: mov    rdx, rax
000dadf6: call   0x182c40ac0
000dadfb: jmp    0x1800dae07
000dadfd: psrldq xmm6, 8
000dae02: movq   rax, xmm6
000dae07: cmp    eax, 1
000dae0a: jne    0x1800dae33
000dae0c: xor    ecx, ecx
000dae0e: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
000dae13: test   rax, rax
000dae16: je     0x1800db1fc
000dae1c: xor    r8d, r8d
000dae1f: mov    rdx, qword ptr [rip + 0x3bf2eca]         ; [0x3ccdcf0] str:'leaderboardsServersFull'
000dae26: mov    rcx, rax
000dae29: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
000dae2e: mov    rdi, rax
000dae31: jmp    0x1800dae66
000dae33: cmp    eax, 2
000dae36: jne    0x1800dae5f
000dae38: xor    ecx, ecx
000dae3a: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
000dae3f: test   rax, rax
000dae42: je     0x1800db201
000dae48: xor    r8d, r8d
000dae4b: mov    rdx, qword ptr [rip + 0x3bf2ade]         ; [0x3ccd930] str:'leaderboardsNoServersAvailable'
000dae52: mov    rcx, rax
000dae55: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
000dae5a: mov    rdi, rax
000dae5d: jmp    0x1800dae66
000dae5f: mov    rdi, qword ptr [rip + 0x3bd900a]         ; [0x3cb3e70] str:''
000dae66: xor    ecx, ecx
000dae68: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
000dae6d: test   rax, rax
000dae70: je     0x1800db210
000dae76: xor    r8d, r8d
000dae79: mov    rdx, qword ptr [rip + 0x3c086b8]         ; [0x3ce3538] str:'unableToJoinLeaderboardSession'
000dae80: mov    rcx, rax
000dae83: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
000dae88: mov    qword ptr [rsp + 0x20], rbx
000dae8d: movss  xmm3, dword ptr [rip + 0x2f89143]        ; [0x3063fd8] flt=5.0 i=1084227584
000dae95: xor    r8d, r8d
000dae98: mov    rdx, rdi
000dae9b: mov    rcx, rax
000dae9e: call   0x180108660                              ; NotificationPopups$$ʶʸʲʸʸˁʵʻʼʴʽ
000daea3: mov    rax, qword ptr [rip + 0x3c149e6]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000daeaa: cmp    dword ptr [rax + 0xe0], 0
000daeb1: jne    0x1800daec2
000daeb3: mov    rcx, rax
000daeb6: call   0x182f60cf0
000daebb: mov    rax, qword ptr [rip + 0x3c149ce]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000daec2: mov    rax, qword ptr [rax + 0xb8]
000daec9: mov    rcx, qword ptr [rax + 8]
000daecd: test   rcx, rcx
000daed0: je     0x1800db20b
000daed6: mov    byte ptr [rcx + 0x75], 1
000daeda: mov    rax, qword ptr [rip + 0x3c0d1c7]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000daee1: mov    rcx, qword ptr [rax + 0xb8]
000daee8: mov    rcx, qword ptr [rcx]
000daeeb: test   rcx, rcx
000daeee: je     0x1800db206
000daef4: xor    r8d, r8d
000daef7: mov    rdx, qword ptr [rip + 0x3bf0b32]         ; [0x3ccba30] str:'Main Menu'
000daefe: call   0x1802583f0                              ; FadeBehaviour$$ʻʲʿʴʺʳˁʷʶʵʾ
000daf03: xor    dil, dil
000daf06: cmp    dword ptr [rsp + 0x2c0], 0
000daf0e: jge    0x1800daf32
000daf10: mov    r8, qword ptr [rsp + 0x2b0]
000daf18: cmp    qword ptr [r8 + 0x28], 0
000daf1d: je     0x1800daf32
000daf1f: xor    ecx, ecx
000daf21: mov    r8, qword ptr [r8 + 0x28]
000daf25: mov    rdx, qword ptr [rip + 0x3c22da4]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000daf2c: call   0x182c42550
000daf31: nop    
000daf32: mov    rax, qword ptr [rsp + 0x2b0]
000daf3a: mov    dword ptr [rax], 0xfffffffe
000daf40: mov    rax, qword ptr [rsp + 0x2b0]
000daf48: mov    qword ptr [rax + 0x28], rbx
000daf4c: mov    rcx, qword ptr [rsp + 0x2b0]
000daf54: add    rcx, 0x28
000daf58: xor    edx, edx
000daf5a: call   0x182f5fc00
000daf5f: mov    rax, qword ptr [rsp + 0x2b0]
000daf67: mov    qword ptr [rax + 0x30], rbx
000daf6b: mov    rcx, qword ptr [rsp + 0x2b0]
000daf73: add    rcx, 0x30
000daf77: xor    edx, edx
000daf79: call   0x182f5fc00
000daf7e: mov    rax, qword ptr [rsp + 0x2b0]
000daf86: cmp    qword ptr [rax + 8], 0
000daf8b: je     0x1800db08d
000daf91: mov    rsi, qword ptr [rax + 8]
000daf95: mov    rax, qword ptr [rip + 0x3bf0444]         ; [0x3ccb3e0] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.SetResult()
000daf9c: mov    rax, qword ptr [rax + 0x20]
000dafa0: test   byte ptr [rax + 0x135], 1
000dafa7: jne    0x1800dafb1
000dafa9: mov    rcx, rax
000dafac: call   0x182f65750
000dafb1: mov    rax, qword ptr [rax + 0xc0]
000dafb8: mov    rdx, qword ptr [rax + 8]
000dafbc: test   byte ptr [rdx + 0x135], 1
000dafc3: jne    0x1800dafd0
000dafc5: mov    rcx, rdx
000dafc8: call   0x182f65750
000dafcd: mov    rdx, rax
000dafd0: mov    r9, qword ptr [rsi]
000dafd3: movzx  ecx, word ptr [r9 + 0x12e]
000dafdb: cmp    bx, cx
000dafde: jae    0x1800db004
000dafe0: mov    r8, qword ptr [r9 + 0xb0]
000dafe7: nop    word ptr [rax + rax]
000daff0: movzx  eax, bx
000daff3: add    rax, rax
000daff6: cmp    qword ptr [r8 + rax*8], rdx
000daffa: je     0x1800db06e
000daffc: inc    bx
000dafff: cmp    bx, cx
000db002: jb     0x1800daff0
000db004: mov    r8d, 2
000db00a: mov    rcx, rsi
000db00d: call   0x182f65070
000db012: mov    r8, qword ptr [rax + 8]
000db016: movzx  edx, dil
000db01a: mov    rcx, rsi
000db01d: call   qword ptr [rax]
000db01f: lea    r11, [rsp + 0x270]
000db027: movaps xmm6, xmmword ptr [r11 - 0x10]
000db02c: movaps xmm7, xmmword ptr [r11 - 0x20]
000db031: movaps xmm8, xmmword ptr [r11 - 0x30]
000db036: movaps xmm9, xmmword ptr [r11 - 0x40]
000db03b: movaps xmm10, xmmword ptr [r11 - 0x50]
000db040: movaps xmm11, xmmword ptr [r11 - 0x60]
000db045: movaps xmm12, xmmword ptr [r11 - 0x70]
000db04a: movaps xmm13, xmmword ptr [r11 - 0x80]
000db04f: movaps xmm14, xmmword ptr [r11 - 0x90]
000db057: movaps xmm15, xmmword ptr [r11 - 0xa0]
000db05f: mov    rsp, r11
000db062: pop    r15
000db064: pop    r14
000db066: pop    r13
000db068: pop    r12
000db06a: pop    rdi
000db06b: pop    rsi
000db06c: pop    rbx
000db06d: ret    
000db06e: movzx  ecx, bx
000db071: add    rcx, rcx
000db074: mov    eax, dword ptr [r8 + rcx*8 + 8]
000db079: add    eax, 2
000db07c: cdqe   
000db07e: shl    rax, 4
000db082: add    rax, 0x138
000db088: add    rax, r9
000db08b: jmp    0x1800db012
000db08d: mov    byte ptr [rax + 0x18], dil
000db091: jmp    0x1800db01f
000db093: cmp    dword ptr [rsp + 0x2c0], 0
000db09b: jge    0x1800db0be
000db09d: mov    rax, qword ptr [rsp + 0x2b0]
000db0a5: cmp    qword ptr [rax + 0x28], 0
000db0aa: je     0x1800db0c6
000db0ac: xor    ecx, ecx
000db0ae: mov    r8, qword ptr [rax + 0x28]
000db0b2: mov    rdx, qword ptr [rip + 0x3c22c17]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
000db0b9: call   0x182c42550
000db0be: mov    rax, qword ptr [rsp + 0x2b0]
000db0c6: mov    rcx, qword ptr [rsp + 0x100]
000db0ce: test   rcx, rcx
000db0d1: jne    0x1800db252
000db0d7: jmp    0x1800db0e1
000db0d9: mov    rax, qword ptr [rsp + 0x2b0]
000db0e1: mov    dword ptr [rax], 0xfffffffe
000db0e7: xor    ebx, ebx
000db0e9: mov    rax, qword ptr [rsp + 0x2b0]
000db0f1: mov    qword ptr [rax + 0x28], rbx
000db0f5: mov    rcx, qword ptr [rsp + 0x2b0]
000db0fd: add    rcx, 0x28
000db101: xor    edx, edx
000db103: call   0x182f5fc00
000db108: mov    rax, qword ptr [rsp + 0x2b0]
000db110: mov    qword ptr [rax + 0x30], rbx
000db114: mov    rcx, qword ptr [rsp + 0x2b0]
000db11c: add    rcx, 0x30
000db120: xor    edx, edx
000db122: call   0x182f5fc00
000db127: mov    rbx, qword ptr [rsp + 0x2b0]
000db12f: lea    rcx, [rip + 0x3bf01ea]                   ; [0x3ccb320] metamethod:Method$Cysharp.Threading.Tasks.CompilerServices.AsyncUniTaskMethodBuilder<bool>.SetException()
000db136: call   0x182f609d0
000db13b: mov    r8, rax
000db13e: mov    eax, dword ptr [rsp + 0xf8]
000db145: dec    eax
000db147: movsxd rdx, eax
000db14a: mov    rdx, qword ptr [rsp + rdx*8 + 0xf0]
000db152: lea    rcx, [rbx + 8]
000db156: call   0x182c49c00
000db15b: jmp    0x1800db01f
000db160: call   0x182f60c50
000db165: call   0x182f60c50
000db16a: call   0x182f60c50
000db16f: call   0x182f60c50
000db174: call   0x182f60c50
000db179: call   0x182f60c50
000db17e: call   0x182f60c50
000db183: call   0x182f60c50
000db188: call   0x182f60c50
000db18d: call   0x182f60c50
000db192: call   0x182f60c50
000db197: call   0x182f60c50
000db19c: call   0x182f60c50
000db1a1: call   0x182f60c50
000db1a6: call   0x182f60c50
000db1ab: call   0x182f60c50
000db1b0: call   0x182f60c50
000db1b5: call   0x182f60c50
000db1ba: call   0x182f60c50
000db1bf: call   0x182f60c50
000db1c4: call   0x182f60c50
000db1c9: call   0x182f60c50
000db1ce: call   0x182f60c50
000db1d3: call   0x182f60c50
000db1d8: nop    
000db1d9: call   0x182f60c50
000db1de: call   0x182f60c50
000db1e3: call   0x182f60c50
000db1e8: call   0x182f60c50
000db1ed: call   0x182f60c50
000db1f2: call   0x182f60c50
000db1f7: call   0x182f60c50
000db1fc: call   0x182f60c50
000db201: call   0x182f60c50
000db206: call   0x182f60c50
000db20b: call   0x182f60c50
000db210: call   0x182f60c50
000db215: call   0x182f60c50
000db21a: call   0x182f60c50
000db21f: call   0x182f60c50
000db224: call   0x182f60c50
000db229: call   0x182f60c50
000db22e: call   0x182f60c50
000db233: call   0x182f60c50
000db238: call   0x182f60c50
000db23d: call   0x182f60c50
000db242: call   0x182f60c50
000db247: call   0x182f60c50
000db24c: call   0x182f60c50
000db251: nop    
000db252: call   0x182f60ce0
000db257: int3   
000db258: int3   
