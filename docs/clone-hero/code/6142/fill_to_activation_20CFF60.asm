020cff60: mov    qword ptr [rsp + 0x10], rbx
020cff65: mov    qword ptr [rsp + 0x20], rsi
020cff6a: mov    qword ptr [rsp + 8], rcx
020cff6f: push   rdi
020cff70: push   r12
020cff72: push   r13
020cff74: push   r14
020cff76: push   r15
020cff78: sub    rsp, 0x70
020cff7c: mov    r13, r8
020cff7f: mov    r12, rdx
020cff82: mov    r14, rcx
020cff85: cmp    byte ptr [rip + 0x1e49379], 0            ; [0x3f19305] (bss)
020cff8c: jne    0x1820d000d
020cff8e: lea    rcx, [rip + 0x1beef5b]                   ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020cff95: call   0x182f609b0
020cff9a: lea    rcx, [rip + 0x1bef00f]                   ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
020cffa1: call   0x182f609b0
020cffa6: lea    rcx, [rip + 0x1bef0c3]                   ; [0x3cbf070] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Current()
020cffad: call   0x182f609b0
020cffb2: lea    rcx, [rip + 0x1bfe847]                   ; [0x3cce800] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.Add()
020cffb9: call   0x182f609b0
020cffbe: lea    rcx, [rip + 0x1bfd403]                   ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
020cffc5: call   0x182f609b0
020cffca: lea    rcx, [rip + 0x1bfe76f]                   ; [0x3cce740] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020cffd1: call   0x182f609b0
020cffd6: lea    rcx, [rip + 0x1bfd4ab]                   ; [0x3ccd488] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.get_Count()
020cffdd: call   0x182f609b0
020cffe2: lea    rcx, [rip + 0x1c24727]                   ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020cffe9: call   0x182f609b0
020cffee: lea    rcx, [rip + 0x1bc20ab]                   ; [0x3c920a0] meta:ʴʴʶʺʻˁʻʿʽʵʲ_TypeInfo
020cfff5: call   0x182f609b0
020cfffa: lea    rcx, [rip + 0x1bc8917]                   ; [0x3c98918] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʷʾˀʼˀʵʹˁʻʻʷ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d0001: call   0x182f609b0
020d0006: mov    byte ptr [rip + 0x1e492f8], 1            ; [0x3f19305] (bss)
020d000d: xorps  xmm0, xmm0
020d0010: xor    eax, eax
020d0012: movups xmmword ptr [rsp + 0x58], xmm0
020d0017: mov    qword ptr [rsp + 0x68], rax
020d001c: test   r13, r13
020d001f: je     0x1820d025c
020d0025: mov    rbx, qword ptr [r13 + 0x88]
020d002c: test   rbx, rbx
020d002f: je     0x1820d025c
020d0035: mov    ebx, dword ptr [rbx + 0x18]
020d0038: mov    rcx, qword ptr [rip + 0x1c246d1]         ; [0x3cf4710] meta:System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>_TypeInfo
020d003f: call   0x182f60c00
020d0044: mov    r15, rax
020d0047: mov    r8, qword ptr [rip + 0x1bfe6f2]          ; [0x3cce740] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>..ctor()
020d004e: mov    edx, ebx
020d0050: mov    rcx, rax
020d0053: call   0x180706bc0                              ; System.Collections.Generic.List<UserDataStore_PlayerPrefs.ControllerElementByRoleMap.Entry>$$.ctor
020d0058: test   r12, r12
020d005b: je     0x1820d025c
020d0061: movsd  xmm0, qword ptr [r12 + 0xb8]
020d006b: mulsd  xmm0, qword ptr [rip + 0x107064d]        ; [0x31406c0] dbl=0.03125 q=0x3fa0000000000000
020d0073: mov    rdx, qword ptr [r13 + 0x88]
020d007a: test   rdx, rdx
020d007d: je     0x1820d025c
020d0083: mov    qword ptr [rsp + 0x30], r15
020d0088: cvttsd2si rdi, xmm0
020d008d: inc    rdi
020d0090: mov    qword ptr [rsp + 0xb0], rdi
020d0098: mov    r8, qword ptr [rip + 0x1bfd329]          ; [0x3ccd3c8] metamethod:Method$System.Collections.Generic.List<ʲʵʺʹʿʵʹʷʿʲʻ>.GetEnumerator()
020d009f: lea    rcx, [rsp + 0x40]
020d00a4: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
020d00a9: movups xmm0, xmmword ptr [rsp + 0x40]
020d00ae: movups xmmword ptr [rsp + 0x58], xmm0
020d00b3: movsd  xmm1, qword ptr [rsp + 0x50]
020d00b9: movsd  qword ptr [rsp + 0x68], xmm1
020d00bf: mov    qword ptr [rsp + 0x40], 0
020d00c8: lea    rbx, [rsp + 0x58]
020d00cd: mov    qword ptr [rsp + 0x48], rbx
020d00d2: jmp    0x1820d00e8
020d00d4: nop    dword ptr [rax]
020d00d8: nop    dword ptr [rax + rax]
020d00e0: mov    r14, qword ptr [rsp + 0xa0]
020d00e8: mov    rdx, qword ptr [rip + 0x1beeec1]         ; [0x3cbefb0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.MoveNext()
020d00ef: lea    rcx, [rsp + 0x58]
020d00f4: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
020d00f9: test   al, al
020d00fb: je     0x1820d020e
020d0101: mov    rsi, qword ptr [rsp + 0x68]
020d0106: test   rsi, rsi
020d0109: je     0x1820d0272
020d010f: cmp    byte ptr [rsi + 0x20], 0
020d0113: jne    0x1820d00e8
020d0115: mov    rdx, qword ptr [rsi + 0x18]
020d0119: add    rdx, qword ptr [rsi + 0x10]
020d011d: mov    rax, qword ptr [rip + 0x1bc87f4]         ; [0x3c98918] metamethod:Method$ʽʹʽʽˁʹʷʺˀʺˁ.ʷʾˀʼˀʵʹˁʻʻʷ<ʽʿʸʸʾʶʶʾʶʹʲ>()
020d0124: lea    r9, [rdx + rdi]
020d0128: mov    qword ptr [rsp + 0x28], rax
020d012d: mov    dword ptr [rsp + 0x20], 0
020d0135: mov    r8, qword ptr [rsi + 0x10]
020d0139: mov    rcx, r14
020d013c: call   0x1805de030                              ; ʽʹʽʽˁʹʷʺˀʺˁ$$ʷʾˀʼˀʵʹˁʻʻʷ<object>
020d0141: test   rax, rax
020d0144: je     0x1820d00e8
020d0146: xor    r8d, r8d
020d0149: mov    edx, 0xffffffff
020d014e: mov    rcx, rax
020d0151: call   0x1820d51d0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ˀʶˀʾʴʳʹʶˁʹʹ
020d0156: mov    r14, rax
020d0159: test   rax, rax
020d015c: je     0x1820d00e0
020d015e: or     dword ptr [rax + 0x7c], 0x800
020d0165: mov    rcx, qword ptr [rip + 0x1bc1f34]         ; [0x3c920a0] meta:ʴʴʶʺʻˁʻʿʽʵʲ_TypeInfo
020d016c: call   0x182f60c00
020d0171: mov    rdi, rax
020d0174: xor    edx, edx
020d0176: mov    rcx, rax
020d0179: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d017e: xor    r8d, r8d
020d0181: mov    rdx, qword ptr [rsi + 0x10]
020d0185: mov    rcx, r12
020d0188: call   0x18212b9c0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʻʵˁʺʾʹʸʻʸʿ
020d018d: test   rdi, rdi
020d0190: je     0x1820d026d
020d0196: movsd  qword ptr [rdi + 0x10], xmm0
020d019b: mov    rcx, qword ptr [r13 + 0x50]
020d019f: test   rcx, rcx
020d01a2: je     0x1820d0268
020d01a8: xor    r9d, r9d
020d01ab: mov    r8, qword ptr [r14 + 0x40]
020d01af: mov    rdx, qword ptr [rsi + 0x10]
020d01b3: call   0x1820cfce0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʴʸˁʳˀʽʼʺʸʾˁ
020d01b8: cvtsd2ss xmm0, xmm0
020d01bc: movss  dword ptr [rdi + 0x18], xmm0
020d01c1: mov    qword ptr [rdi + 0x20], r14
020d01c5: lea    rcx, [rdi + 0x20]
020d01c9: mov    rdx, r14
020d01cc: call   0x182f5fc00
020d01d1: mov    rax, qword ptr [rsi + 0x10]
020d01d5: mov    qword ptr [rdi + 0x28], rax
020d01d9: mov    eax, dword ptr [r14 + 0x40]
020d01dd: sub    eax, dword ptr [rsi + 0x10]
020d01e0: cdqe   
020d01e2: mov    qword ptr [rdi + 0x30], rax
020d01e6: mov    byte ptr [rdi + 0x38], 1
020d01ea: test   r15, r15
020d01ed: je     0x1820d0262
020d01ef: mov    r8, qword ptr [rip + 0x1bfe60a]          ; [0x3cce800] metamethod:Method$System.Collections.Generic.List<ʴʴʶʺʻˁʻʿʽʵʲ>.Add()
020d01f6: mov    rdx, rdi
020d01f9: mov    rcx, r15
020d01fc: call   0x182c425f0
020d0201: mov    rdi, qword ptr [rsp + 0xb0]
020d0209: jmp    0x1820d00e0
020d020e: mov    rdx, qword ptr [rip + 0x1beecdb]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020d0215: mov    rcx, rbx
020d0218: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d021d: jmp    0x1820d023f
020d021f: mov    rdx, qword ptr [rip + 0x1beecca]         ; [0x3cbeef0] metamethod:Method$System.Collections.Generic.List.Enumerator<ʲʵʺʹʿʵʹʷʿʲʻ>.Dispose()
020d0226: mov    rcx, qword ptr [rsp + 0x48]
020d022b: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
020d0230: mov    rcx, qword ptr [rsp + 0x40]
020d0235: test   rcx, rcx
020d0238: jne    0x1820d0278
020d023a: mov    r15, qword ptr [rsp + 0x30]
020d023f: mov    rax, r15
020d0242: lea    r11, [rsp + 0x70]
020d0247: mov    rbx, qword ptr [r11 + 0x38]
020d024b: mov    rsi, qword ptr [r11 + 0x48]
020d024f: mov    rsp, r11
020d0252: pop    r15
020d0254: pop    r14
020d0256: pop    r13
020d0258: pop    r12
020d025a: pop    rdi
020d025b: ret    
020d025c: call   0x182f60c50
020d0261: nop    
020d0262: call   0x182f60c50
020d0267: nop    
020d0268: call   0x182f60c50
020d026d: call   0x182f60c50
020d0272: call   0x182f60c50
020d0277: nop    
020d0278: call   0x182f60ce0
020d027d: int3   
020d027e: int3   
