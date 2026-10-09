=== 20E34F0
020e34f0: sub    rsp, 0x28
020e34f4: mov    rcx, qword ptr [rcx + 0x10]
020e34f8: test   rcx, rcx
020e34fb: je     0x1820e3508
020e34fd: xor    edx, edx
020e34ff: add    rsp, 0x28
020e3503: jmp    0x1801b40b0                              ; System.Xml.XmlNode$$get_LastChild
020e3508: call   0x182f60c50
020e350d: int3   
020e350e: int3   
=== 20E3510
020e3510: push   rdi
020e3512: push   r14
020e3514: sub    rsp, 0x68
020e3518: mov    r14, rdx
020e351b: mov    rdi, rcx
020e351e: mov    rcx, qword ptr [rcx + 0x10]
020e3522: xor    edx, edx
020e3524: call   0x1820cf940
020e3529: cmp    eax, 1
020e352c: jbe    0x1820e36b2
020e3532: mov    qword ptr [rsp + 0x80], rbx
020e353a: mov    qword ptr [rsp + 0x90], rsi
020e3542: mov    qword ptr [rsp + 0x60], r15
020e3547: xor    r15d, r15d
020e354a: mov    qword ptr [rsp + 0x88], rbp
020e3552: cmp    byte ptr [rip + 0x1e35dc2], r15b         ; [0x3f1931b] (bss)
020e3559: mov    rsi, qword ptr [rdi + 0x10]
020e355d: jne    0x1820e3572
020e355f: lea    rcx, [rip + 0x1c2599a]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3566: call   0x182f609b0
020e356b: mov    byte ptr [rip + 0x1e35da9], 1            ; [0x3f1931b] (bss)
020e3572: xor    ecx, ecx
020e3574: call   0x1801bd810
020e3579: mov    rcx, qword ptr [rip + 0x1c25980]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3580: mov    rbx, rax
020e3583: cmp    qword ptr [rcx + 0x38], r15
020e3587: jne    0x1820e358e
020e3589: call   0x182f657d0
020e358e: mov    qword ptr [rsp + 0x38], r15
020e3593: test   rbx, rbx
020e3596: je     0x1820e36c0
020e359c: cmp    dword ptr [rbx + 0x18], 8
020e35a0: jae    0x1820e35a9
020e35a2: xor    ecx, ecx
020e35a4: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e35a9: lea    rax, [rbx + 0x20]
020e35ad: mov    dword ptr [rsp + 0x38], 8
020e35b5: mov    qword ptr [rsp + 0x30], rax
020e35ba: lea    rdx, [rsp + 0x50]
020e35bf: movaps xmm0, xmmword ptr [rsp + 0x30]
020e35c4: xor    r8d, r8d
020e35c7: mov    rcx, rsi
020e35ca: movdqa xmmword ptr [rsp + 0x50], xmm0
020e35d0: call   0x1805f18b0
020e35d5: cmp    dword ptr [rbx + 0x18], r15d
020e35d9: jbe    0x1820e36ba
020e35df: cmp    byte ptr [rip + 0x1e35d35], r15b         ; [0x3f1931b] (bss)
020e35e6: mov    rbp, qword ptr [rbx + 0x20]
020e35ea: mov    rsi, qword ptr [rdi + 0x10]
020e35ee: jne    0x1820e3603
020e35f0: lea    rcx, [rip + 0x1c25909]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e35f7: call   0x182f609b0
020e35fc: mov    byte ptr [rip + 0x1e35d18], 1            ; [0x3f1931b] (bss)
020e3603: xor    ecx, ecx
020e3605: call   0x1801bd810
020e360a: mov    rcx, qword ptr [rip + 0x1c258ef]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3611: mov    rbx, rax
020e3614: cmp    qword ptr [rcx + 0x38], r15
020e3618: jne    0x1820e361f
020e361a: call   0x182f657d0
020e361f: mov    qword ptr [rsp + 0x48], r15
020e3624: test   rbx, rbx
020e3627: je     0x1820e36c0
020e362d: cmp    dword ptr [rbx + 0x18], 8
020e3631: jae    0x1820e363a
020e3633: xor    ecx, ecx
020e3635: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e363a: lea    rax, [rbx + 0x20]
020e363e: mov    dword ptr [rsp + 0x48], 8
020e3646: mov    qword ptr [rsp + 0x40], rax
020e364b: lea    rdx, [rsp + 0x50]
020e3650: movaps xmm0, xmmword ptr [rsp + 0x40]
020e3655: xor    r8d, r8d
020e3658: mov    rcx, rsi
020e365b: movdqa xmmword ptr [rsp + 0x50], xmm0
020e3661: call   0x1805f18b0
020e3666: cmp    dword ptr [rbx + 0x18], r15d
020e366a: jbe    0x1820e36ba
020e366c: mov    rcx, qword ptr [rdi + 0x10]
020e3670: test   rcx, rcx
020e3673: je     0x1820e36e0
020e3675: mov    rax, qword ptr [rcx]
020e3678: mov    rbx, qword ptr [rbx + 0x20]
020e367c: mov    rdx, qword ptr [rax + 0x390]
020e3683: call   qword ptr [rax + 0x388]
020e3689: cmp    eax, 1
020e368c: sete   r9b
020e3690: test   r14, r14
020e3693: je     0x1820e36e0
020e3695: mov    qword ptr [rsp + 0x28], r15
020e369a: mov    r8, rbx
020e369d: mov    rdx, rbp
020e36a0: mov    byte ptr [rsp + 0x20], 0xff
020e36a5: mov    rcx, r14
020e36a8: call   0x18215e4f0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʹʻʳʺˀʲʳʷʾˀʻ
020e36ad: jmp    0x1820e3552
020e36b2: add    rsp, 0x68
020e36b6: pop    r14
020e36b8: pop    rdi
020e36b9: ret    
020e36ba: call   0x182f60c40
020e36bf: int3   
020e36c0: xor    ecx, ecx
020e36c2: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e36c7: xorps  xmm0, xmm0
020e36ca: lea    rdx, [rsp + 0x50]
020e36cf: xor    r8d, r8d
020e36d2: movdqa xmmword ptr [rsp + 0x50], xmm0
020e36d8: mov    rcx, rsi
020e36db: call   0x1805f18b0
020e36e0: call   0x182f60c50
020e36e5: int3   
020e36e6: int3   
=== 20E36F0
020e36f0: push   rbp
020e36f2: push   rbx
020e36f3: push   rsi
020e36f4: push   rdi
020e36f5: push   r14
020e36f7: sub    rsp, 0x90
020e36fe: lea    rbp, [rsp + 0x60]
020e3703: cmp    byte ptr [rip + 0x1e35ba1], 0            ; [0x3f192ab] (bss)
020e370a: mov    rdi, rcx
020e370d: movaps xmmword ptr [rbp + 0x20], xmm6
020e3711: jne    0x1820e3756
020e3713: lea    rcx, [rip + 0x1c263a6]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e371a: call   0x182f609b0
020e371f: lea    rcx, [rip + 0x1bbdd1a]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e3726: call   0x182f609b0
020e372b: lea    rcx, [rip + 0x1bc54d6]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e3732: call   0x182f609b0
020e3737: lea    rcx, [rip + 0x1c2ac92]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e373e: call   0x182f609b0
020e3743: lea    rcx, [rip + 0x1bb9ab6]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e374a: call   0x182f609b0
020e374f: mov    byte ptr [rip + 0x1e35b55], 1            ; [0x3f192ab] (bss)
020e3756: mov    rax, qword ptr [rip + 0x1bb9aa3]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e375d: cmp    dword ptr [rax + 0xe0], 0
020e3764: jne    0x1820e3775
020e3766: mov    rcx, rax
020e3769: call   0x182f60cf0
020e376e: mov    rax, qword ptr [rip + 0x1bb9a8b]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e3775: mov    rax, qword ptr [rax + 0xb8]
020e377c: mov    rsi, qword ptr [rax]
020e377f: test   rsi, rsi
020e3782: je     0x1820e38fc
020e3788: movsxd rbx, dword ptr [rsi + 0x18]
020e378c: test   ebx, ebx
020e378e: je     0x1820e37b9
020e3790: lea    rcx, [rbx + 0xf]
020e3794: cmp    rcx, rbx
020e3797: ja     0x1820e37a3
020e3799: movabs rcx, 0xffffffffffffff0
020e37a3: and    rcx, 0xfffffffffffffff0
020e37a7: mov    rax, rcx
020e37aa: call   0x18302b960
020e37af: sub    rsp, rcx
020e37b2: lea    r14, [rsp + 0x60]
020e37b7: jmp    0x1820e37bc
020e37b9: xor    r14d, r14d
020e37bc: mov    r8, rbx
020e37bf: xor    edx, edx
020e37c1: mov    rcx, r14
020e37c4: call   0x18305d9d0
020e37c9: mov    dword ptr [rbp + 0xc], 0
020e37d0: test   ebx, ebx
020e37d2: jns    0x1820e37db
020e37d4: xor    ecx, ecx
020e37d6: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e37db: mov    rcx, qword ptr [rdi + 0x10]
020e37df: lea    rdx, [rbp]
020e37e3: mov    qword ptr [rbp], r14
020e37e7: xor    r8d, r8d
020e37ea: mov    dword ptr [rbp + 8], ebx
020e37ed: movaps xmm6, xmmword ptr [rbp]
020e37f1: movdqa xmmword ptr [rbp], xmm6
020e37f6: call   0x1805f18b0
020e37fb: mov    r8, qword ptr [rip + 0x1bbdc3e]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e3802: lea    rcx, [rbp]
020e3806: mov    rdx, rsi
020e3809: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e380e: movaps xmm0, xmmword ptr [rbp]
020e3812: lea    rdx, [rbp]
020e3816: mov    r8, qword ptr [rip + 0x1c262a3]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e381d: lea    rcx, [rbp + 0x10]
020e3821: movdqa xmmword ptr [rbp], xmm0
020e3826: movdqa xmmword ptr [rbp + 0x10], xmm6
020e382b: call   0x182c56fc0
020e3830: test   al, al
020e3832: je     0x1820e394b
020e3838: mov    rcx, qword ptr [rdi + 0x10]
020e383c: xor    edx, edx
020e383e: call   0x1820cf940
020e3843: cmp    eax, 0x4e
020e3846: jne    0x1820e3902
020e384c: mov    rcx, qword ptr [rdi + 0x10]
020e3850: xor    edx, edx
020e3852: call   0x1820cf940
020e3857: mov    rcx, qword ptr [rip + 0x1c2ab72]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e385e: movss  xmm6, dword ptr [rdi + 0x18]
020e3863: mov    esi, eax
020e3865: call   0x182f60c00
020e386a: mov    qword ptr [rsp + 0x50], 0
020e3873: mov    r8b, 1
020e3876: mov    byte ptr [rsp + 0x48], 1
020e387b: movaps xmm3, xmm6
020e387e: mov    dword ptr [rsp + 0x40], 1
020e3886: movzx  edx, r8b
020e388a: mov    dword ptr [rsp + 0x38], 0
020e3892: mov    rcx, rax
020e3895: mov    dword ptr [rsp + 0x30], 0xffffffff
020e389d: mov    rbx, rax
020e38a0: mov    byte ptr [rsp + 0x28], 0
020e38a5: mov    dword ptr [rsp + 0x20], 0xffffffff
020e38ad: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e38b2: test   rbx, rbx
020e38b5: je     0x1820e38fc
020e38b7: mov    rcx, qword ptr [rbx + 0x30]
020e38bb: test   rcx, rcx
020e38be: je     0x1820e38fc
020e38c0: xorps  xmm1, xmm1
020e38c3: xor    r8d, r8d
020e38c6: cvtsi2sd xmm1, rsi
020e38cb: call   0x18212ced0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʸʳʽʴʵʽʵʽʶʲʸ
020e38d0: mov    rcx, qword ptr [rbx + 0x30]
020e38d4: test   rcx, rcx
020e38d7: je     0x1820e38fc
020e38d9: movzx  r8d, byte ptr [rdi + 0x1c]
020e38de: xor    r9d, r9d
020e38e1: movzx  edx, byte ptr [rdi + 0x1d]
020e38e5: call   0x1820cfd40
020e38ea: movaps xmm6, xmmword ptr [rbp + 0x20]
020e38ee: mov    rax, rbx
020e38f1: lea    rsp, [rbp + 0x30]
020e38f5: pop    r14
020e38f7: pop    rdi
020e38f8: pop    rsi
020e38f9: pop    rbx
020e38fa: pop    rbp
020e38fb: ret    
020e38fc: call   0x182f60c50
020e3901: int3   
020e3902: lea    rcx, [rip + 0x1c02767]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e3909: call   0x182f609d0
020e390e: mov    rcx, rax
020e3911: call   0x182f60c00
020e3916: lea    rcx, [rip + 0x1c1da13]                   ; [0x3d01330] str:'[{0}] '
020e391d: mov    rbx, rax
020e3920: call   0x182f609d0
020e3925: mov    rdx, rax
020e3928: xor    r8d, r8d
020e392b: mov    rcx, rbx
020e392e: call   0x181b92e30                              ; System.Exception$$.ctor
020e3933: lea    rcx, [rip + 0x1bb9286]                   ; [0x3c9cbc0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʳʸʷʻˀʸʴˀʺʳʲ()
020e393a: call   0x182f609d0
020e393f: mov    rdx, rax
020e3942: mov    rcx, rbx
020e3945: call   0x182f60c10
020e394a: int3   
020e394b: lea    rcx, [rip + 0x1c0271e]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e3952: call   0x182f609d0
020e3957: mov    rcx, rax
020e395a: call   0x182f60c00
020e395f: lea    rcx, [rip + 0x1bf20e2]                   ; [0x3cd5a48] str:'teal'
020e3966: mov    rbx, rax
020e3969: call   0x182f609d0
020e396e: mov    rdx, rax
020e3971: xor    r8d, r8d
020e3974: mov    rcx, rbx
020e3977: call   0x181b92e30                              ; System.Exception$$.ctor
020e397c: lea    rcx, [rip + 0x1bb923d]                   ; [0x3c9cbc0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʳʸʷʻˀʸʴˀʺʳʲ()
020e3983: call   0x182f609d0
020e3988: mov    rdx, rax
020e398b: mov    rcx, rbx
020e398e: call   0x182f60c10
020e3993: int3   
020e3994: int3   
=== 20E39A0
020e39a0: push   rbx
020e39a2: push   rbp
020e39a3: push   rsi
020e39a4: push   rdi
020e39a5: push   r12
020e39a7: push   r13
020e39a9: push   r14
020e39ab: push   r15
020e39ad: sub    rsp, 0x58
020e39b1: mov    r15, rdx
020e39b4: mov    r14, rcx
020e39b7: test   rdx, rdx
020e39ba: je     0x1820e3c08
020e39c0: mov    rcx, qword ptr [rcx + 0x10]
020e39c4: xor    edx, edx
020e39c6: call   0x1820cf940
020e39cb: mov    r12d, eax
020e39ce: cmp    r12, 1
020e39d2: jbe    0x1820e3bd7
020e39d8: mov    ebp, 1
020e39dd: xor    r13d, r13d
020e39e0: cmp    byte ptr [rip + 0x1e35934], r13b         ; [0x3f1931b] (bss)
020e39e7: mov    rdi, qword ptr [r14 + 0x10]
020e39eb: jne    0x1820e3a00
020e39ed: lea    rcx, [rip + 0x1c2550c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e39f4: call   0x182f609b0
020e39f9: mov    byte ptr [rip + 0x1e3591b], 1            ; [0x3f1931b] (bss)
020e3a00: xor    ecx, ecx
020e3a02: call   0x1801bd810
020e3a07: mov    rcx, qword ptr [rip + 0x1c254f2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3a0e: mov    rbx, rax
020e3a11: cmp    qword ptr [rcx + 0x38], r13
020e3a15: jne    0x1820e3a1c
020e3a17: call   0x182f657d0
020e3a1c: mov    qword ptr [rsp + 0x28], r13
020e3a21: test   rbx, rbx
020e3a24: je     0x1820e3be8
020e3a2a: cmp    dword ptr [rbx + 0x18], 8
020e3a2e: jae    0x1820e3a37
020e3a30: xor    ecx, ecx
020e3a32: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e3a37: lea    rax, [rbx + 0x20]
020e3a3b: mov    dword ptr [rsp + 0x28], 8
020e3a43: mov    qword ptr [rsp + 0x20], rax
020e3a48: lea    rdx, [rsp + 0x40]
020e3a4d: movaps xmm0, xmmword ptr [rsp + 0x20]
020e3a52: xor    r8d, r8d
020e3a55: mov    rcx, rdi
020e3a58: movdqa xmmword ptr [rsp + 0x40], xmm0
020e3a5e: call   0x1805f18b0
020e3a63: cmp    dword ptr [rbx + 0x18], r13d
020e3a67: jbe    0x1820e3c0e
020e3a6d: cmp    byte ptr [rip + 0x1e358a7], r13b         ; [0x3f1931b] (bss)
020e3a74: mov    rsi, qword ptr [rbx + 0x20]
020e3a78: mov    rdi, qword ptr [r14 + 0x10]
020e3a7c: jne    0x1820e3a91
020e3a7e: lea    rcx, [rip + 0x1c2547b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3a85: call   0x182f609b0
020e3a8a: mov    byte ptr [rip + 0x1e3588a], 1            ; [0x3f1931b] (bss)
020e3a91: xor    ecx, ecx
020e3a93: call   0x1801bd810
020e3a98: mov    rcx, qword ptr [rip + 0x1c25461]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e3a9f: mov    rbx, rax
020e3aa2: cmp    qword ptr [rcx + 0x38], r13
020e3aa6: jne    0x1820e3aad
020e3aa8: call   0x182f657d0
020e3aad: mov    qword ptr [rsp + 0x38], r13
020e3ab2: test   rbx, rbx
020e3ab5: je     0x1820e3be8
020e3abb: cmp    dword ptr [rbx + 0x18], 8
020e3abf: jae    0x1820e3ac8
020e3ac1: xor    ecx, ecx
020e3ac3: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e3ac8: lea    rax, [rbx + 0x20]
020e3acc: mov    dword ptr [rsp + 0x38], 8
020e3ad4: mov    qword ptr [rsp + 0x30], rax
020e3ad9: lea    rdx, [rsp + 0x40]
020e3ade: movaps xmm0, xmmword ptr [rsp + 0x30]
020e3ae3: xor    r8d, r8d
020e3ae6: mov    rcx, rdi
020e3ae9: movdqa xmmword ptr [rsp + 0x40], xmm0
020e3aef: call   0x1805f18b0
020e3af4: cmp    dword ptr [rbx + 0x18], r13d
020e3af8: jbe    0x1820e3c0e
020e3afe: cmp    byte ptr [rip + 0x1e3581c], r13b         ; [0x3f19321] (bss)
020e3b05: mov    rdi, qword ptr [rbx + 0x20]
020e3b09: jne    0x1820e3b2a
020e3b0b: lea    rcx, [rip + 0x1bedcee]                   ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
020e3b12: call   0x182f609b0
020e3b17: lea    rcx, [rip + 0x1c15ef2]                   ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e3b1e: call   0x182f609b0
020e3b23: mov    byte ptr [rip + 0x1e357f7], 1            ; [0x3f19321] (bss)
020e3b2a: mov    rcx, qword ptr [r15 + 0x40]
020e3b2e: test   rcx, rcx
020e3b31: je     0x1820e3c08
020e3b37: mov    rdx, qword ptr [rip + 0x1c15ed2]         ; [0x3cf9a10] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʸʵʵʾʿˀʺʽʲʾˁ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e3b3e: mov    rbx, qword ptr [r15 + 0x78]
020e3b42: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e3b47: test   rax, rax
020e3b4a: je     0x1820e3c08
020e3b50: xor    r9d, r9d
020e3b53: mov    r8, rdi
020e3b56: mov    rdx, rsi
020e3b59: mov    rcx, rax
020e3b5c: call   0x1820d5e60                              ; ʸʵʵʾʿˀʺʽʲʾˁ$$ʹʳʾˁʺʾʹˁʻʹʲ
020e3b61: mov    r9, rax
020e3b64: test   rbx, rbx
020e3b67: je     0x1820e3c08
020e3b6d: mov    rax, qword ptr [rip + 0x1bedc8c]         ; [0x3cd1800] metamethod:Method$System.Collections.Generic.List<ʸʵʵʾʿˀʺʽʲʾˁ>.Add()
020e3b74: inc    dword ptr [rbx + 0x1c]
020e3b77: mov    rcx, qword ptr [rbx + 0x10]
020e3b7b: test   rcx, rcx
020e3b7e: je     0x1820e3c08
020e3b84: movsxd rdx, dword ptr [rbx + 0x18]
020e3b88: cmp    edx, dword ptr [rcx + 0x18]
020e3b8b: jb     0x1820e3ba9
020e3b8d: mov    rax, qword ptr [rax + 0x20]
020e3b91: mov    rdx, r9
020e3b94: mov    rcx, rbx
020e3b97: mov    r8, qword ptr [rax + 0xc0]
020e3b9e: mov    r8, qword ptr [r8 + 0x70]
020e3ba2: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e3ba7: jmp    0x1820e3bc9
020e3ba9: lea    eax, [rdx + 1]
020e3bac: mov    dword ptr [rbx + 0x18], eax
020e3baf: cmp    edx, dword ptr [rcx + 0x18]
020e3bb2: jae    0x1820e3c0e
020e3bb4: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020e3bb9: lea    rcx, [rcx + rdx*8]
020e3bbd: add    rcx, 0x20
020e3bc1: mov    rdx, r9
020e3bc4: call   0x182f5fc00
020e3bc9: inc    ebp
020e3bcb: movsxd rax, ebp
020e3bce: cmp    rax, r12
020e3bd1: jl     0x1820e39e0
020e3bd7: add    rsp, 0x58
020e3bdb: pop    r15
020e3bdd: pop    r14
020e3bdf: pop    r13
020e3be1: pop    r12
020e3be3: pop    rdi
020e3be4: pop    rsi
020e3be5: pop    rbp
020e3be6: pop    rbx
020e3be7: ret    
020e3be8: xor    ecx, ecx
020e3bea: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e3bef: xorps  xmm0, xmm0
020e3bf2: lea    rdx, [rsp + 0x40]
020e3bf7: xor    r8d, r8d
020e3bfa: movdqa xmmword ptr [rsp + 0x40], xmm0
020e3c00: mov    rcx, rdi
020e3c03: call   0x1805f18b0
020e3c08: call   0x182f60c50
020e3c0d: int3   
020e3c0e: call   0x182f60c40
020e3c13: int3   
020e3c14: int3   
=== 20E3C20
020e3c20: mov    qword ptr [rsp + 0x10], rsi
020e3c25: push   rdi
020e3c26: sub    rsp, 0x20
020e3c2a: xor    edx, edx
020e3c2c: mov    rsi, rcx
020e3c2f: call   0x1820e5130                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʹʿʳʼʴˁʺʶʳʻʸ
020e3c34: mov    rdi, rax
020e3c37: test   rax, rax
020e3c3a: je     0x1820e3cdb
020e3c40: mov    rcx, qword ptr [rax + 0x30]
020e3c44: test   rcx, rcx
020e3c47: je     0x1820e3cdb
020e3c4d: movzx  r8d, byte ptr [rsi + 0x1c]
020e3c52: xor    r9d, r9d
020e3c55: movzx  edx, byte ptr [rsi + 0x1d]
020e3c59: mov    qword ptr [rsp + 0x30], rbx
020e3c5e: call   0x1820cfd40
020e3c63: xor    r8d, r8d
020e3c66: mov    rdx, rdi
020e3c69: mov    rcx, rsi
020e3c6c: mov    rbx, rax
020e3c6f: call   0x1820e4040                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʵˁʲʹʴʷʸʾʳʵʾ
020e3c74: xor    r8d, r8d
020e3c77: mov    rdx, rdi
020e3c7a: mov    rcx, rsi
020e3c7d: call   0x1820e5940                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ˁʲʳʹʴˁʹʴʳʽʶ
020e3c82: xor    r8d, r8d
020e3c85: mov    rdx, rbx
020e3c88: mov    rcx, rsi
020e3c8b: call   0x1820e4bf0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʻʺʹʻʾʳʺʺˀʶˁ
020e3c90: xor    r8d, r8d
020e3c93: mov    rdx, rbx
020e3c96: mov    rcx, rsi
020e3c99: call   0x1820e3cf0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʴˀˀʹʹʶʷʾʴʴʵ
020e3c9e: xor    r8d, r8d
020e3ca1: mov    rdx, rbx
020e3ca4: mov    rcx, rsi
020e3ca7: call   0x1820e4200                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʶʼʲʿʸʸʾˀʶʿʶ
020e3cac: xor    r8d, r8d
020e3caf: mov    rdx, rbx
020e3cb2: mov    rcx, rsi
020e3cb5: call   0x1820e5e70                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʽˁʵˁʿʸʴʳʶʵʹ
020e3cba: xor    r8d, r8d
020e3cbd: mov    rdx, rbx
020e3cc0: mov    rcx, rsi
020e3cc3: call   0x1820e53d0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʺʴʸʶʼʺˀʶʹʶʾ
020e3cc8: mov    rsi, qword ptr [rsp + 0x38]
020e3ccd: mov    rax, rbx
020e3cd0: mov    rbx, qword ptr [rsp + 0x30]
020e3cd5: add    rsp, 0x20
020e3cd9: pop    rdi
020e3cda: ret    
020e3cdb: call   0x182f60c50
020e3ce0: int3   
020e3ce1: int3   
=== 20E4040
020e4040: push   rbx
020e4042: push   rbp
020e4043: push   rsi
020e4044: push   rdi
020e4045: push   r12
020e4047: push   r13
020e4049: push   r14
020e404b: push   r15
020e404d: sub    rsp, 0x58
020e4051: mov    rbp, rcx
020e4054: test   rdx, rdx
020e4057: je     0x1820e41ee
020e405d: mov    r14, qword ptr [rdx + 0x30]
020e4061: xor    r13d, r13d
020e4064: mov    rcx, qword ptr [rcx + 0x10]
020e4068: xor    edx, edx
020e406a: mov    r15d, r13d
020e406d: call   0x1820cf940
020e4072: mov    r12d, eax
020e4075: test   eax, eax
020e4077: je     0x1820e41bd
020e407d: nop    dword ptr [rax]
020e4080: cmp    byte ptr [rip + 0x1e35294], r13b         ; [0x3f1931b] (bss)
020e4087: mov    rdi, qword ptr [rbp + 0x10]
020e408b: jne    0x1820e40a0
020e408d: lea    rcx, [rip + 0x1c24e6c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4094: call   0x182f609b0
020e4099: mov    byte ptr [rip + 0x1e3527b], 1            ; [0x3f1931b] (bss)
020e40a0: xor    ecx, ecx
020e40a2: call   0x1801bd810
020e40a7: mov    rcx, qword ptr [rip + 0x1c24e52]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e40ae: mov    rbx, rax
020e40b1: cmp    qword ptr [rcx + 0x38], r13
020e40b5: jne    0x1820e40bc
020e40b7: call   0x182f657d0
020e40bc: mov    qword ptr [rsp + 0x28], r13
020e40c1: test   rbx, rbx
020e40c4: je     0x1820e41ce
020e40ca: cmp    dword ptr [rbx + 0x18], 8
020e40ce: jae    0x1820e40d7
020e40d0: xor    ecx, ecx
020e40d2: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e40d7: lea    rax, [rbx + 0x20]
020e40db: mov    dword ptr [rsp + 0x28], 8
020e40e3: mov    qword ptr [rsp + 0x20], rax
020e40e8: lea    rdx, [rsp + 0x40]
020e40ed: movaps xmm0, xmmword ptr [rsp + 0x20]
020e40f2: xor    r8d, r8d
020e40f5: mov    rcx, rdi
020e40f8: movdqa xmmword ptr [rsp + 0x40], xmm0
020e40fe: call   0x1805f18b0
020e4103: cmp    dword ptr [rbx + 0x18], r13d
020e4107: jbe    0x1820e41f4
020e410d: cmp    byte ptr [rip + 0x1e2ca7d], r13b         ; [0x3f10b91] (bss)
020e4114: mov    rsi, qword ptr [rbx + 0x20]
020e4118: mov    rdi, qword ptr [rbp + 0x10]
020e411c: jne    0x1820e4131
020e411e: lea    rcx, [rip + 0x1c24ddb]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4125: call   0x182f609b0
020e412a: mov    byte ptr [rip + 0x1e2ca60], 1            ; [0x3f10b91] (bss)
020e4131: xor    ecx, ecx
020e4133: call   0x1801bd810
020e4138: mov    rcx, qword ptr [rip + 0x1c24dc1]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e413f: mov    rbx, rax
020e4142: cmp    qword ptr [rcx + 0x38], r13
020e4146: jne    0x1820e414d
020e4148: call   0x182f657d0
020e414d: mov    qword ptr [rsp + 0x38], r13
020e4152: test   rbx, rbx
020e4155: je     0x1820e41ce
020e4157: cmp    dword ptr [rbx + 0x18], 8
020e415b: jae    0x1820e4164
020e415d: xor    ecx, ecx
020e415f: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4164: lea    rax, [rbx + 0x20]
020e4168: mov    dword ptr [rsp + 0x38], 8
020e4170: mov    qword ptr [rsp + 0x30], rax
020e4175: lea    rdx, [rsp + 0x40]
020e417a: movaps xmm0, xmmword ptr [rsp + 0x30]
020e417f: xor    r8d, r8d
020e4182: mov    rcx, rdi
020e4185: movdqa xmmword ptr [rsp + 0x40], xmm0
020e418b: call   0x1805f18b0
020e4190: cmp    dword ptr [rbx + 0x18], r13d
020e4194: jbe    0x1820e41f4
020e4196: test   r14, r14
020e4199: je     0x1820e41ee
020e419b: movsd  xmm2, qword ptr [rbx + 0x20]
020e41a0: xor    r9d, r9d
020e41a3: mov    rdx, rsi
020e41a6: mov    rcx, r14
020e41a9: call   0x18212cbf0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʷʿʳʹʳʲʽʺʵʻʽ
020e41ae: inc    r15d
020e41b1: movsxd rax, r15d
020e41b4: cmp    rax, r12
020e41b7: jl     0x1820e4080
020e41bd: add    rsp, 0x58
020e41c1: pop    r15
020e41c3: pop    r14
020e41c5: pop    r13
020e41c7: pop    r12
020e41c9: pop    rdi
020e41ca: pop    rsi
020e41cb: pop    rbp
020e41cc: pop    rbx
020e41cd: ret    
020e41ce: xor    ecx, ecx
020e41d0: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e41d5: xorps  xmm0, xmm0
020e41d8: lea    rdx, [rsp + 0x40]
020e41dd: xor    r8d, r8d
020e41e0: movdqa xmmword ptr [rsp + 0x40], xmm0
020e41e6: mov    rcx, rdi
020e41e9: call   0x1805f18b0
020e41ee: call   0x182f60c50
020e41f3: int3   
020e41f4: call   0x182f60c40
020e41f9: int3   
020e41fa: int3   
=== 20E4200
020e4200: push   rsi
020e4202: push   r12
020e4204: push   r14
020e4206: sub    rsp, 0x70
020e420a: mov    rsi, rdx
020e420d: mov    r14, rcx
020e4210: mov    rcx, qword ptr [rcx + 0x10]
020e4214: xor    edx, edx
020e4216: call   0x1820cf940
020e421b: mov    r12d, eax
020e421e: test   eax, eax
020e4220: je     0x1820e4587
020e4226: mov    qword ptr [rsp + 0x90], rbx
020e422e: mov    qword ptr [rsp + 0xa0], rdi
020e4236: mov    qword ptr [rsp + 0x68], r13
020e423b: xor    r13d, r13d
020e423e: mov    qword ptr [rsp + 0x60], r15
020e4243: mov    r15d, r13d
020e4246: mov    qword ptr [rsp + 0x98], rbp
020e424e: nop    
020e4250: cmp    byte ptr [rip + 0x1e350c4], r13b         ; [0x3f1931b] (bss)
020e4257: mov    rdi, qword ptr [r14 + 0x10]
020e425b: jne    0x1820e4270
020e425d: lea    rcx, [rip + 0x1c24c9c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4264: call   0x182f609b0
020e4269: mov    byte ptr [rip + 0x1e350ab], 1            ; [0x3f1931b] (bss)
020e4270: xor    ecx, ecx
020e4272: call   0x1801bd810
020e4277: mov    rcx, qword ptr [rip + 0x1c24c82]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e427e: mov    rbx, rax
020e4281: cmp    qword ptr [rcx + 0x38], r13
020e4285: jne    0x1820e428c
020e4287: call   0x182f657d0
020e428c: mov    qword ptr [rsp + 0x38], r13
020e4291: test   rbx, rbx
020e4294: je     0x1820e4591
020e429a: cmp    dword ptr [rbx + 0x18], 8
020e429e: jae    0x1820e42a7
020e42a0: xor    ecx, ecx
020e42a2: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e42a7: lea    rax, [rbx + 0x20]
020e42ab: mov    dword ptr [rsp + 0x38], 8
020e42b3: mov    qword ptr [rsp + 0x30], rax
020e42b8: lea    rdx, [rsp + 0x50]
020e42bd: movaps xmm0, xmmword ptr [rsp + 0x30]
020e42c2: xor    r8d, r8d
020e42c5: mov    rcx, rdi
020e42c8: movdqa xmmword ptr [rsp + 0x50], xmm0
020e42ce: call   0x1805f18b0
020e42d3: cmp    dword ptr [rbx + 0x18], r13d
020e42d7: jbe    0x1820e45b7
020e42dd: cmp    byte ptr [rip + 0x1e35037], r13b         ; [0x3f1931b] (bss)
020e42e4: mov    rbp, qword ptr [rbx + 0x20]
020e42e8: mov    rdi, qword ptr [r14 + 0x10]
020e42ec: jne    0x1820e4301
020e42ee: lea    rcx, [rip + 0x1c24c0b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e42f5: call   0x182f609b0
020e42fa: mov    byte ptr [rip + 0x1e3501a], 1            ; [0x3f1931b] (bss)
020e4301: xor    ecx, ecx
020e4303: call   0x1801bd810
020e4308: mov    rcx, qword ptr [rip + 0x1c24bf1]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e430f: mov    rbx, rax
020e4312: cmp    qword ptr [rcx + 0x38], r13
020e4316: jne    0x1820e431d
020e4318: call   0x182f657d0
020e431d: mov    qword ptr [rsp + 0x48], r13
020e4322: test   rbx, rbx
020e4325: je     0x1820e4591
020e432b: cmp    dword ptr [rbx + 0x18], 8
020e432f: jae    0x1820e4338
020e4331: xor    ecx, ecx
020e4333: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4338: lea    rax, [rbx + 0x20]
020e433c: mov    dword ptr [rsp + 0x48], 8
020e4344: mov    qword ptr [rsp + 0x40], rax
020e4349: lea    rdx, [rsp + 0x50]
020e434e: movaps xmm0, xmmword ptr [rsp + 0x40]
020e4353: xor    r8d, r8d
020e4356: mov    rcx, rdi
020e4359: movdqa xmmword ptr [rsp + 0x50], xmm0
020e435f: call   0x1805f18b0
020e4364: cmp    dword ptr [rbx + 0x18], r13d
020e4368: jbe    0x1820e45b7
020e436e: mov    rcx, qword ptr [r14 + 0x10]
020e4372: test   rcx, rcx
020e4375: je     0x1820e45b1
020e437b: mov    rax, qword ptr [rcx]
020e437e: mov    rbx, qword ptr [rbx + 0x20]
020e4382: mov    rdx, qword ptr [rax + 0x390]
020e4389: call   qword ptr [rax + 0x388]
020e438f: cmp    eax, 1
020e4392: sete   dil
020e4396: test   rsi, rsi
020e4399: je     0x1820e45b1
020e439f: cmp    byte ptr [rip + 0x1e34f7a], r13b         ; [0x3f19320] (bss)
020e43a6: jne    0x1820e43eb
020e43a8: lea    rcx, [rip + 0x1be9e51]                   ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
020e43af: call   0x182f609b0
020e43b4: lea    rcx, [rip + 0x1bea085]                   ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
020e43bb: call   0x182f609b0
020e43c0: lea    rcx, [rip + 0x1bea139]                   ; [0x3cce500] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Count()
020e43c7: call   0x182f609b0
020e43cc: lea    rcx, [rip + 0x1bea1ed]                   ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020e43d3: call   0x182f609b0
020e43d8: lea    rcx, [rip + 0x1c14261]                   ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e43df: call   0x182f609b0
020e43e4: mov    byte ptr [rip + 0x1e34f35], 1            ; [0x3f19320] (bss)
020e43eb: mov    rcx, qword ptr [rsi + 0x38]
020e43ef: test   rcx, rcx
020e43f2: je     0x1820e45b1
020e43f8: mov    rdx, qword ptr [rip + 0x1c14241]         ; [0x3cf8640] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʳʷʸʽʳʹʶˁʺʳʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e43ff: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e4404: test   rax, rax
020e4407: je     0x1820e45b1
020e440d: mov    qword ptr [rsp + 0x28], r13
020e4412: movzx  r9d, dil
020e4416: mov    r8, rbx
020e4419: mov    byte ptr [rsp + 0x20], 0xff
020e441e: mov    rdx, rbp
020e4421: mov    rcx, rax
020e4424: call   0x182116390                              ; ʳʷʸʽʳʹʶˁʺʳʿ$$ʾʵʺʼʳʾʴˀʷʷʳ
020e4429: mov    rdi, rax
020e442c: test   rax, rax
020e442f: je     0x1820e45b1
020e4435: cmp    byte ptr [rax + 0x21], r13b
020e4439: jne    0x1820e44ea
020e443f: mov    rbx, qword ptr [rsi + 0x80]
020e4446: test   rbx, rbx
020e4449: je     0x1820e45b1
020e444f: mov    ebx, dword ptr [rbx + 0x18]
020e4452: sub    ebx, 1
020e4455: js     0x1820e44ea
020e445b: mov    r8, qword ptr [rip + 0x1bea15e]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020e4462: mov    edx, ebx
020e4464: mov    rcx, qword ptr [rsi + 0x80]
020e446b: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020e4470: test   rax, rax
020e4473: je     0x1820e45b1
020e4479: mov    rcx, qword ptr [rdi + 0x10]
020e447d: cmp    qword ptr [rax + 0x10], rcx
020e4481: jne    0x1820e44ea
020e4483: cmp    byte ptr [rax + 0x21], r13b
020e4487: je     0x1820e44ea
020e4489: test   ebx, ebx
020e448b: je     0x1820e44c7
020e448d: nop    dword ptr [rax]
020e4490: mov    rcx, qword ptr [rsi + 0x80]
020e4497: test   rcx, rcx
020e449a: je     0x1820e45b1
020e44a0: mov    r8, qword ptr [rip + 0x1bea119]          ; [0x3cce5c0] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.get_Item()
020e44a7: lea    edx, [rbx - 1]
020e44aa: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020e44af: test   rax, rax
020e44b2: je     0x1820e45b1
020e44b8: mov    rcx, qword ptr [rdi + 0x10]
020e44bc: cmp    qword ptr [rax + 0x10], rcx
020e44c0: jne    0x1820e44c7
020e44c2: sub    ebx, 1
020e44c5: jne    0x1820e4490
020e44c7: mov    rcx, qword ptr [rsi + 0x80]
020e44ce: test   rcx, rcx
020e44d1: je     0x1820e45b1
020e44d7: mov    r9, qword ptr [rip + 0x1be9f62]          ; [0x3cce440] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Insert()
020e44de: mov    r8, rdi
020e44e1: mov    edx, ebx
020e44e3: call   0x1807746c0                              ; System.Collections.Generic.List<object>$$Insert
020e44e8: jmp    0x1820e4556
020e44ea: mov    rcx, qword ptr [rsi + 0x80]
020e44f1: test   rcx, rcx
020e44f4: je     0x1820e45b1
020e44fa: mov    r9, qword ptr [rip + 0x1be9cff]          ; [0x3cce200] metamethod:Method$System.Collections.Generic.List<ʳʷʸʽʳʹʶˁʺʳʿ>.Add()
020e4501: inc    dword ptr [rcx + 0x1c]
020e4504: mov    rdx, qword ptr [rcx + 0x10]
020e4508: test   rdx, rdx
020e450b: je     0x1820e45b1
020e4511: movsxd r8, dword ptr [rcx + 0x18]
020e4515: cmp    r8d, dword ptr [rdx + 0x18]
020e4519: jb     0x1820e4534
020e451b: mov    rax, qword ptr [r9 + 0x20]
020e451f: mov    rdx, rdi
020e4522: mov    r8, qword ptr [rax + 0xc0]
020e4529: mov    r8, qword ptr [r8 + 0x70]
020e452d: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e4532: jmp    0x1820e4556
020e4534: lea    eax, [r8 + 1]
020e4538: mov    dword ptr [rcx + 0x18], eax
020e453b: cmp    r8d, dword ptr [rdx + 0x18]
020e453f: jae    0x1820e45b7
020e4541: mov    qword ptr [rdx + r8*8 + 0x20], rdi
020e4546: add    rdx, 0x20
020e454a: lea    rcx, [rdx + r8*8]
020e454e: mov    rdx, rdi
020e4551: call   0x182f5fc00
020e4556: inc    r15d
020e4559: movsxd rax, r15d
020e455c: cmp    rax, r12
020e455f: jl     0x1820e4250
020e4565: mov    rbp, qword ptr [rsp + 0x98]
020e456d: mov    rdi, qword ptr [rsp + 0xa0]
020e4575: mov    r13, qword ptr [rsp + 0x68]
020e457a: mov    rbx, qword ptr [rsp + 0x90]
020e4582: mov    r15, qword ptr [rsp + 0x60]
020e4587: add    rsp, 0x70
020e458b: pop    r14
020e458d: pop    r12
020e458f: pop    rsi
020e4590: ret    
020e4591: xor    ecx, ecx
020e4593: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4598: xorps  xmm0, xmm0
020e459b: lea    rdx, [rsp + 0x50]
020e45a0: xor    r8d, r8d
020e45a3: movdqa xmmword ptr [rsp + 0x50], xmm0
020e45a9: mov    rcx, rdi
020e45ac: call   0x1805f18b0
020e45b1: call   0x182f60c50
020e45b6: int3   
020e45b7: call   0x182f60c40
020e45bc: int3   
020e45bd: int3   
=== 20E45C0
020e45c0: push   rbp
020e45c2: push   rbx
020e45c3: push   rsi
020e45c4: push   rdi
020e45c5: push   r12
020e45c7: push   r13
020e45c9: push   r14
020e45cb: push   r15
020e45cd: lea    rbp, [rsp - 0x1f]
020e45d2: sub    rsp, 0x88
020e45d9: mov    r15, rdx
020e45dc: mov    byte ptr [rbp + 0x67], 1
020e45e0: mov    r13, rcx
020e45e3: xor    edx, edx
020e45e5: mov    rcx, qword ptr [rcx + 0x10]
020e45e9: call   0x1820cf940
020e45ee: test   eax, eax
020e45f0: je     0x1820e4974
020e45f6: xor    r14d, r14d
020e45f9: nop    dword ptr [rax]
020e4600: cmp    byte ptr [rip + 0x1e34d14], 0            ; [0x3f1931b] (bss)
020e4607: mov    rdi, qword ptr [r13 + 0x10]
020e460b: jne    0x1820e4620
020e460d: lea    rcx, [rip + 0x1c248ec]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4614: call   0x182f609b0
020e4619: mov    byte ptr [rip + 0x1e34cfb], 1            ; [0x3f1931b] (bss)
020e4620: xor    ecx, ecx
020e4622: call   0x1801bd810
020e4627: mov    rcx, qword ptr [rip + 0x1c248d2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e462e: mov    rbx, rax
020e4631: cmp    qword ptr [rcx + 0x38], 0
020e4636: jne    0x1820e463d
020e4638: call   0x182f657d0
020e463d: mov    qword ptr [rbp - 0x31], r14
020e4641: test   rbx, rbx
020e4644: je     0x1820e49a1
020e464a: cmp    dword ptr [rbx + 0x18], 8
020e464e: jae    0x1820e4657
020e4650: xor    ecx, ecx
020e4652: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4657: lea    rax, [rbx + 0x20]
020e465b: mov    dword ptr [rbp - 0x31], 8
020e4662: mov    qword ptr [rbp - 0x39], rax
020e4666: lea    rdx, [rbp + 7]
020e466a: movaps xmm0, xmmword ptr [rbp - 0x39]
020e466e: xor    r8d, r8d
020e4671: mov    rcx, rdi
020e4674: movdqa xmmword ptr [rbp + 7], xmm0
020e4679: call   0x1805f18b0
020e467e: cmp    dword ptr [rbx + 0x18], 0
020e4682: jbe    0x1820e49c5
020e4688: cmp    byte ptr [rip + 0x1e34c8c], 0            ; [0x3f1931b] (bss)
020e468f: mov    r12, qword ptr [rbx + 0x20]
020e4693: mov    rdi, qword ptr [r13 + 0x10]
020e4697: jne    0x1820e46ac
020e4699: lea    rcx, [rip + 0x1c24860]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e46a0: call   0x182f609b0
020e46a5: mov    byte ptr [rip + 0x1e34c6f], 1            ; [0x3f1931b] (bss)
020e46ac: xor    ecx, ecx
020e46ae: call   0x1801bd810
020e46b3: mov    rcx, qword ptr [rip + 0x1c24846]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e46ba: mov    rbx, rax
020e46bd: cmp    qword ptr [rcx + 0x38], 0
020e46c2: jne    0x1820e46c9
020e46c4: call   0x182f657d0
020e46c9: mov    qword ptr [rbp - 0x21], r14
020e46cd: test   rbx, rbx
020e46d0: je     0x1820e49a1
020e46d6: cmp    dword ptr [rbx + 0x18], 8
020e46da: jae    0x1820e46e3
020e46dc: xor    ecx, ecx
020e46de: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e46e3: lea    rax, [rbx + 0x20]
020e46e7: mov    dword ptr [rbp - 0x21], 8
020e46ee: mov    qword ptr [rbp - 0x29], rax
020e46f2: lea    rdx, [rbp + 7]
020e46f6: movaps xmm0, xmmword ptr [rbp - 0x29]
020e46fa: xor    r8d, r8d
020e46fd: mov    rcx, rdi
020e4700: movdqa xmmword ptr [rbp + 7], xmm0
020e4705: call   0x1805f18b0
020e470a: cmp    dword ptr [rbx + 0x18], 0
020e470e: jbe    0x1820e49c5
020e4714: cmp    byte ptr [rip + 0x1e34bff], 0            ; [0x3f1931a] (bss)
020e471b: mov    rsi, qword ptr [rbx + 0x20]
020e471f: mov    rdi, qword ptr [r13 + 0x10]
020e4723: jne    0x1820e4738
020e4725: lea    rcx, [rip + 0x1c247d4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e472c: call   0x182f609b0
020e4731: mov    byte ptr [rip + 0x1e34be2], 1            ; [0x3f1931a] (bss)
020e4738: xor    ecx, ecx
020e473a: call   0x1801bd810
020e473f: mov    rcx, qword ptr [rip + 0x1c247ba]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4746: mov    rbx, rax
020e4749: cmp    qword ptr [rcx + 0x38], 0
020e474e: jne    0x1820e4755
020e4750: call   0x182f657d0
020e4755: mov    qword ptr [rbp - 0x11], r14
020e4759: test   rbx, rbx
020e475c: je     0x1820e49a1
020e4762: cmp    dword ptr [rbx + 0x18], 4
020e4766: jae    0x1820e476f
020e4768: xor    ecx, ecx
020e476a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e476f: lea    rax, [rbx + 0x20]
020e4773: mov    dword ptr [rbp - 0x11], 4
020e477a: mov    qword ptr [rbp - 0x19], rax
020e477e: lea    rdx, [rbp + 7]
020e4782: movaps xmm0, xmmword ptr [rbp - 0x19]
020e4786: xor    r8d, r8d
020e4789: mov    rcx, rdi
020e478c: movdqa xmmword ptr [rbp + 7], xmm0
020e4791: call   0x1805f18b0
020e4796: cmp    dword ptr [rbx + 0x18], 0
020e479a: jbe    0x1820e49c5
020e47a0: cmp    byte ptr [rip + 0x1e34b73], 0            ; [0x3f1931a] (bss)
020e47a7: mov    r14d, dword ptr [rbx + 0x20]
020e47ab: mov    rdi, qword ptr [r13 + 0x10]
020e47af: jne    0x1820e47c4
020e47b1: lea    rcx, [rip + 0x1c24748]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e47b8: call   0x182f609b0
020e47bd: mov    byte ptr [rip + 0x1e34b56], 1            ; [0x3f1931a] (bss)
020e47c4: xor    ecx, ecx
020e47c6: call   0x1801bd810
020e47cb: mov    rcx, qword ptr [rip + 0x1c2472e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e47d2: mov    rbx, rax
020e47d5: cmp    qword ptr [rcx + 0x38], 0
020e47da: jne    0x1820e47e1
020e47dc: call   0x182f657d0
020e47e1: mov    qword ptr [rbp - 1], 0
020e47e9: test   rbx, rbx
020e47ec: je     0x1820e49a1
020e47f2: cmp    dword ptr [rbx + 0x18], 4
020e47f6: jae    0x1820e47ff
020e47f8: xor    ecx, ecx
020e47fa: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e47ff: lea    rax, [rbx + 0x20]
020e4803: mov    dword ptr [rbp - 1], 4
020e480a: mov    qword ptr [rbp - 9], rax
020e480e: lea    rdx, [rbp + 7]
020e4812: movaps xmm0, xmmword ptr [rbp - 9]
020e4816: xor    r8d, r8d
020e4819: mov    rcx, rdi
020e481c: movdqa xmmword ptr [rbp + 7], xmm0
020e4821: call   0x1805f18b0
020e4826: cmp    dword ptr [rbx + 0x18], 0
020e482a: jbe    0x1820e49c5
020e4830: test   r15, r15
020e4833: je     0x1820e49bf
020e4839: cmp    byte ptr [rip + 0x1e34ade], 0            ; [0x3f1931e] (bss)
020e4840: mov    ebx, dword ptr [rbx + 0x20]
020e4843: jne    0x1820e4870
020e4845: lea    rcx, [rip + 0x1bf2c94]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e484c: call   0x182f609b0
020e4851: lea    rcx, [rip + 0x1bf2ec8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e4858: call   0x182f609b0
020e485d: lea    rcx, [rip + 0x1c1634c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e4864: call   0x182f609b0
020e4869: mov    byte ptr [rip + 0x1e34aae], 1            ; [0x3f1931e] (bss)
020e4870: mov    rax, qword ptr [r15 + 0x50]
020e4874: test   rax, rax
020e4877: je     0x1820e49bf
020e487d: movsxd rcx, dword ptr [rax + 0x114]
020e4884: xor    eax, eax
020e4886: cmp    rsi, rcx
020e4889: cmovle rsi, rax
020e488d: mov    rcx, qword ptr [r15 + 0x10]
020e4891: test   rcx, rcx
020e4894: je     0x1820e49bf
020e489a: mov    rdx, qword ptr [rip + 0x1c1630f]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e48a1: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e48a6: test   rax, rax
020e48a9: je     0x1820e49bf
020e48af: mov    qword ptr [rsp + 0x28], 0
020e48b8: mov    r9, rsi
020e48bb: mov    r8d, r14d
020e48be: mov    dword ptr [rsp + 0x20], ebx
020e48c2: mov    rdx, r12
020e48c5: mov    rcx, rax
020e48c8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e48cd: mov    rcx, qword ptr [r15 + 0x60]
020e48d1: mov    r9, rax
020e48d4: test   rcx, rcx
020e48d7: je     0x1820e49bf
020e48dd: test   rax, rax
020e48e0: je     0x1820e49bf
020e48e6: mov    ecx, dword ptr [rcx + 0x18]
020e48e9: mov    dword ptr [rax + 0x28], ecx
020e48ec: mov    rcx, qword ptr [r15 + 0x60]
020e48f0: test   rcx, rcx
020e48f3: je     0x1820e49bf
020e48f9: mov    r10, qword ptr [rip + 0x1bf2be0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e4900: inc    dword ptr [rcx + 0x1c]
020e4903: mov    rdx, qword ptr [rcx + 0x10]
020e4907: test   rdx, rdx
020e490a: je     0x1820e49bf
020e4910: movsxd r8, dword ptr [rcx + 0x18]
020e4914: cmp    r8d, dword ptr [rdx + 0x18]
020e4918: jb     0x1820e4933
020e491a: mov    rax, qword ptr [r10 + 0x20]
020e491e: mov    rdx, r9
020e4921: mov    r8, qword ptr [rax + 0xc0]
020e4928: mov    r8, qword ptr [r8 + 0x70]
020e492c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e4931: jmp    0x1820e4959
020e4933: lea    eax, [r8 + 1]
020e4937: mov    dword ptr [rcx + 0x18], eax
020e493a: cmp    r8d, dword ptr [rdx + 0x18]
020e493e: jae    0x1820e49c5
020e4944: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e4949: add    rdx, 0x20
020e494d: lea    rcx, [rdx + r8*8]
020e4951: mov    rdx, r9
020e4954: call   0x182f5fc00
020e4959: movzx  eax, byte ptr [rbp + 0x67]
020e495d: mov    r14d, 0
020e4963: test   bl, 0x54
020e4966: mov    ecx, r14d
020e4969: cmove  ecx, eax
020e496c: mov    byte ptr [rbp + 0x67], cl
020e496f: jmp    0x1820e4600
020e4974: test   r15, r15
020e4977: je     0x1820e49bf
020e4979: xor    r8d, r8d
020e497c: mov    byte ptr [r15 + 0xa2], 1
020e4984: xor    edx, edx
020e4986: mov    rcx, r15
020e4989: add    rsp, 0x88
020e4990: pop    r15
020e4992: pop    r14
020e4994: pop    r13
020e4996: pop    r12
020e4998: pop    rdi
020e4999: pop    rsi
020e499a: pop    rbx
020e499b: pop    rbp
020e499c: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e49a1: xor    ecx, ecx
020e49a3: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e49a8: xorps  xmm0, xmm0
020e49ab: lea    rdx, [rbp + 7]
020e49af: xor    r8d, r8d
020e49b2: movdqa xmmword ptr [rbp + 7], xmm0
020e49b7: mov    rcx, rdi
020e49ba: call   0x1805f18b0
020e49bf: call   0x182f60c50
020e49c4: int3   
020e49c5: call   0x182f60c40
020e49ca: int3   
020e49cb: int3   
=== 20E49D0
020e49d0: push   rdi
020e49d2: push   r12
020e49d4: push   r14
020e49d6: push   r15
020e49d8: sub    rsp, 0x68
020e49dc: mov    r15, rdx
020e49df: mov    rdi, rcx
020e49e2: mov    rcx, qword ptr [rcx + 0x10]
020e49e6: xor    edx, edx
020e49e8: mov    r14d, 1
020e49ee: call   0x1820cf940
020e49f3: mov    r12d, eax
020e49f6: cmp    r12, r14
020e49f9: jbe    0x1820e4baa
020e49ff: mov    qword ptr [rsp + 0x90], rbx
020e4a07: mov    qword ptr [rsp + 0xa0], rsi
020e4a0f: mov    qword ptr [rsp + 0x60], r13
020e4a14: xor    r13d, r13d
020e4a17: mov    qword ptr [rsp + 0x98], rbp
020e4a1f: nop    
020e4a20: cmp    byte ptr [rip + 0x1e348f4], r13b         ; [0x3f1931b] (bss)
020e4a27: mov    rsi, qword ptr [rdi + 0x10]
020e4a2b: jne    0x1820e4a40
020e4a2d: lea    rcx, [rip + 0x1c244cc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4a34: call   0x182f609b0
020e4a39: mov    byte ptr [rip + 0x1e348db], 1            ; [0x3f1931b] (bss)
020e4a40: xor    ecx, ecx
020e4a42: call   0x1801bd810
020e4a47: mov    rcx, qword ptr [rip + 0x1c244b2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4a4e: mov    rbx, rax
020e4a51: cmp    qword ptr [rcx + 0x38], r13
020e4a55: jne    0x1820e4a5c
020e4a57: call   0x182f657d0
020e4a5c: mov    qword ptr [rsp + 0x38], r13
020e4a61: test   rbx, rbx
020e4a64: je     0x1820e4bb6
020e4a6a: cmp    dword ptr [rbx + 0x18], 8
020e4a6e: jae    0x1820e4a77
020e4a70: xor    ecx, ecx
020e4a72: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4a77: lea    rax, [rbx + 0x20]
020e4a7b: mov    dword ptr [rsp + 0x38], 8
020e4a83: mov    qword ptr [rsp + 0x30], rax
020e4a88: lea    rdx, [rsp + 0x50]
020e4a8d: movaps xmm0, xmmword ptr [rsp + 0x30]
020e4a92: xor    r8d, r8d
020e4a95: mov    rcx, rsi
020e4a98: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4a9e: call   0x1805f18b0
020e4aa3: cmp    dword ptr [rbx + 0x18], r13d
020e4aa7: jbe    0x1820e4bdc
020e4aad: cmp    byte ptr [rip + 0x1e34867], r13b         ; [0x3f1931b] (bss)
020e4ab4: mov    rbp, qword ptr [rbx + 0x20]
020e4ab8: mov    rsi, qword ptr [rdi + 0x10]
020e4abc: jne    0x1820e4ad1
020e4abe: lea    rcx, [rip + 0x1c2443b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4ac5: call   0x182f609b0
020e4aca: mov    byte ptr [rip + 0x1e3484a], 1            ; [0x3f1931b] (bss)
020e4ad1: xor    ecx, ecx
020e4ad3: call   0x1801bd810
020e4ad8: mov    rcx, qword ptr [rip + 0x1c24421]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4adf: mov    rbx, rax
020e4ae2: cmp    qword ptr [rcx + 0x38], r13
020e4ae6: jne    0x1820e4aed
020e4ae8: call   0x182f657d0
020e4aed: mov    qword ptr [rsp + 0x48], r13
020e4af2: test   rbx, rbx
020e4af5: je     0x1820e4bb6
020e4afb: cmp    dword ptr [rbx + 0x18], 8
020e4aff: jae    0x1820e4b08
020e4b01: xor    ecx, ecx
020e4b03: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4b08: lea    rax, [rbx + 0x20]
020e4b0c: mov    dword ptr [rsp + 0x48], 8
020e4b14: mov    qword ptr [rsp + 0x40], rax
020e4b19: lea    rdx, [rsp + 0x50]
020e4b1e: movaps xmm0, xmmword ptr [rsp + 0x40]
020e4b23: xor    r8d, r8d
020e4b26: mov    rcx, rsi
020e4b29: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4b2f: call   0x1805f18b0
020e4b34: cmp    dword ptr [rbx + 0x18], r13d
020e4b38: jbe    0x1820e4bdc
020e4b3e: mov    rcx, qword ptr [rdi + 0x10]
020e4b42: test   rcx, rcx
020e4b45: je     0x1820e4bd6
020e4b4b: mov    rax, qword ptr [rcx]
020e4b4e: mov    rbx, qword ptr [rbx + 0x20]
020e4b52: mov    rdx, qword ptr [rax + 0x390]
020e4b59: call   qword ptr [rax + 0x388]
020e4b5f: cmp    eax, 1
020e4b62: sete   r9b
020e4b66: test   r15, r15
020e4b69: je     0x1820e4bd6
020e4b6b: mov    r8, rbx
020e4b6e: mov    qword ptr [rsp + 0x20], r13
020e4b73: mov    rdx, rbp
020e4b76: mov    rcx, r15
020e4b79: call   0x18215b7c0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʲʲʲʿʴʴʴʷʲʽʳ
020e4b7e: inc    r14d
020e4b81: movsxd rax, r14d
020e4b84: cmp    rax, r12
020e4b87: jl     0x1820e4a20
020e4b8d: mov    rbp, qword ptr [rsp + 0x98]
020e4b95: mov    rsi, qword ptr [rsp + 0xa0]
020e4b9d: mov    rbx, qword ptr [rsp + 0x90]
020e4ba5: mov    r13, qword ptr [rsp + 0x60]
020e4baa: add    rsp, 0x68
020e4bae: pop    r15
020e4bb0: pop    r14
020e4bb2: pop    r12
020e4bb4: pop    rdi
020e4bb5: ret    
020e4bb6: xor    ecx, ecx
020e4bb8: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4bbd: xorps  xmm0, xmm0
020e4bc0: lea    rdx, [rsp + 0x50]
020e4bc5: xor    r8d, r8d
020e4bc8: movdqa xmmword ptr [rsp + 0x50], xmm0
020e4bce: mov    rcx, rsi
020e4bd1: call   0x1805f18b0
020e4bd6: call   0x182f60c50
020e4bdb: int3   
020e4bdc: call   0x182f60c40
020e4be1: int3   
020e4be2: int3   
=== 20E4BF0
020e4bf0: push   r12
020e4bf2: push   r14
020e4bf4: push   r15
020e4bf6: sub    rsp, 0x60
020e4bfa: mov    r14, rdx
020e4bfd: mov    r15, rcx
020e4c00: mov    rcx, qword ptr [rcx + 0x10]
020e4c04: xor    edx, edx
020e4c06: call   0x1820cf940
020e4c0b: mov    r12d, eax
020e4c0e: test   eax, eax
020e4c10: je     0x1820e4e59
020e4c16: mov    qword ptr [rsp + 0x80], rbx
020e4c1e: mov    qword ptr [rsp + 0x88], rbp
020e4c26: mov    qword ptr [rsp + 0x58], rdi
020e4c2b: mov    qword ptr [rsp + 0x50], r13
020e4c30: xor    r13d, r13d
020e4c33: mov    ebp, r13d
020e4c36: mov    qword ptr [rsp + 0x90], rsi
020e4c3e: nop    
020e4c40: cmp    byte ptr [rip + 0x1e346d4], r13b         ; [0x3f1931b] (bss)
020e4c47: mov    rdi, qword ptr [r15 + 0x10]
020e4c4b: jne    0x1820e4c60
020e4c4d: lea    rcx, [rip + 0x1c242ac]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4c54: call   0x182f609b0
020e4c59: mov    byte ptr [rip + 0x1e346bb], 1            ; [0x3f1931b] (bss)
020e4c60: xor    ecx, ecx
020e4c62: call   0x1801bd810
020e4c67: mov    rcx, qword ptr [rip + 0x1c24292]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4c6e: mov    rbx, rax
020e4c71: cmp    qword ptr [rcx + 0x38], r13
020e4c75: jne    0x1820e4c7c
020e4c77: call   0x182f657d0
020e4c7c: mov    qword ptr [rsp + 0x28], r13
020e4c81: test   rbx, rbx
020e4c84: je     0x1820e4e64
020e4c8a: cmp    dword ptr [rbx + 0x18], 8
020e4c8e: jae    0x1820e4c97
020e4c90: xor    ecx, ecx
020e4c92: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4c97: lea    rax, [rbx + 0x20]
020e4c9b: mov    dword ptr [rsp + 0x28], 8
020e4ca3: mov    qword ptr [rsp + 0x20], rax
020e4ca8: lea    rdx, [rsp + 0x40]
020e4cad: movaps xmm0, xmmword ptr [rsp + 0x20]
020e4cb2: xor    r8d, r8d
020e4cb5: mov    rcx, rdi
020e4cb8: movdqa xmmword ptr [rsp + 0x40], xmm0
020e4cbe: call   0x1805f18b0
020e4cc3: cmp    dword ptr [rbx + 0x18], r13d
020e4cc7: jbe    0x1820e4e8a
020e4ccd: cmp    byte ptr [rip + 0x1e34647], r13b         ; [0x3f1931b] (bss)
020e4cd4: mov    rsi, qword ptr [rbx + 0x20]
020e4cd8: mov    rdi, qword ptr [r15 + 0x10]
020e4cdc: jne    0x1820e4cf1
020e4cde: lea    rcx, [rip + 0x1c2421b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4ce5: call   0x182f609b0
020e4cea: mov    byte ptr [rip + 0x1e3462a], 1            ; [0x3f1931b] (bss)
020e4cf1: xor    ecx, ecx
020e4cf3: call   0x1801bd810
020e4cf8: mov    rcx, qword ptr [rip + 0x1c24201]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e4cff: mov    rbx, rax
020e4d02: cmp    qword ptr [rcx + 0x38], r13
020e4d06: jne    0x1820e4d0d
020e4d08: call   0x182f657d0
020e4d0d: mov    qword ptr [rsp + 0x38], r13
020e4d12: test   rbx, rbx
020e4d15: je     0x1820e4e64
020e4d1b: cmp    dword ptr [rbx + 0x18], 8
020e4d1f: jae    0x1820e4d28
020e4d21: xor    ecx, ecx
020e4d23: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4d28: lea    rax, [rbx + 0x20]
020e4d2c: mov    dword ptr [rsp + 0x38], 8
020e4d34: mov    qword ptr [rsp + 0x30], rax
020e4d39: lea    rdx, [rsp + 0x40]
020e4d3e: movaps xmm0, xmmword ptr [rsp + 0x30]
020e4d43: xor    r8d, r8d
020e4d46: mov    rcx, rdi
020e4d49: movdqa xmmword ptr [rsp + 0x40], xmm0
020e4d4f: call   0x1805f18b0
020e4d54: cmp    dword ptr [rbx + 0x18], r13d
020e4d58: jbe    0x1820e4e8a
020e4d5e: test   r14, r14
020e4d61: je     0x1820e4e84
020e4d67: cmp    byte ptr [rip + 0x1e345ae], r13b         ; [0x3f1931c] (bss)
020e4d6e: mov    rdi, qword ptr [rbx + 0x20]
020e4d72: jne    0x1820e4d93
020e4d74: lea    rcx, [rip + 0x1bea985]                   ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
020e4d7b: call   0x182f609b0
020e4d80: lea    rcx, [rip + 0x1c14241]                   ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e4d87: call   0x182f609b0
020e4d8c: mov    byte ptr [rip + 0x1e34589], 1            ; [0x3f1931c] (bss)
020e4d93: mov    rcx, qword ptr [r14 + 0x48]
020e4d97: test   rcx, rcx
020e4d9a: je     0x1820e4e84
020e4da0: mov    rdx, qword ptr [rip + 0x1c14221]         ; [0x3cf8fc8] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʵʷʳˁʶʺʼʲʵʴʴ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e4da7: mov    rbx, qword ptr [r14 + 0x70]
020e4dab: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e4db0: mov    r9, rax
020e4db3: test   rax, rax
020e4db6: je     0x1820e4e84
020e4dbc: mov    qword ptr [rax + 0x10], rsi
020e4dc0: mov    qword ptr [rax + 0x18], rdi
020e4dc4: test   rbx, rbx
020e4dc7: je     0x1820e4e84
020e4dcd: mov    rax, qword ptr [rip + 0x1bea92c]         ; [0x3ccf700] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.Add()
020e4dd4: inc    dword ptr [rbx + 0x1c]
020e4dd7: mov    rcx, qword ptr [rbx + 0x10]
020e4ddb: test   rcx, rcx
020e4dde: je     0x1820e4e84
020e4de4: movsxd rdx, dword ptr [rbx + 0x18]
020e4de8: cmp    edx, dword ptr [rcx + 0x18]
020e4deb: jb     0x1820e4e09
020e4ded: mov    rax, qword ptr [rax + 0x20]
020e4df1: mov    rdx, r9
020e4df4: mov    rcx, rbx
020e4df7: mov    r8, qword ptr [rax + 0xc0]
020e4dfe: mov    r8, qword ptr [r8 + 0x70]
020e4e02: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e4e07: jmp    0x1820e4e29
020e4e09: lea    eax, [rdx + 1]
020e4e0c: mov    dword ptr [rbx + 0x18], eax
020e4e0f: cmp    edx, dword ptr [rcx + 0x18]
020e4e12: jae    0x1820e4e8a
020e4e14: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020e4e19: lea    rcx, [rcx + rdx*8]
020e4e1d: add    rcx, 0x20
020e4e21: mov    rdx, r9
020e4e24: call   0x182f5fc00
020e4e29: inc    ebp
020e4e2b: movsxd rax, ebp
020e4e2e: cmp    rax, r12
020e4e31: jl     0x1820e4c40
020e4e37: mov    rsi, qword ptr [rsp + 0x90]
020e4e3f: mov    rdi, qword ptr [rsp + 0x58]
020e4e44: mov    rbp, qword ptr [rsp + 0x88]
020e4e4c: mov    rbx, qword ptr [rsp + 0x80]
020e4e54: mov    r13, qword ptr [rsp + 0x50]
020e4e59: add    rsp, 0x60
020e4e5d: pop    r15
020e4e5f: pop    r14
020e4e61: pop    r12
020e4e63: ret    
020e4e64: xor    ecx, ecx
020e4e66: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4e6b: xorps  xmm0, xmm0
020e4e6e: lea    rdx, [rsp + 0x40]
020e4e73: xor    r8d, r8d
020e4e76: movdqa xmmword ptr [rsp + 0x40], xmm0
020e4e7c: mov    rcx, rdi
020e4e7f: call   0x1805f18b0
020e4e84: call   0x182f60c50
020e4e89: int3   
020e4e8a: call   0x182f60c40
020e4e8f: int3   
020e4e90: push   rbp
020e4e92: push   rbx
020e4e93: push   rsi
020e4e94: push   rdi
020e4e95: push   r14
020e4e97: push   r15
020e4e99: sub    rsp, 0x98
020e4ea0: lea    rbp, [rsp + 0x60]
020e4ea5: cmp    byte ptr [rip + 0x1e34401], 0            ; [0x3f192ad] (bss)
020e4eac: mov    rdi, rcx
020e4eaf: movaps xmmword ptr [rbp + 0x20], xmm6
020e4eb3: jne    0x1820e4ef8
020e4eb5: lea    rcx, [rip + 0x1c24c04]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4ebc: call   0x182f609b0
020e4ec1: lea    rcx, [rip + 0x1bbc578]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4ec8: call   0x182f609b0
020e4ecd: lea    rcx, [rip + 0x1bc3d34]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e4ed4: call   0x182f609b0
020e4ed9: lea    rcx, [rip + 0x1c294f0]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e4ee0: call   0x182f609b0
020e4ee5: lea    rcx, [rip + 0x1bb8314]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eec: call   0x182f609b0
020e4ef1: mov    byte ptr [rip + 0x1e343b5], 1            ; [0x3f192ad] (bss)
020e4ef8: mov    rax, qword ptr [rip + 0x1bb8301]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eff: cmp    dword ptr [rax + 0xe0], 0
020e4f06: jne    0x1820e4f17
020e4f08: mov    rcx, rax
020e4f0b: call   0x182f60cf0
020e4f10: mov    rax, qword ptr [rip + 0x1bb82e9]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4f17: mov    rax, qword ptr [rax + 0xb8]
020e4f1e: mov    rsi, qword ptr [rax]
020e4f21: test   rsi, rsi
020e4f24: je     0x1820e5094
020e4f2a: movsxd rbx, dword ptr [rsi + 0x18]
020e4f2e: xor    r15d, r15d
020e4f31: test   ebx, ebx
020e4f33: je     0x1820e4f5e
020e4f35: lea    rcx, [rbx + 0xf]
020e4f39: cmp    rcx, rbx
020e4f3c: ja     0x1820e4f48
020e4f3e: movabs rcx, 0xffffffffffffff0
020e4f48: and    rcx, 0xfffffffffffffff0
020e4f4c: mov    rax, rcx
020e4f4f: call   0x18302b960
020e4f54: sub    rsp, rcx
020e4f57: lea    r14, [rsp + 0x60]
020e4f5c: jmp    0x1820e4f61
020e4f5e: mov    r14, r15
020e4f61: mov    r8, rbx
020e4f64: xor    edx, edx
020e4f66: mov    rcx, r14
020e4f69: call   0x18305d9d0
020e4f6e: mov    dword ptr [rbp + 0xc], r15d
020e4f72: test   ebx, ebx
020e4f74: jns    0x1820e4f7d
020e4f76: xor    ecx, ecx
020e4f78: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4f7d: mov    rcx, qword ptr [rdi + 0x10]
020e4f81: lea    rdx, [rbp]
020e4f85: mov    qword ptr [rbp], r14
020e4f89: xor    r8d, r8d
020e4f8c: mov    dword ptr [rbp + 8], ebx
020e4f8f: movaps xmm6, xmmword ptr [rbp]
020e4f93: movdqa xmmword ptr [rbp], xmm6
020e4f98: call   0x1805f18b0
020e4f9d: mov    r8, qword ptr [rip + 0x1bbc49c]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4fa4: lea    rcx, [rbp]
020e4fa8: mov    rdx, rsi
020e4fab: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e4fb0: movaps xmm0, xmmword ptr [rbp]
020e4fb4: lea    rdx, [rbp]
020e4fb8: mov    r8, qword ptr [rip + 0x1c24b01]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4fbf: lea    rcx, [rbp + 0x10]
020e4fc3: movdqa xmmword ptr [rbp], xmm0
020e4fc8: movdqa xmmword ptr [rbp + 0x10], xmm6
020e4fcd: call   0x182c56fc0
020e4fd2: test   al, al
020e4fd4: je     0x1820e50e3
020e4fda: mov    rcx, qword ptr [rdi + 0x10]
020e4fde: xor    edx, edx
020e4fe0: call   0x1820cf940
020e4fe5: cmp    eax, -0x3d
020e4fe8: jne    0x1820e509a
020e4fee: mov    rcx, qword ptr [rdi + 0x10]
020e4ff2: xor    edx, edx
020e4ff4: call   0x1820cf940
020e4ff9: mov    rcx, qword ptr [rip + 0x1c293d0]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e5000: movss  xmm6, dword ptr [rdi + 0x18]
020e5005: mov    esi, eax
020e5007: call   0x182f60c00
020e500c: mov    qword ptr [rsp + 0x50], r15
020e5011: movaps xmm3, xmm6
020e5014: mov    byte ptr [rsp + 0x48], 1
020e5019: xor    r8d, r8d
020e501c: mov    dword ptr [rsp + 0x40], r15d
020e5021: mov    dl, 1
020e5023: mov    dword ptr [rsp + 0x38], r15d
020e5028: mov    rcx, rax
020e502b: mov    dword ptr [rsp + 0x30], 0xffffffff
020e5033: mov    rbx, rax
020e5036: mov    byte ptr [rsp + 0x28], 1
020e503b: mov    dword ptr [rsp + 0x20], 0xffffffff
020e5043: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e5048: test   rbx, rbx
020e504b: je     0x1820e5094
020e504d: mov    rcx, qword ptr [rbx + 0x30]
020e5051: test   rcx, rcx
020e5054: je     0x1820e5094
020e5056: xorps  xmm1, xmm1
020e5059: xor    r8d, r8d
020e505c: cvtsi2sd xmm1, rsi
020e5061: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e5066: mov    rcx, qword ptr [rbx + 0x30]
020e506a: test   rcx, rcx
020e506d: je     0x1820e5094
020e506f: movzx  r8d, byte ptr [rdi + 0x1c]
020e5074: xor    r9d, r9d
020e5077: movzx  edx, byte ptr [rdi + 0x1d]
020e507b: call   0x18212b860                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʹʵʺʲˀˁʽʳˁʹ
020e5080: movaps xmm6, xmmword ptr [rbp + 0x20]
020e5084: mov    rax, rbx
020e5087: lea    rsp, [rbp + 0x38]
020e508b: pop    r15
020e508d: pop    r14
020e508f: pop    rdi
020e5090: pop    rsi
020e5091: pop    rbx
020e5092: pop    rbp
020e5093: ret    
020e5094: call   0x182f60c50
020e5099: int3   
020e509a: lea    rcx, [rip + 0x1c00fcf]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50a1: call   0x182f609d0
020e50a6: mov    rcx, rax
020e50a9: call   0x182f60c00
020e50ae: lea    rcx, [rip + 0x1bbca6b]                   ; [0x3ca1b20] str:'All Taps'
020e50b5: mov    rbx, rax
020e50b8: call   0x182f609d0
020e50bd: mov    rdx, rax
020e50c0: xor    r8d, r8d
020e50c3: mov    rcx, rbx
020e50c6: call   0x181b92e30                              ; System.Exception$$.ctor
020e50cb: lea    rcx, [rip + 0x1bb7ba6]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e50d2: call   0x182f609d0
020e50d7: mov    rdx, rax
020e50da: mov    rcx, rbx
020e50dd: call   0x182f60c10
020e50e2: int3   
020e50e3: lea    rcx, [rip + 0x1c00f86]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50ea: call   0x182f609d0
020e50ef: mov    rcx, rax
020e50f2: call   0x182f60c00
020e50f7: lea    rcx, [rip + 0x1bfd02a]                   ; [0x3ce2128] str:'+'
020e50fe: mov    rbx, rax
020e5101: call   0x182f609d0
020e5106: mov    rdx, rax
020e5109: xor    r8d, r8d
020e510c: mov    rcx, rbx
020e510f: call   0x181b92e30                              ; System.Exception$$.ctor
020e5114: lea    rcx, [rip + 0x1bb7b5d]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e511b: call   0x182f609d0
020e5120: mov    rdx, rax
020e5123: mov    rcx, rbx
020e5126: call   0x182f60c10
020e512b: int3   
020e512c: int3   
=== 20E4E90
020e4e90: push   rbp
020e4e92: push   rbx
020e4e93: push   rsi
020e4e94: push   rdi
020e4e95: push   r14
020e4e97: push   r15
020e4e99: sub    rsp, 0x98
020e4ea0: lea    rbp, [rsp + 0x60]
020e4ea5: cmp    byte ptr [rip + 0x1e34401], 0            ; [0x3f192ad] (bss)
020e4eac: mov    rdi, rcx
020e4eaf: movaps xmmword ptr [rbp + 0x20], xmm6
020e4eb3: jne    0x1820e4ef8
020e4eb5: lea    rcx, [rip + 0x1c24c04]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4ebc: call   0x182f609b0
020e4ec1: lea    rcx, [rip + 0x1bbc578]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4ec8: call   0x182f609b0
020e4ecd: lea    rcx, [rip + 0x1bc3d34]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e4ed4: call   0x182f609b0
020e4ed9: lea    rcx, [rip + 0x1c294f0]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e4ee0: call   0x182f609b0
020e4ee5: lea    rcx, [rip + 0x1bb8314]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eec: call   0x182f609b0
020e4ef1: mov    byte ptr [rip + 0x1e343b5], 1            ; [0x3f192ad] (bss)
020e4ef8: mov    rax, qword ptr [rip + 0x1bb8301]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4eff: cmp    dword ptr [rax + 0xe0], 0
020e4f06: jne    0x1820e4f17
020e4f08: mov    rcx, rax
020e4f0b: call   0x182f60cf0
020e4f10: mov    rax, qword ptr [rip + 0x1bb82e9]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e4f17: mov    rax, qword ptr [rax + 0xb8]
020e4f1e: mov    rsi, qword ptr [rax]
020e4f21: test   rsi, rsi
020e4f24: je     0x1820e5094
020e4f2a: movsxd rbx, dword ptr [rsi + 0x18]
020e4f2e: xor    r15d, r15d
020e4f31: test   ebx, ebx
020e4f33: je     0x1820e4f5e
020e4f35: lea    rcx, [rbx + 0xf]
020e4f39: cmp    rcx, rbx
020e4f3c: ja     0x1820e4f48
020e4f3e: movabs rcx, 0xffffffffffffff0
020e4f48: and    rcx, 0xfffffffffffffff0
020e4f4c: mov    rax, rcx
020e4f4f: call   0x18302b960
020e4f54: sub    rsp, rcx
020e4f57: lea    r14, [rsp + 0x60]
020e4f5c: jmp    0x1820e4f61
020e4f5e: mov    r14, r15
020e4f61: mov    r8, rbx
020e4f64: xor    edx, edx
020e4f66: mov    rcx, r14
020e4f69: call   0x18305d9d0
020e4f6e: mov    dword ptr [rbp + 0xc], r15d
020e4f72: test   ebx, ebx
020e4f74: jns    0x1820e4f7d
020e4f76: xor    ecx, ecx
020e4f78: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e4f7d: mov    rcx, qword ptr [rdi + 0x10]
020e4f81: lea    rdx, [rbp]
020e4f85: mov    qword ptr [rbp], r14
020e4f89: xor    r8d, r8d
020e4f8c: mov    dword ptr [rbp + 8], ebx
020e4f8f: movaps xmm6, xmmword ptr [rbp]
020e4f93: movdqa xmmword ptr [rbp], xmm6
020e4f98: call   0x1805f18b0
020e4f9d: mov    r8, qword ptr [rip + 0x1bbc49c]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e4fa4: lea    rcx, [rbp]
020e4fa8: mov    rdx, rsi
020e4fab: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e4fb0: movaps xmm0, xmmword ptr [rbp]
020e4fb4: lea    rdx, [rbp]
020e4fb8: mov    r8, qword ptr [rip + 0x1c24b01]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e4fbf: lea    rcx, [rbp + 0x10]
020e4fc3: movdqa xmmword ptr [rbp], xmm0
020e4fc8: movdqa xmmword ptr [rbp + 0x10], xmm6
020e4fcd: call   0x182c56fc0
020e4fd2: test   al, al
020e4fd4: je     0x1820e50e3
020e4fda: mov    rcx, qword ptr [rdi + 0x10]
020e4fde: xor    edx, edx
020e4fe0: call   0x1820cf940
020e4fe5: cmp    eax, -0x3d
020e4fe8: jne    0x1820e509a
020e4fee: mov    rcx, qword ptr [rdi + 0x10]
020e4ff2: xor    edx, edx
020e4ff4: call   0x1820cf940
020e4ff9: mov    rcx, qword ptr [rip + 0x1c293d0]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e5000: movss  xmm6, dword ptr [rdi + 0x18]
020e5005: mov    esi, eax
020e5007: call   0x182f60c00
020e500c: mov    qword ptr [rsp + 0x50], r15
020e5011: movaps xmm3, xmm6
020e5014: mov    byte ptr [rsp + 0x48], 1
020e5019: xor    r8d, r8d
020e501c: mov    dword ptr [rsp + 0x40], r15d
020e5021: mov    dl, 1
020e5023: mov    dword ptr [rsp + 0x38], r15d
020e5028: mov    rcx, rax
020e502b: mov    dword ptr [rsp + 0x30], 0xffffffff
020e5033: mov    rbx, rax
020e5036: mov    byte ptr [rsp + 0x28], 1
020e503b: mov    dword ptr [rsp + 0x20], 0xffffffff
020e5043: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e5048: test   rbx, rbx
020e504b: je     0x1820e5094
020e504d: mov    rcx, qword ptr [rbx + 0x30]
020e5051: test   rcx, rcx
020e5054: je     0x1820e5094
020e5056: xorps  xmm1, xmm1
020e5059: xor    r8d, r8d
020e505c: cvtsi2sd xmm1, rsi
020e5061: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e5066: mov    rcx, qword ptr [rbx + 0x30]
020e506a: test   rcx, rcx
020e506d: je     0x1820e5094
020e506f: movzx  r8d, byte ptr [rdi + 0x1c]
020e5074: xor    r9d, r9d
020e5077: movzx  edx, byte ptr [rdi + 0x1d]
020e507b: call   0x18212b860                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʹʵʺʲˀˁʽʳˁʹ
020e5080: movaps xmm6, xmmword ptr [rbp + 0x20]
020e5084: mov    rax, rbx
020e5087: lea    rsp, [rbp + 0x38]
020e508b: pop    r15
020e508d: pop    r14
020e508f: pop    rdi
020e5090: pop    rsi
020e5091: pop    rbx
020e5092: pop    rbp
020e5093: ret    
020e5094: call   0x182f60c50
020e5099: int3   
020e509a: lea    rcx, [rip + 0x1c00fcf]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50a1: call   0x182f609d0
020e50a6: mov    rcx, rax
020e50a9: call   0x182f60c00
020e50ae: lea    rcx, [rip + 0x1bbca6b]                   ; [0x3ca1b20] str:'All Taps'
020e50b5: mov    rbx, rax
020e50b8: call   0x182f609d0
020e50bd: mov    rdx, rax
020e50c0: xor    r8d, r8d
020e50c3: mov    rcx, rbx
020e50c6: call   0x181b92e30                              ; System.Exception$$.ctor
020e50cb: lea    rcx, [rip + 0x1bb7ba6]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e50d2: call   0x182f609d0
020e50d7: mov    rdx, rax
020e50da: mov    rcx, rbx
020e50dd: call   0x182f60c10
020e50e2: int3   
020e50e3: lea    rcx, [rip + 0x1c00f86]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e50ea: call   0x182f609d0
020e50ef: mov    rcx, rax
020e50f2: call   0x182f60c00
020e50f7: lea    rcx, [rip + 0x1bfd02a]                   ; [0x3ce2128] str:'+'
020e50fe: mov    rbx, rax
020e5101: call   0x182f609d0
020e5106: mov    rdx, rax
020e5109: xor    r8d, r8d
020e510c: mov    rcx, rbx
020e510f: call   0x181b92e30                              ; System.Exception$$.ctor
020e5114: lea    rcx, [rip + 0x1bb7b5d]                   ; [0x3c9cc78] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʴʷʳʷʲʺˁʾʻˁ()
020e511b: call   0x182f609d0
020e5120: mov    rdx, rax
020e5123: mov    rcx, rbx
020e5126: call   0x182f60c10
020e512b: int3   
020e512c: int3   
=== 20E5130
020e5130: push   rbp
020e5132: push   rbx
020e5133: push   rsi
020e5134: push   rdi
020e5135: push   r14
020e5137: push   r15
020e5139: sub    rsp, 0x98
020e5140: lea    rbp, [rsp + 0x60]
020e5145: cmp    byte ptr [rip + 0x1e34162], 0            ; [0x3f192ae] (bss)
020e514c: mov    rdi, rcx
020e514f: movaps xmmword ptr [rbp + 0x20], xmm6
020e5153: jne    0x1820e5198
020e5155: lea    rcx, [rip + 0x1c24964]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e515c: call   0x182f609b0
020e5161: lea    rcx, [rip + 0x1bbc2d8]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e5168: call   0x182f609b0
020e516d: lea    rcx, [rip + 0x1bc3a94]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e5174: call   0x182f609b0
020e5179: lea    rcx, [rip + 0x1c29250]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e5180: call   0x182f609b0
020e5185: lea    rcx, [rip + 0x1bb8074]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e518c: call   0x182f609b0
020e5191: mov    byte ptr [rip + 0x1e34116], 1            ; [0x3f192ae] (bss)
020e5198: mov    rax, qword ptr [rip + 0x1bb8061]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e519f: cmp    dword ptr [rax + 0xe0], 0
020e51a6: jne    0x1820e51b7
020e51a8: mov    rcx, rax
020e51ab: call   0x182f60cf0
020e51b0: mov    rax, qword ptr [rip + 0x1bb8049]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e51b7: mov    rax, qword ptr [rax + 0xb8]
020e51be: mov    rsi, qword ptr [rax]
020e51c1: test   rsi, rsi
020e51c4: je     0x1820e5336
020e51ca: movsxd rbx, dword ptr [rsi + 0x18]
020e51ce: xor    r15d, r15d
020e51d1: test   ebx, ebx
020e51d3: je     0x1820e51fe
020e51d5: lea    rcx, [rbx + 0xf]
020e51d9: cmp    rcx, rbx
020e51dc: ja     0x1820e51e8
020e51de: movabs rcx, 0xffffffffffffff0
020e51e8: and    rcx, 0xfffffffffffffff0
020e51ec: mov    rax, rcx
020e51ef: call   0x18302b960
020e51f4: sub    rsp, rcx
020e51f7: lea    r14, [rsp + 0x60]
020e51fc: jmp    0x1820e5201
020e51fe: mov    r14, r15
020e5201: mov    r8, rbx
020e5204: xor    edx, edx
020e5206: mov    rcx, r14
020e5209: call   0x18305d9d0
020e520e: mov    dword ptr [rbp + 0xc], r15d
020e5212: test   ebx, ebx
020e5214: jns    0x1820e521d
020e5216: xor    ecx, ecx
020e5218: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e521d: mov    rcx, qword ptr [rdi + 0x10]
020e5221: lea    rdx, [rbp]
020e5225: mov    qword ptr [rbp], r14
020e5229: xor    r8d, r8d
020e522c: mov    dword ptr [rbp + 8], ebx
020e522f: movaps xmm6, xmmword ptr [rbp]
020e5233: movdqa xmmword ptr [rbp], xmm6
020e5238: call   0x1805f18b0
020e523d: mov    r8, qword ptr [rip + 0x1bbc1fc]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e5244: lea    rcx, [rbp]
020e5248: mov    rdx, rsi
020e524b: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e5250: movaps xmm0, xmmword ptr [rbp]
020e5254: lea    rdx, [rbp]
020e5258: mov    r8, qword ptr [rip + 0x1c24861]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e525f: lea    rcx, [rbp + 0x10]
020e5263: movdqa xmmword ptr [rbp], xmm0
020e5268: movdqa xmmword ptr [rbp + 0x10], xmm6
020e526d: call   0x182c56fc0
020e5272: test   al, al
020e5274: je     0x1820e5385
020e527a: mov    rcx, qword ptr [rdi + 0x10]
020e527e: xor    edx, edx
020e5280: call   0x1820cf940
020e5285: cmp    eax, 0x134d7c0
020e528a: jne    0x1820e533c
020e5290: mov    rcx, qword ptr [rdi + 0x10]
020e5294: xor    edx, edx
020e5296: call   0x1820cf940
020e529b: mov    rcx, qword ptr [rip + 0x1c2912e]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e52a2: movss  xmm6, dword ptr [rdi + 0x18]
020e52a7: mov    esi, eax
020e52a9: call   0x182f60c00
020e52ae: mov    qword ptr [rsp + 0x50], r15
020e52b3: movaps xmm3, xmm6
020e52b6: mov    byte ptr [rsp + 0x48], r15b
020e52bb: xor    r8d, r8d
020e52be: mov    dword ptr [rsp + 0x40], r15d
020e52c3: xor    edx, edx
020e52c5: mov    dword ptr [rsp + 0x38], r15d
020e52ca: mov    rcx, rax
020e52cd: mov    dword ptr [rsp + 0x30], 0xffffffff
020e52d5: mov    rbx, rax
020e52d8: mov    byte ptr [rsp + 0x28], r15b
020e52dd: mov    dword ptr [rsp + 0x20], 0xffffffff
020e52e5: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e52ea: test   rbx, rbx
020e52ed: je     0x1820e5336
020e52ef: mov    rcx, qword ptr [rbx + 0x30]
020e52f3: test   rcx, rcx
020e52f6: je     0x1820e5336
020e52f8: xorps  xmm1, xmm1
020e52fb: xor    r8d, r8d
020e52fe: cvtsi2sd xmm1, rsi
020e5303: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e5308: mov    rcx, qword ptr [rbx + 0x30]
020e530c: test   rcx, rcx
020e530f: je     0x1820e5336
020e5311: movzx  r8d, byte ptr [rdi + 0x1c]
020e5316: xor    r9d, r9d
020e5319: movzx  edx, byte ptr [rdi + 0x1d]
020e531d: call   0x1820cfd40
020e5322: movaps xmm6, xmmword ptr [rbp + 0x20]
020e5326: mov    rax, rbx
020e5329: lea    rsp, [rbp + 0x38]
020e532d: pop    r15
020e532f: pop    r14
020e5331: pop    rdi
020e5332: pop    rsi
020e5333: pop    rbx
020e5334: pop    rbp
020e5335: ret    
020e5336: call   0x182f60c50
020e533b: int3   
020e533c: lea    rcx, [rip + 0x1c00d2d]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e5343: call   0x182f609d0
020e5348: mov    rcx, rax
020e534b: call   0x182f60c00
020e5350: lea    rcx, [rip + 0x1bf2d29]                   ; [0x3cd8080] str:'Unsupported file version'
020e5357: mov    rbx, rax
020e535a: call   0x182f609d0
020e535f: mov    rdx, rax
020e5362: xor    r8d, r8d
020e5365: mov    rcx, rbx
020e5368: call   0x181b92e30                              ; System.Exception$$.ctor
020e536d: lea    rcx, [rip + 0x1bb79b4]                   ; [0x3c9cd28] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʿʳʼʴˁʺʶʳʻʸ()
020e5374: call   0x182f609d0
020e5379: mov    rdx, rax
020e537c: mov    rcx, rbx
020e537f: call   0x182f60c10
020e5384: int3   
020e5385: lea    rcx, [rip + 0x1c00ce4]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e538c: call   0x182f609d0
020e5391: mov    rcx, rax
020e5394: call   0x182f60c00
020e5399: lea    rcx, [rip + 0x1bbce50]                   ; [0x3ca21f0] str:'Invalid file format'
020e53a0: mov    rbx, rax
020e53a3: call   0x182f609d0
020e53a8: mov    rdx, rax
020e53ab: xor    r8d, r8d
020e53ae: mov    rcx, rbx
020e53b1: call   0x181b92e30                              ; System.Exception$$.ctor
020e53b6: lea    rcx, [rip + 0x1bb796b]                   ; [0x3c9cd28] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ʹʿʳʼʴˁʺʶʳʻʸ()
020e53bd: call   0x182f609d0
020e53c2: mov    rdx, rax
020e53c5: mov    rcx, rbx
020e53c8: call   0x182f60c10
020e53cd: int3   
020e53ce: int3   
=== 20E53D0
020e53d0: mov    qword ptr [rsp + 0x10], rbx
020e53d5: push   rbp
020e53d6: push   rsi
020e53d7: push   rdi
020e53d8: push   r12
020e53da: push   r13
020e53dc: push   r14
020e53de: push   r15
020e53e0: lea    rbp, [rsp - 0x27]
020e53e5: sub    rsp, 0x90
020e53ec: mov    r15, rdx
020e53ef: mov    r13, rcx
020e53f2: mov    rcx, qword ptr [rcx + 0x10]
020e53f6: xor    edx, edx
020e53f8: call   0x1820cf940
020e53fd: mov    eax, eax
020e53ff: mov    qword ptr [rbp - 0x39], rax
020e5403: test   rax, rax
020e5406: je     0x1820e57aa
020e540c: xor    r14d, r14d
020e540f: mov    byte ptr [rbp + 0x67], 0
020e5413: mov    dword ptr [rbp + 0x7f], r14d
020e5417: nop    word ptr [rax + rax]
020e5420: cmp    byte ptr [rip + 0x1e33ef4], 0            ; [0x3f1931b] (bss)
020e5427: mov    rdi, qword ptr [r13 + 0x10]
020e542b: jne    0x1820e5440
020e542d: lea    rcx, [rip + 0x1c23acc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5434: call   0x182f609b0
020e5439: mov    byte ptr [rip + 0x1e33edb], 1            ; [0x3f1931b] (bss)
020e5440: xor    ecx, ecx
020e5442: call   0x1801bd810
020e5447: mov    rcx, qword ptr [rip + 0x1c23ab2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e544e: mov    rbx, rax
020e5451: cmp    qword ptr [rcx + 0x38], 0
020e5456: jne    0x1820e545d
020e5458: call   0x182f657d0
020e545d: mov    qword ptr [rbp - 0x21], r14
020e5461: test   rbx, rbx
020e5464: je     0x1820e57df
020e546a: cmp    dword ptr [rbx + 0x18], 8
020e546e: jae    0x1820e5477
020e5470: xor    ecx, ecx
020e5472: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5477: lea    rax, [rbx + 0x20]
020e547b: mov    dword ptr [rbp - 0x21], 8
020e5482: mov    qword ptr [rbp - 0x29], rax
020e5486: lea    rdx, [rbp + 0x17]
020e548a: movaps xmm0, xmmword ptr [rbp - 0x29]
020e548e: xor    r8d, r8d
020e5491: mov    rcx, rdi
020e5494: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5499: call   0x1805f18b0
020e549e: cmp    dword ptr [rbx + 0x18], 0
020e54a2: jbe    0x1820e5803
020e54a8: cmp    byte ptr [rip + 0x1e33e6c], 0            ; [0x3f1931b] (bss)
020e54af: mov    r12, qword ptr [rbx + 0x20]
020e54b3: mov    rdi, qword ptr [r13 + 0x10]
020e54b7: jne    0x1820e54cc
020e54b9: lea    rcx, [rip + 0x1c23a40]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e54c0: call   0x182f609b0
020e54c5: mov    byte ptr [rip + 0x1e33e4f], 1            ; [0x3f1931b] (bss)
020e54cc: xor    ecx, ecx
020e54ce: call   0x1801bd810
020e54d3: mov    rcx, qword ptr [rip + 0x1c23a26]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e54da: mov    rbx, rax
020e54dd: cmp    qword ptr [rcx + 0x38], 0
020e54e2: jne    0x1820e54e9
020e54e4: call   0x182f657d0
020e54e9: mov    qword ptr [rbp - 0x11], r14
020e54ed: test   rbx, rbx
020e54f0: je     0x1820e57df
020e54f6: cmp    dword ptr [rbx + 0x18], 8
020e54fa: jae    0x1820e5503
020e54fc: xor    ecx, ecx
020e54fe: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5503: lea    rax, [rbx + 0x20]
020e5507: mov    dword ptr [rbp - 0x11], 8
020e550e: mov    qword ptr [rbp - 0x19], rax
020e5512: lea    rdx, [rbp + 0x17]
020e5516: movaps xmm0, xmmword ptr [rbp - 0x19]
020e551a: xor    r8d, r8d
020e551d: mov    rcx, rdi
020e5520: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5525: call   0x1805f18b0
020e552a: cmp    dword ptr [rbx + 0x18], 0
020e552e: jbe    0x1820e5803
020e5534: cmp    byte ptr [rip + 0x1e33ddf], 0            ; [0x3f1931a] (bss)
020e553b: mov    rsi, qword ptr [rbx + 0x20]
020e553f: mov    rdi, qword ptr [r13 + 0x10]
020e5543: jne    0x1820e5558
020e5545: lea    rcx, [rip + 0x1c239b4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e554c: call   0x182f609b0
020e5551: mov    byte ptr [rip + 0x1e33dc2], 1            ; [0x3f1931a] (bss)
020e5558: xor    ecx, ecx
020e555a: call   0x1801bd810
020e555f: mov    rcx, qword ptr [rip + 0x1c2399a]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5566: mov    rbx, rax
020e5569: cmp    qword ptr [rcx + 0x38], 0
020e556e: jne    0x1820e5575
020e5570: call   0x182f657d0
020e5575: mov    qword ptr [rbp - 1], r14
020e5579: test   rbx, rbx
020e557c: je     0x1820e57df
020e5582: cmp    dword ptr [rbx + 0x18], 4
020e5586: jae    0x1820e558f
020e5588: xor    ecx, ecx
020e558a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e558f: lea    rax, [rbx + 0x20]
020e5593: mov    dword ptr [rbp - 1], 4
020e559a: mov    qword ptr [rbp - 9], rax
020e559e: lea    rdx, [rbp + 0x17]
020e55a2: movaps xmm0, xmmword ptr [rbp - 9]
020e55a6: xor    r8d, r8d
020e55a9: mov    rcx, rdi
020e55ac: movdqa xmmword ptr [rbp + 0x17], xmm0
020e55b1: call   0x1805f18b0
020e55b6: cmp    dword ptr [rbx + 0x18], 0
020e55ba: jbe    0x1820e5803
020e55c0: cmp    byte ptr [rip + 0x1e33d53], 0            ; [0x3f1931a] (bss)
020e55c7: mov    r14d, dword ptr [rbx + 0x20]
020e55cb: mov    rdi, qword ptr [r13 + 0x10]
020e55cf: jne    0x1820e55e4
020e55d1: lea    rcx, [rip + 0x1c23928]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e55d8: call   0x182f609b0
020e55dd: mov    byte ptr [rip + 0x1e33d36], 1            ; [0x3f1931a] (bss)
020e55e4: xor    ecx, ecx
020e55e6: call   0x1801bd810
020e55eb: mov    rcx, qword ptr [rip + 0x1c2390e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e55f2: mov    rbx, rax
020e55f5: cmp    qword ptr [rcx + 0x38], 0
020e55fa: jne    0x1820e5601
020e55fc: call   0x182f657d0
020e5601: mov    qword ptr [rbp + 0xf], 0
020e5609: test   rbx, rbx
020e560c: je     0x1820e57df
020e5612: cmp    dword ptr [rbx + 0x18], 4
020e5616: jae    0x1820e561f
020e5618: xor    ecx, ecx
020e561a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e561f: lea    rax, [rbx + 0x20]
020e5623: mov    dword ptr [rbp + 0xf], 4
020e562a: mov    qword ptr [rbp + 7], rax
020e562e: lea    rdx, [rbp + 0x17]
020e5632: movaps xmm0, xmmword ptr [rbp + 7]
020e5636: xor    r8d, r8d
020e5639: mov    rcx, rdi
020e563c: movdqa xmmword ptr [rbp + 0x17], xmm0
020e5641: call   0x1805f18b0
020e5646: cmp    dword ptr [rbx + 0x18], 0
020e564a: jbe    0x1820e5803
020e5650: test   r15, r15
020e5653: je     0x1820e57fd
020e5659: cmp    byte ptr [rip + 0x1e33cbe], 0            ; [0x3f1931e] (bss)
020e5660: mov    ebx, dword ptr [rbx + 0x20]
020e5663: jne    0x1820e5690
020e5665: lea    rcx, [rip + 0x1bf1e74]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e566c: call   0x182f609b0
020e5671: lea    rcx, [rip + 0x1bf20a8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e5678: call   0x182f609b0
020e567d: lea    rcx, [rip + 0x1c1552c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e5684: call   0x182f609b0
020e5689: mov    byte ptr [rip + 0x1e33c8e], 1            ; [0x3f1931e] (bss)
020e5690: mov    rax, qword ptr [r15 + 0x50]
020e5694: test   rax, rax
020e5697: je     0x1820e57fd
020e569d: movsxd rcx, dword ptr [rax + 0x114]
020e56a4: xor    eax, eax
020e56a6: cmp    rsi, rcx
020e56a9: cmovle rsi, rax
020e56ad: mov    rcx, qword ptr [r15 + 0x10]
020e56b1: test   rcx, rcx
020e56b4: je     0x1820e57fd
020e56ba: mov    rdx, qword ptr [rip + 0x1c154ef]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e56c1: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e56c6: test   rax, rax
020e56c9: je     0x1820e57fd
020e56cf: mov    qword ptr [rsp + 0x28], 0
020e56d8: mov    r9, rsi
020e56db: mov    r8d, r14d
020e56de: mov    dword ptr [rsp + 0x20], ebx
020e56e2: mov    rdx, r12
020e56e5: mov    rcx, rax
020e56e8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e56ed: mov    rcx, qword ptr [r15 + 0x60]
020e56f1: mov    r9, rax
020e56f4: test   rcx, rcx
020e56f7: je     0x1820e57fd
020e56fd: test   rax, rax
020e5700: je     0x1820e57fd
020e5706: mov    ecx, dword ptr [rcx + 0x18]
020e5709: mov    dword ptr [rax + 0x28], ecx
020e570c: mov    rcx, qword ptr [r15 + 0x60]
020e5710: test   rcx, rcx
020e5713: je     0x1820e57fd
020e5719: mov    r10, qword ptr [rip + 0x1bf1dc0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e5720: inc    dword ptr [rcx + 0x1c]
020e5723: mov    rdx, qword ptr [rcx + 0x10]
020e5727: test   rdx, rdx
020e572a: je     0x1820e57fd
020e5730: movsxd r8, dword ptr [rcx + 0x18]
020e5734: cmp    r8d, dword ptr [rdx + 0x18]
020e5738: jb     0x1820e5753
020e573a: mov    rax, qword ptr [r10 + 0x20]
020e573e: mov    rdx, r9
020e5741: mov    r8, qword ptr [rax + 0xc0]
020e5748: mov    r8, qword ptr [r8 + 0x70]
020e574c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e5751: jmp    0x1820e5779
020e5753: lea    eax, [r8 + 1]
020e5757: mov    dword ptr [rcx + 0x18], eax
020e575a: cmp    r8d, dword ptr [rdx + 0x18]
020e575e: jae    0x1820e5803
020e5764: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e5769: add    rdx, 0x20
020e576d: lea    rcx, [rdx + r8*8]
020e5771: mov    rdx, r9
020e5774: call   0x182f5fc00
020e5779: mov    ecx, dword ptr [rbp + 0x67]
020e577c: test   bl, 0x20
020e577f: movzx  ecx, cl
020e5782: mov    eax, 1
020e5787: cmovne ecx, eax
020e578a: mov    dword ptr [rbp + 0x67], ecx
020e578d: mov    edx, dword ptr [rbp + 0x7f]
020e5790: mov    r14d, 0
020e5796: inc    edx
020e5798: movsxd rax, edx
020e579b: mov    dword ptr [rbp + 0x7f], edx
020e579e: cmp    rax, qword ptr [rbp - 0x39]
020e57a2: jl     0x1820e5420
020e57a8: jmp    0x1820e57b1
020e57aa: xor    cl, cl
020e57ac: test   r15, r15
020e57af: je     0x1820e57fd
020e57b1: mov    byte ptr [r15 + 0xa2], cl
020e57b8: xor    r8d, r8d
020e57bb: mov    rcx, r15
020e57be: xor    edx, edx
020e57c0: mov    rbx, qword ptr [rsp + 0xd8]
020e57c8: add    rsp, 0x90
020e57cf: pop    r15
020e57d1: pop    r14
020e57d3: pop    r13
020e57d5: pop    r12
020e57d7: pop    rdi
020e57d8: pop    rsi
020e57d9: pop    rbp
020e57da: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e57df: xor    ecx, ecx
020e57e1: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e57e6: xorps  xmm0, xmm0
020e57e9: lea    rdx, [rbp + 0x17]
020e57ed: xor    r8d, r8d
020e57f0: movdqa xmmword ptr [rbp + 0x17], xmm0
020e57f5: mov    rcx, rdi
020e57f8: call   0x1805f18b0
020e57fd: call   0x182f60c50
020e5802: int3   
020e5803: call   0x182f60c40
020e5808: int3   
020e5809: int3   
=== 20E5940
020e5940: push   rbp
020e5942: push   rbx
020e5943: push   rsi
020e5944: push   rdi
020e5945: push   r12
020e5947: push   r13
020e5949: push   r14
020e594b: push   r15
020e594d: mov    rbp, rsp
020e5950: sub    rsp, 0x78
020e5954: mov    r14, rcx
020e5957: test   rdx, rdx
020e595a: je     0x1820e5b86
020e5960: mov    r12, qword ptr [rdx + 0x30]
020e5964: xor    esi, esi
020e5966: mov    rcx, qword ptr [rcx + 0x10]
020e596a: xor    edx, edx
020e596c: mov    r13d, esi
020e596f: call   0x1820cf940
020e5974: mov    eax, eax
020e5976: mov    qword ptr [rbp + 0x50], rax
020e597a: test   rax, rax
020e597d: je     0x1820e5b57
020e5983: cmp    byte ptr [rip + 0x1e33991], 0            ; [0x3f1931b] (bss)
020e598a: mov    rdi, qword ptr [r14 + 0x10]
020e598e: jne    0x1820e59a3
020e5990: lea    rcx, [rip + 0x1c23569]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5997: call   0x182f609b0
020e599c: mov    byte ptr [rip + 0x1e33978], 1            ; [0x3f1931b] (bss)
020e59a3: xor    ecx, ecx
020e59a5: call   0x1801bd810
020e59aa: mov    rcx, qword ptr [rip + 0x1c2354f]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e59b1: mov    rbx, rax
020e59b4: cmp    qword ptr [rcx + 0x38], 0
020e59b9: jne    0x1820e59c0
020e59bb: call   0x182f657d0
020e59c0: mov    qword ptr [rbp - 0x40], rsi
020e59c4: test   rbx, rbx
020e59c7: je     0x1820e5b68
020e59cd: cmp    dword ptr [rbx + 0x18], 8
020e59d1: jae    0x1820e59da
020e59d3: xor    ecx, ecx
020e59d5: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e59da: lea    rax, [rbx + 0x20]
020e59de: mov    dword ptr [rbp - 0x40], 8
020e59e5: mov    qword ptr [rbp - 0x48], rax
020e59e9: lea    rdx, [rbp - 0x18]
020e59ed: movaps xmm0, xmmword ptr [rbp - 0x48]
020e59f1: xor    r8d, r8d
020e59f4: mov    rcx, rdi
020e59f7: movdqa xmmword ptr [rbp - 0x18], xmm0
020e59fc: call   0x1805f18b0
020e5a01: cmp    dword ptr [rbx + 0x18], 0
020e5a05: jbe    0x1820e5b8c
020e5a0b: cmp    byte ptr [rip + 0x1e33908], 0            ; [0x3f1931a] (bss)
020e5a12: mov    r15, qword ptr [rbx + 0x20]
020e5a16: mov    rdi, qword ptr [r14 + 0x10]
020e5a1a: jne    0x1820e5a2f
020e5a1c: lea    rcx, [rip + 0x1c234dd]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5a23: call   0x182f609b0
020e5a28: mov    byte ptr [rip + 0x1e338eb], 1            ; [0x3f1931a] (bss)
020e5a2f: xor    ecx, ecx
020e5a31: call   0x1801bd810
020e5a36: mov    rcx, qword ptr [rip + 0x1c234c3]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5a3d: mov    rbx, rax
020e5a40: cmp    qword ptr [rcx + 0x38], 0
020e5a45: jne    0x1820e5a4c
020e5a47: call   0x182f657d0
020e5a4c: mov    qword ptr [rbp - 0x30], rsi
020e5a50: test   rbx, rbx
020e5a53: je     0x1820e5b68
020e5a59: cmp    dword ptr [rbx + 0x18], 4
020e5a5d: jae    0x1820e5a66
020e5a5f: xor    ecx, ecx
020e5a61: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5a66: lea    rax, [rbx + 0x20]
020e5a6a: mov    dword ptr [rbp - 0x30], 4
020e5a71: mov    qword ptr [rbp - 0x38], rax
020e5a75: lea    rdx, [rbp - 0x18]
020e5a79: movaps xmm0, xmmword ptr [rbp - 0x38]
020e5a7d: xor    r8d, r8d
020e5a80: mov    rcx, rdi
020e5a83: movdqa xmmword ptr [rbp - 0x18], xmm0
020e5a88: call   0x1805f18b0
020e5a8d: cmp    dword ptr [rbx + 0x18], 0
020e5a91: jbe    0x1820e5b8c
020e5a97: cmp    byte ptr [rip + 0x1e3387c], 0            ; [0x3f1931a] (bss)
020e5a9e: mov    esi, dword ptr [rbx + 0x20]
020e5aa1: mov    rdi, qword ptr [r14 + 0x10]
020e5aa5: jne    0x1820e5aba
020e5aa7: lea    rcx, [rip + 0x1c23452]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5aae: call   0x182f609b0
020e5ab3: mov    byte ptr [rip + 0x1e33860], 1            ; [0x3f1931a] (bss)
020e5aba: xor    ecx, ecx
020e5abc: call   0x1801bd810
020e5ac1: mov    rcx, qword ptr [rip + 0x1c23438]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5ac8: mov    rbx, rax
020e5acb: cmp    qword ptr [rcx + 0x38], 0
020e5ad0: jne    0x1820e5ad7
020e5ad2: call   0x182f657d0
020e5ad7: mov    qword ptr [rbp - 0x20], 0
020e5adf: test   rbx, rbx
020e5ae2: je     0x1820e5b68
020e5ae8: cmp    dword ptr [rbx + 0x18], 4
020e5aec: jae    0x1820e5af5
020e5aee: xor    ecx, ecx
020e5af0: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5af5: lea    rax, [rbx + 0x20]
020e5af9: mov    dword ptr [rbp - 0x20], 4
020e5b00: mov    qword ptr [rbp - 0x28], rax
020e5b04: lea    rdx, [rbp - 0x18]
020e5b08: movaps xmm0, xmmword ptr [rbp - 0x28]
020e5b0c: xor    r8d, r8d
020e5b0f: mov    rcx, rdi
020e5b12: movdqa xmmword ptr [rbp - 0x18], xmm0
020e5b17: call   0x1805f18b0
020e5b1c: cmp    dword ptr [rbx + 0x18], 0
020e5b20: jbe    0x1820e5b8c
020e5b22: test   r12, r12
020e5b25: je     0x1820e5b86
020e5b27: mov    r9d, dword ptr [rbx + 0x20]
020e5b2b: mov    r8d, esi
020e5b2e: mov    rdx, r15
020e5b31: mov    qword ptr [rsp + 0x20], 0
020e5b3a: mov    rcx, r12
020e5b3d: call   0x18212d620                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʼʸʼʴʺʾʵʷʽˁˀ
020e5b42: inc    r13d
020e5b45: mov    esi, 0
020e5b4a: movsxd rax, r13d
020e5b4d: cmp    rax, qword ptr [rbp + 0x50]
020e5b51: jl     0x1820e5983
020e5b57: add    rsp, 0x78
020e5b5b: pop    r15
020e5b5d: pop    r14
020e5b5f: pop    r13
020e5b61: pop    r12
020e5b63: pop    rdi
020e5b64: pop    rsi
020e5b65: pop    rbx
020e5b66: pop    rbp
020e5b67: ret    
020e5b68: xor    ecx, ecx
020e5b6a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5b6f: xorps  xmm0, xmm0
020e5b72: lea    rdx, [rbp - 0x18]
020e5b76: xor    r8d, r8d
020e5b79: movdqa xmmword ptr [rbp - 0x18], xmm0
020e5b7e: mov    rcx, rdi
020e5b81: call   0x1805f18b0
020e5b86: call   0x182f60c50
020e5b8b: int3   
020e5b8c: call   0x182f60c40
020e5b91: int3   
020e5b92: int3   
=== 20E5BA0
020e5ba0: push   rbx
020e5ba2: push   rbp
020e5ba3: push   rsi
020e5ba4: push   rdi
020e5ba5: push   r12
020e5ba7: push   r13
020e5ba9: push   r14
020e5bab: push   r15
020e5bad: sub    rsp, 0x58
020e5bb1: mov    rbp, rcx
020e5bb4: test   rdx, rdx
020e5bb7: je     0x1820e5d4f
020e5bbd: mov    r14, qword ptr [rdx + 0x30]
020e5bc1: mov    r15d, 1
020e5bc7: mov    rcx, qword ptr [rcx + 0x10]
020e5bcb: xor    edx, edx
020e5bcd: call   0x1820cf940
020e5bd2: mov    r12d, eax
020e5bd5: cmp    r12, r15
020e5bd8: jbe    0x1820e5d1e
020e5bde: xor    r13d, r13d
020e5be1: cmp    byte ptr [rip + 0x1e33733], r13b         ; [0x3f1931b] (bss)
020e5be8: mov    rdi, qword ptr [rbp + 0x10]
020e5bec: jne    0x1820e5c01
020e5bee: lea    rcx, [rip + 0x1c2330b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5bf5: call   0x182f609b0
020e5bfa: mov    byte ptr [rip + 0x1e3371a], 1            ; [0x3f1931b] (bss)
020e5c01: xor    ecx, ecx
020e5c03: call   0x1801bd810
020e5c08: mov    rcx, qword ptr [rip + 0x1c232f1]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5c0f: mov    rbx, rax
020e5c12: cmp    qword ptr [rcx + 0x38], r13
020e5c16: jne    0x1820e5c1d
020e5c18: call   0x182f657d0
020e5c1d: mov    qword ptr [rsp + 0x28], r13
020e5c22: test   rbx, rbx
020e5c25: je     0x1820e5d2f
020e5c2b: cmp    dword ptr [rbx + 0x18], 8
020e5c2f: jae    0x1820e5c38
020e5c31: xor    ecx, ecx
020e5c33: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5c38: lea    rax, [rbx + 0x20]
020e5c3c: mov    dword ptr [rsp + 0x28], 8
020e5c44: mov    qword ptr [rsp + 0x20], rax
020e5c49: lea    rdx, [rsp + 0x40]
020e5c4e: movaps xmm0, xmmword ptr [rsp + 0x20]
020e5c53: xor    r8d, r8d
020e5c56: mov    rcx, rdi
020e5c59: movdqa xmmword ptr [rsp + 0x40], xmm0
020e5c5f: call   0x1805f18b0
020e5c64: cmp    dword ptr [rbx + 0x18], r13d
020e5c68: jbe    0x1820e5d55
020e5c6e: cmp    byte ptr [rip + 0x1e2af1c], r13b         ; [0x3f10b91] (bss)
020e5c75: mov    rsi, qword ptr [rbx + 0x20]
020e5c79: mov    rdi, qword ptr [rbp + 0x10]
020e5c7d: jne    0x1820e5c92
020e5c7f: lea    rcx, [rip + 0x1c2327a]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5c86: call   0x182f609b0
020e5c8b: mov    byte ptr [rip + 0x1e2aeff], 1            ; [0x3f10b91] (bss)
020e5c92: xor    ecx, ecx
020e5c94: call   0x1801bd810
020e5c99: mov    rcx, qword ptr [rip + 0x1c23260]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5ca0: mov    rbx, rax
020e5ca3: cmp    qword ptr [rcx + 0x38], r13
020e5ca7: jne    0x1820e5cae
020e5ca9: call   0x182f657d0
020e5cae: mov    qword ptr [rsp + 0x38], r13
020e5cb3: test   rbx, rbx
020e5cb6: je     0x1820e5d2f
020e5cb8: cmp    dword ptr [rbx + 0x18], 8
020e5cbc: jae    0x1820e5cc5
020e5cbe: xor    ecx, ecx
020e5cc0: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5cc5: lea    rax, [rbx + 0x20]
020e5cc9: mov    dword ptr [rsp + 0x38], 8
020e5cd1: mov    qword ptr [rsp + 0x30], rax
020e5cd6: lea    rdx, [rsp + 0x40]
020e5cdb: movaps xmm0, xmmword ptr [rsp + 0x30]
020e5ce0: xor    r8d, r8d
020e5ce3: mov    rcx, rdi
020e5ce6: movdqa xmmword ptr [rsp + 0x40], xmm0
020e5cec: call   0x1805f18b0
020e5cf1: cmp    dword ptr [rbx + 0x18], r13d
020e5cf5: jbe    0x1820e5d55
020e5cf7: test   r14, r14
020e5cfa: je     0x1820e5d4f
020e5cfc: movsd  xmm2, qword ptr [rbx + 0x20]
020e5d01: xor    r9d, r9d
020e5d04: mov    rdx, rsi
020e5d07: mov    rcx, r14
020e5d0a: call   0x18212cbf0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʷʿʳʹʳʲʽʺʵʻʽ
020e5d0f: inc    r15d
020e5d12: movsxd rax, r15d
020e5d15: cmp    rax, r12
020e5d18: jl     0x1820e5be1
020e5d1e: add    rsp, 0x58
020e5d22: pop    r15
020e5d24: pop    r14
020e5d26: pop    r13
020e5d28: pop    r12
020e5d2a: pop    rdi
020e5d2b: pop    rsi
020e5d2c: pop    rbp
020e5d2d: pop    rbx
020e5d2e: ret    
020e5d2f: xor    ecx, ecx
020e5d31: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5d36: xorps  xmm0, xmm0
020e5d39: lea    rdx, [rsp + 0x40]
020e5d3e: xor    r8d, r8d
020e5d41: movdqa xmmword ptr [rsp + 0x40], xmm0
020e5d47: mov    rcx, rdi
020e5d4a: call   0x1805f18b0
020e5d4f: call   0x182f60c50
020e5d54: int3   
020e5d55: call   0x182f60c40
020e5d5a: int3   
020e5d5b: int3   
=== 20E5D60
020e5d60: mov    qword ptr [rsp + 0x18], rsi
020e5d65: push   rdi
020e5d66: sub    rsp, 0x40
020e5d6a: mov    rsi, rdx
020e5d6d: mov    rdi, rcx
020e5d70: xor    edx, edx
020e5d72: call   0x1805f1630
020e5d77: cmp    eax, 1
020e5d7a: jle    0x1820e5e39
020e5d80: mov    qword ptr [rsp + 0x50], rbx
020e5d85: mov    qword ptr [rsp + 0x58], rbp
020e5d8a: xor    ebp, ebp
020e5d8c: nop    dword ptr [rax]
020e5d90: cmp    byte ptr [rip + 0x1e33584], bpl          ; [0x3f1931b] (bss)
020e5d97: jne    0x1820e5dac
020e5d99: lea    rcx, [rip + 0x1c23160]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5da0: call   0x182f609b0
020e5da5: mov    byte ptr [rip + 0x1e3356f], 1            ; [0x3f1931b] (bss)
020e5dac: xor    ecx, ecx
020e5dae: call   0x1801bd810
020e5db3: mov    rcx, qword ptr [rip + 0x1c23146]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5dba: mov    rbx, rax
020e5dbd: cmp    qword ptr [rcx + 0x38], rbp
020e5dc1: jne    0x1820e5dc8
020e5dc3: call   0x182f657d0
020e5dc8: mov    qword ptr [rsp + 0x28], rbp
020e5dcd: test   rbx, rbx
020e5dd0: je     0x1820e5e4a
020e5dd2: cmp    dword ptr [rbx + 0x18], 8
020e5dd6: jae    0x1820e5ddf
020e5dd8: xor    ecx, ecx
020e5dda: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5ddf: lea    rax, [rbx + 0x20]
020e5de3: mov    dword ptr [rsp + 0x28], 8
020e5deb: mov    qword ptr [rsp + 0x20], rax
020e5df0: lea    rdx, [rsp + 0x30]
020e5df5: movaps xmm0, xmmword ptr [rsp + 0x20]
020e5dfa: xor    r8d, r8d
020e5dfd: mov    rcx, rdi
020e5e00: movdqa xmmword ptr [rsp + 0x30], xmm0
020e5e06: call   0x1805f18b0
020e5e0b: cmp    dword ptr [rbx + 0x18], ebp
020e5e0e: jbe    0x1820e5e44
020e5e10: mov    rbx, qword ptr [rbx + 0x20]
020e5e14: xor    edx, edx
020e5e16: mov    rcx, rdi
020e5e19: call   0x1820cf830
020e5e1e: test   rsi, rsi
020e5e21: je     0x1820e5e6a
020e5e23: xor    r9d, r9d
020e5e26: mov    r8, rbx
020e5e29: mov    rdx, rax
020e5e2c: mov    rcx, rsi
020e5e2f: call   0x18212c350                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʵʽˀʸʳʳʶʿˀʳʶ
020e5e34: jmp    0x1820e5d90
020e5e39: mov    rsi, qword ptr [rsp + 0x60]
020e5e3e: add    rsp, 0x40
020e5e42: pop    rdi
020e5e43: ret    
020e5e44: call   0x182f60c40
020e5e49: int3   
020e5e4a: xor    ecx, ecx
020e5e4c: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5e51: xorps  xmm0, xmm0
020e5e54: lea    rdx, [rsp + 0x30]
020e5e59: xor    r8d, r8d
020e5e5c: movdqa xmmword ptr [rsp + 0x30], xmm0
020e5e62: mov    rcx, rdi
020e5e65: call   0x1805f18b0
020e5e6a: call   0x182f60c50
020e5e6f: int3   
020e5e70: push   rbp
020e5e72: push   r12
020e5e74: push   r13
020e5e76: sub    rsp, 0x70
020e5e7a: mov    r13, rdx
020e5e7d: mov    rbp, rcx
020e5e80: mov    rcx, qword ptr [rcx + 0x10]
020e5e84: xor    edx, edx
020e5e86: call   0x1820cf940
020e5e8b: mov    r12d, eax
020e5e8e: test   eax, eax
020e5e90: je     0x1820e611b
020e5e96: mov    qword ptr [rsp + 0x90], rbx
020e5e9e: mov    qword ptr [rsp + 0x98], rsi
020e5ea6: xor    esi, esi
020e5ea8: mov    qword ptr [rsp + 0xa0], rdi
020e5eb0: mov    qword ptr [rsp + 0x68], r14
020e5eb5: mov    r14d, esi
020e5eb8: mov    qword ptr [rsp + 0x60], r15
020e5ebd: nop    dword ptr [rax]
020e5ec0: cmp    byte ptr [rip + 0x1e33454], 0            ; [0x3f1931b] (bss)
020e5ec7: mov    rdi, qword ptr [rbp + 0x10]
020e5ecb: jne    0x1820e5ee0
020e5ecd: lea    rcx, [rip + 0x1c2302c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5ed4: call   0x182f609b0
020e5ed9: mov    byte ptr [rip + 0x1e3343b], 1            ; [0x3f1931b] (bss)
020e5ee0: xor    ecx, ecx
020e5ee2: call   0x1801bd810
020e5ee7: mov    rcx, qword ptr [rip + 0x1c23012]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5eee: mov    rbx, rax
020e5ef1: cmp    qword ptr [rcx + 0x38], 0
020e5ef6: jne    0x1820e5efd
020e5ef8: call   0x182f657d0
020e5efd: mov    qword ptr [rsp + 0x38], rsi
020e5f02: test   rbx, rbx
020e5f05: je     0x1820e6125
020e5f0b: cmp    dword ptr [rbx + 0x18], 8
020e5f0f: jae    0x1820e5f18
020e5f11: xor    ecx, ecx
020e5f13: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5f18: lea    rax, [rbx + 0x20]
020e5f1c: mov    dword ptr [rsp + 0x38], 8
020e5f24: mov    qword ptr [rsp + 0x30], rax
020e5f29: lea    rdx, [rsp + 0x50]
020e5f2e: movaps xmm0, xmmword ptr [rsp + 0x30]
020e5f33: xor    r8d, r8d
020e5f36: mov    rcx, rdi
020e5f39: movdqa xmmword ptr [rsp + 0x50], xmm0
020e5f3f: call   0x1805f18b0
020e5f44: cmp    dword ptr [rbx + 0x18], 0
020e5f48: jbe    0x1820e614b
020e5f4e: cmp    byte ptr [rip + 0x1e333c6], 0            ; [0x3f1931b] (bss)
020e5f55: mov    r15, qword ptr [rbx + 0x20]
020e5f59: mov    rdi, qword ptr [rbp + 0x10]
020e5f5d: jne    0x1820e5f72
020e5f5f: lea    rcx, [rip + 0x1c22f9a]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5f66: call   0x182f609b0
020e5f6b: mov    byte ptr [rip + 0x1e333a9], 1            ; [0x3f1931b] (bss)
020e5f72: xor    ecx, ecx
020e5f74: call   0x1801bd810
020e5f79: mov    rcx, qword ptr [rip + 0x1c22f80]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5f80: mov    rbx, rax
020e5f83: cmp    qword ptr [rcx + 0x38], 0
020e5f88: jne    0x1820e5f8f
020e5f8a: call   0x182f657d0
020e5f8f: mov    qword ptr [rsp + 0x48], rsi
020e5f94: test   rbx, rbx
020e5f97: je     0x1820e6125
020e5f9d: cmp    dword ptr [rbx + 0x18], 8
020e5fa1: jae    0x1820e5faa
020e5fa3: xor    ecx, ecx
020e5fa5: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5faa: lea    rax, [rbx + 0x20]
020e5fae: mov    dword ptr [rsp + 0x48], 8
020e5fb6: mov    qword ptr [rsp + 0x40], rax
020e5fbb: lea    rdx, [rsp + 0x50]
020e5fc0: movaps xmm0, xmmword ptr [rsp + 0x40]
020e5fc5: xor    r8d, r8d
020e5fc8: mov    rcx, rdi
020e5fcb: movdqa xmmword ptr [rsp + 0x50], xmm0
020e5fd1: call   0x1805f18b0
020e5fd6: cmp    dword ptr [rbx + 0x18], 0
020e5fda: jbe    0x1820e614b
020e5fe0: mov    rcx, qword ptr [rbp + 0x10]
020e5fe4: test   rcx, rcx
020e5fe7: je     0x1820e6145
020e5fed: mov    rax, qword ptr [rcx]
020e5ff0: mov    rdi, qword ptr [rbx + 0x20]
020e5ff4: mov    rdx, qword ptr [rax + 0x390]
020e5ffb: call   qword ptr [rax + 0x388]
020e6001: cmp    eax, 1
020e6004: sete   sil
020e6008: test   r13, r13
020e600b: je     0x1820e6145
020e6011: cmp    byte ptr [rip + 0x1e3330a], 0            ; [0x3f19322] (bss)
020e6018: jne    0x1820e6039
020e601a: lea    rcx, [rip + 0x1be7167]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020e6021: call   0x182f609b0
020e6026: lea    rcx, [rip + 0x1c12033]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e602d: call   0x182f609b0
020e6032: mov    byte ptr [rip + 0x1e332e9], 1            ; [0x3f19322] (bss)
020e6039: mov    rcx, qword ptr [r13 + 0x30]
020e603d: test   rcx, rcx
020e6040: je     0x1820e6145
020e6046: mov    rdx, qword ptr [rip + 0x1c12013]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e604d: mov    rbx, qword ptr [r13 + 0x88]
020e6054: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e6059: test   rax, rax
020e605c: je     0x1820e6145
020e6062: movzx  r9d, sil
020e6066: mov    qword ptr [rsp + 0x20], 0
020e606f: mov    r8, rdi
020e6072: mov    rdx, r15
020e6075: mov    rcx, rax
020e6078: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
020e607d: mov    r9, rax
020e6080: test   rbx, rbx
020e6083: je     0x1820e6145
020e6089: mov    rax, qword ptr [rip + 0x1be70f8]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020e6090: inc    dword ptr [rbx + 0x1c]
020e6093: mov    rcx, qword ptr [rbx + 0x10]
020e6097: test   rcx, rcx
020e609a: je     0x1820e6145
020e60a0: movsxd rdx, dword ptr [rbx + 0x18]
020e60a4: cmp    edx, dword ptr [rcx + 0x18]
020e60a7: jb     0x1820e60c5
020e60a9: mov    rax, qword ptr [rax + 0x20]
020e60ad: mov    rdx, r9
020e60b0: mov    rcx, rbx
020e60b3: mov    r8, qword ptr [rax + 0xc0]
020e60ba: mov    r8, qword ptr [r8 + 0x70]
020e60be: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e60c3: jmp    0x1820e60e5
020e60c5: lea    eax, [rdx + 1]
020e60c8: mov    dword ptr [rbx + 0x18], eax
020e60cb: cmp    edx, dword ptr [rcx + 0x18]
020e60ce: jae    0x1820e614b
020e60d0: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020e60d5: lea    rcx, [rcx + rdx*8]
020e60d9: add    rcx, 0x20
020e60dd: mov    rdx, r9
020e60e0: call   0x182f5fc00
020e60e5: inc    r14d
020e60e8: mov    esi, 0
020e60ed: movsxd rax, r14d
020e60f0: cmp    rax, r12
020e60f3: jl     0x1820e5ec0
020e60f9: mov    r15, qword ptr [rsp + 0x60]
020e60fe: mov    rdi, qword ptr [rsp + 0xa0]
020e6106: mov    rsi, qword ptr [rsp + 0x98]
020e610e: mov    rbx, qword ptr [rsp + 0x90]
020e6116: mov    r14, qword ptr [rsp + 0x68]
020e611b: add    rsp, 0x70
020e611f: pop    r13
020e6121: pop    r12
020e6123: pop    rbp
020e6124: ret    
020e6125: xor    ecx, ecx
020e6127: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e612c: xorps  xmm0, xmm0
020e612f: lea    rdx, [rsp + 0x50]
020e6134: xor    r8d, r8d
020e6137: movdqa xmmword ptr [rsp + 0x50], xmm0
020e613d: mov    rcx, rdi
020e6140: call   0x1805f18b0
020e6145: call   0x182f60c50
020e614a: int3   
020e614b: call   0x182f60c40
020e6150: int3   
020e6151: int3   
=== 20E5E70
020e5e70: push   rbp
020e5e72: push   r12
020e5e74: push   r13
020e5e76: sub    rsp, 0x70
020e5e7a: mov    r13, rdx
020e5e7d: mov    rbp, rcx
020e5e80: mov    rcx, qword ptr [rcx + 0x10]
020e5e84: xor    edx, edx
020e5e86: call   0x1820cf940
020e5e8b: mov    r12d, eax
020e5e8e: test   eax, eax
020e5e90: je     0x1820e611b
020e5e96: mov    qword ptr [rsp + 0x90], rbx
020e5e9e: mov    qword ptr [rsp + 0x98], rsi
020e5ea6: xor    esi, esi
020e5ea8: mov    qword ptr [rsp + 0xa0], rdi
020e5eb0: mov    qword ptr [rsp + 0x68], r14
020e5eb5: mov    r14d, esi
020e5eb8: mov    qword ptr [rsp + 0x60], r15
020e5ebd: nop    dword ptr [rax]
020e5ec0: cmp    byte ptr [rip + 0x1e33454], 0            ; [0x3f1931b] (bss)
020e5ec7: mov    rdi, qword ptr [rbp + 0x10]
020e5ecb: jne    0x1820e5ee0
020e5ecd: lea    rcx, [rip + 0x1c2302c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5ed4: call   0x182f609b0
020e5ed9: mov    byte ptr [rip + 0x1e3343b], 1            ; [0x3f1931b] (bss)
020e5ee0: xor    ecx, ecx
020e5ee2: call   0x1801bd810
020e5ee7: mov    rcx, qword ptr [rip + 0x1c23012]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5eee: mov    rbx, rax
020e5ef1: cmp    qword ptr [rcx + 0x38], 0
020e5ef6: jne    0x1820e5efd
020e5ef8: call   0x182f657d0
020e5efd: mov    qword ptr [rsp + 0x38], rsi
020e5f02: test   rbx, rbx
020e5f05: je     0x1820e6125
020e5f0b: cmp    dword ptr [rbx + 0x18], 8
020e5f0f: jae    0x1820e5f18
020e5f11: xor    ecx, ecx
020e5f13: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5f18: lea    rax, [rbx + 0x20]
020e5f1c: mov    dword ptr [rsp + 0x38], 8
020e5f24: mov    qword ptr [rsp + 0x30], rax
020e5f29: lea    rdx, [rsp + 0x50]
020e5f2e: movaps xmm0, xmmword ptr [rsp + 0x30]
020e5f33: xor    r8d, r8d
020e5f36: mov    rcx, rdi
020e5f39: movdqa xmmword ptr [rsp + 0x50], xmm0
020e5f3f: call   0x1805f18b0
020e5f44: cmp    dword ptr [rbx + 0x18], 0
020e5f48: jbe    0x1820e614b
020e5f4e: cmp    byte ptr [rip + 0x1e333c6], 0            ; [0x3f1931b] (bss)
020e5f55: mov    r15, qword ptr [rbx + 0x20]
020e5f59: mov    rdi, qword ptr [rbp + 0x10]
020e5f5d: jne    0x1820e5f72
020e5f5f: lea    rcx, [rip + 0x1c22f9a]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5f66: call   0x182f609b0
020e5f6b: mov    byte ptr [rip + 0x1e333a9], 1            ; [0x3f1931b] (bss)
020e5f72: xor    ecx, ecx
020e5f74: call   0x1801bd810
020e5f79: mov    rcx, qword ptr [rip + 0x1c22f80]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e5f80: mov    rbx, rax
020e5f83: cmp    qword ptr [rcx + 0x38], 0
020e5f88: jne    0x1820e5f8f
020e5f8a: call   0x182f657d0
020e5f8f: mov    qword ptr [rsp + 0x48], rsi
020e5f94: test   rbx, rbx
020e5f97: je     0x1820e6125
020e5f9d: cmp    dword ptr [rbx + 0x18], 8
020e5fa1: jae    0x1820e5faa
020e5fa3: xor    ecx, ecx
020e5fa5: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e5faa: lea    rax, [rbx + 0x20]
020e5fae: mov    dword ptr [rsp + 0x48], 8
020e5fb6: mov    qword ptr [rsp + 0x40], rax
020e5fbb: lea    rdx, [rsp + 0x50]
020e5fc0: movaps xmm0, xmmword ptr [rsp + 0x40]
020e5fc5: xor    r8d, r8d
020e5fc8: mov    rcx, rdi
020e5fcb: movdqa xmmword ptr [rsp + 0x50], xmm0
020e5fd1: call   0x1805f18b0
020e5fd6: cmp    dword ptr [rbx + 0x18], 0
020e5fda: jbe    0x1820e614b
020e5fe0: mov    rcx, qword ptr [rbp + 0x10]
020e5fe4: test   rcx, rcx
020e5fe7: je     0x1820e6145
020e5fed: mov    rax, qword ptr [rcx]
020e5ff0: mov    rdi, qword ptr [rbx + 0x20]
020e5ff4: mov    rdx, qword ptr [rax + 0x390]
020e5ffb: call   qword ptr [rax + 0x388]
020e6001: cmp    eax, 1
020e6004: sete   sil
020e6008: test   r13, r13
020e600b: je     0x1820e6145
020e6011: cmp    byte ptr [rip + 0x1e3330a], 0            ; [0x3f19322] (bss)
020e6018: jne    0x1820e6039
020e601a: lea    rcx, [rip + 0x1be7167]                   ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020e6021: call   0x182f609b0
020e6026: lea    rcx, [rip + 0x1c12033]                   ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e602d: call   0x182f609b0
020e6032: mov    byte ptr [rip + 0x1e332e9], 1            ; [0x3f19322] (bss)
020e6039: mov    rcx, qword ptr [r13 + 0x30]
020e603d: test   rcx, rcx
020e6040: je     0x1820e6145
020e6046: mov    rdx, qword ptr [rip + 0x1c12013]         ; [0x3cf8060] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʲʵʺʹʿʵʹʷʿʲʻ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e604d: mov    rbx, qword ptr [r13 + 0x88]
020e6054: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e6059: test   rax, rax
020e605c: je     0x1820e6145
020e6062: movzx  r9d, sil
020e6066: mov    qword ptr [rsp + 0x20], 0
020e606f: mov    r8, rdi
020e6072: mov    rdx, r15
020e6075: mov    rcx, rax
020e6078: call   0x1820fb9a0                              ; ʲʵʺʹʿʵʹʷʿʲʻ$$ʹʻˀʷˀʿʳʹʴʳˁ
020e607d: mov    r9, rax
020e6080: test   rbx, rbx
020e6083: je     0x1820e6145
020e6089: mov    rax, qword ptr [rip + 0x1be70f8]         ; [0x3ccd188] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.Add()
020e6090: inc    dword ptr [rbx + 0x1c]
020e6093: mov    rcx, qword ptr [rbx + 0x10]
020e6097: test   rcx, rcx
020e609a: je     0x1820e6145
020e60a0: movsxd rdx, dword ptr [rbx + 0x18]
020e60a4: cmp    edx, dword ptr [rcx + 0x18]
020e60a7: jb     0x1820e60c5
020e60a9: mov    rax, qword ptr [rax + 0x20]
020e60ad: mov    rdx, r9
020e60b0: mov    rcx, rbx
020e60b3: mov    r8, qword ptr [rax + 0xc0]
020e60ba: mov    r8, qword ptr [r8 + 0x70]
020e60be: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e60c3: jmp    0x1820e60e5
020e60c5: lea    eax, [rdx + 1]
020e60c8: mov    dword ptr [rbx + 0x18], eax
020e60cb: cmp    edx, dword ptr [rcx + 0x18]
020e60ce: jae    0x1820e614b
020e60d0: mov    qword ptr [rcx + rdx*8 + 0x20], r9
020e60d5: lea    rcx, [rcx + rdx*8]
020e60d9: add    rcx, 0x20
020e60dd: mov    rdx, r9
020e60e0: call   0x182f5fc00
020e60e5: inc    r14d
020e60e8: mov    esi, 0
020e60ed: movsxd rax, r14d
020e60f0: cmp    rax, r12
020e60f3: jl     0x1820e5ec0
020e60f9: mov    r15, qword ptr [rsp + 0x60]
020e60fe: mov    rdi, qword ptr [rsp + 0xa0]
020e6106: mov    rsi, qword ptr [rsp + 0x98]
020e610e: mov    rbx, qword ptr [rsp + 0x90]
020e6116: mov    r14, qword ptr [rsp + 0x68]
020e611b: add    rsp, 0x70
020e611f: pop    r13
020e6121: pop    r12
020e6123: pop    rbp
020e6124: ret    
020e6125: xor    ecx, ecx
020e6127: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e612c: xorps  xmm0, xmm0
020e612f: lea    rdx, [rsp + 0x50]
020e6134: xor    r8d, r8d
020e6137: movdqa xmmword ptr [rsp + 0x50], xmm0
020e613d: mov    rcx, rdi
020e6140: call   0x1805f18b0
020e6145: call   0x182f60c50
020e614a: int3   
020e614b: call   0x182f60c40
020e6150: int3   
020e6151: int3   
=== 20E6160
020e6160: push   rbp
020e6162: push   rbx
020e6163: push   rsi
020e6164: push   rdi
020e6165: push   r14
020e6167: sub    rsp, 0x90
020e616e: lea    rbp, [rsp + 0x60]
020e6173: cmp    byte ptr [rip + 0x1e33132], 0            ; [0x3f192ac] (bss)
020e617a: mov    rdi, rcx
020e617d: movaps xmmword ptr [rbp + 0x20], xmm6
020e6181: jne    0x1820e61c6
020e6183: lea    rcx, [rip + 0x1c23936]                   ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e618a: call   0x182f609b0
020e618f: lea    rcx, [rip + 0x1bbb2aa]                   ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e6196: call   0x182f609b0
020e619b: lea    rcx, [rip + 0x1bc2a66]                   ; [0x3ca8c08] metamethod:Method$System.Span<byte>..ctor()
020e61a2: call   0x182f609b0
020e61a7: lea    rcx, [rip + 0x1c28222]                   ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e61ae: call   0x182f609b0
020e61b3: lea    rcx, [rip + 0x1bb7046]                   ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61ba: call   0x182f609b0
020e61bf: mov    byte ptr [rip + 0x1e330e6], 1            ; [0x3f192ac] (bss)
020e61c6: mov    rax, qword ptr [rip + 0x1bb7033]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61cd: cmp    dword ptr [rax + 0xe0], 0
020e61d4: jne    0x1820e61e5
020e61d6: mov    rcx, rax
020e61d9: call   0x182f60cf0
020e61de: mov    rax, qword ptr [rip + 0x1bb701b]         ; [0x3c9d200] meta:ʾʵʻʹʾʹʴˁʼʶˁ_TypeInfo
020e61e5: mov    rax, qword ptr [rax + 0xb8]
020e61ec: mov    rsi, qword ptr [rax]
020e61ef: test   rsi, rsi
020e61f2: je     0x1820e636c
020e61f8: movsxd rbx, dword ptr [rsi + 0x18]
020e61fc: test   ebx, ebx
020e61fe: je     0x1820e6229
020e6200: lea    rcx, [rbx + 0xf]
020e6204: cmp    rcx, rbx
020e6207: ja     0x1820e6213
020e6209: movabs rcx, 0xffffffffffffff0
020e6213: and    rcx, 0xfffffffffffffff0
020e6217: mov    rax, rcx
020e621a: call   0x18302b960
020e621f: sub    rsp, rcx
020e6222: lea    r14, [rsp + 0x60]
020e6227: jmp    0x1820e622c
020e6229: xor    r14d, r14d
020e622c: mov    r8, rbx
020e622f: xor    edx, edx
020e6231: mov    rcx, r14
020e6234: call   0x18305d9d0
020e6239: mov    dword ptr [rbp + 0xc], 0
020e6240: test   ebx, ebx
020e6242: jns    0x1820e624b
020e6244: xor    ecx, ecx
020e6246: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e624b: mov    rcx, qword ptr [rdi + 0x10]
020e624f: lea    rdx, [rbp]
020e6253: mov    qword ptr [rbp], r14
020e6257: xor    r8d, r8d
020e625a: mov    dword ptr [rbp + 8], ebx
020e625d: movaps xmm6, xmmword ptr [rbp]
020e6261: movdqa xmmword ptr [rbp], xmm6
020e6266: call   0x1805f18b0
020e626b: mov    r8, qword ptr [rip + 0x1bbb1ce]          ; [0x3ca1440] metamethod:Method$System.ReadOnlySpan<byte>.op_Implicit()
020e6272: lea    rcx, [rbp]
020e6276: mov    rdx, rsi
020e6279: call   0x18094e510                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
020e627e: movaps xmm0, xmmword ptr [rbp]
020e6282: lea    rdx, [rbp]
020e6286: mov    r8, qword ptr [rip + 0x1c23833]          ; [0x3d09ac0] metamethod:Method$System.MemoryExtensions.SequenceEqual<byte>()
020e628d: lea    rcx, [rbp + 0x10]
020e6291: movdqa xmmword ptr [rbp], xmm0
020e6296: movdqa xmmword ptr [rbp + 0x10], xmm6
020e629b: call   0x182c56fc0
020e62a0: test   al, al
020e62a2: je     0x1820e63bb
020e62a8: mov    rcx, qword ptr [rdi + 0x10]
020e62ac: xor    edx, edx
020e62ae: call   0x1820cf940
020e62b3: cmp    eax, -0x5e
020e62b6: jne    0x1820e6372
020e62bc: mov    rcx, qword ptr [rdi + 0x10]
020e62c0: xor    edx, edx
020e62c2: call   0x1820cf940
020e62c7: mov    rcx, qword ptr [rip + 0x1c28102]         ; [0x3d0e3d0] meta:ʲʻʿʾʲʲʾʽˀʶʽ_TypeInfo
020e62ce: movss  xmm6, dword ptr [rdi + 0x18]
020e62d3: mov    esi, eax
020e62d5: call   0x182f60c00
020e62da: mov    qword ptr [rsp + 0x50], 0
020e62e3: mov    r8b, 1
020e62e6: mov    byte ptr [rsp + 0x48], 1
020e62eb: movaps xmm3, xmm6
020e62ee: mov    dword ptr [rsp + 0x40], 1
020e62f6: movzx  edx, r8b
020e62fa: mov    dword ptr [rsp + 0x38], 1
020e6302: mov    rcx, rax
020e6305: mov    dword ptr [rsp + 0x30], 0xffffffff
020e630d: mov    rbx, rax
020e6310: mov    byte ptr [rsp + 0x28], 0
020e6315: mov    dword ptr [rsp + 0x20], 0xffffffff
020e631d: call   0x1820e73e0                              ; ʲʻʿʾʲʲʾʽˀʶʽ$$.ctor
020e6322: test   rbx, rbx
020e6325: je     0x1820e636c
020e6327: mov    rcx, qword ptr [rbx + 0x30]
020e632b: test   rcx, rcx
020e632e: je     0x1820e636c
020e6330: xorps  xmm1, xmm1
020e6333: xor    r8d, r8d
020e6336: cvtsi2sd xmm1, rsi
020e633b: call   0x18212c0b0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴˁʹˀʵʲʾʺʴʼʵ
020e6340: mov    rcx, qword ptr [rbx + 0x30]
020e6344: test   rcx, rcx
020e6347: je     0x1820e636c
020e6349: movzx  r8d, byte ptr [rdi + 0x1c]
020e634e: xor    r9d, r9d
020e6351: movzx  edx, byte ptr [rdi + 0x1d]
020e6355: call   0x18212b860                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʹʵʺʲˀˁʽʳˁʹ
020e635a: movaps xmm6, xmmword ptr [rbp + 0x20]
020e635e: mov    rax, rbx
020e6361: lea    rsp, [rbp + 0x30]
020e6365: pop    r14
020e6367: pop    rdi
020e6368: pop    rsi
020e6369: pop    rbx
020e636a: pop    rbp
020e636b: ret    
020e636c: call   0x182f60c50
020e6371: int3   
020e6372: lea    rcx, [rip + 0x1bffcf7]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e6379: call   0x182f609d0
020e637e: mov    rcx, rax
020e6381: call   0x182f60c00
020e6386: lea    rcx, [rip + 0x1bf5eb3]                   ; [0x3cdc240] str:'max'
020e638d: mov    rbx, rax
020e6390: call   0x182f609d0
020e6395: mov    rdx, rax
020e6398: xor    r8d, r8d
020e639b: mov    rcx, rbx
020e639e: call   0x181b92e30                              ; System.Exception$$.ctor
020e63a3: lea    rcx, [rip + 0x1bb6a36]                   ; [0x3c9cde0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ˀʾʸʸʾʲʸʷʼʿʺ()
020e63aa: call   0x182f609d0
020e63af: mov    rdx, rax
020e63b2: mov    rcx, rbx
020e63b5: call   0x182f60c10
020e63ba: int3   
020e63bb: lea    rcx, [rip + 0x1bffcae]                   ; [0x3ce6070] meta:System.Exception_TypeInfo
020e63c2: call   0x182f609d0
020e63c7: mov    rcx, rax
020e63ca: call   0x182f60c00
020e63cf: lea    rcx, [rip + 0x1be681a]                   ; [0x3cccbf0] str:'sustain_yellow'
020e63d6: mov    rbx, rax
020e63d9: call   0x182f609d0
020e63de: mov    rdx, rax
020e63e1: xor    r8d, r8d
020e63e4: mov    rcx, rbx
020e63e7: call   0x181b92e30                              ; System.Exception$$.ctor
020e63ec: lea    rcx, [rip + 0x1bb69ed]                   ; [0x3c9cde0] metamethod:Method$ʿˀʻʳʺʻʼʲʷʻʷ.ˀʾʸʸʾʲʸʷʼʿʺ()
020e63f3: call   0x182f609d0
020e63f8: mov    rdx, rax
020e63fb: mov    rcx, rbx
020e63fe: call   0x182f60c10
020e6403: int3   
020e6404: int3   
=== 20E6680
020e6680: push   rbp
020e6682: push   r12
020e6684: push   r13
020e6686: push   r14
020e6688: push   r15
020e668a: sub    rsp, 0x50
020e668e: mov    r14, rdx
020e6691: mov    rbp, rcx
020e6694: mov    rcx, qword ptr [rcx + 0x10]
020e6698: xor    r13d, r13d
020e669b: xor    edx, edx
020e669d: mov    r15d, r13d
020e66a0: call   0x1820cf940
020e66a5: mov    r12d, eax
020e66a8: test   eax, eax
020e66aa: je     0x1820e6828
020e66b0: mov    qword ptr [rsp + 0x80], rbx
020e66b8: mov    qword ptr [rsp + 0x90], rdi
020e66c0: mov    qword ptr [rsp + 0x88], rsi
020e66c8: nop    dword ptr [rax + rax]
020e66d0: cmp    byte ptr [rip + 0x1e32c44], r13b         ; [0x3f1931b] (bss)
020e66d7: mov    rdi, qword ptr [rbp + 0x10]
020e66db: jne    0x1820e66f0
020e66dd: lea    rcx, [rip + 0x1c2281c]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e66e4: call   0x182f609b0
020e66e9: mov    byte ptr [rip + 0x1e32c2b], 1            ; [0x3f1931b] (bss)
020e66f0: xor    ecx, ecx
020e66f2: call   0x1801bd810
020e66f7: mov    rcx, qword ptr [rip + 0x1c22802]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e66fe: mov    rbx, rax
020e6701: cmp    qword ptr [rcx + 0x38], r13
020e6705: jne    0x1820e670c
020e6707: call   0x182f657d0
020e670c: mov    qword ptr [rsp + 0x28], r13
020e6711: test   rbx, rbx
020e6714: je     0x1820e6836
020e671a: cmp    dword ptr [rbx + 0x18], 8
020e671e: jae    0x1820e6727
020e6720: xor    ecx, ecx
020e6722: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6727: lea    rax, [rbx + 0x20]
020e672b: mov    dword ptr [rsp + 0x28], 8
020e6733: mov    qword ptr [rsp + 0x20], rax
020e6738: lea    rdx, [rsp + 0x40]
020e673d: movaps xmm0, xmmword ptr [rsp + 0x20]
020e6742: xor    r8d, r8d
020e6745: mov    rcx, rdi
020e6748: movdqa xmmword ptr [rsp + 0x40], xmm0
020e674e: call   0x1805f18b0
020e6753: cmp    dword ptr [rbx + 0x18], r13d
020e6757: jbe    0x1820e685c
020e675d: cmp    byte ptr [rip + 0x1e32bb7], r13b         ; [0x3f1931b] (bss)
020e6764: mov    rsi, qword ptr [rbx + 0x20]
020e6768: mov    rdi, qword ptr [rbp + 0x10]
020e676c: jne    0x1820e6781
020e676e: lea    rcx, [rip + 0x1c2278b]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6775: call   0x182f609b0
020e677a: mov    byte ptr [rip + 0x1e32b9a], 1            ; [0x3f1931b] (bss)
020e6781: xor    ecx, ecx
020e6783: call   0x1801bd810
020e6788: mov    rcx, qword ptr [rip + 0x1c22771]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e678f: mov    rbx, rax
020e6792: cmp    qword ptr [rcx + 0x38], r13
020e6796: jne    0x1820e679d
020e6798: call   0x182f657d0
020e679d: mov    qword ptr [rsp + 0x38], r13
020e67a2: test   rbx, rbx
020e67a5: je     0x1820e6836
020e67ab: cmp    dword ptr [rbx + 0x18], 8
020e67af: jae    0x1820e67b8
020e67b1: xor    ecx, ecx
020e67b3: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e67b8: lea    rax, [rbx + 0x20]
020e67bc: mov    dword ptr [rsp + 0x38], 8
020e67c4: mov    qword ptr [rsp + 0x30], rax
020e67c9: lea    rdx, [rsp + 0x40]
020e67ce: movaps xmm0, xmmword ptr [rsp + 0x30]
020e67d3: xor    r8d, r8d
020e67d6: mov    rcx, rdi
020e67d9: movdqa xmmword ptr [rsp + 0x40], xmm0
020e67df: call   0x1805f18b0
020e67e4: cmp    dword ptr [rbx + 0x18], r13d
020e67e8: jbe    0x1820e685c
020e67ea: test   r14, r14
020e67ed: je     0x1820e6856
020e67ef: mov    r8, qword ptr [rbx + 0x20]
020e67f3: xor    r9d, r9d
020e67f6: mov    rdx, rsi
020e67f9: mov    rcx, r14
020e67fc: call   0x18215d860                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶʵʻʵʳʴʹʸʷʼʴ
020e6801: inc    r15d
020e6804: movsxd rax, r15d
020e6807: cmp    rax, r12
020e680a: jl     0x1820e66d0
020e6810: mov    rsi, qword ptr [rsp + 0x88]
020e6818: mov    rbx, qword ptr [rsp + 0x80]
020e6820: mov    rdi, qword ptr [rsp + 0x90]
020e6828: add    rsp, 0x50
020e682c: pop    r15
020e682e: pop    r14
020e6830: pop    r13
020e6832: pop    r12
020e6834: pop    rbp
020e6835: ret    
020e6836: xor    ecx, ecx
020e6838: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e683d: xorps  xmm0, xmm0
020e6840: lea    rdx, [rsp + 0x40]
020e6845: xor    r8d, r8d
020e6848: movdqa xmmword ptr [rsp + 0x40], xmm0
020e684e: mov    rcx, rdi
020e6851: call   0x1805f18b0
020e6856: call   0x182f60c50
020e685b: int3   
020e685c: call   0x182f60c40
020e6861: int3   
020e6862: int3   
=== 20E69A0
020e69a0: mov    qword ptr [rsp + 0x10], rbx
020e69a5: push   rbp
020e69a6: push   rsi
020e69a7: push   rdi
020e69a8: push   r12
020e69aa: push   r13
020e69ac: push   r14
020e69ae: push   r15
020e69b0: mov    rbp, rsp
020e69b3: sub    rsp, 0x80
020e69ba: mov    r15, rdx
020e69bd: mov    r13, rcx
020e69c0: mov    rcx, qword ptr [rcx + 0x10]
020e69c4: xor    edx, edx
020e69c6: call   0x1820cf940
020e69cb: mov    eax, eax
020e69cd: mov    qword ptr [rbp + 0x58], rax
020e69d1: cmp    rax, 1
020e69d5: jbe    0x1820e6d66
020e69db: mov    dword ptr [rbp + 0x40], 1
020e69e2: xor    r14d, r14d
020e69e5: nop    word ptr [rax + rax]
020e69f0: cmp    byte ptr [rip + 0x1e32924], 0            ; [0x3f1931b] (bss)
020e69f7: mov    rdi, qword ptr [r13 + 0x10]
020e69fb: jne    0x1820e6a10
020e69fd: lea    rcx, [rip + 0x1c224fc]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a04: call   0x182f609b0
020e6a09: mov    byte ptr [rip + 0x1e3290b], 1            ; [0x3f1931b] (bss)
020e6a10: xor    ecx, ecx
020e6a12: call   0x1801bd810
020e6a17: mov    rcx, qword ptr [rip + 0x1c224e2]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a1e: mov    rbx, rax
020e6a21: cmp    qword ptr [rcx + 0x38], 0
020e6a26: jne    0x1820e6a2d
020e6a28: call   0x182f657d0
020e6a2d: mov    qword ptr [rbp - 0x48], r14
020e6a31: test   rbx, rbx
020e6a34: je     0x1820e6d9a
020e6a3a: cmp    dword ptr [rbx + 0x18], 8
020e6a3e: jae    0x1820e6a47
020e6a40: xor    ecx, ecx
020e6a42: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6a47: lea    rax, [rbx + 0x20]
020e6a4b: mov    dword ptr [rbp - 0x48], 8
020e6a52: mov    qword ptr [rbp - 0x50], rax
020e6a56: lea    rdx, [rbp - 0x10]
020e6a5a: movaps xmm0, xmmword ptr [rbp - 0x50]
020e6a5e: xor    r8d, r8d
020e6a61: mov    rcx, rdi
020e6a64: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6a69: call   0x1805f18b0
020e6a6e: cmp    dword ptr [rbx + 0x18], 0
020e6a72: jbe    0x1820e6dbe
020e6a78: cmp    byte ptr [rip + 0x1e3289c], 0            ; [0x3f1931b] (bss)
020e6a7f: mov    r12, qword ptr [rbx + 0x20]
020e6a83: mov    rdi, qword ptr [r13 + 0x10]
020e6a87: jne    0x1820e6a9c
020e6a89: lea    rcx, [rip + 0x1c22470]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6a90: call   0x182f609b0
020e6a95: mov    byte ptr [rip + 0x1e3287f], 1            ; [0x3f1931b] (bss)
020e6a9c: xor    ecx, ecx
020e6a9e: call   0x1801bd810
020e6aa3: mov    rcx, qword ptr [rip + 0x1c22456]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6aaa: mov    rbx, rax
020e6aad: cmp    qword ptr [rcx + 0x38], 0
020e6ab2: jne    0x1820e6ab9
020e6ab4: call   0x182f657d0
020e6ab9: mov    qword ptr [rbp - 0x38], r14
020e6abd: test   rbx, rbx
020e6ac0: je     0x1820e6d9a
020e6ac6: cmp    dword ptr [rbx + 0x18], 8
020e6aca: jae    0x1820e6ad3
020e6acc: xor    ecx, ecx
020e6ace: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6ad3: lea    rax, [rbx + 0x20]
020e6ad7: mov    dword ptr [rbp - 0x38], 8
020e6ade: mov    qword ptr [rbp - 0x40], rax
020e6ae2: lea    rdx, [rbp - 0x10]
020e6ae6: movaps xmm0, xmmword ptr [rbp - 0x40]
020e6aea: xor    r8d, r8d
020e6aed: mov    rcx, rdi
020e6af0: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6af5: call   0x1805f18b0
020e6afa: cmp    dword ptr [rbx + 0x18], 0
020e6afe: jbe    0x1820e6dbe
020e6b04: cmp    byte ptr [rip + 0x1e3280f], 0            ; [0x3f1931a] (bss)
020e6b0b: mov    rsi, qword ptr [rbx + 0x20]
020e6b0f: mov    rdi, qword ptr [r13 + 0x10]
020e6b13: jne    0x1820e6b28
020e6b15: lea    rcx, [rip + 0x1c223e4]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6b1c: call   0x182f609b0
020e6b21: mov    byte ptr [rip + 0x1e327f2], 1            ; [0x3f1931a] (bss)
020e6b28: xor    ecx, ecx
020e6b2a: call   0x1801bd810
020e6b2f: mov    rcx, qword ptr [rip + 0x1c223ca]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6b36: mov    rbx, rax
020e6b39: cmp    qword ptr [rcx + 0x38], 0
020e6b3e: jne    0x1820e6b45
020e6b40: call   0x182f657d0
020e6b45: mov    qword ptr [rbp - 0x28], r14
020e6b49: test   rbx, rbx
020e6b4c: je     0x1820e6d9a
020e6b52: cmp    dword ptr [rbx + 0x18], 4
020e6b56: jae    0x1820e6b5f
020e6b58: xor    ecx, ecx
020e6b5a: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6b5f: lea    rax, [rbx + 0x20]
020e6b63: mov    dword ptr [rbp - 0x28], 4
020e6b6a: mov    qword ptr [rbp - 0x30], rax
020e6b6e: lea    rdx, [rbp - 0x10]
020e6b72: movaps xmm0, xmmword ptr [rbp - 0x30]
020e6b76: xor    r8d, r8d
020e6b79: mov    rcx, rdi
020e6b7c: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6b81: call   0x1805f18b0
020e6b86: cmp    dword ptr [rbx + 0x18], 0
020e6b8a: jbe    0x1820e6dbe
020e6b90: cmp    byte ptr [rip + 0x1e32783], 0            ; [0x3f1931a] (bss)
020e6b97: mov    r14d, dword ptr [rbx + 0x20]
020e6b9b: mov    rdi, qword ptr [r13 + 0x10]
020e6b9f: jne    0x1820e6bb4
020e6ba1: lea    rcx, [rip + 0x1c22358]                   ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6ba8: call   0x182f609b0
020e6bad: mov    byte ptr [rip + 0x1e32766], 1            ; [0x3f1931a] (bss)
020e6bb4: xor    ecx, ecx
020e6bb6: call   0x1801bd810
020e6bbb: mov    rcx, qword ptr [rip + 0x1c2233e]         ; [0x3d08f00] metamethod:Method$System.MemoryExtensions.AsSpan<byte>()
020e6bc2: mov    rbx, rax
020e6bc5: cmp    qword ptr [rcx + 0x38], 0
020e6bca: jne    0x1820e6bd1
020e6bcc: call   0x182f657d0
020e6bd1: mov    qword ptr [rbp - 0x18], 0
020e6bd9: test   rbx, rbx
020e6bdc: je     0x1820e6d9a
020e6be2: cmp    dword ptr [rbx + 0x18], 4
020e6be6: jae    0x1820e6bef
020e6be8: xor    ecx, ecx
020e6bea: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6bef: lea    rax, [rbx + 0x20]
020e6bf3: mov    dword ptr [rbp - 0x18], 4
020e6bfa: mov    qword ptr [rbp - 0x20], rax
020e6bfe: lea    rdx, [rbp - 0x10]
020e6c02: movaps xmm0, xmmword ptr [rbp - 0x20]
020e6c06: xor    r8d, r8d
020e6c09: mov    rcx, rdi
020e6c0c: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6c11: call   0x1805f18b0
020e6c16: cmp    dword ptr [rbx + 0x18], 0
020e6c1a: jbe    0x1820e6dbe
020e6c20: test   r15, r15
020e6c23: je     0x1820e6db8
020e6c29: cmp    byte ptr [rip + 0x1e326ee], 0            ; [0x3f1931e] (bss)
020e6c30: mov    ebx, dword ptr [rbx + 0x20]
020e6c33: jne    0x1820e6c60
020e6c35: lea    rcx, [rip + 0x1bf08a4]                   ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e6c3c: call   0x182f609b0
020e6c41: lea    rcx, [rip + 0x1bf0ad8]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
020e6c48: call   0x182f609b0
020e6c4d: lea    rcx, [rip + 0x1c13f5c]                   ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e6c54: call   0x182f609b0
020e6c59: mov    byte ptr [rip + 0x1e326be], 1            ; [0x3f1931e] (bss)
020e6c60: mov    rax, qword ptr [r15 + 0x50]
020e6c64: test   rax, rax
020e6c67: je     0x1820e6db8
020e6c6d: movsxd rcx, dword ptr [rax + 0x114]
020e6c74: xor    eax, eax
020e6c76: cmp    rsi, rcx
020e6c79: cmovle rsi, rax
020e6c7d: mov    rcx, qword ptr [r15 + 0x10]
020e6c81: test   rcx, rcx
020e6c84: je     0x1820e6db8
020e6c8a: mov    rdx, qword ptr [rip + 0x1c13f1f]         ; [0x3cfabb0] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʾʳʿʽʸʸʷʾʼʴʿ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020e6c91: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020e6c96: test   rax, rax
020e6c99: je     0x1820e6db8
020e6c9f: mov    qword ptr [rsp + 0x28], 0
020e6ca8: mov    r9, rsi
020e6cab: mov    r8d, r14d
020e6cae: mov    dword ptr [rsp + 0x20], ebx
020e6cb2: mov    rdx, r12
020e6cb5: mov    rcx, rax
020e6cb8: call   0x18214d240                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʺʷʻʲʴʺʷʺʺʶʴ
020e6cbd: mov    rcx, qword ptr [r15 + 0x60]
020e6cc1: mov    r9, rax
020e6cc4: test   rcx, rcx
020e6cc7: je     0x1820e6db8
020e6ccd: test   rax, rax
020e6cd0: je     0x1820e6db8
020e6cd6: mov    ecx, dword ptr [rcx + 0x18]
020e6cd9: mov    dword ptr [rax + 0x28], ecx
020e6cdc: mov    rcx, qword ptr [r15 + 0x60]
020e6ce0: test   rcx, rcx
020e6ce3: je     0x1820e6db8
020e6ce9: mov    r10, qword ptr [rip + 0x1bf07f0]         ; [0x3cd74e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.Add()
020e6cf0: inc    dword ptr [rcx + 0x1c]
020e6cf3: mov    rdx, qword ptr [rcx + 0x10]
020e6cf7: test   rdx, rdx
020e6cfa: je     0x1820e6db8
020e6d00: movsxd r8, dword ptr [rcx + 0x18]
020e6d04: cmp    r8d, dword ptr [rdx + 0x18]
020e6d08: jb     0x1820e6d23
020e6d0a: mov    rax, qword ptr [r10 + 0x20]
020e6d0e: mov    rdx, r9
020e6d11: mov    r8, qword ptr [rax + 0xc0]
020e6d18: mov    r8, qword ptr [r8 + 0x70]
020e6d1c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
020e6d21: jmp    0x1820e6d49
020e6d23: lea    eax, [r8 + 1]
020e6d27: mov    dword ptr [rcx + 0x18], eax
020e6d2a: cmp    r8d, dword ptr [rdx + 0x18]
020e6d2e: jae    0x1820e6dbe
020e6d34: mov    qword ptr [rdx + r8*8 + 0x20], r9
020e6d39: add    rdx, 0x20
020e6d3d: lea    rcx, [rdx + r8*8]
020e6d41: mov    rdx, r9
020e6d44: call   0x182f5fc00
020e6d49: mov    ecx, dword ptr [rbp + 0x40]
020e6d4c: mov    r14d, 0
020e6d52: inc    ecx
020e6d54: movsxd rax, ecx
020e6d57: mov    dword ptr [rbp + 0x40], ecx
020e6d5a: cmp    rax, qword ptr [rbp + 0x58]
020e6d5e: jl     0x1820e69f0
020e6d64: jmp    0x1820e6d6b
020e6d66: test   r15, r15
020e6d69: je     0x1820e6db8
020e6d6b: xor    r8d, r8d
020e6d6e: mov    byte ptr [r15 + 0xa2], 0
020e6d76: xor    edx, edx
020e6d78: mov    rcx, r15
020e6d7b: mov    rbx, qword ptr [rsp + 0xc8]
020e6d83: add    rsp, 0x80
020e6d8a: pop    r15
020e6d8c: pop    r14
020e6d8e: pop    r13
020e6d90: pop    r12
020e6d92: pop    rdi
020e6d93: pop    rsi
020e6d94: pop    rbp
020e6d95: jmp    0x18215be50                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʴʷʸʽʴʼʶˀʶʲʷ
020e6d9a: xor    ecx, ecx
020e6d9c: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020e6da1: xorps  xmm0, xmm0
020e6da4: lea    rdx, [rbp - 0x10]
020e6da8: xor    r8d, r8d
020e6dab: movdqa xmmword ptr [rbp - 0x10], xmm0
020e6db0: mov    rcx, rdi
020e6db3: call   0x1805f18b0
020e6db8: call   0x182f60c50
020e6dbd: int3   
020e6dbe: call   0x182f60c40
020e6dc3: int3   
020e6dc4: int3   
=== 20E6DD0
020e6dd0: mov    qword ptr [rsp + 0x10], rsi
020e6dd5: push   rdi
020e6dd6: sub    rsp, 0x20
020e6dda: xor    edx, edx
020e6ddc: mov    rsi, rcx
020e6ddf: call   0x1820e4e90                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʹʴʷʳʷʲʺˁʾʻˁ
020e6de4: mov    rdi, rax
020e6de7: test   rax, rax
020e6dea: je     0x1820e6e8b
020e6df0: mov    rcx, qword ptr [rax + 0x30]
020e6df4: test   rcx, rcx
020e6df7: je     0x1820e6e8b
020e6dfd: movzx  r8d, byte ptr [rsi + 0x1c]
020e6e02: xor    r9d, r9d
020e6e05: movzx  edx, byte ptr [rsi + 0x1d]
020e6e09: mov    qword ptr [rsp + 0x30], rbx
020e6e0e: call   0x1820cfd40
020e6e13: xor    r8d, r8d
020e6e16: mov    rdx, rdi
020e6e19: mov    rcx, rsi
020e6e1c: mov    rbx, rax
020e6e1f: call   0x1820e4040                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʵˁʲʹʴʷʸʾʳʵʾ
020e6e24: xor    r8d, r8d
020e6e27: mov    rdx, rdi
020e6e2a: mov    rcx, rsi
020e6e2d: call   0x1820e5940                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ˁʲʳʹʴˁʹʴʳʽʶ
020e6e32: xor    r8d, r8d
020e6e35: mov    rdx, rbx
020e6e38: mov    rcx, rsi
020e6e3b: call   0x1820e6410                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ˁʳʳʳʲˁˁˀʳʾʴ
020e6e40: xor    r8d, r8d
020e6e43: mov    rdx, rbx
020e6e46: mov    rcx, rsi
020e6e49: call   0x1820e39a0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʳʽʶʵʻˁʸʴʽʸʻ
020e6e4e: xor    r8d, r8d
020e6e51: mov    rdx, rbx
020e6e54: mov    rcx, rsi
020e6e57: call   0x1820e3510                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʳʵˁʿʲʺʷʲʳʲʿ
020e6e5c: xor    r8d, r8d
020e6e5f: mov    rdx, rbx
020e6e62: mov    rcx, rsi
020e6e65: call   0x1820e49d0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ʸʴʼʳʸʼʶʵʺʷʼ
020e6e6a: xor    r8d, r8d
020e6e6d: mov    rdx, rbx
020e6e70: mov    rcx, rsi
020e6e73: call   0x1820e69a0                              ; ʿˀʻʳʺʻʼʲʷʻʷ$$ˁʻʴʵʾʻʷʻʾʶʸ
020e6e78: mov    rsi, qword ptr [rsp + 0x38]
020e6e7d: mov    rax, rbx
020e6e80: mov    rbx, qword ptr [rsp + 0x30]
020e6e85: add    rsp, 0x20
020e6e89: pop    rdi
020e6e8a: ret    
020e6e8b: call   0x182f60c50
020e6e90: int3   
020e6e91: int3   
=== 20E6EA0
020e6ea0: mov    qword ptr [rsp + 8], rbx
020e6ea5: mov    qword ptr [rsp + 0x10], rsi
020e6eaa: push   rdi
020e6eab: sub    rsp, 0x30
020e6eaf: mov    rbx, rdx
020e6eb2: movaps xmmword ptr [rsp + 0x20], xmm6
020e6eb7: xor    edx, edx
020e6eb9: movzx  esi, r9b
020e6ebd: movaps xmm6, xmm2
020e6ec0: mov    rdi, rcx
020e6ec3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020e6ec8: lea    rcx, [rdi + 0x10]
020e6ecc: mov    qword ptr [rdi + 0x10], rbx
020e6ed0: mov    rdx, rbx
020e6ed3: call   0x182f5fc00
020e6ed8: movzx  eax, byte ptr [rsp + 0x60]
020e6edd: mov    rbx, qword ptr [rsp + 0x40]
020e6ee2: movss  dword ptr [rdi + 0x18], xmm6
020e6ee7: movaps xmm6, xmmword ptr [rsp + 0x20]
020e6eec: mov    byte ptr [rdi + 0x1d], sil
020e6ef0: mov    rsi, qword ptr [rsp + 0x48]
020e6ef5: mov    byte ptr [rdi + 0x1c], al
020e6ef8: add    rsp, 0x30
020e6efc: pop    rdi
020e6efd: ret    
020e6efe: int3   
