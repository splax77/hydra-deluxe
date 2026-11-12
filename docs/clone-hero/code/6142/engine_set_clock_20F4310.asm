020f4310: mov    qword ptr [rsp + 8], rbx
020f4315: push   rdi
020f4316: sub    rsp, 0x40
020f431a: mov    rbx, rcx
020f431d: movaps xmmword ptr [rsp + 0x30], xmm6
020f4322: mov    rcx, qword ptr [rcx + 0x230]
020f4329: movaps xmm6, xmm1
020f432c: test   rcx, rcx
020f432f: je     0x1820f438d
020f4331: xor    r8d, r8d
020f4334: call   0x18212bcc0                              ; ˁʿʺʲʲʹˀʴʾʻʻ$$ʳʽʸʴʷʷʼʵʳʻʾ
020f4339: xorps  xmm0, xmm0
020f433c: lea    rcx, [rsp + 0x20]
020f4341: xor    r9d, r9d
020f4344: mov    r8, rax
020f4347: movaps xmm1, xmm6
020f434a: mov    rdi, rax
020f434d: movups xmmword ptr [rsp + 0x20], xmm0
020f4352: call   0x182117120                              ; ʴʻʽʻʲʾʸʵʺʶʲ$$.ctor
020f4357: movups xmm0, xmmword ptr [rsp + 0x20]
020f435c: mov    rcx, qword ptr [rbx + 0x1d8]
020f4363: movups xmmword ptr [rbx + 0x100], xmm0
020f436a: test   rcx, rcx
020f436d: je     0x1820f438d
020f436f: xor    r9d, r9d
020f4372: movaps xmm2, xmm6
020f4375: mov    rdx, rdi
020f4378: call   0x1820dc060                              ; ʹʷʽʶʶʸʻʿˁʾʵ$$ʽʾʿʴʳʵʻʻʿʶʼ
020f437d: mov    rbx, qword ptr [rsp + 0x50]
020f4382: movaps xmm6, xmmword ptr [rsp + 0x30]
020f4387: add    rsp, 0x40
020f438b: pop    rdi
020f438c: ret    
020f438d: call   0x182f60c50
020f4392: int3   
020f4393: int3   
