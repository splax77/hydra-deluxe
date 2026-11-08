002b84a0: mov    qword ptr [rsp + 8], rcx
002b84a5: push   rbx
002b84a6: push   rsi
002b84a7: push   rdi
002b84a8: push   r12
002b84aa: push   r13
002b84ac: push   r14
002b84ae: push   r15
002b84b0: sub    rsp, 0x90
002b84b7: mov    rbx, rdx
002b84ba: mov    rdi, rcx
002b84bd: cmp    byte ptr [rip + 0x3c553ce], 0            ; [0x3f0d892] (bss)
002b84c4: jne    0x1802b8545
002b84c6: lea    rcx, [rip + 0x3a0398b]                   ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
002b84cd: call   0x182f609b0
002b84d2: lea    rcx, [rip + 0x39e4e0f]                   ; [0x3c9d2e8] meta:byte[]_TypeInfo
002b84d9: call   0x182f609b0
002b84de: lea    rcx, [rip + 0x3a457eb]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b84e5: call   0x182f609b0
002b84ea: lea    rcx, [rip + 0x39f2f37]                   ; [0x3cab428] meta:ʳˁʿʶʵʺʻʺʿʽʴ[]_TypeInfo
002b84f1: call   0x182f609b0
002b84f6: lea    rcx, [rip + 0x3a49d1b]                   ; [0x3d02218] metamethod:Method$ʵʵʹʲʻʾʼʺˀʳʴ.ʽʹʵʾʺʷʾʸˀʾʻ<ulong>()
002b84fd: call   0x182f609b0
002b8502: lea    rcx, [rip + 0x39da097]                   ; [0x3c925a0] metamethod:Method$ʺʻʻˁˀʹʿʲʻʴʿ..ctor()
002b8509: call   0x182f609b0
002b850e: lea    rcx, [rip + 0x39e41ab]                   ; [0x3c9c6c0] meta:ʽʼʲʴʲˀʸʻʸʲʹ_TypeInfo
002b8515: call   0x182f609b0
002b851a: lea    rcx, [rip + 0x39e5987]                   ; [0x3c9dea8] meta:ʿʳʶʳʾˀʺʸʹʳʽ_TypeInfo
002b8521: call   0x182f609b0
002b8526: lea    rcx, [rip + 0x39e70ab]                   ; [0x3c9f5d8] meta:ˀʷʶʵʼʹʴˁʸˀʼ_TypeInfo
002b852d: call   0x182f609b0
002b8532: lea    rcx, [rip + 0x39f004f]                   ; [0x3ca8588] meta:ulong[]_TypeInfo
002b8539: call   0x182f609b0
002b853e: mov    byte ptr [rip + 0x3c5534d], 1            ; [0x3f0d892] (bss)
002b8545: xor    esi, esi
002b8547: mov    qword ptr [rsp + 0x40], rsi
002b854c: mov    dword ptr [rsp + 0xd8], esi
002b8553: mov    dword ptr [rsp + 0x80], esi
002b855a: xor    edx, edx
002b855c: mov    rcx, rdi
002b855f: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
002b8564: nop    
002b8565: xor    edx, edx
002b8567: mov    rcx, rbx
002b856a: call   0x181b1b5b0                              ; System.IO.File$$OpenRead
002b856f: mov    qword ptr [rsp + 0x30], rsi
002b8574: mov    byte ptr [rsp + 0x28], sil
002b8579: mov    dword ptr [rsp + 0x20], esi
002b857d: mov    r9d, 1
002b8583: xor    r8d, r8d
002b8586: xor    edx, edx
002b8588: mov    rcx, rax
002b858b: call   0x1822045e0                              ; System.IO.MemoryMappedFiles.MemoryMappedFile$$CreateFromFile
002b8590: mov    qword ptr [rdi + 0x10], rax
002b8594: lea    rcx, [rdi + 0x10]
002b8598: mov    rdx, rax
002b859b: call   0x182f5fc00
002b85a0: mov    rcx, qword ptr [rdi + 0x10]
002b85a4: test   rcx, rcx
002b85a7: je     0x1802b8e52
002b85ad: mov    qword ptr [rsp + 0x20], rsi
002b85b2: mov    r9d, 1
002b85b8: xor    r8d, r8d
002b85bb: xor    edx, edx
002b85bd: call   0x182204970                              ; System.IO.MemoryMappedFiles.MemoryMappedFile$$CreateViewAccessor
002b85c2: mov    qword ptr [rsp + 0x40], rax
002b85c7: mov    qword ptr [rsp + 0x68], rsi
002b85cc: lea    rbx, [rsp + 0x40]
002b85d1: mov    qword ptr [rsp + 0x70], rbx
002b85d6: mov    rcx, qword ptr [rsp + 0x40]
002b85db: test   rcx, rcx
002b85de: je     0x1802b8ea3
002b85e4: mov    r12, qword ptr [rcx + 0x20]
002b85e8: mov    rax, qword ptr [rip + 0x3a03869]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
002b85ef: cmp    dword ptr [rax + 0xe0], esi
002b85f5: jne    0x1802b860b
002b85f7: mov    rcx, rax
002b85fa: call   0x182f60cf0
002b85ff: mov    rcx, qword ptr [rsp + 0x40]
002b8604: mov    rax, qword ptr [rip + 0x3a0384d]         ; [0x3cbbe58] meta:BassAudioManager_TypeInfo
002b860b: mov    rax, qword ptr [rax + 0xb8]
002b8612: mov    rdx, qword ptr [rax + 0x10]
002b8616: test   rdx, rdx
002b8619: je     0x1802b8e9e
002b861f: test   rcx, rcx
002b8622: je     0x1802b8e99
002b8628: mov    r13d, dword ptr [rdx + 0x18]
002b862c: xor    r8d, r8d
002b862f: xor    edx, edx
002b8631: call   0x181b0a960                              ; System.IO.UnmanagedMemoryAccessor$$ReadUInt64
002b8636: mov    r14, rax
002b8639: mov    rsi, rax
002b863c: shr    rsi, 0x20
002b8640: bswap  esi
002b8642: bswap  r14d
002b8645: mov    rcx, qword ptr [rsp + 0x40]
002b864a: test   rcx, rcx
002b864d: je     0x1802b8e94
002b8653: xor    r8d, r8d
002b8656: mov    edx, 8
002b865b: call   0x181b0a960                              ; System.IO.UnmanagedMemoryAccessor$$ReadUInt64
002b8660: mov    r15, rax
002b8663: mov    rcx, qword ptr [rip + 0x39e6f6e]         ; [0x3c9f5d8] meta:ˀʷʶʵʼʹʴˁʸˀʼ_TypeInfo
002b866a: cmp    dword ptr [rcx + 0xe0], 0
002b8671: jne    0x1802b8678
002b8673: call   0x182f60cf0
002b8678: mov    edx, r14d
002b867b: shl    rdx, 0x20
002b867f: mov    eax, esi
002b8681: or     rdx, rax
002b8684: xor    r8d, r8d
002b8687: mov    rcx, r15
002b868a: call   0x18214efd0                              ; ˀʷʶʵʼʹʴˁʸˀʼ$$ʷʹʴʳʾʹʳʼˁʴʵ
002b868f: mov    r15, rax
002b8692: cmp    rax, r12
002b8695: ja     0x1802b8d41
002b869b: mov    rcx, qword ptr [rip + 0x39e401e]         ; [0x3c9c6c0] meta:ʽʼʲʴʲˀʸʻʸʲʹ_TypeInfo
002b86a2: call   0x182f60c00
002b86a7: mov    rsi, rax
002b86aa: xor    edx, edx
002b86ac: mov    rcx, rax
002b86af: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
002b86b4: mov    qword ptr [rdi + 0x18], rsi
002b86b8: lea    rcx, [rdi + 0x18]
002b86bc: mov    rdx, rsi
002b86bf: call   0x182f5fc00
002b86c4: movabs rax, 0x7fffffffffffffff
002b86ce: cmp    r15, rax
002b86d1: ja     0x1802b8d9e
002b86d7: mov    edx, r15d
002b86da: mov    rcx, qword ptr [rip + 0x39e4c07]         ; [0x3c9d2e8] meta:byte[]_TypeInfo
002b86e1: call   0x182f5fc80
002b86e6: mov    rsi, rax
002b86e9: mov    r14, qword ptr [rsp + 0x40]
002b86ee: cmp    byte ptr [rip + 0x3c55196], 0            ; [0x3f0d88b] (bss)
002b86f5: jne    0x1802b870a
002b86f7: lea    rcx, [rip + 0x39f83ba]                   ; [0x3cb0ab8] metamethod:Method$System.Runtime.CompilerServices.Unsafe.AsRef<byte>()
002b86fe: call   0x182f609b0
002b8703: mov    byte ptr [rip + 0x3c55181], 1            ; [0x3f0d88b] (bss)
002b870a: mov    qword ptr [rsp + 0xe8], 0
002b8716: test   r14, r14
002b8719: je     0x1802b8e8f
002b871f: xor    edx, edx
002b8721: mov    rcx, r14
002b8724: call   0x182204f20                              ; System.IO.MemoryMappedFiles.MemoryMappedViewAccessor$$get_SafeMemoryMappedViewHandle
002b8729: test   rax, rax
002b872c: je     0x1802b8e8a
002b8732: xor    r8d, r8d
002b8735: lea    rdx, [rsp + 0xe8]
002b873d: mov    rcx, rax
002b8740: call   0x181a49000                              ; System.Runtime.InteropServices.SafeBuffer$$AcquirePointer
002b8745: test   rsi, rsi
002b8748: je     0x1802b8e85
002b874e: cmp    dword ptr [rsi + 0x18], 0
002b8752: jbe    0x1802b8db5
002b8758: mov    r8d, r15d
002b875b: mov    rdx, qword ptr [rsp + 0xe8]
002b8763: add    rdx, 0x10
002b8767: lea    rcx, [rsi + 0x20]
002b876b: call   0x18305dd90
002b8770: xor    edx, edx
002b8772: mov    rcx, r14
002b8775: call   0x182204f20                              ; System.IO.MemoryMappedFiles.MemoryMappedViewAccessor$$get_SafeMemoryMappedViewHandle
002b877a: test   rax, rax
002b877d: je     0x1802b8e80
002b8783: xor    edx, edx
002b8785: mov    rcx, rax
002b8788: call   0x181a492a0                              ; System.Runtime.InteropServices.SafeBuffer$$ReleasePointer
002b878d: xor    edx, edx
002b878f: mov    rcx, rsi
002b8792: call   0x1802e0a10                              ; ʲʲʹʻʿʳʿʻʼʴʾ$$ʺʼʳʵʻʺʻʿʹʵʻ
002b8797: mov    rsi, rax
002b879a: mov    dword ptr [rsp + 0xd8], 0
002b87a5: test   rax, rax
002b87a8: je     0x1802b8e7b
002b87ae: cmp    dword ptr [rax + 0x18], 0
002b87b2: jbe    0x1802b8dba
002b87b8: mov    eax, dword ptr [rax + 0x20]
002b87bb: mov    dword ptr [rsp + 0xd8], 4
002b87c6: mov    dword ptr [rdi + 0x40], eax
002b87c9: cmp    eax, 0x1346234
002b87ce: jne    0x1802b8d26
002b87d4: mov    rcx, qword ptr [rdi + 0x18]
002b87d8: test   rcx, rcx
002b87db: je     0x1802b8e76
002b87e1: xor    r9d, r9d
002b87e4: lea    r8, [rsp + 0xd8]
002b87ec: mov    rdx, rsi
002b87ef: call   0x1801cf270                              ; ʽʼʲʴʲˀʸʻʸʲʹ$$ʼˀʸʳʵˁʵʽˀʿʴ
002b87f4: movsxd rcx, dword ptr [rsp + 0xd8]
002b87fc: cmp    ecx, dword ptr [rsi + 0x18]
002b87ff: jae    0x1802b8dbf
002b8805: mov    r14, qword ptr [rcx + rsi + 0x20]
002b880a: mov    qword ptr [rsp + 0x48], r14
002b880f: add    ecx, 8
002b8812: mov    dword ptr [rsp + 0xd8], ecx
002b8819: cmp    r14, r12
002b881c: jg     0x1802b8d0b
002b8822: cmp    ecx, dword ptr [rsi + 0x18]
002b8825: jae    0x1802b8dc4
002b882b: movsxd rax, ecx
002b882e: mov    edx, dword ptr [rax + rsi + 0x20]
002b8832: mov    dword ptr [rsp + 0xe8], edx
002b8839: add    ecx, 4
002b883c: mov    dword ptr [rsp + 0xd8], ecx
002b8843: cmp    edx, r13d
002b8846: jg     0x1802b8ced
002b884c: lea    edx, [rcx + 1]
002b884f: mov    dword ptr [rsp + 0xd8], edx
002b8856: cmp    ecx, dword ptr [rsi + 0x18]
002b8859: jae    0x1802b8dc9
002b885f: movsxd rax, ecx
002b8862: movzx  r8d, byte ptr [rax + rsi + 0x20]
002b8868: mov    byte ptr [rdi + 0x44], r8b
002b886c: cmp    edx, dword ptr [rsi + 0x18]
002b886f: jae    0x1802b8dce
002b8875: movsxd rax, edx
002b8878: mov    r9d, dword ptr [rax + rsi + 0x20]
002b887d: add    edx, 4
002b8880: mov    dword ptr [rsp + 0xd8], edx
002b8887: mov    dword ptr [rdi + 0x5c], r9d
002b888b: cmp    r9d, 0x10000
002b8892: ja     0x1802b8ccf
002b8898: test   r8b, 2
002b889c: je     0x1802b88bb
002b889e: cmp    edx, dword ptr [rsi + 0x18]
002b88a1: jae    0x1802b8dd3
002b88a7: movsxd rax, edx
002b88aa: mov    ecx, dword ptr [rax + rsi + 0x20]
002b88ae: add    edx, 4
002b88b1: mov    dword ptr [rsp + 0xd8], edx
002b88b8: mov    dword ptr [rdi + 0x50], ecx
002b88bb: test   r8b, 1
002b88bf: je     0x1802b88de
002b88c1: cmp    edx, dword ptr [rsi + 0x18]
002b88c4: jae    0x1802b8dd8
002b88ca: movsxd rax, edx
002b88cd: mov    ecx, dword ptr [rax + rsi + 0x20]
002b88d1: add    edx, 4
002b88d4: mov    dword ptr [rsp + 0xd8], edx
002b88db: mov    dword ptr [rdi + 0x54], ecx
002b88de: mov    edx, r9d
002b88e1: mov    rcx, qword ptr [rip + 0x39efca0]         ; [0x3ca8588] meta:ulong[]_TypeInfo
002b88e8: call   0x182f5fc80
002b88ed: mov    qword ptr [rdi + 0x48], rax
002b88f1: lea    rcx, [rdi + 0x48]
002b88f5: mov    rdx, rax
002b88f8: call   0x182f5fc00
002b88fd: cmp    dword ptr [rdi + 0x5c], 0
002b8901: jle    0x1802b8971
002b8903: mov    rax, qword ptr [rip + 0x3a4990e]         ; [0x3d02218] metamethod:Method$ʵʵʹʲʻʾʼʺˀʳʴ.ʽʹʵʾʺʷʾʸˀʾʻ<ulong>()
002b890a: mov    qword ptr [rsp + 0x20], rax
002b890f: mov    r9d, dword ptr [rdi + 0x5c]
002b8913: mov    r8, qword ptr [rdi + 0x48]
002b8917: lea    rdx, [rsp + 0xd8]
002b891f: mov    rcx, rsi
002b8922: call   0x1802aa5e0                              ; ʵʵʹʲʻʾʼʺˀʳʴ$$ʽʹʵʾʺʷʾʸˀʾʻ<ulong>
002b8927: mov    rdx, qword ptr [rdi + 0x48]
002b892b: nop    dword ptr [rax + rax]
002b8930: xor    ecx, ecx
002b8932: test   rdx, rdx
002b8935: je     0x1802b8de2
002b893b: cmp    ecx, dword ptr [rdx + 0x18]
002b893e: jge    0x1802b8971
002b8940: jae    0x1802b8ddd
002b8946: movsxd rax, ecx
002b8949: cmp    qword ptr [rdx + rax*8 + 0x20], r12
002b894e: ja     0x1802b8954
002b8950: inc    ecx
002b8952: jmp    0x1802b893b
002b8954: cmp    qword ptr [rbx], 0
002b8958: je     0x1802b896c
002b895a: xor    ecx, ecx
002b895c: mov    r8, qword ptr [rbx]
002b895f: mov    rdx, qword ptr [rip + 0x3a4536a]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8966: call   0x182c42550
002b896b: nop    
002b896c: jmp    0x1802b8d8b
002b8971: mov    r13d, dword ptr [rsp + 0xe8]
002b8979: mov    edx, r13d
002b897c: mov    rcx, qword ptr [rip + 0x39efc05]         ; [0x3ca8588] meta:ulong[]_TypeInfo
002b8983: call   0x182f5fc80
002b8988: mov    qword ptr [rdi + 0x28], rax
002b898c: lea    rcx, [rdi + 0x28]
002b8990: mov    rdx, rax
002b8993: call   0x182f5fc00
002b8998: mov    rax, qword ptr [rip + 0x3a49879]         ; [0x3d02218] metamethod:Method$ʵʵʹʲʻʾʼʺˀʳʴ.ʽʹʵʾʺʷʾʸˀʾʻ<ulong>()
002b899f: mov    qword ptr [rsp + 0x20], rax
002b89a4: mov    r9d, r13d
002b89a7: mov    r8, qword ptr [rdi + 0x28]
002b89ab: lea    rdx, [rsp + 0xd8]
002b89b3: mov    rcx, rsi
002b89b6: call   0x1802aa5e0                              ; ʵʵʹʲʻʾʼʺˀʳʴ$$ʽʹʵʾʺʷʾʸˀʾʻ<ulong>
002b89bb: mov    rdx, qword ptr [rdi + 0x28]
002b89bf: nop    
002b89c0: xor    ecx, ecx
002b89c2: test   rdx, rdx
002b89c5: je     0x1802b8e71
002b89cb: cmp    ecx, dword ptr [rdx + 0x18]
002b89ce: jge    0x1802b8a01
002b89d0: jae    0x1802b8de7
002b89d6: movsxd rax, ecx
002b89d9: cmp    qword ptr [rdx + rax*8 + 0x20], r12
002b89de: ja     0x1802b89e4
002b89e0: inc    ecx
002b89e2: jmp    0x1802b89cb
002b89e4: cmp    qword ptr [rbx], 0
002b89e8: je     0x1802b89fc
002b89ea: xor    ecx, ecx
002b89ec: mov    r8, qword ptr [rbx]
002b89ef: mov    rdx, qword ptr [rip + 0x3a452da]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b89f6: call   0x182c42550
002b89fb: nop    
002b89fc: jmp    0x1802b8d8b
002b8a01: mov    edx, r13d
002b8a04: mov    rcx, qword ptr [rip + 0x39efb7d]         ; [0x3ca8588] meta:ulong[]_TypeInfo
002b8a0b: call   0x182f5fc80
002b8a10: mov    qword ptr [rdi + 0x30], rax
002b8a14: lea    rcx, [rdi + 0x30]
002b8a18: mov    rdx, rax
002b8a1b: call   0x182f5fc00
002b8a20: mov    rax, qword ptr [rip + 0x3a497f1]         ; [0x3d02218] metamethod:Method$ʵʵʹʲʻʾʼʺˀʳʴ.ʽʹʵʾʺʷʾʸˀʾʻ<ulong>()
002b8a27: mov    qword ptr [rsp + 0x20], rax
002b8a2c: mov    r9d, r13d
002b8a2f: mov    r8, qword ptr [rdi + 0x30]
002b8a33: lea    rdx, [rsp + 0xd8]
002b8a3b: mov    rcx, rsi
002b8a3e: call   0x1802aa5e0                              ; ʵʵʹʲʻʾʼʺˀʳʴ$$ʽʹʵʾʺʷʾʸˀʾʻ<ulong>
002b8a43: mov    rdx, qword ptr [rdi + 0x30]
002b8a47: nop    word ptr [rax + rax]
002b8a50: xor    ecx, ecx
002b8a52: test   rdx, rdx
002b8a55: je     0x1802b8e6c
002b8a5b: cmp    ecx, dword ptr [rdx + 0x18]
002b8a5e: jge    0x1802b8a91
002b8a60: jae    0x1802b8dec
002b8a66: movsxd rax, ecx
002b8a69: cmp    qword ptr [rdx + rax*8 + 0x20], r12
002b8a6e: ja     0x1802b8a74
002b8a70: inc    ecx
002b8a72: jmp    0x1802b8a5b
002b8a74: cmp    qword ptr [rbx], 0
002b8a78: je     0x1802b8a8c
002b8a7a: xor    ecx, ecx
002b8a7c: mov    r8, qword ptr [rbx]
002b8a7f: mov    rdx, qword ptr [rip + 0x3a4524a]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8a86: call   0x182c42550
002b8a8b: nop    
002b8a8c: jmp    0x1802b8d8b
002b8a91: mov    r12, qword ptr [rsp + 0x48]
002b8a96: mov    edx, r12d
002b8a99: mov    rcx, qword ptr [rip + 0x39e4848]         ; [0x3c9d2e8] meta:byte[]_TypeInfo
002b8aa0: call   0x182f5fc80
002b8aa5: mov    qword ptr [rdi + 0x20], rax
002b8aa9: lea    rcx, [rdi + 0x20]
002b8aad: mov    rdx, rax
002b8ab0: call   0x182f5fc00
002b8ab5: mov    r14, qword ptr [rsp + 0x40]
002b8aba: mov    rsi, qword ptr [rdi + 0x20]
002b8abe: cmp    byte ptr [rip + 0x3c54dc6], 0            ; [0x3f0d88b] (bss)
002b8ac5: jne    0x1802b8ada
002b8ac7: lea    rcx, [rip + 0x39f7fea]                   ; [0x3cb0ab8] metamethod:Method$System.Runtime.CompilerServices.Unsafe.AsRef<byte>()
002b8ace: call   0x182f609b0
002b8ad3: mov    byte ptr [rip + 0x3c54db1], 1            ; [0x3f0d88b] (bss)
002b8ada: mov    qword ptr [rsp + 0x48], 0
002b8ae3: test   r14, r14
002b8ae6: je     0x1802b8e67
002b8aec: xor    edx, edx
002b8aee: mov    rcx, r14
002b8af1: call   0x182204f20                              ; System.IO.MemoryMappedFiles.MemoryMappedViewAccessor$$get_SafeMemoryMappedViewHandle
002b8af6: test   rax, rax
002b8af9: je     0x1802b8e62
002b8aff: xor    r8d, r8d
002b8b02: lea    rdx, [rsp + 0x48]
002b8b07: mov    rcx, rax
002b8b0a: call   0x181a49000                              ; System.Runtime.InteropServices.SafeBuffer$$AcquirePointer
002b8b0f: test   rsi, rsi
002b8b12: je     0x1802b8e5d
002b8b18: cmp    dword ptr [rsi + 0x18], 0
002b8b1c: jbe    0x1802b8df1
002b8b22: movsxd r15, r15d
002b8b25: add    r15, 0x10
002b8b29: mov    r8d, r12d
002b8b2c: mov    rdx, qword ptr [rsp + 0x48]
002b8b31: add    rdx, r15
002b8b34: lea    rcx, [rsi + 0x20]
002b8b38: call   0x18305dd90
002b8b3d: xor    edx, edx
002b8b3f: mov    rcx, r14
002b8b42: call   0x182204f20                              ; System.IO.MemoryMappedFiles.MemoryMappedViewAccessor$$get_SafeMemoryMappedViewHandle
002b8b47: test   rax, rax
002b8b4a: je     0x1802b8e58
002b8b50: xor    edx, edx
002b8b52: mov    rcx, rax
002b8b55: call   0x181a492a0                              ; System.Runtime.InteropServices.SafeBuffer$$ReleasePointer
002b8b5a: movsxd r14, r12d
002b8b5d: add    r14, r15
002b8b60: xor    edx, edx
002b8b62: mov    rcx, qword ptr [rdi + 0x20]
002b8b66: call   0x1802e0a10                              ; ʲʲʹʻʿʳʿʻʼʴʾ$$ʺʼʳʵʻʺʻʿʹʵʻ
002b8b6b: mov    qword ptr [rdi + 0x20], rax
002b8b6f: lea    rcx, [rdi + 0x20]
002b8b73: mov    rdx, rax
002b8b76: call   0x182f5fc00
002b8b7b: mov    dword ptr [rdi + 0x58], r14d
002b8b7f: xor    ecx, ecx
002b8b81: cmp    ecx, dword ptr [rdi + 0x5c]
002b8b84: jge    0x1802b8ba8
002b8b86: mov    rdx, qword ptr [rdi + 0x48]
002b8b8a: test   rdx, rdx
002b8b8d: je     0x1802b8dfb
002b8b93: cmp    ecx, dword ptr [rdx + 0x18]
002b8b96: jae    0x1802b8df6
002b8b9c: movsxd rax, ecx
002b8b9f: add    r14, qword ptr [rdx + rax*8 + 0x20]
002b8ba4: inc    ecx
002b8ba6: jmp    0x1802b8b81
002b8ba8: mov    edx, r13d
002b8bab: mov    rcx, qword ptr [rip + 0x39f2876]         ; [0x3cab428] meta:ʳˁʿʶʵʺʻʺʿʽʴ[]_TypeInfo
002b8bb2: call   0x182f5fc80
002b8bb7: mov    qword ptr [rdi + 0x38], rax
002b8bbb: lea    rcx, [rdi + 0x38]
002b8bbf: mov    rdx, rax
002b8bc2: call   0x182f5fc00
002b8bc7: xor    esi, esi
002b8bc9: nop    dword ptr [rax]
002b8bd0: cmp    esi, r13d
002b8bd3: jge    0x1802b8cb2
002b8bd9: mov    rcx, qword ptr [rdi + 0x30]
002b8bdd: test   rcx, rcx
002b8be0: je     0x1802b8e4c
002b8be6: cmp    esi, dword ptr [rcx + 0x18]
002b8be9: jae    0x1802b8e00
002b8bef: movsxd rax, esi
002b8bf2: mov    r13, qword ptr [rcx + rax*8 + 0x20]
002b8bf7: movabs rax, 0x7fffffffffffffff
002b8c01: cmp    r13, rax
002b8c04: ja     0x1802b8e05
002b8c0a: mov    r15, qword ptr [rdi + 0x38]
002b8c0e: mov    rax, qword ptr [rdi + 0x10]
002b8c12: mov    qword ptr [rsp + 0x48], rax
002b8c17: mov    rcx, qword ptr [rip + 0x39e528a]         ; [0x3c9dea8] meta:ʿʳʶʳʾˀʺʸʹʳʽ_TypeInfo
002b8c1e: call   0x182f60c00
002b8c23: mov    r12, rax
002b8c26: mov    qword ptr [rsp + 0x20], 0
002b8c2f: mov    r9, r13
002b8c32: mov    r8, r14
002b8c35: mov    rdx, qword ptr [rsp + 0x48]
002b8c3a: mov    rcx, rax
002b8c3d: call   0x1801f2550                              ; ʿʳʶʳʾˀʺʸʹʳʽ$$.ctor
002b8c42: test   r15, r15
002b8c45: je     0x1802b8e47
002b8c4b: test   r12, r12
002b8c4e: je     0x1802b8c68
002b8c50: mov    rdx, qword ptr [r15]
002b8c53: mov    rdx, qword ptr [rdx + 0x40]
002b8c57: mov    rcx, r12
002b8c5a: call   0x182f5fc20
002b8c5f: test   rax, rax
002b8c62: je     0x1802b8e1c
002b8c68: cmp    esi, dword ptr [r15 + 0x18]
002b8c6c: jae    0x1802b8e2b
002b8c72: movsxd rax, esi
002b8c75: mov    qword ptr [r15 + rax*8 + 0x20], r12
002b8c7a: movsxd rax, esi
002b8c7d: add    rax, 4
002b8c81: lea    rcx, [r15 + rax*8]
002b8c85: mov    rdx, r12
002b8c88: call   0x182f5fc00
002b8c8d: mov    rdx, r13
002b8c90: mov    rcx, r14
002b8c93: call   0x182c4ba50
002b8c98: test   al, al
002b8c9a: jne    0x1802b8e30
002b8ca0: add    r14, r13
002b8ca3: inc    esi
002b8ca5: mov    r13d, dword ptr [rsp + 0xe8]
002b8cad: jmp    0x1802b8bd0
002b8cb2: cmp    qword ptr [rbx], 0
002b8cb6: je     0x1802b8cca
002b8cb8: xor    ecx, ecx
002b8cba: mov    r8, qword ptr [rbx]
002b8cbd: mov    rdx, qword ptr [rip + 0x3a4500c]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8cc4: call   0x182c42550
002b8cc9: nop    
002b8cca: jmp    0x1802b8d8b
002b8ccf: mov    r8, qword ptr [rsp + 0x40]
002b8cd4: test   r8, r8
002b8cd7: je     0x1802b8ce8
002b8cd9: xor    ecx, ecx
002b8cdb: mov    rdx, qword ptr [rip + 0x3a44fee]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8ce2: call   0x182c42550
002b8ce7: nop    
002b8ce8: jmp    0x1802b8d8b
002b8ced: mov    r8, qword ptr [rsp + 0x40]
002b8cf2: test   r8, r8
002b8cf5: je     0x1802b8d06
002b8cf7: xor    ecx, ecx
002b8cf9: mov    rdx, qword ptr [rip + 0x3a44fd0]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8d00: call   0x182c42550
002b8d05: nop    
002b8d06: jmp    0x1802b8d8b
002b8d0b: mov    r8, qword ptr [rsp + 0x40]
002b8d10: test   r8, r8
002b8d13: je     0x1802b8d24
002b8d15: xor    ecx, ecx
002b8d17: mov    rdx, qword ptr [rip + 0x3a44fb2]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8d1e: call   0x182c42550
002b8d23: nop    
002b8d24: jmp    0x1802b8d8b
002b8d26: mov    r8, qword ptr [rsp + 0x40]
002b8d2b: test   r8, r8
002b8d2e: je     0x1802b8d3f
002b8d30: xor    ecx, ecx
002b8d32: mov    rdx, qword ptr [rip + 0x3a44f97]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8d39: call   0x182c42550
002b8d3e: nop    
002b8d3f: jmp    0x1802b8d8b
002b8d41: mov    r8, qword ptr [rsp + 0x40]
002b8d46: test   r8, r8
002b8d49: je     0x1802b8d5a
002b8d4b: xor    ecx, ecx
002b8d4d: mov    rdx, qword ptr [rip + 0x3a44f7c]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8d54: call   0x182c42550
002b8d59: nop    
002b8d5a: jmp    0x1802b8d8b
002b8d5c: mov    r8, qword ptr [rsp + 0x70]
002b8d61: cmp    qword ptr [r8], 0
002b8d65: je     0x1802b8d78
002b8d67: xor    ecx, ecx
002b8d69: mov    r8, qword ptr [r8]
002b8d6c: mov    rdx, qword ptr [rip + 0x3a44f5d]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002b8d73: call   0x182c42550
002b8d78: mov    rcx, qword ptr [rsp + 0x68]
002b8d7d: test   rcx, rcx
002b8d80: jne    0x1802b8ea9
002b8d86: jmp    0x1802b8cca
002b8d8b: add    rsp, 0x90
002b8d92: pop    r15
002b8d94: pop    r14
002b8d96: pop    r13
002b8d98: pop    r12
002b8d9a: pop    rdi
002b8d9b: pop    rsi
002b8d9c: pop    rbx
002b8d9d: ret    
002b8d9e: mov    rbx, qword ptr [rip + 0x39d97fb]         ; [0x3c925a0] metamethod:Method$ʺʻʻˁˀʹʿʲʻʴʿ..ctor()
002b8da5: call   0x182f60790
002b8daa: mov    rdx, rbx
002b8dad: mov    rcx, rax
002b8db0: call   0x182f60c10
002b8db5: call   0x182f60c40
002b8dba: call   0x182f60c40
002b8dbf: call   0x182f60c40
002b8dc4: call   0x182f60c40
002b8dc9: call   0x182f60c40
002b8dce: call   0x182f60c40
002b8dd3: call   0x182f60c40
002b8dd8: call   0x182f60c40
002b8ddd: call   0x182f60c40
002b8de2: call   0x182f60c50
002b8de7: call   0x182f60c40
002b8dec: call   0x182f60c40
002b8df1: call   0x182f60c40
002b8df6: call   0x182f60c40
002b8dfb: call   0x182f60c50
002b8e00: call   0x182f60c40
002b8e05: mov    rbx, qword ptr [rip + 0x39d9794]         ; [0x3c925a0] metamethod:Method$ʺʻʻˁˀʹʿʲʻʴʿ..ctor()
002b8e0c: call   0x182f60790
002b8e11: mov    rdx, rbx
002b8e14: mov    rcx, rax
002b8e17: call   0x182f60c10
002b8e1c: call   0x182f604f0
002b8e21: xor    edx, edx
002b8e23: mov    rcx, rax
002b8e26: call   0x182f60c10
002b8e2b: call   0x182f60c40
002b8e30: mov    rbx, qword ptr [rip + 0x39d9769]         ; [0x3c925a0] metamethod:Method$ʺʻʻˁˀʹʿʲʻʴʿ..ctor()
002b8e37: call   0x182f60790
002b8e3c: mov    rdx, rbx
002b8e3f: mov    rcx, rax
002b8e42: call   0x182f60c10
002b8e47: call   0x182f60c50
002b8e4c: call   0x182f60c50
002b8e51: nop    
002b8e52: call   0x182f60c50
002b8e57: nop    
002b8e58: call   0x182f60c50
002b8e5d: call   0x182f60c50
002b8e62: call   0x182f60c50
002b8e67: call   0x182f60c50
002b8e6c: call   0x182f60c50
002b8e71: call   0x182f60c50
002b8e76: call   0x182f60c50
002b8e7b: call   0x182f60c50
002b8e80: call   0x182f60c50
002b8e85: call   0x182f60c50
002b8e8a: call   0x182f60c50
002b8e8f: call   0x182f60c50
002b8e94: call   0x182f60c50
002b8e99: call   0x182f60c50
002b8e9e: call   0x182f60c50
002b8ea3: call   0x182f60c50
002b8ea8: nop    
002b8ea9: call   0x182f60ce0
002b8eae: nop    
002b8eaf: xor    edx, edx
002b8eb1: mov    rcx, qword ptr [rsp + 0xd0]
002b8eb9: call   0x1802b56c0                              ; ʺʻʻˁˀʹʿʲʻʴʿ$$Dispose
002b8ebe: lea    rcx, [rsp + 0x78]
002b8ec3: call   0x182c41290
002b8ec8: mov    rcx, rax
002b8ecb: call   0x182f60ce0
002b8ed0: int3   
002b8ed1: int3   
