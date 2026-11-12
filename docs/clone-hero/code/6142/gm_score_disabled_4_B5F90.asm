000b5f90: push   rdi
000b5f92: push   r14
000b5f94: sub    rsp, 0x28
000b5f98: cmp    byte ptr [rip + 0x3e5620d], 0            ; [0x3f0c1ac] (bss)
000b5f9f: mov    r14, rdx
000b5fa2: mov    rdi, rcx
000b5fa5: jne    0x1800b601a
000b5fa7: lea    rcx, [rip + 0x3c05eaa]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b5fae: call   0x182f609b0
000b5fb3: lea    rcx, [rip + 0x3c0d85e]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b5fba: call   0x182f609b0
000b5fbf: lea    rcx, [rip + 0x3c0eeaa]                   ; [0x3cc4e70] metamethod:Method$UnityEngine.GameObject.GetComponent<PauseMenu>()
000b5fc6: call   0x182f609b0
000b5fcb: lea    rcx, [rip + 0x3c398be]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b5fd2: call   0x182f609b0
000b5fd7: lea    rcx, [rip + 0x3c48632]                   ; [0x3cfe610] metamethod:Method$System.Nullable<bool>.GetValueOrDefault()
000b5fde: call   0x182f609b0
000b5fe3: lea    rcx, [rip + 0x3c4856e]                   ; [0x3cfe558] metamethod:Method$System.Nullable<bool>..ctor()
000b5fea: call   0x182f609b0
000b5fef: lea    rcx, [rip + 0x3c486da]                   ; [0x3cfe6d0] metamethod:Method$System.Nullable<bool>.get_HasValue()
000b5ff6: call   0x182f609b0
000b5ffb: lea    rcx, [rip + 0x3bfcc66]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b6002: call   0x182f609b0
000b6007: lea    rcx, [rip + 0x3c5853a]                   ; [0x3d0e548] meta:ʲʼʸʳʻʵʹˁʷʶˁ_TypeInfo
000b600e: call   0x182f609b0
000b6013: mov    byte ptr [rip + 0x3e56192], 1            ; [0x3f0c1ac] (bss)
000b601a: cmp    byte ptr [rdi + 0x90], 0
000b6021: mov    qword ptr [rsp + 0x48], rbx
000b6026: mov    qword ptr [rsp + 0x50], rbp
000b602b: mov    qword ptr [rsp + 0x20], rsi
000b6030: jne    0x1800b63ba
000b6036: cmp    byte ptr [rdi + 0x91], 0
000b603d: jne    0x1800b62c0
000b6043: xor    edx, edx
000b6045: mov    rcx, rdi
000b6048: call   0x1800bd520                              ; GameManager$$ʼʴʹʼʸʶʵʾˀʺʶ
000b604d: mov    rax, qword ptr [rip + 0x3c3983c]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b6054: cmp    dword ptr [rax + 0xe0], 0
000b605b: jne    0x1800b606c
000b605d: mov    rcx, rax
000b6060: call   0x182f60cf0
000b6065: mov    rax, qword ptr [rip + 0x3c39824]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b606c: mov    rax, qword ptr [rax + 0xb8]
000b6073: mov    rcx, qword ptr [rax + 8]
000b6077: test   rcx, rcx
000b607a: je     0x1800b64b8
000b6080: xor    r8d, r8d
000b6083: xor    edx, edx
000b6085: call   0x180234790                              ; GlobalVariables$$ʶʼʴʹʷʴʾʿʴʾʴ
000b608a: mov    rax, qword ptr [rip + 0x3c0d787]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b6091: mov    rcx, qword ptr [rax + 0xb8]
000b6098: mov    rcx, qword ptr [rcx]
000b609b: test   rcx, rcx
000b609e: je     0x1800b64b8
000b60a4: xor    edx, edx
000b60a6: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000b60ab: test   al, al
000b60ad: jne    0x1800b6258
000b60b3: mov    rbp, qword ptr [rdi + 0x58]
000b60b7: test   rbp, rbp
000b60ba: je     0x1800b64b8
000b60c0: mov    ebx, 1
000b60c5: mov    eax, ebx
000b60c7: cmp    ebx, dword ptr [rbp + 0x18]
000b60ca: jge    0x1800b6123
000b60cc: cmp    ebx, dword ptr [rbp + 0x18]
000b60cf: jae    0x1800b64be
000b60d5: mov    rcx, qword ptr [rip + 0x3bfcb8c]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b60dc: movsxd rax, ebx
000b60df: cmp    dword ptr [rcx + 0xe0], 0
000b60e6: mov    rsi, qword ptr [rbp + rax*8 + 0x20]
000b60eb: jne    0x1800b60f2
000b60ed: call   0x182f60cf0
000b60f2: xor    r8d, r8d
000b60f5: xor    edx, edx
000b60f7: mov    rcx, rsi
000b60fa: call   0x18287fed0                              ; UnityEngine.Object$$op_Inequality
000b60ff: test   al, al
000b6101: je     0x1800b611f
000b6103: test   rsi, rsi
000b6106: je     0x1800b64b8
000b610c: mov    rax, qword ptr [rsi]
000b610f: mov    rcx, rsi
000b6112: mov    rdx, qword ptr [rax + 0x440]
000b6119: call   qword ptr [rax + 0x438]
000b611f: inc    ebx
000b6121: jmp    0x1800b60c5
000b6123: mov    rcx, qword ptr [rdi + 0xb0]
000b612a: test   rcx, rcx
000b612d: je     0x1800b64b8
000b6133: xor    edx, edx
000b6135: call   0x18016c910                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ˀʽˁˁʸʵʲʻʺʷʻ
000b613a: test   al, al
000b613c: je     0x1800b617b
000b613e: mov    rax, qword ptr [rip + 0x3c05d13]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b6145: cmp    dword ptr [rax + 0xe0], 0
000b614c: jne    0x1800b615d
000b614e: mov    rcx, rax
000b6151: call   0x182f60cf0
000b6156: mov    rax, qword ptr [rip + 0x3c05cfb]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000b615d: mov    rax, qword ptr [rax + 0xb8]
000b6164: mov    rcx, qword ptr [rax + 8]
000b6168: test   rcx, rcx
000b616b: je     0x1800b64b8
000b6171: xor    r8d, r8d
000b6174: mov    dl, 1
000b6176: call   0x1803681a0                              ; BassAudioManager$$ʷʻʶˀʾʵʸˀʳˀʽ
000b617b: mov    rax, qword ptr [rdi + 0x38]
000b617f: test   rax, rax
000b6182: je     0x1800b64b8
000b6188: cmp    byte ptr [rax + 0x60], 0
000b618c: je     0x1800b6198
000b618e: xor    edx, edx
000b6190: mov    rcx, rax
000b6193: call   0x180182de0                              ; SongBackground$$ʽʿˁʹʲʴʴʳʾˀʼ
000b6198: movss  xmm0, dword ptr [rip + 0x2fadef0]        ; [0x3064090] flt=594.0 i=1142194176
000b61a0: xor    edx, edx
000b61a2: mov    byte ptr [rdi + 0x90], 1
000b61a9: call   0x182887ce0                              ; UnityEngine.Time$$set_timeScale
000b61ae: cmp    qword ptr [rdi + 0x150], 0
000b61b6: je     0x1800b61ca
000b61b8: mov    rdx, qword ptr [rdi + 0x150]
000b61bf: xor    r8d, r8d
000b61c2: mov    rcx, rdi
000b61c5: call   0x18287d8f0                              ; UnityEngine.MonoBehaviour$$StopCoroutine
000b61ca: mov    rcx, qword ptr [rip + 0x3c396bf]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b61d1: cmp    dword ptr [rcx + 0xe0], 0
000b61d8: jne    0x1800b61e6
000b61da: call   0x182f60cf0
000b61df: mov    rcx, qword ptr [rip + 0x3c396aa]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b61e6: mov    rax, qword ptr [rcx + 0xb8]
000b61ed: mov    rdx, qword ptr [rax + 8]
000b61f1: test   rdx, rdx
000b61f4: je     0x1800b64b8
000b61fa: cmp    dword ptr [rdx + 0x7c], 0
000b61fe: jne    0x1800b6258
000b6200: cmp    dword ptr [rcx + 0xe0], 0
000b6207: jne    0x1800b6215
000b6209: call   0x182f60cf0
000b620e: mov    rcx, qword ptr [rip + 0x3c3967b]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b6215: mov    rax, qword ptr [rcx + 0xb8]
000b621c: mov    rcx, qword ptr [rax + 8]
000b6220: test   rcx, rcx
000b6223: je     0x1800b64b8
000b6229: cmp    byte ptr [rcx + 0x72], 0
000b622d: jne    0x1800b6258
000b622f: movsd  xmm1, qword ptr [rdi + 0x70]
000b6234: subsd  xmm1, qword ptr [rdi + 0x118]
000b623c: movsd  xmm0, qword ptr [rip + 0x2fadd8c]        ; [0x3063fd0] dbl=1185.0 q=0x4092840000000000
000b6244: comisd xmm0, xmm1
000b6248: ja     0x1800b62d7
000b624e: mov    dword ptr [rdi + 0x120], 1
000b6258: mov    rcx, qword ptr [rdi + 0x140]
000b625f: test   rcx, rcx
000b6262: je     0x1800b64b8
000b6268: mov    rdx, qword ptr [rip + 0x3c0ec01]         ; [0x3cc4e70] metamethod:Method$UnityEngine.GameObject.GetComponent<PauseMenu>()
000b626f: call   0x1804a0650                              ; UnityEngine.GameObject$$GetComponent<object>
000b6274: mov    rbx, rax
000b6277: test   rax, rax
000b627a: je     0x1800b64b8
000b6280: lea    rcx, [rax + 0x20]
000b6284: mov    qword ptr [rax + 0x20], r14
000b6288: mov    rdx, r14
000b628b: call   0x182f5fc00
000b6290: xor    edx, edx
000b6292: mov    rcx, rbx
000b6295: call   0x18023c790                              ; PauseMenu$$ʷʾʿʻʲʶʾʻʳˁʸ
000b629a: mov    rcx, qword ptr [rdi + 0xb0]
000b62a1: mov    byte ptr [rdi + 0x90], 1
000b62a8: test   rcx, rcx
000b62ab: je     0x1800b64b8
000b62b1: xor    edx, edx
000b62b3: call   0x18016c550                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʹʷˁʴʼˀʿʴʳʷʷ
000b62b8: movsd  qword ptr [rdi + 0x118], xmm0
000b62c0: mov    rsi, qword ptr [rsp + 0x20]
000b62c5: mov    rbp, qword ptr [rsp + 0x50]
000b62ca: mov    rbx, qword ptr [rsp + 0x48]
000b62cf: add    rsp, 0x28
000b62d3: pop    r14
000b62d5: pop    rdi
000b62d6: ret    
000b62d7: mov    eax, dword ptr [rdi + 0x120]
000b62dd: mov    dword ptr [rdi + 0x120], eax
000b62e3: cmp    eax, 3
000b62e6: jle    0x1800b6258
000b62ec: mov    rax, qword ptr [rip + 0x3c3959d]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b62f3: cmp    dword ptr [rax + 0xe0], 0
000b62fa: jne    0x1800b630b
000b62fc: mov    rcx, rax
000b62ff: call   0x182f60cf0
000b6304: mov    rax, qword ptr [rip + 0x3c39585]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b630b: mov    rax, qword ptr [rax + 0xb8]
000b6312: mov    rcx, qword ptr [rax + 8]
000b6316: test   rcx, rcx
000b6319: je     0x1800b64b8
000b631f: mov    dword ptr [rcx + 0x7c], 4
000b6326: mov    rbp, qword ptr [rdi + 0x58]
000b632a: test   rbp, rbp
000b632d: je     0x1800b64b8
000b6333: xor    ebx, ebx
000b6335: cmp    dword ptr [rbp + 0x18], ebx
000b6338: jle    0x1800b6258
000b633e: jbe    0x1800b64be
000b6344: mov    rsi, qword ptr [rbp + 0x20]
000b6348: test   rsi, rsi
000b634b: je     0x1800b6335
000b634d: mov    rax, qword ptr [rsi]
000b6350: mov    rcx, rsi
000b6353: mov    rdx, qword ptr [rax + 0x300]
000b635a: call   qword ptr [rax + 0x2f8]
000b6360: test   rax, rax
000b6363: je     0x1800b6399
000b6365: mov    rdx, qword ptr [rip + 0x3c581dc]         ; [0x3d0e548] meta:ʲʼʸʳʻʵʹˁʷʶˁ_TypeInfo
000b636c: mov    ecx, 8
000b6371: mov    r8, rax
000b6374: call   0x182c44210
000b6379: mov    r8, qword ptr [rip + 0x3c481d8]          ; [0x3cfe558] metamethod:Method$System.Nullable<bool>..ctor()
000b6380: lea    rcx, [rsp + 0x40]
000b6385: movzx  edx, al
000b6388: mov    word ptr [rsp + 0x40], bx
000b638d: call   0x1808685f0                              ; System.Data.SqlTypes.SqlByte$$.ctor
000b6392: movzx  ecx, word ptr [rsp + 0x40]
000b6397: jmp    0x1800b639b
000b6399: mov    ecx, ebx
000b639b: movzx  eax, cx
000b639e: shr    ax, 8
000b63a2: test   al, al
000b63a4: sete   dl
000b63a7: test   cl, dl
000b63a9: je     0x1800b6335
000b63ab: xor    edx, edx
000b63ad: mov    rcx, rsi
000b63b0: call   0x1802083a0                              ; BasePlayer$$ʴʻʳʽʻʼʲʸˁˀʸ
000b63b5: jmp    0x1800b6335
000b63ba: mov    rax, qword ptr [rip + 0x3c0d457]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000b63c1: mov    rcx, qword ptr [rax + 0xb8]
000b63c8: mov    rcx, qword ptr [rcx]
000b63cb: test   rcx, rcx
000b63ce: je     0x1800b64b8
000b63d4: xor    edx, edx
000b63d6: call   0x180146cd0                              ; CHNetManager$$ʾʻʼʺʵʶʷʹʵʽʹ
000b63db: test   al, al
000b63dd: je     0x1800b63e8
000b63df: mov    byte ptr [rdi + 0x90], 0
000b63e6: jmp    0x1800b640f
000b63e8: movss  xmm0, dword ptr [rip + 0x2fadc50]        ; [0x3064040] flt=319.0 i=1134526464
000b63f0: xor    edx, edx
000b63f2: call   0x182887ce0                              ; UnityEngine.Time$$set_timeScale
000b63f7: xor    edx, edx
000b63f9: mov    rcx, rdi
000b63fc: call   0x1800b5d40                              ; GameManager$$ʴʴˀʼʽʻʷʴʸʴʵ
000b6401: xor    r8d, r8d
000b6404: mov    rdx, rax
000b6407: mov    rcx, rdi
000b640a: call   0x18287d480                              ; UnityEngine.MonoBehaviour$$StartCoroutine_Auto
000b640f: mov    rax, qword ptr [rip + 0x3c3947a]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b6416: cmp    dword ptr [rax + 0xe0], 0
000b641d: jne    0x1800b642e
000b641f: mov    rcx, rax
000b6422: call   0x182f60cf0
000b6427: mov    rax, qword ptr [rip + 0x3c39462]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b642e: mov    rax, qword ptr [rax + 0xb8]
000b6435: mov    rcx, qword ptr [rax + 8]
000b6439: test   rcx, rcx
000b643c: je     0x1800b64b8
000b643e: xor    r8d, r8d
000b6441: xor    edx, edx
000b6443: call   0x180234790                              ; GlobalVariables$$ʶʼʴʹʷʴʾʿʴʾʴ
000b6448: xor    edx, edx
000b644a: mov    rcx, rdi
000b644d: call   0x1800b6570                              ; GameManager$$ʶʴʸʹʼʵʸʴʳʻʶ
000b6452: mov    rsi, qword ptr [rdi + 0x58]
000b6456: test   rsi, rsi
000b6459: je     0x1800b64b8
000b645b: xor    ebx, ebx
000b645d: mov    eax, ebx
000b645f: nop    
000b6460: cmp    ebx, dword ptr [rsi + 0x18]
000b6463: jge    0x1800b62c0
000b6469: cmp    ebx, dword ptr [rsi + 0x18]
000b646c: jae    0x1800b64be
000b646e: mov    rcx, qword ptr [rip + 0x3bfc7f3]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b6475: movsxd rax, ebx
000b6478: cmp    dword ptr [rcx + 0xe0], 0
000b647f: mov    rdi, qword ptr [rsi + rax*8 + 0x20]
000b6484: jne    0x1800b648b
000b6486: call   0x182f60cf0
000b648b: xor    r8d, r8d
000b648e: xor    edx, edx
000b6490: mov    rcx, rdi
000b6493: call   0x18287fed0                              ; UnityEngine.Object$$op_Inequality
000b6498: test   al, al
000b649a: je     0x1800b64b4
000b649c: test   rdi, rdi
000b649f: je     0x1800b64b8
000b64a1: mov    rax, qword ptr [rdi]
000b64a4: mov    rcx, rdi
000b64a7: mov    rdx, qword ptr [rax + 0x3a0]
000b64ae: call   qword ptr [rax + 0x398]
000b64b4: inc    ebx
000b64b6: jmp    0x1800b645d
000b64b8: call   0x182f60c50
000b64bd: int3   
000b64be: call   0x182f60c40
000b64c3: int3   
000b64c4: int3   
