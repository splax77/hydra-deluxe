01b4f710: mov    qword ptr [rsp + 0x10], rsi
01b4f715: push   rdi
01b4f716: sub    rsp, 0xd0
01b4f71d: cmp    byte ptr [rip + 0x23c53e0], 0            ; [0x3f14b04] (bss)
01b4f724: mov    rsi, rdx
01b4f727: mov    rdi, rcx
01b4f72a: jne    0x181b4f73f
01b4f72c: lea    rcx, [rip + 0x2162a6d]                   ; [0x3cb21a0] meta:System.Number_TypeInfo
01b4f733: call   0x182f609b0
01b4f738: mov    byte ptr [rip + 0x23c53c5], 1            ; [0x3f14b04] (bss)
01b4f73f: test   rdi, rdi
01b4f742: je     0x181b4f868
01b4f748: movaps xmmword ptr [rsp + 0xc0], xmm6
01b4f750: cmp    byte ptr [rip + 0x23bca04], 0            ; [0x3f0c15b] (bss)
01b4f757: jne    0x181b4f76c
01b4f759: lea    rcx, [rip + 0x2151e48]                   ; [0x3ca15a8] metamethod:Method$System.ReadOnlySpan<char>..ctor()
01b4f760: call   0x182f609b0
01b4f765: mov    byte ptr [rip + 0x23bc9ef], 1            ; [0x3f0c15b] (bss)
01b4f76c: xor    edx, edx
01b4f76e: mov    dword ptr [rsp + 0x3c], 0
01b4f776: mov    rcx, rdi
01b4f779: call   0x1819829d0                              ; System.String$$GetRawStringData
01b4f77e: mov    qword ptr [rsp + 0x30], rax
01b4f783: xor    ecx, ecx
01b4f785: mov    eax, dword ptr [rdi + 0x10]
01b4f788: mov    dword ptr [rsp + 0x38], eax
01b4f78c: movaps xmm6, xmmword ptr [rsp + 0x30]
01b4f791: call   0x181abecb0                              ; System.Globalization.NumberFormatInfo$$get_CurrentInfo
01b4f796: mov    rcx, qword ptr [rip + 0x2162a03]         ; [0x3cb21a0] meta:System.Number_TypeInfo
01b4f79d: mov    rdi, rax
01b4f7a0: cmp    dword ptr [rcx + 0xe0], 0
01b4f7a7: jne    0x181b4f7ae
01b4f7a9: call   0x182f60cf0
01b4f7ae: cmp    byte ptr [rip + 0x23c53e4], 0            ; [0x3f14b99] (bss)
01b4f7b5: jne    0x181b4f7ca
01b4f7b7: lea    rcx, [rip + 0x21629e2]                   ; [0x3cb21a0] meta:System.Number_TypeInfo
01b4f7be: call   0x182f609b0
01b4f7c3: mov    byte ptr [rip + 0x23c53cf], 1            ; [0x3f14b99] (bss)
01b4f7ca: mov    rcx, qword ptr [rip + 0x21629cf]         ; [0x3cb21a0] meta:System.Number_TypeInfo
01b4f7d1: xorps  xmm0, xmm0
01b4f7d4: xor    eax, eax
01b4f7d6: movups xmmword ptr [rsp + 0xa0], xmm0
01b4f7de: mov    byte ptr [rsp + 0xe0], al
01b4f7e5: movups xmmword ptr [rsp + 0x40], xmm0
01b4f7ea: movups xmmword ptr [rsp + 0x50], xmm0
01b4f7ef: movups xmmword ptr [rsp + 0x60], xmm0
01b4f7f4: movups xmmword ptr [rsp + 0x70], xmm0
01b4f7f9: movups xmmword ptr [rsp + 0x80], xmm0
01b4f801: movups xmmword ptr [rsp + 0x90], xmm0
01b4f809: movups xmmword ptr [rsp + 0xaa], xmm0
01b4f811: cmp    dword ptr [rcx + 0xe0], eax
01b4f817: jne    0x181b4f81e
01b4f819: call   0x182f60cf0
01b4f81e: lea    rax, [rsp + 0xe0]
01b4f826: mov    qword ptr [rsp + 0x28], 0
01b4f82f: mov    r9, rsi
01b4f832: mov    qword ptr [rsp + 0x20], rax
01b4f837: mov    r8, rdi
01b4f83a: movdqa xmmword ptr [rsp + 0x30], xmm6
01b4f840: mov    edx, 7
01b4f845: lea    rcx, [rsp + 0x30]
01b4f84a: call   0x181b5dc50                              ; System.Number$$TryParseInt32IntegerStyle
01b4f84f: movaps xmm6, xmmword ptr [rsp + 0xc0]
01b4f857: mov    rsi, qword ptr [rsp + 0xe8]
01b4f85f: add    rsp, 0xd0
01b4f866: pop    rdi
01b4f867: ret    
01b4f868: mov    dword ptr [rsi], 0
01b4f86e: xor    al, al
01b4f870: mov    rsi, qword ptr [rsp + 0xe8]
01b4f878: add    rsp, 0xd0
01b4f87f: pop    rdi
01b4f880: ret    
01b4f881: int3   
