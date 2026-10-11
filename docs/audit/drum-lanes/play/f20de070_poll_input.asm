020de070: push   rbx
020de072: push   rbp
020de073: push   rsi
020de074: push   rdi
020de075: sub    rsp, 0x28
020de079: movzx  eax, word ptr [rcx + 0x152]
020de080: mov    rsi, rcx
020de083: mov    word ptr [rcx + 0x150], ax
020de08a: mov    rax, qword ptr [rcx + 0x2a8]
020de091: mov    qword ptr [rcx + 0x309], rax
020de098: add    rcx, 0x2a4
020de09f: xor    edx, edx
020de0a1: call   0x182176a80                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʾʿʳʴʷʽʸʷʹʹʵ
020de0a6: movzx  ebx, al
020de0a9: mov    ebp, 0x80
020de0ae: imul   ebx, ebp
020de0b1: lea    rcx, [rsi + 0x2a4]
020de0b8: xor    edx, edx
020de0ba: call   0x182176650                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʸʼʾˀˀʵˀʶʶʶʽ
020de0bf: movzx  edi, bx
020de0c2: or     di, 0x20
020de0c6: test   al, al
020de0c8: cmove  di, bx
020de0cc: lea    rcx, [rsi + 0x2a4]
020de0d3: xor    edx, edx
020de0d5: call   0x182176560                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷʳʼˀˀʵʾʳʳʼʺ
020de0da: movzx  ebx, di
020de0dd: or     bx, 0x40
020de0e1: test   al, al
020de0e3: cmove  bx, di
020de0e7: lea    rcx, [rsi + 0x2a4]
020de0ee: xor    edx, edx
020de0f0: call   0x182176600                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʷˀʽʺʷˁʷʿˀʴʾ
020de0f5: movzx  edi, bx
020de0f8: or     di, 2
020de0fc: test   al, al
020de0fe: cmove  di, bx
020de102: lea    rcx, [rsi + 0x2a4]
020de109: xor    edx, edx
020de10b: call   0x182176a30                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʻʲʿˀʻʿʸʹˀʶ
020de110: movzx  ebx, di
020de113: or     bx, 4
020de117: test   al, al
020de119: cmove  bx, di
020de11d: lea    rcx, [rsi + 0x2a4]
020de124: xor    edx, edx
020de126: call   0x182176ad0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʿʲʺʶˁʺʸʼʽʼʻ
020de12b: movzx  edi, bx
020de12e: or     di, 8
020de132: test   al, al
020de134: cmove  di, bx
020de138: lea    rcx, [rsi + 0x2a4]
020de13f: xor    edx, edx
020de141: call   0x1821769e0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʽʳʶʵʺʵʸʹʶʶˁ
020de146: movzx  r8d, di
020de14a: or     r8w, 0x10
020de14f: test   al, al
020de151: cmove  r8w, di
020de156: cmp    byte ptr [rsi + 0x308], 0
020de15d: mov    word ptr [rsi + 0x152], r8w
020de165: je     0x1820de1d2
020de167: movzx  ecx, r8w
020de16b: shr    cx, 3
020de16f: and    cx, 2
020de173: movzx  edx, cx
020de176: or     dx, 8
020de17a: test   r8b, 4
020de17e: cmove  dx, cx
020de182: movzx  ecx, dx
020de185: or     cx, 4
020de189: test   r8b, 8
020de18d: cmove  cx, dx
020de191: movzx  edx, cx
020de194: or     dx, 0x10
020de198: test   r8b, 2
020de19c: cmove  dx, cx
020de1a0: movzx  ecx, dx
020de1a3: or     cx, bp
020de1a6: test   bpl, r8b
020de1a9: cmove  cx, dx
020de1ad: movzx  edx, cx
020de1b0: or     dx, 0x40
020de1b4: test   r8b, 0x20
020de1b8: cmove  dx, cx
020de1bc: movzx  eax, dx
020de1bf: or     ax, 0x20
020de1c3: test   r8b, 0x40
020de1c7: cmove  ax, dx
020de1cb: mov    word ptr [rsi + 0x152], ax
020de1d2: cmp    byte ptr [rsi + 0x2f6], 0
020de1d9: movzx  eax, word ptr [rsi + 0x152]
020de1e0: mov    word ptr [rsi + 0x2f4], ax
020de1e7: jne    0x1820de207
020de1e9: shr    ax, 3
020de1ed: mov    ecx, 0xfffd
020de1f2: and    ax, cx
020de1f5: or     ax, word ptr [rsi + 0x152]
020de1fc: and    ax, 0x1e
020de200: mov    word ptr [rsi + 0x152], ax
020de207: lea    rcx, [rsi + 0x2a4]
020de20e: xor    edx, edx
020de210: call   0x1821766a0                              ; ʾʸʸʻʷʸˁʾˁʽʸ$$ʹʻʺʺʲʼʲʿʽʳʸ
020de215: test   al, al
020de217: je     0x1820de229
020de219: or     word ptr [rsi + 0x152], 1
020de221: or     word ptr [rsi + 0x2f4], 1
020de229: xor    edx, edx
020de22b: mov    rcx, rsi
020de22e: add    rsp, 0x28
020de232: pop    rdi
020de233: pop    rsi
020de234: pop    rbp
020de235: pop    rbx
020de236: jmp    0x1820deaa0                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʿʶʲʳʲʴʴʵʴʲʷ
020de23b: int3   
