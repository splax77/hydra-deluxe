00398f00: mov    qword ptr [rsp + 8], rbx
00398f05: mov    qword ptr [rsp + 0x10], rsi
00398f0a: push   rdi
00398f0b: sub    rsp, 0x40
00398f0f: cmp    byte ptr [rip + 0x3b7544d], 0            ; [0x3f0e363] (bss)
00398f16: mov    rbx, rcx
00398f19: jne    0x180398faa
00398f1f: lea    rcx, [rip + 0x397190a]                   ; [0x3d0a830] metamethod:Method$UnityEngine.Application.Quit()
00398f26: call   0x182f609b0
00398f2b: lea    rcx, [rip + 0x394f176]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
00398f32: call   0x182f609b0
00398f37: lea    rcx, [rip + 0x3956952]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00398f3e: call   0x182f609b0
00398f43: lea    rcx, [rip + 0x3964e86]                   ; [0x3cfddd0] metamethod:Method$System.Collections.Generic.List<ValueTuple<ʼˁʿʷʶʸʴʼʹʾʶ, ˁʹʴʸʸʲʸˁʺʸʸ>>.get_Count()
00398f4a: call   0x182f609b0
00398f4f: lea    rcx, [rip + 0x395dc7a]                   ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
00398f56: call   0x182f609b0
00398f5b: lea    rcx, [rip + 0x38ff3be]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00398f62: call   0x182f609b0
00398f67: lea    rcx, [rip + 0x395306a]                   ; [0x3cebfd8] meta:UnityEngine.Events.UnityAction_TypeInfo
00398f6e: call   0x182f609b0
00398f73: lea    rcx, [rip + 0x3953746]                   ; [0x3cec6c0] str:'No players were loaded. This may be caused by trying to load instruments/difficulties that do not exist in the chart file.'
00398f7a: call   0x182f609b0
00398f7f: lea    rcx, [rip + 0x395cdf2]                   ; [0x3cf5d78] str:'OK'
00398f86: call   0x182f609b0
00398f8b: lea    rcx, [rip + 0x393ce66]                   ; [0x3cd5df8] str:'Gameplay'
00398f92: call   0x182f609b0
00398f97: lea    rcx, [rip + 0x391b45a]                   ; [0x3cb43f8] str:'Error while loading CLI chart'
00398f9e: call   0x182f609b0
00398fa3: mov    byte ptr [rip + 0x3b753b9], 1            ; [0x3f0e363] (bss)
00398faa: mov    rax, qword ptr [rip + 0x39568df]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00398fb1: cmp    dword ptr [rax + 0xe0], 0
00398fb8: jne    0x180398fc9
00398fba: mov    rcx, rax
00398fbd: call   0x182f60cf0
00398fc2: mov    rax, qword ptr [rip + 0x39568c7]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00398fc9: mov    rax, qword ptr [rax + 0xb8]
00398fd0: xor    edi, edi
00398fd2: xor    ecx, ecx
00398fd4: mov    rsi, qword ptr [rax + 8]
00398fd8: mov    rax, qword ptr [rbx + 0x20]
00398fdc: test   rax, rax
00398fdf: je     0x180399144
00398fe5: cmp    ecx, dword ptr [rax + 0x18]
00398fe8: jge    0x180399011
00398fea: xor    r8d, r8d
00398fed: mov    edx, edi
00398fef: mov    rcx, rbx
00398ff2: call   0x1803968d0                              ; CliGameLauncher$$ʶʷʶʲʽʼʼʾʿʵʵ
00398ff7: test   al, al
00398ff9: je     0x180398ffe
00398ffb: inc    dword ptr [rbx + 0x78]
00398ffe: mov    rax, qword ptr [rbx + 0x20]
00399002: inc    edi
00399004: mov    ecx, edi
00399006: test   rax, rax
00399009: je     0x180399144
0039900f: jmp    0x180398fe5
00399011: cmp    dword ptr [rbx + 0x78], 0
00399015: jg     0x18039909c
0039901b: mov    rcx, qword ptr [rip + 0x3952fb6]         ; [0x3cebfd8] meta:UnityEngine.Events.UnityAction_TypeInfo
00399022: mov    rdi, qword ptr [rbx + 0x98]
00399029: call   0x182f60c00
0039902e: mov    r8, qword ptr [rip + 0x39717fb]          ; [0x3d0a830] metamethod:Method$UnityEngine.Application.Quit()
00399035: xor    r9d, r9d
00399038: xor    edx, edx
0039903a: mov    rcx, rax
0039903d: mov    rsi, rax
00399040: call   0x180026bd0                              ; UnityEngine.AI.NavMesh.OnNavMeshPreUpdate$$.ctor
00399045: test   rdi, rdi
00399048: je     0x180399144
0039904e: mov    rdx, qword ptr [rip + 0x395cd23]         ; [0x3cf5d78] str:'OK'
00399055: mov    rcx, rdi
00399058: mov    r9, qword ptr [rip + 0x3953661]          ; [0x3cec6c0] str:'No players were loaded. This may be caused by trying to load instruments/difficulties that do not exist in the chart file.'
0039905f: mov    r8, qword ptr [rip + 0x391b392]          ; [0x3cb43f8] str:'Error while loading CLI chart'
00399066: mov    qword ptr [rsp + 0x30], 0
0039906f: mov    qword ptr [rsp + 0x28], rdx
00399074: xor    edx, edx
00399076: mov    qword ptr [rsp + 0x20], rsi
0039907b: call   0x180363aa0                              ; UnpackingScene$$ʸʼʹʾʵʽʺʽʴʿʹ
00399080: xor    r8d, r8d
00399083: xor    edx, edx
00399085: mov    rcx, rbx
00399088: mov    rbx, qword ptr [rsp + 0x50]
0039908d: mov    rsi, qword ptr [rsp + 0x58]
00399092: add    rsp, 0x40
00399096: pop    rdi
00399097: jmp    0x182877d00                              ; UnityEngine.Behaviour$$set_enabled
0039909c: test   rsi, rsi
0039909f: je     0x180399144
003990a5: mov    eax, dword ptr [rbx + 0x78]
003990a8: lea    rcx, [rsi + 0x28]
003990ac: mov    dword ptr [rsi + 0x54], eax
003990af: mov    rdx, qword ptr [rbx + 0x40]
003990b3: mov    qword ptr [rsi + 0x28], rdx
003990b7: call   0x182f5fc00
003990bc: mov    rax, qword ptr [rip + 0x38ff25d]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
003990c3: mov    rcx, qword ptr [rax + 0xb8]
003990ca: mov    rcx, qword ptr [rcx + 0x150]
003990d1: test   rcx, rcx
003990d4: je     0x180399144
003990d6: mov    r8, qword ptr [rip + 0x395daf3]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
003990dd: xor    edx, edx
003990df: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
003990e4: movzx  eax, byte ptr [rbx + 0x58]
003990e8: mov    byte ptr [rsi + 0x73], al
003990eb: cmp    byte ptr [rbx + 0x58], 0
003990ef: je     0x1803990f5
003990f1: mov    byte ptr [rsi + 0x72], 1
003990f5: movzx  eax, byte ptr [rbx + 0x49]
003990f9: neg    al
003990fb: sbb    ecx, ecx
003990fd: and    ecx, 2
00399100: mov    dword ptr [rsi + 0x50], ecx
00399103: mov    rax, qword ptr [rip + 0x394ef9e]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
0039910a: mov    rcx, qword ptr [rax + 0xb8]
00399111: mov    rcx, qword ptr [rcx]
00399114: test   rcx, rcx
00399117: je     0x180399144
00399119: mov    rdx, qword ptr [rip + 0x393ccd8]         ; [0x3cd5df8] str:'Gameplay'
00399120: xor    r8d, r8d
00399123: call   0x1802583f0                              ; FadeBehaviour$$ʻʲʿʴʺʳˁʷʶʵʾ
00399128: xor    r8d, r8d
0039912b: xor    edx, edx
0039912d: mov    rcx, rbx
00399130: mov    rbx, qword ptr [rsp + 0x50]
00399135: mov    rsi, qword ptr [rsp + 0x58]
0039913a: add    rsp, 0x40
0039913e: pop    rdi
0039913f: jmp    0x182877d00                              ; UnityEngine.Behaviour$$set_enabled
00399144: call   0x182f60c50
00399149: int3   
0039914a: int3   
