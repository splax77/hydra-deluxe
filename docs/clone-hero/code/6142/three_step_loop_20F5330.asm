020f5330: push   rbx
020f5332: sub    rsp, 0x80
020f5339: mov    rax, qword ptr [rcx]
020f533c: mov    r8b, 1
020f533f: movaps xmmword ptr [rsp + 0x50], xmm8
020f5345: mov    rbx, rcx
020f5348: movaps xmm8, xmm1
020f534c: mov    r9, qword ptr [rax + 0x1c0]
020f5353: call   qword ptr [rax + 0x1b8]
020f5359: cmp    byte ptr [rbx + 0x290], 0
020f5360: je     0x1820f5463
020f5366: mov    qword ptr [rsp + 0x90], rdi
020f536e: movaps xmmword ptr [rsp + 0x70], xmm6
020f5373: movaps xmmword ptr [rsp + 0x60], xmm7
020f5378: movaps xmmword ptr [rsp + 0x40], xmm9
020f537e: movaps xmmword ptr [rsp + 0x30], xmm10
020f5384: movsd  xmm9, qword ptr [rip + 0xf70ef3]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f538d: xor    edi, edi
020f538f: movups xmm10, xmmword ptr [rbx + 0x100]
020f5397: movaps xmm7, xmm9
020f539b: nop    dword ptr [rax + rax]
020f53a0: xor    r9d, r9d
020f53a3: movaps xmm2, xmm8
020f53a7: movaps xmm1, xmm7
020f53aa: mov    rcx, rbx
020f53ad: call   0x1820f6ff0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʿˁʿʶʴʸʵʶʿʲˁ
020f53b2: ucomisd xmm0, xmm9
020f53b7: movaps xmm6, xmm0
020f53ba: jp     0x1820f53be
020f53bc: je     0x1820f542e
020f53be: comisd xmm7, xmm6
020f53c2: jae    0x1820f542e
020f53c4: mov    rcx, qword ptr [rbx + 0x230]
020f53cb: test   rcx, rcx
020f53ce: je     0x1820f5488
020f53d4: xor    r8d, r8d
020f53d7: movaps xmm1, xmm6
020f53da: movaps xmm7, xmm6
020f53dd: call   0x18212bcc0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʽʸʴʷʷʼʵʳʻʾ
020f53e2: xorps  xmm0, xmm0
020f53e5: lea    rcx, [rsp + 0x20]
020f53ea: xor    r9d, r9d
020f53ed: mov    r8, rax
020f53f0: movaps xmm1, xmm6
020f53f3: movups xmmword ptr [rsp + 0x20], xmm0
020f53f8: call   0x182117120                              ; ʴʻʽʻʲʾʸʵʺʶʲ$$.ctor
020f53fd: movups xmm0, xmmword ptr [rsp + 0x20]
020f5402: mov    rax, qword ptr [rbx]
020f5405: xor    r8d, r8d
020f5408: movaps xmm1, xmm8
020f540c: mov    rcx, rbx
020f540f: movups xmmword ptr [rbx + 0x100], xmm0
020f5416: mov    r9, qword ptr [rax + 0x1c0]
020f541d: call   qword ptr [rax + 0x1b8]
020f5423: inc    edi
020f5425: cmp    edi, 3
020f5428: jl     0x1820f53a0
020f542e: movaps xmm9, xmmword ptr [rsp + 0x40]
020f5434: movaps xmm7, xmmword ptr [rsp + 0x60]
020f5439: movaps xmm6, xmmword ptr [rsp + 0x70]
020f543e: mov    rdi, qword ptr [rsp + 0x90]
020f5446: movups xmmword ptr [rbx + 0x100], xmm10
020f544e: movaps xmm10, xmmword ptr [rsp + 0x30]
020f5454: movaps xmm8, xmmword ptr [rsp + 0x50]
020f545a: add    rsp, 0x80
020f5461: pop    rbx
020f5462: ret    
020f5463: movsd  xmm1, qword ptr [rip + 0xf70e15]         ; [0x3066280] dbl=-1.0 q=0xbff0000000000000
020f546b: xor    r9d, r9d
020f546e: movaps xmm2, xmm8
020f5472: mov    rcx, rbx
020f5475: movaps xmm8, xmmword ptr [rsp + 0x50]
020f547b: add    rsp, 0x80
020f5482: pop    rbx
020f5483: jmp    0x1820f6ff0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʿˁʿʶʴʸʵʶʿʲˁ
020f5488: call   0x182f60c50
020f548d: int3   
020f548e: int3   
