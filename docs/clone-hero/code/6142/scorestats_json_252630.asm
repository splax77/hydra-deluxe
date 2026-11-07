00252630: mov    qword ptr [rsp + 8], rbx
00252635: mov    qword ptr [rsp + 0x10], rsi
0025263a: push   rdi
0025263b: push   r14
0025263d: push   r15
0025263f: sub    rsp, 0xd0
00252646: movaps xmmword ptr [rsp + 0xc0], xmm6
0025264e: mov    rsi, rcx
00252651: cmp    byte ptr [rip + 0x3cba8f8], 0            ; [0x3f0cf50] (bss)
00252658: jne    0x18025276d
0025265e: lea    rcx, [rip + 0x3a84eab]                   ; [0x3cd7510] meta:System.DateTime_TypeInfo
00252665: call   0x182f609b0
0025266a: lea    rcx, [rip + 0x3a6e377]                   ; [0x3cc09e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.Dispose()
00252671: call   0x182f609b0
00252676: lea    rcx, [rip + 0x3a6e42b]                   ; [0x3cc0aa8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.MoveNext()
0025267d: call   0x182f609b0
00252682: lea    rcx, [rip + 0x3a6e4df]                   ; [0x3cc0b68] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.get_Current()
00252689: call   0x182f609b0
0025268e: lea    rcx, [rip + 0x3aab63b]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00252695: call   0x182f609b0
0025269a: lea    rcx, [rip + 0x3a807df]                   ; [0x3cd2e80] metamethod:Method$System.Collections.Generic.List<ʹʵʽˁʼʺʷʿʷʲʻ>.GetEnumerator()
002526a1: call   0x182f609b0
002526a6: lea    rcx, [rip + 0x3a51883]                   ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
002526ad: call   0x182f609b0
002526b2: lea    rcx, [rip + 0x3a46c8f]                   ; [0x3c99348] meta:ʺʽʿˁʶʾʾʳʺʻʿ_TypeInfo
002526b9: call   0x182f609b0
002526be: lea    rcx, [rip + 0x3a9c193]                   ; [0x3cee858] meta:System.Text.Json.Utf8JsonWriter_TypeInfo
002526c5: call   0x182f609b0
002526ca: lea    rcx, [rip + 0x3a5476f]                   ; [0x3ca6e40] str:'band_score'
002526d1: call   0x182f609b0
002526d6: lea    rcx, [rip + 0x3ab888b]                   ; [0x3d0af68] str:'playback_speed'
002526dd: call   0x182f609b0
002526e2: lea    rcx, [rip + 0x3a5480f]                   ; [0x3ca6ef8] str:'band_stars'
002526e9: call   0x182f609b0
002526ee: lea    rcx, [rip + 0x3a6701b]                   ; [0x3cb9710] str:'song_name'
002526f5: call   0x182f609b0
002526fa: lea    rcx, [rip + 0x3a55e07]                   ; [0x3ca8508] str:'score_timestamp'
00252701: call   0x182f609b0
00252706: lea    rcx, [rip + 0x3a6f503]                   ; [0x3cc1c10] str:'charter_name'
0025270d: call   0x182f609b0
00252712: lea    rcx, [rip + 0x3aa31df]                   ; [0x3cf58f8] str:'O'
00252719: call   0x182f609b0
0025271e: lea    rcx, [rip + 0x3ab8b33]                   ; [0x3d0b258] str:'player_count'
00252725: call   0x182f609b0
0025272a: lea    rcx, [rip + 0x3a5010f]                   ; [0x3ca2840] str:'artist_name'
00252731: call   0x182f609b0
00252736: lea    rcx, [rip + 0x3a6fad3]                   ; [0x3cc2210] str:'checksum'
0025273d: call   0x182f609b0
00252742: lea    rcx, [rip + 0x3ab2347]                   ; [0x3d04a90] str:'game_mode'
00252749: call   0x182f609b0
0025274e: lea    rcx, [rip + 0x3ab8df3]                   ; [0x3d0b548] str:'players'
00252755: call   0x182f609b0
0025275a: lea    rcx, [rip + 0x3ab23ef]                   ; [0x3d04b50] str:'game_version'
00252761: call   0x182f609b0
00252766: mov    byte ptr [rip + 0x3cba7e3], 1            ; [0x3f0cf50] (bss)
0025276d: xor    r15d, r15d
00252770: mov    qword ptr [rsp + 0x108], r15
00252778: mov    qword ptr [rsp + 0x100], r15
00252780: xorps  xmm0, xmm0
00252783: xor    eax, eax
00252785: movups xmmword ptr [rsp + 0x68], xmm0
0025278a: mov    qword ptr [rsp + 0x78], rax
0025278f: mov    qword ptr [rsp + 0x50], r15
00252794: movups xmmword ptr [rsp + 0x80], xmm0
0025279c: xor    r8d, r8d
0025279f: mov    dl, 1
002527a1: lea    rcx, [rsp + 0x80]
002527a9: call   0x1823c1f00                              ; System.Text.Json.JsonWriterOptions$$set_Indented
002527ae: movaps xmm6, xmmword ptr [rsp + 0x80]
002527b6: mov    rcx, qword ptr [rip + 0x3a51773]         ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
002527bd: call   0x182f60c00
002527c2: mov    rbx, rax
002527c5: xor    edx, edx
002527c7: mov    rcx, rax
002527ca: call   0x181af8fa0                              ; System.IO.MemoryStream$$.ctor
002527cf: mov    qword ptr [rsp + 0x108], rbx
002527d7: mov    qword ptr [rsp + 0x40], r15
002527dc: lea    rdi, [rsp + 0x108]
002527e4: mov    qword ptr [rsp + 0x48], rdi
002527e9: mov    r14, qword ptr [rsp + 0x108]
002527f1: mov    rcx, qword ptr [rip + 0x3a9c060]         ; [0x3cee858] meta:System.Text.Json.Utf8JsonWriter_TypeInfo
002527f8: call   0x182f60c00
002527fd: mov    rbx, rax
00252800: movdqa xmmword ptr [rsp + 0x20], xmm6
00252806: xor    r9d, r9d
00252809: lea    r8, [rsp + 0x20]
0025280e: mov    rdx, r14
00252811: mov    rcx, rax
00252814: call   0x1823d5d30                              ; System.Text.Json.Utf8JsonWriter$$.ctor
00252819: mov    qword ptr [rsp + 0x100], rbx
00252821: mov    qword ptr [rsp + 0x58], r15
00252826: lea    rbx, [rsp + 0x100]
0025282e: mov    qword ptr [rsp + 0x60], rbx
00252833: mov    rcx, qword ptr [rsp + 0x100]
0025283b: test   rcx, rcx
0025283e: je     0x180252cd9
00252844: xor    edx, edx
00252846: call   0x1823cfaf0                              ; System.Text.Json.Utf8JsonWriter$$WriteStartObject
0025284b: mov    rcx, qword ptr [rsp + 0x100]
00252853: test   rcx, rcx
00252856: je     0x180252cd4
0025285c: xor    r9d, r9d
0025285f: mov    r8, qword ptr [rsi + 0x10]
00252863: mov    rdx, qword ptr [rip + 0x3ab22e6]         ; [0x3d04b50] str:'game_version'
0025286a: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
0025286f: mov    rcx, qword ptr [rsp + 0x100]
00252877: test   rcx, rcx
0025287a: je     0x180252ccf
00252880: xor    r9d, r9d
00252883: mov    r8, qword ptr [rsi + 0x18]
00252887: mov    rdx, qword ptr [rip + 0x3a6f982]         ; [0x3cc2210] str:'checksum'
0025288e: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
00252893: mov    rcx, qword ptr [rsp + 0x100]
0025289b: test   rcx, rcx
0025289e: je     0x180252cca
002528a4: xor    r9d, r9d
002528a7: mov    r8, qword ptr [rsi + 0x20]
002528ab: mov    rdx, qword ptr [rip + 0x3a66e5e]         ; [0x3cb9710] str:'song_name'
002528b2: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
002528b7: mov    rcx, qword ptr [rsp + 0x100]
002528bf: test   rcx, rcx
002528c2: je     0x180252cc5
002528c8: xor    r9d, r9d
002528cb: mov    r8, qword ptr [rsi + 0x28]
002528cf: mov    rdx, qword ptr [rip + 0x3a4ff6a]         ; [0x3ca2840] str:'artist_name'
002528d6: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
002528db: mov    rcx, qword ptr [rsp + 0x100]
002528e3: test   rcx, rcx
002528e6: je     0x180252cc0
002528ec: xor    r9d, r9d
002528ef: mov    r8, qword ptr [rsi + 0x30]
002528f3: mov    rdx, qword ptr [rip + 0x3a6f316]         ; [0x3cc1c10] str:'charter_name'
002528fa: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
002528ff: mov    rcx, qword ptr [rsp + 0x100]
00252907: test   rcx, rcx
0025290a: je     0x180252cbb
00252910: xor    r9d, r9d
00252913: mov    r8d, dword ptr [rsi + 0x38]
00252917: mov    rdx, qword ptr [rip + 0x3ab864a]         ; [0x3d0af68] str:'playback_speed'
0025291e: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00252923: mov    r14, qword ptr [rsp + 0x100]
0025292b: mov    rax, qword ptr [rip + 0x3a46a16]         ; [0x3c99348] meta:ʺʽʿˁʶʾʾʳʺʻʿ_TypeInfo
00252932: mov    qword ptr [rsp + 0x20], rax
00252937: mov    qword ptr [rsp + 0x28], 0xffffffffffffffff
00252940: mov    eax, dword ptr [rsi + 0x3c]
00252943: mov    dword ptr [rsp + 0x30], eax
00252947: xor    edx, edx
00252949: lea    rcx, [rsp + 0x20]
0025294e: call   0x181b91030                              ; System.Enum$$ToString
00252953: test   r14, r14
00252956: je     0x180252cb6
0025295c: xor    r9d, r9d
0025295f: mov    r8, rax
00252962: mov    rdx, qword ptr [rip + 0x3ab2127]         ; [0x3d04a90] str:'game_mode'
00252969: mov    rcx, r14
0025296c: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
00252971: mov    r14, qword ptr [rsp + 0x100]
00252979: mov    rcx, qword ptr [rip + 0x3a84b90]         ; [0x3cd7510] meta:System.DateTime_TypeInfo
00252980: cmp    dword ptr [rcx + 0xe0], r15d
00252987: jne    0x18025298e
00252989: call   0x182f60cf0
0025298e: lea    rcx, [rsi + 0x40]
00252992: xor    r8d, r8d
00252995: mov    rdx, qword ptr [rip + 0x3aa2f5c]         ; [0x3cf58f8] str:'O'
0025299c: call   0x181b452b0                              ; System.DateTime$$ToString
002529a1: test   r14, r14
002529a4: je     0x180252cb1
002529aa: xor    r9d, r9d
002529ad: mov    r8, rax
002529b0: mov    rdx, qword ptr [rip + 0x3a55b51]         ; [0x3ca8508] str:'score_timestamp'
002529b7: mov    rcx, r14
002529ba: call   0x1823d58b0                              ; System.Text.Json.Utf8JsonWriter$$WriteString
002529bf: mov    rcx, qword ptr [rsp + 0x100]
002529c7: test   rcx, rcx
002529ca: je     0x180252cac
002529d0: xor    r9d, r9d
002529d3: mov    r8d, dword ptr [rsi + 0x48]
002529d7: mov    rdx, qword ptr [rip + 0x3a54462]         ; [0x3ca6e40] str:'band_score'
002529de: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
002529e3: mov    rcx, qword ptr [rsp + 0x100]
002529eb: test   rcx, rcx
002529ee: je     0x180252ca7
002529f4: xor    r9d, r9d
002529f7: mov    r8d, dword ptr [rsi + 0x4c]
002529fb: mov    rdx, qword ptr [rip + 0x3a544f6]         ; [0x3ca6ef8] str:'band_stars'
00252a02: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00252a07: mov    rcx, qword ptr [rsp + 0x100]
00252a0f: test   rcx, rcx
00252a12: je     0x180252ca2
00252a18: xor    r9d, r9d
00252a1b: mov    r8d, dword ptr [rsi + 0x50]
00252a1f: mov    rdx, qword ptr [rip + 0x3ab8832]         ; [0x3d0b258] str:'player_count'
00252a26: call   0x1823cd1e0                              ; System.Text.Json.Utf8JsonWriter$$WriteNumber
00252a2b: mov    rcx, qword ptr [rsp + 0x100]
00252a33: test   rcx, rcx
00252a36: je     0x180252c9d
00252a3c: xor    r8d, r8d
00252a3f: mov    rdx, qword ptr [rip + 0x3ab8b02]         ; [0x3d0b548] str:'players'
00252a46: call   0x1823cef70                              ; System.Text.Json.Utf8JsonWriter$$WriteStartArray
00252a4b: mov    rdx, qword ptr [rsi + 0x58]
00252a4f: test   rdx, rdx
00252a52: je     0x180252c98
00252a58: mov    r8, qword ptr [rip + 0x3a80421]          ; [0x3cd2e80] metamethod:Method$System.Collections.Generic.List<ʹʵʽˁʼʺʷʿʷʲʻ>.GetEnumerator()
00252a5f: lea    rcx, [rsp + 0xa8]
00252a67: call   0x18071c480                              ; System.Collections.Generic.List<zSDEFvrHcwablfPQnYQrBdwuaKoGb.GWKxPOWIKesulFdKzwlOvEHpfeku>$$GetEnumerator
00252a6c: movups xmm0, xmmword ptr [rsp + 0xa8]
00252a74: movups xmmword ptr [rsp + 0x68], xmm0
00252a79: movsd  xmm1, qword ptr [rsp + 0xb8]
00252a82: movsd  qword ptr [rsp + 0x78], xmm1
00252a88: mov    qword ptr [rsp + 0x20], r15
00252a8d: lea    rsi, [rsp + 0x68]
00252a92: mov    qword ptr [rsp + 0x28], rsi
00252a97: nop    word ptr [rax + rax]
00252aa0: mov    rdx, qword ptr [rip + 0x3a6e001]         ; [0x3cc0aa8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.MoveNext()
00252aa7: lea    rcx, [rsp + 0x68]
00252aac: call   0x1814fb3b0                              ; System.Collections.Generic.List.Enumerator<object>$$MoveNext
00252ab1: test   al, al
00252ab3: je     0x180252b08
00252ab5: mov    rcx, qword ptr [rsp + 0x100]
00252abd: test   rcx, rcx
00252ac0: je     0x180252c83
00252ac6: mov    r14, qword ptr [rsp + 0x78]
00252acb: xor    edx, edx
00252acd: call   0x1823cfaf0                              ; System.Text.Json.Utf8JsonWriter$$WriteStartObject
00252ad2: test   r14, r14
00252ad5: je     0x180252c7e
00252adb: xor    r8d, r8d
00252ade: mov    rdx, qword ptr [rsp + 0x100]
00252ae6: mov    rcx, r14
00252ae9: call   0x1801144c0                              ; ʹʵʽˁʼʺʷʿʷʲʻ$$ʽʻʷʶʸʽʼʽʵˀʵ
00252aee: mov    rcx, qword ptr [rsp + 0x100]
00252af6: test   rcx, rcx
00252af9: je     0x180252c78
00252aff: xor    edx, edx
00252b01: call   0x1823c63d0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndObject
00252b06: jmp    0x180252aa0
00252b08: mov    rdx, qword ptr [rip + 0x3a6ded9]         ; [0x3cc09e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.Dispose()
00252b0f: mov    rcx, rsi
00252b12: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00252b17: jmp    0x180252b42
00252b19: mov    rdx, qword ptr [rip + 0x3a6dec8]         ; [0x3cc09e8] metamethod:Method$System.Collections.Generic.List.Enumerator<ʹʵʽˁʼʺʷʿʷʲʻ>.Dispose()
00252b20: mov    rcx, qword ptr [rsp + 0x28]
00252b25: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
00252b2a: mov    rcx, qword ptr [rsp + 0x20]
00252b2f: test   rcx, rcx
00252b32: jne    0x180252c89
00252b38: mov    rdi, qword ptr [rsp + 0x48]
00252b3d: mov    rbx, qword ptr [rsp + 0x60]
00252b42: mov    rcx, qword ptr [rsp + 0x100]
00252b4a: test   rcx, rcx
00252b4d: je     0x180252c93
00252b53: xor    edx, edx
00252b55: call   0x1823c60f0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndArray
00252b5a: mov    rcx, qword ptr [rsp + 0x100]
00252b62: test   rcx, rcx
00252b65: je     0x180252c8e
00252b6b: xor    edx, edx
00252b6d: call   0x1823c63d0                              ; System.Text.Json.Utf8JsonWriter$$WriteEndObject
00252b72: nop    
00252b73: cmp    qword ptr [rbx], 0
00252b77: je     0x180252b8a
00252b79: xor    ecx, ecx
00252b7b: mov    r8, qword ptr [rbx]
00252b7e: mov    rdx, qword ptr [rip + 0x3aab14b]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00252b85: call   0x182c42550
00252b8a: jmp    0x180252bbb
00252b8c: mov    r8, qword ptr [rsp + 0x60]
00252b91: cmp    qword ptr [r8], 0
00252b95: je     0x180252ba8
00252b97: xor    ecx, ecx
00252b99: mov    r8, qword ptr [r8]
00252b9c: mov    rdx, qword ptr [rip + 0x3aab12d]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00252ba3: call   0x182c42550
00252ba8: mov    rcx, qword ptr [rsp + 0x58]
00252bad: test   rcx, rcx
00252bb0: jne    0x180252cdf
00252bb6: mov    rdi, qword ptr [rsp + 0x48]
00252bbb: xor    ecx, ecx
00252bbd: call   0x1819a93b0                              ; System.Text.Encoding$$get_UTF8
00252bc2: mov    rbx, rax
00252bc5: mov    rcx, qword ptr [rsp + 0x108]
00252bcd: test   rcx, rcx
00252bd0: je     0x180252ce9
00252bd6: mov    r8, qword ptr [rcx]
00252bd9: mov    rdx, qword ptr [r8 + 0x420]
00252be0: call   qword ptr [r8 + 0x418]
00252be7: test   rbx, rbx
00252bea: je     0x180252ce4
00252bf0: mov    r9, qword ptr [rbx]
00252bf3: mov    r8, qword ptr [r9 + 0x370]
00252bfa: mov    rdx, rax
00252bfd: mov    rcx, rbx
00252c00: call   qword ptr [r9 + 0x368]
00252c07: mov    rbx, rax
00252c0a: mov    qword ptr [rsp + 0x50], rax
00252c0f: cmp    qword ptr [rdi], 0
00252c13: je     0x180252c26
00252c15: xor    ecx, ecx
00252c17: mov    r8, qword ptr [rdi]
00252c1a: mov    rdx, qword ptr [rip + 0x3aab0af]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00252c21: call   0x182c42550
00252c26: jmp    0x180252c57
00252c28: mov    r8, qword ptr [rsp + 0x48]
00252c2d: cmp    qword ptr [r8], 0
00252c31: je     0x180252c44
00252c33: xor    ecx, ecx
00252c35: mov    r8, qword ptr [r8]
00252c38: mov    rdx, qword ptr [rip + 0x3aab091]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
00252c3f: call   0x182c42550
00252c44: mov    rcx, qword ptr [rsp + 0x40]
00252c49: test   rcx, rcx
00252c4c: jne    0x180252cef
00252c52: mov    rbx, qword ptr [rsp + 0x50]
00252c57: mov    rax, rbx
00252c5a: lea    r11, [rsp + 0xd0]
00252c62: mov    rbx, qword ptr [r11 + 0x20]
00252c66: mov    rsi, qword ptr [r11 + 0x28]
00252c6a: movaps xmm6, xmmword ptr [r11 - 0x10]
00252c6f: mov    rsp, r11
00252c72: pop    r15
00252c74: pop    r14
00252c76: pop    rdi
00252c77: ret    
00252c78: call   0x182f60c50
00252c7d: nop    
00252c7e: call   0x182f60c50
00252c83: call   0x182f60c50
00252c88: nop    
00252c89: call   0x182f60ce0
00252c8e: call   0x182f60c50
00252c93: call   0x182f60c50
00252c98: call   0x182f60c50
00252c9d: call   0x182f60c50
00252ca2: call   0x182f60c50
00252ca7: call   0x182f60c50
00252cac: call   0x182f60c50
00252cb1: call   0x182f60c50
00252cb6: call   0x182f60c50
00252cbb: call   0x182f60c50
00252cc0: call   0x182f60c50
00252cc5: call   0x182f60c50
00252cca: call   0x182f60c50
00252ccf: call   0x182f60c50
00252cd4: call   0x182f60c50
00252cd9: call   0x182f60c50
00252cde: nop    
00252cdf: call   0x182f60ce0
00252ce4: call   0x182f60c50
00252ce9: call   0x182f60c50
00252cee: nop    
00252cef: call   0x182f60ce0
00252cf4: int3   
00252cf5: int3   
