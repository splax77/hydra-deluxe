000beb50: push   rdi
000beb52: sub    rsp, 0x80
000beb59: cmp    byte ptr [rip + 0x3e4d652], 0            ; [0x3f0c1b2] (bss)
000beb60: mov    rdi, rcx
000beb63: jne    0x1800beba8
000beb65: lea    rcx, [rip + 0x3bfd2ec]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000beb6c: call   0x182f609b0
000beb71: lea    rcx, [rip + 0x3c04ca0]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
000beb78: call   0x182f609b0
000beb7d: lea    rcx, [rip + 0x3c29524]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000beb84: call   0x182f609b0
000beb89: lea    rcx, [rip + 0x3c30d00]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000beb90: call   0x182f609b0
000beb95: lea    rcx, [rip + 0x3bdacc4]                   ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000beb9c: call   0x182f609b0
000beba1: mov    byte ptr [rip + 0x3e4d60a], 1            ; [0x3f0c1b2] (bss)
000beba8: cmp    byte ptr [rdi + 0x158], 0
000bebaf: je     0x1800bf1d8
000bebb5: mov    qword ptr [rsp + 0x90], rbx
000bebbd: mov    ebx, dword ptr [rdi + 0x194]
000bebc3: xor    ecx, ecx
000bebc5: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000bebca: cmp    ebx, eax
000bebcc: jne    0x1800bebdf
000bebce: mov    ebx, dword ptr [rdi + 0x198]
000bebd4: xor    ecx, ecx
000bebd6: call   0x182871890                              ; UnityEngine.Screen$$get_height
000bebdb: cmp    ebx, eax
000bebdd: je     0x1800bec22
000bebdf: xor    ecx, ecx
000bebe1: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000bebe6: xor    ecx, ecx
000bebe8: mov    dword ptr [rdi + 0x194], eax
000bebee: call   0x182871890                              ; UnityEngine.Screen$$get_height
000bebf3: xor    ecx, ecx
000bebf5: mov    dword ptr [rdi + 0x198], eax
000bebfb: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000bec00: xor    ecx, ecx
000bec02: mov    ebx, eax
000bec04: call   0x182871890                              ; UnityEngine.Screen$$get_height
000bec09: movd   xmm1, ebx
000bec0d: xor    r9d, r9d
000bec10: cvtdq2ps xmm1, xmm1
000bec13: mov    rcx, rdi
000bec16: movd   xmm2, eax
000bec1a: cvtdq2ps xmm2, xmm2
000bec1d: call   0x1800c02f0                              ; GameManager$$ˀˀʴʸʾʴʶʵʴʽʷ
000bec22: movsd  xmm0, qword ptr [rdi + 0x70]
000bec27: comisd xmm0, xmmword ptr [rdi + 0x178]
000bec2f: mov    qword ptr [rsp + 0x98], rsi
000bec37: movaps xmmword ptr [rsp + 0x70], xmm6
000bec3c: movaps xmmword ptr [rsp + 0x60], xmm7
000bec41: movaps xmmword ptr [rsp + 0x50], xmm8
000bec47: movaps xmmword ptr [rsp + 0x40], xmm9
000bec4d: movaps xmmword ptr [rsp + 0x30], xmm10
000bec53: movaps xmmword ptr [rsp + 0x20], xmm11
000bec59: jb     0x1800bec95
000bec5b: mov    rax, qword ptr [rip + 0x3bfd1f6]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000bec62: cmp    dword ptr [rax + 0xe0], 0
000bec69: jne    0x1800bec7a
000bec6b: mov    rcx, rax
000bec6e: call   0x182f60cf0
000bec73: mov    rax, qword ptr [rip + 0x3bfd1de]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000bec7a: mov    rax, qword ptr [rax + 0xb8]
000bec81: mov    rcx, qword ptr [rax + 8]
000bec85: test   rcx, rcx
000bec88: je     0x1800bf1e7
000bec8e: mov    byte ptr [rcx + 0x175], 0
000bec95: cmp    byte ptr [rdi + 0x90], 0
000bec9c: mov    byte ptr [rdi + 0xc0], 0
000beca3: je     0x1800becce
000beca5: mov    rax, qword ptr [rip + 0x3c04b6c]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000becac: mov    rcx, qword ptr [rax + 0xb8]
000becb3: mov    rcx, qword ptr [rcx]
000becb6: test   rcx, rcx
000becb9: je     0x1800bf1e7
000becbf: xor    edx, edx
000becc1: call   0x180146cd0                              ; CHNetManager$$ʾʻʼʺʵʶʷʹʵʽʹ
000becc6: test   al, al
000becc8: je     0x1800bf1a6
000becce: mov    rcx, qword ptr [rip + 0x3bfd183]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000becd5: cmp    dword ptr [rcx + 0xe0], 0
000becdc: jne    0x1800becea
000becde: call   0x182f60cf0
000bece3: mov    rcx, qword ptr [rip + 0x3bfd16e]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000becea: mov    rax, qword ptr [rcx + 0xb8]
000becf1: mov    rdx, qword ptr [rax + 8]
000becf5: test   rdx, rdx
000becf8: je     0x1800bf1e7
000becfe: cmp    byte ptr [rdx + 0x175], 0
000bed05: jne    0x1800bede4
000bed0b: cmp    dword ptr [rcx + 0xe0], 0
000bed12: jne    0x1800bed20
000bed14: call   0x182f60cf0
000bed19: mov    rcx, qword ptr [rip + 0x3bfd138]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000bed20: mov    rax, qword ptr [rcx + 0xb8]
000bed27: mov    rcx, qword ptr [rax + 8]
000bed2b: test   rcx, rcx
000bed2e: je     0x1800bf1e7
000bed34: cmp    byte ptr [rcx + 0x174], 0
000bed3b: je     0x1800bede4
000bed41: mov    rax, qword ptr [rip + 0x3c04ad0]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000bed48: mov    rcx, qword ptr [rax + 0xb8]
000bed4f: mov    rcx, qword ptr [rcx]
000bed52: test   rcx, rcx
000bed55: je     0x1800bf1e7
000bed5b: xor    edx, edx
000bed5d: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000bed62: test   al, al
000bed64: je     0x1800bedd7
000bed66: mov    rcx, qword ptr [rdi + 0xb0]
000bed6d: test   rcx, rcx
000bed70: je     0x1800bf1e7
000bed76: xor    edx, edx
000bed78: call   0x18016c910                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ˀʽˁˁʸʵʲʻʺʷʻ
000bed7d: test   al, al
000bed7f: je     0x1800bedd7
000bed81: mov    rcx, qword ptr [rdi + 0xa0]
000bed88: test   rcx, rcx
000bed8b: je     0x1800bf1e7
000bed91: xor    r8d, r8d
000bed94: xor    edx, edx
000bed96: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
000bed9b: mov    rax, qword ptr [rip + 0x3bfd0b6]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000beda2: cmp    dword ptr [rax + 0xe0], 0
000beda9: jne    0x1800bedba
000bedab: mov    rcx, rax
000bedae: call   0x182f60cf0
000bedb3: mov    rax, qword ptr [rip + 0x3bfd09e]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000bedba: mov    rax, qword ptr [rax + 0xb8]
000bedc1: mov    rcx, qword ptr [rax + 8]
000bedc5: test   rcx, rcx
000bedc8: je     0x1800bf1e7
000bedce: xor    edx, edx
000bedd0: call   0x18036b6f0                              ; BassAudioManager$$ʽˁʵʵʷˀʾʴʿʷʿ
000bedd5: jmp    0x1800bede4
000bedd7: xor    r8d, r8d
000bedda: xor    edx, edx
000beddc: mov    rcx, rdi
000beddf: call   0x1800b5f90                              ; GameManager$$ʵʿʲʾʺʵʽʾʼˀʽ
000bede4: mov    rcx, qword ptr [rdi + 0xb0]
000bedeb: test   rcx, rcx
000bedee: je     0x1800bf1e7
000bedf4: xor    edx, edx
000bedf6: call   0x18016c1a0                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʴʼʴʷʼʶʽʾʳʶʹ
000bedfb: mov    rcx, qword ptr [rdi + 0xb0]
000bee02: mov    rax, qword ptr [rdi + 0x70]
000bee06: mov    qword ptr [rdi + 0x80], rax
000bee0d: test   rcx, rcx
000bee10: je     0x1800bf1e7
000bee16: xor    edx, edx
000bee18: call   0x18016c480                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʵˁʶʹʴʾʸˁʾˀʹ
000bee1d: mov    rcx, qword ptr [rdi + 0xb0]
000bee24: movsd  qword ptr [rdi + 0x70], xmm0
000bee29: test   rcx, rcx
000bee2c: je     0x1800bf1e7
000bee32: xor    edx, edx
000bee34: call   0x18016c490                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʳʶʿʹˁʴʻʺʶʹ
000bee39: mov    rcx, qword ptr [rdi + 0x20]
000bee3d: movsd  qword ptr [rdi + 0x78], xmm0
000bee42: test   rcx, rcx
000bee45: je     0x1800bf1e7
000bee4b: xor    edx, edx
000bee4d: call   0x180356190                              ; NoteAnimator$$ʴʳʷʴʹʹʾˀʹʾʶ
000bee52: movsd  xmm0, qword ptr [rdi + 0x70]
000bee57: comisd xmm0, xmmword ptr [rdi + 0x88]
000bee5f: jbe    0x1800bee95
000bee61: mov    rax, qword ptr [rdi + 0x70]
000bee65: movaps xmm6, xmm0
000bee68: mov    qword ptr [rdi + 0x88], rax
000bee6f: mov    byte ptr [rdi + 0xc0], 1
000bee76: mov    rcx, qword ptr [rip + 0x3bda9e3]         ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000bee7d: cmp    dword ptr [rcx + 0xe0], 0
000bee84: jne    0x1800bee8b
000bee86: call   0x182f60cf0
000bee8b: xor    edx, edx
000bee8d: movaps xmm0, xmm6
000bee90: call   0x18022b760                              ; ʻʳʿʼʵʷʺʹˀʸʾ$$ʴʵʾˀʾʺˀˁʽʵʹ
000bee95: movsd  xmm0, qword ptr [rdi + 0x70]
000bee9a: comisd xmm0, xmmword ptr [rdi + 0x50]
000bee9f: jb     0x1800bef18
000beea1: mov    rax, qword ptr [rip + 0x3c309e8]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000beea8: cmp    dword ptr [rax + 0xe0], 0
000beeaf: jne    0x1800beec0
000beeb1: mov    rcx, rax
000beeb4: call   0x182f60cf0
000beeb9: mov    rax, qword ptr [rip + 0x3c309d0]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000beec0: mov    rax, qword ptr [rax + 0xb8]
000beec7: mov    rcx, qword ptr [rax + 8]
000beecb: test   rcx, rcx
000beece: je     0x1800bf1e7
000beed4: cmp    byte ptr [rcx + 0x72], 0
000beed8: jne    0x1800bef0b
000beeda: mov    rax, qword ptr [rip + 0x3c291c7]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000beee1: mov    rcx, qword ptr [rax + 0xb8]
000beee8: mov    rcx, qword ptr [rcx]
000beeeb: test   rcx, rcx
000beeee: je     0x1800bf1e7
000beef4: xor    edx, edx
000beef6: call   0x180258860                              ; FadeBehaviour$$ʾˁʼʶʲʻˁʶʿʴˁ
000beefb: test   al, al
000beefd: jne    0x1800bef18
000beeff: xor    edx, edx
000bef01: mov    rcx, rdi
000bef04: call   0x1800bd640                              ; GameManager$$ʼʼʹʿʿˀˁʵʹʾʻ
000bef09: jmp    0x1800bef18
000bef0b: xor    r8d, r8d
000bef0e: mov    dl, 1
000bef10: mov    rcx, rdi
000bef13: call   0x1800bf510                              ; GameManager$$ʾʽʽʹʽʿʳʾʽʻʼ
000bef18: mov    rax, qword ptr [rdi + 0x28]
000bef1c: test   rax, rax
000bef1f: je     0x1800bf1e7
000bef25: cmp    qword ptr [rax + 0x40], 0
000bef2a: je     0x1800bef6a
000bef2c: mov    rcx, qword ptr [rax + 0x40]
000bef30: mov    rcx, qword ptr [rcx + 0x18]
000bef34: test   rcx, rcx
000bef37: je     0x1800bf1e7
000bef3d: xor    r8d, r8d
000bef40: mov    edx, 0xfffffff6
000bef45: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000bef4a: test   al, al
000bef4c: je     0x1800bef6a
000bef4e: mov    rdx, qword ptr [rdi + 0x28]
000bef52: test   rdx, rdx
000bef55: je     0x1800bf1e7
000bef5b: mov    rdx, qword ptr [rdx + 0x40]
000bef5f: xor    r8d, r8d
000bef62: mov    rcx, rdi
000bef65: call   0x1800b9680                              ; GameManager$$ʷʻʶˀʾʵʸˀʳˀʽ
000bef6a: xor    edx, edx
000bef6c: mov    rcx, rdi
000bef6f: call   0x1800b5c70                              ; GameManager$$ʴʳʾʵʴʻʺʸʵʿʿ
000bef74: mov    rcx, qword ptr [rip + 0x3c30915]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000bef7b: cmp    dword ptr [rcx + 0xe0], 0
000bef82: jne    0x1800bef90
000bef84: call   0x182f60cf0
000bef89: mov    rcx, qword ptr [rip + 0x3c30900]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000bef90: mov    rax, qword ptr [rcx + 0xb8]
000bef97: mov    rdx, qword ptr [rax + 8]
000bef9b: test   rdx, rdx
000bef9e: je     0x1800bf1e7
000befa4: cmp    byte ptr [rdx + 0x72], 0
000befa8: je     0x1800bf1a6
000befae: cmp    dword ptr [rcx + 0xe0], 0
000befb5: jne    0x1800befc3
000befb7: call   0x182f60cf0
000befbc: mov    rcx, qword ptr [rip + 0x3c308cd]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000befc3: mov    rax, qword ptr [rcx + 0xb8]
000befca: mov    rcx, qword ptr [rax + 8]
000befce: test   rcx, rcx
000befd1: je     0x1800bf1e7
000befd7: mov    rsi, qword ptr [rcx + 0x38]
000befdb: test   rsi, rsi
000befde: je     0x1800bf1e7
000befe4: movss  xmm6, dword ptr [rip + 0x2fa4ff0]        ; [0x3063fdc] flt=7.0 i=1088421888
000befec: movss  xmm8, dword ptr [rip + 0x2fa50ab]        ; [0x30640a0] flt=626.0 i=1142718464
000beff5: movss  xmm9, dword ptr [rip + 0x2fa51fe]        ; [0x30641fc] flt=1871.0 i=1156177920
000beffe: movss  xmm7, dword ptr [rip + 0x2fa51ee]        ; [0x30641f4] flt=1845.0 i=1155964928
000bf006: movss  xmm10, dword ptr [rip + 0x2fa5085]       ; [0x3064094] flt=606.0 i=1142390784
000bf00f: movss  xmm11, dword ptr [rip + 0x2fa5148]       ; [0x3064160] flt=1368.0 i=1152057344
000bf018: nop    dword ptr [rax + rax]
000bf020: cmp    dword ptr [rsi + 0x18], 0
000bf024: jle    0x1800bf1a6
000bf02a: jbe    0x1800bf1e1
000bf030: mov    rbx, qword ptr [rsi + 0x20]
000bf034: test   rbx, rbx
000bf037: je     0x1800bf1e7
000bf03d: xor    edx, edx
000bf03f: mov    rcx, rbx
000bf042: call   0x180337cc0                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʻʻʽʴʷˁʳˀʲʹʹ
000bf047: test   al, al
000bf049: je     0x1800bf020
000bf04b: mov    rcx, qword ptr [rbx + 0x18]
000bf04f: test   rcx, rcx
000bf052: je     0x1800bf1e7
000bf058: xor    r8d, r8d
000bf05b: mov    edx, 2
000bf060: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000bf065: test   al, al
000bf067: je     0x1800bf076
000bf069: xor    r8d, r8d
000bf06c: mov    dl, 1
000bf06e: mov    rcx, rdi
000bf071: call   0x1800bf510                              ; GameManager$$ʾʽʽʹʽʿʳʾʽʻʼ
000bf076: mov    rcx, qword ptr [rbx + 0x18]
000bf07a: test   rcx, rcx
000bf07d: je     0x1800bf1e7
000bf083: xor    r8d, r8d
000bf086: mov    edx, 0x44
000bf08b: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000bf090: test   al, al
000bf092: jne    0x1800bf138
000bf098: mov    rcx, qword ptr [rbx + 0x18]
000bf09c: test   rcx, rcx
000bf09f: je     0x1800bf1e7
000bf0a5: xor    r8d, r8d
000bf0a8: mov    edx, 0x3b
000bf0ad: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000bf0b2: test   al, al
000bf0b4: je     0x1800bf020
000bf0ba: comiss xmm6, dword ptr [rdi + 0xdc]
000bf0c1: jae    0x1800bf1a6
000bf0c7: movss  xmm1, dword ptr [rdi + 0xd8]
000bf0cf: movss  xmm0, dword ptr [rdi + 0xdc]
000bf0d7: subss  xmm1, xmm8
000bf0dc: mov    rax, qword ptr [rdi + 0xb0]
000bf0e3: subss  xmm0, xmm9
000bf0e8: movss  dword ptr [rdi + 0xd8], xmm1
000bf0f0: movss  dword ptr [rdi + 0xdc], xmm0
000bf0f8: test   rax, rax
000bf0fb: je     0x1800bf1e7
000bf101: xor    edx, edx
000bf103: movss  dword ptr [rax + 0x30], xmm1
000bf108: mov    rcx, rdi
000bf10b: call   0x1800ba640                              ; GameManager$$ʸʿʽˁʸʹˀʵʺʿʵ
000bf110: mov    rcx, qword ptr [rdi + 0x30]
000bf114: test   rcx, rcx
000bf117: je     0x1800bf1e7
000bf11d: cvttss2si edx, dword ptr [rdi + 0xdc]
000bf125: xor    r9d, r9d
000bf128: mov    r8b, 1
000bf12b: sub    edx, 0x24
000bf12e: call   0x18013b030                              ; PracticeUI$$ʻʸʸʺʷˀʼʲʸˀʷ
000bf133: jmp    0x1800bf020
000bf138: movss  xmm0, dword ptr [rdi + 0xdc]
000bf140: comiss xmm0, xmm7
000bf143: jae    0x1800bf1a6
000bf145: movss  xmm1, dword ptr [rdi + 0xd8]
000bf14d: addss  xmm0, xmm11
000bf152: mov    rax, qword ptr [rdi + 0xb0]
000bf159: addss  xmm1, xmm10
000bf15e: movss  dword ptr [rdi + 0xdc], xmm0
000bf166: movss  dword ptr [rdi + 0xd8], xmm1
000bf16e: test   rax, rax
000bf171: je     0x1800bf1e7
000bf173: xor    edx, edx
000bf175: movss  dword ptr [rax + 0x30], xmm1
000bf17a: mov    rcx, rdi
000bf17d: call   0x1800ba640                              ; GameManager$$ʸʿʽˁʸʹˀʵʺʿʵ
000bf182: mov    rcx, qword ptr [rdi + 0x30]
000bf186: test   rcx, rcx
000bf189: je     0x1800bf1e7
000bf18b: cvttss2si edx, dword ptr [rdi + 0xdc]
000bf193: xor    r9d, r9d
000bf196: mov    r8b, 1
000bf199: add    edx, 0x28
000bf19c: call   0x18013b870                              ; PracticeUI$$ʿʸʳʻʶˁʹʿʹˁʽ
000bf1a1: jmp    0x1800bf020
000bf1a6: movaps xmm10, xmmword ptr [rsp + 0x30]
000bf1ac: movaps xmm9, xmmword ptr [rsp + 0x40]
000bf1b2: movaps xmm8, xmmword ptr [rsp + 0x50]
000bf1b8: movaps xmm7, xmmword ptr [rsp + 0x60]
000bf1bd: movaps xmm6, xmmword ptr [rsp + 0x70]
000bf1c2: mov    rsi, qword ptr [rsp + 0x98]
000bf1ca: movaps xmm11, xmmword ptr [rsp + 0x20]
000bf1d0: mov    rbx, qword ptr [rsp + 0x90]
000bf1d8: add    rsp, 0x80
000bf1df: pop    rdi
000bf1e0: ret    
000bf1e1: call   0x182f60c40
000bf1e6: int3   
000bf1e7: call   0x182f60c50
000bf1ec: int3   
000bf1ed: int3   
