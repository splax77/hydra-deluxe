02155050: mov    rax, rsp
02155053: mov    qword ptr [rax + 8], rbx
02155057: mov    qword ptr [rax + 0x10], rsi
0215505b: mov    byte ptr [rax + 0x18], r8b
0215505f: push   rdi
02155060: push   r12
02155062: push   r13
02155064: push   r14
02155066: push   r15
02155068: sub    rsp, 0x150
0215506f: movaps xmmword ptr [rax - 0x38], xmm6
02155073: movzx  r15d, r8b
02155077: mov    rbx, rdx
0215507a: mov    rdi, rcx
0215507d: cmp    byte ptr [rip + 0x1dc4619], 0            ; [0x3f1969d] (bss)
02155084: jne    0x182155199
0215508a: lea    rcx, [rip + 0x1b6ae1f]                   ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
02155091: call   0x182f609b0
02155096: lea    rcx, [rip + 0x1b6a513]                   ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
0215509d: call   0x182f609b0
021550a2: lea    rcx, [rip + 0x1b6a5c7]                   ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
021550a9: call   0x182f609b0
021550ae: lea    rcx, [rip + 0x1b6aebb]                   ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
021550b5: call   0x182f609b0
021550ba: lea    rcx, [rip + 0x1b6af6f]                   ; [0x3cc0030] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.get_Current()
021550c1: call   0x182f609b0
021550c6: lea    rcx, [rip + 0x1b6a663]                   ; [0x3cbf730] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Current()
021550cd: call   0x182f609b0
021550d2: lea    rcx, [rip + 0x1b7a6e7]                   ; [0x3ccf7c0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.AddRange()
021550d9: call   0x182f609b0
021550de: lea    rcx, [rip + 0x1b7c7db]                   ; [0x3cd18c0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.AddRange()
021550e5: call   0x182f609b0
021550ea: lea    rcx, [rip + 0x1b78157]                   ; [0x3ccd248] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.AddRange()
021550f1: call   0x182f609b0
021550f6: lea    rcx, [rip + 0x1b79103]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
021550fd: call   0x182f609b0
02155102: lea    rcx, [rip + 0x1b7abf7]                   ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
02155109: call   0x182f609b0
0215510e: lea    rcx, [rip + 0x1b7926b]                   ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
02155115: call   0x182f609b0
0215511a: lea    rcx, [rip + 0x1b4d6cf]                   ; [0x3ca27f0] metamethod:Method$System.ReadOnlySpan<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Length()
02155121: call   0x182f609b0
02155126: lea    rcx, [rip + 0x1b3f213]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215512d: call   0x182f609b0
02155132: lea    rcx, [rip + 0x1b3f95f]                   ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
02155139: call   0x182f609b0
0215513e: lea    rcx, [rip + 0x1b4118b]                   ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
02155145: call   0x182f609b0
0215514a: lea    rcx, [rip + 0x1b4ae47]                   ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02155151: call   0x182f609b0
02155156: lea    rcx, [rip + 0x1b4b10b]                   ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
0215515d: call   0x182f609b0
02155162: lea    rcx, [rip + 0x1ba77bf]                   ; [0x3cfc928] str:'[ENABLE_CHART_DYNAMICS]'
02155169: call   0x182f609b0
0215516e: lea    rcx, [rip + 0x1b90b33]                   ; [0x3ce5ca8] str:'mix'
02155175: call   0x182f609b0
0215517a: lea    rcx, [rip + 0x1b511af]                   ; [0x3ca6330] str:'ENABLE_CHART_DYNAMICS'
02155181: call   0x182f609b0
02155186: lea    rcx, [rip + 0x1babb0b]                   ; [0x3d00c98] str:'[mix'
0215518d: call   0x182f609b0
02155192: mov    byte ptr [rip + 0x1dc4504], 1            ; [0x3f1969d] (bss)
02155199: xorps  xmm0, xmm0
0215519c: movups xmmword ptr [rsp + 0xb8], xmm0
021551a4: xorps  xmm1, xmm1
021551a7: xor    eax, eax
021551a9: movups xmmword ptr [rsp + 0xc8], xmm1
021551b1: mov    qword ptr [rsp + 0xd8], rax
021551b9: movups xmmword ptr [rsp + 0xe0], xmm0
021551c1: mov    qword ptr [rsp + 0xf0], rax
021551c9: movups xmmword ptr [rsp + 0xf8], xmm1
021551d1: mov    qword ptr [rsp + 0x108], rax
021551d9: mov    rcx, qword ptr [rip + 0x1b3f160]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021551e0: cmp    dword ptr [rcx + 0xe0], eax
021551e6: jne    0x1821551ed
021551e8: call   0x182f60cf0
021551ed: xor    ecx, ecx
021551ef: call   0x1821581e0                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʿʷʺˁʲʷʴʸʼʽʵ
021551f4: test   al, al
021551f6: jne    0x182155a4b
021551fc: mov    rax, qword ptr [rbx]
021551ff: test   rax, rax
02155202: je     0x182155b23
02155208: mov    r14, qword ptr [rax + 0x30]
0215520c: mov    qword ptr [rsp + 0x48], r14
02155211: test   r14, r14
02155214: je     0x182155b23
0215521a: movzx  esi, byte ptr [rax + 0x11]
0215521e: movzx  ebx, byte ptr [rax + 0x10]
02155222: mov    dword ptr [rsp + 0x54], ebx
02155226: xor    r9d, r9d
02155229: mov    r8b, 3
0215522c: movzx  edx, r15b
02155230: mov    rcx, r14
02155233: call   0x1820cfd40
02155238: mov    r13, rax
0215523b: test   rdi, rdi
0215523e: je     0x182155b23
02155244: mov    byte ptr [rsp + 0x42], sil
02155249: mov    byte ptr [rsp + 0x40], bl
0215524d: mov    byte ptr [rsp + 0x41], 0
02155252: mov    qword ptr [rsp + 0xb0], rax
0215525a: mov    r8, qword ptr [rip + 0x1b7aa9f]          ; [0x3ccfd00] metamethod:Method$System.Collections.Generic.List<ʵʸʸʲˁˁʾʶʿʿʷ>.GetEnumerator()
02155261: mov    rdx, rdi
02155264: lea    rcx, [rsp + 0x90]
0215526c: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02155271: movups xmm0, xmmword ptr [rsp + 0x90]
02155279: movups xmmword ptr [rsp + 0xc8], xmm0
02155281: movsd  xmm1, qword ptr [rsp + 0xa0]
0215528a: movsd  qword ptr [rsp + 0xd8], xmm1
02155293: xor    r12d, r12d
02155296: mov    qword ptr [rsp + 0x60], r12
0215529b: lea    rbx, [rsp + 0xc8]
021552a3: mov    qword ptr [rsp + 0x68], rbx
021552a8: jmp    0x1821552af
021552aa: mov    r14, qword ptr [rsp + 0x48]
021552af: mov    rdx, qword ptr [rip + 0x1b6acba]         ; [0x3cbff70] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.MoveNext()
021552b6: lea    rcx, [rsp + 0xc8]
021552be: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
021552c3: test   al, al
021552c5: je     0x1821557d7
021552cb: mov    r15, qword ptr [rsp + 0xd8]
021552d3: test   r15, r15
021552d6: je     0x1821552af
021552d8: mov    rsi, r12
021552db: mov    rax, qword ptr [rip + 0x1b3f7b6]         ; [0x3c94a98] meta:ʶʹʿʷʸʿʼʵʷʴʾ_TypeInfo
021552e2: cmp    qword ptr [r15], rax
021552e5: cmove  rsi, r15
021552e9: test   rsi, rsi
021552ec: je     0x182155664
021552f2: mov    rax, qword ptr [rsi + 0x30]
021552f6: test   rax, rax
021552f9: je     0x182155afb
021552ff: mov    r14, qword ptr [rsi + 0x10]
02155303: mov    qword ptr [rsp + 0x70], r14
02155308: mov    rdi, qword ptr [rax + 0x10]
0215530c: cmp    byte ptr [rsp + 0x54], 0
02155311: jne    0x182155337
02155313: xor    edx, edx
02155315: movzx  ecx, byte ptr [rsi + 0x28]
02155319: call   0x18210d780                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʳʺʾʶʷʻʷʴʴʶˀ
0215531e: mov    ecx, dword ptr [rsp + 0x54]
02155322: movzx  ecx, cl
02155325: test   al, al
02155327: mov    eax, 1
0215532c: cmovne ecx, eax
0215532f: mov    dword ptr [rsp + 0x54], ecx
02155333: mov    byte ptr [rsp + 0x40], cl
02155337: mov    rdx, rdi
0215533a: sub    rdx, r14
0215533d: mov    qword ptr [rsp + 0x58], rdx
02155342: movzx  ecx, byte ptr [rsi + 0x28]
02155346: cmp    cl, 0x74
02155349: jbe    0x182155444
0215534f: cmp    cl, 0x78
02155352: jne    0x182155378
02155354: test   r13, r13
02155357: je     0x182155a94
0215535d: mov    qword ptr [rsp + 0x20], r12
02155362: xor    r9d, r9d
02155365: mov    r8, rdx
02155368: mov    rdx, r14
0215536b: mov    rcx, r13
0215536e: call   0x1820cedc0
02155373: jmp    0x1821552aa
02155378: cmp    cl, 0x7e
0215537b: jne    0x1821553e9
0215537d: movzx  edi, byte ptr [rsi + 0x29]
02155381: mov    rcx, qword ptr [rip + 0x1b3efb8]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155388: cmp    dword ptr [rcx + 0xe0], 0
0215538f: jne    0x18215539b
02155391: call   0x182f60cf0
02155396: mov    rdx, qword ptr [rsp + 0x58]
0215539b: cmp    dil, 0x33
0215539f: jae    0x1821553bf
021553a1: cmp    dil, 0x29
021553a5: jae    0x1821553bb
021553a7: cmp    dil, 0x1f
021553ab: jae    0x1821553b7
021553ad: cmp    dil, 0x15
021553b1: sbb    al, al
021553b3: and    al, 3
021553b5: jmp    0x1821553c1
021553b7: mov    al, 1
021553b9: jmp    0x1821553c1
021553bb: mov    al, 2
021553bd: jmp    0x1821553c1
021553bf: mov    al, 3
021553c1: test   r13, r13
021553c4: je     0x182155a9a
021553ca: xor    r9d, r9d
021553cd: mov    qword ptr [rsp + 0x28], r12
021553d2: mov    byte ptr [rsp + 0x20], al
021553d6: mov    r8, rdx
021553d9: mov    rdx, r14
021553dc: mov    rcx, r13
021553df: call   0x1820ceec0
021553e4: jmp    0x1821552aa
021553e9: cmp    cl, 0x7f
021553ec: jne    0x18215559b
021553f2: movzx  edi, byte ptr [rsi + 0x29]
021553f6: mov    rcx, qword ptr [rip + 0x1b3ef43]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
021553fd: cmp    dword ptr [rcx + 0xe0], 0
02155404: jne    0x182155410
02155406: call   0x182f60cf0
0215540b: mov    rdx, qword ptr [rsp + 0x58]
02155410: cmp    dil, 0x33
02155414: jae    0x182155434
02155416: cmp    dil, 0x29
0215541a: jae    0x182155430
0215541c: cmp    dil, 0x1f
02155420: jae    0x18215542c
02155422: cmp    dil, 0x15
02155426: sbb    al, al
02155428: and    al, 3
0215542a: jmp    0x182155436
0215542c: mov    al, 1
0215542e: jmp    0x182155436
02155430: mov    al, 2
02155432: jmp    0x182155436
02155434: mov    al, 3
02155436: test   r13, r13
02155439: je     0x182155a9f
0215543f: mov    r9b, 1
02155442: jmp    0x1821553cd
02155444: cmp    cl, 0x67
02155447: jne    0x182155468
02155449: test   r13, r13
0215544c: je     0x182155aa4
02155452: xor    r9d, r9d
02155455: mov    r8, rdx
02155458: mov    rdx, r14
0215545b: mov    rcx, r13
0215545e: call   0x1820cf1b0
02155463: jmp    0x1821552aa
02155468: lea    eax, [rcx - 0x6d]
0215546b: cmp    eax, 7
0215546e: ja     0x18215559b
02155474: lea    eax, [rcx - 0x6d]
02155477: movsxd rcx, eax
0215547a: lea    r8, [rip - 0x2155481]                    ; [0x0] (bss)
02155481: mov    eax, dword ptr [r8 + rcx*4 + 0x2155b2c]
02155489: add    rax, r8
0215548c: jmp    rax
0215548e: xorps  xmm0, xmm0
02155491: movups xmmword ptr [rsp + 0x70], xmm0
02155496: movups xmmword ptr [rsp + 0x80], xmm0
0215549e: mov    qword ptr [rsp + 0x30], r12
021554a3: mov    dword ptr [rsp + 0x28], r12d
021554a8: mov    byte ptr [rsp + 0x20], 0xff
021554ad: mov    r9b, 8
021554b0: mov    r8, rdi
021554b3: mov    rdx, r14
021554b6: lea    rcx, [rsp + 0x70]
021554bb: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
021554c0: mov    rcx, qword ptr [rip + 0x1b4aad1]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
021554c7: movups xmm1, xmmword ptr [rsp + 0x80]
021554cf: movups xmm0, xmmword ptr [rsp + 0x70]
021554d4: cmp    dword ptr [rcx + 0xe0], 0
021554db: movups xmmword ptr [rsp + 0x120], xmm1
021554e3: movups xmmword ptr [rsp + 0x110], xmm0
021554eb: jne    0x1821554f2
021554ed: call   0x182f60cf0
021554f2: xor    r9d, r9d
021554f5: lea    r8, [rsp + 0x110]
021554fd: movzx  edx, byte ptr [rsp + 0x190]
02155505: mov    r14, qword ptr [rsp + 0x48]
0215550a: mov    rcx, r14
0215550d: call   0x1820cfbc0
02155512: jmp    0x1821552af
02155517: mov    byte ptr [rsp + 0x42], 1
0215551c: movzx  ecx, byte ptr [rsi + 0x28]
02155520: sub    ecx, 0x6e
02155523: je     0x182155541
02155525: sub    ecx, 1
02155528: je     0x18215553a
0215552a: cmp    ecx, 1
0215552d: jne    0x182155aa9
02155533: mov    eax, 0x12
02155538: jmp    0x182155546
0215553a: mov    eax, 0x10
0215553f: jmp    0x182155546
02155541: mov    eax, 0xf
02155546: xorps  xmm0, xmm0
02155549: movups xmmword ptr [rsp + 0x70], xmm0
0215554e: movups xmmword ptr [rsp + 0x80], xmm0
02155556: mov    qword ptr [rsp + 0x30], r12
0215555b: mov    dword ptr [rsp + 0x28], eax
0215555f: mov    byte ptr [rsp + 0x20], 0xff
02155564: mov    r9b, 6
02155567: mov    r8, rdi
0215556a: mov    rdx, r14
0215556d: lea    rcx, [rsp + 0x70]
02155572: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
02155577: jmp    0x1821554c0
0215557c: test   r13, r13
0215557f: je     0x182155af1
02155585: xor    r9d, r9d
02155588: mov    r8, rdx
0215558b: mov    rdx, r14
0215558e: mov    rcx, r13
02155591: call   0x1820cf0b0
02155596: jmp    0x1821552aa
0215559b: movzx  ecx, byte ptr [rsi + 0x28]
0215559f: xor    edx, edx
021555a1: call   0x18210d9c0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʼʴʷʽˀʺʻʹʿʺʲ
021555a6: mov    byte ptr [rsp + 0x50], al
021555aa: cmp    al, 0xff
021555ac: mov    r14, qword ptr [rsp + 0x48]
021555b1: je     0x1821552af
021555b7: xor    edx, edx
021555b9: movzx  ecx, byte ptr [rsi + 0x28]
021555bd: call   0x18210d7b0                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʸʴʶʳʻˁʶˀʻʸˀ
021555c2: mov    r14d, eax
021555c5: test   eax, eax
021555c7: je     0x1821552aa
021555cd: movzx  ecx, byte ptr [rsi + 0x28]
021555d1: sub    cl, 0x3b
021555d4: cmp    cl, 0x24
021555d7: ja     0x1821555ee
021555d9: movabs rax, 0x1001001001
021555e3: bt     rax, rcx
021555e7: mov    edi, 8
021555ec: jb     0x1821555f1
021555ee: mov    edi, r12d
021555f1: cmp    byte ptr [rsp + 0x41], 0
021555f6: je     0x18215560e
021555f8: cmp    byte ptr [rsi + 0x29], 1
021555fc: je     0x18215560a
021555fe: cmp    byte ptr [rsi + 0x29], 0x7f
02155602: jne    0x18215560e
02155604: bts    edi, 0xa
02155608: jmp    0x18215560e
0215560a: bts    edi, 9
0215560e: cmp    r14d, 0xe
02155612: je     0x18215561f
02155614: cmp    r14d, 0xd
02155618: je     0x182155622
0215561a: or     edi, 0x20
0215561d: jmp    0x182155622
0215561f: or     edi, 0x10
02155622: xor    r9d, r9d
02155625: movzx  r8d, byte ptr [rsp + 0x50]
0215562b: movzx  edx, byte ptr [rsp + 0x190]
02155633: mov    rcx, qword ptr [rsp + 0x48]
02155638: call   0x1820cfd40
0215563d: test   rax, rax
02155640: je     0x182155af6
02155646: mov    qword ptr [rsp + 0x28], r12
0215564b: mov    dword ptr [rsp + 0x20], edi
0215564f: mov    r9, qword ptr [rsp + 0x58]
02155654: mov    r8d, r14d
02155657: mov    rdx, qword ptr [rsp + 0x70]
0215565c: mov    rcx, rax
0215565f: call   0x1820cec60
02155664: mov    rdi, r12
02155667: mov    rax, qword ptr [rip + 0x1b4abfa]         ; [0x3ca0268] meta:ˁʲʾʸʷʿʶʻʷʵʶ_TypeInfo
0215566e: cmp    qword ptr [r15], rax
02155671: cmove  rdi, r15
02155675: test   rdi, rdi
02155678: je     0x1821552aa
0215567e: cmp    qword ptr [rdi + 0x20], 0
02155683: je     0x1821552aa
02155689: mov    r14, qword ptr [rdi + 0x20]
0215568d: xor    r8d, r8d
02155690: mov    rdx, qword ptr [rip + 0x1bab601]         ; [0x3d00c98] str:'[mix'
02155697: mov    rcx, r14
0215569a: call   0x181988830                              ; System.String$$StartsWith
0215569f: test   al, al
021556a1: jne    0x1821556bd
021556a3: xor    r8d, r8d
021556a6: mov    rdx, qword ptr [rip + 0x1b905fb]         ; [0x3ce5ca8] str:'mix'
021556ad: mov    rcx, r14
021556b0: call   0x181988830                              ; System.String$$StartsWith
021556b5: test   al, al
021556b7: je     0x182155798
021556bd: mov    rsi, qword ptr [rdi + 0x20]
021556c1: cmp    byte ptr [rip + 0x1db6a93], 0            ; [0x3f0c15b] (bss)
021556c8: jne    0x1821556dd
021556ca: lea    rcx, [rip + 0x1b4bed7]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
021556d1: call   0x182f609b0
021556d6: mov    byte ptr [rip + 0x1db6a7e], 1            ; [0x3f0c15b] (bss)
021556dd: xorps  xmm6, xmm6
021556e0: test   rsi, rsi
021556e3: je     0x182155711
021556e5: xor    edx, edx
021556e7: mov    rcx, rsi
021556ea: call   0x1819829d0                              ; System.String$$GetRawStringData
021556ef: mov    dword ptr [rsp + 0x9c], r12d
021556f7: mov    qword ptr [rsp + 0x90], rax
021556ff: mov    eax, dword ptr [rsi + 0x10]
02155702: mov    dword ptr [rsp + 0x98], eax
02155709: movaps xmm6, xmmword ptr [rsp + 0x90]
02155711: mov    rdi, qword ptr [rdi + 0x10]
02155715: mov    rcx, qword ptr [rip + 0x1b40bb4]         ; [0x3c962d0] meta:ʷʿʽʽʵʻʶʹʼʼʺ_TypeInfo
0215571c: cmp    dword ptr [rcx + 0xe0], 0
02155723: jne    0x18215572a
02155725: call   0x182f60cf0
0215572a: movdqa xmmword ptr [rsp + 0x70], xmm6
02155730: xor    r9d, r9d
02155733: lea    r8, [rsp + 0xe0]
0215573b: mov    rdx, rdi
0215573e: lea    rcx, [rsp + 0x70]
02155743: call   0x18215c750                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʼʻʹʾʲʼʹʷʸʳ
02155748: test   al, al
0215574a: je     0x182155798
0215574c: xor    r9d, r9d
0215574f: movzx  r8d, byte ptr [rsp + 0xe0]
02155758: movzx  edx, byte ptr [rsp + 0x190]
02155760: mov    rcx, qword ptr [rsp + 0x48]
02155765: call   0x1820cfd40
0215576a: test   rax, rax
0215576d: je     0x182155b00
02155773: mov    qword ptr [rsp + 0x20], r12
02155778: mov    r9, qword ptr [rsp + 0xf0]
02155780: mov    r8, qword ptr [rsp + 0xe8]
02155788: movzx  edx, byte ptr [rsp + 0xe0]
02155790: mov    rcx, rax
02155793: call   0x18213b130
02155798: xor    r8d, r8d
0215579b: mov    rdx, qword ptr [rip + 0x1b50b8e]         ; [0x3ca6330] str:'ENABLE_CHART_DYNAMICS'
021557a2: mov    rcx, r14
021557a5: call   0x181981570                              ; System.String$$op_Equality
021557aa: test   al, al
021557ac: jne    0x1821557cd
021557ae: xor    r8d, r8d
021557b1: mov    rdx, qword ptr [rip + 0x1ba7170]         ; [0x3cfc928] str:'[ENABLE_CHART_DYNAMICS]'
021557b8: mov    rcx, r14
021557bb: call   0x181981570                              ; System.String$$op_Equality
021557c0: test   al, al
021557c2: mov    r14, qword ptr [rsp + 0x48]
021557c7: je     0x1821552af
021557cd: mov    byte ptr [rsp + 0x41], 1
021557d2: jmp    0x1821552aa
021557d7: mov    rdx, qword ptr [rip + 0x1b6a6d2]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021557de: mov    rcx, rbx
021557e1: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021557e6: jmp    0x182155820
021557e8: mov    rdx, qword ptr [rip + 0x1b6a6c1]         ; [0x3cbfeb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʵʸʸʲˁˁʾʶʿʿʷ>.Dispose()
021557ef: mov    rcx, qword ptr [rsp + 0x68]
021557f4: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021557f9: mov    rcx, qword ptr [rsp + 0x60]
021557fe: test   rcx, rcx
02155801: jne    0x182155b06
02155807: xor    r12d, r12d
0215580a: mov    r14, qword ptr [rsp + 0x48]
0215580f: movzx  eax, byte ptr [rsp + 0x40]
02155814: mov    dword ptr [rsp + 0x54], eax
02155818: mov    r13, qword ptr [rsp + 0xb0]
02155820: xor    r9d, r9d
02155823: lea    r8, [rsp + 0xb8]
0215582b: movzx  edx, byte ptr [rsp + 0x190]
02155833: mov    rcx, r14
02155836: call   0x182152b20
0215583b: test   al, al
0215583d: je     0x182155a72
02155843: mov    r15, qword ptr [rsp + 0xb8]
0215584b: mov    qword ptr [rsp + 0x70], r15
02155850: mov    r14d, dword ptr [rsp + 0xc0]
02155858: mov    dword ptr [rsp + 0x48], r14d
0215585d: mov    esi, r12d
02155860: mov    dword ptr [rsp + 0x58], esi
02155864: cmp    esi, r14d
02155867: jge    0x1821559f9
0215586d: jae    0x182155b1d
02155873: movsxd rax, esi
02155876: mov    rdi, qword ptr [r15 + rax*8]
0215587a: test   rdi, rdi
0215587d: je     0x1821559f2
02155883: cmp    rdi, r13
02155886: je     0x1821559f2
0215588c: test   r13, r13
0215588f: je     0x182155b23
02155895: mov    rcx, qword ptr [rdi + 0x78]
02155899: test   rcx, rcx
0215589c: je     0x182155b23
021558a2: mov    r8, qword ptr [rip + 0x1b7c017]          ; [0x3cd18c0] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.AddRange()
021558a9: mov    rdx, qword ptr [r13 + 0x78]
021558ad: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
021558b2: mov    rcx, qword ptr [rdi + 0x70]
021558b6: test   rcx, rcx
021558b9: je     0x182155b23
021558bf: mov    r8, qword ptr [rip + 0x1b79efa]          ; [0x3ccf7c0] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.AddRange()
021558c6: mov    rdx, qword ptr [r13 + 0x70]
021558ca: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
021558cf: mov    rcx, qword ptr [rdi + 0x88]
021558d6: test   rcx, rcx
021558d9: je     0x182155b23
021558df: mov    r8, qword ptr [rip + 0x1b77962]          ; [0x3ccd248] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.AddRange()
021558e6: mov    rdx, qword ptr [r13 + 0x88]
021558ed: call   0x18076fc70                              ; System.Collections.Generic.List<object>$$AddRange
021558f2: mov    rdx, qword ptr [r13 + 0x80]
021558f9: test   rdx, rdx
021558fc: je     0x182155b23
02155902: mov    r8, qword ptr [rip + 0x1b78a77]          ; [0x3cce380] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.GetEnumerator()
02155909: lea    rcx, [rsp + 0x90]
02155911: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
02155916: movups xmm0, xmmword ptr [rsp + 0x90]
0215591e: movups xmmword ptr [rsp + 0xf8], xmm0
02155926: movsd  xmm1, qword ptr [rsp + 0xa0]
0215592f: movsd  qword ptr [rsp + 0x108], xmm1
02155938: mov    qword ptr [rsp + 0x60], r12
0215593d: lea    rbx, [rsp + 0xf8]
02155945: mov    qword ptr [rsp + 0x68], rbx
0215594a: nop    word ptr [rax + rax]
02155950: mov    rdx, qword ptr [rip + 0x1b69d19]         ; [0x3cbf670] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.MoveNext()
02155957: lea    rcx, [rsp + 0xf8]
0215595f: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02155964: test   al, al
02155966: je     0x1821559a0
02155968: mov    rdx, qword ptr [rsp + 0x108]
02155970: test   rdx, rdx
02155973: je     0x182155b11
02155979: movzx  eax, byte ptr [rdx + 0x20]
0215597d: cmp    byte ptr [rdi + 0x59], al
02155980: jl     0x182155950
02155982: mov    rcx, qword ptr [rdi + 0x80]
02155989: test   rcx, rcx
0215598c: je     0x182155b0c
02155992: mov    r8, qword ptr [rip + 0x1b78867]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
02155999: call   0x182c425f0
0215599e: jmp    0x182155950
021559a0: mov    rdx, qword ptr [rip + 0x1b69c09]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
021559a7: mov    rcx, rbx
021559aa: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021559af: jmp    0x1821559f2
021559b1: mov    rdx, qword ptr [rip + 0x1b69bf8]         ; [0x3cbf5b0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʷʸʽʳʹʶˁʺʳʿ>.Dispose()
021559b8: mov    rcx, qword ptr [rsp + 0x68]
021559bd: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021559c2: mov    rcx, qword ptr [rsp + 0x60]
021559c7: test   rcx, rcx
021559ca: jne    0x182155b17
021559d0: xor    r12d, r12d
021559d3: movzx  ebx, byte ptr [rsp + 0x40]
021559d8: mov    dword ptr [rsp + 0x54], ebx
021559dc: mov    r13, qword ptr [rsp + 0xb0]
021559e4: mov    esi, dword ptr [rsp + 0x58]
021559e8: mov    r14d, dword ptr [rsp + 0x48]
021559ed: mov    r15, qword ptr [rsp + 0x70]
021559f2: inc    esi
021559f4: jmp    0x182155860
021559f9: mov    rdi, qword ptr [rsp + 0xb8]
02155a01: mov    ebx, dword ptr [rsp + 0xc0]
02155a08: test   ebx, ebx
02155a0a: jle    0x182155a72
02155a0c: nop    dword ptr [rax]
02155a10: cmp    r12d, ebx
02155a13: jae    0x182155b1d
02155a19: mov    eax, r12d
02155a1c: mov    rcx, qword ptr [rdi + rax*8]
02155a20: test   rcx, rcx
02155a23: je     0x182155a41
02155a25: mov    eax, dword ptr [rsp + 0x54]
02155a29: mov    byte ptr [rcx + 0xa1], al
02155a2f: movzx  eax, byte ptr [rsp + 0x42]
02155a34: mov    byte ptr [rcx + 0xa2], al
02155a3a: xor    edx, edx
02155a3c: call   0x18215c320                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʻʳʶʷʷʽʶʼʽʽ
02155a41: inc    r12d
02155a44: cmp    r12d, ebx
02155a47: jl     0x182155a13
02155a49: jmp    0x182155a72
02155a4b: mov    rcx, qword ptr [rip + 0x1b3e8ee]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02155a52: cmp    dword ptr [rcx + 0xe0], 0
02155a59: jne    0x182155a60
02155a5b: call   0x182f60cf0
02155a60: xor    r9d, r9d
02155a63: movzx  r8d, r15b
02155a67: mov    rdx, rbx
02155a6a: mov    rcx, rdi
02155a6d: call   0x182154180                              ; ʶʲʻʾʾʺʴˀʷʼˀ$$ʲʲʴʹʳʹʸʿʴˀʺ
02155a72: lea    r11, [rsp + 0x150]
02155a7a: mov    rbx, qword ptr [r11 + 0x30]
02155a7e: mov    rsi, qword ptr [r11 + 0x38]
02155a82: movaps xmm6, xmmword ptr [r11 - 0x10]
02155a87: mov    rsp, r11
02155a8a: pop    r15
02155a8c: pop    r14
02155a8e: pop    r13
02155a90: pop    r12
02155a92: pop    rdi
02155a93: ret    
02155a94: call   0x182f60c50
02155a99: nop    
02155a9a: call   0x182f60c50
02155a9f: call   0x182f60c50
02155aa4: call   0x182f60c50
02155aa9: lea    rcx, [rip + 0x1b3fc60]                   ; [0x3c95710] meta:ʷʶʴʳʽʷʿʳʾˁʺ_TypeInfo
02155ab0: call   0x182f609d0
02155ab5: mov    rcx, rax
02155ab8: call   0x182f60c00
02155abd: mov    rbx, rax
02155ac0: lea    rcx, [rip + 0x1b50009]                   ; [0x3ca5ad0] str:'Invalid tom marker note number'
02155ac7: call   0x182f609d0
02155acc: xor    r8d, r8d
02155acf: mov    rdx, rax
02155ad2: mov    rcx, rbx
02155ad5: call   0x1820d5f30                              ; ʷʶʴʳʽʷʿʳʾˁʺ$$.ctor
02155ada: lea    rcx, [rip + 0x1bae0e7]                   ; [0x3d03bc8] metamethod:Method$ʶʲʻʾʾʺʴˀʷʼˀ.ʵʽʳʷʿʾʴʻʸʿʾ()
02155ae1: call   0x182f609d0
02155ae6: mov    rdx, rax
02155ae9: mov    rcx, rbx
02155aec: call   0x182f60c10
02155af1: call   0x182f60c50
02155af6: call   0x182f60c50
02155afb: call   0x182f60c50
02155b00: call   0x182f60c50
02155b05: nop    
02155b06: call   0x182f60ce0
02155b0b: nop    
02155b0c: call   0x182f60c50
02155b11: call   0x182f60c50
02155b16: nop    
02155b17: call   0x182f60ce0
02155b1c: int3   
02155b1d: call   0x182f60c40
02155b22: int3   
02155b23: call   0x182f60c50
02155b28: int3   
02155b29: nop    dword ptr [rax]
02155b2c: mov    ss, word ptr [rbp + rdx + 2]
