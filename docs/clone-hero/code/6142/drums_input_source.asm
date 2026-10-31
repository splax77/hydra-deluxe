0033a880: mov    r11, rsp
0033a883: mov    qword ptr [r11 + 0x18], r8
0033a887: push   rbx
0033a888: push   r13
0033a88a: sub    rsp, 0x98
0033a891: xor    eax, eax
0033a893: mov    rbx, r8
0033a896: mov    r13, rcx
0033a899: mov    qword ptr [r11 - 0x50], rax
0033a89d: mov    dword ptr [rsp + 0x60], eax
0033a8a1: cmp    byte ptr [rcx + 0xc0], al
0033a8a7: je     0x18033ac07
0033a8ad: mov    qword ptr [r11 + 0x10], rbp
0033a8b1: mov    qword ptr [r11 - 0x18], rsi
0033a8b5: mov    qword ptr [r11 - 0x20], rdi
0033a8b9: mov    qword ptr [r11 - 0x28], r12
0033a8bd: mov    qword ptr [r11 - 0x30], r14
0033a8c1: mov    qword ptr [r11 - 0x38], r15
0033a8c5: mov    rax, qword ptr [rcx + 0x88]
0033a8cc: test   rax, rax
0033a8cf: je     0x18033b030
0033a8d5: mov    rcx, qword ptr [rax + 0x10]
0033a8d9: test   rcx, rcx
0033a8dc: je     0x18033b030
0033a8e2: cmp    byte ptr [rcx + 0xb1], 0
0033a8e9: jne    0x18033abdd
0033a8ef: mov    rcx, qword ptr [rax + 0x18]
0033a8f3: test   rcx, rcx
0033a8f6: je     0x18033b030
0033a8fc: xor    r8d, r8d
0033a8ff: mov    edx, 4
0033a904: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033a909: mov    rcx, qword ptr [r13 + 0x88]
0033a910: movzx  edi, al
0033a913: test   rcx, rcx
0033a916: je     0x18033b030
0033a91c: mov    rcx, qword ptr [rcx + 0x18]
0033a920: test   rcx, rcx
0033a923: je     0x18033b030
0033a929: xor    r8d, r8d
0033a92c: mov    edx, 0x18
0033a931: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033a936: or     al, dil
0033a939: xor    r8d, r8d
0033a93c: movzx  edx, al
0033a93f: mov    rcx, rbx
0033a942: call   0x1821767d0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʻʷʼʿʲʿˁʻʼʶˀ
0033a947: xor    edx, edx
0033a949: mov    rcx, rbx
0033a94c: call   0x1821766a0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʹʻʺʺʲʼʲʿʽʳʸ
0033a951: test   al, al
0033a953: je     0x18033a963
0033a955: lea    eax, [rdi*2 - 1]
0033a95c: mov    dword ptr [r13 + 0x228], eax
0033a963: cmp    byte ptr [r13 + 0x218], 0
0033a96b: mov    rax, qword ptr [r13 + 0x88]
0033a972: jne    0x18033ac12
0033a978: test   rax, rax
0033a97b: je     0x18033b036
0033a981: mov    rcx, qword ptr [rax + 0x18]
0033a985: test   rcx, rcx
0033a988: je     0x18033b036
0033a98e: mov    ebp, 1
0033a993: xor    r8d, r8d
0033a996: mov    edx, ebp
0033a998: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033a99d: xor    r8d, r8d
0033a9a0: movzx  edx, al
0033a9a3: mov    rcx, rbx
0033a9a6: call   0x182176760                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺˀʺʺʳʳʿʶʽʹʻ
0033a9ab: mov    rcx, qword ptr [r13 + 0x88]
0033a9b2: test   rcx, rcx
0033a9b5: je     0x18033b036
0033a9bb: mov    rcx, qword ptr [rcx + 0x18]
0033a9bf: test   rcx, rcx
0033a9c2: je     0x18033b036
0033a9c8: xor    r8d, r8d
0033a9cb: mov    edx, 2
0033a9d0: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033a9d5: xor    r8d, r8d
0033a9d8: movzx  edx, al
0033a9db: mov    rcx, rbx
0033a9de: call   0x182176900                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʸʴʿʶʹʻʲʶʷʸ
0033a9e3: mov    rcx, qword ptr [r13 + 0x88]
0033a9ea: test   rcx, rcx
0033a9ed: je     0x18033b036
0033a9f3: mov    rcx, qword ptr [rcx + 0x18]
0033a9f7: test   rcx, rcx
0033a9fa: je     0x18033b036
0033aa00: xor    r8d, r8d
0033aa03: mov    edx, 3
0033aa08: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033aa0d: xor    r8d, r8d
0033aa10: movzx  edx, al
0033aa13: mov    rcx, rbx
0033aa16: call   0x182176e00                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ˁʴʹʸʳʽʹʺʻʲʽ
0033aa1b: mov    rcx, qword ptr [r13 + 0x88]
0033aa22: test   rcx, rcx
0033aa25: je     0x18033b036
0033aa2b: mov    rcx, qword ptr [rcx + 0x18]
0033aa2f: test   rcx, rcx
0033aa32: je     0x18033b036
0033aa38: xor    r8d, r8d
0033aa3b: xor    edx, edx
0033aa3d: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033aa42: xor    r8d, r8d
0033aa45: movzx  edx, al
0033aa48: mov    rcx, rbx
0033aa4b: call   0x182176890                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʴʴʷʺʻʿʵʴʻˁ
0033aa50: mov    rcx, qword ptr [r13 + 0x88]
0033aa57: test   rcx, rcx
0033aa5a: je     0x18033b036
0033aa60: mov    rcx, qword ptr [rcx + 0x18]
0033aa64: test   rcx, rcx
0033aa67: je     0x18033b036
0033aa6d: xor    r8d, r8d
0033aa70: mov    edx, 0x25
0033aa75: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033aa7a: xor    r8d, r8d
0033aa7d: movzx  edx, al
0033aa80: mov    rcx, rbx
0033aa83: call   0x182176970                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʿʼʼʵʷˁʵʹʻʻ
0033aa88: mov    rcx, qword ptr [r13 + 0x88]
0033aa8f: test   rcx, rcx
0033aa92: je     0x18033b036
0033aa98: mov    rcx, qword ptr [rcx + 0x18]
0033aa9c: test   rcx, rcx
0033aa9f: je     0x18033b036
0033aaa5: xor    r8d, r8d
0033aaa8: mov    edx, 0x26
0033aaad: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033aab2: xor    r8d, r8d
0033aab5: movzx  edx, al
0033aab8: mov    rcx, rbx
0033aabb: call   0x1821766f0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺʸʷʸʻʹʻˁˀˀʶ
0033aac0: mov    rax, qword ptr [r13 + 0x88]
0033aac7: test   rax, rax
0033aaca: je     0x18033b036
0033aad0: mov    rcx, qword ptr [rax + 0x18]
0033aad4: test   rcx, rcx
0033aad7: je     0x18033b036
0033aadd: xor    r8d, r8d
0033aae0: mov    edx, 0x27
0033aae5: call   0x181f39540                              ; Rewired.Player$$GetButtonDown
0033aaea: xor    r8d, r8d
0033aaed: movzx  edx, al
0033aaf0: mov    rcx, rbx
0033aaf3: call   0x182175fb0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʲʴʳʽʶʾˁʵʳʴʾ
0033aaf8: xor    r8d, r8d
0033aafb: mov    edx, ebp
0033aafd: mov    rcx, r13
0033ab00: call   0x180337e10
0033ab05: xor    r8d, r8d
0033ab08: mov    edx, 2
0033ab0d: mov    rcx, r13
0033ab10: movzx  r12d, al
0033ab14: call   0x180337e10
0033ab19: xor    r8d, r8d
0033ab1c: mov    edx, 3
0033ab21: mov    rcx, r13
0033ab24: movzx  r15d, al
0033ab28: call   0x180337e10
0033ab2d: xor    r8d, r8d
0033ab30: xor    edx, edx
0033ab32: mov    rcx, r13
0033ab35: movzx  r14d, al
0033ab39: call   0x180337e10
0033ab3e: xor    r8d, r8d
0033ab41: mov    edx, 0x25
0033ab46: mov    rcx, r13
0033ab49: movzx  ebp, al
0033ab4c: call   0x180337e10
0033ab51: xor    r8d, r8d
0033ab54: mov    edx, 0x26
0033ab59: mov    rcx, r13
0033ab5c: movzx  esi, al
0033ab5f: call   0x180337e10
0033ab64: xor    r8d, r8d
0033ab67: mov    edx, 0x27
0033ab6c: mov    rcx, r13
0033ab6f: movzx  edi, al
0033ab72: call   0x180337e10
0033ab77: xor    r8d, r8d
0033ab7a: mov    edx, 4
0033ab7f: mov    rcx, r13
0033ab82: movzx  ebx, al
0033ab85: call   0x180337e10
0033ab8a: xor    ecx, ecx
0033ab8c: movzx  r9d, r14b
0033ab90: mov    qword ptr [rsp + 0x48], rcx
0033ab95: movzx  r8d, r15b
0033ab99: mov    byte ptr [rsp + 0x40], al
0033ab9d: movzx  edx, r12b
0033aba1: mov    byte ptr [rsp + 0x38], bl
0033aba5: mov    byte ptr [rsp + 0x30], dil
0033abaa: mov    qword ptr [rsp + 0xb0], rcx
0033abb2: lea    rcx, [rsp + 0xb0]
0033abba: mov    byte ptr [rsp + 0x28], sil
0033abbf: mov    byte ptr [rsp + 0x20], bpl
0033abc4: call   0x1821770c0                              ; ʾʿʴʹʻʹʸʳʼʲʵ$$.ctor
0033abc9: mov    rcx, qword ptr [rsp + 0xc0]
0033abd1: mov    rax, qword ptr [rsp + 0xb0]
0033abd9: mov    qword ptr [rcx + 4], rax
0033abdd: mov    r14, qword ptr [rsp + 0x78]
0033abe2: mov    r12, qword ptr [rsp + 0x80]
0033abea: mov    rdi, qword ptr [rsp + 0x88]
0033abf2: mov    rsi, qword ptr [rsp + 0x90]
0033abfa: mov    rbp, qword ptr [rsp + 0xb8]
0033ac02: mov    r15, qword ptr [rsp + 0x70]
0033ac07: add    rsp, 0x98
0033ac0e: pop    r13
0033ac10: pop    rbx
0033ac11: ret    
0033ac12: test   rax, rax
0033ac15: je     0x18033b030
0033ac1b: mov    rcx, qword ptr [rax + 0x18]
0033ac1f: test   rcx, rcx
0033ac22: je     0x18033b030
0033ac28: xor    r8d, r8d
0033ac2b: mov    edx, 3
0033ac30: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ac35: mov    rcx, qword ptr [r13 + 0x88]
0033ac3c: mov    byte ptr [rsp + 0x51], al
0033ac40: test   rcx, rcx
0033ac43: je     0x18033b030
0033ac49: mov    rcx, qword ptr [rcx + 0x18]
0033ac4d: test   rcx, rcx
0033ac50: je     0x18033b030
0033ac56: xor    r8d, r8d
0033ac59: mov    edx, 2
0033ac5e: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ac63: mov    rcx, qword ptr [r13 + 0x88]
0033ac6a: mov    byte ptr [rsp + 0x50], al
0033ac6e: test   rcx, rcx
0033ac71: je     0x18033b030
0033ac77: mov    rcx, qword ptr [rcx + 0x18]
0033ac7b: test   rcx, rcx
0033ac7e: je     0x18033b030
0033ac84: xor    r8d, r8d
0033ac87: xor    edx, edx
0033ac89: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ac8e: mov    rcx, qword ptr [r13 + 0x88]
0033ac95: movzx  r12d, al
0033ac99: test   rcx, rcx
0033ac9c: je     0x18033b030
0033aca2: mov    rcx, qword ptr [rcx + 0x18]
0033aca6: test   rcx, rcx
0033aca9: je     0x18033b030
0033acaf: mov    ebp, 1
0033acb4: xor    r8d, r8d
0033acb7: mov    edx, ebp
0033acb9: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033acbe: mov    rcx, qword ptr [r13 + 0x88]
0033acc5: mov    byte ptr [rsp + 0xb0], al
0033accc: test   rcx, rcx
0033accf: je     0x18033b030
0033acd5: mov    rcx, qword ptr [rcx + 0x18]
0033acd9: test   rcx, rcx
0033acdc: je     0x18033b030
0033ace2: xor    r8d, r8d
0033ace5: mov    edx, 0x28
0033acea: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033acef: mov    rcx, qword ptr [r13 + 0x88]
0033acf6: movzx  edi, al
0033acf9: test   rcx, rcx
0033acfc: je     0x18033b030
0033ad02: mov    rcx, qword ptr [rcx + 0x18]
0033ad06: test   rcx, rcx
0033ad09: je     0x18033b030
0033ad0f: xor    r8d, r8d
0033ad12: mov    edx, 0x29
0033ad17: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ad1c: mov    rcx, qword ptr [r13 + 0x88]
0033ad23: movzx  r14d, al
0033ad27: test   rcx, rcx
0033ad2a: je     0x18033b030
0033ad30: mov    rcx, qword ptr [rcx + 0x18]
0033ad34: test   rcx, rcx
0033ad37: je     0x18033b030
0033ad3d: xor    r8d, r8d
0033ad40: mov    edx, 0x2a
0033ad45: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ad4a: movzx  r15d, al
0033ad4e: mov    rax, qword ptr [r13 + 0x88]
0033ad55: test   rax, rax
0033ad58: je     0x18033b030
0033ad5e: mov    rcx, qword ptr [rax + 0x18]
0033ad62: test   rcx, rcx
0033ad65: je     0x18033b030
0033ad6b: xor    r8d, r8d
0033ad6e: mov    edx, 0x2b
0033ad73: call   0x181f3b730                              ; Rewired.Player$$GetButton
0033ad78: movzx  esi, al
0033ad7b: test   dil, r14b
0033ad7e: je     0x18033ae09
0033ad84: movzx  eax, byte ptr [rsp + 0x51]
0033ad89: mov    edx, esi
0033ad8b: movzx  r8d, byte ptr [rsp + 0xb0]
0033ad94: or     edx, eax
0033ad96: movzx  eax, byte ptr [rsp + 0x50]
0033ad9b: mov    ecx, r15d
0033ad9e: or     ecx, eax
0033ada0: add    ecx, edx
0033ada2: add    ecx, r8d
0033ada5: test   r12b, r12b
0033ada8: jne    0x18033adb5
0033adaa: movzx  eax, sil
0033adae: or     al, r15b
0033adb1: je     0x18033adb5
0033adb3: xor    ebp, ebp
0033adb5: lea    eax, [rcx + rbp]
0033adb8: cmp    eax, 1
0033adbb: jbe    0x18033ae12
0033adbd: test   r15b, r15b
0033adc0: je     0x18033add7
0033adc2: xor    r8d, r8d
0033adc5: mov    dl, 1
0033adc7: mov    rcx, rbx
0033adca: call   0x182176970                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʿʼʼʵʷˁʵʹʻʻ
0033adcf: xor    r14b, r14b
0033add2: mov    byte ptr [rsp + 0x50], 0
0033add7: test   sil, sil
0033adda: je     0x18033adf1
0033addc: xor    r8d, r8d
0033addf: mov    dl, 1
0033ade1: mov    rcx, rbx
0033ade4: call   0x1821766f0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺʸʷʸʻʹʻˁˀˀʶ
0033ade9: xor    r14b, r14b
0033adec: mov    byte ptr [rsp + 0x51], 0
0033adf1: or     sil, r15b
0033adf4: jne    0x18033ae09
0033adf6: xor    r8d, r8d
0033adf9: mov    dl, 1
0033adfb: mov    rcx, rbx
0033adfe: call   0x182175fb0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʲʴʳʽʶʾˁʵʳʴʾ
0033ae03: xor    r12b, r12b
0033ae06: xor    r14b, r14b
0033ae09: movzx  r8d, byte ptr [rsp + 0xb0]
0033ae12: test   dil, dil
0033ae15: je     0x18033ae6b
0033ae17: test   r8b, r8b
0033ae1a: je     0x18033ae29
0033ae1c: xor    r8d, r8d
0033ae1f: mov    dl, 1
0033ae21: mov    rcx, rbx
0033ae24: call   0x182176760                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺˀʺʺʳʳʿʶʽʹʻ
0033ae29: movzx  edi, byte ptr [rsp + 0x50]
0033ae2e: test   dil, dil
0033ae31: je     0x18033ae40
0033ae33: xor    r8d, r8d
0033ae36: mov    dl, 1
0033ae38: mov    rcx, rbx
0033ae3b: call   0x182176900                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʸʴʿʶʹʻʲʶʷʸ
0033ae40: movzx  esi, byte ptr [rsp + 0x51]
0033ae45: test   sil, sil
0033ae48: je     0x18033ae57
0033ae4a: xor    r8d, r8d
0033ae4d: mov    dl, 1
0033ae4f: mov    rcx, rbx
0033ae52: call   0x182176e00                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ˁʴʹʸʳʽʹʺʻʲʽ
0033ae57: test   r12b, r12b
0033ae5a: je     0x18033ae75
0033ae5c: xor    r8d, r8d
0033ae5f: mov    dl, 1
0033ae61: mov    rcx, rbx
0033ae64: call   0x182176890                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʴʴʷʺʻʿʵʴʻˁ
0033ae69: jmp    0x18033ae75
0033ae6b: movzx  edi, byte ptr [rsp + 0x50]
0033ae70: movzx  esi, byte ptr [rsp + 0x51]
0033ae75: test   r14b, r14b
0033ae78: je     0x18033aeb0
0033ae7a: test   dil, dil
0033ae7d: je     0x18033ae8c
0033ae7f: xor    r8d, r8d
0033ae82: mov    dl, 1
0033ae84: mov    rcx, rbx
0033ae87: call   0x182176970                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʿʼʼʵʷˁʵʹʻʻ
0033ae8c: test   sil, sil
0033ae8f: je     0x18033ae9e
0033ae91: xor    r8d, r8d
0033ae94: mov    dl, 1
0033ae96: mov    rcx, rbx
0033ae99: call   0x1821766f0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺʸʷʸʻʹʻˁˀˀʶ
0033ae9e: test   r12b, r12b
0033aea1: je     0x18033aeb0
0033aea3: xor    r8d, r8d
0033aea6: mov    dl, 1
0033aea8: mov    rcx, rbx
0033aeab: call   0x182175fb0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʲʴʳʽʶʾˁʵʳʴʾ
0033aeb0: movsd  xmm0, qword ptr [rbx]
0033aeb4: lea    rcx, [rsp + 0x58]
0033aeb9: mov    eax, dword ptr [rbx + 8]
0033aebc: xor    edx, edx
0033aebe: movsd  qword ptr [rsp + 0x58], xmm0
0033aec4: mov    dword ptr [rsp + 0x60], eax
0033aec8: call   0x182176600                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷˀʽʺʷˁʷʿˀʴʾ
0033aecd: test   al, al
0033aecf: jne    0x18033aed3
0033aed1: jmp    0x18033aee3
0033aed3: lea    rcx, [r13 + 0x21c]
0033aeda: xor    edx, edx
0033aedc: call   0x182176600                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷˀʽʺʷˁʷʿˀʴʾ
0033aee1: xor    al, 1
0033aee3: xor    r8d, r8d
0033aee6: movzx  edx, al
0033aee9: mov    rcx, rbx
0033aeec: call   0x182176760                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺˀʺʺʳʳʿʶʽʹʻ
0033aef1: xor    edx, edx
0033aef3: lea    rcx, [rsp + 0x58]
0033aef8: call   0x182176a30                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʻʲʿˀʻʿʸʹˀʶ
0033aefd: test   al, al
0033aeff: jne    0x18033af03
0033af01: jmp    0x18033af13
0033af03: lea    rcx, [r13 + 0x21c]
0033af0a: xor    edx, edx
0033af0c: call   0x182176a30                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʻʲʿˀʻʿʸʹˀʶ
0033af11: xor    al, 1
0033af13: xor    r8d, r8d
0033af16: movzx  edx, al
0033af19: mov    rcx, rbx
0033af1c: call   0x182176900                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʸʴʿʶʹʻʲʶʷʸ
0033af21: xor    edx, edx
0033af23: lea    rcx, [rsp + 0x58]
0033af28: call   0x182176ad0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʿʲʺʶˁʺʸʼʽʼʻ
0033af2d: test   al, al
0033af2f: jne    0x18033af33
0033af31: jmp    0x18033af43
0033af33: lea    rcx, [r13 + 0x21c]
0033af3a: xor    edx, edx
0033af3c: call   0x182176ad0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʿʲʺʶˁʺʸʼʽʼʻ
0033af41: xor    al, 1
0033af43: xor    r8d, r8d
0033af46: movzx  edx, al
0033af49: mov    rcx, rbx
0033af4c: call   0x182176e00                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ˁʴʹʸʳʽʹʺʻʲʽ
0033af51: xor    edx, edx
0033af53: lea    rcx, [rsp + 0x58]
0033af58: call   0x1821769e0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʳʶʵʺʵʸʹʶʶˁ
0033af5d: test   al, al
0033af5f: jne    0x18033af63
0033af61: jmp    0x18033af73
0033af63: lea    rcx, [r13 + 0x21c]
0033af6a: xor    edx, edx
0033af6c: call   0x1821769e0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʳʶʵʺʵʸʹʶʶˁ
0033af71: xor    al, 1
0033af73: xor    r8d, r8d
0033af76: movzx  edx, al
0033af79: mov    rcx, rbx
0033af7c: call   0x182176890                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʴʴʷʺʻʿʵʴʻˁ
0033af81: xor    edx, edx
0033af83: lea    rcx, [rsp + 0x58]
0033af88: call   0x182176650                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʸʼʾˀˀʵˀʶʶʶʽ
0033af8d: test   al, al
0033af8f: jne    0x18033af93
0033af91: jmp    0x18033afa3
0033af93: lea    rcx, [r13 + 0x21c]
0033af9a: xor    edx, edx
0033af9c: call   0x182176650                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʸʼʾˀˀʵˀʶʶʶʽ
0033afa1: xor    al, 1
0033afa3: xor    r8d, r8d
0033afa6: movzx  edx, al
0033afa9: mov    rcx, rbx
0033afac: call   0x182176970                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʼʿʼʼʵʷˁʵʹʻʻ
0033afb1: xor    edx, edx
0033afb3: lea    rcx, [rsp + 0x58]
0033afb8: call   0x182176560                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷʳʼˀˀʵʾʳʳʼʺ
0033afbd: test   al, al
0033afbf: jne    0x18033afc3
0033afc1: jmp    0x18033afd3
0033afc3: lea    rcx, [r13 + 0x21c]
0033afca: xor    edx, edx
0033afcc: call   0x182176560                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷʳʼˀˀʵʾʳʳʼʺ
0033afd1: xor    al, 1
0033afd3: xor    r8d, r8d
0033afd6: movzx  edx, al
0033afd9: mov    rcx, rbx
0033afdc: call   0x1821766f0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʺʸʷʸʻʹʻˁˀˀʶ
0033afe1: xor    edx, edx
0033afe3: lea    rcx, [rsp + 0x58]
0033afe8: call   0x182176a80                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʾʿʳʴʷʽʸʷʹʹʵ
0033afed: test   al, al
0033afef: jne    0x18033aff3
0033aff1: jmp    0x18033b003
0033aff3: lea    rcx, [r13 + 0x21c]
0033affa: xor    edx, edx
0033affc: call   0x182176a80                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʾʿʳʴʷʽʸʷʹʹʵ
0033b001: xor    al, 1
0033b003: xor    r8d, r8d
0033b006: movzx  edx, al
0033b009: mov    rcx, rbx
0033b00c: call   0x182175fb0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʲʴʳʽʶʾˁʵʳʴʾ
0033b011: movsd  xmm0, qword ptr [rsp + 0x58]
0033b017: mov    eax, dword ptr [rsp + 0x60]
0033b01b: movsd  qword ptr [r13 + 0x21c], xmm0
0033b024: mov    dword ptr [r13 + 0x224], eax
0033b02b: jmp    0x18033abdd
0033b030: call   0x182f60c50
0033b035: int3   
0033b036: call   0x182f60c50
0033b03b: int3   
0033b03c: int3   
