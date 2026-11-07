003968d0: push   rsi
003968d2: push   rdi
003968d3: push   r13
003968d5: sub    rsp, 0xc0
003968dc: cmp    byte ptr [rip + 0x3b77a8f], 0            ; [0x3f0e372] (bss)
003968e3: mov    rsi, rcx
003968e6: movsxd rdi, edx
003968e9: jne    0x1803969ce
003968ef: lea    rcx, [rip + 0x3973f3a]                   ; [0x3d0a830] metamethod:Method$UnityEngine.Application.Quit()
003968f6: call   0x182f609b0
003968fb: lea    rcx, [rip + 0x3958f8e]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00396902: call   0x182f609b0
00396907: lea    rcx, [rip + 0x39459ba]                   ; [0x3cdc2c8] meta:System.Collections.Generic.IList<Player>_TypeInfo
0039690e: call   0x182f609b0
00396913: lea    rcx, [rip + 0x396756e]                   ; [0x3cfde88] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.get_Item()
0039691a: call   0x182f609b0
0039691f: lea    rcx, [rip + 0x396bd82]                   ; [0x3d026a8] metamethod:Method$System.Nullable<Hash>.GetValueOrDefault()
00396926: call   0x182f609b0
0039692b: lea    rcx, [rip + 0x396be36]                   ; [0x3d02768] metamethod:Method$System.Nullable<Hash>.get_HasValue()
00396932: call   0x182f609b0
00396937: lea    rcx, [rip + 0x39296ea]                   ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
0039693e: call   0x182f609b0
00396943: lea    rcx, [rip + 0x391053e]                   ; [0x3ca6e88] meta:string[]_TypeInfo
0039694a: call   0x182f609b0
0039694f: lea    rcx, [rip + 0x396027a]                   ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396956: call   0x182f609b0
0039695b: lea    rcx, [rip + 0x3955676]                   ; [0x3cebfd8] meta:UnityEngine.Events.UnityAction_TypeInfo
00396962: call   0x182f609b0
00396967: lea    rcx, [rip + 0x39233d2]                   ; [0x3cb9d40] str:'Replay file '
0039696e: call   0x182f609b0
00396973: lea    rcx, [rip + 0x395f3fe]                   ; [0x3cf5d78] str:'OK'
0039697a: call   0x182f609b0
0039697f: lea    rcx, [rip + 0x3932bc2]                   ; [0x3cc9548] str:'Unable to load chart file '
00396986: call   0x182f609b0
0039698b: lea    rcx, [rip + 0x391db1e]                   ; [0x3cb44b0] str:'Error while loading CLI replay'
00396992: call   0x182f609b0
00396997: lea    rcx, [rip + 0x391da5a]                   ; [0x3cb43f8] str:'Error while loading CLI chart'
0039699e: call   0x182f609b0
003969a3: lea    rcx, [rip + 0x392b256]                   ; [0x3cc1c00] str:' does not match the song file '
003969aa: call   0x182f609b0
003969af: lea    rcx, [rip + 0x391d4ba]                   ; [0x3cb3e70] str:''
003969b6: call   0x182f609b0
003969bb: lea    rcx, [rip + 0x3955ab6]                   ; [0x3cec478] str:'.'
003969c2: call   0x182f609b0
003969c7: mov    byte ptr [rip + 0x3b779a4], 1            ; [0x3f0e372] (bss)
003969ce: mov    rcx, qword ptr [rsi + 0x20]
003969d2: xor    r13d, r13d
003969d5: mov    qword ptr [rsp + 0xf0], rbx
003969dd: mov    qword ptr [rsp + 0xb8], rbp
003969e5: mov    qword ptr [rsp + 0xb0], r12
003969ed: mov    qword ptr [rsp + 0xa8], r14
003969f5: mov    qword ptr [rsp + 0xa0], r15
003969fd: mov    qword ptr [rsp + 0xe0], r13
00396a05: test   rcx, rcx
00396a08: je     0x1803970c0
00396a0e: mov    r8, qword ptr [rip + 0x3967473]          ; [0x3cfde88] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.get_Item()
00396a15: mov    edx, edi
00396a17: call   0x180735c20                              ; System.Collections.Generic.List<ushort>$$get_Item
00396a1c: movsx  ebx, ax
00396a1f: mov    eax, 6
00396a24: cmp    bl, 9
00396a27: movzx  ebp, bl
00396a2a: mov    word ptr [rsp + 0xe8], bx
00396a32: cmove  ebp, eax
00396a35: mov    rcx, qword ptr [rsi + 0x40]
00396a39: test   rcx, rcx
00396a3c: je     0x1803970c0
00396a42: xor    r9d, r9d
00396a45: mov    qword ptr [rsp + 0x20], r13
00396a4a: xor    r8d, r8d
00396a4d: xor    edx, edx
00396a4f: call   0x18035c110                              ; SongEntry$$ʿʲʷʹʿʳʾʳʷˀʾ
00396a54: mov    r14, rax
00396a57: test   rax, rax
00396a5a: je     0x18039703b
00396a60: movzx  r12d, byte ptr [rsp + 0xe9]
00396a69: xor    r9d, r9d
00396a6c: movzx  r8d, r12b
00396a70: movzx  edx, bpl
00396a74: mov    rcx, rax
00396a77: call   0x180393080
00396a7c: test   al, al
00396a7e: je     0x180396c4e
00396a84: cmp    qword ptr [rsi + 0x50], r13
00396a88: je     0x180396c84
00396a8e: lea    r9, [rsp + 0xe0]
00396a96: mov    qword ptr [rsp + 0x20], r13
00396a9b: movzx  r8d, r12b
00396a9f: movzx  edx, bpl
00396aa3: mov    rcx, r14
00396aa6: call   0x180392fa0
00396aab: test   al, al
00396aad: je     0x180396c84
00396ab3: mov    rdx, qword ptr [rsp + 0xe0]
00396abb: test   rdx, rdx
00396abe: je     0x1803970c0
00396ac4: xor    r8d, r8d
00396ac7: lea    rcx, [rsp + 0x80]
00396acf: call   0x18215f280                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʽʳʸʿʷʽʼˁʸʻʼ
00396ad4: mov    rcx, qword ptr [rsi + 0x60]
00396ad8: test   rcx, rcx
00396adb: je     0x1803970c0
00396ae1: cmp    byte ptr [rcx + 0x30], r13b
00396ae5: je     0x180396b24
00396ae7: movups xmm0, xmmword ptr [rcx + 0x31]
00396aeb: movups xmm1, xmmword ptr [rcx + 0x41]
00396aef: movaps xmmword ptr [rsp + 0x40], xmm0
00396af4: movups xmm0, xmmword ptr [rax]
00396af7: movaps xmmword ptr [rsp + 0x50], xmm1
00396afc: movups xmm1, xmmword ptr [rax + 0x10]
00396b00: movaps xmmword ptr [rsp + 0x60], xmm0
00396b05: movaps xmmword ptr [rsp + 0x70], xmm1
00396b0a: xor    r8d, r8d
00396b0d: lea    rdx, [rsp + 0x40]
00396b12: lea    rcx, [rsp + 0x60]
00396b17: call   0x180003150                              ; Blake3.Hash$$op_Inequality
00396b1c: test   al, al
00396b1e: je     0x180396c84
00396b24: mov    rcx, qword ptr [rip + 0x391035d]         ; [0x3ca6e88] meta:string[]_TypeInfo
00396b2b: mov    edx, 5
00396b30: mov    rdi, qword ptr [rsi + 0x98]
00396b37: call   0x182f5fc80
00396b3c: mov    rbx, rax
00396b3f: test   rax, rax
00396b42: je     0x1803970c0
00396b48: cmp    dword ptr [rax + 0x18], r13d
00396b4c: jbe    0x1803970c6
00396b52: mov    rdx, qword ptr [rip + 0x39231e7]         ; [0x3cb9d40] str:'Replay file '
00396b59: lea    rcx, [rax + 0x20]
00396b5d: mov    qword ptr [rax + 0x20], rdx
00396b61: call   0x182f5fc00
00396b66: cmp    dword ptr [rbx + 0x18], 1
00396b6a: jbe    0x1803970c6
00396b70: mov    rdx, qword ptr [rsi + 0x50]
00396b74: lea    rcx, [rbx + 0x28]
00396b78: mov    qword ptr [rbx + 0x28], rdx
00396b7c: call   0x182f5fc00
00396b81: cmp    dword ptr [rbx + 0x18], 2
00396b85: jbe    0x1803970c6
00396b8b: mov    rdx, qword ptr [rip + 0x392b06e]         ; [0x3cc1c00] str:' does not match the song file '
00396b92: lea    rcx, [rbx + 0x30]
00396b96: mov    qword ptr [rbx + 0x30], rdx
00396b9a: call   0x182f5fc00
00396b9f: mov    rcx, qword ptr [rsi + 0x40]
00396ba3: test   rcx, rcx
00396ba6: je     0x1803970c0
00396bac: xor    edx, edx
00396bae: call   0x1803602c0                              ; SongEntry$$get_ChartPath
00396bb3: cmp    dword ptr [rbx + 0x18], 3
00396bb7: jbe    0x1803970c6
00396bbd: lea    rcx, [rbx + 0x38]
00396bc1: mov    qword ptr [rbx + 0x38], rax
00396bc5: mov    rdx, rax
00396bc8: call   0x182f5fc00
00396bcd: cmp    dword ptr [rbx + 0x18], 4
00396bd1: jbe    0x1803970c6
00396bd7: mov    rdx, qword ptr [rip + 0x395589a]         ; [0x3cec478] str:'.'
00396bde: lea    rcx, [rbx + 0x40]
00396be2: mov    qword ptr [rbx + 0x40], rdx
00396be6: call   0x182f5fc00
00396beb: xor    edx, edx
00396bed: mov    rcx, rbx
00396bf0: call   0x18197eb90                              ; System.String$$Concat
00396bf5: mov    rcx, qword ptr [rip + 0x39553dc]         ; [0x3cebfd8] meta:UnityEngine.Events.UnityAction_TypeInfo
00396bfc: mov    rsi, rax
00396bff: call   0x182f60c00
00396c04: mov    r8, qword ptr [rip + 0x3973c25]          ; [0x3d0a830] metamethod:Method$UnityEngine.Application.Quit()
00396c0b: xor    r9d, r9d
00396c0e: xor    edx, edx
00396c10: mov    rcx, rax
00396c13: mov    rbx, rax
00396c16: call   0x180026bd0                              ; UnityEngine.AI.NavMesh.OnNavMeshPreUpdate$$.ctor
00396c1b: test   rdi, rdi
00396c1e: je     0x1803970c0
00396c24: mov    rdx, qword ptr [rip + 0x395f14d]         ; [0x3cf5d78] str:'OK'
00396c2b: mov    r9, rsi
00396c2e: mov    r8, qword ptr [rip + 0x391d87b]          ; [0x3cb44b0] str:'Error while loading CLI replay'
00396c35: mov    rcx, rdi
00396c38: mov    qword ptr [rsp + 0x30], r13
00396c3d: mov    qword ptr [rsp + 0x28], rdx
00396c42: xor    edx, edx
00396c44: mov    qword ptr [rsp + 0x20], rbx
00396c49: call   0x180363aa0                              ; UnpackingScene$$ʸʼʹʾʵʽʺʽʴʿʹ
00396c4e: xor    al, al
00396c50: mov    r15, qword ptr [rsp + 0xa0]
00396c58: mov    r14, qword ptr [rsp + 0xa8]
00396c60: mov    r12, qword ptr [rsp + 0xb0]
00396c68: mov    rbp, qword ptr [rsp + 0xb8]
00396c70: mov    rbx, qword ptr [rsp + 0xf0]
00396c78: add    rsp, 0xc0
00396c7f: pop    r13
00396c81: pop    rdi
00396c82: pop    rsi
00396c83: ret    
00396c84: mov    rax, qword ptr [rip + 0x3958c05]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00396c8b: cmp    dword ptr [rax + 0xe0], r13d
00396c92: jne    0x180396ca3
00396c94: mov    rcx, rax
00396c97: call   0x182f60cf0
00396c9c: mov    rax, qword ptr [rip + 0x3958bed]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00396ca3: mov    rax, qword ptr [rax + 0xb8]
00396caa: mov    rbp, qword ptr [rax + 8]
00396cae: test   rbp, rbp
00396cb1: je     0x1803970c0
00396cb7: mov    rbp, qword ptr [rbp + 0x38]
00396cbb: test   rbp, rbp
00396cbe: je     0x1803970c0
00396cc4: cmp    edi, dword ptr [rbp + 0x18]
00396cc7: jae    0x1803970c6
00396ccd: mov    rbp, qword ptr [rbp + rdi*8 + 0x20]
00396cd2: test   rbp, rbp
00396cd5: je     0x1803970c0
00396cdb: mov    dword ptr [rbp + 0x28], edi
00396cde: mov    rcx, qword ptr [rip + 0x3929343]         ; [0x3cc0028] meta:Rewired.ReInput_TypeInfo
00396ce5: cmp    byte ptr [rsi + 0x58], r13b
00396ce9: je     0x180396d32
00396ceb: cmp    dword ptr [rcx + 0xe0], r13d
00396cf2: jne    0x180396cf9
00396cf4: call   0x182f60cf0
00396cf9: xor    ecx, ecx
00396cfb: call   0x181f73d10                              ; Rewired.ReInput$$get_players
00396d00: test   rax, rax
00396d03: je     0x1803970c0
00396d09: xor    edx, edx
00396d0b: mov    rcx, rax
00396d0e: call   0x181d98a70                              ; Rewired.ReInput.PlayerHelper$$get_Players
00396d13: test   rax, rax
00396d16: je     0x1803970c0
00396d1c: mov    rdx, qword ptr [rip + 0x39455a5]         ; [0x3cdc2c8] meta:System.Collections.Generic.IList<Player>_TypeInfo
00396d23: xor    ecx, ecx
00396d25: xor    r9d, r9d
00396d28: mov    r8, rax
00396d2b: call   0x182c421d0
00396d30: jmp    0x180396d5a
00396d32: cmp    dword ptr [rcx + 0xe0], r13d
00396d39: jne    0x180396d40
00396d3b: call   0x182f60cf0
00396d40: xor    ecx, ecx
00396d42: call   0x181f73d10                              ; Rewired.ReInput$$get_players
00396d47: test   rax, rax
00396d4a: je     0x1803970c0
00396d50: xor    edx, edx
00396d52: mov    rcx, rax
00396d55: call   0x181d98940                              ; Rewired.ReInput.PlayerHelper$$GetSystemPlayer
00396d5a: lea    rcx, [rbp + 0x18]
00396d5e: mov    qword ptr [rbp + 0x18], rax
00396d62: mov    rdx, rax
00396d65: call   0x182f5fc00
00396d6a: mov    rcx, qword ptr [rsi + 0x38]
00396d6e: test   rcx, rcx
00396d71: je     0x1803970c0
00396d77: xor    edx, edx
00396d79: call   0x1800d8470                              ; ʹʺʿʻʲʹʻʹʹʵʸ$$ʽʲʾʸʿʷʲʶʲʻˁ
00396d7e: mov    rdi, rax
00396d81: test   rax, rax
00396d84: je     0x1803970c0
00396d8a: mov    rcx, qword ptr [rax + 0x50]
00396d8e: test   rcx, rcx
00396d91: je     0x1803970c0
00396d97: mov    r8, qword ptr [rip + 0x395fe32]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396d9e: xor    edx, edx
00396da0: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00396da5: mov    rcx, qword ptr [rdi + 0x18]
00396da9: test   rcx, rcx
00396dac: je     0x1803970c0
00396db2: mov    rdx, qword ptr [rip + 0x391d0b7]         ; [0x3cb3e70] str:''
00396db9: xor    r8d, r8d
00396dbc: call   0x18212b040                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʸʾʵʽʳʾʾʻʷʲʽ
00396dc1: mov    rax, qword ptr [rip + 0x3958ac8]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00396dc8: cmp    dword ptr [rax + 0xe0], r13d
00396dcf: jne    0x180396de0
00396dd1: mov    rcx, rax
00396dd4: call   0x182f60cf0
00396dd9: mov    rax, qword ptr [rip + 0x3958ab0]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00396de0: mov    rax, qword ptr [rax + 0xb8]
00396de7: mov    rcx, qword ptr [rax + 8]
00396deb: test   rcx, rcx
00396dee: je     0x1803970c0
00396df4: mov    r14, qword ptr [rcx + 0x140]
00396dfb: xor    edx, edx
00396dfd: mov    rcx, qword ptr [rdi + 0xa0]
00396e04: call   0x18212ad70                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʴˁʺʸʲʶʸʵʹʹʿ
00396e09: test   r14, r14
00396e0c: je     0x1803970c0
00396e12: xor    r8d, r8d
00396e15: mov    rdx, rax
00396e18: mov    rcx, r14
00396e1b: call   0x1802ad050                              ; ColorManager$$ˀʻʲʶʴʳʿʲʵʸʺ
00396e20: lea    rcx, [rdi + 0xa8]
00396e27: mov    qword ptr [rdi + 0xa8], rax
00396e2e: mov    rdx, rax
00396e31: call   0x182f5fc00
00396e36: cmp    byte ptr [rsi + 0x48], r13b
00396e3a: je     0x180396ea0
00396e3c: mov    r14, qword ptr [rdi + 0x18]
00396e40: xor    ecx, ecx
00396e42: call   0x18012f120                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʸʳˁʸʺʳʸʶʻˁʹ
00396e47: xor    edx, edx
00396e49: movzx  ecx, bl
00396e4c: mov    r15, rax
00396e4f: call   0x18214e3d0                              ; ʿʼʽʽʶʽʹʳʳʲʵ$$ʶˁʼʵʺʸʿʴʼˀʻ
00396e54: test   r15, r15
00396e57: je     0x1803970c0
00396e5d: xor    r8d, r8d
00396e60: mov    rdx, rax
00396e63: mov    rcx, r15
00396e66: call   0x18012ee30                              ; ʽʵʷˀʿʺʵʹʽʶˀ$$ʳʵʻʾˁʷʲˀˀʻʶ
00396e6b: test   r14, r14
00396e6e: je     0x1803970c0
00396e74: xor    r8d, r8d
00396e77: mov    rdx, rax
00396e7a: mov    rcx, r14
00396e7d: call   0x18212b040                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʸʾʵʽʳʾʾʻʷʲʽ
00396e82: mov    rcx, qword ptr [rdi + 0x50]
00396e86: test   rcx, rcx
00396e89: je     0x1803970c0
00396e8f: mov    r8, qword ptr [rip + 0x395fd3a]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396e96: mov    edx, 1
00396e9b: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00396ea0: mov    byte ptr [rdi + 0x10], bl
00396ea3: mov    byte ptr [rdi + 0x11], r12b
00396ea7: cmp    qword ptr [rsi + 0x60], r13
00396eab: jne    0x180396f44
00396eb1: mov    rcx, qword ptr [rdi + 0x48]
00396eb5: test   rcx, rcx
00396eb8: je     0x1803970c0
00396ebe: mov    r8, qword ptr [rip + 0x395fd0b]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396ec5: mov    edx, 1
00396eca: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00396ecf: mov    dword ptr [rdi + 0x14], 1
00396ed6: cmp    bl, 6
00396ed9: je     0x180396f1a
00396edb: cmp    bl, 9
00396ede: je     0x180396f1a
00396ee0: cmp    bl, 0xe
00396ee3: ja     0x180396f06
00396ee5: mov    ecx, 0x4c30
00396eea: bt     ecx, ebx
00396eed: jae    0x180396f06
00396eef: mov    rcx, qword ptr [rdi + 0x60]
00396ef3: test   rcx, rcx
00396ef6: je     0x1803970c0
00396efc: mov    edx, 1
00396f01: jmp    0x180397018
00396f06: mov    rcx, qword ptr [rdi + 0x60]
00396f0a: test   rcx, rcx
00396f0d: je     0x1803970c0
00396f13: xor    edx, edx
00396f15: jmp    0x180397018
00396f1a: mov    rcx, qword ptr [rdi + 0x60]
00396f1e: test   rcx, rcx
00396f21: je     0x1803970c0
00396f27: mov    r8, qword ptr [rip + 0x395fca2]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396f2e: mov    edx, 2
00396f33: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00396f38: mov    dword ptr [rdi + 0x14], 2
00396f3f: jmp    0x180397024
00396f44: mov    rax, qword ptr [rsi + 0x60]
00396f48: test   rax, rax
00396f4b: je     0x1803970c0
00396f51: mov    rdx, qword ptr [rax + 0x20]
00396f55: test   rdx, rdx
00396f58: je     0x1803970c0
00396f5e: mov    rcx, qword ptr [rdi + 0x60]
00396f62: test   rcx, rcx
00396f65: je     0x1803970c0
00396f6b: movzx  edx, byte ptr [rdx + 0x6c]
00396f6f: mov    r8, qword ptr [rip + 0x395fc5a]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00396f76: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00396f7b: mov    rax, qword ptr [rsi + 0x60]
00396f7f: test   rax, rax
00396f82: je     0x1803970c0
00396f88: mov    rax, qword ptr [rax + 0x20]
00396f8c: test   rax, rax
00396f8f: je     0x1803970c0
00396f95: mov    eax, dword ptr [rax + 0x70]
00396f98: mov    dword ptr [rdi + 0x14], eax
00396f9b: mov    rax, qword ptr [rsi + 0x60]
00396f9f: test   rax, rax
00396fa2: je     0x1803970c0
00396fa8: mov    rdx, qword ptr [rax + 0x20]
00396fac: test   rdx, rdx
00396faf: je     0x1803970c0
00396fb5: mov    rcx, qword ptr [rdi + 0x18]
00396fb9: test   rcx, rcx
00396fbc: je     0x1803970c0
00396fc2: mov    rdx, qword ptr [rdx + 0x50]
00396fc6: xor    r8d, r8d
00396fc9: call   0x18212b040                              ; ˁʼʼʼʷʳʹʲʿˀʻ$$ʸʾʵʽʳʾʾʻʷʲʽ
00396fce: mov    rdx, qword ptr [rsi + 0x60]
00396fd2: lea    rcx, [rdi + 0xe0]
00396fd9: mov    qword ptr [rdi + 0xe0], rdx
00396fe0: call   0x182f5fc00
00396fe5: mov    rax, qword ptr [rsi + 0x60]
00396fe9: test   rax, rax
00396fec: je     0x1803970c0
00396ff2: mov    rax, qword ptr [rax + 0x20]
00396ff6: test   rax, rax
00396ff9: je     0x1803970c0
00396fff: mov    rcx, qword ptr [rdi + 0x38]
00397003: mov    edx, r13d
00397006: cmp    byte ptr [rax + 0x8d], dl
0039700c: setne  dl
0039700f: test   rcx, rcx
00397012: je     0x1803970c0
00397018: mov    r8, qword ptr [rip + 0x395fbb1]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
0039701f: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
00397024: lea    rcx, [rbp + 0x10]
00397028: mov    qword ptr [rbp + 0x10], rdi
0039702c: mov    rdx, rdi
0039702f: call   0x182f5fc00
00397034: mov    al, 1
00397036: jmp    0x180396c50
0039703b: mov    rcx, qword ptr [rsi + 0x40]
0039703f: test   rcx, rcx
00397042: je     0x1803970c0
00397044: mov    rbx, qword ptr [rsi + 0x98]
0039704b: xor    edx, edx
0039704d: call   0x1803602c0                              ; SongEntry$$get_ChartPath
00397052: mov    r8, qword ptr [rip + 0x395541f]          ; [0x3cec478] str:'.'
00397059: xor    r9d, r9d
0039705c: mov    rcx, qword ptr [rip + 0x39324e5]         ; [0x3cc9548] str:'Unable to load chart file '
00397063: mov    rdx, rax
00397066: call   0x18197eef0                              ; System.String$$Concat
0039706b: mov    rcx, qword ptr [rip + 0x3954f66]         ; [0x3cebfd8] meta:UnityEngine.Events.UnityAction_TypeInfo
00397072: mov    rsi, rax
00397075: call   0x182f60c00
0039707a: mov    r8, qword ptr [rip + 0x39737af]          ; [0x3d0a830] metamethod:Method$UnityEngine.Application.Quit()
00397081: xor    r9d, r9d
00397084: xor    edx, edx
00397086: mov    rcx, rax
00397089: mov    rdi, rax
0039708c: call   0x180026bd0                              ; UnityEngine.AI.NavMesh.OnNavMeshPreUpdate$$.ctor
00397091: test   rbx, rbx
00397094: je     0x1803970c0
00397096: mov    rdx, qword ptr [rip + 0x395ecdb]         ; [0x3cf5d78] str:'OK'
0039709d: mov    r9, rsi
003970a0: mov    r8, qword ptr [rip + 0x391d351]          ; [0x3cb43f8] str:'Error while loading CLI chart'
003970a7: mov    rcx, rbx
003970aa: mov    qword ptr [rsp + 0x30], r13
003970af: mov    qword ptr [rsp + 0x28], rdx
003970b4: xor    edx, edx
003970b6: mov    qword ptr [rsp + 0x20], rdi
003970bb: jmp    0x180396c49
003970c0: call   0x182f60c50
003970c5: int3   
003970c6: call   0x182f60c40
003970cb: int3   
003970cc: int3   
003970cd: int3   
003970ce: int3   
003970cf: int3   
