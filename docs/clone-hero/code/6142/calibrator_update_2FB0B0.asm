002fb0b0: push   rdi
002fb0b2: sub    rsp, 0x50
002fb0b6: cmp    byte ptr [rip + 0x3c129fb], 0            ; [0x3f0dab8] (bss)
002fb0bd: mov    rdi, rcx
002fb0c0: jne    0x1802fb0f9
002fb0c2: lea    rcx, [rip + 0x39f47c7]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb0c9: call   0x182f609b0
002fb0ce: lea    rcx, [rip + 0x39cf8cb]                   ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
002fb0d5: call   0x182f609b0
002fb0da: lea    rcx, [rip + 0x39a1e4f]                   ; [0x3c9cf30] meta:ʽˁʹˀʶʻʻˀʻʷʾ_TypeInfo
002fb0e1: call   0x182f609b0
002fb0e6: lea    rcx, [rip + 0x39d0943]                   ; [0x3ccba30] str:'Main Menu'
002fb0ed: call   0x182f609b0
002fb0f2: mov    byte ptr [rip + 0x3c129bf], 1            ; [0x3f0dab8] (bss)
002fb0f9: mov    ecx, dword ptr [rdi + 0x44]
002fb0fc: mov    qword ptr [rsp + 0x68], rbx
002fb101: mov    qword ptr [rsp + 0x70], rbp
002fb106: mov    qword ptr [rsp + 0x78], rsi
002fb10b: movaps xmmword ptr [rsp + 0x40], xmm6
002fb110: movaps xmmword ptr [rsp + 0x30], xmm7
002fb115: movaps xmmword ptr [rsp + 0x20], xmm8
002fb11b: test   ecx, ecx
002fb11d: je     0x1802fb5e8
002fb123: sub    ecx, 1
002fb126: je     0x1802fb3e7
002fb12c: sub    ecx, 1
002fb12f: je     0x1802fb3aa
002fb135: sub    ecx, 1
002fb138: je     0x1802fb222
002fb13e: cmp    ecx, 1
002fb141: jne    0x1802fb1fd
002fb147: xor    edx, edx
002fb149: mov    rcx, rdi
002fb14c: call   0x1802fafd0                              ; Calibrator$$ˀʷʵʲʷʼˁʽʿˁʺ
002fb151: test   rax, rax
002fb154: je     0x1802fb18e
002fb156: xor    r8d, r8d
002fb159: mov    rdx, rax
002fb15c: mov    rcx, rdi
002fb15f: call   0x1802f94f0                              ; Calibrator$$ʳʶˁʻʹʾʻʼʼʸʴ
002fb164: mov    rcx, qword ptr [rdi + 0x28]
002fb168: test   rcx, rcx
002fb16b: je     0x1802fb6c8
002fb171: xor    edx, edx
002fb173: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
002fb178: test   rax, rax
002fb17b: je     0x1802fb6c8
002fb181: xor    r8d, r8d
002fb184: xor    edx, edx
002fb186: mov    rcx, rax
002fb189: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
002fb18e: xor    edx, edx
002fb190: mov    rcx, rdi
002fb193: call   0x1802fa3f0                              ; Calibrator$$ʸʸʿʴʼʵˁˁˁʸʲ
002fb198: test   rax, rax
002fb19b: je     0x1802fb1fd
002fb19d: mov    rax, qword ptr [rip + 0x39f46ec]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb1a4: cmp    dword ptr [rax + 0xe0], 0
002fb1ab: jne    0x1802fb1bc
002fb1ad: mov    rcx, rax
002fb1b0: call   0x182f60cf0
002fb1b5: mov    rax, qword ptr [rip + 0x39f46d4]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb1bc: mov    rax, qword ptr [rax + 0xb8]
002fb1c3: mov    rcx, qword ptr [rax + 8]
002fb1c7: test   rcx, rcx
002fb1ca: je     0x1802fb6c8
002fb1d0: xor    r8d, r8d
002fb1d3: mov    dl, 1
002fb1d5: call   0x180234790                              ; GlobalVariables$$ʶʼʴʹʷʴʾʿʴʾʴ
002fb1da: mov    rcx, qword ptr [rip + 0x39cf7bf]         ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
002fb1e1: cmp    dword ptr [rcx + 0xe0], 0
002fb1e8: jne    0x1802fb1ef
002fb1ea: call   0x182f60cf0
002fb1ef: mov    rcx, qword ptr [rip + 0x39d083a]         ; [0x3ccba30] str:'Main Menu'
002fb1f6: xor    edx, edx
002fb1f8: call   0x18289c250                              ; UnityEngine.SceneManagement.SceneManager$$LoadScene
002fb1fd: movaps xmm8, xmmword ptr [rsp + 0x20]
002fb203: movaps xmm7, xmmword ptr [rsp + 0x30]
002fb208: movaps xmm6, xmmword ptr [rsp + 0x40]
002fb20d: mov    rsi, qword ptr [rsp + 0x78]
002fb212: mov    rbp, qword ptr [rsp + 0x70]
002fb217: mov    rbx, qword ptr [rsp + 0x68]
002fb21c: add    rsp, 0x50
002fb220: pop    rdi
002fb221: ret    
002fb222: xor    ecx, ecx
002fb224: call   0x182887b60                              ; UnityEngine.Time$$get_realtimeSinceStartup
002fb229: movss  dword ptr [rdi + 0x3c], xmm0
002fb22e: xor    edx, edx
002fb230: subss  xmm0, dword ptr [rdi + 0x38]
002fb235: mov    rcx, rdi
002fb238: movss  dword ptr [rdi + 0x40], xmm0
002fb23d: call   0x1802faa40                              ; Calibrator$$ʾʸʷʺʹˁʿʹʽʺʿ
002fb242: movss  xmm0, dword ptr [rdi + 0x40]
002fb247: cvtps2pd xmm0, xmm0
002fb24a: comisd xmm0, xmmword ptr [rdi + 0x68]
002fb24f: jbe    0x1802fb391
002fb255: cmp    byte ptr [rip + 0x3c12867], 0            ; [0x3f0dac3] (bss)
002fb25c: jne    0x1802fb295
002fb25e: lea    rcx, [rip + 0x3a134d3]                   ; [0x3d0e738] meta:int_TypeInfo
002fb265: call   0x182f609b0
002fb26a: lea    rcx, [rip + 0x39fb95f]                   ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
002fb271: call   0x182f609b0
002fb276: lea    rcx, [rip + 0x399d0a3]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
002fb27d: call   0x182f609b0
002fb282: lea    rcx, [rip + 0x39eb64f]                   ; [0x3ce68d8] str:'Video Cal: {0} ms\npress green to return to the main menu, or red to try again!'
002fb289: call   0x182f609b0
002fb28e: mov    byte ptr [rip + 0x3c1282e], 1            ; [0x3f0dac3] (bss)
002fb295: mov    rcx, qword ptr [rdi + 0x28]
002fb299: test   rcx, rcx
002fb29c: je     0x1802fb6ce
002fb2a2: xor    edx, edx
002fb2a4: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
002fb2a9: test   rax, rax
002fb2ac: je     0x1802fb6ce
002fb2b2: xor    r8d, r8d
002fb2b5: mov    dl, 1
002fb2b7: mov    rcx, rax
002fb2ba: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
002fb2bf: mov    rcx, qword ptr [rdi + 0x48]
002fb2c3: xor    edx, edx
002fb2c5: call   0x18027a0b0                              ; ʵʾʴʲʷˀʽʸʴʻʽ$$ʾʾʶʵʶˀʶʲʸʵʴ
002fb2ca: cmp    byte ptr [rip + 0x3c10be0], 0            ; [0x3f0beb1] (bss)
002fb2d1: xorps  xmm6, xmm6
002fb2d4: cvtsd2ss xmm6, xmm0
002fb2d8: mulss  xmm6, dword ptr [rip + 0x2d69624]        ; [0x3064904] flt=1000.0 i=1148846080
002fb2e0: jne    0x1802fb2f5
002fb2e2: lea    rcx, [rip + 0x39a747f]                   ; [0x3ca2768] meta:System.Math_TypeInfo
002fb2e9: call   0x182f609b0
002fb2ee: mov    byte ptr [rip + 0x3c10bbc], 1            ; [0x3f0beb1] (bss)
002fb2f5: mov    rcx, qword ptr [rip + 0x39a746c]         ; [0x3ca2768] meta:System.Math_TypeInfo
002fb2fc: cmp    dword ptr [rcx + 0xe0], 0
002fb303: jne    0x1802fb30a
002fb305: call   0x182f60cf0
002fb30a: cvtps2pd xmm0, xmm6
002fb30d: call   0x183031130
002fb312: mov    rax, qword ptr [rip + 0x399d007]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
002fb319: mov    rcx, qword ptr [rax + 0xb8]
002fb320: mov    rcx, qword ptr [rcx + 0x20]
002fb324: test   rcx, rcx
002fb327: je     0x1802fb6ce
002fb32d: mov    r8, qword ptr [rip + 0x39fb89c]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
002fb334: cvttsd2si ebx, xmm0
002fb338: mov    edx, ebx
002fb33a: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
002fb33f: mov    rcx, qword ptr [rip + 0x3a133f2]         ; [0x3d0e738] meta:int_TypeInfo
002fb346: lea    rdx, [rsp + 0x60]
002fb34b: mov    rsi, qword ptr [rdi + 0x28]
002fb34f: mov    dword ptr [rsp + 0x60], ebx
002fb353: call   0x182f5fbe0
002fb358: mov    rcx, qword ptr [rip + 0x39eb579]         ; [0x3ce68d8] str:'Video Cal: {0} ms\npress green to return to the main menu, or red to try again!'
002fb35f: xor    r8d, r8d
002fb362: mov    rdx, rax
002fb365: call   0x181982590                              ; System.String$$Format
002fb36a: test   rsi, rsi
002fb36d: je     0x1802fb6ce
002fb373: mov    r9, qword ptr [rsi]
002fb376: mov    rdx, rax
002fb379: mov    rcx, rsi
002fb37c: mov    r8, qword ptr [r9 + 0x560]
002fb383: call   qword ptr [r9 + 0x558]
002fb38a: mov    dword ptr [rdi + 0x44], 4
002fb391: mov    rcx, qword ptr [rdi + 0x70]
002fb395: test   rcx, rcx
002fb398: je     0x1802fb6c8
002fb39e: xor    edx, edx
002fb3a0: call   0x180121120                              ; CalVideoRenderer$$ʶʾʴˁʼʳˁʽʷˁʹ
002fb3a5: jmp    0x1802fb1fd
002fb3aa: xor    edx, edx
002fb3ac: mov    rcx, rdi
002fb3af: call   0x1802fafd0                              ; Calibrator$$ˀʷʵʲʷʼˁʽʿˁʺ
002fb3b4: mov    rcx, rdi
002fb3b7: test   rax, rax
002fb3ba: jne    0x1802fb3df
002fb3bc: xor    edx, edx
002fb3be: call   0x1802fa3f0                              ; Calibrator$$ʸʸʿʴʼʵˁˁˁʸʲ
002fb3c3: test   rax, rax
002fb3c6: je     0x1802fb1fd
002fb3cc: xor    r8d, r8d
002fb3cf: mov    rdx, rax
002fb3d2: mov    rcx, rdi
002fb3d5: call   0x1802f94f0                              ; Calibrator$$ʳʶˁʻʹʾʻʼʼʸʴ
002fb3da: jmp    0x1802fb6a1
002fb3df: mov    rdx, rax
002fb3e2: jmp    0x1802fb699
002fb3e7: cmp    byte ptr [rdi + 0x60], 0
002fb3eb: xorps  xmm8, xmm8
002fb3ef: je     0x1802fb446
002fb3f1: mov    rcx, qword ptr [rip + 0x39a1b38]         ; [0x3c9cf30] meta:ʽˁʹˀʶʻʻˀʻʷʾ_TypeInfo
002fb3f8: cmp    dword ptr [rcx + 0xe0], 0
002fb3ff: jne    0x1802fb406
002fb401: call   0x182f60cf0
002fb406: xor    ecx, ecx
002fb408: call   0x18028bdf0                              ; ʽˁʹˀʶʻʻˀʻʷʾ$$ʷʺˁʵʵʼʴʻʻˁʻ
002fb40d: ucomisd xmm0, xmm8
002fb412: movaps xmm6, xmm0
002fb415: jp     0x1802fb41d
002fb417: je     0x1802fb1fd
002fb41d: xor    ecx, ecx
002fb41f: call   0x182887b60                              ; UnityEngine.Time$$get_realtimeSinceStartup
002fb424: subss  xmm0, dword ptr [rdi + 0x38]
002fb429: xorps  xmm1, xmm1
002fb42c: mov    byte ptr [rdi + 0x60], 0
002fb430: cvtss2sd xmm0, xmm0
002fb434: subsd  xmm0, xmm6
002fb438: cvtsd2ss xmm1, xmm0
002fb43c: addss  xmm1, dword ptr [rdi + 0x38]
002fb441: movss  dword ptr [rdi + 0x38], xmm1
002fb446: cmp    byte ptr [rip + 0x3c1266d], 0            ; [0x3f0daba] (bss)
002fb44d: jne    0x1802fb486
002fb44f: lea    rcx, [rip + 0x39c9082]                   ; [0x3cc44d8] meta:Calibrator_TypeInfo
002fb456: call   0x182f609b0
002fb45b: lea    rcx, [rip + 0x3a132d6]                   ; [0x3d0e738] meta:int_TypeInfo
002fb462: call   0x182f609b0
002fb467: lea    rcx, [rip + 0x39a1ac2]                   ; [0x3c9cf30] meta:ʽˁʹˀʶʻʻˀʻʷʾ_TypeInfo
002fb46e: call   0x182f609b0
002fb473: lea    rcx, [rip + 0x39989be]                   ; [0x3c93e38] str:'<size=75>{0}</size>'
002fb47a: call   0x182f609b0
002fb47f: mov    byte ptr [rip + 0x3c12634], 1            ; [0x3f0daba] (bss)
002fb486: mov    rcx, qword ptr [rip + 0x39a1aa3]         ; [0x3c9cf30] meta:ʽˁʹˀʶʻʻˀʻʷʾ_TypeInfo
002fb48d: cmp    dword ptr [rcx + 0xe0], 0
002fb494: jne    0x1802fb49b
002fb496: call   0x182f60cf0
002fb49b: xor    ecx, ecx
002fb49d: call   0x18028bdf0                              ; ʽˁʹˀʶʻʻˀʻʷʾ$$ʷʺˁʵʵʼʴʻʻˁʻ
002fb4a2: mov    rax, qword ptr [rip + 0x39c902f]         ; [0x3cc44d8] meta:Calibrator_TypeInfo
002fb4a9: movaps xmm7, xmm0
002fb4ac: cmp    dword ptr [rax + 0xe0], 0
002fb4b3: jne    0x1802fb4c4
002fb4b5: mov    rcx, rax
002fb4b8: call   0x182f60cf0
002fb4bd: mov    rax, qword ptr [rip + 0x39c9014]         ; [0x3cc44d8] meta:Calibrator_TypeInfo
002fb4c4: comisd xmm7, xmm8
002fb4c9: mov    rax, qword ptr [rax + 0xb8]
002fb4d0: movsd  xmm1, qword ptr [rax]
002fb4d4: divsd  xmm1, qword ptr [rip + 0x2d6a6f4]        ; [0x3065bd0] dbl=60.0 q=0x404e000000000000
002fb4dc: movsd  xmm6, qword ptr [rip + 0x2d688dc]        ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
002fb4e4: divsd  xmm6, xmm1
002fb4e8: jbe    0x1802fb56e
002fb4ee: mov    rcx, qword ptr [rdi + 0x28]
002fb4f2: test   rcx, rcx
002fb4f5: je     0x1802fb6d4
002fb4fb: xor    edx, edx
002fb4fd: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
002fb502: test   rax, rax
002fb505: je     0x1802fb6d4
002fb50b: xor    r8d, r8d
002fb50e: mov    dl, 1
002fb510: mov    rcx, rax
002fb513: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
002fb518: mov    rcx, qword ptr [rip + 0x3a13219]         ; [0x3d0e738] meta:int_TypeInfo
002fb51f: lea    rdx, [rsp + 0x60]
002fb524: mov    rbx, qword ptr [rdi + 0x28]
002fb528: movaps xmm0, xmm7
002fb52b: divsd  xmm0, xmm6
002fb52f: cvttsd2si eax, xmm0
002fb533: mov    dword ptr [rsp + 0x60], eax
002fb537: call   0x182f5fbe0
002fb53c: mov    rcx, qword ptr [rip + 0x39988f5]         ; [0x3c93e38] str:'<size=75>{0}</size>'
002fb543: xor    r8d, r8d
002fb546: mov    rdx, rax
002fb549: call   0x181982590                              ; System.String$$Format
002fb54e: test   rbx, rbx
002fb551: je     0x1802fb6d4
002fb557: mov    r9, qword ptr [rbx]
002fb55a: mov    rdx, rax
002fb55d: mov    rcx, rbx
002fb560: mov    r8, qword ptr [r9 + 0x560]
002fb567: call   qword ptr [r9 + 0x558]
002fb56e: mulsd  xmm6, qword ptr [rip + 0x2d6a64a]        ; [0x3065bc0] dbl=4.0 q=0x4010000000000000
002fb576: comisd xmm7, xmm6
002fb57a: jbe    0x1802fb5a6
002fb57c: mov    rcx, qword ptr [rdi + 0x28]
002fb580: test   rcx, rcx
002fb583: je     0x1802fb6c8
002fb589: xor    edx, edx
002fb58b: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
002fb590: test   rax, rax
002fb593: je     0x1802fb6c8
002fb599: xor    r8d, r8d
002fb59c: xor    edx, edx
002fb59e: mov    rcx, rax
002fb5a1: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
002fb5a6: xor    ecx, ecx
002fb5a8: call   0x182887b60                              ; UnityEngine.Time$$get_realtimeSinceStartup
002fb5ad: movss  dword ptr [rdi + 0x3c], xmm0
002fb5b2: xor    edx, edx
002fb5b4: subss  xmm0, dword ptr [rdi + 0x38]
002fb5b9: mov    rcx, rdi
002fb5bc: movss  dword ptr [rdi + 0x40], xmm0
002fb5c1: call   0x1802faa40                              ; Calibrator$$ʾʸʷʺʹˁʿʹʽʺʿ
002fb5c6: movss  xmm0, dword ptr [rdi + 0x40]
002fb5cb: cvtps2pd xmm0, xmm0
002fb5ce: comisd xmm0, xmmword ptr [rdi + 0x68]
002fb5d3: jbe    0x1802fb1fd
002fb5d9: xor    edx, edx
002fb5db: mov    rcx, rdi
002fb5de: call   0x1802f9bc0                              ; Calibrator$$ʵʿʲʵʼˁʶˁʶˁʺ
002fb5e3: jmp    0x1802fb1fd
002fb5e8: cmp    byte ptr [rip + 0x3c124c6], 0            ; [0x3f0dab5] (bss)
002fb5ef: jne    0x1802fb604
002fb5f1: lea    rcx, [rip + 0x39f4298]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb5f8: call   0x182f609b0
002fb5fd: mov    byte ptr [rip + 0x3c124b1], 1            ; [0x3f0dab5] (bss)
002fb604: mov    rax, qword ptr [rip + 0x39f4285]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb60b: cmp    dword ptr [rax + 0xe0], 0
002fb612: jne    0x1802fb623
002fb614: mov    rcx, rax
002fb617: call   0x182f60cf0
002fb61c: mov    rax, qword ptr [rip + 0x39f426d]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002fb623: mov    rax, qword ptr [rax + 0xb8]
002fb62a: mov    rbp, qword ptr [rax + 8]
002fb62e: test   rbp, rbp
002fb631: je     0x1802fb6c8
002fb637: mov    rbp, qword ptr [rbp + 0x38]
002fb63b: test   rbp, rbp
002fb63e: je     0x1802fb6c8
002fb644: xor    ebx, ebx
002fb646: xor    eax, eax
002fb648: nop    dword ptr [rax + rax]
002fb650: cmp    eax, dword ptr [rbp + 0x18]
002fb653: jge    0x1802fb1fd
002fb659: cmp    ebx, dword ptr [rbp + 0x18]
002fb65c: jae    0x1802fb6da
002fb65e: movsxd rax, ebx
002fb661: mov    rsi, qword ptr [rbp + rax*8 + 0x20]
002fb666: test   rsi, rsi
002fb669: je     0x1802fb6c8
002fb66b: xor    edx, edx
002fb66d: mov    rcx, rsi
002fb670: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
002fb675: test   al, al
002fb677: je     0x1802fb68d
002fb679: mov    rcx, qword ptr [rsi + 0x18]
002fb67d: test   rcx, rcx
002fb680: je     0x1802fb6c8
002fb682: xor    edx, edx
002fb684: call   0x181f362c0                              ; Rewired.Player$$GetAnyButtonDown
002fb689: test   al, al
002fb68b: jne    0x1802fb693
002fb68d: inc    ebx
002fb68f: mov    eax, ebx
002fb691: jmp    0x1802fb650
002fb693: mov    rdx, rsi
002fb696: mov    rcx, rdi
002fb699: xor    r8d, r8d
002fb69c: call   0x1802fa910                              ; Calibrator$$ʽʹˁʻʳʲʽˁʲʽʼ
002fb6a1: mov    rcx, qword ptr [rdi + 0x28]
002fb6a5: test   rcx, rcx
002fb6a8: je     0x1802fb6c8
002fb6aa: xor    edx, edx
002fb6ac: call   0x1828783d0                              ; UnityEngine.Component$$get_gameObject
002fb6b1: test   rax, rax
002fb6b4: je     0x1802fb6c8
002fb6b6: xor    r8d, r8d
002fb6b9: xor    edx, edx
002fb6bb: mov    rcx, rax
002fb6be: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
002fb6c3: jmp    0x1802fb1fd
002fb6c8: call   0x182f60c50
002fb6cd: int3   
002fb6ce: call   0x182f60c50
002fb6d3: int3   
002fb6d4: call   0x182f60c50
002fb6d9: int3   
002fb6da: call   0x182f60c40
002fb6df: int3   
