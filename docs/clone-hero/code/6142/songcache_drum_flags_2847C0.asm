002847c0: mov    rax, rsp
002847c3: mov    qword ptr [rax + 0x18], rbx
002847c7: mov    qword ptr [rax + 0x10], rdx
002847cb: mov    qword ptr [rax + 8], rcx
002847cf: push   rsi
002847d0: push   rdi
002847d1: push   r13
002847d3: push   r14
002847d5: push   r15
002847d7: sub    rsp, 0xf0
002847de: movaps xmmword ptr [rax - 0x38], xmm6
002847e2: mov    rsi, rdx
002847e5: mov    r13, rcx
002847e8: cmp    byte ptr [rip + 0x3c88f38], 0            ; [0x3f0d727] (bss)
002847ef: jne    0x180284898
002847f5: lea    rcx, [rip + 0x3a3bd74]                   ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
002847fc: call   0x182f609b0
00284801: lea    rcx, [rip + 0x3a3c660]                   ; [0x3cc0e68] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.Dispose()
00284808: call   0x182f609b0
0028480d: lea    rcx, [rip + 0x3a3c714]                   ; [0x3cc0f28] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.MoveNext()
00284814: call   0x182f609b0
00284819: lea    rcx, [rip + 0x3a3be10]                   ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
00284820: call   0x182f609b0
00284825: lea    rcx, [rip + 0x3a3bebc]                   ; [0x3cc06e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Current()
0028482c: call   0x182f609b0
00284831: lea    rcx, [rip + 0x3a3c7b0]                   ; [0x3cc0fe8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.get_Current()
00284838: call   0x182f609b0
0028483d: lea    rcx, [rip + 0x3a3383c]                   ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
00284844: call   0x182f609b0
00284849: lea    rcx, [rip + 0x3a4ca70]                   ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
00284850: call   0x182f609b0
00284855: lea    rcx, [rip + 0x3a4f224]                   ; [0x3cd3a80] metamethod:Method$System.Collections.Generic.List<ʹʿʸʷʲʻˁʺʿʷʵ>.GetEnumerator()
0028485c: call   0x182f609b0
00284861: lea    rcx, [rip + 0x3a4f2d8]                   ; [0x3cd3b40] metamethod:Method$System.Collections.Generic.List<ʹʿʸʷʲʻˁʺʿʷʵ>.get_Count()
00284868: call   0x182f609b0
0028486d: lea    rcx, [rip + 0x3a52eac]                   ; [0x3cd7720] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Count()
00284874: call   0x182f609b0
00284879: lea    rcx, [rip + 0x3a27e90]                   ; [0x3cac710] str:'setFlags'
00284880: call   0x182f609b0
00284885: lea    rcx, [rip + 0x3a35aec]                   ; [0x3cba378] str:'Keys'
0028488c: call   0x182f609b0
00284891: mov    byte ptr [rip + 0x3c88e8f], 1            ; [0x3f0d727] (bss)
00284898: xor    r15d, r15d
0028489b: mov    qword ptr [rsp + 0x30], r15
002848a0: mov    byte ptr [rsp + 0x138], r15b
002848a8: xorps  xmm0, xmm0
002848ab: xor    eax, eax
002848ad: movups xmmword ptr [rsp + 0x90], xmm0
002848b5: mov    qword ptr [rsp + 0xa0], rax
002848bd: xorps  xmm1, xmm1
002848c0: movups xmmword ptr [rsp + 0xa8], xmm1
002848c8: mov    qword ptr [rsp + 0xb8], rax
002848d0: mov    qword ptr [rsp + 0x50], r15
002848d5: mov    qword ptr [rsp + 0x58], r15
002848da: lea    rdi, [rsp + 0x50]
002848df: mov    qword ptr [rsp + 0x60], rdi
002848e4: test   rsi, rsi
002848e7: je     0x180284e87
002848ed: mov    qword ptr [rsp + 0x20], r15
002848f2: xor    r9d, r9d
002848f5: mov    r8b, 1
002848f8: movzx  edx, r8b
002848fc: mov    rcx, rsi
002848ff: call   0x18035c110                              ; SongEntry$$ʿʲʷʹʿʳʾʳʷˀʾ
00284904: mov    qword ptr [rsp + 0x50], rax
00284909: test   rax, rax
0028490c: je     0x180284dc6
00284912: cmp    byte ptr [rax + 0xc0], r15b
00284919: je     0x180284dc6
0028491f: cmp    byte ptr [rax + 0x110], r15b
00284926: je     0x18028494a
00284928: mov    rcx, qword ptr [r13 + 0x38]
0028492c: test   rcx, rcx
0028492f: je     0x180284e2e
00284935: mov    r8, qword ptr [rip + 0x3a33744]          ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
0028493c: mov    rdx, qword ptr [rsi + 0x30]
00284940: call   0x182c425f0
00284945: mov    rax, qword ptr [rsp + 0x50]
0028494a: mov    qword ptr [rsi + 0x20], r15
0028494e: test   rax, rax
00284951: je     0x180284e82
00284957: mov    rdx, qword ptr [rax + 0xd0]
0028495e: test   rdx, rdx
00284961: je     0x180284e7d
00284967: mov    r8, qword ptr [rip + 0x3a4c952]          ; [0x3cd12c0] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.GetEnumerator()
0028496e: lea    rcx, [rsp + 0x70]
00284973: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
00284978: movups xmm0, xmmword ptr [rsp + 0x70]
0028497d: movups xmmword ptr [rsp + 0x90], xmm0
00284985: movsd  xmm1, qword ptr [rsp + 0x80]
0028498e: movsd  qword ptr [rsp + 0xa0], xmm1
00284997: mov    qword ptr [rsp + 0x40], r15
0028499c: lea    rbx, [rsp + 0x90]
002849a4: mov    qword ptr [rsp + 0x48], rbx
002849a9: nop    dword ptr [rax]
002849b0: mov    rdx, qword ptr [rip + 0x3a3bc79]         ; [0x3cc0630] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.MoveNext()
002849b7: lea    rcx, [rsp + 0x90]
002849bf: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
002849c4: test   al, al
002849c6: je     0x180284a89
002849cc: mov    r14, qword ptr [rsp + 0xa0]
002849d4: test   r14, r14
002849d7: je     0x1802849b0
002849d9: mov    rax, qword ptr [r14 + 0x60]
002849dd: test   rax, rax
002849e0: je     0x180284e34
002849e6: cmp    dword ptr [rax + 0x18], 0
002849ea: je     0x1802849b0
002849ec: cmp    byte ptr [r14 + 0x58], 0xb7
002849f1: je     0x1802849b0
002849f3: cmp    byte ptr [r14 + 0x58], 0x84
002849f8: je     0x1802849b0
002849fa: cmp    byte ptr [r14 + 0x58], 2
002849ff: jne    0x180284a5b
00284a01: xor    edx, edx
00284a03: mov    rcx, r14
00284a06: call   0x18215e000                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʶˀʼˀʶʺʾʹʿʿʴ
00284a0b: test   al, al
00284a0d: jne    0x180284a1f
00284a0f: cmp    byte ptr [rsi + 0x92], al
00284a15: jne    0x180284a1f
00284a17: cmp    byte ptr [rsi + 0x93], al
00284a1d: je     0x180284a32
00284a1f: lea    rcx, [rsi + 0x20]
00284a23: xor    r9d, r9d
00284a26: movzx  r8d, byte ptr [r14 + 0x59]
00284a2b: mov    dl, 0xe
00284a2d: call   0x1801f1250                              ; ʷʾʻʵˀʽʺʷʼʲʶ$$ʹʽʵʵʷˁˀʶʽʶˁ
00284a32: movsx  ecx, byte ptr [r14 + 0x59]
00284a37: sub    ecx, 0x20
00284a3a: mov    edx, 1
00284a3f: shl    rdx, cl
00284a42: mov    rax, rdx
00284a45: and    rax, qword ptr [rsi + 0x20]
00284a49: cmp    rax, rdx
00284a4c: je     0x1802849b0
00284a52: or     qword ptr [rsi + 0x20], rdx
00284a56: jmp    0x1802849b0
00284a5b: movsx  ecx, byte ptr [r14 + 0x58]
00284a60: movsx  eax, byte ptr [r14 + 0x59]
00284a65: lea    ecx, [rax + rcx*4]
00284a68: mov    edx, 1
00284a6d: shl    rdx, cl
00284a70: mov    rax, rdx
00284a73: and    rax, qword ptr [rsi + 0x20]
00284a77: cmp    rax, rdx
00284a7a: je     0x1802849b0
00284a80: or     qword ptr [rsi + 0x20], rdx
00284a84: jmp    0x1802849b0
00284a89: mov    rdx, qword ptr [rip + 0x3a3bae0]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
00284a90: mov    rcx, rbx
00284a93: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00284a98: jmp    0x180284ad1
00284a9a: mov    rdx, qword ptr [rip + 0x3a3bacf]         ; [0x3cc0570] metamethod:Method$System.Collections.Generic.List.Enumerator<ʷʿʽʽʵʻʶʹʼʼʺ>.Dispose()
00284aa1: mov    rcx, qword ptr [rsp + 0x48]
00284aa6: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00284aab: mov    rcx, qword ptr [rsp + 0x40]
00284ab0: test   rcx, rcx
00284ab3: jne    0x180284e3a
00284ab9: xor    r15d, r15d
00284abc: mov    rsi, qword ptr [rsp + 0x128]
00284ac4: mov    r13, qword ptr [rsp + 0x120]
00284acc: mov    rdi, qword ptr [rsp + 0x60]
00284ad1: mov    rdx, qword ptr [rsp + 0x50]
00284ad6: test   rdx, rdx
00284ad9: je     0x180284e78
00284adf: mov    rcx, qword ptr [rdx + 0xf0]
00284ae6: test   rcx, rcx
00284ae9: je     0x180284e73
00284aef: mov    ecx, dword ptr [rcx + 0x18]
00284af2: test   ecx, ecx
00284af4: setg   al
00284af7: mov    byte ptr [rsi + 0x90], al
00284afd: test   ecx, ecx
00284aff: jle    0x180284be5
00284b05: test   rdx, rdx
00284b08: je     0x180284e5a
00284b0e: mov    rdx, qword ptr [rdx + 0xf0]
00284b15: test   rdx, rdx
00284b18: je     0x180284e55
00284b1e: mov    r8, qword ptr [rip + 0x3a4ef5b]          ; [0x3cd3a80] metamethod:Method$System.Collections.Generic.List<ʹʿʸʷʲʻˁʺʿʷʵ>.GetEnumerator()
00284b25: lea    rcx, [rsp + 0x70]
00284b2a: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
00284b2f: movups xmm0, xmmword ptr [rsp + 0x70]
00284b34: movups xmmword ptr [rsp + 0xa8], xmm0
00284b3c: movsd  xmm1, qword ptr [rsp + 0x80]
00284b45: movsd  qword ptr [rsp + 0xb8], xmm1
00284b4e: mov    qword ptr [rsp + 0x40], r15
00284b53: lea    rbx, [rsp + 0xa8]
00284b5b: mov    qword ptr [rsp + 0x48], rbx
00284b60: mov    rdx, qword ptr [rip + 0x3a3c3c1]         ; [0x3cc0f28] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.MoveNext()
00284b67: lea    rcx, [rsp + 0xa8]
00284b6f: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
00284b74: test   al, al
00284b76: je     0x180284bd6
00284b78: mov    rax, qword ptr [rsp + 0xb8]
00284b80: test   rax, rax
00284b83: je     0x180284e4a
00284b89: mov    rcx, qword ptr [rax + 0x18]
00284b8d: test   rcx, rcx
00284b90: je     0x180284e45
00284b96: mov    edx, 3
00284b9b: xor    r8d, r8d
00284b9e: call   0x18197f870                              ; System.String$$Contains
00284ba3: test   al, al
00284ba5: je     0x180284b60
00284ba7: mov    rcx, qword ptr [r13 + 0x30]
00284bab: test   rcx, rcx
00284bae: je     0x180284e40
00284bb4: mov    r8, qword ptr [rip + 0x3a334c5]          ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
00284bbb: mov    rdx, qword ptr [rsi + 0x30]
00284bbf: call   0x182c425f0
00284bc4: nop    
00284bc5: mov    rdx, qword ptr [rip + 0x3a3c29c]         ; [0x3cc0e68] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.Dispose()
00284bcc: mov    rcx, rbx
00284bcf: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00284bd4: jmp    0x180284be5
00284bd6: mov    rdx, qword ptr [rip + 0x3a3c28b]         ; [0x3cc0e68] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.Dispose()
00284bdd: mov    rcx, rbx
00284be0: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00284be5: cmp    qword ptr [rsi + 0x20], 0
00284bea: je     0x180284d8e
00284bf0: cmp    byte ptr [rsi + 0xb2], 0
00284bf7: je     0x180284c36
00284bf9: xor    bl, bl
00284bfb: jmp    0x180284c46
00284bfd: mov    rdx, qword ptr [rip + 0x3a3c264]         ; [0x3cc0e68] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʿʸʷʲʻˁʺʿʷʵ>.Dispose()
00284c04: mov    rcx, qword ptr [rsp + 0x48]
00284c09: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00284c0e: mov    rcx, qword ptr [rsp + 0x40]
00284c13: test   rcx, rcx
00284c16: jne    0x180284e50
00284c1c: xor    r15d, r15d
00284c1f: mov    rsi, qword ptr [rsp + 0x128]
00284c27: mov    r13, qword ptr [rsp + 0x120]
00284c2f: mov    rdi, qword ptr [rsp + 0x60]
00284c34: jmp    0x180284be5
00284c36: xor    r8d, r8d
00284c39: xor    edx, edx
00284c3b: mov    rcx, rsi
00284c3e: call   0x18035f6e0                              ; SongEntry$$ˀʹʳʸʾʿʺʿʴˀʴ
00284c43: movzx  ebx, al
00284c46: cmp    byte ptr [rip + 0x3c88ade], 0            ; [0x3f0d72b] (bss)
00284c4d: jne    0x180284c6e
00284c4f: lea    rcx, [rip + 0x3a136ca]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00284c56: call   0x182f609b0
00284c5b: lea    rcx, [rip + 0x3a18436]                   ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00284c62: call   0x182f609b0
00284c67: mov    byte ptr [rip + 0x3c88abd], 1            ; [0x3f0d72b] (bss)
00284c6e: mov    rax, qword ptr [rip + 0x3a136ab]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00284c75: mov    rcx, qword ptr [rax + 0xb8]
00284c7c: xor    edx, edx
00284c7e: mov    rcx, qword ptr [rcx + 0x1e0]
00284c85: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
00284c8a: test   al, al
00284c8c: jne    0x180284d64
00284c92: movups xmm6, xmmword ptr [rsi + 0x38]
00284c96: mov    rcx, qword ptr [rip + 0x3a183fb]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00284c9d: cmp    dword ptr [rcx + 0xe0], 0
00284ca4: jne    0x180284cab
00284ca6: call   0x182f60cf0
00284cab: movdqa xmmword ptr [rsp + 0x40], xmm6
00284cb1: xor    r8d, r8d
00284cb4: lea    rdx, [rsp + 0x30]
00284cb9: lea    rcx, [rsp + 0x40]
00284cbe: call   0x180174950                              ; ʾʲʽʻʺʶʺʺʷʿʺ$$ʻʵˁʸʷʾʶʾʴʳʿ
00284cc3: test   al, al
00284cc5: je     0x180284d04
00284cc7: mov    rax, qword ptr [rsp + 0x30]
00284ccc: test   rax, rax
00284ccf: je     0x180284e5f
00284cd5: movups xmm0, xmmword ptr [rsi + 0x38]
00284cd9: movaps xmmword ptr [rsp + 0x40], xmm0
00284cde: movups xmm1, xmmword ptr [rax + 0x38]
00284ce2: movaps xmmword ptr [rsp + 0x70], xmm1
00284ce7: xor    r8d, r8d
00284cea: lea    rdx, [rsp + 0x40]
00284cef: lea    rcx, [rsp + 0x70]
00284cf4: call   0x1820fa100                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʶʿʵʽʳʵʲʼˀʽʹ
00284cf9: test   al, al
00284cfb: je     0x180284d04
00284cfd: cmp    qword ptr [rsp + 0x30], rsi
00284d02: jne    0x180284d75
00284d04: mov    qword ptr [rsp + 0x30], r15
00284d09: xor    edx, edx
00284d0b: lea    rcx, [rsp + 0x30]
00284d10: call   0x182f5fc00
00284d15: mov    rcx, qword ptr [rsp + 0x30]
00284d1a: test   rcx, rcx
00284d1d: je     0x180284e69
00284d23: mov    r14, qword ptr [r13 + 0x68]
00284d27: mov    qword ptr [rsp + 0x20], r15
00284d2c: mov    r9, qword ptr [rip + 0x3a279dd]          ; [0x3cac710] str:'setFlags'
00284d33: mov    r8, qword ptr [rsi + 0x30]
00284d37: mov    rdx, qword ptr [rip + 0x3a3563a]         ; [0x3cba378] str:'Keys'
00284d3e: mov    rcx, qword ptr [rcx + 0x30]
00284d42: call   0x18197e8f0                              ; System.String$$Concat
00284d47: test   r14, r14
00284d4a: je     0x180284e64
00284d50: mov    r8, qword ptr [rip + 0x3a33329]          ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
00284d57: mov    rdx, rax
00284d5a: mov    rcx, r14
00284d5d: call   0x182c425f0
00284d62: jmp    0x180284d75
00284d64: mov    qword ptr [rsp + 0x30], r15
00284d69: xor    edx, edx
00284d6b: lea    rcx, [rsp + 0x30]
00284d70: call   0x182f5fc00
00284d75: mov    byte ptr [rsp + 0x138], bl
00284d7c: cmp    qword ptr [rdi], 0
00284d80: je     0x180284d8c
00284d82: xor    edx, edx
00284d84: mov    rcx, qword ptr [rdi]
00284d87: call   0x18212b430                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$Dispose
00284d8c: jmp    0x180284e0b
00284d8e: mov    rcx, qword ptr [r13 + 0x70]
00284d92: test   rcx, rcx
00284d95: je     0x180284e6e
00284d9b: mov    r8, qword ptr [rip + 0x3a332de]          ; [0x3cb8080] metamethod:Method$System.Collections.Generic.List<string>.Add()
00284da2: mov    rdx, qword ptr [rsi + 0x30]
00284da6: call   0x182c425f0
00284dab: xor    bl, bl
00284dad: mov    byte ptr [rsp + 0x138], bl
00284db4: cmp    qword ptr [rdi], 0
00284db8: je     0x180284dc4
00284dba: xor    edx, edx
00284dbc: mov    rcx, qword ptr [rdi]
00284dbf: call   0x18212b430                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$Dispose
00284dc4: jmp    0x180284e0b
00284dc6: mov    bl, 1
00284dc8: mov    byte ptr [rsp + 0x138], bl
00284dcf: test   rax, rax
00284dd2: je     0x180284dde
00284dd4: xor    edx, edx
00284dd6: mov    rcx, rax
00284dd9: call   0x18212b430                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$Dispose
00284dde: jmp    0x180284e0b
00284de0: mov    rcx, qword ptr [rsp + 0x60]
00284de5: cmp    qword ptr [rcx], 0
00284de9: je     0x180284df5
00284deb: xor    edx, edx
00284ded: mov    rcx, qword ptr [rcx]
00284df0: call   0x18212b430                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$Dispose
00284df5: mov    rcx, qword ptr [rsp + 0x58]
00284dfa: test   rcx, rcx
00284dfd: jne    0x180284e8d
00284e03: movzx  ebx, byte ptr [rsp + 0x138]
00284e0b: movzx  eax, bl
00284e0e: mov    rbx, qword ptr [rsp + 0x130]
00284e16: movaps xmm6, xmmword ptr [rsp + 0xe0]
00284e1e: add    rsp, 0xf0
00284e25: pop    r15
00284e27: pop    r14
00284e29: pop    r13
00284e2b: pop    rdi
00284e2c: pop    rsi
00284e2d: ret    
00284e2e: call   0x182f60c50
00284e33: nop    
00284e34: call   0x182f60c50
00284e39: nop    
00284e3a: call   0x182f60ce0
00284e3f: nop    
00284e40: call   0x182f60c50
00284e45: call   0x182f60c50
00284e4a: call   0x182f60c50
00284e4f: nop    
00284e50: call   0x182f60ce0
00284e55: call   0x182f60c50
00284e5a: call   0x182f60c50
00284e5f: call   0x182f60c50
00284e64: call   0x182f60c50
00284e69: call   0x182f60c50
00284e6e: call   0x182f60c50
00284e73: call   0x182f60c50
00284e78: call   0x182f60c50
00284e7d: call   0x182f60c50
00284e82: call   0x182f60c50
00284e87: call   0x182f60c50
00284e8c: nop    
00284e8d: call   0x182f60ce0
00284e92: int3   
00284e93: int3   
