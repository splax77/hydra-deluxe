002161e0: mov    qword ptr [rsp + 8], rbx
002161e5: push   rsi
002161e6: sub    rsp, 0x80
002161ed: cmp    byte ptr [rip + 0x3cf6b3d], 0            ; [0x3f0cd31] (bss)
002161f4: mov    rsi, rdx
002161f7: mov    rbx, rcx
002161fa: jne    0x18021621b
002161fc: lea    rcx, [rip + 0x3a8fb8d]                   ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
00216203: call   0x182f609b0
00216208: lea    rcx, [rip + 0x3a8fda9]                   ; [0x3ca5fb8] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.set_Item()
0021620f: call   0x182f609b0
00216214: mov    byte ptr [rip + 0x3cf6b16], 1            ; [0x3f0cd31] (bss)
0021621b: cmp    byte ptr [rip + 0x3cf6b12], 0            ; [0x3f0cd34] (bss)
00216222: jne    0x180216243
00216224: lea    rcx, [rip + 0x3a8f93d]                   ; [0x3ca5b68] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.ContainsKey()
0021622b: call   0x182f609b0
00216230: lea    rcx, [rip + 0x3a8fc11]                   ; [0x3ca5e48] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Item()
00216237: call   0x182f609b0
0021623c: mov    byte ptr [rip + 0x3cf6af1], 1            ; [0x3f0cd34] (bss)
00216243: cmp    qword ptr [rbx + 0x20], 0
00216248: je     0x18021628a
0021624a: mov    r8, qword ptr [rip + 0x3a8f917]          ; [0x3ca5b68] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.ContainsKey()
00216251: movzx  edx, byte ptr [rsi]
00216254: mov    rcx, qword ptr [rbx + 0x20]
00216258: call   0x181413cd0                              ; System.Collections.Generic.Dictionary<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$ContainsKey
0021625d: test   al, al
0021625f: je     0x18021628a
00216261: mov    rdx, qword ptr [rbx + 0x20]
00216265: test   rdx, rdx
00216268: je     0x180216352
0021626e: mov    r9, qword ptr [rip + 0x3a8fbd3]          ; [0x3ca5e48] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Item()
00216275: lea    rcx, [rsp + 0x20]
0021627a: movzx  r8d, byte ptr [rsi]
0021627e: call   0x181427670                              ; System.Collections.Generic.Dictionary<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$get_Item
00216283: movups xmm2, xmmword ptr [rsp + 0x30]
00216288: jmp    0x180216297
0021628a: xorps  xmm2, xmm2
0021628d: movups xmmword ptr [rsp + 0x20], xmm2
00216292: mov    byte ptr [rsp + 0x20], 0xf
00216297: movups xmm3, xmmword ptr [rsp + 0x20]
0021629c: xor    r9d, r9d
0021629f: lea    r8, [rsp + 0x20]
002162a4: movups xmm0, xmmword ptr [rsi]
002162a7: lea    rdx, [rsp + 0x40]
002162ac: movups xmm1, xmmword ptr [rsi + 0x10]
002162b0: lea    rcx, [rsp + 0x60]
002162b5: movaps xmmword ptr [rsp + 0x20], xmm0
002162ba: movaps xmmword ptr [rsp + 0x30], xmm1
002162bf: movaps xmmword ptr [rsp + 0x40], xmm3
002162c4: movaps xmmword ptr [rsp + 0x50], xmm2
002162c9: call   0x1802ba4f0                              ; ˀʵʳʲʺʽʵˁʹʺʼ$$ʲʷʵʺʸʹʾˀʻʾʳ
002162ce: movups xmm0, xmmword ptr [rax + 1]
002162d2: movzx  edx, byte ptr [rax]
002162d5: movups xmm1, xmmword ptr [rax + 0x10]
002162d9: mov    eax, 0xffff
002162de: movups xmmword ptr [rsp + 0x41], xmm0
002162e3: movups xmmword ptr [rsp + 0x50], xmm1
002162e8: cmp    word ptr [rbx + 0x28], ax
002162ec: jae    0x1802162f2
002162ee: inc    word ptr [rbx + 0x28]
002162f2: mov    rcx, qword ptr [rbx + 0x20]
002162f6: test   rcx, rcx
002162f9: je     0x180216352
002162fb: movups xmm0, xmmword ptr [rsp + 0x41]
00216300: mov    r9, qword ptr [rip + 0x3a8fcb1]          ; [0x3ca5fb8] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.set_Item()
00216307: lea    r8, [rsp + 0x40]
0021630c: mov    byte ptr [rsp + 0x20], dl
00216310: movups xmmword ptr [rsp + 0x21], xmm0
00216315: movaps xmm0, xmmword ptr [rsp + 0x20]
0021631a: movaps xmmword ptr [rsp + 0x40], xmm0
0021631f: movaps xmmword ptr [rsp + 0x50], xmm1
00216324: call   0x1814279a0                              ; System.Collections.Generic.Dictionary<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$set_Item
00216329: mov    rcx, qword ptr [rbx + 0x20]
0021632d: test   rcx, rcx
00216330: je     0x180216352
00216332: mov    rdx, qword ptr [rip + 0x3a8fa57]         ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
00216339: call   0x18136e720                              ; System.Collections.Generic.Dictionary<YamlAttributeOverrides.AttributeKey, object>$$get_Count
0021633e: mov    byte ptr [rbx + 0x2a], al
00216341: mov    rbx, qword ptr [rsp + 0x90]
00216349: add    rsp, 0x80
00216350: pop    rsi
00216351: ret    
00216352: call   0x182f60c50
00216357: int3   
00216358: int3   
