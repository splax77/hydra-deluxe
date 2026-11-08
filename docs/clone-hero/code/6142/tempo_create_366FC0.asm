00366fc0: mov    qword ptr [rsp + 0x10], rbx
00366fc5: push   rsi
00366fc6: sub    rsp, 0x50
00366fca: cmp    byte ptr [rip + 0x3ba70f4], 0            ; [0x3f0e0c5] (bss)
00366fd1: mov    ebx, edx
00366fd3: movaps xmmword ptr [rsp + 0x30], xmm7
00366fd8: movaps xmm7, xmm2
00366fdb: jne    0x180367038
00366fdd: lea    rcx, [rip + 0x3954e74]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
00366fe4: call   0x182f609b0
00366fe9: lea    rcx, [rip + 0x398f9b0]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
00366ff0: call   0x182f609b0
00366ff5: lea    rcx, [rip + 0x3931324]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00366ffc: call   0x182f609b0
00367001: lea    rcx, [rip + 0x395deb8]                   ; [0x3cc4ec0] str:'Failed to set BassFX pitch'
00367008: call   0x182f609b0
0036700d: lea    rcx, [rip + 0x395df6c]                   ; [0x3cc4f80] str:'Failed to set BassFX quality attribute'
00367014: call   0x182f609b0
00367019: lea    rcx, [rip + 0x395af60]                   ; [0x3cc1f80] str:'Failed to create tempo stream'
00367020: call   0x182f609b0
00367025: lea    rcx, [rip + 0x395e014]                   ; [0x3cc5040] str:'Failed to set BassFX starting tempo'
0036702c: call   0x182f609b0
00367031: mov    byte ptr [rip + 0x3ba708d], 1            ; [0x3f0e0c5] (bss)
00367038: xor    r8d, r8d
0036703b: mov    edx, 0x10000
00367040: mov    ecx, ebx
00367042: call   0x1803a8520                              ; ʾʻʲʾʿʵʳˁʿʷˀ$$ʻʳʸʷʲʺʶʷʹʿʶ
00367047: mov    rcx, qword ptr [rip + 0x3954e0a]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036704e: mov    ebx, eax
00367050: cmp    dword ptr [rcx + 0xe0], 0
00367057: jne    0x18036705e
00367059: call   0x182f60cf0
0036705e: cmp    byte ptr [rip + 0x3ba704f], 0            ; [0x3f0e0b4] (bss)
00367065: mov    rsi, qword ptr [rip + 0x395af14]         ; [0x3cc1f80] str:'Failed to create tempo stream'
0036706c: jne    0x180367099
0036706e: lea    rcx, [rip + 0x39712db]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
00367075: call   0x182f609b0
0036707a: lea    rcx, [rip + 0x392eaf7]                   ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
00367081: call   0x182f609b0
00367086: lea    rcx, [rip + 0x394fefb]                   ; [0x3cb6f88] str:"Bass Error expected, but Bass.LastError was set to 'OK'. Not throwing exception."
0036708d: call   0x182f609b0
00367092: mov    byte ptr [rip + 0x3ba701b], 1            ; [0x3f0e0b4] (bss)
00367099: mov    qword ptr [rsp + 0x60], rdi
0036709e: test   ebx, ebx
003670a0: jne    0x1803670e9
003670a2: mov    rcx, qword ptr [rip + 0x392eacf]         ; [0x3c95b78] meta:ʷʹʹʻʳʶʶʶʶˁʿ_TypeInfo
003670a9: cmp    dword ptr [rcx + 0xe0], ebx
003670af: jne    0x1803670b6
003670b1: call   0x182f60cf0
003670b6: xor    ecx, ecx
003670b8: call   0x18024edf0                              ; ʷʹʹʻʳʶʶʶʶˁʿ$$ˁʼʿʶʳˀʼʾʳˁʲ
003670bd: mov    edi, eax
003670bf: test   eax, eax
003670c1: jne    0x180367238
003670c7: mov    rcx, qword ptr [rip + 0x3971282]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
003670ce: cmp    dword ptr [rcx + 0xe0], eax
003670d4: jne    0x1803670db
003670d6: call   0x182f60cf0
003670db: mov    rcx, qword ptr [rip + 0x394fea6]         ; [0x3cb6f88] str:"Bass Error expected, but Bass.LastError was set to 'OK'. Not throwing exception."
003670e2: xor    edx, edx
003670e4: call   0x18285fb70                              ; UnityEngine.Debug$$LogWarning
003670e9: mov    rax, qword ptr [rip + 0x3931230]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
003670f0: mov    rcx, qword ptr [rax + 0xb8]
003670f7: mov    rcx, qword ptr [rcx + 0xb8]
003670fe: test   rcx, rcx
00367101: je     0x180367232
00367107: xor    edx, edx
00367109: call   0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
0036710e: xor    edi, edi
00367110: test   al, al
00367112: je     0x1803671ce
00367118: mov    rax, qword ptr [rip + 0x3931201]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0036711f: mov    rcx, qword ptr [rax + 0xb8]
00367126: mov    rax, qword ptr [rcx + 0x10]
0036712a: test   rax, rax
0036712d: je     0x180367232
00367133: cmp    byte ptr [rip + 0x3ba6fef], dil          ; [0x3f0e129] (bss)
0036713a: movaps xmmword ptr [rsp + 0x40], xmm6
0036713f: movd   xmm6, dword ptr [rax + 0x10]
00367144: cvtdq2ps xmm6, xmm6
00367147: divss  xmm6, dword ptr [rip + 0x2cfcead]        ; [0x3063ffc] flt=100.0 i=1120403456
0036714f: jne    0x180367164
00367151: lea    rcx, [rip + 0x393b610]                   ; [0x3ca2768] meta:System.Math_TypeInfo
00367158: call   0x182f609b0
0036715d: mov    byte ptr [rip + 0x3ba6fc5], 1            ; [0x3f0e129] (bss)
00367164: mov    rcx, qword ptr [rip + 0x393b5fd]         ; [0x3ca2768] meta:System.Math_TypeInfo
0036716b: cmp    dword ptr [rcx + 0xe0], edi
00367171: jne    0x180367178
00367173: call   0x182f60cf0
00367178: movsd  xmm1, qword ptr [rip + 0x2cfe728]        ; [0x30658a8] dbl=2.0 q=0x4000000000000000
00367180: xor    r8d, r8d
00367183: cvtps2pd xmm0, xmm6
00367186: call   0x181b51c80                              ; System.Math$$Log
0036718b: mov    rcx, qword ptr [rip + 0x3954cc6]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
00367192: xorps  xmm6, xmm6
00367195: cvtsd2ss xmm6, xmm0
00367199: mulss  xmm6, dword ptr [rip + 0x2cff087]        ; [0x3066228] flt=12.0 i=1094713344
003671a1: cmp    dword ptr [rcx + 0xe0], edi
003671a7: jne    0x1803671ae
003671a9: call   0x182f60cf0
003671ae: mov    r9, qword ptr [rip + 0x395dd0b]          ; [0x3cc4ec0] str:'Failed to set BassFX pitch'
003671b5: movaps xmm2, xmm6
003671b8: mov    edx, 0x10001
003671bd: mov    qword ptr [rsp + 0x20], rdi
003671c2: mov    ecx, ebx
003671c4: call   0x18036c8d0                              ; BassAudioManager$$ʿʲʶʷˁʾʸʺʲʿʿ
003671c9: movaps xmm6, xmmword ptr [rsp + 0x40]
003671ce: mov    rcx, qword ptr [rip + 0x3954c83]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
003671d5: cmp    dword ptr [rcx + 0xe0], edi
003671db: jne    0x1803671e2
003671dd: call   0x182f60cf0
003671e2: mov    r9, qword ptr [rip + 0x395de57]          ; [0x3cc5040] str:'Failed to set BassFX starting tempo'
003671e9: movaps xmm2, xmm7
003671ec: mov    edx, 0x10000
003671f1: mov    qword ptr [rsp + 0x20], rdi
003671f6: mov    ecx, ebx
003671f8: call   0x18036c8d0                              ; BassAudioManager$$ʿʲʶʷˁʾʸʺʲʿʿ
003671fd: mov    r9, qword ptr [rip + 0x395dd7c]          ; [0x3cc4f80] str:'Failed to set BassFX quality attribute'
00367204: mov    edx, 0x10016
00367209: mov    r8d, 1
0036720f: mov    qword ptr [rsp + 0x20], rdi
00367214: mov    ecx, ebx
00367216: call   0x18036c9e0                              ; BassAudioManager$$ʿʲʶʷˁʾʸʺʲʿʿ
0036721b: mov    rdi, qword ptr [rsp + 0x60]
00367220: mov    eax, ebx
00367222: mov    rbx, qword ptr [rsp + 0x68]
00367227: movaps xmm7, xmmword ptr [rsp + 0x30]
0036722c: add    rsp, 0x50
00367230: pop    rsi
00367231: ret    
00367232: call   0x182f60c50
00367237: int3   
00367238: lea    rcx, [rip + 0x39a73c1]                   ; [0x3d0e600] meta:ʲʽˁʲʺʾʷʾˀʽʻ_TypeInfo
0036723f: call   0x182f609d0
00367244: mov    rcx, rax
00367247: call   0x182f60c00
0036724c: xor    r9d, r9d
0036724f: mov    r8d, edi
00367252: mov    rdx, rsi
00367255: mov    rcx, rax
00367258: mov    rbx, rax
0036725b: call   0x180186dc0                              ; ʲʽˁʲʺʾʷʾˀʽʻ$$.ctor
00367260: lea    rcx, [rip + 0x3954e69]                   ; [0x3cbc0d0] metamethod:Method$BassAudioManager.ˁʳʲʳʲʶʲʻʵʵʽ()
00367267: call   0x182f609d0
0036726c: mov    rdx, rax
0036726f: mov    rcx, rbx
00367272: call   0x182f60c10
00367277: int3   
00367278: int3   
