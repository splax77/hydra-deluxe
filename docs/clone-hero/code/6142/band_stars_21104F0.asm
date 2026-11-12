021104f0: mov    qword ptr [rsp + 0x10], rbx
021104f5: mov    qword ptr [rsp + 8], rcx
021104fa: push   rsi
021104fb: push   rdi
021104fc: push   r13
021104fe: push   r14
02110500: push   r15
02110502: sub    rsp, 0x70
02110506: mov    rdi, rcx
02110509: cmp    byte ptr [rip + 0x1e08f5b], 0            ; [0x3f1946b] (bss)
02110510: jne    0x182110591
02110512: lea    rcx, [rip + 0x1b98827]                   ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110519: call   0x182f609b0
0211051e: lea    rcx, [rip + 0x1b99cf3]                   ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
02110525: call   0x182f609b0
0211052a: lea    rcx, [rip + 0x1baee3f]                   ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
02110531: call   0x182f609b0
02110536: lea    rcx, [rip + 0x1baeef3]                   ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0211053d: call   0x182f609b0
02110542: lea    rcx, [rip + 0x1baefa7]                   ; [0x3cbf4f0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Current()
02110549: call   0x182f609b0
0211054e: lea    rcx, [rip + 0x1bae6cb]                   ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
02110555: call   0x182f609b0
0211055a: lea    rcx, [rip + 0x1bbda67]                   ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
02110561: call   0x182f609b0
02110566: lea    rcx, [rip + 0x1bbdb1b]                   ; [0x3cce088] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.get_Count()
0211056d: call   0x182f609b0
02110572: lea    rcx, [rip + 0x1bcb1af]                   ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
02110579: call   0x182f609b0
0211057e: lea    rcx, [rip + 0x1bf67a3]                   ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110585: call   0x182f609b0
0211058a: mov    byte ptr [rip + 0x1e08eda], 1            ; [0x3f1946b] (bss)
02110591: xorps  xmm0, xmm0
02110594: xor    eax, eax
02110596: movups xmmword ptr [rsp + 0x50], xmm0
0211059b: mov    qword ptr [rsp + 0x60], rax
021105a0: test   rdi, rdi
021105a3: je     0x18211090b
021105a9: mov    r14, qword ptr [rdi + 0x50]
021105ad: mov    rcx, qword ptr [rip + 0x1bf6774]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021105b4: cmp    dword ptr [rcx + 0xe0], eax
021105ba: jne    0x1821105c8
021105bc: call   0x182f60cf0
021105c1: mov    rcx, qword ptr [rip + 0x1bf6760]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021105c8: mov    rax, qword ptr [rcx + 0xb8]
021105cf: mov    rsi, qword ptr [rax + 8]
021105d3: test   rsi, rsi
021105d6: jne    0x182110646
021105d8: cmp    dword ptr [rcx + 0xe0], esi
021105de: jne    0x1821105ec
021105e0: call   0x182f60cf0
021105e5: mov    rcx, qword ptr [rip + 0x1bf673c]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
021105ec: mov    rax, qword ptr [rcx + 0xb8]
021105f3: mov    rbx, qword ptr [rax]
021105f6: mov    rcx, qword ptr [rip + 0x1bae623]         ; [0x3cbec20] meta:System.Func<ʳʲʵʿʺʷˁʺˀʿʵ, bool>_TypeInfo
021105fd: call   0x182f60c00
02110602: mov    rsi, rax
02110605: xor    r9d, r9d
02110608: mov    r8, qword ptr [rip + 0x1bcb119]          ; [0x3cdb728] metamethod:Method$ʿˁʺˀʷʼʽʼʶʸʻ.<>c.ʹʺˀʿˁʲʸʶʽʲˁ()
0211060f: mov    rdx, rbx
02110612: mov    rcx, rax
02110615: call   0x18170ede0                              ; System.Data.Listeners.Func<object, object, bool>$$.ctor
0211061a: mov    rax, qword ptr [rip + 0x1bf6707]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110621: mov    r8, qword ptr [rax + 0xb8]
02110628: mov    qword ptr [r8 + 8], rsi
0211062c: mov    rax, qword ptr [rip + 0x1bf66f5]         ; [0x3d06d28] meta:ʿˁʺˀʷʼʽʼʶʸʻ.<>c_TypeInfo
02110633: mov    rcx, qword ptr [rax + 0xb8]
0211063a: add    rcx, 8
0211063e: mov    rdx, rsi
02110641: call   0x182f5fc00
02110646: mov    r8, qword ptr [rip + 0x1b99bcb]          ; [0x3caa218] metamethod:Method$System.Linq.Enumerable.Where<ʳʲʵʿʺʷˁʺˀʿʵ>()
0211064d: mov    rdx, rsi
02110650: mov    rcx, r14
02110653: call   0x18048ee00                              ; System.Linq.Enumerable$$Where<object>
02110658: mov    rdx, qword ptr [rip + 0x1b986e1]         ; [0x3ca8d40] metamethod:Method$System.Linq.Enumerable.ToList<ʳʲʵʿʺʷˁʺˀʿʵ>()
0211065f: mov    rcx, rax
02110662: call   0x18048e370                              ; System.Linq.Enumerable$$ToList<object>
02110667: mov    r13, rax
0211066a: mov    qword ptr [rsp + 0x20], rax
0211066f: mov    r15d, 1
02110675: mov    qword ptr [rdi + 0x10], r15
02110679: mov    dword ptr [rdi + 0x18], 0
02110680: mov    dword ptr [rdi + 0x1c], r15d
02110684: mov    word ptr [rdi + 0x24], r15w
02110689: test   rax, rax
0211068c: je     0x18211090b
02110692: mov    dword ptr [rsp + 0xb0], r15d
0211069a: xor    r14d, r14d
0211069d: mov    dword ptr [rsp + 0xb8], r14d
021106a5: mov    r8, qword ptr [rip + 0x1bbd91c]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
021106ac: mov    rdx, rax
021106af: lea    rcx, [rsp + 0x38]
021106b4: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
021106b9: movups xmm0, xmmword ptr [rsp + 0x38]
021106be: movups xmmword ptr [rsp + 0x50], xmm0
021106c3: movsd  xmm1, qword ptr [rsp + 0x48]
021106c9: movsd  qword ptr [rsp + 0x60], xmm1
021106cf: mov    qword ptr [rsp + 0x38], r14
021106d4: lea    rbx, [rsp + 0x50]
021106d9: mov    qword ptr [rsp + 0x40], rbx
021106de: nop    
021106e0: mov    rdx, qword ptr [rip + 0x1baed49]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
021106e7: lea    rcx, [rsp + 0x50]
021106ec: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
021106f1: test   al, al
021106f3: je     0x182110799
021106f9: mov    rsi, qword ptr [rsp + 0x60]
021106fe: test   rsi, rsi
02110701: je     0x1821108f3
02110707: mov    eax, dword ptr [rsi + 0x14]
0211070a: add    dword ptr [rdi + 0x18], eax
0211070d: mov    eax, dword ptr [rsi + 0x10]
02110710: add    dword ptr [rdi + 0x10], eax
02110713: mov    eax, dword ptr [rsi + 0x18]
02110716: add    dword ptr [rdi + 0x14], eax
02110719: mov    eax, dword ptr [rsi + 0x1c]
0211071c: add    dword ptr [rdi + 0x1c], eax
0211071f: xor    edx, edx
02110721: mov    rcx, rsi
02110724: call   0x182130ec0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʽʹʶˁʻˀʸʵʴʴʷ
02110729: movzx  eax, al
0211072c: add    r15d, eax
0211072f: mov    dword ptr [rsp + 0xb0], r15d
02110737: xor    edx, edx
02110739: mov    rcx, rsi
0211073c: call   0x18212f510                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʵʴʽʴˁʽʲʸʷʺ
02110741: test   al, al
02110743: je     0x18211074d
02110745: mov    eax, dword ptr [rdi + 0x28]
02110748: mov    dword ptr [rdi + 0x28], eax
0211074b: jmp    0x182110753
0211074d: mov    word ptr [rdi + 0x24], 0
02110753: xor    edx, edx
02110755: mov    rcx, rsi
02110758: call   0x18212f5d0                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʲʷʵʹʲʼˀʶˀʲʴ
0211075d: test   al, al
0211075f: je     0x182110769
02110761: mov    eax, dword ptr [rdi + 0x2c]
02110764: mov    dword ptr [rdi + 0x2c], eax
02110767: jmp    0x18211076d
02110769: mov    byte ptr [rdi + 0x25], 0
0211076d: xor    edx, edx
0211076f: mov    rcx, rsi
02110772: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
02110777: cmp    r14d, eax
0211077a: jge    0x1821106e0
02110780: xor    edx, edx
02110782: mov    rcx, rsi
02110785: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
0211078a: mov    r14d, eax
0211078d: mov    dword ptr [rsp + 0xb8], eax
02110794: jmp    0x1821106e0
02110799: mov    rdx, qword ptr [rip + 0x1baebd0]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
021107a0: mov    rcx, rbx
021107a3: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021107a8: jmp    0x1821107e6
021107aa: mov    rdx, qword ptr [rip + 0x1baebbf]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
021107b1: mov    rcx, qword ptr [rsp + 0x40]
021107b6: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021107bb: mov    rcx, qword ptr [rsp + 0x38]
021107c0: test   rcx, rcx
021107c3: jne    0x1821108f9
021107c9: mov    rdi, qword ptr [rsp + 0xa0]
021107d1: mov    r15d, dword ptr [rsp + 0xb0]
021107d9: mov    r14d, dword ptr [rsp + 0xb8]
021107e1: mov    r13, qword ptr [rsp + 0x20]
021107e6: mov    eax, r15d
021107e9: cdq    
021107ea: idiv   dword ptr [r13 + 0x18]
021107ee: mov    byte ptr [rdi + 0x4c], al
021107f1: xor    r8d, r8d
021107f4: mov    edx, dword ptr [rdi + 0x10]
021107f7: mov    ecx, dword ptr [rdi + 0x18]
021107fa: call   0x18210dd00                              ; ʿˁʺˀʷʼʽʼʶʸʻ$$ʳʴʽʲʺʹˁʵʼʶʴ
021107ff: mov    dword ptr [rdi + 0x20], eax
02110802: mov    ecx, dword ptr [rdi + 0x18]
02110805: add    ecx, dword ptr [rdi + 0x14]
02110808: mov    dword ptr [rdi + 0x18], ecx
0211080b: mov    eax, dword ptr [rdi + 0x1c]
0211080e: add    eax, ecx
02110810: mov    dword ptr [rdi + 0x18], eax
02110813: xor    r15b, r15b
02110816: mov    r8, qword ptr [rip + 0x1bbd7ab]          ; [0x3ccdfc8] metamethod:Method$System.Collections.Generic.List<ʳʲʵʿʺʷˁʺˀʿʵ>.GetEnumerator()
0211081d: mov    rdx, r13
02110820: lea    rcx, [rsp + 0x38]
02110825: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0211082a: movups xmm0, xmmword ptr [rsp + 0x38]
0211082f: movups xmmword ptr [rsp + 0x50], xmm0
02110834: movsd  xmm1, qword ptr [rsp + 0x48]
0211083a: movsd  qword ptr [rsp + 0x60], xmm1
02110840: mov    qword ptr [rsp + 0x38], 0
02110849: lea    rbx, [rsp + 0x50]
0211084e: mov    qword ptr [rsp + 0x40], rbx
02110853: mov    rdx, qword ptr [rip + 0x1baebd6]         ; [0x3cbf430] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.MoveNext()
0211085a: lea    rcx, [rsp + 0x50]
0211085f: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
02110864: test   al, al
02110866: je     0x1821108b2
02110868: mov    rsi, qword ptr [rsp + 0x60]
0211086d: test   rsi, rsi
02110870: je     0x1821108ff
02110876: xor    edx, edx
02110878: mov    rcx, rsi
0211087b: call   0x182130490                              ; ʳʲʵʿʺʷˁʺˀʿʵ$$ʴʿʴʽʺˁʽʵʹʳʾ
02110880: cmp    r14d, eax
02110883: sete   dl
02110886: movzx  eax, dl
02110889: xor    ecx, ecx
0211088b: test   r15b, r15b
0211088e: cmove  ecx, eax
02110891: test   ecx, ecx
02110893: jne    0x1821108a6
02110895: test   r15b, dl
02110898: je     0x1821108aa
0211089a: mov    byte ptr [rdi + 0x4d], 1
0211089e: mov    byte ptr [rsi + 0xa9], dl
021108a4: jmp    0x182110853
021108a6: movzx  r15d, dl
021108aa: mov    byte ptr [rsi + 0xa9], dl
021108b0: jmp    0x182110853
021108b2: mov    rdx, qword ptr [rip + 0x1baeab7]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
021108b9: mov    rcx, rbx
021108bc: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021108c1: jmp    0x1821108de
021108c3: mov    rdx, qword ptr [rip + 0x1baeaa6]         ; [0x3cbf370] metamethod:Method$System.Collections.Generic.List.Enumerator<ʳʲʵʿʺʷˁʺˀʿʵ>.Dispose()
021108ca: mov    rcx, qword ptr [rsp + 0x40]
021108cf: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
021108d4: mov    rcx, qword ptr [rsp + 0x38]
021108d9: test   rcx, rcx
021108dc: jne    0x182110905
021108de: mov    rbx, qword ptr [rsp + 0xa8]
021108e6: add    rsp, 0x70
021108ea: pop    r15
021108ec: pop    r14
021108ee: pop    r13
021108f0: pop    rdi
021108f1: pop    rsi
021108f2: ret    
021108f3: call   0x182f60c50
021108f8: nop    
021108f9: call   0x182f60ce0
021108fe: nop    
021108ff: call   0x182f60c50
02110904: nop    
02110905: call   0x182f60ce0
0211090a: int3   
0211090b: call   0x182f60c50
02110910: int3   
02110911: int3   
