0212e130: mov    qword ptr [rsp + 0x18], rbx
0212e135: mov    byte ptr [rsp + 0x10], dl
0212e139: mov    qword ptr [rsp + 8], rcx
0212e13e: push   rsi
0212e13f: push   rdi
0212e140: push   r14
0212e142: sub    rsp, 0x60
0212e146: movzx  r14d, dl
0212e14a: mov    rsi, rcx
0212e14d: cmp    byte ptr [rip + 0x1deb43e], 0            ; [0x3f19592] (bss)
0212e154: jne    0x18212e1b1
0212e156: lea    rcx, [rip + 0x1b92413]                   ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
0212e15d: call   0x182f609b0
0212e162: lea    rcx, [rip + 0x1b924c7]                   ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
0212e169: call   0x182f609b0
0212e16e: lea    rcx, [rip + 0x1b92573]                   ; [0x3cc06e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Current()
0212e175: call   0x182f609b0
0212e17a: lea    rcx, [rip + 0x1ba313f]                   ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
0212e181: call   0x182f609b0
0212e186: lea    rcx, [rip + 0x1bd4d33]                   ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
0212e18d: call   0x182f609b0
0212e192: lea    rcx, [rip + 0x1bd5017]                   ; [0x3d031b0] metamethod:Method$System.Nullable<long>.get_HasValue()
0212e199: call   0x182f609b0
0212e19e: lea    rcx, [rip + 0x1bd50cb]                   ; [0x3d03270] metamethod:Method$System.Nullable<long>.get_Value()
0212e1a5: call   0x182f609b0
0212e1aa: mov    byte ptr [rip + 0x1deb3e1], 1            ; [0x3f19592] (bss)
0212e1b1: xorps  xmm0, xmm0
0212e1b4: xor    eax, eax
0212e1b6: movups xmmword ptr [rsp + 0x40], xmm0
0212e1bb: mov    qword ptr [rsp + 0x50], rax
0212e1c0: cmp    byte ptr [rsi + 0x120], al
0212e1c6: jne    0x18212e2c5
0212e1cc: mov    rdx, qword ptr [rsi + 0xd0]
0212e1d3: test   rdx, rdx
0212e1d6: je     0x18212e2fa
0212e1dc: xor    edi, edi
0212e1de: mov    qword ptr [rsp + 0x98], rdi
0212e1e6: mov    r8, qword ptr [rip + 0x1ba30d3]          ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
0212e1ed: lea    rcx, [rsp + 0x28]
0212e1f2: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
0212e1f7: movups xmm0, xmmword ptr [rsp + 0x28]
0212e1fc: movups xmmword ptr [rsp + 0x40], xmm0
0212e201: movsd  xmm1, qword ptr [rsp + 0x38]
0212e207: movsd  qword ptr [rsp + 0x50], xmm1
0212e20d: mov    qword ptr [rsp + 0x28], rdi
0212e212: lea    rbx, [rsp + 0x40]
0212e217: mov    qword ptr [rsp + 0x30], rbx
0212e21c: nop    dword ptr [rax]
0212e220: mov    rdx, qword ptr [rip + 0x1b92409]         ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
0212e227: lea    rcx, [rsp + 0x40]
0212e22c: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
0212e231: test   al, al
0212e233: je     0x18212e258
0212e235: mov    rcx, qword ptr [rsp + 0x50]
0212e23a: test   rcx, rcx
0212e23d: je     0x18212e220
0212e23f: xor    edx, edx
0212e241: call   0x18215fed0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʿʼʿʿʵʵʲʿʷˁʻ
0212e246: cmp    rax, rdi
0212e249: jle    0x18212e220
0212e24b: mov    rdi, rax
0212e24e: mov    qword ptr [rsp + 0x98], rax
0212e256: jmp    0x18212e220
0212e258: mov    rdx, qword ptr [rip + 0x1b92311]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
0212e25f: mov    rcx, rbx
0212e262: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0212e267: jmp    0x18212e29d
0212e269: mov    rdx, qword ptr [rip + 0x1b92300]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
0212e270: mov    rcx, qword ptr [rsp + 0x30]
0212e275: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0212e27a: mov    rcx, qword ptr [rsp + 0x28]
0212e27f: test   rcx, rcx
0212e282: jne    0x18212e300
0212e284: movzx  r14d, byte ptr [rsp + 0x88]
0212e28d: mov    rsi, qword ptr [rsp + 0x80]
0212e295: mov    rdi, qword ptr [rsp + 0x98]
0212e29d: xorps  xmm0, xmm0
0212e2a0: movups xmmword ptr [rsp + 0x28], xmm0
0212e2a5: mov    r8, qword ptr [rip + 0x1bd4c14]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
0212e2ac: mov    rdx, rdi
0212e2af: lea    rcx, [rsp + 0x28]
0212e2b4: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
0212e2b9: movups xmm0, xmmword ptr [rsp + 0x28]
0212e2be: movups xmmword ptr [rsi + 0x120], xmm0
0212e2c5: lea    rcx, [rsi + 0x120]
0212e2cc: mov    rdx, qword ptr [rip + 0x1bd4f9d]         ; [0x3d03270] metamethod:Method$System.Nullable<long>.get_Value()
0212e2d3: call   0x1808689b0                              ; System.Nullable<ulong>$$get_Value
0212e2d8: test   r14b, r14b
0212e2db: je     0x18212e2e9
0212e2dd: cvttsd2si rcx, qword ptr [rsi + 0xb0]
0212e2e6: add    rax, rcx
0212e2e9: mov    rbx, qword ptr [rsp + 0x90]
0212e2f1: add    rsp, 0x60
0212e2f5: pop    r14
0212e2f7: pop    rdi
0212e2f8: pop    rsi
0212e2f9: ret    
0212e2fa: call   0x182f60c50
0212e2ff: int3   
0212e300: call   0x182f60ce0
0212e305: int3   
0212e306: int3   
