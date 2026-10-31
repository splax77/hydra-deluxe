020f7210: mov    qword ptr [rsp + 8], rbx
020f7215: push   rdi
020f7216: sub    rsp, 0x50
020f721a: cmp    byte ptr [rip + 0x1e221a9], 0            ; [0x3f193ca] (bss)
020f7221: mov    rbx, r8
020f7224: movaps xmmword ptr [rsp + 0x20], xmm8
020f722a: mov    rdi, rdx
020f722d: movaps xmm8, xmm3
020f7231: jne    0x1820f7246
020f7233: lea    rcx, [rip + 0x1bab52e]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f723a: call   0x182f609b0
020f723f: mov    byte ptr [rip + 0x1e22184], 1            ; [0x3f193ca] (bss)
020f7246: test   rdi, rdi
020f7249: je     0x1820f7353
020f724f: test   rbx, rbx
020f7252: je     0x1820f7353
020f7258: movsd  xmm1, qword ptr [rdi + 0x28]
020f725d: xorps  xmm0, xmm0
020f7260: subsd  xmm1, qword ptr [rbx + 0x28]
020f7265: ucomisd xmm1, xmm0
020f7269: jp     0x1820f7271
020f726b: je     0x1820f7353
020f7271: movaps xmmword ptr [rsp + 0x40], xmm6
020f7276: movaps xmmword ptr [rsp + 0x30], xmm7
020f727b: mov    rcx, qword ptr [rip + 0x1bab4e6]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f7282: movsd  xmm6, qword ptr [rbx + 0x28]
020f7287: movsd  xmm7, qword ptr [rdi + 0x28]
020f728c: cmp    dword ptr [rcx + 0xe0], 0
020f7293: jne    0x1820f729a
020f7295: call   0x182f60cf0
020f729a: cmp    byte ptr [rip + 0x1e15609], 0            ; [0x3f0c8aa] (bss)
020f72a1: subsd  xmm6, xmm7
020f72a5: mulsd  xmm6, qword ptr [rip + 0xf6cd03]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
020f72ad: jne    0x1820f72ce
020f72af: lea    rcx, [rip + 0x1c0f26a]                   ; [0x3d06520] metamethod:Method$System.Math.ThrowMinMaxException<double>()
020f72b6: call   0x182f609b0
020f72bb: lea    rcx, [rip + 0x1bab4a6]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020f72c2: call   0x182f609b0
020f72c7: mov    byte ptr [rip + 0x1e155dc], 1            ; [0x3f0c8aa] (bss)
020f72ce: movsd  xmm7, qword ptr [rsp + 0x80]
020f72d7: comisd xmm8, xmm7
020f72dc: jbe    0x1820f7306
020f72de: mov    rcx, qword ptr [rip + 0x1bab483]         ; [0x3ca2768] meta:System.Math_TypeInfo
020f72e5: cmp    dword ptr [rcx + 0xe0], 0
020f72ec: jne    0x1820f72f3
020f72ee: call   0x182f60cf0
020f72f3: mov    r8, qword ptr [rip + 0x1c0f226]          ; [0x3d06520] metamethod:Method$System.Math.ThrowMinMaxException<double>()
020f72fa: movaps xmm1, xmm7
020f72fd: movaps xmm0, xmm8
020f7301: call   0x1804d1790                              ; System.Math$$ThrowMinMaxException<double>
020f7306: comisd xmm8, xmm6
020f730b: ja     0x1820f7331
020f730d: comisd xmm6, xmm7
020f7311: jbe    0x1820f7335
020f7313: movaps xmm6, xmmword ptr [rsp + 0x40]
020f7318: movaps xmm0, xmm7
020f731b: movaps xmm7, xmmword ptr [rsp + 0x30]
020f7320: mov    rbx, qword ptr [rsp + 0x60]
020f7325: movaps xmm8, xmmword ptr [rsp + 0x20]
020f732b: add    rsp, 0x50
020f732f: pop    rdi
020f7330: ret    
020f7331: movaps xmm6, xmm8
020f7335: movaps xmm7, xmmword ptr [rsp + 0x30]
020f733a: movaps xmm0, xmm6
020f733d: movaps xmm6, xmmword ptr [rsp + 0x40]
020f7342: mov    rbx, qword ptr [rsp + 0x60]
020f7347: movaps xmm8, xmmword ptr [rsp + 0x20]
020f734d: add    rsp, 0x50
020f7351: pop    rdi
020f7352: ret    
020f7353: movsd  xmm0, qword ptr [rsp + 0x80]
020f735c: mov    rbx, qword ptr [rsp + 0x60]
020f7361: movaps xmm8, xmmword ptr [rsp + 0x20]
020f7367: add    rsp, 0x50
020f736b: pop    rdi
020f736c: ret    
020f736d: int3   
