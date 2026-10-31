0210dd00: mov    qword ptr [rsp + 8], rbx
0210dd05: mov    qword ptr [rsp + 0x10], rsi
0210dd0a: push   rdi
0210dd0b: sub    rsp, 0x40
0210dd0f: cmp    byte ptr [rip + 0x1e0b754], 0            ; [0x3f1946a] (bss)
0210dd16: mov    edi, edx
0210dd18: mov    esi, ecx
0210dd1a: jne    0x18210dd3b
0210dd1c: lea    rcx, [rip + 0x1b85255]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0210dd23: call   0x182f609b0
0210dd28: lea    rcx, [rip + 0x1b8f2b1]                   ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0210dd2f: call   0x182f609b0
0210dd34: mov    byte ptr [rip + 0x1e0b72f], 1            ; [0x3f1946a] (bss)
0210dd3b: movaps xmmword ptr [rsp + 0x30], xmm6
0210dd40: xor    ebx, ebx
0210dd42: movaps xmmword ptr [rsp + 0x20], xmm7
0210dd47: nop    word ptr [rax + rax]
0210dd50: mov    rcx, qword ptr [rip + 0x1b8f289]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0210dd57: cmp    dword ptr [rcx + 0xe0], 0
0210dd5e: jne    0x18210dd65
0210dd60: call   0x182f60cf0
0210dd65: mov    rax, qword ptr [rip + 0x1b8f274]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0210dd6c: mov    rcx, qword ptr [rax + 0xb8]
0210dd73: mov    rdx, qword ptr [rcx]
0210dd76: test   rdx, rdx
0210dd79: je     0x18210de16
0210dd7f: cmp    ebx, dword ptr [rdx + 0x18]
0210dd82: jae    0x18210de1c
0210dd88: mov    rcx, qword ptr [rip + 0x1b851e9]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
0210dd8f: movss  xmm7, dword ptr [rdx + rbx*4 + 0x20]
0210dd95: cmp    dword ptr [rcx + 0xe0], 0
0210dd9c: jne    0x18210dda3
0210dd9e: call   0x182f60cf0
0210dda3: cmp    byte ptr [rip + 0x1dfded2], 0            ; [0x3f0bc7c] (bss)
0210ddaa: movd   xmm6, edi
0210ddae: cvtdq2ps xmm6, xmm6
0210ddb1: mulss  xmm6, xmm7
0210ddb5: jne    0x18210ddca
0210ddb7: lea    rcx, [rip + 0x1b949aa]                   ; [0x3ca2768] meta:System.Math_TypeInfo
0210ddbe: call   0x182f609b0
0210ddc3: mov    byte ptr [rip + 0x1dfdeb2], 1            ; [0x3f0bc7c] (bss)
0210ddca: mov    rcx, qword ptr [rip + 0x1b94997]         ; [0x3ca2768] meta:System.Math_TypeInfo
0210ddd1: cmp    dword ptr [rcx + 0xe0], 0
0210ddd8: jne    0x18210dddf
0210ddda: call   0x182f60cf0
0210dddf: cvtps2pd xmm0, xmm6
0210dde2: call   0x183031130
0210dde7: cvttsd2si ecx, xmm0
0210ddeb: cmp    esi, ecx
0210dded: jl     0x18210ddfa
0210ddef: inc    ebx
0210ddf1: cmp    ebx, 7
0210ddf4: jl     0x18210dd50
0210ddfa: movaps xmm7, xmmword ptr [rsp + 0x20]
0210ddff: mov    eax, ebx
0210de01: mov    rbx, qword ptr [rsp + 0x50]
0210de06: movaps xmm6, xmmword ptr [rsp + 0x30]
0210de0b: mov    rsi, qword ptr [rsp + 0x58]
0210de10: add    rsp, 0x40
0210de14: pop    rdi
0210de15: ret    
0210de16: call   0x182f60c50
0210de1b: int3   
0210de1c: call   0x182f60c40
0210de21: int3   
0210de22: int3   
