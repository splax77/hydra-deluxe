0197f670: mov    qword ptr [rsp + 8], rbx
0197f675: mov    qword ptr [rsp + 0x10], rsi
0197f67a: push   rdi
0197f67b: sub    rsp, 0x40
0197f67f: cmp    byte ptr [rip + 0x259465a], 0            ; [0x3f13ce0] (bss)
0197f686: mov    rsi, rdx
0197f689: mov    edi, dword ptr [rcx + 0x10]
0197f68c: mov    rbx, rcx
0197f68f: jne    0x18197f6b0
0197f691: lea    rcx, [rip + 0x234aa80]                   ; [0x3cca118] meta:System.Globalization.CompareInfo_TypeInfo
0197f698: call   0x182f609b0
0197f69d: lea    rcx, [rip + 0x2350f8c]                   ; [0x3cd0630] meta:System.Globalization.CultureInfo_TypeInfo
0197f6a4: call   0x182f609b0
0197f6a9: mov    byte ptr [rip + 0x2594630], 1            ; [0x3f13ce0] (bss)
0197f6b0: test   rsi, rsi
0197f6b3: je     0x18197f733
0197f6b5: cmp    dword ptr [rbx + 0x10], 0
0197f6b9: jl     0x18197f7dd
0197f6bf: test   edi, edi
0197f6c1: js     0x18197f782
0197f6c7: cmp    dword ptr [rbx + 0x10], edi
0197f6ca: js     0x18197f782
0197f6d0: mov    rax, qword ptr [rip + 0x234aa41]         ; [0x3cca118] meta:System.Globalization.CompareInfo_TypeInfo
0197f6d7: cmp    dword ptr [rax + 0xe0], 0
0197f6de: jne    0x18197f6ef
0197f6e0: mov    rcx, rax
0197f6e3: call   0x182f60cf0
0197f6e8: mov    rax, qword ptr [rip + 0x234aa29]         ; [0x3cca118] meta:System.Globalization.CompareInfo_TypeInfo
0197f6ef: mov    rax, qword ptr [rax + 0xb8]
0197f6f6: mov    rcx, qword ptr [rax]
0197f6f9: test   rcx, rcx
0197f6fc: je     0x18197f77c
0197f6fe: mov    qword ptr [rsp + 0x30], 0
0197f707: xor    r9d, r9d
0197f70a: mov    byte ptr [rsp + 0x28], 0
0197f70f: mov    r8, rsi
0197f712: mov    rdx, rbx
0197f715: mov    dword ptr [rsp + 0x20], edi
0197f719: call   0x181a72950                              ; System.Globalization.CompareInfo$$IndexOfOrdinal
0197f71e: mov    rbx, qword ptr [rsp + 0x50]
0197f723: mov    rsi, qword ptr [rsp + 0x58]
0197f728: shr    eax, 0x1f
0197f72b: xor    al, 1
0197f72d: add    rsp, 0x40
0197f731: pop    rdi
0197f732: ret    
0197f733: lea    rcx, [rip + 0x2336e56]                   ; [0x3cb6590] meta:System.ArgumentNullException_TypeInfo
0197f73a: call   0x182f609d0
0197f73f: mov    rcx, rax
0197f742: call   0x182f60c00
0197f747: lea    rcx, [rip + 0x236e7fa]                   ; [0x3cedf48] str:'value'
0197f74e: mov    rbx, rax
0197f751: call   0x182f609d0
0197f756: mov    rdx, rax
0197f759: xor    r8d, r8d
0197f75c: mov    rcx, rbx
0197f75f: call   0x181a8b930                              ; System.ArgumentNullException$$.ctor
0197f764: lea    rcx, [rip + 0x2377dfd]                   ; [0x3cf7568] metamethod:Method$System.String.IndexOf()
0197f76b: call   0x182f609d0
0197f770: mov    rdx, rax
0197f773: mov    rcx, rbx
0197f776: call   0x182f60c10
0197f77b: int3   
0197f77c: call   0x182f60c50
0197f781: int3   
0197f782: lea    rcx, [rip + 0x2336ebf]                   ; [0x3cb6648] meta:System.ArgumentOutOfRangeException_TypeInfo
0197f789: call   0x182f609d0
0197f78e: mov    rcx, rax
0197f791: call   0x182f60c00
0197f796: lea    rcx, [rip + 0x237f2f3]                   ; [0x3cfea90] str:'Count must be positive and count must refer to a location within the string/array/collection.'
0197f79d: mov    rdi, rax
0197f7a0: call   0x182f609d0
0197f7a5: lea    rcx, [rip + 0x235197c]                   ; [0x3cd1128] str:'count'
0197f7ac: mov    rbx, rax
0197f7af: call   0x182f609d0
0197f7b4: xor    r9d, r9d
0197f7b7: mov    r8, rbx
0197f7ba: mov    rdx, rax
0197f7bd: mov    rcx, rdi
0197f7c0: call   0x181a8bdd0                              ; System.ArgumentOutOfRangeException$$.ctor
0197f7c5: lea    rcx, [rip + 0x2377d9c]                   ; [0x3cf7568] metamethod:Method$System.String.IndexOf()
0197f7cc: call   0x182f609d0
0197f7d1: mov    rdx, rax
0197f7d4: mov    rcx, rdi
0197f7d7: call   0x182f60c10
0197f7dc: int3   
0197f7dd: lea    rcx, [rip + 0x2336e64]                   ; [0x3cb6648] meta:System.ArgumentOutOfRangeException_TypeInfo
0197f7e4: call   0x182f609d0
0197f7e9: mov    rcx, rax
0197f7ec: call   0x182f60c00
0197f7f1: lea    rcx, [rip + 0x238e520]                   ; [0x3d0dd18] str:'Index was out of range. Must be non-negative and less than the size of the collection.'
0197f7f8: mov    rdi, rax
0197f7fb: call   0x182f609d0
0197f800: lea    rcx, [rip + 0x2344ac9]                   ; [0x3cc42d0] str:'startIndex'
0197f807: mov    rbx, rax
0197f80a: call   0x182f609d0
0197f80f: xor    r9d, r9d
0197f812: mov    r8, rbx
0197f815: mov    rdx, rax
0197f818: mov    rcx, rdi
0197f81b: call   0x181a8bdd0                              ; System.ArgumentOutOfRangeException$$.ctor
0197f820: lea    rcx, [rip + 0x2377d41]                   ; [0x3cf7568] metamethod:Method$System.String.IndexOf()
0197f827: call   0x182f609d0
0197f82c: mov    rdx, rax
0197f82f: mov    rcx, rdi
0197f832: call   0x182f60c10
0197f837: int3   
0197f838: int3   
