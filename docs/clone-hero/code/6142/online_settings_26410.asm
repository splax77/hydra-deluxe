00026410: mov    qword ptr [rsp + 8], rbx
00026415: push   rdi
00026416: sub    rsp, 0x60
0002641a: cmp    byte ptr [rip + 0x3ee5876], 0            ; [0x3f0bc97] (bss)
00026421: movaps xmmword ptr [rsp + 0x50], xmm6
00026426: jne    0x1800264ff
0002642c: lea    rcx, [rip + 0x3ce7b35]                   ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
00026433: call   0x182f609b0
00026438: lea    rcx, [rip + 0x3c76551]                   ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
0002643f: call   0x182f609b0
00026444: lea    rcx, [rip + 0x3c77b0d]                   ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
0002644b: call   0x182f609b0
00026450: lea    rcx, [rip + 0x3c7a7e1]                   ; [0x3ca0c38] meta:ˁʼʼʼʷʳʹʲʿˀʻ_TypeInfo
00026457: call   0x182f609b0
0002645c: lea    rcx, [rip + 0x3cbea15]                   ; [0x3ce4e78] str:'minrequiredplayers'
00026463: call   0x182f609b0
00026468: lea    rcx, [rip + 0x3ca2461]                   ; [0x3cc88d0] str:'clientremovesongs'
0002646f: call   0x182f609b0
00026474: lea    rcx, [rip + 0x3cb797d]                   ; [0x3cdddf8] str:'maxplayers'
0002647b: call   0x182f609b0
00026480: lea    rcx, [rip + 0x3cd2491]                   ; [0x3cf8918] str:'onlyhostchoosesongs'
00026487: call   0x182f609b0
0002648c: lea    rcx, [rip + 0x3c73e15]                   ; [0x3c9a2a8] str:'quickplay'
00026493: call   0x182f609b0
00026498: lea    rcx, [rip + 0x3c86bc9]                   ; [0x3cad068] str:'settings'
0002649f: call   0x182f609b0
000264a4: lea    rcx, [rip + 0x3cb7acd]                   ; [0x3cddf78] str:'maxspectators'
000264ab: call   0x182f609b0
000264b0: lea    rcx, [rip + 0x3cb6941]                   ; [0x3cdcdf8] str:'defaultgamemode'
000264b7: call   0x182f609b0
000264bc: lea    rcx, [rip + 0x3caf485]                   ; [0x3cd5948] str:'lowsongspeed'
000264c3: call   0x182f609b0
000264c8: lea    rcx, [rip + 0x3cb79e9]                   ; [0x3cddeb8] str:'maxsongspeed'
000264cf: call   0x182f609b0
000264d4: lea    rcx, [rip + 0x3c85825]                   ; [0x3cabd00] str:'servertickrate'
000264db: call   0x182f609b0
000264e0: lea    rcx, [rip + 0x3cd21f9]                   ; [0x3cf86e0] str:'online'
000264e7: call   0x182f609b0
000264ec: lea    rcx, [rip + 0x3c93725]                   ; [0x3cb9c18] str:'songsperclient'
000264f3: call   0x182f609b0
000264f8: mov    byte ptr [rip + 0x3ee5798], 1            ; [0x3f0bc97] (bss)
000264ff: mov    rcx, qword ptr [rip + 0x3ce7a62]         ; [0x3d0df68] meta:ʲʷʽʹʺʴˀʵʽˁʻ_TypeInfo
00026506: cmp    dword ptr [rcx + 0xe0], 0
0002650d: jne    0x180026514
0002650f: call   0x182f60cf0
00026514: mov    r8, qword ptr [rip + 0x3cd21c5]          ; [0x3cf86e0] str:'online'
0002651b: lea    rcx, [rsp + 0x40]
00026520: mov    rdx, qword ptr [rip + 0x3c86b41]         ; [0x3cad068] str:'settings'
00026527: xor    r9d, r9d
0002652a: call   0x182115820                              ; ʲʷʽʹʺʴˀʵʽˁʻ$$ˁˀʽʵʶʾʻʸʷʽʾ
0002652f: mov    rcx, qword ptr [rip + 0x3c7645a]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
00026536: movups xmm6, xmmword ptr [rax]
00026539: call   0x182f60c00
0002653e: mov    r8, qword ptr [rip + 0x3c936d3]          ; [0x3cb9c18] str:'songsperclient'
00026545: lea    rdx, [rsp + 0x40]
0002654a: xor    edi, edi
0002654c: movdqa xmmword ptr [rsp + 0x40], xmm6
00026552: mov    qword ptr [rsp + 0x38], rdi
00026557: mov    r9d, 1
0002655d: mov    dword ptr [rsp + 0x30], 1
00026565: mov    rcx, rax
00026568: mov    dword ptr [rsp + 0x28], 0x19
00026570: mov    rbx, rax
00026573: mov    dword ptr [rsp + 0x20], 1
0002657b: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026580: mov    rcx, qword ptr [rip + 0x3c779d1]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026587: mov    rdx, rbx
0002658a: mov    rax, qword ptr [rcx + 0xb8]
00026591: mov    qword ptr [rax], rbx
00026594: mov    rcx, qword ptr [rip + 0x3c779bd]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
0002659b: mov    rcx, qword ptr [rcx + 0xb8]
000265a2: call   0x182f5fc00
000265a7: mov    rcx, qword ptr [rip + 0x3c763e2]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
000265ae: call   0x182f60c00
000265b3: mov    r8, qword ptr [rip + 0x3ca2316]          ; [0x3cc88d0] str:'clientremovesongs'
000265ba: lea    rdx, [rsp + 0x40]
000265bf: xor    r9d, r9d
000265c2: movdqa xmmword ptr [rsp + 0x40], xmm6
000265c8: mov    rcx, rax
000265cb: mov    qword ptr [rsp + 0x20], rdi
000265d0: mov    rbx, rax
000265d3: call   0x18210c430                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
000265d8: mov    rcx, qword ptr [rip + 0x3c77979]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000265df: mov    rdx, qword ptr [rcx + 0xb8]
000265e6: mov    qword ptr [rdx + 8], rbx
000265ea: mov    rdx, rbx
000265ed: mov    rcx, qword ptr [rip + 0x3c77964]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000265f4: mov    rcx, qword ptr [rcx + 0xb8]
000265fb: add    rcx, 8
000265ff: call   0x182f5fc00
00026604: mov    rcx, qword ptr [rip + 0x3c76385]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
0002660b: call   0x182f60c00
00026610: mov    r8, qword ptr [rip + 0x3cb78a1]          ; [0x3cddeb8] str:'maxsongspeed'
00026617: lea    rdx, [rsp + 0x40]
0002661c: mov    qword ptr [rsp + 0x38], rdi
00026621: mov    r9d, 0xfa
00026627: mov    dword ptr [rsp + 0x30], 5
0002662f: mov    rcx, rax
00026632: mov    dword ptr [rsp + 0x28], 0x1388
0002663a: mov    rbx, rax
0002663d: mov    dword ptr [rsp + 0x20], 5
00026645: movdqa xmmword ptr [rsp + 0x40], xmm6
0002664b: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026650: mov    rcx, qword ptr [rip + 0x3c77901]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026657: mov    rdx, rbx
0002665a: mov    rax, qword ptr [rcx + 0xb8]
00026661: mov    qword ptr [rax + 0x10], rbx
00026665: mov    rax, qword ptr [rip + 0x3c778ec]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
0002666c: mov    rcx, qword ptr [rax + 0xb8]
00026673: add    rcx, 0x10
00026677: call   0x182f5fc00
0002667c: mov    rcx, qword ptr [rip + 0x3c7630d]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
00026683: call   0x182f60c00
00026688: mov    r8, qword ptr [rip + 0x3caf2b9]          ; [0x3cd5948] str:'lowsongspeed'
0002668f: lea    rdx, [rsp + 0x40]
00026694: mov    qword ptr [rsp + 0x38], rdi
00026699: mov    r9d, 0x32
0002669f: mov    dword ptr [rsp + 0x30], 5
000266a7: mov    rcx, rax
000266aa: mov    dword ptr [rsp + 0x28], 0x1388
000266b2: mov    rbx, rax
000266b5: mov    dword ptr [rsp + 0x20], 5
000266bd: movdqa xmmword ptr [rsp + 0x40], xmm6
000266c3: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
000266c8: mov    rcx, qword ptr [rip + 0x3c77889]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000266cf: mov    rdx, qword ptr [rcx + 0xb8]
000266d6: mov    qword ptr [rdx + 0x18], rbx
000266da: mov    rdx, rbx
000266dd: mov    rax, qword ptr [rip + 0x3c77874]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000266e4: mov    rcx, qword ptr [rax + 0xb8]
000266eb: add    rcx, 0x18
000266ef: call   0x182f5fc00
000266f4: mov    rcx, qword ptr [rip + 0x3c76295]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
000266fb: call   0x182f60c00
00026700: mov    r8, qword ptr [rip + 0x3c855f9]          ; [0x3cabd00] str:'servertickrate'
00026707: lea    rdx, [rsp + 0x40]
0002670c: mov    qword ptr [rsp + 0x38], rdi
00026711: mov    r9d, 0x1e
00026717: mov    dword ptr [rsp + 0x30], 1
0002671f: mov    rcx, rax
00026722: mov    dword ptr [rsp + 0x28], 0x3e8
0002672a: mov    rbx, rax
0002672d: mov    dword ptr [rsp + 0x20], 1
00026735: movdqa xmmword ptr [rsp + 0x40], xmm6
0002673b: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026740: mov    rcx, qword ptr [rip + 0x3c77811]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026747: mov    rdx, qword ptr [rcx + 0xb8]
0002674e: mov    qword ptr [rdx + 0x20], rbx
00026752: mov    rdx, rbx
00026755: mov    rax, qword ptr [rip + 0x3c777fc]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
0002675c: mov    rcx, qword ptr [rax + 0xb8]
00026763: add    rcx, 0x20
00026767: call   0x182f5fc00
0002676c: mov    rcx, qword ptr [rip + 0x3c7621d]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
00026773: call   0x182f60c00
00026778: mov    r8, qword ptr [rip + 0x3cb77f9]          ; [0x3cddf78] str:'maxspectators'
0002677f: lea    rdx, [rsp + 0x40]
00026784: mov    qword ptr [rsp + 0x38], rdi
00026789: mov    r9d, 4
0002678f: mov    dword ptr [rsp + 0x30], 1
00026797: mov    rcx, rax
0002679a: mov    dword ptr [rsp + 0x28], 0x3e7
000267a2: mov    rbx, rax
000267a5: mov    dword ptr [rsp + 0x20], edi
000267a9: movdqa xmmword ptr [rsp + 0x40], xmm6
000267af: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
000267b4: mov    rcx, qword ptr [rip + 0x3c7779d]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000267bb: mov    rdx, qword ptr [rcx + 0xb8]
000267c2: mov    qword ptr [rdx + 0x28], rbx
000267c6: mov    rdx, rbx
000267c9: mov    rax, qword ptr [rip + 0x3c77788]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000267d0: mov    rcx, qword ptr [rax + 0xb8]
000267d7: add    rcx, 0x28
000267db: call   0x182f5fc00
000267e0: mov    rcx, qword ptr [rip + 0x3c761a9]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
000267e7: call   0x182f60c00
000267ec: mov    r8, qword ptr [rip + 0x3cd2125]          ; [0x3cf8918] str:'onlyhostchoosesongs'
000267f3: lea    rdx, [rsp + 0x40]
000267f8: xor    r9d, r9d
000267fb: movdqa xmmword ptr [rsp + 0x40], xmm6
00026801: mov    rcx, rax
00026804: mov    qword ptr [rsp + 0x20], rdi
00026809: mov    rbx, rax
0002680c: call   0x18210c430                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026811: mov    rcx, qword ptr [rip + 0x3c77740]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026818: mov    rdx, qword ptr [rcx + 0xb8]
0002681f: mov    qword ptr [rdx + 0x30], rbx
00026823: mov    rdx, rbx
00026826: mov    rcx, qword ptr [rip + 0x3c7772b]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
0002682d: mov    rcx, qword ptr [rcx + 0xb8]
00026834: add    rcx, 0x30
00026838: call   0x182f5fc00
0002683d: mov    rcx, qword ptr [rip + 0x3c7614c]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
00026844: call   0x182f60c00
00026849: mov    r8, qword ptr [rip + 0x3cb75a8]          ; [0x3cdddf8] str:'maxplayers'
00026850: lea    rdx, [rsp + 0x40]
00026855: mov    qword ptr [rsp + 0x38], rdi
0002685a: mov    r9d, 4
00026860: mov    dword ptr [rsp + 0x30], 1
00026868: mov    rcx, rax
0002686b: mov    dword ptr [rsp + 0x28], 4
00026873: mov    rbx, rax
00026876: mov    dword ptr [rsp + 0x20], 1
0002687e: movdqa xmmword ptr [rsp + 0x40], xmm6
00026884: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026889: mov    rcx, qword ptr [rip + 0x3c776c8]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026890: mov    rdx, qword ptr [rcx + 0xb8]
00026897: mov    qword ptr [rdx + 0x38], rbx
0002689b: mov    rdx, rbx
0002689e: mov    rax, qword ptr [rip + 0x3c776b3]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
000268a5: mov    rcx, qword ptr [rax + 0xb8]
000268ac: add    rcx, 0x38
000268b0: call   0x182f5fc00
000268b5: mov    rcx, qword ptr [rip + 0x3c760d4]         ; [0x3c9c990] meta:ʽʾʺʼʶʺʻʹʹʵʵ_TypeInfo
000268bc: call   0x182f60c00
000268c1: mov    r8, qword ptr [rip + 0x3cbe5b0]          ; [0x3ce4e78] str:'minrequiredplayers'
000268c8: lea    rdx, [rsp + 0x40]
000268cd: mov    qword ptr [rsp + 0x38], rdi
000268d2: mov    r9d, 1
000268d8: mov    dword ptr [rsp + 0x30], 1
000268e0: mov    rcx, rax
000268e3: mov    dword ptr [rsp + 0x28], 4
000268eb: mov    rbx, rax
000268ee: mov    dword ptr [rsp + 0x20], 1
000268f6: movdqa xmmword ptr [rsp + 0x40], xmm6
000268fc: call   0x18210c470                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$.ctor
00026901: mov    rcx, qword ptr [rip + 0x3c77650]         ; [0x3c9df58] meta:ʿʳʶʵˁʵʳʿʷʴʹ_TypeInfo
00026908: mov    rdx, qword ptr [rcx + 0xb8]
