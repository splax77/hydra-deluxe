00208070: mov    qword ptr [rsp + 0x10], rbx
00208075: mov    qword ptr [rsp + 0x20], rsi
0020807a: mov    qword ptr [rsp + 8], rcx
0020807f: push   rdi
00208080: sub    rsp, 0x70
00208084: movaps xmmword ptr [rsp + 0x60], xmm6
00208089: mov    rdi, rcx
0020808c: cmp    byte ptr [rip + 0x3d04c45], 0            ; [0x3f0ccd8] (bss)
00208093: jne    0x180208114
00208095: lea    rcx, [rip + 0x3ad6364]                   ; [0x3cde400] meta:double_TypeInfo
0020809c: call   0x182f609b0
002080a1: lea    rcx, [rip + 0x3ab1fd0]                   ; [0x3cba078] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.Dispose()
002080a8: call   0x182f609b0
002080ad: lea    rcx, [rip + 0x3ab207c]                   ; [0x3cba130] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.MoveNext()
002080b4: call   0x182f609b0
002080b9: lea    rcx, [rip + 0x3ab2128]                   ; [0x3cba1e8] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.get_Current()
002080c0: call   0x182f609b0
002080c5: lea    rcx, [rip + 0x3aad6f4]                   ; [0x3cb57c0] metamethod:Method$System.Collections.Generic.List<float>.Add()
002080cc: call   0x182f609b0
002080d1: lea    rcx, [rip + 0x3aad7a0]                   ; [0x3cb5878] metamethod:Method$System.Collections.Generic.List<float>.GetEnumerator()
002080d8: call   0x182f609b0
002080dd: lea    rcx, [rip + 0x3aad84c]                   ; [0x3cb5930] metamethod:Method$System.Collections.Generic.List<float>.RemoveAt()
002080e4: call   0x182f609b0
002080e9: lea    rcx, [rip + 0x3aad8f8]                   ; [0x3cb59e8] metamethod:Method$System.Collections.Generic.List<float>.get_Count()
002080f0: call   0x182f609b0
002080f5: lea    rcx, [rip + 0x3ac81bc]                   ; [0x3cd02b8] meta:float_TypeInfo
002080fc: call   0x182f609b0
00208101: lea    rcx, [rip + 0x3a95820]                   ; [0x3c9d928] str:'Accuracy: {0:0.0} ms\nAverage: {1:0.0} ms'
00208108: call   0x182f609b0
0020810d: mov    byte ptr [rip + 0x3d04bc4], 1            ; [0x3f0ccd8] (bss)
00208114: xorps  xmm0, xmm0
00208117: xor    eax, eax
00208119: movups xmmword ptr [rsp + 0x40], xmm0
0020811e: mov    qword ptr [rsp + 0x50], rax
00208123: mov    rax, qword ptr [rdi + 0x88]
0020812a: test   rax, rax
0020812d: je     0x18020837c
00208133: mov    rcx, qword ptr [rax + 0x10]
00208137: test   rcx, rcx
0020813a: je     0x18020837c
00208140: xor    edx, edx
00208142: mov    rcx, qword ptr [rcx + 0x90]
00208149: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
0020814e: test   al, al
00208150: je     0x180208365
00208156: mov    rax, qword ptr [rdi + 0xc8]
0020815d: test   rax, rax
00208160: je     0x18020837c
00208166: cmp    dword ptr [rax + 0x18], 0x100
0020816d: jl     0x180208180
0020816f: mov    r8, qword ptr [rip + 0x3aad7ba]          ; [0x3cb5930] metamethod:Method$System.Collections.Generic.List<float>.RemoveAt()
00208176: xor    edx, edx
00208178: mov    rcx, rax
0020817b: call   0x1807103c0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$RemoveAt
00208180: mov    rcx, qword ptr [rdi + 0xc8]
00208187: test   rcx, rcx
0020818a: je     0x18020837c
00208190: movss  xmm0, dword ptr [rdi + 0xc4]
00208198: mov    r9, qword ptr [rip + 0x3aad621]          ; [0x3cb57c0] metamethod:Method$System.Collections.Generic.List<float>.Add()
0020819f: inc    dword ptr [rcx + 0x1c]
002081a2: mov    rdx, qword ptr [rcx + 0x10]
002081a6: test   rdx, rdx
002081a9: je     0x18020837c
002081af: movsxd r8, dword ptr [rcx + 0x18]
002081b3: cmp    r8d, dword ptr [rdx + 0x18]
002081b7: jb     0x1802081d2
002081b9: mov    rax, qword ptr [r9 + 0x20]
002081bd: mov    r8, qword ptr [rax + 0xc0]
002081c4: mov    r8, qword ptr [r8 + 0x70]
002081c8: movaps xmm1, xmm0
002081cb: call   0x18077cca0                              ; System.Collections.Generic.List<float>$$AddWithResize
002081d0: jmp    0x1802081ea
002081d2: lea    eax, [r8 + 1]
002081d6: mov    dword ptr [rcx + 0x18], eax
002081d9: cmp    r8d, dword ptr [rdx + 0x18]
002081dd: jae    0x180208382
002081e3: movss  dword ptr [rdx + r8*4 + 0x20], xmm0
002081ea: mov    rdx, qword ptr [rdi + 0xc8]
002081f1: test   rdx, rdx
002081f4: je     0x18020837c
002081fa: xorps  xmm6, xmm6
002081fd: movsd  qword ptr [rsp + 0x90], xmm6
00208206: mov    r8, qword ptr [rip + 0x3aad66b]          ; [0x3cb5878] metamethod:Method$System.Collections.Generic.List<float>.GetEnumerator()
0020820d: lea    rcx, [rsp + 0x28]
00208212: call   0x18073a550                              ; System.Collections.Generic.List<RegexCharClass.SingleRange>$$GetEnumerator
00208217: movups xmm0, xmmword ptr [rsp + 0x28]
0020821c: movups xmmword ptr [rsp + 0x40], xmm0
00208221: movsd  xmm1, qword ptr [rsp + 0x38]
00208227: movsd  qword ptr [rsp + 0x50], xmm1
0020822d: mov    qword ptr [rsp + 0x28], 0
00208236: lea    rbx, [rsp + 0x40]
0020823b: mov    qword ptr [rsp + 0x30], rbx
00208240: mov    rdx, qword ptr [rip + 0x3ab1ee9]         ; [0x3cba130] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.MoveNext()
00208247: lea    rcx, [rsp + 0x40]
0020824c: call   0x1814f1140                              ; System.Collections.Generic.List.Enumerator<RegexCharClass.SingleRange>$$MoveNext
00208251: test   al, al
00208253: je     0x18020826d
00208255: movss  xmm0, dword ptr [rsp + 0x50]
0020825b: cvtps2pd xmm0, xmm0
0020825e: addsd  xmm6, xmm0
00208262: movsd  qword ptr [rsp + 0x90], xmm6
0020826b: jmp    0x180208240
0020826d: mov    rdx, qword ptr [rip + 0x3ab1e04]         ; [0x3cba078] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.Dispose()
00208274: mov    rcx, rbx
00208277: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0020827c: jmp    0x1802082ae
0020827e: mov    rdx, qword ptr [rip + 0x3ab1df3]         ; [0x3cba078] metamethod:Method$System.Collections.Generic.List.Enumerator<float>.Dispose()
00208285: mov    rcx, qword ptr [rsp + 0x30]
0020828a: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0020828f: mov    rcx, qword ptr [rsp + 0x28]
00208294: test   rcx, rcx
00208297: jne    0x180208388
0020829d: mov    rdi, qword ptr [rsp + 0x80]
002082a5: movsd  xmm6, qword ptr [rsp + 0x90]
002082ae: mov    rbx, qword ptr [rdi + 0xc8]
002082b5: test   rbx, rbx
002082b8: je     0x18020838e
002082be: mov    ebx, dword ptr [rbx + 0x18]
002082c1: mov    rsi, qword ptr [rdi + 0x78]
002082c5: movss  xmm0, dword ptr [rdi + 0xc4]
002082cd: xorps  xmm0, xmmword ptr [rip + 0x2e5bb0c]      ; [0x3063de0] xmm dbl=(-1.0609978955e-314, -1.0609978955e-314) flt=(-0.0, -0.0, -0.0, -0.0)
002082d4: mulss  xmm0, dword ptr [rip + 0x2e5c628]        ; [0x3064904] flt=1000.0 i=1148846080
002082dc: movss  dword ptr [rsp + 0x80], xmm0
002082e5: lea    rdx, [rsp + 0x80]
002082ed: mov    rcx, qword ptr [rip + 0x3ac7fc4]         ; [0x3cd02b8] meta:float_TypeInfo
002082f4: call   0x182f5fbe0
002082f9: mov    rdi, rax
002082fc: movd   xmm0, ebx
00208300: cvtdq2pd xmm0, xmm0
00208304: divsd  xmm6, xmm0
00208308: xorps  xmm6, xmmword ptr [rip + 0x2e5c621]      ; [0x3064930] xmm dbl=(-0.0, -0.0) flt=(0.0, -0.0, 0.0, -0.0)
0020830f: mulsd  xmm6, qword ptr [rip + 0x2e5c911]        ; [0x3064c28] dbl=1000.0 q=0x408f400000000000
00208317: movsd  qword ptr [rsp + 0x90], xmm6
00208320: lea    rdx, [rsp + 0x90]
00208328: mov    rcx, qword ptr [rip + 0x3ad60d1]         ; [0x3cde400] meta:double_TypeInfo
0020832f: call   0x182f5fbe0
00208334: xor    r9d, r9d
00208337: mov    r8, rax
0020833a: mov    rdx, rdi
0020833d: mov    rcx, qword ptr [rip + 0x3a955e4]         ; [0x3c9d928] str:'Accuracy: {0:0.0} ms\nAverage: {1:0.0} ms'
00208344: call   0x181982840                              ; System.String$$Format
00208349: test   rsi, rsi
0020834c: je     0x18020838e
0020834e: mov    r9, qword ptr [rsi]
00208351: mov    r8, qword ptr [r9 + 0x560]
00208358: mov    rdx, rax
0020835b: mov    rcx, rsi
0020835e: call   qword ptr [r9 + 0x558]
00208365: lea    r11, [rsp + 0x70]
0020836a: mov    rbx, qword ptr [r11 + 0x18]
0020836e: mov    rsi, qword ptr [r11 + 0x28]
00208372: movaps xmm6, xmmword ptr [rsp + 0x60]
00208377: mov    rsp, r11
0020837a: pop    rdi
0020837b: ret    
0020837c: call   0x182f60c50
00208381: int3   
00208382: call   0x182f60c40
00208387: int3   
00208388: call   0x182f60ce0
0020838d: int3   
0020838e: call   0x182f60c50
00208393: int3   
00208394: int3   
