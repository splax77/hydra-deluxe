002167b0: mov    qword ptr [rsp + 8], rbx
002167b5: push   rdi
002167b6: sub    rsp, 0xd0
002167bd: mov    rdi, rdx
002167c0: mov    rbx, rcx
002167c3: cmp    byte ptr [rip + 0x3cf6568], 0            ; [0x3f0cd32] (bss)
002167ca: jne    0x180216827
002167cc: lea    rcx, [rip + 0x3a8f5bd]                   ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
002167d3: call   0x182f609b0
002167d8: lea    rcx, [rip + 0x3a8f721]                   ; [0x3ca5f00] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Values()
002167df: call   0x182f609b0
002167e4: lea    rcx, [rip + 0x3ab9415]                   ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
002167eb: call   0x182f609b0
002167f0: lea    rcx, [rip + 0x3ab94c9]                   ; [0x3ccfcc0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.MoveNext()
002167f7: call   0x182f609b0
002167fc: lea    rcx, [rip + 0x3ab957d]                   ; [0x3ccfd80] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Current()
00216803: call   0x182f609b0
00216808: lea    rcx, [rip + 0x3a92be1]                   ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
0021680f: call   0x182f609b0
00216814: lea    rcx, [rip + 0x3acd9b5]                   ; [0x3ce41d0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.GetEnumerator()
0021681b: call   0x182f609b0
00216820: mov    byte ptr [rip + 0x3cf650b], 1            ; [0x3f0cd32] (bss)
00216827: xorps  xmm0, xmm0
0021682a: movups xmmword ptr [rsp + 0x40], xmm0
0021682f: movups xmmword ptr [rsp + 0x50], xmm0
00216834: movups xmmword ptr [rsp + 0x60], xmm0
00216839: movups xmmword ptr [rsp + 0x70], xmm0
0021683e: movups xmmword ptr [rsp + 0x80], xmm0
00216846: mov    rcx, qword ptr [rbx + 0x20]
0021684a: test   rcx, rcx
0021684d: je     0x180216a01
00216853: mov    rdx, qword ptr [rip + 0x3a8f536]         ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
0021685a: call   0x18136e720                              ; System.Collections.Generic.Dictionary<YamlAttributeOverrides.AttributeKey, object>$$get_Count
0021685f: test   eax, eax
00216861: je     0x1802169ea
00216867: lea    rdx, [rbx + 0x10]
0021686b: xor    r8d, r8d
0021686e: lea    rcx, [rsp + 0x30]
00216873: call   0x1820fa3f0                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʸˁʳʷˀʶʴʶʳʼʻ
00216878: movups xmm0, xmmword ptr [rax]
0021687b: movaps xmmword ptr [rsp + 0x20], xmm0
00216880: mov    r8, qword ptr [rip + 0x3a92b69]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
00216887: lea    rdx, [rsp + 0x20]
0021688c: lea    rcx, [rsp + 0x30]
00216891: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
00216896: test   rdi, rdi
00216899: je     0x180216a01
0021689f: movaps xmm0, xmmword ptr [rsp + 0x30]
002168a4: mov    rax, qword ptr [rdi]
002168a7: movdqa xmmword ptr [rsp + 0x30], xmm0
002168ad: mov    r8, qword ptr [rax + 0x1f0]
002168b4: lea    rdx, [rsp + 0x30]
002168b9: mov    rcx, rdi
002168bc: call   qword ptr [rax + 0x1e8]
002168c2: mov    rax, qword ptr [rdi]
002168c5: mov    r8, qword ptr [rax + 0x220]
002168cc: movzx  edx, byte ptr [rbx + 0x2a]
002168d0: mov    rcx, rdi
002168d3: call   qword ptr [rax + 0x218]
002168d9: mov    rax, qword ptr [rdi]
002168dc: mov    r8, qword ptr [rax + 0x2a0]
002168e3: movzx  edx, word ptr [rbx + 0x28]
002168e7: mov    rcx, rdi
002168ea: call   qword ptr [rax + 0x298]
002168f0: mov    rax, qword ptr [rdi]
002168f3: mov    r8, qword ptr [rax + 0x220]
002168fa: movzx  edx, byte ptr [rbx + 0x2b]
002168fe: mov    rcx, rdi
00216901: call   qword ptr [rax + 0x218]
00216907: mov    rcx, qword ptr [rbx + 0x20]
0021690b: test   rcx, rcx
0021690e: je     0x180216a01
00216914: mov    rdx, qword ptr [rip + 0x3a8f5e5]         ; [0x3ca5f00] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Values()
0021691b: call   0x181367ee0                              ; System.Collections.Generic.Dictionary<YamlAttributeOverrides.AttributeKey, object>$$System.Collections.Generic.IReadOnlyDictionary<TKey,TValue>.get_Values
00216920: test   rax, rax
00216923: je     0x180216a01
00216929: mov    r8, qword ptr [rip + 0x3acd8a0]          ; [0x3ce41d0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.GetEnumerator()
00216930: mov    rdx, rax
00216933: lea    rcx, [rsp + 0x98]
0021693b: call   0x180bbf320                              ; System.Collections.Generic.Dictionary.ValueCollection<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$GetEnumerator
00216940: movups xmm0, xmmword ptr [rsp + 0x98]
00216948: movups xmmword ptr [rsp + 0x40], xmm0
0021694d: movups xmm1, xmmword ptr [rsp + 0xa8]
00216955: movups xmmword ptr [rsp + 0x50], xmm1
0021695a: movups xmm0, xmmword ptr [rsp + 0xb8]
00216962: movups xmmword ptr [rsp + 0x60], xmm0
00216967: mov    qword ptr [rsp + 0x20], 0
00216970: lea    rbx, [rsp + 0x40]
00216975: mov    qword ptr [rsp + 0x28], rbx
0021697a: nop    word ptr [rax + rax]
00216980: mov    rdx, qword ptr [rip + 0x3ab9339]         ; [0x3ccfcc0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.MoveNext()
00216987: lea    rcx, [rsp + 0x40]
0021698c: call   0x18151c4a0                              ; System.Collections.Generic.Dictionary.ValueCollection.Enumerator<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$MoveNext
00216991: test   al, al
00216993: je     0x1802169be
00216995: movups xmm0, xmmword ptr [rsp + 0x50]
0021699a: movups xmmword ptr [rsp + 0x70], xmm0
0021699f: movups xmm1, xmmword ptr [rsp + 0x60]
002169a4: movups xmmword ptr [rsp + 0x80], xmm1
002169ac: xor    r8d, r8d
002169af: mov    rdx, rdi
002169b2: lea    rcx, [rsp + 0x70]
002169b7: call   0x1802bab60                              ; ˀʵʳʲʺʽʵˁʹʺʼ$$ˁʷʲʽʼʾˁʶʴʷʺ
002169bc: jmp    0x180216980
002169be: mov    rdx, qword ptr [rip + 0x3ab923b]         ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
002169c5: mov    rcx, rbx
002169c8: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002169cd: jmp    0x1802169ea
002169cf: mov    rdx, qword ptr [rip + 0x3ab922a]         ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
002169d6: mov    rcx, qword ptr [rsp + 0x28]
002169db: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002169e0: mov    rcx, qword ptr [rsp + 0x20]
002169e5: test   rcx, rcx
002169e8: jne    0x1802169fb
002169ea: mov    rbx, qword ptr [rsp + 0xe0]
002169f2: add    rsp, 0xd0
002169f9: pop    rdi
002169fa: ret    
002169fb: call   0x182f60ce0
00216a00: int3   
00216a01: call   0x182f60c50
00216a06: int3   
00216a07: int3   
