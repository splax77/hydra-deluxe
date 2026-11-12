001f2550: push   rbx
001f2552: push   rbp
001f2553: push   rsi
001f2554: push   rdi
001f2555: sub    rsp, 0x38
001f2559: cmp    byte ptr [rip + 0x3d1a59a], 0            ; [0x3f0cafa] (bss)
001f2560: mov    rsi, r9
001f2563: mov    rbp, r8
001f2566: mov    rbx, rdx
001f2569: mov    rdi, rcx
001f256c: jne    0x1801f2581
001f256e: lea    rcx, [rip + 0x3aa3ca3]                   ; [0x3c96218] meta:ʷʾʿʸʸˀˀʻʾʺʼ_TypeInfo
001f2575: call   0x182f609b0
001f257a: mov    byte ptr [rip + 0x3d1a579], 1            ; [0x3f0cafa] (bss)
001f2581: xor    edx, edx
001f2583: mov    dword ptr [rdi + 0x30], 0xffffffff
001f258a: mov    rcx, rdi
001f258d: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
001f2592: mov    byte ptr [rdi + 0x18], 1
001f2596: test   rbx, rbx
001f2599: je     0x1801f26c5
001f259f: mov    r9d, 1
001f25a5: mov    qword ptr [rsp + 0x20], 0
001f25ae: mov    r8, rsi
001f25b1: mov    rdx, rbp
001f25b4: mov    rcx, rbx
001f25b7: call   0x182204970                              ; System.IO.MemoryMappedFiles.MemoryMappedFile$$CreateViewAccessor
001f25bc: lea    rcx, [rdi + 0x10]
001f25c0: mov    qword ptr [rdi + 0x10], rax
001f25c4: mov    rdx, rax
001f25c7: call   0x182f5fc00
001f25cc: mov    rcx, qword ptr [rip + 0x3aa3c45]         ; [0x3c96218] meta:ʷʾʿʸʸˀˀʻʾʺʼ_TypeInfo
001f25d3: call   0x182f60c00
001f25d8: xor    r8d, r8d
001f25db: mov    rdx, rdi
001f25de: mov    rcx, rax
001f25e1: mov    rbx, rax
001f25e4: call   0x1803a59a0                              ; ʷʾʿʸʸˀˀʻʾʺʼ$$.ctor
001f25e9: lea    rcx, [rdi + 0x28]
001f25ed: mov    qword ptr [rdi + 0x28], rbx
001f25f1: mov    rdx, rbx
001f25f4: call   0x182f5fc00
001f25f9: mov    rcx, qword ptr [rdi + 0x10]
001f25fd: test   rcx, rcx
001f2600: je     0x1801f26c5
001f2606: xor    edx, edx
001f2608: call   0x182204f20                              ; System.IO.MemoryMappedFiles.MemoryMappedViewAccessor$$get_SafeMemoryMappedViewHandle
001f260d: test   rax, rax
001f2610: je     0x1801f26c5
001f2616: lea    rdx, [rdi + 0x20]
001f261a: xor    r8d, r8d
001f261d: mov    rcx, rax
001f2620: call   0x181a49000                              ; System.Runtime.InteropServices.SafeBuffer$$AcquirePointer
001f2625: mov    rax, qword ptr [rdi + 0x20]
001f2629: movabs rdx, 0x7fffffffffffffff
001f2633: mov    ecx, dword ptr [rax]
001f2635: mov    dword ptr [rdi + 0x30], ecx
001f2638: mov    rcx, qword ptr [rax + 8]
001f263c: mov    qword ptr [rdi + 0x58], rcx
001f2640: mov    rcx, qword ptr [rax + 0x10]
001f2644: mov    qword ptr [rdi + 0x60], rcx
001f2648: mov    rcx, qword ptr [rax + 0x18]
001f264c: add    qword ptr [rdi + 0x20], 0x20
001f2651: and    rcx, rdx
001f2654: mov    qword ptr [rdi + 0x38], rcx
001f2658: lea    rax, [rcx + 0x20]
001f265c: cmp    rax, rsi
001f265f: jg     0x1801f266a
001f2661: add    rsp, 0x38
001f2665: pop    rdi
001f2666: pop    rsi
001f2667: pop    rbp
001f2668: pop    rbx
001f2669: ret    
001f266a: lea    rcx, [rip + 0x3ac3e6f]                   ; [0x3cb64e0] meta:System.ArgumentException_TypeInfo
001f2671: call   0x182f609d0
001f2676: mov    rcx, rax
001f2679: call   0x182f60c00
001f267e: lea    rcx, [rip + 0x3abf32b]                   ; [0x3cb19b0] str:'size'
001f2685: mov    rdi, rax
001f2688: call   0x182f609d0
001f268d: lea    rcx, [rip + 0x3abf6b4]                   ; [0x3cb1d48] str:'size too small'
001f2694: mov    rbx, rax
001f2697: call   0x182f609d0
001f269c: xor    r9d, r9d
001f269f: mov    r8, rbx
001f26a2: mov    rdx, rax
001f26a5: mov    rcx, rdi
001f26a8: call   0x181a8b790                              ; System.ArgumentException$$.ctor
001f26ad: lea    rcx, [rip + 0x3aa8de4]                   ; [0x3c9b498] metamethod:Method$ʿʳʶʳʾˀʺʸʹʳʽ..ctor()
001f26b4: call   0x182f609d0
001f26b9: mov    rdx, rax
001f26bc: mov    rcx, rdi
001f26bf: call   0x182f60c10
001f26c4: int3   
001f26c5: call   0x182f60c50
001f26ca: int3   
001f26cb: int3   
