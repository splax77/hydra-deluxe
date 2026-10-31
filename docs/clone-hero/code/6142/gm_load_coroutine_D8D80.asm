; function 0xd8d80 GameManager.ʻʵˀʸʷˀʿʸʳʲʲ$$MoveNext next method starts at 0xd99e0 bytes 0xc60
000d8d80: mov    rax, rsp
000d8d83: mov    qword ptr [rax + 0x10], rbx
000d8d87: mov    qword ptr [rax + 0x20], rsi
000d8d8b: push   rdi
000d8d8c: push   r12
000d8d8e: push   r13
000d8d90: push   r14
000d8d92: push   r15
000d8d94: sub    rsp, 0xa0
000d8d9b: movaps xmmword ptr [rax - 0x38], xmm6
000d8d9f: movaps xmmword ptr [rax - 0x48], xmm7
000d8da3: movaps xmmword ptr [rax - 0x58], xmm8
000d8da8: movaps xmmword ptr [rax - 0x68], xmm9
000d8dad: movaps xmmword ptr [rax - 0x78], xmm10
000d8db2: mov    rsi, rcx
000d8db5: cmp    byte ptr [rip + 0x3e33401], 0            ; [0x3f0c1bd] (bss)
000d8dbc: jne    0x1800d8e95
000d8dc2: lea    rcx, [rip + 0x3be308f]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d8dc9: call   0x182f609b0
000d8dce: lea    rcx, [rip + 0x3be624b]                   ; [0x3cbf020] meta:bool_TypeInfo
000d8dd5: call   0x182f609b0
000d8dda: lea    rcx, [rip + 0x3beaa37]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d8de1: call   0x182f609b0
000d8de6: lea    rcx, [rip + 0x3bfebfb]                   ; [0x3cd79e8] metamethod:Method$UnityEngine.Component.GetComponent<ScoreManager>()
000d8ded: call   0x182f609b0
000d8df2: lea    rcx, [rip + 0x3bff557]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d8df9: call   0x182f609b0
000d8dfe: lea    rcx, [rip + 0x3c16a8b]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d8e05: call   0x182f609b0
000d8e0a: lea    rcx, [rip + 0x3bc9957]                   ; [0x3ca2768] meta:System.Math_TypeInfo
000d8e11: call   0x182f609b0
000d8e16: lea    rcx, [rip + 0x3c3622b]                   ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d8e1d: call   0x182f609b0
000d8e22: lea    rcx, [rip + 0x3bb95f7]                   ; [0x3c92420] meta:ʴʶʺˀˀʺʹʴʿʹʷ_TypeInfo
000d8e29: call   0x182f609b0
000d8e2e: lea    rcx, [rip + 0x3c1e08b]                   ; [0x3cf6ec0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ˁʾʽʿʻʺʽʲʼʾˀ()
000d8e35: call   0x182f609b0
000d8e3a: lea    rcx, [rip + 0x3bbf4df]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d8e41: call   0x182f609b0
000d8e46: lea    rcx, [rip + 0x3bc0a13]                   ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000d8e4d: call   0x182f609b0
000d8e52: lea    rcx, [rip + 0x3bc2137]                   ; [0x3c9af90] meta:ʼʸʺʳʽʿʽʾʽˀʼ_TypeInfo
000d8e59: call   0x182f609b0
000d8e5e: lea    rcx, [rip + 0x3beb623]                   ; [0x3cc4488] str:'Loading Song: '
000d8e65: call   0x182f609b0
000d8e6a: lea    rcx, [rip + 0x3c08b7f]                   ; [0x3ce19f0] str:'Song length is 0! Is responsible for brokenness: {0}'
000d8e71: call   0x182f609b0
000d8e76: lea    rcx, [rip + 0x3be2f83]                   ; [0x3cbbe00] str:' - '
000d8e7d: call   0x182f609b0
000d8e82: lea    rcx, [rip + 0x3bea5f7]                   ; [0x3cc3480] str:'Failed to load song'
000d8e89: call   0x182f609b0
000d8e8e: mov    byte ptr [rip + 0x3e33328], 1            ; [0x3f0c1bd] (bss)
000d8e95: mov    rdi, qword ptr [rsi + 0x20]
000d8e99: mov    qword ptr [rsp + 0xd0], rdi
000d8ea1: mov    ecx, dword ptr [rsi + 0x10]
000d8ea4: test   ecx, ecx
000d8ea6: je     0x1800d8ef4
000d8ea8: sub    ecx, 1
000d8eab: je     0x1800d8edf
000d8ead: sub    ecx, 1
000d8eb0: je     0x1800d8eca
000d8eb2: cmp    ecx, 1
000d8eb5: jne    0x1800d9947
000d8ebb: mov    dword ptr [rsi + 0x10], 0xffffffff
000d8ec2: xor    r14d, r14d
000d8ec5: jmp    0x1800d9882
000d8eca: mov    dword ptr [rsi + 0x10], 0xffffffff
000d8ed1: xor    r14d, r14d
000d8ed4: mov    r15d, 1
000d8eda: jmp    0x1800d96c1
000d8edf: mov    dword ptr [rsi + 0x10], 0xffffffff
000d8ee6: xor    r14d, r14d
000d8ee9: mov    r15d, 1
000d8eef: jmp    0x1800d91be
000d8ef4: mov    dword ptr [rsi + 0x10], 0xffffffff
000d8efb: test   rdi, rdi
000d8efe: je     0x1800d99c8
000d8f04: cmp    qword ptr [rdi + 0xa8], 0
000d8f0c: jne    0x1800d9947
000d8f12: mov    rax, qword ptr [rip + 0x3c16977]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d8f19: cmp    dword ptr [rax + 0xe0], 0
000d8f20: jne    0x1800d8f31
000d8f22: mov    rcx, rax
000d8f25: call   0x182f60cf0
000d8f2a: mov    rax, qword ptr [rip + 0x3c1695f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d8f31: mov    rax, qword ptr [rax + 0xb8]
000d8f38: mov    rdx, qword ptr [rax + 8]
000d8f3c: test   rdx, rdx
000d8f3f: je     0x1800d99c8
000d8f45: mov    rdx, qword ptr [rdx + 0x28]
000d8f49: mov    qword ptr [rsi + 0x28], rdx
000d8f4d: lea    rcx, [rsi + 0x28]
000d8f51: call   0x182f5fc00
000d8f56: mov    rax, qword ptr [rip + 0x3bea8bb]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d8f5d: mov    rcx, qword ptr [rax + 0xb8]
000d8f64: mov    rcx, qword ptr [rcx]
000d8f67: test   rcx, rcx
000d8f6a: je     0x1800d99c8
000d8f70: xor    edx, edx
000d8f72: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000d8f77: test   al, al
000d8f79: jne    0x1800d8f7f
000d8f7b: mov    al, 1
000d8f7d: jmp    0x1800d8fa0
000d8f7f: mov    rax, qword ptr [rip + 0x3bea892]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d8f86: mov    rcx, qword ptr [rax + 0xb8]
000d8f8d: mov    rcx, qword ptr [rcx]
000d8f90: test   rcx, rcx
000d8f93: je     0x1800d99c8
000d8f99: xor    edx, edx
000d8f9b: call   0x180146a90                              ; CHNetManager$$ʺˁʸʿʺʸʳʹʼʵʸ
000d8fa0: mov    byte ptr [rsi + 0x30], al
000d8fa3: mov    rcx, qword ptr [rsi + 0x28]
000d8fa7: test   rcx, rcx
000d8faa: je     0x1800d99c8
000d8fb0: xor    edx, edx
000d8fb2: call   0x1803600c0                              ; SongEntry$$get_Artist
000d8fb7: test   rax, rax
000d8fba: je     0x1800d99c8
000d8fc0: mov    rcx, qword ptr [rsi + 0x28]
000d8fc4: test   rcx, rcx
000d8fc7: je     0x1800d99c8
000d8fcd: mov    rbx, qword ptr [rax + 0x10]
000d8fd1: xor    edx, edx
000d8fd3: call   0x18035a5f0                              ; SongEntry$$ʷˁʼˁʴʸʾʻʿʿʿ
000d8fd8: test   rax, rax
000d8fdb: je     0x1800d99c8
000d8fe1: xor    r14d, r14d
000d8fe4: mov    qword ptr [rsp + 0x20], r14
000d8fe9: mov    r9, qword ptr [rax + 0x10]
000d8fed: mov    r8, qword ptr [rip + 0x3be2e0c]          ; [0x3cbbe00] str:' - '
000d8ff4: mov    rdx, rbx
000d8ff7: mov    rcx, qword ptr [rip + 0x3beb48a]         ; [0x3cc4488] str:'Loading Song: '
000d8ffe: call   0x18197e8f0                              ; System.String$$Concat
000d9003: mov    rbx, rax
000d9006: mov    rcx, qword ptr [rip + 0x3bff343]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d900d: cmp    dword ptr [rcx + 0xe0], r14d
000d9014: jne    0x1800d901b
000d9016: call   0x182f60cf0
000d901b: xor    edx, edx
000d901d: mov    rcx, rbx
000d9020: call   0x18285fd00                              ; UnityEngine.Debug$$Log
000d9025: mov    byte ptr [rsi + 0x31], r14b
000d9029: mov    rax, qword ptr [rip + 0x3c16860]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9030: cmp    dword ptr [rax + 0xe0], r14d
000d9037: jne    0x1800d9048
000d9039: mov    rcx, rax
000d903c: call   0x182f60cf0
000d9041: mov    rax, qword ptr [rip + 0x3c16848]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9048: mov    rax, qword ptr [rax + 0xb8]
000d904f: mov    rcx, qword ptr [rax + 8]
000d9053: test   rcx, rcx
000d9056: je     0x1800d99c8
000d905c: mov    byte ptr [rcx + 0x70], r14b
000d9060: mov    rax, qword ptr [rip + 0x3bbf2b9]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d9067: mov    rcx, qword ptr [rax + 0xb8]
000d906e: mov    rcx, qword ptr [rcx + 0x10]
000d9072: test   rcx, rcx
000d9075: je     0x1800d99c8
000d907b: mov    rdx, qword ptr [rip + 0x3c1de3e]         ; [0x3cf6ec0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ˁʾʽʿʻʺʽʲʼʾˀ()
000d9082: call   0x180fe6590                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ˁʾʽʿʻʺʽʲʼʾˀ
000d9087: test   al, al
000d9089: jne    0x1800d9096
000d908b: mov    r15d, 1
000d9091: mov    eax, r15d
000d9094: jmp    0x1800d90d0
000d9096: mov    rcx, qword ptr [rip + 0x3c167f3]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d909d: cmp    dword ptr [rcx + 0xe0], r14d
000d90a4: jne    0x1800d90ab
000d90a6: call   0x182f60cf0
000d90ab: mov    rax, qword ptr [rip + 0x3c167de]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d90b2: mov    rcx, qword ptr [rax + 0xb8]
000d90b9: mov    rax, qword ptr [rcx + 8]
000d90bd: test   rax, rax
000d90c0: je     0x1800d99c8
000d90c6: movzx  eax, byte ptr [rax + 0x72]
000d90ca: mov    r15d, 1
000d90d0: test   eax, eax
000d90d2: setne  r13b
000d90d6: mov    rax, qword ptr [rip + 0x3be2d7b]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d90dd: cmp    dword ptr [rax + 0xe0], r14d
000d90e4: jne    0x1800d90f5
000d90e6: mov    rcx, rax
000d90e9: call   0x182f60cf0
000d90ee: mov    rax, qword ptr [rip + 0x3be2d63]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d90f5: mov    rax, qword ptr [rax + 0xb8]
000d90fc: mov    rbx, qword ptr [rax + 8]
000d9100: mov    r12, qword ptr [rsi + 0x28]
000d9104: mov    rcx, qword ptr [rip + 0x3c16785]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d910b: cmp    dword ptr [rcx + 0xe0], r14d
000d9112: jne    0x1800d9119
000d9114: call   0x182f60cf0
000d9119: mov    rax, qword ptr [rip + 0x3c16770]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9120: mov    rcx, qword ptr [rax + 0xb8]
000d9127: mov    rax, qword ptr [rcx + 8]
000d912b: test   rax, rax
000d912e: je     0x1800d99c8
000d9134: cmp    byte ptr [rax + 0x72], r14b
000d9138: mov    r9b, 0xc
000d913b: jne    0x1800d9140
000d913d: mov    r9b, 0xf
000d9140: test   rbx, rbx
000d9143: je     0x1800d99c8
000d9149: mov    qword ptr [rsp + 0x28], r14
000d914e: mov    byte ptr [rsp + 0x20], 0xf
000d9153: movzx  r8d, r13b
000d9157: mov    rdx, r12
000d915a: mov    rcx, rbx
000d915d: call   0x18036b700                              ; BassAudioManager$$ʾʳʸʴʻʳʾʴʺˀʺ
000d9162: cmp    byte ptr [rsi + 0x30], r14b
000d9166: jne    0x1800d9253
000d916c: mov    rax, qword ptr [rip + 0x3bea6a5]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d9173: mov    rcx, qword ptr [rax + 0xb8]
000d917a: mov    rcx, qword ptr [rcx]
000d917d: test   rcx, rcx
000d9180: je     0x1800d99c8
000d9186: xor    edx, edx
000d9188: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000d918d: test   al, al
000d918f: je     0x1800d92a0
000d9195: mov    rax, qword ptr [rip + 0x3bea67c]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d919c: mov    rcx, qword ptr [rax + 0xb8]
000d91a3: mov    rcx, qword ptr [rcx]
000d91a6: test   rcx, rcx
000d91a9: je     0x1800d99c8
000d91af: xor    edx, edx
000d91b1: call   0x180146a90                              ; CHNetManager$$ʺˁʸʿʺʸʳʹʼʵʸ
000d91b6: test   al, al
000d91b8: jne    0x1800d92a0
000d91be: mov    rax, qword ptr [rip + 0x3c166cb]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d91c5: cmp    dword ptr [rax + 0xe0], 0
000d91cc: jne    0x1800d91dd
000d91ce: mov    rcx, rax
000d91d1: call   0x182f60cf0
000d91d6: mov    rax, qword ptr [rip + 0x3c166b3]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d91dd: mov    rax, qword ptr [rax + 0xb8]
000d91e4: mov    rax, qword ptr [rax + 8]
000d91e8: test   rax, rax
000d91eb: je     0x1800d99c8
000d91f1: mov    rax, qword ptr [rax + 0x30]
000d91f5: test   rax, rax
000d91f8: je     0x1800d99c8
000d91fe: cmp    byte ptr [rax + 0x50], 0
000d9202: jne    0x1800d92a0
000d9208: mov    qword ptr [rsi + 0x18], r14
000d920c: lea    rcx, [rsi + 0x18]
000d9210: xor    edx, edx
000d9212: call   0x182f5fc00
000d9217: mov    dword ptr [rsi + 0x10], r15d
000d921b: mov    al, 1
000d921d: lea    r11, [rsp + 0xa0]
000d9225: mov    rbx, qword ptr [r11 + 0x38]
000d9229: mov    rsi, qword ptr [r11 + 0x48]
000d922d: movaps xmm6, xmmword ptr [r11 - 0x10]
000d9232: movaps xmm7, xmmword ptr [r11 - 0x20]
000d9237: movaps xmm8, xmmword ptr [r11 - 0x30]
000d923c: movaps xmm9, xmmword ptr [r11 - 0x40]
000d9241: movaps xmm10, xmmword ptr [r11 - 0x50]
000d9246: mov    rsp, r11
000d9249: pop    r15
000d924b: pop    r14
000d924d: pop    r13
000d924f: pop    r12
000d9251: pop    rdi
000d9252: ret    
000d9253: mov    rax, qword ptr [rip + 0x3c16636]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d925a: cmp    dword ptr [rax + 0xe0], 0
000d9261: jne    0x1800d9272
000d9263: mov    rcx, rax
000d9266: call   0x182f60cf0
000d926b: mov    rax, qword ptr [rip + 0x3c1661e]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9272: mov    rax, qword ptr [rax + 0xb8]
000d9279: mov    rcx, qword ptr [rax + 8]
000d927d: test   rcx, rcx
000d9280: je     0x1800d99d3
000d9286: mov    rcx, qword ptr [rcx + 0x30]
000d928a: test   rcx, rcx
000d928d: je     0x1800d99ce
000d9293: xor    r8d, r8d
000d9296: mov    rdx, qword ptr [rsi + 0x28]
000d929a: call   0x1801f0160                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʻʸʳʶˁʷʵʴʻʾʹ
000d929f: nop    
000d92a0: mov    rcx, qword ptr [rsi + 0x28]
000d92a4: test   rcx, rcx
000d92a7: je     0x1800d99c8
000d92ad: mov    qword ptr [rsp + 0x20], r14
000d92b2: mov    r9b, 1
000d92b5: xor    r8d, r8d
000d92b8: xor    edx, edx
000d92ba: call   0x18035c110                              ; SongEntry$$ʿʲʷʹʿʳʾʳʷˀʾ
000d92bf: test   rdi, rdi
000d92c2: je     0x1800d99c8
000d92c8: mov    qword ptr [rdi + 0xa8], rax
000d92cf: lea    rcx, [rdi + 0xa8]
000d92d6: mov    rdx, rax
000d92d9: call   0x182f5fc00
000d92de: cmp    qword ptr [rdi + 0xa8], 0
000d92e6: je     0x1800d991d
000d92ec: cmp    byte ptr [rsi + 0x30], 0
000d92f0: jne    0x1800d9317
000d92f2: mov    rax, qword ptr [rdi + 0x28]
000d92f6: test   rax, rax
000d92f9: je     0x1800d99c8
000d92ff: mov    rcx, qword ptr [rax + 0x30]
000d9303: test   rcx, rcx
000d9306: je     0x1800d99c8
000d930c: movsd  xmm9, qword ptr [rcx + 0x30]
000d9312: jmp    0x1800d94da
000d9317: mov    rax, qword ptr [rip + 0x3bbf002]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d931e: mov    rcx, qword ptr [rax + 0xb8]
000d9325: mov    rcx, qword ptr [rcx + 0x10]
000d9329: test   rcx, rcx
000d932c: je     0x1800d99c8
000d9332: xor    edx, edx
000d9334: call   0x18210c090                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʸʺˀʳˁʽʸʺʲʻˁ
000d9339: movaps xmm7, xmm0
000d933c: mov    rcx, qword ptr [rdi + 0xa8]
000d9343: test   rcx, rcx
000d9346: je     0x1800d99c8
000d934c: mov    rbx, rcx
000d934f: xor    r8d, r8d
000d9352: xor    edx, edx
000d9354: call   0x18212e130                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʿʼʿʿʵʵʲʿʷˁʻ
000d9359: mov    rdx, rax
000d935c: xor    r8d, r8d
000d935f: mov    rcx, rbx
000d9362: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
000d9367: movaps xmm8, xmm0
000d936b: mov    rax, qword ptr [rip + 0x3be2ae6]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d9372: cmp    dword ptr [rax + 0xe0], 0
000d9379: jne    0x1800d938a
000d937b: mov    rcx, rax
000d937e: call   0x182f60cf0
000d9383: mov    rax, qword ptr [rip + 0x3be2ace]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000d938a: mov    rax, qword ptr [rax + 0xb8]
000d9391: mov    rcx, qword ptr [rax + 8]
000d9395: test   rcx, rcx
000d9398: je     0x1800d99c8
000d939e: xorps  xmm6, xmm6
000d93a1: cvtss2sd xmm6, xmm7
000d93a5: mulsd  xmm6, xmm8
000d93aa: movsd  xmm8, qword ptr [rcx + 0x40]
000d93b0: mov    rcx, qword ptr [rip + 0x3bc93b1]         ; [0x3ca2768] meta:System.Math_TypeInfo
000d93b7: cmp    dword ptr [rcx + 0xe0], 0
000d93be: jne    0x1800d93c5
000d93c0: call   0x182f60cf0
000d93c5: xor    r8d, r8d
000d93c8: movaps xmm1, xmm6
000d93cb: movaps xmm0, xmm8
000d93cf: call   0x181b51dc0                              ; System.Math$$Max
000d93d4: movaps xmm9, xmm0
000d93d8: mov    rcx, qword ptr [rdi + 0xa8]
000d93df: test   rcx, rcx
000d93e2: je     0x1800d99c8
000d93e8: xor    edx, edx
000d93ea: call   0x18212c7e0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʶˁʻʷʼʷʶʻʳʳʿ
000d93ef: test   al, al
000d93f1: je     0x1800d9455
000d93f3: mov    rax, qword ptr [rdi + 0xa8]
000d93fa: test   rax, rax
000d93fd: je     0x1800d99c8
000d9403: mov    rdx, qword ptr [rax + 0x108]
000d940a: test   rdx, rdx
000d940d: je     0x1800d99c8
000d9413: xor    r8d, r8d
000d9416: mov    rdx, qword ptr [rdx + 0x10]
000d941a: mov    rcx, rax
000d941d: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
000d9422: movaps xmm6, xmm0
000d9425: mov    rcx, qword ptr [rip + 0x3bc933c]         ; [0x3ca2768] meta:System.Math_TypeInfo
000d942c: cmp    dword ptr [rcx + 0xe0], 0
000d9433: jne    0x1800d943a
000d9435: call   0x182f60cf0
000d943a: xorps  xmm0, xmm0
000d943d: cvtss2sd xmm0, xmm7
000d9441: mulsd  xmm0, xmm6
000d9445: xor    r8d, r8d
000d9448: movaps xmm1, xmm9
000d944c: call   0x181b51fb0                              ; System.Math$$Min
000d9451: movaps xmm9, xmm0
000d9455: mov    rax, qword ptr [rdi + 0x28]
000d9459: test   rax, rax
000d945c: je     0x1800d99c8
000d9462: mov    rcx, qword ptr [rax + 0x30]
000d9466: test   rcx, rcx
000d9469: je     0x1800d99c8
000d946f: addsd  xmm9, qword ptr [rip + 0x2f8a948]        ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
000d9478: movsd  qword ptr [rcx + 0x30], xmm9
000d947e: mov    byte ptr [rsi + 0x31], 1
000d9482: mov    rax, qword ptr [rip + 0x3bea38f]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d9489: mov    rcx, qword ptr [rax + 0xb8]
000d9490: mov    rcx, qword ptr [rcx]
000d9493: test   rcx, rcx
000d9496: je     0x1800d99c8
000d949c: xor    edx, edx
000d949e: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000d94a3: test   al, al
000d94a5: je     0x1800d94da
000d94a7: mov    rdx, qword ptr [rdi + 0x28]
000d94ab: test   rdx, rdx
000d94ae: je     0x1800d99c8
000d94b4: mov    rax, qword ptr [rip + 0x3c35b8d]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d94bb: mov    rcx, qword ptr [rax + 0xb8]
000d94c2: mov    rcx, qword ptr [rcx]
000d94c5: test   rcx, rcx
000d94c8: je     0x1800d99c8
000d94ce: xor    r8d, r8d
000d94d1: mov    rdx, qword ptr [rdx + 0x30]
000d94d5: call   0x1800922d0                              ; ʳʷʴʾʾˁʾʻʲʹʺ$$ʽˀˀʹʻʳʹʶʿʸʺ
000d94da: xorps  xmm0, xmm0
000d94dd: ucomisd xmm9, xmm0
000d94e2: jp     0x1800d9547
000d94e4: jne    0x1800d9547
000d94e6: movzx  eax, byte ptr [rsi + 0x31]
000d94ea: mov    byte ptr [rsp + 0xd0], al
000d94f1: lea    rdx, [rsp + 0xd0]
000d94f9: mov    rcx, qword ptr [rip + 0x3be5b20]         ; [0x3cbf020] meta:bool_TypeInfo
000d9500: call   0x182f5fbe0
000d9505: xor    r8d, r8d
000d9508: mov    rdx, rax
000d950b: mov    rcx, qword ptr [rip + 0x3c084de]         ; [0x3ce19f0] str:'Song length is 0! Is responsible for brokenness: {0}'
000d9512: call   0x181982590                              ; System.String$$Format
000d9517: mov    rbx, rax
000d951a: mov    rcx, qword ptr [rip + 0x3bfee2f]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d9521: cmp    dword ptr [rcx + 0xe0], 0
000d9528: jne    0x1800d952f
000d952a: call   0x182f60cf0
000d952f: xor    edx, edx
000d9531: mov    rcx, rbx
000d9534: call   0x18285f400                              ; UnityEngine.Debug$$LogError
000d9539: mov    byte ptr [rdi + 0xc2], 1
000d9540: xor    al, al
000d9542: jmp    0x1800d921d
000d9547: mov    rax, qword ptr [rdi + 0x28]
000d954b: test   rax, rax
000d954e: je     0x1800d99c8
000d9554: mov    rcx, qword ptr [rax + 0x30]
000d9558: test   rcx, rcx
000d955b: je     0x1800d99c8
000d9561: xor    edx, edx
000d9563: call   0x1801ef1a0                              ; ʵʴʺˁʵʽʾʹʸʹˀ$$ʲʺʾʷʽʻʾʿʳʸʸ
000d9568: movaps xmm10, xmm0
000d956c: mov    rax, qword ptr [rip + 0x3bbedad]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d9573: mov    rcx, qword ptr [rax + 0xb8]
000d957a: mov    rcx, qword ptr [rcx + 0x10]
000d957e: test   rcx, rcx
000d9581: je     0x1800d99c8
000d9587: xor    edx, edx
000d9589: call   0x18210c090                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʸʺˀʳˁʽʸʺʲʻˁ
000d958e: movaps xmm8, xmm0
000d9592: movss  xmm7, dword ptr [rdi + 0x6c]
000d9597: movss  xmm6, dword ptr [rdi + 0x68]
000d959c: mov    rcx, qword ptr [rip + 0x3bb8e7d]         ; [0x3c92420] meta:ʴʶʺˀˀʺʹʴʿʹʷ_TypeInfo
000d95a3: call   0x182f60c00
000d95a8: mov    rbx, rax
000d95ab: mov    qword ptr [rsp + 0x30], r14
000d95b0: movss  dword ptr [rsp + 0x28], xmm6
000d95b6: movss  dword ptr [rsp + 0x20], xmm7
000d95bc: movaps xmm3, xmm8
000d95c0: movaps xmm2, xmm10
000d95c4: movaps xmm1, xmm9
000d95c8: mov    rcx, rax
000d95cb: call   0x18016c9a0                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$.ctor
000d95d0: mov    qword ptr [rdi + 0xb0], rbx
000d95d7: lea    rcx, [rdi + 0xb0]
000d95de: mov    rdx, rbx
000d95e1: call   0x182f5fc00
000d95e6: mov    rcx, qword ptr [rdi + 0xb0]
000d95ed: test   rcx, rcx
000d95f0: je     0x1800d99c8
000d95f6: mov    eax, dword ptr [rdi + 0xd8]
000d95fc: mov    dword ptr [rcx + 0x30], eax
000d95ff: mov    rax, qword ptr [rdi + 0xb0]
000d9606: test   rax, rax
000d9609: je     0x1800d99c8
000d960f: mov    rax, qword ptr [rax + 0x10]
000d9613: mov    qword ptr [rdi + 0x50], rax
000d9617: mov    rcx, qword ptr [rdi + 0xb0]
000d961e: test   rcx, rcx
000d9621: je     0x1800d99c8
000d9627: xor    edx, edx
000d9629: call   0x18016c480                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʵˁʶʹʴʾʸˁʾˀʹ
000d962e: movsd  qword ptr [rdi + 0x70], xmm0
000d9633: movsd  qword ptr [rdi + 0x88], xmm0
000d963b: mov    rcx, qword ptr [rdi + 0xb0]
000d9642: test   rcx, rcx
000d9645: je     0x1800d99c8
000d964b: xor    edx, edx
000d964d: call   0x18016c490                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʳʶʿʹˁʴʻʺʶʹ
000d9652: movsd  qword ptr [rdi + 0x78], xmm0
000d9657: movsd  xmm6, qword ptr [rdi + 0x70]
000d965c: mov    rcx, qword ptr [rip + 0x3bc01fd]         ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000d9663: cmp    dword ptr [rcx + 0xe0], 0
000d966a: jne    0x1800d9671
000d966c: call   0x182f60cf0
000d9671: xor    edx, edx
000d9673: movaps xmm0, xmm6
000d9676: call   0x18022bbf0                              ; ʻʳʿʼʵʷʺʹˀʸʾ$$ʸʺʹʷˀʶʿʿʵˀʵ
000d967b: mov    dword ptr [rdi + 0x120], r14d
000d9682: mov    qword ptr [rdi + 0x118], r14
000d9689: mov    rdx, qword ptr [rip + 0x3bfe358]         ; [0x3cd79e8] metamethod:Method$UnityEngine.Component.GetComponent<ScoreManager>()
000d9690: mov    rcx, rdi
000d9693: call   0x1804674a0                              ; UnityEngine.Component$$GetComponent<object>
000d9698: mov    qword ptr [rdi + 0xb8], rax
000d969f: lea    rcx, [rdi + 0xb8]
000d96a6: mov    rdx, rax
000d96a9: call   0x182f5fc00
000d96ae: mov    rax, qword ptr [rdi + 0x38]
000d96b2: test   rax, rax
000d96b5: je     0x1800d99c8
000d96bb: cmp    byte ptr [rax + 0x38], 0
000d96bf: je     0x1800d96f8
000d96c1: test   rdi, rdi
000d96c4: je     0x1800d99c8
000d96ca: mov    rax, qword ptr [rdi + 0x38]
000d96ce: test   rax, rax
000d96d1: je     0x1800d99c8
000d96d7: cmp    byte ptr [rax + 0x28], 0
000d96db: jne    0x1800d96f8
000d96dd: mov    qword ptr [rsi + 0x18], r14
000d96e1: lea    rcx, [rsi + 0x18]
000d96e5: xor    edx, edx
000d96e7: call   0x182f5fc00
000d96ec: mov    dword ptr [rsi + 0x10], 2
000d96f3: jmp    0x1800d921b
000d96f8: xor    edx, edx
000d96fa: mov    rcx, rdi
000d96fd: call   0x1800b5220                              ; GameManager$$ʳʷʺʼʺʲˁʳʻʼʾ
000d9702: mov    rcx, qword ptr [rdi + 0x48]
000d9706: test   rcx, rcx
000d9709: je     0x1800d99c8
000d970f: xor    r8d, r8d
000d9712: mov    rdx, rdi
000d9715: call   0x1803ad310                              ; LyricDisplay$$ʹʿʿʸʴʸʺʸʹʶʼ
000d971a: mov    rax, qword ptr [rip + 0x3c1616f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9721: cmp    dword ptr [rax + 0xe0], 0
000d9728: jne    0x1800d9739
000d972a: mov    rcx, rax
000d972d: call   0x182f60cf0
000d9732: mov    rax, qword ptr [rip + 0x3c16157]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9739: mov    rax, qword ptr [rax + 0xb8]
000d9740: mov    rcx, qword ptr [rax + 8]
000d9744: test   rcx, rcx
000d9747: je     0x1800d99c8
000d974d: xor    edx, edx
000d974f: call   0x180235a40                              ; GlobalVariables$$ʺʳʾʶʿʳʻˁʶˁʿ
000d9754: test   al, al
000d9756: je     0x1800d9792
000d9758: mov    rax, qword ptr [rip + 0x3c16131]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d975f: cmp    dword ptr [rax + 0xe0], 0
000d9766: jne    0x1800d9777
000d9768: mov    rcx, rax
000d976b: call   0x182f60cf0
000d9770: mov    rax, qword ptr [rip + 0x3c16119]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d9777: mov    rax, qword ptr [rax + 0xb8]
000d977e: mov    rcx, qword ptr [rax + 8]
000d9782: test   rcx, rcx
000d9785: je     0x1800d99c8
000d978b: mov    dword ptr [rcx + 0x7c], 2
000d9792: mov    rax, qword ptr [rip + 0x3bea07f]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000d9799: mov    rcx, qword ptr [rax + 0xb8]
000d97a0: mov    rcx, qword ptr [rcx]
000d97a3: test   rcx, rcx
000d97a6: je     0x1800d99c8
000d97ac: xor    edx, edx
000d97ae: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000d97b3: test   al, al
000d97b5: je     0x1800d98bc
000d97bb: mov    rax, qword ptr [rip + 0x3c35886]         ; [0x3d0f048] meta:ʳʷʴʾʾˁʾʻʲʹʺ_TypeInfo
000d97c2: mov    rcx, qword ptr [rax + 0xb8]
000d97c9: mov    rdx, qword ptr [rcx]
000d97cc: test   rdx, rdx
000d97cf: je     0x1800d99c8
000d97d5: mov    rdx, qword ptr [rdx + 0xa0]
000d97dc: test   rdx, rdx
000d97df: jne    0x1800d97ea
000d97e1: mov    qword ptr [rsi + 0x38], r14
000d97e5: mov    rax, r14
000d97e8: jmp    0x1800d9862
000d97ea: mov    r9, qword ptr [rdx]
000d97ed: mov    r8, qword ptr [rip + 0x3bc179c]          ; [0x3c9af90] meta:ʼʸʺʳʽʿʽʾʽˀʼ_TypeInfo
000d97f4: movzx  eax, byte ptr [r8 + 0x130]
000d97fc: cmp    byte ptr [r9 + 0x130], al
000d9803: jb     0x1800d9819
000d9805: movzx  ecx, al
000d9808: mov    rax, qword ptr [r9 + 0xc8]
000d980f: cmp    qword ptr [rax + rcx*8 - 8], r8
000d9814: mov    ecx, r15d
000d9817: je     0x1800d981c
000d9819: mov    ecx, r14d
000d981c: mov    rax, r14
000d981f: test   ecx, ecx
000d9821: cmovne rax, rdx
000d9825: mov    qword ptr [rsi + 0x38], rax
000d9829: mov    r9, qword ptr [rdx]
000d982c: mov    r8, qword ptr [rip + 0x3bc175d]          ; [0x3c9af90] meta:ʼʸʺʳʽʿʽʾʽˀʼ_TypeInfo
000d9833: movzx  eax, byte ptr [r8 + 0x130]
000d983b: cmp    byte ptr [r9 + 0x130], al
000d9842: jb     0x1800d9855
000d9844: movzx  ecx, al
000d9847: mov    rax, qword ptr [r9 + 0xc8]
000d984e: cmp    qword ptr [rax + rcx*8 - 8], r8
000d9853: je     0x1800d9858
000d9855: mov    r15d, r14d
000d9858: mov    rax, r14
000d985b: test   r15d, r15d
000d985e: cmovne rax, rdx
000d9862: lea    rcx, [rsi + 0x38]
000d9866: mov    rdx, rax
000d9869: call   0x182f5fc00
000d986e: mov    rcx, qword ptr [rsi + 0x38]
000d9872: test   rcx, rcx
000d9875: je     0x1800d99c8
000d987b: xor    edx, edx
000d987d: call   0x1802699c0                              ; ʼʸʺʳʽʿʽʾʽˀʼ$$ʺʷʵʿʺʴʺʸʲʽˁ
000d9882: mov    rax, qword ptr [rsi + 0x38]
000d9886: test   rax, rax
000d9889: je     0x1800d99c8
000d988f: xor    edx, edx
000d9891: cmp    byte ptr [rax + 0x48], dl
000d9894: jne    0x1800d98af
000d9896: mov    qword ptr [rsi + 0x18], r14
000d989a: lea    rcx, [rsi + 0x18]
000d989e: call   0x182f5fc00
000d98a3: mov    dword ptr [rsi + 0x10], 3
000d98aa: jmp    0x1800d921b
000d98af: mov    qword ptr [rsi + 0x38], r14
000d98b3: lea    rcx, [rsi + 0x38]
000d98b7: call   0x182f5fc00
000d98bc: test   rdi, rdi
000d98bf: je     0x1800d99c8
000d98c5: mov    byte ptr [rdi + 0xc1], 1
000d98cc: mov    qword ptr [rdi + 0x178], r14
000d98d3: mov    rax, qword ptr [rip + 0x3c15fb6]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d98da: cmp    dword ptr [rax + 0xe0], 0
000d98e1: jne    0x1800d98f2
000d98e3: mov    rcx, rax
000d98e6: call   0x182f60cf0
000d98eb: mov    rax, qword ptr [rip + 0x3c15f9e]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000d98f2: mov    rax, qword ptr [rax + 0xb8]
000d98f9: mov    rcx, qword ptr [rax + 8]
000d98fd: test   rcx, rcx
000d9900: je     0x1800d99c8
000d9906: cmp    byte ptr [rcx + 0x72], 0
000d990a: jne    0x1800d9947
000d990c: xor    edx, edx
000d990e: mov    rcx, rdi
000d9911: call   0x1800b4940                              ; GameManager$$ʲʲʷʻʻʽʲʷʺʿʶ
000d9916: xor    al, al
000d9918: jmp    0x1800d921d
000d991d: mov    rcx, qword ptr [rip + 0x3bfea2c]         ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d9924: cmp    dword ptr [rcx + 0xe0], 0
000d992b: jne    0x1800d9932
000d992d: call   0x182f60cf0
000d9932: xor    edx, edx
000d9934: mov    rcx, qword ptr [rip + 0x3be9b45]         ; [0x3cc3480] str:'Failed to load song'
000d993b: call   0x18285f400                              ; UnityEngine.Debug$$LogError
000d9940: mov    byte ptr [rdi + 0xc2], 1
000d9947: xor    al, al
000d9949: jmp    0x1800d921d
000d994e: mov    rax, qword ptr [rsp + 0xd0]
000d9956: mov    byte ptr [rax + 0xc2], 1
000d995d: mov    rcx, qword ptr [rsp + 0xe0]
000d9965: test   rcx, rcx
000d9968: je     0x1800d99c8
000d996a: mov    rax, qword ptr [rcx]
000d996d: mov    rdx, qword ptr [rax + 0x1d0]
000d9974: call   qword ptr [rax + 0x1c8]
000d997a: mov    rbx, rax
000d997d: lea    rcx, [rip + 0x3bd9d8c]                   ; [0x3cb3710] str:'Error reading metadata: '
000d9984: call   0x182f609d0
000d9989: mov    rcx, rax
000d998c: xor    r8d, r8d
000d998f: mov    rdx, rbx
000d9992: call   0x18197f200                              ; System.String$$Concat
000d9997: mov    rbx, rax
000d999a: lea    rcx, [rip + 0x3bfe9af]                   ; [0x3cd8350] meta:UnityEngine.Debug_TypeInfo
000d99a1: call   0x182f609d0
000d99a6: cmp    dword ptr [rax + 0xe0], 0
000d99ad: jne    0x1800d99b7
000d99af: mov    rcx, rax
000d99b2: call   0x182f60cf0
000d99b7: xor    edx, edx
000d99b9: mov    rcx, rbx
000d99bc: call   0x18285f400                              ; UnityEngine.Debug$$LogError
000d99c1: xor    al, al
000d99c3: jmp    0x1800d921d
000d99c8: call   0x182f60c50
000d99cd: nop    
000d99ce: call   0x182f60c50
000d99d3: call   0x182f60c50
000d99d8: int3   
000d99d9: int3   
000d99da: int3   
000d99db: int3   
000d99dc: int3   
000d99dd: int3   
000d99de: int3   
000d99df: int3   
