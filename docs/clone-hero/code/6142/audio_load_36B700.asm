0036b700: mov    qword ptr [rsp + 0x10], rdx
0036b705: push   rbx
0036b706: push   rdi
0036b707: push   r13
0036b709: push   r14
0036b70b: sub    rsp, 0xd8
0036b712: cmp    byte ptr [rip + 0x3ba29e3], 0            ; [0x3f0e0fc] (bss)
0036b719: movzx  edi, r9b
0036b71d: movzx  ebx, r8b
0036b721: mov    r14, rdx
0036b724: mov    r13, rcx
0036b727: jne    0x18036b800
0036b72d: lea    rcx, [rip + 0x395026c]                   ; [0x3cbb9a0] metamethod:Method$BassAudioManager.ʵˀʼʸʻʻʼʵʸʲʵ()
0036b734: call   0x182f609b0
0036b739: lea    rcx, [rip + 0x3950718]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036b740: call   0x182f609b0
0036b745: lea    rcx, [rip + 0x394c934]                   ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
0036b74c: call   0x182f609b0
0036b751: lea    rcx, [rip + 0x394c710]                   ; [0x3cb7e68] metamethod:Method$System.Collections.Generic.List<string>..ctor()
0036b758: call   0x182f609b0
0036b75d: lea    rcx, [rip + 0x3984b6c]                   ; [0x3cf02d0] meta:System.Collections.Generic.List<string>_TypeInfo
0036b764: call   0x182f609b0
0036b769: lea    rcx, [rip + 0x394b9e0]                   ; [0x3cb7150] meta:System.IO.Path_TypeInfo
0036b770: call   0x182f609b0
0036b775: lea    rcx, [rip + 0x393efdc]                   ; [0x3caa758] metamethod:Method$System.Span<string>.get_Length()
0036b77c: call   0x182f609b0
0036b781: lea    rcx, [rip + 0x39267a0]                   ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036b788: call   0x182f609b0
0036b78d: lea    rcx, [rip + 0x3927454]                   ; [0x3c92be8] meta:ʴʾʴʹʳʼʼʲʸʾʸ_TypeInfo
0036b794: call   0x182f609b0
0036b799: lea    rcx, [rip + 0x398b200]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0036b7a0: call   0x182f609b0
0036b7a5: lea    rcx, [rip + 0x392cb74]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0036b7ac: call   0x182f609b0
0036b7b1: lea    rcx, [rip + 0x392d090]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0036b7b8: call   0x182f609b0
0036b7bd: lea    rcx, [rip + 0x392f22c]                   ; [0x3c9a9f0] meta:ʼʳʾʺʻˀʻʸʵʾʶ_TypeInfo
0036b7c4: call   0x182f609b0
0036b7c9: lea    rcx, [rip + 0x3932a58]                   ; [0x3c9e228] meta:ʿʷʼˁʺʻʴʿʴʺʾ_TypeInfo
0036b7d0: call   0x182f609b0
0036b7d5: lea    rcx, [rip + 0x3999c54]                   ; [0x3d05430] meta:ʼʳʾʺʻˀʻʸʵʾʶ.ʿʿʳʳʻʴʿʹʽʼʵ_TypeInfo
0036b7dc: call   0x182f609b0
0036b7e1: lea    rcx, [rip + 0x3985b48]                   ; [0x3cf1330] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<string>()
0036b7e8: call   0x182f609b0
0036b7ed: lea    rcx, [rip + 0x3954d54]                   ; [0x3cc0548] str:'Failed to add channel to mixer'
0036b7f4: call   0x182f609b0
0036b7f9: mov    byte ptr [rip + 0x3ba28fc], 1            ; [0x3f0e0fc] (bss)
0036b800: mov    qword ptr [rsp + 0x100], rbp
0036b808: xor    r8d, r8d
0036b80b: mov    qword ptr [rsp + 0x110], rsi
0036b813: mov    dl, 1
0036b815: mov    qword ptr [rsp + 0x118], r12
0036b81d: mov    rcx, r13
0036b820: mov    qword ptr [rsp + 0xd0], r15
0036b828: movaps xmmword ptr [rsp + 0xc0], xmm6
0036b830: movaps xmmword ptr [rsp + 0xb0], xmm7
0036b838: call   0x180369810                              ; BassAudioManager$$ʺʴʻʼʴʿʼʿʳˁʷ
0036b83d: test   r14, r14
0036b840: je     0x18036c2b1
0036b846: movzx  eax, byte ptr [r14 + 0xb2]
0036b84e: mov    byte ptr [r13 + 0x5c], al
0036b852: movzx  ebp, byte ptr [r14 + 0xb3]
0036b85a: mov    byte ptr [rsp + 0x30], bpl
0036b85f: test   al, al
0036b861: jne    0x18036b87b
0036b863: test   bpl, bpl
0036b866: jne    0x18036b87b
0036b868: mov    rcx, qword ptr [r14 + 0x30]
0036b86c: xor    edx, edx
0036b86e: call   0x181b12270                              ; System.IO.Directory$$Exists
0036b873: test   al, al
0036b875: je     0x18036c197
0036b87b: mov    byte ptr [r13 + 0x3b], 0
0036b880: mov    byte ptr [r13 + 0x29], 0
0036b885: mov    byte ptr [r13 + 0x68], bl
0036b889: mov    rcx, qword ptr [rip + 0x39505c8]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036b890: cmp    dword ptr [rcx + 0xe0], 0
0036b897: jne    0x18036b89e
0036b899: call   0x182f60cf0
0036b89e: xor    edx, edx
0036b8a0: movzx  ecx, dil
0036b8a4: call   0x18036db60                              ; BassAudioManager$$ˀʿʷʾʳʶʲʴʷʸʽ
0036b8a9: movzx  ecx, byte ptr [rsp + 0x120]
0036b8b1: xor    edx, edx
0036b8b3: mov    rbx, rax
0036b8b6: mov    qword ptr [rsp + 0x58], rax
0036b8bb: call   0x18036db60                              ; BassAudioManager$$ˀʿʷʾʳʶʲʴʷʸʽ
0036b8c0: cmp    qword ptr [r13 + 0x70], 0
0036b8c5: mov    rdi, rax
0036b8c8: mov    qword ptr [rsp + 0x60], rax
0036b8cd: jne    0x18036b925
0036b8cf: mov    rcx, qword ptr [rip + 0x3999b5a]         ; [0x3d05430] meta:ʼʳʾʺʻˀʻʸʵʾʶ.ʿʿʳʳʻʴʿʹʽʼʵ_TypeInfo
0036b8d6: call   0x182f60c00
0036b8db: mov    r8, qword ptr [rip + 0x39500be]          ; [0x3cbb9a0] metamethod:Method$BassAudioManager.ʵˀʼʸʻʻʼʵʸʲʵ()
0036b8e2: xor    r9d, r9d
0036b8e5: mov    rdx, r13
0036b8e8: mov    rcx, rax
0036b8eb: mov    rbp, rax
0036b8ee: call   0x1803923b0                              ; ʼʳʾʺʻˀʻʸʵʾʶ.ʿʿʳʳʻʴʿʹʽʼʵ$$.ctor
0036b8f3: mov    rcx, qword ptr [rip + 0x392f0f6]         ; [0x3c9a9f0] meta:ʼʳʾʺʻˀʻʸʵʾʶ_TypeInfo
0036b8fa: call   0x182f60c00
0036b8ff: xor    r8d, r8d
0036b902: mov    rdx, rbp
0036b905: mov    rcx, rax
0036b908: mov    rsi, rax
0036b90b: call   0x180390700                              ; ʼʳʾʺʻˀʻʸʵʾʶ$$.ctor
0036b910: lea    rcx, [r13 + 0x70]
0036b914: mov    qword ptr [r13 + 0x70], rsi
0036b918: mov    rdx, rsi
0036b91b: call   0x182f5fc00
0036b920: movzx  ebp, byte ptr [rsp + 0x30]
0036b925: cmp    byte ptr [r13 + 0x68], 0
0036b92a: je     0x18036b94f
0036b92c: mov    rax, qword ptr [rip + 0x392c9ed]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
0036b933: mov    rcx, qword ptr [rax + 0xb8]
0036b93a: mov    rdx, qword ptr [rcx + 0x10]
0036b93e: test   rdx, rdx
0036b941: je     0x18036c2b1
0036b947: mov    edx, dword ptr [rdx + 0x10]
0036b94a: sub    edx, 0x64
0036b94d: jmp    0x18036b954
0036b94f: mov    edx, 0xffffffff
0036b954: xor    r8d, r8d
0036b957: mov    rcx, r13
0036b95a: call   0x180368590                              ; BassAudioManager$$ʸʾʼʶʸʺʿʲʶʴʺ
0036b95f: mov    rcx, qword ptr [rip + 0x398496a]         ; [0x3cf02d0] meta:System.Collections.Generic.List<string>_TypeInfo
0036b966: call   0x182f60c00
0036b96b: mov    rdx, qword ptr [rip + 0x394c4f6]         ; [0x3cb7e68] metamethod:Method$System.Collections.Generic.List<string>..ctor()
0036b972: mov    rcx, rax
0036b975: mov    rsi, rax
0036b978: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
0036b97d: cmp    byte ptr [r13 + 0x5c], 0
0036b982: jne    0x18036ba6f
0036b988: test   bpl, bpl
0036b98b: jne    0x18036ba6f
0036b991: mov    rcx, qword ptr [r14 + 0x30]
0036b995: xor    edx, edx
0036b997: call   0x181b12530                              ; System.IO.Directory$$GetFiles
0036b99c: mov    r14, rax
0036b99f: test   rax, rax
0036b9a2: je     0x18036c2b1
0036b9a8: xor    ebp, ebp
0036b9aa: xor    eax, eax
0036b9ac: nop    dword ptr [rax]
0036b9b0: cmp    eax, dword ptr [r14 + 0x18]
0036b9b4: jge    0x18036ba67
0036b9ba: cmp    ebp, dword ptr [r14 + 0x18]
0036b9be: jae    0x18036c2f7
0036b9c4: mov    rcx, qword ptr [rip + 0x394b785]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
0036b9cb: movsxd rax, ebp
0036b9ce: cmp    dword ptr [rcx + 0xe0], 0
0036b9d5: mov    r15, qword ptr [r14 + rax*8 + 0x20]
0036b9da: jne    0x18036b9e1
0036b9dc: call   0x182f60cf0
0036b9e1: xor    edx, edx
0036b9e3: mov    rcx, r15
0036b9e6: call   0x181b201f0                              ; System.IO.Path$$GetFileName
0036b9eb: mov    r9, rax
0036b9ee: test   rsi, rsi
0036b9f1: je     0x18036c2b1
0036b9f7: mov    rax, qword ptr [rip + 0x394c682]         ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
0036b9fe: inc    dword ptr [rsi + 0x1c]
0036ba01: mov    rcx, qword ptr [rsi + 0x10]
0036ba05: test   rcx, rcx
0036ba08: je     0x18036c2b1
0036ba0e: movsxd rdx, dword ptr [rsi + 0x18]
0036ba12: cmp    edx, dword ptr [rcx + 0x18]
0036ba15: jb     0x18036ba3a
0036ba17: mov    rax, qword ptr [rax + 0x20]
0036ba1b: mov    rdx, r9
0036ba1e: mov    rcx, rsi
0036ba21: mov    r8, qword ptr [rax + 0xc0]
0036ba28: mov    r8, qword ptr [r8 + 0x70]
0036ba2c: call   0x18076fd70                              ; System.Collections.Generic.List<object>$$AddWithResize
0036ba31: inc    ebp
0036ba33: mov    eax, ebp
0036ba35: jmp    0x18036b9b0
0036ba3a: lea    eax, [rdx + 1]
0036ba3d: mov    dword ptr [rsi + 0x18], eax
0036ba40: cmp    edx, dword ptr [rcx + 0x18]
0036ba43: jae    0x18036c2f7
0036ba49: mov    qword ptr [rcx + rdx*8 + 0x20], r9
0036ba4e: lea    rcx, [rcx + rdx*8]
0036ba52: add    rcx, 0x20
0036ba56: mov    rdx, r9
0036ba59: call   0x182f5fc00
0036ba5e: inc    ebp
0036ba60: mov    eax, ebp
0036ba62: jmp    0x18036b9b0
0036ba67: mov    r14, qword ptr [rsp + 0x108]
0036ba6f: mov    rbp, qword ptr [rip + 0x39858ba]         ; [0x3cf1330] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<string>()
0036ba76: cmp    qword ptr [rbp + 0x38], 0
0036ba7b: jne    0x18036ba85
0036ba7d: mov    rcx, rbp
0036ba80: call   0x182f657d0
0036ba85: mov    r8, qword ptr [rbp + 0x38]
0036ba89: lea    rcx, [rsp + 0x40]
0036ba8e: mov    rdx, rsi
0036ba91: mov    r8, qword ptr [r8 + 8]
0036ba95: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
0036ba9a: mov    rax, qword ptr [r13 + 0x20]
0036ba9e: test   rax, rax
0036baa1: je     0x18036c2b1
0036baa7: mov    esi, dword ptr [rsp + 0x48]
0036baab: xor    r15d, r15d
0036baae: mov    rcx, qword ptr [rip + 0x39503a3]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bab5: xor    r12d, r12d
0036bab8: mov    dword ptr [rsp + 0x34], r15d
0036babd: xor    edx, edx
0036babf: mov    dword ptr [rsp + 0x38], r15d
0036bac4: cmp    edx, dword ptr [rax + 0x18]
0036bac7: jge    0x18036c1d7
0036bacd: cmp    r12d, ebx
0036bad0: jl     0x18036badd
0036bad2: cmp    r12d, dword ptr [rsp + 0x5c]
0036bad7: jle    0x18036bf55
0036badd: cmp    r12d, edi
0036bae0: jl     0x18036baed
0036bae2: cmp    r12d, dword ptr [rsp + 0x64]
0036bae7: jle    0x18036bf55
0036baed: cmp    byte ptr [r13 + 0x5c], 0
0036baf2: jne    0x18036c0b6
0036baf8: cmp    byte ptr [rsp + 0x30], 0
0036bafd: jne    0x18036bf75
0036bb03: xor    r15d, r15d
0036bb06: mov    qword ptr [rsp + 0x50], 0
0036bb0f: test   esi, esi
0036bb11: jle    0x18036bf50
0036bb17: nop    word ptr [rax + rax]
0036bb20: xor    r14d, r14d
0036bb23: cmp    dword ptr [rcx + 0xe0], 0
0036bb2a: jne    0x18036bb38
0036bb2c: call   0x182f60cf0
0036bb31: mov    rcx, qword ptr [rip + 0x3950320]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bb38: mov    rax, qword ptr [rcx + 0xb8]
0036bb3f: mov    rdx, qword ptr [rax + 0x18]
0036bb43: test   rdx, rdx
0036bb46: je     0x18036c2b1
0036bb4c: cmp    r14d, dword ptr [rdx + 0x18]
0036bb50: jge    0x18036bcb4
0036bb56: cmp    dword ptr [rcx + 0xe0], 0
0036bb5d: jne    0x18036bb6b
0036bb5f: call   0x182f60cf0
0036bb64: mov    rcx, qword ptr [rip + 0x39502ed]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bb6b: mov    rax, qword ptr [rcx + 0xb8]
0036bb72: mov    rdx, qword ptr [rax + 0x18]
0036bb76: test   rdx, rdx
0036bb79: je     0x18036c2b1
0036bb7f: mov    rcx, qword ptr [rax + 0x28]
0036bb83: test   rcx, rcx
0036bb86: je     0x18036c2b1
0036bb8c: mov    eax, r12d
0036bb8f: imul   eax, dword ptr [rdx + 0x18]
0036bb93: lea    edx, [rax + r14]
0036bb97: cmp    edx, dword ptr [rcx + 0x18]
0036bb9a: jae    0x18036c2f7
0036bba0: cmp    r15d, esi
0036bba3: jae    0x18036c2f7
0036bba9: cmp    byte ptr [rip + 0x3ba0419], 0            ; [0x3f0bfc9] (bss)
0036bbb0: movsxd rax, edx
0036bbb3: mov    rbp, qword ptr [rcx + rax*8 + 0x20]
0036bbb8: mov    rcx, qword ptr [rsp + 0x40]
0036bbbd: mov    rsi, qword ptr [rcx + r15*8]
0036bbc1: jne    0x18036bbd6
0036bbc3: lea    rcx, [rip + 0x39359de]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0036bbca: call   0x182f609b0
0036bbcf: mov    byte ptr [rip + 0x3ba03f3], 1            ; [0x3f0bfc9] (bss)
0036bbd6: xorps  xmm7, xmm7
0036bbd9: test   rsi, rsi
0036bbdc: je     0x18036bc01
0036bbde: xor    edx, edx
0036bbe0: mov    dword ptr [rsp + 0x7c], 0
0036bbe8: mov    rcx, rsi
0036bbeb: call   0x1819829d0                              ; System.String$$GetRawStringData
0036bbf0: mov    qword ptr [rsp + 0x70], rax
0036bbf5: mov    eax, dword ptr [rsi + 0x10]
0036bbf8: mov    dword ptr [rsp + 0x78], eax
0036bbfc: movaps xmm7, xmmword ptr [rsp + 0x70]
0036bc01: cmp    byte ptr [rip + 0x3ba0553], 0            ; [0x3f0c15b] (bss)
0036bc08: jne    0x18036bc1d
0036bc0a: lea    rcx, [rip + 0x3935997]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
0036bc11: call   0x182f609b0
0036bc16: mov    byte ptr [rip + 0x3ba053e], 1            ; [0x3f0c15b] (bss)
0036bc1d: xorps  xmm6, xmm6
0036bc20: test   rbp, rbp
0036bc23: je     0x18036bc54
0036bc25: xor    edx, edx
0036bc27: mov    dword ptr [rsp + 0x8c], 0
0036bc32: mov    rcx, rbp
0036bc35: call   0x1819829d0                              ; System.String$$GetRawStringData
0036bc3a: mov    qword ptr [rsp + 0x80], rax
0036bc42: mov    eax, dword ptr [rbp + 0x10]
0036bc45: mov    dword ptr [rsp + 0x88], eax
0036bc4c: movaps xmm6, xmmword ptr [rsp + 0x80]
0036bc54: mov    rcx, qword ptr [rip + 0x392cbed]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
0036bc5b: cmp    dword ptr [rcx + 0xe0], 0
0036bc62: jne    0x18036bc69
0036bc64: call   0x182f60cf0
0036bc69: xor    r9d, r9d
0036bc6c: movdqa xmmword ptr [rsp + 0x90], xmm6
0036bc75: mov    r8b, 1
0036bc78: movdqa xmmword ptr [rsp + 0xa0], xmm7
0036bc81: lea    rdx, [rsp + 0x90]
0036bc89: lea    rcx, [rsp + 0xa0]
0036bc91: call   0x1820fe510                              ; ʹʾʽʼʷʲʴʸʷʼˁ$$ʼʽʸʵʻʺʼʶʶʲʹ
0036bc96: mov    rcx, qword ptr [rip + 0x39501bb]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bc9d: mov    esi, dword ptr [rsp + 0x48]
0036bca1: test   al, al
0036bca3: jne    0x18036bcad
0036bca5: inc    r14d
0036bca8: jmp    0x18036bb23
0036bcad: mov    qword ptr [rsp + 0x50], rbp
0036bcb2: jmp    0x18036bcb9
0036bcb4: mov    rbp, qword ptr [rsp + 0x50]
0036bcb9: inc    r15d
0036bcbc: cmp    r15d, esi
0036bcbf: jl     0x18036bb20
0036bcc5: test   rbp, rbp
0036bcc8: je     0x18036bf50
0036bcce: mov    rcx, qword ptr [rip + 0x394b47b]         ; [0x3cb7150] meta:System.IO.Path_TypeInfo
0036bcd5: mov    rsi, qword ptr [rsp + 0x108]
0036bcdd: cmp    dword ptr [rcx + 0xe0], 0
0036bce4: mov    rsi, qword ptr [rsi + 0x30]
0036bce8: jne    0x18036bcef
0036bcea: call   0x182f60cf0
0036bcef: xor    r8d, r8d
0036bcf2: mov    rdx, rbp
0036bcf5: mov    rcx, rsi
0036bcf8: call   0x181b1eee0                              ; System.IO.Path$$Combine
0036bcfd: mov    rcx, qword ptr [rip + 0x3932524]         ; [0x3c9e228] meta:ʿʷʼˁʺʻʴʿʴʺʾ_TypeInfo
0036bd04: mov    rsi, rax
0036bd07: call   0x182f60c00
0036bd0c: xor    r8d, r8d
0036bd0f: mov    rdx, rsi
0036bd12: mov    rcx, rax
0036bd15: mov    r14, rax
0036bd18: call   0x180102e10                              ; ʿʷʼˁʺʻʴʿʴʺʾ$$.ctor
0036bd1d: test   r14, r14
0036bd20: je     0x18036bf45
0036bd26: mov    r8, qword ptr [r14]
0036bd29: mov    rcx, r14
0036bd2c: mov    rdx, qword ptr [r8 + 0x1e0]
0036bd33: call   qword ptr [r8 + 0x1d8]
0036bd3a: xor    edx, edx
0036bd3c: mov    rcx, r14
0036bd3f: call   0x1801ed8a0                              ; ʲʵʸʸʻʹʶʾʻʴʼ$$ʽʽʷʹʲʺʿʺʷʲʽ
0036bd44: test   al, al
0036bd46: je     0x18036bf32
0036bd4c: mov    rsi, qword ptr [r13 + 0x20]
0036bd50: test   rsi, rsi
0036bd53: je     0x18036c2b1
0036bd59: mov    rdx, qword ptr [rsi]
0036bd5c: mov    rcx, r14
0036bd5f: mov    rdx, qword ptr [rdx + 0x40]
0036bd63: call   0x182f5fc20
0036bd68: test   rax, rax
0036bd6b: je     0x18036c2b7
0036bd71: cmp    r12d, dword ptr [rsi + 0x18]
0036bd75: jae    0x18036c2f7
0036bd7b: movsxd rax, r12d
0036bd7e: mov    rdx, r14
0036bd81: mov    qword ptr [rsi + rax*8 + 0x20], r14
0036bd86: movsxd rax, r12d
0036bd89: add    rax, 4
0036bd8d: lea    rcx, [rsi + rax*8]
0036bd91: call   0x182f5fc00
0036bd96: mov    r15d, dword ptr [rsp + 0x34]
0036bd9b: mov    rcx, qword ptr [r13 + 0x20]
0036bd9f: test   rcx, rcx
0036bda2: je     0x18036c2b1
0036bda8: cmp    r12d, dword ptr [rcx + 0x18]
0036bdac: jae    0x18036c2f7
0036bdb2: movsxd rax, r12d
0036bdb5: cmp    qword ptr [rcx + rax*8 + 0x20], 0
0036bdbb: je     0x18036bf45
0036bdc1: mov    rax, qword ptr [rip + 0x3950090]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bdc8: cmp    dword ptr [rax + 0xe0], 0
0036bdcf: jne    0x18036bde0
0036bdd1: mov    rcx, rax
0036bdd4: call   0x182f60cf0
0036bdd9: mov    rax, qword ptr [rip + 0x3950078]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bde0: mov    rax, qword ptr [rax + 0xb8]
0036bde7: mov    rcx, qword ptr [rax + 0x10]
0036bdeb: test   rcx, rcx
0036bdee: je     0x18036c2b1
0036bdf4: mov    ecx, dword ptr [rcx + 0x18]
0036bdf7: lea    eax, [r15 + 1]
0036bdfb: dec    ecx
0036bdfd: cmp    r12d, ecx
0036be00: mov    rcx, qword ptr [r13 + 0x20]
0036be04: cmovge eax, r15d
0036be08: mov    dword ptr [rsp + 0x34], eax
0036be0c: mov    r15d, eax
0036be0f: test   rcx, rcx
0036be12: je     0x18036c2b1
0036be18: cmp    r12d, dword ptr [rcx + 0x18]
0036be1c: jae    0x18036c2f7
0036be22: movsxd rax, r12d
0036be25: mov    r8, qword ptr [rcx + rax*8 + 0x20]
0036be2a: test   r8, r8
0036be2d: je     0x18036c2b1
0036be33: mov    rdx, qword ptr [rip + 0x39260ee]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036be3a: xor    ecx, ecx
0036be3c: call   0x182c44210
0036be41: mov    rcx, qword ptr [rip + 0x3950010]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036be48: mov    esi, eax
0036be4a: cmp    dword ptr [rcx + 0xe0], 0
0036be51: jne    0x18036be58
0036be53: call   0x182f60cf0
0036be58: xor    edx, edx
0036be5a: mov    ecx, esi
0036be5c: call   0x18036e100                              ; BassAudioManager$$ˁʷˁʲʷʷʿʶʴʼʼ
0036be61: comisd xmm0, xmmword ptr [r13 + 0x40]
0036be67: jbe    0x18036bea9
0036be69: mov    rcx, qword ptr [r13 + 0x20]
0036be6d: movsd  qword ptr [r13 + 0x40], xmm0
0036be73: test   rcx, rcx
0036be76: je     0x18036c2b1
0036be7c: cmp    r12d, dword ptr [rcx + 0x18]
0036be80: jae    0x18036c2f7
0036be86: movsxd rax, r12d
0036be89: mov    r8, qword ptr [rcx + rax*8 + 0x20]
0036be8e: test   r8, r8
0036be91: je     0x18036c2b1
0036be97: mov    rdx, qword ptr [rip + 0x392608a]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036be9e: xor    ecx, ecx
0036bea0: call   0x182c44210
0036bea5: mov    dword ptr [rsp + 0x38], eax
0036bea9: xor    edx, edx
0036beab: mov    rcx, r13
0036beae: call   0x180367ed0                              ; BassAudioManager$$ʶˀʷʵʽʺˁʶʺʾʺ
0036beb3: mov    rdx, qword ptr [r13 + 0x20]
0036beb7: mov    esi, eax
0036beb9: test   rdx, rdx
0036bebc: je     0x18036c2b1
0036bec2: cmp    r12d, dword ptr [rdx + 0x18]
0036bec6: jae    0x18036c2f7
0036becc: movsxd rcx, r12d
0036becf: mov    r8, qword ptr [rdx + rcx*8 + 0x20]
0036bed4: test   r8, r8
0036bed7: je     0x18036c2b1
0036bedd: mov    rdx, qword ptr [rip + 0x3926044]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036bee4: xor    ecx, ecx
0036bee6: call   0x182c44210
0036beeb: xor    r9d, r9d
0036beee: xor    r8d, r8d
0036bef1: mov    edx, eax
0036bef3: mov    ecx, esi
0036bef5: call   0x18034c7b0                              ; ʽʺʸʳʷʲˁʳʷʵʳ$$ʵʶʳʸʳʷʸʶʴʻʾ
0036befa: mov    rcx, qword ptr [rip + 0x394ff57]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bf01: movzx  esi, al
0036bf04: cmp    dword ptr [rcx + 0xe0], 0
0036bf0b: jne    0x18036bf12
0036bf0d: call   0x182f60cf0
0036bf12: mov    rdx, qword ptr [rip + 0x395462f]         ; [0x3cc0548] str:'Failed to add channel to mixer'
0036bf19: xor    r8d, r8d
0036bf1c: movzx  ecx, sil
0036bf20: call   0x18036d890                              ; BassAudioManager$$ˀʷʹʴʴʸʿʻʷʳʷ
0036bf25: mov    rcx, qword ptr [rip + 0x394ff2c]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bf2c: mov    esi, dword ptr [rsp + 0x48]
0036bf30: jmp    0x18036bf55
0036bf32: mov    rax, qword ptr [r14]
0036bf35: mov    rcx, r14
0036bf38: mov    rdx, qword ptr [rax + 0x1f0]
0036bf3f: call   qword ptr [rax + 0x1e8]
0036bf45: mov    esi, dword ptr [rsp + 0x48]
0036bf49: mov    rcx, qword ptr [rip + 0x394ff08]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bf50: mov    r15d, dword ptr [rsp + 0x34]
0036bf55: mov    rax, qword ptr [r13 + 0x20]
0036bf59: inc    r12d
0036bf5c: mov    edx, r12d
0036bf5f: test   rax, rax
0036bf62: je     0x18036c2b1
0036bf68: mov    r14, qword ptr [rsp + 0x108]
0036bf70: jmp    0x18036bac4
0036bf75: xor    edx, edx
0036bf77: mov    rcx, r14
0036bf7a: call   0x180360a20                              ; SongEntry$$get_Sng
0036bf7f: mov    r8, qword ptr [rip + 0x394fed2]          ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bf86: mov    rsi, rax
0036bf89: cmp    dword ptr [r8 + 0xe0], 0
0036bf91: jne    0x18036bfa2
0036bf93: mov    rcx, r8
0036bf96: call   0x182f60cf0
0036bf9b: mov    r8, qword ptr [rip + 0x394feb6]          ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036bfa2: mov    rcx, qword ptr [r8 + 0xb8]
0036bfa9: mov    r10, qword ptr [rcx + 0x10]
0036bfad: test   r10, r10
0036bfb0: je     0x18036c2b1
0036bfb6: cmp    r12d, dword ptr [r10 + 0x18]
0036bfba: jae    0x18036c2f7
0036bfc0: test   rsi, rsi
0036bfc3: je     0x18036c2b1
0036bfc9: mov    r8, qword ptr [rcx + 0x18]
0036bfcd: xor    r9d, r9d
0036bfd0: movsxd rdx, r12d
0036bfd3: mov    rcx, rsi
0036bfd6: mov    rdx, qword ptr [r10 + rdx*8 + 0x20]
0036bfdb: call   0x180264ea0                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʻʵʾʴʽʻʲʶʳʲˀ
0036bfe0: mov    rbp, rax
0036bfe3: test   rax, rax
0036bfe6: je     0x18036bf45
0036bfec: xor    edx, edx
0036bfee: mov    rcx, r14
0036bff1: call   0x180360a20                              ; SongEntry$$get_Sng
0036bff6: mov    rcx, qword ptr [rip + 0x3926beb]         ; [0x3c92be8] meta:ʴʾʴʹʳʼʼʲʸʾʸ_TypeInfo
0036bffd: mov    rsi, rax
0036c000: call   0x182f60c00
0036c005: xor    r9d, r9d
0036c008: mov    r8, rbp
0036c00b: mov    rdx, rsi
0036c00e: mov    rcx, rax
0036c011: mov    r14, rax
0036c014: call   0x1802e1530                              ; ʴʾʴʹʳʼʼʲʸʾʸ$$.ctor
0036c019: test   r14, r14
0036c01c: je     0x18036c2b1
0036c022: mov    r8, qword ptr [r14]
0036c025: mov    rcx, r14
0036c028: mov    rdx, qword ptr [r8 + 0x1e0]
0036c02f: call   qword ptr [r8 + 0x1d8]
0036c036: xor    edx, edx
0036c038: mov    rcx, r14
0036c03b: call   0x1801ed8a0                              ; ʲʵʸʸʻʹʶʾʻʴʼ$$ʽʽʷʹʲʺʿʺʷʲʽ
0036c040: test   al, al
0036c042: je     0x18036c093
0036c044: mov    rsi, qword ptr [r13 + 0x20]
0036c048: test   rsi, rsi
0036c04b: je     0x18036c2b1
0036c051: mov    rdx, qword ptr [rsi]
0036c054: mov    rcx, r14
0036c057: mov    rdx, qword ptr [rdx + 0x40]
0036c05b: call   0x182f5fc20
0036c060: test   rax, rax
0036c063: je     0x18036c2c7
0036c069: cmp    r12d, dword ptr [rsi + 0x18]
0036c06d: jae    0x18036c2f7
0036c073: movsxd rax, r12d
0036c076: mov    rdx, r14
0036c079: mov    qword ptr [rsi + rax*8 + 0x20], r14
0036c07e: movsxd rax, r12d
0036c081: add    rax, 4
0036c085: lea    rcx, [rsi + rax*8]
0036c089: call   0x182f5fc00
0036c08e: jmp    0x18036bd9b
0036c093: mov    rax, qword ptr [r14]
0036c096: mov    rcx, r14
0036c099: mov    rdx, qword ptr [rax + 0x1f0]
0036c0a0: call   qword ptr [rax + 0x1e8]
0036c0a6: mov    rcx, qword ptr [rip + 0x394fdab]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
0036c0ad: mov    esi, dword ptr [rsp + 0x48]
0036c0b1: jmp    0x18036bf55
0036c0b6: xor    edx, edx
0036c0b8: mov    rcx, r14
0036c0bb: call   0x180360460                              ; SongEntry$$get_EncSong
0036c0c0: test   rax, rax
0036c0c3: je     0x18036c2b1
0036c0c9: mov    r15, qword ptr [rax + 0x38]
0036c0cd: nop    dword ptr [rax]
0036c0d0: xor    ebp, ebp
0036c0d2: test   r15, r15
0036c0d5: je     0x18036c2b1
0036c0db: nop    dword ptr [rax + rax]
0036c0e0: cmp    ebp, dword ptr [r15 + 0x18]
0036c0e4: jge    0x18036bd96
0036c0ea: jae    0x18036c2f7
0036c0f0: movsxd rax, ebp
0036c0f3: mov    rsi, qword ptr [r15 + rax*8 + 0x20]
0036c0f8: test   rsi, rsi
0036c0fb: je     0x18036c190
0036c101: mov    rdx, qword ptr [rip + 0x3925e20]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036c108: mov    ecx, 1
0036c10d: mov    r8, rsi
0036c110: call   0x182c44210
0036c115: cmp    eax, r12d
0036c118: jne    0x18036c190
0036c11a: mov    rdx, qword ptr [rip + 0x3925e07]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036c121: mov    ecx, 2
0036c126: mov    r8, rsi
0036c129: call   0x182c42550
0036c12e: mov    rdx, qword ptr [rip + 0x3925df3]         ; [0x3c91f28] meta:ʳˁʿʶʵʺʻʺʿʽʴ_TypeInfo
0036c135: mov    ecx, 4
0036c13a: mov    r8, rsi
0036c13d: call   0x182c44210
0036c142: test   al, al
0036c144: je     0x18036c197
0036c146: mov    r14, qword ptr [r13 + 0x20]
0036c14a: test   r14, r14
0036c14d: je     0x18036c2b1
0036c153: mov    rdx, qword ptr [r14]
0036c156: mov    rcx, rsi
0036c159: mov    rdx, qword ptr [rdx + 0x40]
0036c15d: call   0x182f5fc20
0036c162: test   rax, rax
0036c165: je     0x18036c2d7
0036c16b: cmp    r12d, dword ptr [r14 + 0x18]
0036c16f: jae    0x18036c2f7
0036c175: movsxd rax, r12d
0036c178: mov    rdx, rsi
0036c17b: mov    qword ptr [r14 + rax*8 + 0x20], rsi
0036c180: movsxd rax, r12d
0036c183: add    rax, 4
0036c187: lea    rcx, [r14 + rax*8]
0036c18b: call   0x182f5fc00
0036c190: inc    ebp
0036c192: jmp    0x18036c0e0
0036c197: xor    al, al
0036c199: movaps xmm7, xmmword ptr [rsp + 0xb0]
0036c1a1: movaps xmm6, xmmword ptr [rsp + 0xc0]
0036c1a9: mov    r15, qword ptr [rsp + 0xd0]
0036c1b1: mov    r12, qword ptr [rsp + 0x118]
0036c1b9: mov    rsi, qword ptr [rsp + 0x110]
0036c1c1: mov    rbp, qword ptr [rsp + 0x100]
0036c1c9: add    rsp, 0xd8
0036c1d0: pop    r14
0036c1d2: pop    r13
0036c1d4: pop    rdi
0036c1d5: pop    rbx
0036c1d6: ret    
0036c1d7: mov    ebp, dword ptr [rsp + 0x38]
0036c1db: test   ebp, ebp
0036c1dd: je     0x18036c205
0036c1df: mov    rcx, qword ptr [r13 + 0x70]
0036c1e3: test   rcx, rcx
0036c1e6: je     0x18036c2b1
0036c1ec: xor    r9d, r9d
0036c1ef: mov    qword ptr [rsp + 0x20], 0
0036c1f8: mov    r8d, 2
0036c1fe: mov    edx, ebp
0036c200: call   0x18038fe40                              ; ʼʳʾʺʻˀʻʸʵʾʶ$$ʷʸʾʸʵʹʾʼʾʵʿ
0036c205: mov    rax, qword ptr [r13 + 0x20]
0036c209: test   rax, rax
0036c20c: je     0x18036c2b1
0036c212: cmp    dword ptr [rax + 0x18], 1
0036c216: jbe    0x18036c2f7
0036c21c: cmp    qword ptr [rax + 0x28], 0
0036c221: jne    0x18036c292
0036c223: cmp    dword ptr [rax + 0x18], 2
0036c227: jbe    0x18036c2f7
0036c22d: mov    rbx, rax
0036c230: test   rax, rax
0036c233: je     0x18036c2b1
0036c235: mov    rdi, qword ptr [rax + 0x30]
0036c239: test   rdi, rdi
0036c23c: je     0x18036c256
0036c23e: mov    rdx, qword ptr [rax]
0036c241: mov    rcx, rdi
0036c244: mov    rdx, qword ptr [rdx + 0x40]
0036c248: call   0x182f5fc20
0036c24d: test   rax, rax
0036c250: je     0x18036c2e7
0036c256: cmp    dword ptr [rbx + 0x18], 1
0036c25a: jbe    0x18036c2f7
0036c260: lea    rcx, [rbx + 0x28]
0036c264: mov    qword ptr [rbx + 0x28], rdi
0036c268: mov    rdx, rdi
0036c26b: call   0x182f5fc00
0036c270: mov    rcx, qword ptr [r13 + 0x20]
0036c274: test   rcx, rcx
0036c277: je     0x18036c2b1
0036c279: cmp    dword ptr [rcx + 0x18], 2
0036c27d: jbe    0x18036c2f7
0036c27f: mov    qword ptr [rcx + 0x30], 0
0036c287: xor    edx, edx
0036c289: add    rcx, 0x30
0036c28d: call   0x182f5fc00
0036c292: cmp    r15d, 1
0036c296: jle    0x18036c29d
0036c298: mov    byte ptr [r13 + 0x3b], 1
0036c29d: mov    rax, qword ptr [r13 + 0x40]
0036c2a1: mov    qword ptr [r13 + 0x48], rax
0036c2a5: mov    al, 1
0036c2a7: mov    byte ptr [r13 + 0x28], 1
0036c2ac: jmp    0x18036c199
0036c2b1: call   0x182f60c50
0036c2b6: int3   
0036c2b7: call   0x182f604f0
0036c2bc: mov    rcx, rax
0036c2bf: xor    edx, edx
0036c2c1: call   0x182f60c10
0036c2c6: int3   
0036c2c7: call   0x182f604f0
0036c2cc: mov    rcx, rax
0036c2cf: xor    edx, edx
0036c2d1: call   0x182f60c10
0036c2d6: int3   
0036c2d7: call   0x182f604f0
0036c2dc: mov    rcx, rax
0036c2df: xor    edx, edx
0036c2e1: call   0x182f60c10
0036c2e6: int3   
0036c2e7: call   0x182f604f0
0036c2ec: mov    rcx, rax
0036c2ef: xor    edx, edx
0036c2f1: call   0x182f60c10
0036c2f6: int3   
0036c2f7: call   0x182f60c40
0036c2fc: int3   
0036c2fd: int3   
