001d7060: push   rbx
001d7062: sub    rsp, 0xb0
001d7069: cmp    byte ptr [rip + 0x3d35add], 0            ; [0x3f0cb4d] (bss)
001d7070: mov    rbx, rcx
001d7073: jne    0x1801d70a0
001d7075: lea    rcx, [rip + 0x3ae43d4]                   ; [0x3cbb450] meta:BaseMenu_TypeInfo
001d707c: call   0x182f609b0
001d7081: lea    rcx, [rip + 0x3b11020]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
001d7088: call   0x182f609b0
001d708d: lea    rcx, [rip + 0x3b187fc]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d7094: call   0x182f609b0
001d7099: mov    byte ptr [rip + 0x3d35aad], 1            ; [0x3f0cb4d] (bss)
001d70a0: mov    rax, qword ptr [rip + 0x3b11001]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
001d70a7: mov    qword ptr [rsp + 0xc0], rbp
001d70af: mov    qword ptr [rsp + 0xc8], rsi
001d70b7: mov    qword ptr [rsp + 0xd0], rdi
001d70bf: mov    rcx, qword ptr [rax + 0xb8]
001d70c6: movaps xmmword ptr [rsp + 0xa0], xmm6
001d70ce: movaps xmmword ptr [rsp + 0x80], xmm8
001d70d7: movaps xmmword ptr [rsp + 0x70], xmm9
001d70dd: mov    rcx, qword ptr [rcx]
001d70e0: movaps xmmword ptr [rsp + 0x60], xmm10
001d70e6: movaps xmmword ptr [rsp + 0x50], xmm11
001d70ec: movaps xmmword ptr [rsp + 0x40], xmm12
001d70f2: test   rcx, rcx
001d70f5: je     0x1801d7563
001d70fb: xor    edx, edx
001d70fd: call   0x180258820                              ; FadeBehaviour$$ʾʽʻʷʺʺʺʳʴʵʸ
001d7102: test   al, al
001d7104: jne    0x1801d7519
001d710a: mov    rcx, qword ptr [rip + 0x3b1877f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d7111: cmp    byte ptr [rbx + 0xc8], al
001d7117: jne    0x1801d7181
001d7119: cmp    dword ptr [rcx + 0xe0], 0
001d7120: jne    0x1801d712e
001d7122: call   0x182f60cf0
001d7127: mov    rcx, qword ptr [rip + 0x3b18762]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d712e: mov    rax, qword ptr [rcx + 0xb8]
001d7135: mov    rdx, qword ptr [rax + 8]
001d7139: test   rdx, rdx
001d713c: je     0x1801d7563
001d7142: mov    rax, qword ptr [rdx + 0x138]
001d7149: test   rax, rax
001d714c: je     0x1801d7563
001d7152: cmp    dword ptr [rax + 0x2c], 0
001d7156: setle  al
001d7159: cmp    al, byte ptr [rbx + 0x9b]
001d715f: je     0x1801d7181
001d7161: mov    byte ptr [rbx + 0x9b], al
001d7167: mov    rcx, rbx
001d716a: mov    rax, qword ptr [rbx]
001d716d: mov    rdx, qword ptr [rax + 0x440]
001d7174: call   qword ptr [rax + 0x438]
001d717a: mov    rcx, qword ptr [rip + 0x3b1870f]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d7181: cmp    qword ptr [rbx + 0x20], 0
001d7186: je     0x1801d71b0
001d7188: mov    rax, qword ptr [rbx]
001d718b: mov    rcx, rbx
001d718e: mov    rdx, qword ptr [rbx + 0x20]
001d7192: mov    r8, qword ptr [rax + 0x320]
001d7199: call   qword ptr [rax + 0x318]
001d719f: test   al, al
001d71a1: je     0x1801d7332
001d71a7: mov    rdx, qword ptr [rbx + 0x20]
001d71ab: jmp    0x1801d7327
001d71b0: cmp    dword ptr [rcx + 0xe0], 0
001d71b7: jne    0x1801d71c5
001d71b9: call   0x182f60cf0
001d71be: mov    rcx, qword ptr [rip + 0x3b186cb]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d71c5: mov    rax, qword ptr [rcx + 0xb8]
001d71cc: mov    rdx, qword ptr [rax + 8]
001d71d0: test   rdx, rdx
001d71d3: je     0x1801d7563
001d71d9: cmp    qword ptr [rdx + 0x40], 0
001d71de: jne    0x1801d72ac
001d71e4: xor    esi, esi
001d71e6: cmp    dword ptr [rcx + 0xe0], 0
001d71ed: jne    0x1801d71fb
001d71ef: call   0x182f60cf0
001d71f4: mov    rcx, qword ptr [rip + 0x3b18695]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d71fb: mov    rax, qword ptr [rcx + 0xb8]
001d7202: mov    rdi, qword ptr [rax + 8]
001d7206: test   rdi, rdi
001d7209: je     0x1801d7563
001d720f: mov    rdi, qword ptr [rdi + 0x38]
001d7213: test   rdi, rdi
001d7216: je     0x1801d7563
001d721c: cmp    esi, dword ptr [rdi + 0x18]
001d721f: jae    0x1801d7569
001d7225: mov    rdi, qword ptr [rdi + rsi*8 + 0x20]
001d722a: test   rdi, rdi
001d722d: je     0x1801d7563
001d7233: xor    edx, edx
001d7235: mov    rcx, rdi
001d7238: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
001d723d: test   al, al
001d723f: je     0x1801d7295
001d7241: cmp    byte ptr [rdi + 0x2c], 0
001d7245: je     0x1801d7250
001d7247: cmp    byte ptr [rbx + 0x9c], 0
001d724e: je     0x1801d7295
001d7250: cmp    byte ptr [rbx + 0x9b], 0
001d7257: jne    0x1801d7295
001d7259: cmp    qword ptr [rdi + 0x10], 0
001d725e: je     0x1801d726d
001d7260: mov    rax, qword ptr [rdi + 0x10]
001d7264: cmp    byte ptr [rax + 0xb1], 0
001d726b: jne    0x1801d7295
001d726d: mov    rax, qword ptr [rbx]
001d7270: mov    rdx, rdi
001d7273: mov    rcx, rbx
001d7276: mov    r8, qword ptr [rax + 0x320]
001d727d: call   qword ptr [rax + 0x318]
001d7283: test   al, al
001d7285: je     0x1801d7295
001d7287: xor    r8d, r8d
001d728a: mov    rdx, rdi
001d728d: mov    rcx, rbx
001d7290: call   0x1801d6b10                              ; BaseMenu$$ˁʴʵˁʴʳʺʲʿʶʽ
001d7295: inc    esi
001d7297: cmp    esi, 4
001d729a: jge    0x1801d7332
001d72a0: mov    rcx, qword ptr [rip + 0x3b185e9]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d72a7: jmp    0x1801d71e6
001d72ac: cmp    dword ptr [rcx + 0xe0], 0
001d72b3: jne    0x1801d72c1
001d72b5: call   0x182f60cf0
001d72ba: mov    rcx, qword ptr [rip + 0x3b185cf]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d72c1: mov    rax, qword ptr [rcx + 0xb8]
001d72c8: mov    rdx, qword ptr [rax + 8]
001d72cc: test   rdx, rdx
001d72cf: je     0x1801d7563
001d72d5: mov    rax, qword ptr [rbx]
001d72d8: mov    rcx, rbx
001d72db: mov    rdx, qword ptr [rdx + 0x40]
001d72df: mov    r8, qword ptr [rax + 0x320]
001d72e6: call   qword ptr [rax + 0x318]
001d72ec: test   al, al
001d72ee: je     0x1801d7332
001d72f0: mov    rax, qword ptr [rip + 0x3b18599]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d72f7: cmp    dword ptr [rax + 0xe0], 0
001d72fe: jne    0x1801d730f
001d7300: mov    rcx, rax
001d7303: call   0x182f60cf0
001d7308: mov    rax, qword ptr [rip + 0x3b18581]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
001d730f: mov    rax, qword ptr [rax + 0xb8]
001d7316: mov    rdx, qword ptr [rax + 8]
001d731a: test   rdx, rdx
001d731d: je     0x1801d7563
001d7323: mov    rdx, qword ptr [rdx + 0x40]
001d7327: xor    r8d, r8d
001d732a: mov    rcx, rbx
001d732d: call   0x1801d6b10                              ; BaseMenu$$ˁʴʵˁʴʳʺʲʿʶʽ
001d7332: cmp    byte ptr [rbx + 0x2e], 0
001d7336: je     0x1801d734f
001d7338: mov    rax, qword ptr [rbx]
001d733b: mov    rcx, rbx
001d733e: mov    byte ptr [rbx + 0x2e], 0
001d7342: mov    rdx, qword ptr [rax + 0x3f0]
001d7349: call   qword ptr [rax + 0x3e8]
001d734f: mov    rax, qword ptr [rip + 0x3ae40fa]         ; [0x3cbb450] meta:BaseMenu_TypeInfo
001d7356: cmp    dword ptr [rax + 0xe0], 0
001d735d: jne    0x1801d736e
001d735f: mov    rcx, rax
001d7362: call   0x182f60cf0
001d7367: mov    rax, qword ptr [rip + 0x3ae40e2]         ; [0x3cbb450] meta:BaseMenu_TypeInfo
001d736e: mov    rax, qword ptr [rax + 0xb8]
001d7375: cmp    byte ptr [rax + 1], 0
001d7379: je     0x1801d74c8
001d737f: xor    ecx, ecx
001d7381: movaps xmmword ptr [rsp + 0x90], xmm7
001d7389: call   0x182887aa0                              ; UnityEngine.Time$$get_deltaTime
001d738e: movaps xmm10, xmm0
001d7392: xorps  xmm7, xmm7
001d7395: mulss  xmm10, xmm7
001d739a: movaps xmm8, xmm0
001d739e: xor    r8d, r8d
001d73a1: mov    edx, 0xc8
001d73a6: xor    ecx, ecx
001d73a8: call   0x182882470                              ; UnityEngine.Random$$RandomRangeInt
001d73ad: movaps xmm9, xmm8
001d73b1: mulss  xmm8, xmm7
001d73b6: mulss  xmm9, dword ptr [rip + 0x2e8d911]        ; [0x3064cd0] flt=30.0 i=1106247680
001d73bf: movd   xmm0, eax
001d73c3: cvtdq2ps xmm0, xmm0
001d73c6: mulss  xmm9, xmm0
001d73cb: mulss  xmm10, xmm0
001d73d0: mulss  xmm8, xmm0
001d73d5: movss  xmm6, dword ptr [rip + 0x2e8c9df]        ; [0x3063dbc] flt=1.0 i=1065353216
001d73dd: xor    r8d, r8d
001d73e0: movaps xmm1, xmm6
001d73e3: xorps  xmm0, xmm0
001d73e6: call   0x1828824b0                              ; UnityEngine.Random$$Range
001d73eb: movaps xmm11, xmm0
001d73ef: xor    r8d, r8d
001d73f2: xorps  xmm0, xmm0
001d73f5: movaps xmm1, xmm6
001d73f8: call   0x1828824b0                              ; UnityEngine.Random$$Range
001d73fd: movaps xmm12, xmm0
001d7401: xor    r8d, r8d
001d7404: xorps  xmm0, xmm0
001d7407: movaps xmm1, xmm6
001d740a: call   0x1828824b0                              ; UnityEngine.Random$$Range
001d740f: mov    rbp, qword ptr [rbx + 0x88]
001d7416: movaps xmm6, xmm0
001d7419: movaps xmm7, xmmword ptr [rsp + 0x90]
001d7421: test   rbp, rbp
001d7424: je     0x1801d7563
001d742a: xor    esi, esi
001d742c: xor    eax, eax
001d742e: nop    
001d7430: cmp    eax, dword ptr [rbp + 0x18]
001d7433: jge    0x1801d74c8
001d7439: cmp    esi, dword ptr [rbp + 0x18]
001d743c: jae    0x1801d7569
001d7442: movsxd rax, esi
001d7445: mov    rdi, qword ptr [rbp + rax*8 + 0x20]
001d744a: test   rdi, rdi
001d744d: je     0x1801d7563
001d7453: xor    edx, edx
001d7455: mov    rcx, rdi
001d7458: call   0x1828186b0                              ; TMPro.TMP_Text$$get_rectTransform
001d745d: test   rax, rax
001d7460: je     0x1801d7563
001d7466: xor    r8d, r8d
001d7469: movss  dword ptr [rsp + 0x20], xmm9
001d7470: lea    rdx, [rsp + 0x20]
001d7475: movss  dword ptr [rsp + 0x24], xmm10
001d747c: mov    rcx, rax
001d747f: movss  dword ptr [rsp + 0x28], xmm8
001d7486: call   0x182888fe0                              ; UnityEngine.Transform$$Rotate
001d748b: mov    rax, qword ptr [rdi]
001d748e: lea    rdx, [rsp + 0x30]
001d7493: mov    rcx, rdi
001d7496: movss  dword ptr [rsp + 0x30], xmm11
001d749d: movss  dword ptr [rsp + 0x34], xmm12
001d74a4: movss  dword ptr [rsp + 0x38], xmm6
001d74aa: mov    r8, qword ptr [rax + 0x2b0]
001d74b1: mov    dword ptr [rsp + 0x3c], 0x3f800000
001d74b9: call   qword ptr [rax + 0x2a8]
001d74bf: inc    esi
001d74c1: mov    eax, esi
001d74c3: jmp    0x1801d7430
001d74c8: movss  xmm6, dword ptr [rbx + 0x100]
001d74d0: xor    ecx, ecx
001d74d2: call   0x182887c80                              ; UnityEngine.Time$$get_unscaledDeltaTime
001d74d7: subss  xmm6, xmm0
001d74db: xor    ecx, ecx
001d74dd: movss  dword ptr [rbx + 0x100], xmm6
001d74e5: movss  xmm6, dword ptr [rbx + 0x104]
001d74ed: call   0x182887c80                              ; UnityEngine.Time$$get_unscaledDeltaTime
001d74f2: subss  xmm6, xmm0
001d74f6: xor    ecx, ecx
001d74f8: movss  dword ptr [rbx + 0x104], xmm6
001d7500: movss  xmm6, dword ptr [rbx + 0x110]
001d7508: call   0x182887c80                              ; UnityEngine.Time$$get_unscaledDeltaTime
001d750d: subss  xmm6, xmm0
001d7511: movss  dword ptr [rbx + 0x110], xmm6
001d7519: movaps xmm12, xmmword ptr [rsp + 0x40]
001d751f: movaps xmm11, xmmword ptr [rsp + 0x50]
001d7525: movaps xmm10, xmmword ptr [rsp + 0x60]
001d752b: movaps xmm9, xmmword ptr [rsp + 0x70]
001d7531: movaps xmm8, xmmword ptr [rsp + 0x80]
001d753a: movaps xmm6, xmmword ptr [rsp + 0xa0]
001d7542: mov    rdi, qword ptr [rsp + 0xd0]
001d754a: mov    rsi, qword ptr [rsp + 0xc8]
001d7552: mov    rbp, qword ptr [rsp + 0xc0]
001d755a: add    rsp, 0xb0
001d7561: pop    rbx
001d7562: ret    
001d7563: call   0x182f60c50
001d7568: int3   
001d7569: call   0x182f60c40
001d756e: int3   
001d756f: int3   
