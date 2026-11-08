001370a0: push   rdi
001370a2: sub    rsp, 0xd0
001370a9: cmp    byte ptr [rip + 0x3dd5496], 0            ; [0x3f0c546] (bss)
001370b0: mov    rdi, rcx
001370b3: jne    0x180137168
001370b9: lea    rcx, [rip + 0x3b9fd98]                   ; [0x3cd6e58] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʲʵˁʲʶˁʶʲʲʾʳ()
001370c0: call   0x182f609b0
001370c5: lea    rcx, [rip + 0x3b9ff0c]                   ; [0x3cd6fd8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʲʿʵˁʶˀʵʵʲʲʷ()
001370cc: call   0x182f609b0
001370d1: lea    rcx, [rip + 0x3b9ffc0]                   ; [0x3cd7098] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʳʳˁʸʺʸʿʻʲʽʼ()
001370d8: call   0x182f609b0
001370dd: lea    rcx, [rip + 0x3ba0074]                   ; [0x3cd7158] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʵʶʴˀʹʻʺʶˁʸʼ()
001370e4: call   0x182f609b0
001370e9: lea    rcx, [rip + 0x3ba0128]                   ; [0x3cd7218] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʵʻˁʾˁʽˁʴʵʳʾ()
001370f0: call   0x182f609b0
001370f5: lea    rcx, [rip + 0x3ba01dc]                   ; [0x3cd72d8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʷʸʷʼʹʵʲʲˀˀʳ()
001370fc: call   0x182f609b0
00137101: lea    rcx, [rip + 0x3b9fb10]                   ; [0x3cd6c18] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʸʽʿʲʵʹʷʷʸʵʲ<ʴʴʶʺʻˁʻʿʽʵʲ>()
00137108: call   0x182f609b0
0013710d: lea    rcx, [rip + 0x3b9fbc4]                   ; [0x3cd6cd8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʹʴʶʽʶʹʿʷˀʳʾ<ʴʴʶʺʻˁʻʿʽʵʲ>()
00137114: call   0x182f609b0
00137119: lea    rcx, [rip + 0x3ba0338]                   ; [0x3cd7458] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʿˁʾʺʶʻʽʶʸʿʹ()
00137120: call   0x182f609b0
00137125: lea    rcx, [rip + 0x3ba00b4]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
0013712c: call   0x182f609b0
00137131: lea    rcx, [rip + 0x3b979c8]                   ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
00137138: call   0x182f609b0
0013713d: lea    rcx, [rip + 0x3b7d4f4]                   ; [0x3cb4638] meta:UnityEngine.Rendering.OnDemandRendering_TypeInfo
00137144: call   0x182f609b0
00137149: lea    rcx, [rip + 0x3b616f8]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
00137150: call   0x182f609b0
00137155: lea    rcx, [rip + 0x3bba954]                   ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
0013715c: call   0x182f609b0
00137161: mov    byte ptr [rip + 0x3dd53de], 1            ; [0x3f0c546] (bss)
00137168: mov    rcx, qword ptr [rip + 0x3b7d4c9]         ; [0x3cb4638] meta:UnityEngine.Rendering.OnDemandRendering_TypeInfo
0013716f: cmp    dword ptr [rcx + 0xe0], 0
00137176: jne    0x18013717d
00137178: call   0x182f60cf0
0013717d: xor    ecx, ecx
0013717f: call   0x182895320                              ; UnityEngine.Rendering.OnDemandRendering$$get_willCurrentFrameRender
00137184: test   al, al
00137186: je     0x18013776b
0013718c: mov    qword ptr [rsp + 0xe0], rbx
00137194: mov    qword ptr [rsp + 0xe8], rbp
0013719c: mov    qword ptr [rsp + 0xf0], rsi
001371a4: mov    qword ptr [rsp + 0xc8], r12
001371ac: mov    qword ptr [rsp + 0xc0], r13
001371b4: mov    qword ptr [rsp + 0xb8], r14
001371bc: mov    qword ptr [rsp + 0xb0], r15
001371c4: movaps xmmword ptr [rsp + 0xa0], xmm6
001371cc: movaps xmmword ptr [rsp + 0x90], xmm7
001371d4: movaps xmmword ptr [rsp + 0x80], xmm8
001371dd: movaps xmmword ptr [rsp + 0x70], xmm9
001371e3: mov    rax, qword ptr [rdi + 0x20]
001371e7: test   rax, rax
001371ea: je     0x180137774
001371f0: mov    rcx, qword ptr [rax + 0x120]
001371f7: test   rcx, rcx
001371fa: je     0x180137774
00137200: xor    edx, edx
00137202: call   0x1820f4f90                              ; TMPro.TMP_Text$$get_styleSheet
00137207: mov    rsi, rax
0013720a: test   rax, rax
0013720d: je     0x180137714
00137213: mov    rdx, qword ptr [rip + 0x3b9fdbe]         ; [0x3cd6fd8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʲʿʵˁʶˀʵʵʲʲʷ()
0013721a: mov    rcx, rdi
0013721d: call   0x181162fe0                              ; BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, object, object>$$ʲʿʵˁʶˀʵʵʲʲʷ
00137222: mov    ecx, dword ptr [rdi + 0x30]
00137225: mov    dword ptr [rdi + 0x134], ecx
0013722b: mov    dword ptr [rdi + 0x130], 0
00137235: cmp    byte ptr [rip + 0x3dd5257], 0            ; [0x3f0c493] (bss)
0013723c: jne    0x180137251
0013723e: lea    rcx, [rip + 0x3bba193]                   ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
00137245: call   0x182f609b0
0013724a: mov    byte ptr [rip + 0x3dd5242], 1            ; [0x3f0c493] (bss)
00137251: mov    rax, qword ptr [rip + 0x3bba180]         ; [0x3cf13d8] meta:UnityEngine.Vector3_TypeInfo
00137258: mov    rcx, qword ptr [rdi + 0x20]
0013725c: mov    rax, qword ptr [rax + 0xb8]
00137263: movsd  xmm0, qword ptr [rax]
00137267: movsd  qword ptr [rsp + 0x50], xmm0
0013726d: test   rcx, rcx
00137270: je     0x180137774
00137276: mov    rcx, qword ptr [rcx + 0x120]
0013727d: test   rcx, rcx
00137280: je     0x180137774
00137286: mov    rbx, qword ptr [rdi + 0xd0]
0013728d: xor    edx, edx
0013728f: call   0x1820f57f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʿʺʺʽʲʿʶˀʸʷ
00137294: test   rbx, rbx
00137297: je     0x180137774
0013729d: mov    byte ptr [rbx + 0x22], al
001372a0: cmp    byte ptr [rdi + 0x54], 0
001372a4: jne    0x180137714
001372aa: mov    rax, qword ptr [rdi + 0x28]
001372ae: test   rax, rax
001372b1: je     0x180137774
001372b7: mov    rbx, qword ptr [rip + 0x3bba7f2]         ; [0x3cf1ab0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʽʿʸʸʾʶʶʾʶʹʲ>()
001372be: movsd  xmm9, qword ptr [rax + 0x78]
001372c4: mov    r14d, dword ptr [rsi + 0x18]
001372c8: cmp    qword ptr [rbx + 0x38], 0
001372cd: jne    0x1801372d7
001372cf: mov    rcx, rbx
001372d2: call   0x182f657d0
001372d7: mov    r8, qword ptr [rbx + 0x38]
001372db: lea    rcx, [rsp + 0x60]
001372e0: mov    rdx, rsi
001372e3: mov    r8, qword ptr [r8 + 8]
001372e7: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
001372ec: mov    rcx, qword ptr [rdi + 0x20]
001372f0: test   rcx, rcx
001372f3: je     0x180137774
001372f9: mov    r12, qword ptr [rsp + 0x60]
001372fe: xor    edx, edx
00137300: mov    r15d, dword ptr [rsp + 0x68]
00137305: call   0x180208dd0                              ; BasePlayer$$ʷʴʵʼˁʲʶʾʺʹʻ
0013730a: cmp    dword ptr [rdi + 0x130], 0x1f4
00137314: movzx  r13d, al
00137318: jge    0x18013758e
0013731e: nop    
00137320: cmp    dword ptr [rdi + 0x134], r14d
00137327: jge    0x18013758e
0013732d: movsxd rax, dword ptr [rdi + 0x134]
00137334: cmp    eax, r15d
00137337: jae    0x18013777a
0013733d: mov    rbx, qword ptr [r12 + rax*8]
00137341: test   rbx, rbx
00137344: je     0x180137774
0013734a: movsd  xmm0, qword ptr [rbx + 0x28]
0013734f: subsd  xmm0, xmm9
00137354: comisd xmm0, xmmword ptr [rdi + 0xf0]
0013735c: jae    0x18013758e
00137362: mov    eax, dword ptr [rdi + 0x30]
00137365: cmp    dword ptr [rdi + 0x134], eax
0013736b: jne    0x18013738c
0013736d: movsd  xmm0, qword ptr [rdi + 0xf8]
00137375: comisd xmm0, xmmword ptr [rbx + 0x28]
0013737a: jbe    0x18013738c
0013737c: inc    eax
0013737e: mov    dword ptr [rdi + 0x30], eax
00137381: mov    dword ptr [rdi + 0x134], eax
00137387: jmp    0x18013757e
0013738c: xor    edx, edx
0013738e: mov    rcx, rbx
00137391: call   0x18214b610                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʲʺʾʽʽʹʳʳʼʹ
00137396: test   al, al
00137398: je     0x1801373ac
0013739a: xor    edx, edx
0013739c: mov    rcx, rbx
0013739f: call   0x18214b770                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʶˀʺʿʹʻʼʿʴʻʴ
001373a4: test   al, al
001373a6: je     0x180137578
001373ac: xor    edx, edx
001373ae: mov    rcx, rbx
001373b1: call   0x18214b610                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʲʺʾʽʽʹʳʳʼʹ
001373b6: test   al, al
001373b8: jne    0x180137578
001373be: xor    edx, edx
001373c0: mov    rcx, rbx
001373c3: call   0x18214b770                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʶˀʺʿʹʻʼʿʴʻʴ
001373c8: test   al, al
001373ca: jne    0x180137578
001373d0: mov    rcx, qword ptr [rip + 0x3b61471]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
001373d7: movsd  xmm6, qword ptr [rbx + 0x28]
001373dc: movss  xmm7, dword ptr [rdi + 0x48]
001373e1: movss  xmm8, dword ptr [rdi + 0x34]
001373e7: cmp    dword ptr [rcx + 0xe0], 0
001373ee: jne    0x1801373f5
001373f0: call   0x182f60cf0
001373f5: mov    rax, qword ptr [rdi]
001373f8: mov    rcx, rdi
001373fb: movzx  edx, word ptr [rbx + 0x80]
00137402: mov    r8, qword ptr [rax + 0x1f0]
00137409: call   qword ptr [rax + 0x1e8]
0013740f: subsd  xmm6, xmm9
00137414: mov    ebp, eax
00137416: cvtps2pd xmm0, xmm8
0013741a: cvtps2pd xmm1, xmm7
0013741d: mulsd  xmm6, xmm0
00137421: addsd  xmm6, xmm1
00137425: test   eax, eax
00137427: jg     0x18013742e
00137429: xorps  xmm0, xmm0
0013742c: jmp    0x180137447
0013742e: dec    eax
00137430: movd   xmm0, eax
00137434: cvtdq2ps xmm0, xmm0
00137437: mulss  xmm0, dword ptr [rdi + 0xec]
0013743f: addss  xmm0, dword ptr [rdi + 0xe8]
00137447: movss  dword ptr [rsp + 0x50], xmm0
0013744d: cvtpd2ps xmm1, xmm6
00137451: test   r13b, r13b
00137454: jne    0x18013745a
00137456: xor    esi, esi
00137458: jmp    0x180137488
0013745a: mov    rax, qword ptr [rdi + 0x20]
0013745e: test   rax, rax
00137461: je     0x180137774
00137467: mov    rcx, qword ptr [rax + 0x120]
0013746e: test   rcx, rcx
00137471: je     0x180137774
00137477: movsd  xmm0, qword ptr [rcx + 0x120]
0013747f: comisd xmm0, xmmword ptr [rbx + 0x28]
00137484: setae  sil
00137488: mov    rcx, qword ptr [rdi + 0xb8]
0013748f: test   rcx, rcx
00137492: je     0x180137774
00137498: movsxd rax, dword ptr [rdi + 0x130]
0013749f: cmp    eax, dword ptr [rcx + 0x18]
001374a2: jae    0x18013777a
001374a8: mov    rcx, qword ptr [rcx + rax*8 + 0x20]
001374ad: test   rcx, rcx
001374b0: je     0x180137774
001374b6: movsd  xmm0, qword ptr [rsp + 0x50]
001374bc: lea    rdx, [rsp + 0x60]
001374c1: xor    r8d, r8d
001374c4: movsd  qword ptr [rsp + 0x60], xmm0
001374ca: movss  dword ptr [rsp + 0x68], xmm1
001374d0: call   0x18288a510                              ; UnityEngine.Transform$$set_localPosition
001374d5: cmp    byte ptr [rdi + 0x108], 0
001374dc: je     0x180137511
001374de: mov    edx, dword ptr [rbx + 0x20]
001374e1: lea    ecx, [rdx + rbp]
001374e4: xor    ecx, 0x5f3759df
001374ea: mov    eax, ecx
001374ec: sar    eax, 0xd
001374ef: xor    eax, ecx
001374f1: imul   ecx, eax, 0x27d4eb2d
001374f7: mov    eax, ecx
001374f9: sar    eax, 0xf
001374fc: xor    eax, ecx
001374fe: mov    ecx, dword ptr [rdi + 0xd8]
00137504: add    eax, edx
00137506: inc    ecx
00137508: btr    eax, 0x1f
0013750c: cdq    
0013750d: idiv   ecx
0013750f: jmp    0x180137513
00137511: mov    edx, ebp
00137513: mov    rcx, qword ptr [rdi + 0xc8]
0013751a: test   rcx, rcx
0013751d: je     0x180137774
00137523: movsxd rax, dword ptr [rdi + 0x130]
0013752a: cmp    eax, dword ptr [rcx + 0x18]
0013752d: jae    0x18013777a
00137533: mov    qword ptr [rsp + 0x40], 0
0013753c: add    rcx, 0x20
00137540: shl    rax, 6
00137544: mov    r9d, edx
00137547: mov    rdx, qword ptr [rdi + 0xd0]
0013754e: add    rcx, rax
00137551: mov    eax, dword ptr [rdi + 0x130]
00137557: mov    r8d, ebp
0013755a: mov    byte ptr [rsp + 0x38], 0
0013755f: mov    byte ptr [rsp + 0x30], sil
00137564: mov    dword ptr [rsp + 0x28], eax
00137568: mov    qword ptr [rsp + 0x20], rbx
0013756d: call   0x1802b0850                              ; ʳʻˁʾʺʳʽʸʳʽʲ$$ʲʸʸʾʲʳʴʷʹʼʸ
00137572: inc    dword ptr [rdi + 0x130]
00137578: inc    dword ptr [rdi + 0x134]
0013757e: cmp    dword ptr [rdi + 0x130], 0x1f4
00137588: jl     0x180137320
0013758e: mov    rdx, qword ptr [rip + 0x3b9fd43]         ; [0x3cd72d8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʷʸʷʼʹʵʲʲˀˀʳ()
00137595: mov    rcx, rdi
00137598: call   0x181163d00                              ; BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, object, object>$$ʷʸʷʼʹʵʲʲˀˀʳ
0013759d: mov    rdx, qword ptr [rip + 0x3b9feb4]         ; [0x3cd7458] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʿˁʾʺʶʻʽʶʸʿʹ()
001375a4: mov    rcx, rdi
001375a7: call   0x181166890                              ; BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, object, object>$$ʿˁʾʺʶʻʽʶʸʿʹ
001375ac: mov    rcx, qword ptr [rdi + 0x20]
001375b0: test   rcx, rcx
001375b3: je     0x180137774
001375b9: mov    rcx, qword ptr [rcx + 0x120]
001375c0: test   rcx, rcx
001375c3: je     0x180137774
001375c9: xor    edx, edx
001375cb: xor    ebp, ebp
001375cd: call   0x1820f57f0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʹʿʺʺʽʲʿʶˀʸʷ
001375d2: test   al, al
001375d4: je     0x1801376f0
001375da: mov    rax, qword ptr [rdi + 0x20]
001375de: test   rax, rax
001375e1: je     0x180137774
001375e7: mov    rcx, qword ptr [rax + 0x120]
001375ee: test   rcx, rcx
001375f1: je     0x180137774
001375f7: xor    edx, edx
001375f9: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
001375fe: mov    r9, qword ptr [rip + 0x3b9f613]          ; [0x3cd6c18] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʸʽʿʲʵʹʷʷʸʵʲ<ʴʴʶʺʻˁʻʿʽʵʲ>()
00137605: lea    r8, [rdi + 0x150]
0013760c: mov    rdx, rax
0013760f: mov    rcx, rdi
00137612: call   0x18041b5c0                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʸʽʿʲʵʹʷʷʸʵʲ<object>
00137617: mov    rbx, rax
0013761a: cmp    eax, -1
0013761d: je     0x1801376f0
00137623: mov    rsi, rax
00137626: shr    rsi, 0x20
0013762a: cmp    eax, esi
0013762c: jg     0x1801376f0
00137632: mov    rax, qword ptr [rdi + 0x20]
00137636: test   rax, rax
00137639: je     0x180137774
0013763f: mov    rcx, qword ptr [rax + 0x120]
00137646: test   rcx, rcx
00137649: je     0x180137774
0013764f: xor    edx, edx
00137651: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
00137656: test   rax, rax
00137659: je     0x180137774
0013765f: mov    r8, qword ptr [rip + 0x3b9749a]          ; [0x3cceb00] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.get_Item()
00137666: mov    edx, ebx
00137668: mov    rcx, rax
0013766b: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
00137670: test   rax, rax
00137673: je     0x180137774
00137679: cmp    byte ptr [rax + 0x38], bpl
0013767d: jne    0x180137687
0013767f: inc    ebx
00137681: cmp    ebx, esi
00137683: jle    0x180137632
00137685: jmp    0x1801376f0
00137687: cmp    ebx, esi
00137689: jg     0x1801376f0
0013768b: mov    rax, qword ptr [rdi + 0x20]
0013768f: test   rax, rax
00137692: je     0x180137774
00137698: mov    rcx, qword ptr [rax + 0x120]
0013769f: test   rcx, rcx
001376a2: je     0x180137774
001376a8: xor    edx, edx
001376aa: call   0x1820f4650                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʵʽʸʺʾʲʴʼʻʳˀ
001376af: mov    rcx, qword ptr [rip + 0x3b9f622]         ; [0x3cd6cd8] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʹʴʶʽʶʹʿʷˀʳʾ<ʴʴʶʺʻˁʻʿʽʵʲ>()
001376b6: mov    rdx, rax
001376b9: movss  xmm0, dword ptr [rip + 0x2f2ce27]        ; [0x30644e8] flt=1.25 i=1067450368
001376c1: mov    r9, qword ptr [rdi + 0x148]
001376c8: mov    r8, qword ptr [rdi + 0x140]
001376cf: mov    qword ptr [rsp + 0x40], rcx
001376d4: mov    rcx, rdi
001376d7: movss  dword ptr [rsp + 0x38], xmm0
001376dd: mov    dword ptr [rsp + 0x30], ebp
001376e1: mov    dword ptr [rsp + 0x28], esi
001376e5: mov    dword ptr [rsp + 0x20], ebx
001376e9: call   0x18041be70                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʹʴʶʽʶʹʿʷˀʳʾ<object>
001376ee: mov    ebp, eax
001376f0: mov    rdx, qword ptr [rip + 0x3b9fa61]         ; [0x3cd7158] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʵʶʴˀʹʻʺʶˁʸʼ()
001376f7: mov    rcx, rdi
001376fa: call   0x181160aa0                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʵʶʴˀʹʻʺʶˁʸʼ
001376ff: mov    r8, qword ptr [rip + 0x3b9f992]          ; [0x3cd7098] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>.ʳʳˁʸʺʸʿʻʲʽʼ()
00137706: mov    edx, ebp
00137708: mov    rcx, qword ptr [rdi + 0x140]
0013770f: call   0x181160a20                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$ʳʳˁʸʺʸʿʻʲʽʼ
00137714: movaps xmm8, xmmword ptr [rsp + 0x80]
0013771d: movaps xmm7, xmmword ptr [rsp + 0x90]
00137725: movaps xmm6, xmmword ptr [rsp + 0xa0]
0013772d: mov    r15, qword ptr [rsp + 0xb0]
00137735: mov    r14, qword ptr [rsp + 0xb8]
0013773d: mov    r13, qword ptr [rsp + 0xc0]
00137745: mov    r12, qword ptr [rsp + 0xc8]
0013774d: mov    rsi, qword ptr [rsp + 0xf0]
00137755: mov    rbp, qword ptr [rsp + 0xe8]
0013775d: mov    rbx, qword ptr [rsp + 0xe0]
00137765: movaps xmm9, xmmword ptr [rsp + 0x70]
0013776b: add    rsp, 0xd0
00137772: pop    rdi
00137773: ret    
00137774: call   0x182f60c50
00137779: int3   
0013777a: call   0x182f60c40
0013777f: int3   
00137780: push   rbx
00137782: sub    rsp, 0x20
00137786: cmp    byte ptr [rip + 0x3dd4db5], 0            ; [0x3f0c542] (bss)
0013778d: mov    rbx, rcx
00137790: jne    0x1801377a5
00137792: lea    rcx, [rip + 0x3b9f5ff]                   ; [0x3cd6d98] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>..ctor()
00137799: call   0x182f609b0
0013779e: mov    byte ptr [rip + 0x3dd4d9d], 1            ; [0x3f0c542] (bss)
001377a5: mov    rdx, qword ptr [rip + 0x3b9f5ec]         ; [0x3cd6d98] metamethod:Method$BaseNoteRenderer<ʳʻˁʾʺʳʽʸʳʽʲ, DrumsNoteContainer, DrumNoteSprites>..ctor()
001377ac: mov    rcx, rbx
001377af: add    rsp, 0x20
001377b3: pop    rbx
001377b4: jmp    0x181161fd0                              ; BaseNoteRenderer<ʸʵʾʵʳʳʵʷʶʺʽ, object, object>$$.ctor
001377b9: int3   
