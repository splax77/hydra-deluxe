00216fa0: mov    qword ptr [rsp + 8], rbx
00216fa5: push   rdi
00216fa6: sub    rsp, 0xd0
00216fad: mov    rdi, rdx
00216fb0: mov    rbx, rcx
00216fb3: cmp    byte ptr [rip + 0x3cf5d6f], 0            ; [0x3f0cd29] (bss)
00216fba: jne    0x180217017
00216fbc: lea    rcx, [rip + 0x3a8edcd]                   ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
00216fc3: call   0x182f609b0
00216fc8: lea    rcx, [rip + 0x3a8ef31]                   ; [0x3ca5f00] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Values()
00216fcf: call   0x182f609b0
00216fd4: lea    rcx, [rip + 0x3ab8c25]                   ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
00216fdb: call   0x182f609b0
00216fe0: lea    rcx, [rip + 0x3ab8cd9]                   ; [0x3ccfcc0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.MoveNext()
00216fe7: call   0x182f609b0
00216fec: lea    rcx, [rip + 0x3ab8d8d]                   ; [0x3ccfd80] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Current()
00216ff3: call   0x182f609b0
00216ff8: lea    rcx, [rip + 0x3a923f1]                   ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
00216fff: call   0x182f609b0
00217004: lea    rcx, [rip + 0x3acd1c5]                   ; [0x3ce41d0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.GetEnumerator()
0021700b: call   0x182f609b0
00217010: mov    byte ptr [rip + 0x3cf5d12], 1            ; [0x3f0cd29] (bss)
00217017: xorps  xmm0, xmm0
0021701a: movups xmmword ptr [rsp + 0x40], xmm0
0021701f: movups xmmword ptr [rsp + 0x50], xmm0
00217024: movups xmmword ptr [rsp + 0x60], xmm0
00217029: movups xmmword ptr [rsp + 0x70], xmm0
0021702e: movups xmmword ptr [rsp + 0x80], xmm0
00217036: mov    rcx, qword ptr [rbx + 0x20]
0021703a: test   rcx, rcx
0021703d: je     0x1802171f1
00217043: mov    rdx, qword ptr [rip + 0x3a8ed46]         ; [0x3ca5d90] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Count()
0021704a: call   0x18136e720                              ; System.Collections.Generic.Dictionary<YamlAttributeOverrides.AttributeKey, object>$$get_Count
0021704f: test   eax, eax
00217051: je     0x1802171da
00217057: lea    rdx, [rbx + 0x10]
0021705b: xor    r8d, r8d
0021705e: lea    rcx, [rsp + 0x30]
00217063: call   0x1820fa3f0                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʸˁʳʷˀʶʴʶʳʼʻ
00217068: movups xmm0, xmmword ptr [rax]
0021706b: movaps xmmword ptr [rsp + 0x20], xmm0
00217070: mov    r8, qword ptr [rip + 0x3a92379]          ; [0x3ca93f0] metamethod:Method$System.Span<byte>.op_Implicit()
00217077: lea    rdx, [rsp + 0x20]
0021707c: lea    rcx, [rsp + 0x30]
00217081: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
00217086: test   rdi, rdi
00217089: je     0x1802171f1
0021708f: movaps xmm0, xmmword ptr [rsp + 0x30]
00217094: mov    rax, qword ptr [rdi]
00217097: movdqa xmmword ptr [rsp + 0x30], xmm0
0021709d: mov    r8, qword ptr [rax + 0x1f0]
002170a4: lea    rdx, [rsp + 0x30]
002170a9: mov    rcx, rdi
002170ac: call   qword ptr [rax + 0x1e8]
002170b2: mov    rax, qword ptr [rdi]
002170b5: mov    r8, qword ptr [rax + 0x220]
002170bc: movzx  edx, byte ptr [rbx + 0x2a]
002170c0: mov    rcx, rdi
002170c3: call   qword ptr [rax + 0x218]
002170c9: mov    rax, qword ptr [rdi]
002170cc: mov    r8, qword ptr [rax + 0x2a0]
002170d3: movzx  edx, word ptr [rbx + 0x28]
002170d7: mov    rcx, rdi
002170da: call   qword ptr [rax + 0x298]
002170e0: mov    rax, qword ptr [rdi]
002170e3: mov    r8, qword ptr [rax + 0x220]
002170ea: movzx  edx, byte ptr [rbx + 0x2b]
002170ee: mov    rcx, rdi
002170f1: call   qword ptr [rax + 0x218]
002170f7: mov    rcx, qword ptr [rbx + 0x20]
002170fb: test   rcx, rcx
002170fe: je     0x1802171f1
00217104: mov    rdx, qword ptr [rip + 0x3a8edf5]         ; [0x3ca5f00] metamethod:Method$System.Collections.Generic.Dictionary<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.get_Values()
0021710b: call   0x181367ee0                              ; System.Collections.Generic.Dictionary<YamlAttributeOverrides.AttributeKey, object>$$System.Collections.Generic.IReadOnlyDictionary<TKey,TValue>.get_Values
00217110: test   rax, rax
00217113: je     0x1802171f1
00217119: mov    r8, qword ptr [rip + 0x3acd0b0]          ; [0x3ce41d0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.GetEnumerator()
00217120: mov    rdx, rax
00217123: lea    rcx, [rsp + 0x98]
0021712b: call   0x180bbf320                              ; System.Collections.Generic.Dictionary.ValueCollection<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$GetEnumerator
00217130: movups xmm0, xmmword ptr [rsp + 0x98]
00217138: movups xmmword ptr [rsp + 0x40], xmm0
0021713d: movups xmm1, xmmword ptr [rsp + 0xa8]
00217145: movups xmmword ptr [rsp + 0x50], xmm1
0021714a: movups xmm0, xmmword ptr [rsp + 0xb8]
00217152: movups xmmword ptr [rsp + 0x60], xmm0
00217157: mov    qword ptr [rsp + 0x20], 0
00217160: lea    rbx, [rsp + 0x40]
00217165: mov    qword ptr [rsp + 0x28], rbx
0021716a: nop    word ptr [rax + rax]
00217170: mov    rdx, qword ptr [rip + 0x3ab8b49]         ; [0x3ccfcc0] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.MoveNext()
00217177: lea    rcx, [rsp + 0x40]
0021717c: call   0x18151c4a0                              ; System.Collections.Generic.Dictionary.ValueCollection.Enumerator<SByteEnum, ˀʵʳʲʺʽʵˁʹʺʼ>$$MoveNext
00217181: test   al, al
00217183: je     0x1802171ae
00217185: movups xmm0, xmmword ptr [rsp + 0x50]
0021718a: movups xmmword ptr [rsp + 0x70], xmm0
0021718f: movups xmm1, xmmword ptr [rsp + 0x60]
00217194: movups xmmword ptr [rsp + 0x80], xmm1
0021719c: xor    r8d, r8d
0021719f: mov    rdx, rdi
002171a2: lea    rcx, [rsp + 0x70]
002171a7: call   0x1802bab60                              ; ˀʵʳʲʺʽʵˁʹʺʼ$$ˁʷʲʽʼʾˁʶʴʷʺ
002171ac: jmp    0x180217170
002171ae: mov    rdx, qword ptr [rip + 0x3ab8a4b]         ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
002171b5: mov    rcx, rbx
002171b8: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002171bd: jmp    0x1802171da
002171bf: mov    rdx, qword ptr [rip + 0x3ab8a3a]         ; [0x3ccfc00] metamethod:Method$System.Collections.Generic.Dictionary.ValueCollection.Enumerator<ʼˁʿʷʶʸʴʼʹʾʶ, ˀʵʳʲʺʽʵˁʹʺʼ>.Dispose()
002171c6: mov    rcx, qword ptr [rsp + 0x28]
002171cb: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002171d0: mov    rcx, qword ptr [rsp + 0x20]
002171d5: test   rcx, rcx
002171d8: jne    0x1802171eb
002171da: mov    rbx, qword ptr [rsp + 0xe0]
002171e2: add    rsp, 0xd0
002171e9: pop    rdi
002171ea: ret    
002171eb: call   0x182f60ce0
002171f0: int3   
002171f1: call   0x182f60c50
002171f6: int3   
002171f7: int3   
