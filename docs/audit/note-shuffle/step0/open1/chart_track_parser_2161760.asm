02161760: push   rbp
02161762: push   rbx
02161763: push   rsi
02161764: push   rdi
02161765: push   r12
02161767: push   r14
02161769: push   r15
0216176b: mov    rbp, rsp
0216176e: sub    rsp, 0x50
02161772: cmp    byte ptr [rip + 0x1db7f2c], 0            ; [0x3f196a5] (bss)
02161779: mov    rdi, r9
0216177c: mov    esi, r8d
0216177f: mov    r14, rdx
02161782: mov    r15, rcx
02161785: jne    0x1821617be
02161787: lea    rcx, [rip + 0x1ba83ea]                   ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
0216178e: call   0x182f609b0
02161793: lea    rcx, [rip + 0x1b97dee]                   ; [0x3cf9588] str:'SyncTrack'
0216179a: call   0x182f609b0
0216179f: lea    rcx, [rip + 0x1b54e12]                   ; [0x3cb65b8] str:'Events'
021617a6: call   0x182f609b0
021617ab: lea    rcx, [rip + 0x1b7f87e]                   ; [0x3ce1030] str:'Song'
021617b2: call   0x182f609b0
021617b7: mov    byte ptr [rip + 0x1db7ee7], 1            ; [0x3f196a5] (bss)
021617be: cmp    byte ptr [rip + 0x1daa996], 0            ; [0x3f0c15b] (bss)
021617c5: mov    rbx, qword ptr [rip + 0x1b7f864]         ; [0x3ce1030] str:'Song'
021617cc: jne    0x1821617e1
021617ce: lea    rcx, [rip + 0x1b3fdd3]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
021617d5: call   0x182f609b0
021617da: mov    byte ptr [rip + 0x1daa97a], 1            ; [0x3f0c15b] (bss)
021617e1: xor    r12d, r12d
021617e4: test   rbx, rbx
021617e7: je     0x182161807
021617e9: xor    edx, edx
021617eb: mov    dword ptr [rbp - 0x14], r12d
021617ef: mov    rcx, rbx
021617f2: call   0x1819829d0                              ; System.String$$GetRawStringData
021617f7: mov    qword ptr [rbp - 0x20], rax
021617fb: mov    eax, dword ptr [rbx + 0x10]
021617fe: mov    dword ptr [rbp - 0x18], eax
02161801: movaps xmm0, xmmword ptr [rbp - 0x20]
02161805: jmp    0x18216180a
02161807: xorps  xmm0, xmm0
0216180a: mov    r8, qword ptr [rip + 0x1ba8367]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02161811: lea    rdx, [rbp - 0x20]
02161815: movdqa xmmword ptr [rbp - 0x20], xmm0
0216181a: lea    rcx, [rbp - 0x10]
0216181e: movups xmm0, xmmword ptr [r15]
02161822: movaps xmmword ptr [rbp - 0x10], xmm0
02161826: call   0x182c61850
0216182b: test   al, al
0216182d: jne    0x182161997
02161833: cmp    byte ptr [rip + 0x1daa921], r12b         ; [0x3f0c15b] (bss)
0216183a: mov    rbx, qword ptr [rip + 0x1b97d47]         ; [0x3cf9588] str:'SyncTrack'
02161841: jne    0x182161856
02161843: lea    rcx, [rip + 0x1b3fd5e]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0216184a: call   0x182f609b0
0216184f: mov    byte ptr [rip + 0x1daa905], 1            ; [0x3f0c15b] (bss)
02161856: test   rbx, rbx
02161859: je     0x182161879
0216185b: xor    edx, edx
0216185d: mov    dword ptr [rbp - 0x14], r12d
02161861: mov    rcx, rbx
02161864: call   0x1819829d0                              ; System.String$$GetRawStringData
02161869: mov    qword ptr [rbp - 0x20], rax
0216186d: mov    eax, dword ptr [rbx + 0x10]
02161870: mov    dword ptr [rbp - 0x18], eax
02161873: movaps xmm0, xmmword ptr [rbp - 0x20]
02161877: jmp    0x18216187c
02161879: xorps  xmm0, xmm0
0216187c: mov    r8, qword ptr [rip + 0x1ba82f5]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
02161883: lea    rdx, [rbp - 0x10]
02161887: movdqa xmmword ptr [rbp - 0x10], xmm0
0216188c: lea    rcx, [rbp - 0x20]
02161890: movups xmm0, xmmword ptr [r15]
02161894: movaps xmmword ptr [rbp - 0x20], xmm0
02161898: call   0x182c61850
0216189d: test   al, al
0216189f: jne    0x18216196f
021618a5: cmp    byte ptr [rip + 0x1daa8af], r12b         ; [0x3f0c15b] (bss)
021618ac: mov    rbx, qword ptr [rip + 0x1b54d05]         ; [0x3cb65b8] str:'Events'
021618b3: jne    0x1821618c8
021618b5: lea    rcx, [rip + 0x1b3fcec]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
021618bc: call   0x182f609b0
021618c1: mov    byte ptr [rip + 0x1daa893], 1            ; [0x3f0c15b] (bss)
021618c8: test   rbx, rbx
021618cb: je     0x1821618eb
021618cd: xor    edx, edx
021618cf: mov    dword ptr [rbp - 0x14], r12d
021618d3: mov    rcx, rbx
021618d6: call   0x1819829d0                              ; System.String$$GetRawStringData
021618db: mov    qword ptr [rbp - 0x20], rax
021618df: mov    eax, dword ptr [rbx + 0x10]
021618e2: mov    dword ptr [rbp - 0x18], eax
021618e5: movaps xmm0, xmmword ptr [rbp - 0x20]
021618e9: jmp    0x1821618ee
021618eb: xorps  xmm0, xmm0
021618ee: mov    r8, qword ptr [rip + 0x1ba8283]          ; [0x3d09b78] metamethod:Method$System.MemoryExtensions.SequenceEqual<char>()
021618f5: lea    rdx, [rbp - 0x10]
021618f9: movdqa xmmword ptr [rbp - 0x10], xmm0
021618fe: lea    rcx, [rbp - 0x20]
02161902: movups xmm0, xmmword ptr [r15]
02161906: movaps xmmword ptr [rbp - 0x20], xmm0
0216190a: call   0x182c61850
0216190f: mov    edx, esi
02161911: test   al, al
02161913: jne    0x182161949
02161915: movups xmm0, xmmword ptr [r15]
02161919: mov    r9, rdi
0216191c: lea    r8, [rbp - 0x10]
02161920: movups xmm1, xmmword ptr [r14]
02161924: lea    rcx, [rbp - 0x20]
02161928: mov    qword ptr [rsp + 0x20], r12
0216192d: movaps xmmword ptr [rbp - 0x10], xmm0
02161931: movaps xmmword ptr [rbp - 0x20], xmm1
02161935: call   0x182151650                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʻʸˀʴʾˀʲʵʸʿʾ
0216193a: add    rsp, 0x50
0216193e: pop    r15
02161940: pop    r14
02161942: pop    r12
02161944: pop    rdi
02161945: pop    rsi
02161946: pop    rbx
02161947: pop    rbp
02161948: ret    
02161949: movups xmm0, xmmword ptr [r14]
0216194d: xor    r9d, r9d
02161950: lea    rcx, [rbp - 0x10]
02161954: mov    r8, rdi
02161957: movaps xmmword ptr [rbp - 0x10], xmm0
0216195b: call   0x182151830
02161960: add    rsp, 0x50
02161964: pop    r15
02161966: pop    r14
02161968: pop    r12
0216196a: pop    rdi
0216196b: pop    rsi
0216196c: pop    rbx
0216196d: pop    rbp
0216196e: ret    
0216196f: movups xmm0, xmmword ptr [r14]
02161973: xor    r9d, r9d
02161976: lea    rcx, [rbp - 0x10]
0216197a: mov    r8, rdi
0216197d: mov    edx, esi
0216197f: movaps xmmword ptr [rbp - 0x10], xmm0
02161983: call   0x182150e60
02161988: add    rsp, 0x50
0216198c: pop    r15
0216198e: pop    r14
02161990: pop    r12
02161992: pop    rdi
02161993: pop    rsi
02161994: pop    rbx
02161995: pop    rbp
02161996: ret    
02161997: movups xmm0, xmmword ptr [r14]
0216199b: mov    r8, qword ptr [rdi]
0216199e: lea    rcx, [rbp - 0x10]
021619a2: xor    r9d, r9d
021619a5: mov    edx, esi
021619a7: movaps xmmword ptr [rbp - 0x10], xmm0
021619ab: call   0x182164100                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ˁʸʷʶʳˀʼʹʿʺʳ
021619b0: add    rsp, 0x50
021619b4: pop    r15
021619b6: pop    r14
021619b8: pop    r12
021619ba: pop    rdi
021619bb: pop    rsi
021619bc: pop    rbx
021619bd: pop    rbp
021619be: ret    
021619bf: int3   
