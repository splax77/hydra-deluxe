005fad90: push   rsi
005fad92: push   r13
005fad94: push   r14
005fad96: push   r15
005fad98: sub    rsp, 0x28
005fad9c: cmp    qword ptr [r9 + 0x38], 0
005fada1: mov    rsi, r9
005fada4: mov    r13, r8
005fada7: mov    r15, rdx
005fadaa: mov    r14, rcx
005fadad: jne    0x1805fadca
005fadaf: lea    rcx, [rip + 0x36ece82]                   ; [0x3ce7c38] metamethod:Method$System.ValueTuple<int, int>..ctor()
005fadb6: call   0x182f609b0
005fadbb: cmp    qword ptr [rsi + 0x38], 0
005fadc0: jne    0x1805fadca
005fadc2: mov    rcx, rsi
005fadc5: call   0x182f657d0
005fadca: cmp    r15, r13
005fadcd: jg     0x1805faf56
005fadd3: cmp    dword ptr [r14 + 8], 1
005fadd8: jl     0x1805faf56
005fadde: mov    qword ptr [rsp + 0x50], rbx
005fade3: mov    qword ptr [rsp + 0x60], rdi
005fade8: mov    qword ptr [rsp + 0x20], r12
005faded: mov    rax, qword ptr [rsi + 0x38]
005fadf1: mov    r12, qword ptr [r14]
005fadf4: mov    edi, dword ptr [r14 + 8]
005fadf8: mov    rcx, qword ptr [rax + 0x18]
005fadfc: cmp    qword ptr [rcx + 0x38], 0
005fae01: jne    0x1805fae08
005fae03: call   0x182f657d0
005fae08: xor    ebx, ebx
005fae0a: lea    r8d, [rdi - 1]
005fae0e: test   r8d, r8d
005fae11: jle    0x1805fae4e
005fae13: lea    eax, [r8 + rbx]
005fae17: cdq    
005fae18: sub    eax, edx
005fae1a: sar    eax, 1
005fae1c: cmp    eax, edi
005fae1e: jae    0x1805faf8f
005fae24: movsxd rcx, eax
005fae27: mov    rdx, qword ptr [r12 + rcx*8]
005fae2b: test   rdx, rdx
005fae2e: je     0x1805faf89
005fae34: cmp    qword ptr [rdx + 0x10], r15
005fae38: mov    ecx, eax
005fae3a: cmovl  ecx, r8d
005fae3e: inc    eax
005fae40: cmp    qword ptr [rdx + 0x10], r15
005fae44: mov    r8d, ecx
005fae47: cmovl  ebx, eax
005fae4a: cmp    ebx, ecx
005fae4c: jl     0x1805fae13
005fae4e: cmp    ebx, edi
005fae50: jae    0x1805faf8f
005fae56: movsxd rax, ebx
005fae59: mov    rcx, qword ptr [r12 + rax*8]
005fae5d: test   rcx, rcx
005fae60: je     0x1805faf89
005fae66: cmp    qword ptr [rcx + 0x10], r15
005fae6a: jl     0x1805faf3c
005fae70: cmp    ebx, -1
005fae73: je     0x1805faf3c
005fae79: mov    rax, qword ptr [rsi + 0x38]
005fae7d: mov    rcx, qword ptr [rax + 0x20]
005fae81: cmp    qword ptr [rcx + 0x38], 0
005fae86: jne    0x1805fae8d
005fae88: call   0x182f657d0
005fae8d: lea    r9d, [rdi - 1]
005fae91: mov    r8d, ebx
005fae94: cmp    ebx, r9d
005fae97: jge    0x1805faedb
005fae99: nop    dword ptr [rax]
005faea0: lea    eax, [r8 + 1]
005faea4: add    eax, r9d
005faea7: cdq    
005faea8: sub    eax, edx
005faeaa: sar    eax, 1
005faeac: cmp    eax, edi
005faeae: jae    0x1805faf8f
005faeb4: movsxd rcx, eax
005faeb7: mov    rdx, qword ptr [r12 + rcx*8]
005faebb: test   rdx, rdx
005faebe: je     0x1805faf89
005faec4: cmp    qword ptr [rdx + 0x10], r13
005faec8: lea    ecx, [rax - 1]
005faecb: cmovle ecx, r9d
005faecf: cmovle r8d, eax
005faed3: mov    r9d, ecx
005faed6: cmp    r8d, ecx
005faed9: jl     0x1805faea0
005faedb: cmp    r8d, edi
005faede: jae    0x1805faf8f
005faee4: movsxd rax, r8d
005faee7: mov    rcx, qword ptr [r12 + rax*8]
005faeeb: test   rcx, rcx
005faeee: je     0x1805faf89
005faef4: cmp    qword ptr [rcx + 0x10], r13
005faef8: jg     0x1805faf3c
005faefa: cmp    r8d, -1
005faefe: je     0x1805faf3c
005faf00: mov    r9, qword ptr [rip + 0x36ecd31]          ; [0x3ce7c38] metamethod:Method$System.ValueTuple<int, int>..ctor()
005faf07: mov    edx, ebx
005faf09: mov    qword ptr [rsp + 0x58], 0
005faf12: lea    rcx, [rsp + 0x58]
005faf17: call   0x1803363c0                              ; UnityEngine.TerrainUtils.TerrainTileCoord$$.ctor
005faf1c: mov    rax, qword ptr [rsp + 0x58]
005faf21: mov    rdi, qword ptr [rsp + 0x60]
005faf26: mov    rbx, qword ptr [rsp + 0x50]
005faf2b: mov    r12, qword ptr [rsp + 0x20]
005faf30: add    rsp, 0x28
005faf34: pop    r15
005faf36: pop    r14
005faf38: pop    r13
005faf3a: pop    rsi
005faf3b: ret    
005faf3c: mov    r9, qword ptr [rip + 0x36eccf5]          ; [0x3ce7c38] metamethod:Method$System.ValueTuple<int, int>..ctor()
005faf43: mov    edx, 0xffffffff
005faf48: mov    r8d, edx
005faf4b: mov    qword ptr [rsp + 0x58], 0
005faf54: jmp    0x1805faf12
005faf56: mov    r9, qword ptr [rip + 0x36eccdb]          ; [0x3ce7c38] metamethod:Method$System.ValueTuple<int, int>..ctor()
005faf5d: lea    rcx, [rsp + 0x58]
005faf62: mov    edx, 0xffffffff
005faf67: mov    qword ptr [rsp + 0x58], 0
005faf70: mov    r8d, edx
005faf73: call   0x1803363c0                              ; UnityEngine.TerrainUtils.TerrainTileCoord$$.ctor
005faf78: mov    rax, qword ptr [rsp + 0x58]
005faf7d: add    rsp, 0x28
005faf81: pop    r15
005faf83: pop    r14
005faf85: pop    r13
005faf87: pop    rsi
005faf88: ret    
005faf89: call   0x182f60c50
005faf8e: int3   
005faf8f: call   0x182f60c40
005faf94: int3   
005faf95: int3   
