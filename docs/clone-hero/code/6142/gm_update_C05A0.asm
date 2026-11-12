000c05a0: push   rdi
000c05a2: sub    rsp, 0x60
000c05a6: cmp    byte ptr [rip + 0x3e4bbd2], 0            ; [0x3f0c17f] (bss)
000c05ad: mov    rdi, rcx
000c05b0: jne    0x1800c05f5
000c05b2: lea    rcx, [rip + 0x3bfb89f]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c05b9: call   0x182f609b0
000c05be: lea    rcx, [rip + 0x3c03253]                   ; [0x3cc3818] meta:CHNetManager_TypeInfo
000c05c5: call   0x182f609b0
000c05ca: lea    rcx, [rip + 0x3c27ad7]                   ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000c05d1: call   0x182f609b0
000c05d6: lea    rcx, [rip + 0x3c2f2b3]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c05dd: call   0x182f609b0
000c05e2: lea    rcx, [rip + 0x3bd9277]                   ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000c05e9: call   0x182f609b0
000c05ee: mov    byte ptr [rip + 0x3e4bb8a], 1            ; [0x3f0c17f] (bss)
000c05f5: cmp    byte ptr [rdi + 0x158], 0
000c05fc: je     0x1800c0bb1
000c0602: mov    qword ptr [rsp + 0x70], rbx
000c0607: mov    ebx, dword ptr [rdi + 0x194]
000c060d: xor    ecx, ecx
000c060f: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000c0614: cmp    ebx, eax
000c0616: jne    0x1800c0629
000c0618: mov    ebx, dword ptr [rdi + 0x198]
000c061e: xor    ecx, ecx
000c0620: call   0x182871890                              ; UnityEngine.Screen$$get_height
000c0625: cmp    ebx, eax
000c0627: je     0x1800c066c
000c0629: xor    ecx, ecx
000c062b: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000c0630: xor    ecx, ecx
000c0632: mov    dword ptr [rdi + 0x194], eax
000c0638: call   0x182871890                              ; UnityEngine.Screen$$get_height
000c063d: xor    ecx, ecx
000c063f: mov    dword ptr [rdi + 0x198], eax
000c0645: call   0x1828719e0                              ; UnityEngine.Screen$$get_width
000c064a: xor    ecx, ecx
000c064c: mov    ebx, eax
000c064e: call   0x182871890                              ; UnityEngine.Screen$$get_height
000c0653: movd   xmm1, ebx
000c0657: xor    r9d, r9d
000c065a: cvtdq2ps xmm1, xmm1
000c065d: mov    rcx, rdi
000c0660: movd   xmm2, eax
000c0664: cvtdq2ps xmm2, xmm2
000c0667: call   0x1800c02f0                              ; GameManager$$ˀˀʴʸʾʴʶʵʴʽʷ
000c066c: movsd  xmm0, qword ptr [rdi + 0x70]
000c0671: comisd xmm0, xmmword ptr [rdi + 0x178]
000c0679: mov    qword ptr [rsp + 0x78], rbp
000c067e: mov    qword ptr [rsp + 0x80], rsi
000c0686: movaps xmmword ptr [rsp + 0x50], xmm6
000c068b: movaps xmmword ptr [rsp + 0x40], xmm7
000c0690: movaps xmmword ptr [rsp + 0x30], xmm8
000c0696: movaps xmmword ptr [rsp + 0x20], xmm9
000c069c: jb     0x1800c06d8
000c069e: mov    rax, qword ptr [rip + 0x3bfb7b3]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c06a5: cmp    dword ptr [rax + 0xe0], 0
000c06ac: jne    0x1800c06bd
000c06ae: mov    rcx, rax
000c06b1: call   0x182f60cf0
000c06b6: mov    rax, qword ptr [rip + 0x3bfb79b]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c06bd: mov    rax, qword ptr [rax + 0xb8]
000c06c4: mov    rcx, qword ptr [rax + 8]
000c06c8: test   rcx, rcx
000c06cb: je     0x1800c0bbd
000c06d1: mov    byte ptr [rcx + 0x175], 1
000c06d8: cmp    byte ptr [rdi + 0x90], 0
000c06df: mov    byte ptr [rdi + 0xc0], 0
000c06e6: je     0x1800c0711
000c06e8: mov    rax, qword ptr [rip + 0x3c03129]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000c06ef: mov    rcx, qword ptr [rax + 0xb8]
000c06f6: mov    rcx, qword ptr [rcx]
000c06f9: test   rcx, rcx
000c06fc: je     0x1800c0bbd
000c0702: xor    edx, edx
000c0704: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000c0709: test   al, al
000c070b: je     0x1800c0b89
000c0711: mov    rcx, qword ptr [rip + 0x3bfb740]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c0718: cmp    dword ptr [rcx + 0xe0], 0
000c071f: jne    0x1800c072d
000c0721: call   0x182f60cf0
000c0726: mov    rcx, qword ptr [rip + 0x3bfb72b]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c072d: mov    rax, qword ptr [rcx + 0xb8]
000c0734: mov    rdx, qword ptr [rax + 8]
000c0738: test   rdx, rdx
000c073b: je     0x1800c0bbd
000c0741: cmp    byte ptr [rdx + 0x175], 0
000c0748: jne    0x1800c0827
000c074e: cmp    dword ptr [rcx + 0xe0], 0
000c0755: jne    0x1800c0763
000c0757: call   0x182f60cf0
000c075c: mov    rcx, qword ptr [rip + 0x3bfb6f5]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c0763: mov    rax, qword ptr [rcx + 0xb8]
000c076a: mov    rcx, qword ptr [rax + 8]
000c076e: test   rcx, rcx
000c0771: je     0x1800c0bbd
000c0777: cmp    byte ptr [rcx + 0x174], 0
000c077e: je     0x1800c0827
000c0784: mov    rax, qword ptr [rip + 0x3c0308d]         ; [0x3cc3818] meta:CHNetManager_TypeInfo
000c078b: mov    rcx, qword ptr [rax + 0xb8]
000c0792: mov    rcx, qword ptr [rcx]
000c0795: test   rcx, rcx
000c0798: je     0x1800c0bbd
000c079e: xor    edx, edx
000c07a0: call   0x180145fe0                              ; CHNetManager$$ʵʵʸʿʹʷʹʸʺʷʹ
000c07a5: test   al, al
000c07a7: je     0x1800c081a
000c07a9: mov    rcx, qword ptr [rdi + 0xb0]
000c07b0: test   rcx, rcx
000c07b3: je     0x1800c0bbd
000c07b9: xor    edx, edx
000c07bb: call   0x18016c910                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ˀʽˁˁʸʵʲʻʺʷʻ
000c07c0: test   al, al
000c07c2: je     0x1800c081a
000c07c4: mov    rcx, qword ptr [rdi + 0xa0]
000c07cb: test   rcx, rcx
000c07ce: je     0x1800c0bbd
000c07d4: xor    r8d, r8d
000c07d7: mov    dl, 1
000c07d9: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
000c07de: mov    rax, qword ptr [rip + 0x3bfb673]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c07e5: cmp    dword ptr [rax + 0xe0], 0
000c07ec: jne    0x1800c07fd
000c07ee: mov    rcx, rax
000c07f1: call   0x182f60cf0
000c07f6: mov    rax, qword ptr [rip + 0x3bfb65b]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
000c07fd: mov    rax, qword ptr [rax + 0xb8]
000c0804: mov    rcx, qword ptr [rax + 8]
000c0808: test   rcx, rcx
000c080b: je     0x1800c0bbd
000c0811: xor    edx, edx
000c0813: call   0x18036b6f0                              ; BassAudioManager$$ʽˁʵʵʷˀʾʴʿʷʿ
000c0818: jmp    0x1800c0827
000c081a: xor    r8d, r8d
000c081d: xor    edx, edx
000c081f: mov    rcx, rdi
000c0822: call   0x1800b9680                              ; GameManager$$ʷʻʶˀʾʵʸˀʳˀʽ
000c0827: mov    rcx, qword ptr [rdi + 0xb0]
000c082e: test   rcx, rcx
000c0831: je     0x1800c0bbd
000c0837: xor    edx, edx
000c0839: call   0x18016c650                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʸʶˁʷˁʼʳʶʴʸ
000c083e: mov    rcx, qword ptr [rdi + 0xb0]
000c0845: mov    rax, qword ptr [rdi + 0x70]
000c0849: mov    qword ptr [rdi + 0x80], rax
000c0850: test   rcx, rcx
000c0853: je     0x1800c0bbd
000c0859: xor    edx, edx
000c085b: call   0x18016c480                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʵˁʶʹʴʾʸˁʾˀʹ
000c0860: mov    rcx, qword ptr [rdi + 0xb0]
000c0867: movsd  qword ptr [rdi + 0x70], xmm0
000c086c: test   rcx, rcx
000c086f: je     0x1800c0bbd
000c0875: xor    edx, edx
000c0877: call   0x18016c490                              ; ʴʶʺˀˀʺʹʴʿʹʷ$$ʿʳʶʿʹˁʴʻʺʶʹ
000c087c: mov    rcx, qword ptr [rdi + 0x20]
000c0880: movsd  qword ptr [rdi + 0x78], xmm0
000c0885: test   rcx, rcx
000c0888: je     0x1800c0bbd
000c088e: xor    edx, edx
000c0890: call   0x180356690                              ; NoteAnimator$$ʹʼʻˀʲʴʻʺʵʾʼ
000c0895: movsd  xmm0, qword ptr [rdi + 0x70]
000c089a: comisd xmm0, xmmword ptr [rdi + 0x88]
000c08a2: jbe    0x1800c08d8
000c08a4: mov    rax, qword ptr [rdi + 0x70]
000c08a8: movaps xmm6, xmm0
000c08ab: mov    qword ptr [rdi + 0x88], rax
000c08b2: mov    byte ptr [rdi + 0xc0], 1
000c08b9: mov    rcx, qword ptr [rip + 0x3bd8fa0]         ; [0x3c99860] meta:ʻʳʿʼʵʷʺʹˀʸʾ_TypeInfo
000c08c0: cmp    dword ptr [rcx + 0xe0], 0
000c08c7: jne    0x1800c08ce
000c08c9: call   0x182f60cf0
000c08ce: xor    edx, edx
000c08d0: movaps xmm0, xmm6
000c08d3: call   0x18022b760                              ; ʻʳʿʼʵʷʺʹˀʸʾ$$ʴʵʾˀʾʺˀˁʽʵʹ
000c08d8: movsd  xmm0, qword ptr [rdi + 0x70]
000c08dd: comisd xmm0, xmmword ptr [rdi + 0x50]
000c08e2: jb     0x1800c095b
000c08e4: mov    rax, qword ptr [rip + 0x3c2efa5]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c08eb: cmp    dword ptr [rax + 0xe0], 0
000c08f2: jne    0x1800c0903
000c08f4: mov    rcx, rax
000c08f7: call   0x182f60cf0
000c08fc: mov    rax, qword ptr [rip + 0x3c2ef8d]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c0903: mov    rax, qword ptr [rax + 0xb8]
000c090a: mov    rcx, qword ptr [rax + 8]
000c090e: test   rcx, rcx
000c0911: je     0x1800c0bbd
000c0917: cmp    byte ptr [rcx + 0x72], 0
000c091b: jne    0x1800c094e
000c091d: mov    rax, qword ptr [rip + 0x3c27784]         ; [0x3ce80a8] meta:FadeBehaviour_TypeInfo
000c0924: mov    rcx, qword ptr [rax + 0xb8]
000c092b: mov    rcx, qword ptr [rcx]
000c092e: test   rcx, rcx
000c0931: je     0x1800c0bbd
000c0937: xor    edx, edx
000c0939: call   0x180258820                              ; FadeBehaviour$$ʾʽʻʷʺʺʺʳʴʵʸ
000c093e: test   al, al
000c0940: jne    0x1800c095b
000c0942: xor    edx, edx
000c0944: mov    rcx, rdi
000c0947: call   0x1800bd640                              ; GameManager$$ʼʼʹʿʿˀˁʵʹʾʻ
000c094c: jmp    0x1800c095b
000c094e: xor    r8d, r8d
000c0951: mov    dl, 1
000c0953: mov    rcx, rdi
000c0956: call   0x1800bf510                              ; GameManager$$ʾʽʽʹʽʿʳʾʽʻʼ
000c095b: mov    rax, qword ptr [rdi + 0x28]
000c095f: test   rax, rax
000c0962: je     0x1800c0bbd
000c0968: cmp    qword ptr [rax + 0x40], 0
000c096d: je     0x1800c09ad
000c096f: mov    rcx, qword ptr [rax + 0x40]
000c0973: mov    rcx, qword ptr [rcx + 0x18]
000c0977: test   rcx, rcx
000c097a: je     0x1800c0bbd
000c0980: xor    r8d, r8d
000c0983: mov    edx, 0x10
000c0988: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000c098d: test   al, al
000c098f: je     0x1800c09ad
000c0991: mov    rdx, qword ptr [rdi + 0x28]
000c0995: test   rdx, rdx
000c0998: je     0x1800c0bbd
000c099e: mov    rdx, qword ptr [rdx + 0x40]
000c09a2: xor    r8d, r8d
000c09a5: mov    rcx, rdi
000c09a8: call   0x1800b9680                              ; GameManager$$ʷʻʶˀʾʵʸˀʳˀʽ
000c09ad: xor    edx, edx
000c09af: mov    rcx, rdi
000c09b2: call   0x1800b5c70                              ; GameManager$$ʴʳʾʵʴʻʺʸʵʿʿ
000c09b7: mov    rcx, qword ptr [rip + 0x3c2eed2]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c09be: cmp    dword ptr [rcx + 0xe0], 0
000c09c5: jne    0x1800c09d3
000c09c7: call   0x182f60cf0
000c09cc: mov    rcx, qword ptr [rip + 0x3c2eebd]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c09d3: mov    rax, qword ptr [rcx + 0xb8]
000c09da: mov    rdx, qword ptr [rax + 8]
000c09de: test   rdx, rdx
000c09e1: je     0x1800c0bbd
000c09e7: cmp    byte ptr [rdx + 0x72], 0
000c09eb: je     0x1800c0b89
000c09f1: cmp    dword ptr [rcx + 0xe0], 0
000c09f8: jne    0x1800c0a06
000c09fa: call   0x182f60cf0
000c09ff: mov    rcx, qword ptr [rip + 0x3c2ee8a]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000c0a06: mov    rax, qword ptr [rcx + 0xb8]
000c0a0d: mov    rcx, qword ptr [rax + 8]
000c0a11: test   rcx, rcx
000c0a14: je     0x1800c0bbd
000c0a1a: mov    rbp, qword ptr [rcx + 0x38]
000c0a1e: test   rbp, rbp
000c0a21: je     0x1800c0bbd
000c0a27: movss  xmm8, dword ptr [rip + 0x2fa3804]        ; [0x3064234] flt=-90.0 i=3266576384
000c0a30: xor    esi, esi
000c0a32: movss  xmm6, dword ptr [rip + 0x2fa3426]        ; [0x3063e60] flt=0.05000000074505806 i=1028443341
000c0a3a: xor    eax, eax
000c0a3c: movss  xmm7, dword ptr [rip + 0x2fa3594]        ; [0x3063fd8] flt=5.0 i=1084227584
000c0a44: movss  xmm9, dword ptr [rip + 0x2fa37d3]        ; [0x3064220] flt=4900.0 i=1167663104
000c0a4d: nop    dword ptr [rax]
000c0a50: cmp    eax, dword ptr [rbp + 0x18]
000c0a53: jge    0x1800c0b89
000c0a59: cmp    esi, dword ptr [rbp + 0x18]
000c0a5c: jae    0x1800c0bb7
000c0a62: movsxd rax, esi
000c0a65: mov    rbx, qword ptr [rbp + rax*8 + 0x20]
000c0a6a: test   rbx, rbx
000c0a6d: je     0x1800c0bbd
000c0a73: xor    edx, edx
000c0a75: mov    rcx, rbx
000c0a78: call   0x180337540                              ; ˁʻʽʷʽʻʾʵʷˀʶ$$ʳʷʻʿʻʲʿʾʵʷʶ
000c0a7d: test   al, al
000c0a7f: je     0x1800c0b80
000c0a85: mov    rcx, qword ptr [rbx + 0x18]
000c0a89: test   rcx, rcx
000c0a8c: je     0x1800c0bbd
000c0a92: xor    r8d, r8d
000c0a95: mov    edx, 6
000c0a9a: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000c0a9f: test   al, al
000c0aa1: je     0x1800c0ab0
000c0aa3: xor    r8d, r8d
000c0aa6: mov    dl, 1
000c0aa8: mov    rcx, rdi
000c0aab: call   0x1800bf510                              ; GameManager$$ʾʽʽʹʽʿʳʾʽʻʼ
000c0ab0: mov    rcx, qword ptr [rbx + 0x18]
000c0ab4: test   rcx, rcx
000c0ab7: je     0x1800c0bbd
000c0abd: xor    r8d, r8d
000c0ac0: mov    edx, 0x1f
000c0ac5: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000c0aca: test   al, al
000c0acc: jne    0x1800c0b18
000c0ace: mov    rcx, qword ptr [rbx + 0x18]
000c0ad2: test   rcx, rcx
000c0ad5: je     0x1800c0bbd
000c0adb: xor    r8d, r8d
000c0ade: mov    edx, 0x20
000c0ae3: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
000c0ae8: test   al, al
000c0aea: je     0x1800c0b80
000c0af0: comiss xmm8, dword ptr [rdi + 0xdc]
000c0af8: jae    0x1800c0b89
000c0afe: movss  xmm1, dword ptr [rdi + 0xd8]
000c0b06: movss  xmm0, dword ptr [rdi + 0xdc]
000c0b0e: subss  xmm1, xmm6
000c0b12: subss  xmm0, xmm7
000c0b16: jmp    0x1800c0b36
000c0b18: movss  xmm0, dword ptr [rdi + 0xdc]
000c0b20: comiss xmm0, xmm9
000c0b24: jae    0x1800c0b89
000c0b26: movss  xmm1, dword ptr [rdi + 0xd8]
000c0b2e: addss  xmm0, xmm7
000c0b32: addss  xmm1, xmm6
000c0b36: mov    rax, qword ptr [rdi + 0xb0]
000c0b3d: movss  dword ptr [rdi + 0xdc], xmm0
000c0b45: movss  dword ptr [rdi + 0xd8], xmm1
000c0b4d: test   rax, rax
000c0b50: je     0x1800c0bbd
000c0b52: xor    edx, edx
000c0b54: movss  dword ptr [rax + 0x30], xmm1
000c0b59: mov    rcx, rdi
000c0b5c: call   0x1800ba640                              ; GameManager$$ʸʿʽˁʸʹˀʵʺʿʵ
000c0b61: mov    rcx, qword ptr [rdi + 0x30]
000c0b65: test   rcx, rcx
000c0b68: je     0x1800c0bbd
000c0b6a: cvttss2si edx, dword ptr [rdi + 0xdc]
000c0b72: xor    r9d, r9d
000c0b75: xor    r8d, r8d
000c0b78: add    edx, 0x64
000c0b7b: call   0x18013b870                              ; PracticeUI$$ʿʸʳʻʶˁʹʿʹˁʽ
000c0b80: inc    esi
000c0b82: mov    eax, esi
000c0b84: jmp    0x1800c0a50
000c0b89: movaps xmm8, xmmword ptr [rsp + 0x30]
000c0b8f: movaps xmm7, xmmword ptr [rsp + 0x40]
000c0b94: movaps xmm6, xmmword ptr [rsp + 0x50]
000c0b99: mov    rsi, qword ptr [rsp + 0x80]
000c0ba1: mov    rbp, qword ptr [rsp + 0x78]
000c0ba6: movaps xmm9, xmmword ptr [rsp + 0x20]
000c0bac: mov    rbx, qword ptr [rsp + 0x70]
000c0bb1: add    rsp, 0x60
000c0bb5: pop    rdi
000c0bb6: ret    
000c0bb7: call   0x182f60c40
000c0bbc: int3   
000c0bbd: call   0x182f60c50
000c0bc2: int3   
000c0bc3: int3   
