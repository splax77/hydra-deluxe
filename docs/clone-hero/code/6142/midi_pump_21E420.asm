0021e420: mov    rax, rsp
0021e423: mov    qword ptr [rax + 8], rcx
0021e427: push   rbx
0021e428: push   rsi
0021e429: push   rdi
0021e42a: push   r12
0021e42c: push   r13
0021e42e: push   r14
0021e430: push   r15
0021e432: sub    rsp, 0x110
0021e439: movaps xmmword ptr [rax - 0x48], xmm6
0021e43d: movaps xmmword ptr [rax - 0x58], xmm7
0021e441: movaps xmmword ptr [rax - 0x68], xmm8
0021e446: movaps xmmword ptr [rax - 0x78], xmm9
0021e44b: movaps xmmword ptr [rax - 0x88], xmm10
0021e453: movaps xmmword ptr [rax - 0x98], xmm11
0021e45b: mov    rsi, rcx
0021e45e: cmp    byte ptr [rip + 0x3ceea14], 0            ; [0x3f0ce79] (bss)
0021e465: jne    0x18021e5b6
0021e46b: lea    rcx, [rip + 0x3aa6476]                   ; [0x3cc48e8] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.Dispose()
0021e472: call   0x182f609b0
0021e477: lea    rcx, [rip + 0x3aa652a]                   ; [0x3cc49a8] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.MoveNext()
0021e47e: call   0x182f609b0
0021e483: lea    rcx, [rip + 0x3aa65de]                   ; [0x3cc4a68] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.get_Current()
0021e48a: call   0x182f609b0
0021e48f: lea    rcx, [rip + 0x3ac50fa]                   ; [0x3ce3590] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.Clear()
0021e496: call   0x182f609b0
0021e49b: lea    rcx, [rip + 0x3ac51ae]                   ; [0x3ce3650] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.GetEnumerator()
0021e4a2: call   0x182f609b0
0021e4a7: lea    rcx, [rip + 0x3ac59e2]                   ; [0x3ce3e90] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Count()
0021e4ae: call   0x182f609b0
0021e4b3: lea    rcx, [rip + 0x3ac5a96]                   ; [0x3ce3f50] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Item()
0021e4ba: call   0x182f609b0
0021e4bf: lea    rcx, [rip + 0x3ae601a]                   ; [0x3d044e0] str:'Cymbal2'
0021e4c6: call   0x182f609b0
0021e4cb: lea    rcx, [rip + 0x3aa18b6]                   ; [0x3cbfd88] str:'Blue Cymbal'
0021e4d2: call   0x182f609b0
0021e4d7: lea    rcx, [rip + 0x3ab0942]                   ; [0x3ccee20] str:'Select'
0021e4de: call   0x182f609b0
0021e4e3: lea    rcx, [rip + 0x3ae60b6]                   ; [0x3d045a0] str:'Cymbal3'
0021e4ea: call   0x182f609b0
0021e4ef: lea    rcx, [rip + 0x3ab0c2a]                   ; [0x3ccf120] str:'SelectPad'
0021e4f6: call   0x182f609b0
0021e4fb: lea    rcx, [rip + 0x3a9c156]                   ; [0x3cba658] str:'Kick'
0021e502: call   0x182f609b0
0021e507: lea    rcx, [rip + 0x3ada14a]                   ; [0x3cf8658] str:'Yellow Cymbal'
0021e50e: call   0x182f609b0
0021e513: lea    rcx, [rip + 0x3ac0116]                   ; [0x3cde630] str:'Snare'
0021e51a: call   0x182f609b0
0021e51f: lea    rcx, [rip + 0x3a9c6f2]                   ; [0x3cbac18] str:'Kick Pad'
0021e526: call   0x182f609b0
0021e52b: lea    rcx, [rip + 0x3a9ab5e]                   ; [0x3cb9090] str:'Tom3'
0021e532: call   0x182f609b0
0021e537: lea    rcx, [rip + 0x3acf9c2]                   ; [0x3cedf00] str:'Start'
0021e53e: call   0x182f609b0
0021e543: lea    rcx, [rip + 0x3ada3fe]                   ; [0x3cf8948] str:'Yellow Pad'
0021e54a: call   0x182f609b0
0021e54f: lea    rcx, [rip + 0x3aa1b32]                   ; [0x3cc0088] str:'Blue Pad'
0021e556: call   0x182f609b0
0021e55b: lea    rcx, [rip + 0x3ad01de]                   ; [0x3cee740] str:'StartPad'
0021e562: call   0x182f609b0
0021e567: lea    rcx, [rip + 0x3abd642]                   ; [0x3cdbbb0] str:'Green Pad'
0021e56e: call   0x182f609b0
0021e573: lea    rcx, [rip + 0x3a9a9a6]                   ; [0x3cb8f20] str:'Tom1'
0021e57a: call   0x182f609b0
0021e57f: lea    rcx, [rip + 0x3ae5ea2]                   ; [0x3d04428] str:'Cymbal1'
0021e586: call   0x182f609b0
0021e58b: lea    rcx, [rip + 0x3abd31e]                   ; [0x3cdb8b0] str:'Green Cymbal'
0021e592: call   0x182f609b0
0021e597: lea    rcx, [rip + 0x3a9aa3a]                   ; [0x3cb8fd8] str:'Tom2'
0021e59e: call   0x182f609b0
0021e5a3: lea    rcx, [rip + 0x3a9648e]                   ; [0x3cb4a38] str:'Red Pad'
0021e5aa: call   0x182f609b0
0021e5af: mov    byte ptr [rip + 0x3cee8c3], 1            ; [0x3f0ce79] (bss)
0021e5b6: xorps  xmm0, xmm0
0021e5b9: xor    eax, eax
0021e5bb: movups xmmword ptr [rsp + 0x60], xmm0
0021e5c0: movups xmmword ptr [rsp + 0x70], xmm0
0021e5c5: mov    qword ptr [rsp + 0x80], rax
0021e5cd: xor    edx, edx
0021e5cf: mov    rcx, rsi
0021e5d2: call   0x180220660                              ; MIDIInputManager$$ʾˀʾʿʾʸʶʵʽʿʵ
0021e5d7: xor    r14d, r14d
0021e5da: mov    r15d, r14d
0021e5dd: mov    dword ptr [rsp + 0x168], r14d
0021e5e5: movss  xmm11, dword ptr [rip + 0x2e4688a]       ; [0x3064e78] flt=127.0 i=1123942400
0021e5ee: movss  xmm10, dword ptr [rip + 0x2e45dbd]       ; [0x30643b4] flt=0.5 i=1056964608
0021e5f7: movss  xmm9, dword ptr [rip + 0x2e457bc]        ; [0x3063dbc] flt=1.0 i=1065353216
0021e600: mov    rax, qword ptr [rsi + 0x38]
0021e604: test   rax, rax
0021e607: je     0x18021ec40
0021e60d: cmp    r15d, dword ptr [rax + 0x18]
0021e611: jge    0x18021ebd1
0021e617: mov    r8, qword ptr [rip + 0x3ac5932]          ; [0x3ce3f50] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ˀʷˁʻʵʹʹʺʲʹʲ>.get_Item()
0021e61e: mov    edx, r15d
0021e621: mov    rcx, rax
0021e624: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0021e629: mov    rdi, rax
0021e62c: mov    qword ptr [rsp + 0x98], rax
0021e634: test   rax, rax
0021e637: je     0x18021ec40
0021e63d: cmp    byte ptr [rax + 0x48], 0
0021e641: je     0x18021ebc1
0021e647: xor    r8d, r8d
0021e64a: mov    rdx, rax
0021e64d: mov    rcx, rsi
0021e650: call   0x18021e100                              ; MIDIInputManager$$ʶʹʷʳʹʴʴʶʺʹʶ
0021e655: mov    rdx, qword ptr [rdi + 0x38]
0021e659: test   rdx, rdx
0021e65c: je     0x18021ec40
0021e662: mov    r8, qword ptr [rip + 0x3ac4fe7]          ; [0x3ce3650] metamethod:Method$System.Collections.Generic.List<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.GetEnumerator()
0021e669: lea    rcx, [rsp + 0x30]
0021e66e: call   0x1806fe7e0                              ; System.Collections.Generic.List<ʿʻʾʿʳʷʿʷʼʽʲ.ʽʽʾˀʼʲʿʴʶˀʺ>$$GetEnumerator
0021e673: movups xmm0, xmmword ptr [rsp + 0x30]
0021e678: movups xmmword ptr [rsp + 0x60], xmm0
0021e67d: movups xmm1, xmmword ptr [rsp + 0x40]
0021e682: movups xmmword ptr [rsp + 0x70], xmm1
0021e687: movsd  xmm0, qword ptr [rsp + 0x50]
0021e68d: movsd  qword ptr [rsp + 0x80], xmm0
0021e696: mov    qword ptr [rsp + 0x88], r14
0021e69e: lea    rbx, [rsp + 0x60]
0021e6a3: mov    qword ptr [rsp + 0x90], rbx
0021e6ab: nop    dword ptr [rax + rax]
0021e6b0: mov    rdx, qword ptr [rip + 0x3aa62f1]         ; [0x3cc49a8] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.MoveNext()
0021e6b7: lea    rcx, [rsp + 0x60]
0021e6bc: call   0x1814ec710                              ; System.Collections.Generic.List.Enumerator<VisualTreeAsset.UsingEntry>$$MoveNext
0021e6c1: test   al, al
0021e6c3: je     0x18021eb2a
0021e6c9: movsd  xmm7, qword ptr [rsp + 0x70]
0021e6cf: movzx  r12d, byte ptr [rsp + 0x78]
0021e6d5: mov    r15, qword ptr [rsp + 0x78]
0021e6da: shr    r15, 8
0021e6de: movsd  xmm8, qword ptr [rsp + 0x7a]
0021e6e5: mov    r13d, dword ptr [rsp + 0x82]
0021e6ed: movzx  ecx, word ptr [rsp + 0x86]
0021e6f5: mov    word ptr [rsp + 0x160], cx
0021e6fd: movzx  eax, r15b
0021e701: movd   xmm6, eax
0021e705: cvtdq2ps xmm6, xmm6
0021e708: divss  xmm6, xmm11
0021e70d: mulss  xmm6, xmm10
0021e712: addss  xmm6, xmm10
0021e717: movsd  qword ptr [rsp + 0x30], xmm7
0021e71d: mov    byte ptr [rsp + 0x38], r12b
0021e722: mov    byte ptr [rsp + 0x39], r15b
0021e727: movsd  qword ptr [rsp + 0x3a], xmm8
0021e72e: mov    dword ptr [rsp + 0x42], r13d
0021e733: mov    word ptr [rsp + 0x46], cx
0021e738: mov    qword ptr [rsp + 0x20], r14
0021e73d: lea    r9, [rsp + 0x30]
0021e742: mov    r8, qword ptr [rip + 0x3a962ef]          ; [0x3cb4a38] str:'Red Pad'
0021e749: mov    rdx, rdi
0021e74c: mov    rcx, rsi
0021e74f: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e754: test   al, al
0021e756: je     0x18021e777
0021e758: mov    rcx, qword ptr [rdi + 0x30]
0021e75c: test   rcx, rcx
0021e75f: je     0x18021ec06
0021e765: xor    r9d, r9d
0021e768: movaps xmm2, xmm6
0021e76b: mov    rdx, qword ptr [rip + 0x3abfebe]         ; [0x3cde630] str:'Snare'
0021e772: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e777: movsd  qword ptr [rsp + 0x30], xmm7
0021e77d: mov    byte ptr [rsp + 0x38], r12b
0021e782: mov    byte ptr [rsp + 0x39], r15b
0021e787: movsd  qword ptr [rsp + 0x3a], xmm8
0021e78e: mov    dword ptr [rsp + 0x42], r13d
0021e793: movzx  eax, word ptr [rsp + 0x160]
0021e79b: mov    word ptr [rsp + 0x46], ax
0021e7a0: mov    qword ptr [rsp + 0x20], r14
0021e7a5: lea    r9, [rsp + 0x30]
0021e7aa: mov    r8, qword ptr [rip + 0x3ada197]          ; [0x3cf8948] str:'Yellow Pad'
0021e7b1: mov    rdx, rdi
0021e7b4: mov    rcx, rsi
0021e7b7: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e7bc: test   al, al
0021e7be: je     0x18021e7df
0021e7c0: mov    rcx, qword ptr [rdi + 0x30]
0021e7c4: test   rcx, rcx
0021e7c7: je     0x18021ec0c
0021e7cd: xor    r9d, r9d
0021e7d0: movaps xmm2, xmm6
0021e7d3: mov    rdx, qword ptr [rip + 0x3a9a746]         ; [0x3cb8f20] str:'Tom1'
0021e7da: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e7df: movsd  qword ptr [rsp + 0x30], xmm7
0021e7e5: mov    byte ptr [rsp + 0x38], r12b
0021e7ea: mov    byte ptr [rsp + 0x39], r15b
0021e7ef: movsd  qword ptr [rsp + 0x3a], xmm8
0021e7f6: mov    dword ptr [rsp + 0x42], r13d
0021e7fb: movzx  eax, word ptr [rsp + 0x160]
0021e803: mov    word ptr [rsp + 0x46], ax
0021e808: mov    qword ptr [rsp + 0x20], r14
0021e80d: lea    r9, [rsp + 0x30]
0021e812: mov    r8, qword ptr [rip + 0x3aa186f]          ; [0x3cc0088] str:'Blue Pad'
0021e819: mov    rdx, rdi
0021e81c: mov    rcx, rsi
0021e81f: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e824: test   al, al
0021e826: je     0x18021e847
0021e828: mov    rcx, qword ptr [rdi + 0x30]
0021e82c: test   rcx, rcx
0021e82f: je     0x18021ec11
0021e835: xor    r9d, r9d
0021e838: movaps xmm2, xmm6
0021e83b: mov    rdx, qword ptr [rip + 0x3a9a796]         ; [0x3cb8fd8] str:'Tom2'
0021e842: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e847: movsd  qword ptr [rsp + 0x30], xmm7
0021e84d: mov    byte ptr [rsp + 0x38], r12b
0021e852: mov    byte ptr [rsp + 0x39], r15b
0021e857: movsd  qword ptr [rsp + 0x3a], xmm8
0021e85e: mov    dword ptr [rsp + 0x42], r13d
0021e863: movzx  eax, word ptr [rsp + 0x160]
0021e86b: mov    word ptr [rsp + 0x46], ax
0021e870: mov    qword ptr [rsp + 0x20], r14
0021e875: lea    r9, [rsp + 0x30]
0021e87a: mov    r8, qword ptr [rip + 0x3abd32f]          ; [0x3cdbbb0] str:'Green Pad'
0021e881: mov    rdx, rdi
0021e884: mov    rcx, rsi
0021e887: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e88c: test   al, al
0021e88e: je     0x18021e8af
0021e890: mov    rcx, qword ptr [rdi + 0x30]
0021e894: test   rcx, rcx
0021e897: je     0x18021ec16
0021e89d: xor    r9d, r9d
0021e8a0: movaps xmm2, xmm6
0021e8a3: mov    rdx, qword ptr [rip + 0x3a9a7e6]         ; [0x3cb9090] str:'Tom3'
0021e8aa: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e8af: movsd  qword ptr [rsp + 0x30], xmm7
0021e8b5: mov    byte ptr [rsp + 0x38], r12b
0021e8ba: mov    byte ptr [rsp + 0x39], r15b
0021e8bf: movsd  qword ptr [rsp + 0x3a], xmm8
0021e8c6: mov    dword ptr [rsp + 0x42], r13d
0021e8cb: movzx  eax, word ptr [rsp + 0x160]
0021e8d3: mov    word ptr [rsp + 0x46], ax
0021e8d8: mov    qword ptr [rsp + 0x20], r14
0021e8dd: lea    r9, [rsp + 0x30]
0021e8e2: mov    r8, qword ptr [rip + 0x3ad9d6f]          ; [0x3cf8658] str:'Yellow Cymbal'
0021e8e9: mov    rdx, rdi
0021e8ec: mov    rcx, rsi
0021e8ef: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e8f4: test   al, al
0021e8f6: je     0x18021e917
0021e8f8: mov    rcx, qword ptr [rdi + 0x30]
0021e8fc: test   rcx, rcx
0021e8ff: je     0x18021ec1b
0021e905: xor    r9d, r9d
0021e908: movaps xmm2, xmm6
0021e90b: mov    rdx, qword ptr [rip + 0x3ae5b16]         ; [0x3d04428] str:'Cymbal1'
0021e912: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e917: movsd  qword ptr [rsp + 0x30], xmm7
0021e91d: mov    byte ptr [rsp + 0x38], r12b
0021e922: mov    byte ptr [rsp + 0x39], r15b
0021e927: movsd  qword ptr [rsp + 0x3a], xmm8
0021e92e: mov    dword ptr [rsp + 0x42], r13d
0021e933: movzx  eax, word ptr [rsp + 0x160]
0021e93b: mov    word ptr [rsp + 0x46], ax
0021e940: mov    qword ptr [rsp + 0x20], r14
0021e945: lea    r9, [rsp + 0x30]
0021e94a: mov    r8, qword ptr [rip + 0x3aa1437]          ; [0x3cbfd88] str:'Blue Cymbal'
0021e951: mov    rdx, rdi
0021e954: mov    rcx, rsi
0021e957: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e95c: test   al, al
0021e95e: je     0x18021e97f
0021e960: mov    rcx, qword ptr [rdi + 0x30]
0021e964: test   rcx, rcx
0021e967: je     0x18021ec20
0021e96d: xor    r9d, r9d
0021e970: movaps xmm2, xmm6
0021e973: mov    rdx, qword ptr [rip + 0x3ae5b66]         ; [0x3d044e0] str:'Cymbal2'
0021e97a: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e97f: movsd  qword ptr [rsp + 0x30], xmm7
0021e985: mov    byte ptr [rsp + 0x38], r12b
0021e98a: mov    byte ptr [rsp + 0x39], r15b
0021e98f: movsd  qword ptr [rsp + 0x3a], xmm8
0021e996: mov    dword ptr [rsp + 0x42], r13d
0021e99b: movzx  eax, word ptr [rsp + 0x160]
0021e9a3: mov    word ptr [rsp + 0x46], ax
0021e9a8: mov    qword ptr [rsp + 0x20], r14
0021e9ad: lea    r9, [rsp + 0x30]
0021e9b2: mov    r8, qword ptr [rip + 0x3abcef7]          ; [0x3cdb8b0] str:'Green Cymbal'
0021e9b9: mov    rdx, rdi
0021e9bc: mov    rcx, rsi
0021e9bf: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021e9c4: test   al, al
0021e9c6: je     0x18021e9e7
0021e9c8: mov    rcx, qword ptr [rdi + 0x30]
0021e9cc: test   rcx, rcx
0021e9cf: je     0x18021ec25
0021e9d5: xor    r9d, r9d
0021e9d8: movaps xmm2, xmm6
0021e9db: mov    rdx, qword ptr [rip + 0x3ae5bbe]         ; [0x3d045a0] str:'Cymbal3'
0021e9e2: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021e9e7: movsd  qword ptr [rsp + 0x30], xmm7
0021e9ed: mov    byte ptr [rsp + 0x38], r12b
0021e9f2: mov    byte ptr [rsp + 0x39], r15b
0021e9f7: movsd  qword ptr [rsp + 0x3a], xmm8
0021e9fe: mov    dword ptr [rsp + 0x42], r13d
0021ea03: movzx  eax, word ptr [rsp + 0x160]
0021ea0b: mov    word ptr [rsp + 0x46], ax
0021ea10: mov    qword ptr [rsp + 0x20], r14
0021ea15: lea    r9, [rsp + 0x30]
0021ea1a: mov    r8, qword ptr [rip + 0x3a9c1f7]          ; [0x3cbac18] str:'Kick Pad'
0021ea21: mov    rdx, rdi
0021ea24: mov    rcx, rsi
0021ea27: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021ea2c: test   al, al
0021ea2e: je     0x18021ea4f
0021ea30: mov    rcx, qword ptr [rdi + 0x30]
0021ea34: test   rcx, rcx
0021ea37: je     0x18021ec2a
0021ea3d: xor    r9d, r9d
0021ea40: movaps xmm2, xmm6
0021ea43: mov    rdx, qword ptr [rip + 0x3a9bc0e]         ; [0x3cba658] str:'Kick'
0021ea4a: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021ea4f: movsd  qword ptr [rsp + 0x30], xmm7
0021ea55: mov    byte ptr [rsp + 0x38], r12b
0021ea5a: mov    byte ptr [rsp + 0x39], r15b
0021ea5f: movsd  qword ptr [rsp + 0x3a], xmm8
0021ea66: mov    dword ptr [rsp + 0x42], r13d
0021ea6b: movzx  eax, word ptr [rsp + 0x160]
0021ea73: mov    word ptr [rsp + 0x46], ax
0021ea78: mov    qword ptr [rsp + 0x20], r14
0021ea7d: lea    r9, [rsp + 0x30]
0021ea82: mov    r8, qword ptr [rip + 0x3acf477]          ; [0x3cedf00] str:'Start'
0021ea89: mov    rdx, rdi
0021ea8c: mov    rcx, rsi
0021ea8f: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021ea94: test   al, al
0021ea96: je     0x18021eab8
0021ea98: mov    rcx, qword ptr [rdi + 0x30]
0021ea9c: test   rcx, rcx
0021ea9f: je     0x18021ec2f
0021eaa5: xor    r9d, r9d
0021eaa8: movaps xmm2, xmm9
0021eaac: mov    rdx, qword ptr [rip + 0x3acfc8d]         ; [0x3cee740] str:'StartPad'
0021eab3: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021eab8: movsd  qword ptr [rsp + 0x30], xmm7
0021eabe: mov    byte ptr [rsp + 0x38], r12b
0021eac3: mov    byte ptr [rsp + 0x39], r15b
0021eac8: movsd  qword ptr [rsp + 0x3a], xmm8
0021eacf: mov    dword ptr [rsp + 0x42], r13d
0021ead4: movzx  eax, word ptr [rsp + 0x160]
0021eadc: mov    word ptr [rsp + 0x46], ax
0021eae1: mov    qword ptr [rsp + 0x20], r14
0021eae6: lea    r9, [rsp + 0x30]
0021eaeb: mov    r8, qword ptr [rip + 0x3ab032e]          ; [0x3ccee20] str:'Select'
0021eaf2: mov    rdx, rdi
0021eaf5: mov    rcx, rsi
0021eaf8: call   0x18021f840                              ; MIDIInputManager$$ʺʷʵˁˀʴʾʿʿˀʾ
0021eafd: test   al, al
0021eaff: je     0x18021e6b0
0021eb05: mov    rcx, qword ptr [rdi + 0x30]
0021eb09: test   rcx, rcx
0021eb0c: je     0x18021ec34
0021eb12: xor    r9d, r9d
0021eb15: movaps xmm2, xmm9
0021eb19: mov    rdx, qword ptr [rip + 0x3ab0600]         ; [0x3ccf120] str:'SelectPad'
0021eb20: call   0x181d5df10                              ; Rewired.CustomController$$SetAxisValue
0021eb25: jmp    0x18021e6b0
0021eb2a: mov    rdx, qword ptr [rip + 0x3aa5db7]         ; [0x3cc48e8] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.Dispose()
0021eb31: mov    rcx, rbx
0021eb34: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0021eb39: jmp    0x18021eb8e
0021eb3b: mov    rdx, qword ptr [rip + 0x3aa5da6]         ; [0x3cc48e8] metamethod:Method$System.Collections.Generic.List.Enumerator<MIDIInputManager.ʹʴʴʿʲʺʶʷʶʽʶ>.Dispose()
0021eb42: mov    rcx, qword ptr [rsp + 0x90]
0021eb4a: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0021eb4f: mov    rcx, qword ptr [rsp + 0x88]
0021eb57: test   rcx, rcx
0021eb5a: jne    0x18021ec3a
0021eb60: xor    r14d, r14d
0021eb63: mov    rsi, qword ptr [rsp + 0x150]
0021eb6b: mov    rdi, qword ptr [rsp + 0x98]
0021eb73: movss  xmm11, dword ptr [rip + 0x2e462fc]       ; [0x3064e78] flt=127.0 i=1123942400
0021eb7c: movss  xmm10, dword ptr [rip + 0x2e4582f]       ; [0x30643b4] flt=0.5 i=1056964608
0021eb85: movss  xmm9, dword ptr [rip + 0x2e4522e]        ; [0x3063dbc] flt=1.0 i=1065353216
0021eb8e: mov    rcx, qword ptr [rdi + 0x38]
0021eb92: test   rcx, rcx
0021eb95: je     0x18021ec40
0021eb9b: inc    dword ptr [rcx + 0x1c]
0021eb9e: mov    r8d, dword ptr [rcx + 0x18]
0021eba2: mov    dword ptr [rcx + 0x18], r14d
0021eba6: test   r8d, r8d
0021eba9: jle    0x18021ebb9
0021ebab: xor    r9d, r9d
0021ebae: xor    edx, edx
0021ebb0: mov    rcx, qword ptr [rcx + 0x10]
0021ebb4: call   0x181b642b0                              ; System.Array$$Clear
0021ebb9: mov    r15d, dword ptr [rsp + 0x168]
0021ebc1: inc    r15d
0021ebc4: mov    dword ptr [rsp + 0x168], r15d
0021ebcc: jmp    0x18021e600
0021ebd1: lea    r11, [rsp + 0x110]
0021ebd9: movaps xmm6, xmmword ptr [r11 - 0x10]
0021ebde: movaps xmm7, xmmword ptr [r11 - 0x20]
0021ebe3: movaps xmm8, xmmword ptr [r11 - 0x30]
0021ebe8: movaps xmm9, xmmword ptr [r11 - 0x40]
0021ebed: movaps xmm10, xmmword ptr [r11 - 0x50]
0021ebf2: movaps xmm11, xmmword ptr [r11 - 0x60]
0021ebf7: mov    rsp, r11
0021ebfa: pop    r15
0021ebfc: pop    r14
0021ebfe: pop    r13
0021ec00: pop    r12
0021ec02: pop    rdi
0021ec03: pop    rsi
0021ec04: pop    rbx
0021ec05: ret    
0021ec06: call   0x182f60c50
0021ec0b: nop    
0021ec0c: call   0x182f60c50
0021ec11: call   0x182f60c50
0021ec16: call   0x182f60c50
0021ec1b: call   0x182f60c50
0021ec20: call   0x182f60c50
0021ec25: call   0x182f60c50
0021ec2a: call   0x182f60c50
0021ec2f: call   0x182f60c50
0021ec34: call   0x182f60c50
0021ec39: nop    
0021ec3a: call   0x182f60ce0
0021ec3f: int3   
0021ec40: call   0x182f60c50
0021ec45: int3   
0021ec46: int3   
