001bd150: push   rbp
001bd152: push   rdi
001bd153: push   r12
001bd155: push   r14
001bd157: push   r15
001bd159: sub    rsp, 0x60
001bd15d: lea    rbp, [rsp + 0x20]
001bd162: cmp    byte ptr [rip + 0x3d4f95a], 0            ; [0x3f0cac3] (bss)
001bd169: mov    rdi, r8
001bd16c: mov    qword ptr [rbp + 0x70], rbx
001bd170: mov    r15, rdx
001bd173: mov    qword ptr [rbp + 0x78], rsi
001bd177: mov    rbx, r9
001bd17a: movaps xmmword ptr [rbp + 0x30], xmm6
001bd17e: mov    r12, rcx
001bd181: jne    0x1801bd1d2
001bd183: lea    rcx, [rip + 0x3ad596e]                   ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
001bd18a: call   0x182f609b0
001bd18f: lea    rcx, [rip + 0x3aecafa]                   ; [0x3ca9c90] metamethod:Method$System.Span<char>.ToString()
001bd196: call   0x182f609b0
001bd19b: lea    rcx, [rip + 0x3aec52e]                   ; [0x3ca96d0] metamethod:Method$System.Span<char>..ctor()
001bd1a2: call   0x182f609b0
001bd1a7: lea    rcx, [rip + 0x3b19f2a]                   ; [0x3cd70d8] meta:string_TypeInfo
001bd1ae: call   0x182f609b0
001bd1b3: lea    rcx, [rip + 0x3ad9366]                   ; [0x3c96520] metamethod:Method$ʼʿʺʿʹʳʹʶʹʹʼ.ʳʺˀʽˁʲʾʴʸʼʻ()
001bd1ba: call   0x182f609b0
001bd1bf: lea    rcx, [rip + 0x3ae24ca]                   ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
001bd1c6: call   0x182f609b0
001bd1cb: mov    byte ptr [rip + 0x3d4f8f1], 1            ; [0x3f0cac3] (bss)
001bd1d2: mov    rcx, qword ptr [rip + 0x3ae24b7]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
001bd1d9: xorps  xmm0, xmm0
001bd1dc: mov    qword ptr [rbp + 0x80], 0
001bd1e7: mov    qword ptr [rbp], 0
001bd1ef: movups xmmword ptr [rbp + 0x20], xmm0
001bd1f3: cmp    dword ptr [rcx + 0xe0], 0
001bd1fa: jne    0x1801bd201
001bd1fc: call   0x182f60cf0
001bd201: xor    edx, edx
001bd203: mov    rcx, rdi
001bd206: call   0x18216bb00                              ; ˀʷʿʽʽˀʿʿʷʲʳ$$ʿʸʴʾʶʺʴˁʷʴʸ
001bd20b: test   al, al
001bd20d: jne    0x1801bd2ba
001bd213: test   rdi, rdi
001bd216: je     0x1801bd340
001bd21c: movsxd r14, dword ptr [rdi + 0x10]
001bd220: mov    r8, r14
001bd223: add    r8, r8
001bd226: je     0x1801bd24e
001bd228: lea    rax, [r8 + 0xf]
001bd22c: cmp    rax, r8
001bd22f: ja     0x1801bd23b
001bd231: movabs rax, 0xffffffffffffff0
001bd23b: and    rax, 0xfffffffffffffff0
001bd23f: call   0x18302b960
001bd244: sub    rsp, rax
001bd247: lea    rsi, [rsp + 0x20]
001bd24c: jmp    0x1801bd250
001bd24e: xor    esi, esi
001bd250: xor    edx, edx
001bd252: mov    rcx, rsi
001bd255: call   0x18305d9d0
001bd25a: mov    dword ptr [rbp + 0x1c], 0
001bd261: test   r14d, r14d
001bd264: jns    0x1801bd26d
001bd266: xor    ecx, ecx
001bd268: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
001bd26d: mov    rcx, qword ptr [rip + 0x3ae241c]         ; [0x3c9f690] meta:ˀʷʿʽʽˀʿʿʷʲʳ_TypeInfo
001bd274: mov    qword ptr [rbp + 0x10], rsi
001bd278: mov    dword ptr [rbp + 0x18], r14d
001bd27c: movaps xmm6, xmmword ptr [rbp + 0x10]
001bd280: movdqa xmmword ptr [rbp + 0x20], xmm6
001bd285: cmp    dword ptr [rcx + 0xe0], 0
001bd28c: jne    0x1801bd293
001bd28e: call   0x182f60cf0
001bd293: xor    r8d, r8d
001bd296: movdqa xmmword ptr [rbp + 0x10], xmm6
001bd29b: lea    rdx, [rbp + 0x10]
001bd29f: mov    rcx, rdi
001bd2a2: call   0x1801bd9a0
001bd2a7: mov    rdx, qword ptr [rip + 0x3aec9e2]         ; [0x3ca9c90] metamethod:Method$System.Span<char>.ToString()
001bd2ae: lea    rcx, [rbp + 0x20]
001bd2b2: call   0x1809b6a30                              ; System.Span<char>$$ToString
001bd2b7: mov    rdi, rax
001bd2ba: xor    r9d, r9d
001bd2bd: lea    r8, [rbp + 0x80]
001bd2c4: mov    rdx, r15
001bd2c7: mov    rcx, r12
001bd2ca: call   0x1801bd350
001bd2cf: test   al, al
001bd2d1: je     0x1801bd309
001bd2d3: mov    rcx, qword ptr [rbp + 0x80]
001bd2da: test   rcx, rcx
001bd2dd: je     0x1801bd340
001bd2df: mov    r9, qword ptr [rip + 0x3ad5812]          ; [0x3c92af8] metamethod:Method$System.Collections.Generic.Dictionary<string, string>.TryGetValue()
001bd2e6: lea    r8, [rbp]
001bd2ea: mov    rdx, rdi
001bd2ed: call   0x1813ff9d0                              ; System.Collections.Generic.Dictionary<object, object>$$TryGetValue
001bd2f2: test   al, al
001bd2f4: je     0x1801bd309
001bd2f6: mov    rdx, qword ptr [rbp]
001bd2fa: mov    rcx, rbx
001bd2fd: mov    qword ptr [rbx], rdx
001bd300: call   0x182f5fc00
001bd305: mov    al, 1
001bd307: jmp    0x1801bd327
001bd309: mov    rax, qword ptr [rip + 0x3b19dc8]         ; [0x3cd70d8] meta:string_TypeInfo
001bd310: mov    rcx, qword ptr [rax + 0xb8]
001bd317: mov    rdx, qword ptr [rcx]
001bd31a: mov    rcx, rbx
001bd31d: mov    qword ptr [rbx], rdx
001bd320: call   0x182f5fc00
001bd325: xor    al, al
001bd327: mov    rbx, qword ptr [rbp + 0x70]
001bd32b: mov    rsi, qword ptr [rbp + 0x78]
001bd32f: movaps xmm6, xmmword ptr [rbp + 0x30]
001bd333: lea    rsp, [rbp + 0x40]
001bd337: pop    r15
001bd339: pop    r14
001bd33b: pop    r12
001bd33d: pop    rdi
001bd33e: pop    rbp
001bd33f: ret    
001bd340: call   0x182f60c50
001bd345: int3   
001bd346: int3   
