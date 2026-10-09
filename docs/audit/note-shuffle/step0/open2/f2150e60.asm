02150e60: push   rbx
02150e62: push   rdi
02150e63: push   r15
02150e65: sub    rsp, 0x70
02150e69: cmp    byte ptr [rip + 0x1dc88ed], 0            ; [0x3f1975d] (bss)
02150e70: mov    rbx, r8
02150e73: mov    edi, edx
02150e75: mov    r15, rcx
02150e78: jne    0x182150ea5
02150e7a: lea    rcx, [rip + 0x1bbdc5f]                   ; [0x3d0eae0] meta:long_TypeInfo
02150e81: call   0x182f609b0
02150e86: lea    rcx, [rip + 0x1b43b4b]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02150e8d: call   0x182f609b0
02150e92: lea    rcx, [rip + 0x1b5a16f]                   ; [0x3cab008] str:'The sync event occuring at offset {0} is out of order. It comes after the sync event at {1} which is illegal.'
02150e99: call   0x182f609b0
02150e9e: mov    byte ptr [rip + 0x1dc88b8], 1            ; [0x3f1975d] (bss)
02150ea5: mov    rax, qword ptr [rbx]
02150ea8: mov    qword ptr [rsp + 0x90], rbp
02150eb0: mov    qword ptr [rsp + 0x98], rsi
02150eb8: mov    qword ptr [rsp + 0xa8], r12
02150ec0: mov    qword ptr [rsp + 0x68], r13
02150ec5: mov    qword ptr [rsp + 0x60], r14
02150eca: movaps xmmword ptr [rsp + 0x50], xmm6
02150ecf: test   rax, rax
02150ed2: je     0x182151259
02150ed8: mov    r13d, dword ptr [r15 + 8]
02150edc: cmp    edi, r13d
02150edf: jae    0x18215125f
02150ee5: mov    r12, qword ptr [rax + 0x30]
02150ee9: movsd  xmm6, qword ptr [rip + 0xf13d37]         ; [0x3064c28] dbl=1000.0 q=0x408f400000000000
02150ef1: mov    qword ptr [rsp + 0xa0], 0
02150efd: mov    rcx, qword ptr [r15]
02150f00: movsxd rax, edi
02150f03: movzx  edx, word ptr [rcx + rax*2]
02150f07: cmp    edx, 0x20
02150f0a: jbe    0x182150f4d
02150f0c: cmp    edx, 0x7b
02150f0f: je     0x18215124d
02150f15: cmp    edx, 0x7d
02150f18: jne    0x182150f7d
02150f1a: movaps xmm6, xmmword ptr [rsp + 0x50]
02150f1f: lea    eax, [rdi + 1]
02150f22: mov    r14, qword ptr [rsp + 0x60]
02150f27: mov    r13, qword ptr [rsp + 0x68]
02150f2c: mov    r12, qword ptr [rsp + 0xa8]
02150f34: mov    rsi, qword ptr [rsp + 0x98]
02150f3c: mov    rbp, qword ptr [rsp + 0x90]
02150f44: add    rsp, 0x70
02150f48: pop    r15
02150f4a: pop    rdi
02150f4b: pop    rbx
02150f4c: ret    
02150f4d: mov    ecx, edx
02150f4f: sub    ecx, 9
02150f52: je     0x18215124d
02150f58: sub    ecx, 1
02150f5b: je     0x18215124d
02150f61: sub    ecx, 1
02150f64: je     0x182150f7d
02150f66: sub    ecx, 1
02150f69: je     0x182150f7d
02150f6b: cmp    ecx, 1
02150f6e: je     0x18215124d
02150f74: cmp    edx, 0x20
02150f77: je     0x18215124d
02150f7d: mov    rcx, qword ptr [rip + 0x1b43a54]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02150f84: mov    r14, qword ptr [r15]
02150f87: mov    ebx, dword ptr [r15 + 0xc]
02150f8b: cmp    dword ptr [rcx + 0xe0], 0
02150f92: jne    0x182150f99
02150f94: call   0x182f60cf0
02150f99: cmp    byte ptr [rip + 0x1dc86c6], 0            ; [0x3f19666] (bss)
02150fa0: jne    0x182150fc1
02150fa2: lea    rcx, [rip + 0x1b43a2f]                   ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02150fa9: call   0x182f609b0
02150fae: lea    rcx, [rip + 0x1b96503]                   ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
02150fb5: call   0x182f609b0
02150fba: mov    byte ptr [rip + 0x1dc86a5], 1            ; [0x3f19666] (bss)
02150fc1: mov    rcx, qword ptr [rip + 0x1b43a10]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02150fc8: cmp    dword ptr [rcx + 0xe0], 0
02150fcf: jne    0x182150fd6
02150fd1: call   0x182f60cf0
02150fd6: xor    r8d, r8d
02150fd9: nop    dword ptr [rax]
02150fe0: cmp    edi, r13d
02150fe3: jae    0x18215131e
02150fe9: movsxd rax, edi
02150fec: movzx  edx, word ptr [r14 + rax*2]
02150ff1: lea    eax, [rdx - 0x30]
02150ff4: cmp    ax, 9
02150ff8: ja     0x18215100a
02150ffa: lea    r8, [r8 + r8*4]
02150ffe: inc    edi
02151000: lea    r8, [r8 - 0x18]
02151004: lea    r8, [rdx + r8*2]
02151008: jmp    0x182150fe0
0215100a: movsxd rax, edi
0215100d: cmp    word ptr [r14 + rax*2], 0x20
02151013: jne    0x1821512f3
02151019: lea    eax, [rdi + 1]
0215101c: cmp    eax, r13d
0215101f: jae    0x18215131e
02151025: movsxd rax, edi
02151028: cmp    word ptr [r14 + rax*2 + 2], 0x3d
0215102f: jne    0x1821512f3
02151035: lea    eax, [rdi + 2]
02151038: cmp    eax, r13d
0215103b: jae    0x18215131e
02151041: movsxd rax, edi
02151044: cmp    word ptr [r14 + rax*2 + 4], 0x20
0215104b: jne    0x1821512f3
02151051: lea    eax, [rdi + 3]
02151054: cmp    eax, r13d
02151057: jae    0x18215131e
0215105d: lea    ecx, [rdi + 4]
02151060: cmp    ecx, r13d
02151063: jae    0x18215131e
02151069: movsxd rax, edi
0215106c: movzx  edx, word ptr [r14 + rax*2 + 6]
02151072: xorps  xmm0, xmm0
02151075: mov    r9, qword ptr [rip + 0x1b9643c]          ; [0x3ce74b8] metamethod:Method$System.ValueTuple<char, long>..ctor()
0215107c: movsxd rax, ecx
0215107f: xor    ecx, ecx
02151081: cmp    word ptr [r14 + rax*2], 0x20
02151087: movups xmmword ptr [rsp + 0x30], xmm0
0215108c: setne  cl
0215108f: add    ecx, 5
02151092: add    edi, ecx
02151094: lea    rcx, [rsp + 0x30]
02151099: call   0x180ce1560                              ; System.ValueTuple<short, long>$$.ctor
0215109e: mov    rbp, qword ptr [rsp + 0x38]
021510a3: mov    rsi, qword ptr [rsp + 0xa0]
021510ab: mov    qword ptr [rsp + 0xa0], rbp
021510b3: cmp    rbp, rsi
021510b6: jl     0x18215128b
021510bc: movzx  eax, word ptr [rsp + 0x30]
021510c1: cmp    ax, 0x41
021510c5: jne    0x182151103
021510c7: mov    rcx, qword ptr [rip + 0x1b4390a]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
021510ce: cmp    dword ptr [rcx + 0xe0], 0
021510d5: jne    0x1821510e0
021510d7: call   0x182f60cf0
021510dc: nop    dword ptr [rax]
021510e0: cmp    edi, r13d
021510e3: jae    0x18215125f
021510e9: movsxd rax, edi
021510ec: movzx  ecx, word ptr [r14 + rax*2]
021510f1: sub    cx, 0x30
021510f5: cmp    cx, 9
021510f9: ja     0x18215124f
021510ff: inc    edi
02151101: jmp    0x1821510e0
02151103: cmp    ax, 0x42
02151107: jne    0x182151172
02151109: mov    rcx, qword ptr [rip + 0x1b438c8]         ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151110: cmp    dword ptr [rcx + 0xe0], 0
02151117: jne    0x18215111e
02151119: call   0x182f60cf0
0215111e: xor    ecx, ecx
02151120: cmp    edi, r13d
02151123: jae    0x18215125f
02151129: movsxd rax, edi
0215112c: movzx  edx, word ptr [r14 + rax*2]
02151131: lea    eax, [rdx - 0x30]
02151134: cmp    ax, 9
02151138: ja     0x18215114a
0215113a: lea    rcx, [rcx + rcx*4]
0215113e: inc    edi
02151140: lea    rcx, [rcx - 0x18]
02151144: lea    rcx, [rdx + rcx*2]
02151148: jmp    0x182151120
0215114a: test   r12, r12
0215114d: je     0x182151259
02151153: xorps  xmm2, xmm2
02151156: xor    r9d, r9d
02151159: cvtsi2sd xmm2, rcx
0215115e: mov    rdx, rbp
02151161: mov    rcx, r12
02151164: divsd  xmm2, xmm6
02151168: call   0x18212cbf0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʷʿʳʹʳʲʽʺʵʻʽ
0215116d: jmp    0x18215124f
02151172: cmp    ax, 0x54
02151176: jne    0x182151265
0215117c: mov    r8, qword ptr [rip + 0x1b43855]          ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
02151183: cmp    dword ptr [r8 + 0xe0], 0
0215118b: jne    0x18215119c
0215118d: mov    rcx, r8
02151190: call   0x182f60cf0
02151195: mov    r8, qword ptr [rip + 0x1b4383c]          ; [0x3c949d8] meta:ʶʸʻʻʷˁʳʳʻʽʴ_TypeInfo
0215119c: xor    ebx, ebx
0215119e: nop    
021511a0: cmp    edi, r13d
021511a3: jae    0x18215125f
021511a9: movsxd rax, edi
021511ac: movzx  edx, word ptr [r14 + rax*2]
021511b1: lea    eax, [rdx - 0x30]
021511b4: cmp    ax, 9
021511b8: ja     0x1821511ca
021511ba: lea    rbx, [rbx + rbx*4]
021511be: inc    edi
021511c0: lea    rbx, [rbx - 0x18]
021511c4: lea    rbx, [rdx + rbx*2]
021511c8: jmp    0x1821511a0
021511ca: cmp    edi, r13d
021511cd: jae    0x18215125f
021511d3: movsxd rdx, edi
021511d6: mov    r9d, 4
021511dc: inc    edi
021511de: cmp    word ptr [r14 + rdx*2], 0x20
021511e4: jne    0x18215122f
021511e6: cmp    dword ptr [r8 + 0xe0], 0
021511ee: jne    0x1821511f8
021511f0: mov    rcx, r8
021511f3: call   0x182f60cf0
021511f8: xor    ecx, ecx
021511fa: nop    word ptr [rax + rax]
02151200: cmp    edi, r13d
02151203: jae    0x18215125f
02151205: movsxd rax, edi
02151208: movzx  edx, word ptr [r14 + rax*2]
0215120d: lea    eax, [rdx - 0x30]
02151210: cmp    ax, 9
02151214: ja     0x182151226
02151216: lea    rcx, [rcx + rcx*4]
0215121a: inc    edi
0215121c: lea    rcx, [rcx - 0x18]
02151220: lea    rcx, [rdx + rcx*2]
02151224: jmp    0x182151200
02151226: mov    r9d, 1
0215122c: shl    r9d, cl
0215122f: test   r12, r12
02151232: je     0x182151259
02151234: mov    r8d, ebx
02151237: mov    qword ptr [rsp + 0x20], 0
02151240: mov    rdx, rbp
02151243: mov    rcx, r12
02151246: call   0x18212d620                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʼʸʼʴʺʾʵʷʽˁˀ
0215124b: jmp    0x18215124f
0215124d: inc    edi
0215124f: cmp    edi, r13d
02151252: jae    0x18215125f
02151254: jmp    0x182150efd
02151259: call   0x182f60c50
0215125e: int3   
0215125f: call   0x182f60c40
02151264: int3   
02151265: movups xmm0, xmmword ptr [r15]
02151269: xor    r9d, r9d
0215126c: mov    qword ptr [rsp + 0x20], 0
02151275: mov    r8d, edi
02151278: lea    rdx, [rsp + 0x40]
0215127d: mov    rcx, r12
02151280: movaps xmmword ptr [rsp + 0x40], xmm0
02151285: call   0x182162780                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʸʸʿʵʵʵʴʼˁʽʹ
0215128a: int3   
0215128b: mov    rcx, qword ptr [rip + 0x1bbd84e]         ; [0x3d0eae0] meta:long_TypeInfo
02151292: lea    rdx, [rsp + 0xa0]
0215129a: call   0x182f5fbe0
0215129f: mov    rcx, qword ptr [rip + 0x1bbd83a]         ; [0x3d0eae0] meta:long_TypeInfo
021512a6: lea    rdx, [rsp + 0x30]
021512ab: mov    rbx, rax
021512ae: mov    qword ptr [rsp + 0x30], rsi
021512b3: call   0x182f5fbe0
021512b8: mov    rcx, qword ptr [rip + 0x1b59d49]         ; [0x3cab008] str:'The sync event occuring at offset {0} is out of order. It comes after the sync event at {1} which is illegal.'
021512bf: xor    r9d, r9d
021512c2: mov    r8, rax
021512c5: mov    rdx, rbx
021512c8: call   0x181982840                              ; System.String$$Format
021512cd: movups xmm0, xmmword ptr [r15]
021512d1: mov    r9, rax
021512d4: mov    qword ptr [rsp + 0x20], 0
021512dd: mov    r8d, edi
021512e0: lea    rdx, [rsp + 0x40]
021512e5: mov    rcx, r12
021512e8: movaps xmmword ptr [rsp + 0x40], xmm0
021512ed: call   0x182162530                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʷʵʴʴˁʾʲʵʿʶʺ
021512f2: int3   
021512f3: xor    r9d, r9d
021512f6: mov    qword ptr [rsp + 0x40], r14
021512fb: mov    r8d, edi
021512fe: mov    dword ptr [rsp + 0x48], r13d
02151303: lea    rdx, [rsp + 0x40]
02151308: mov    dword ptr [rsp + 0x4c], ebx
0215130c: mov    rcx, r12
0215130f: mov    qword ptr [rsp + 0x20], 0
02151318: call   0x182162810                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʸˀʹʺʶʳʴʿʾʻʵ
0215131d: int3   
0215131e: call   0x182f60c40
02151323: int3   
02151324: int3   
