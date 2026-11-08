020f3dc0: push   rbx
020f3dc2: push   rbp
020f3dc3: sub    rsp, 0x58
020f3dc7: cmp    byte ptr [rip + 0x1e255e8], 0            ; [0x3f193b6] (bss)
020f3dce: mov    rbp, rcx
020f3dd1: jne    0x1820f3e62
020f3dd7: lea    rcx, [rip + 0x1be3402]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f3dde: call   0x182f609b0
020f3de3: lea    rcx, [rip + 0x1bdbc16]                   ; [0x3ccfa00] metamethod:Method$System.Collections.Generic.List<ʵʷʳˁʶʺʼʲʵʴʴ>.get_Count()
020f3dea: call   0x182f609b0
020f3def: lea    rcx, [rip + 0x1be242a]                   ; [0x3cd6220] metamethod:Method$System.Collections.Generic.List<ʼʿʲˀʼˀʵʹʼʳʾ>.get_Count()
020f3df6: call   0x182f609b0
020f3dfb: lea    rcx, [rip + 0x1be349e]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f3e02: call   0x182f609b0
020f3e07: lea    rcx, [rip + 0x1be24d2]                   ; [0x3cd62e0] metamethod:Method$System.Collections.Generic.List<ʼʿʲˀʼˀʵʹʼʳʾ>.get_Item()
020f3e0e: call   0x182f609b0
020f3e13: lea    rcx, [rip + 0x1bb76d6]                   ; [0x3cab4f0] metamethod:Method$System.Span<ʼʿʲˀʼˀʵʹʼʳʾ>.Slice()
020f3e1a: call   0x182f609b0
020f3e1f: lea    rcx, [rip + 0x1bb7782]                   ; [0x3cab5a8] metamethod:Method$System.Span<ʼʿʲˀʼˀʵʹʼʳʾ>.get_Length()
020f3e26: call   0x182f609b0
020f3e2b: lea    rcx, [rip + 0x1c1ac36]                   ; [0x3d0ea68] meta:ʳʲʵʿʺʷˁʺˀʿʵ_TypeInfo
020f3e32: call   0x182f609b0
020f3e37: lea    rcx, [rip + 0x1bb7c62]                   ; [0x3cabaa0] meta:ʼʽʶʿˀʵʶʻʴʽʼ[]_TypeInfo
020f3e3e: call   0x182f609b0
020f3e43: lea    rcx, [rip + 0x1bfdba6]                   ; [0x3cf19f0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʿʲˀʼˀʵʹʼʳʾ>()
020f3e4a: call   0x182f609b0
020f3e4f: lea    rcx, [rip + 0x1ba0da2]                   ; [0x3c94bf8] str:'_endofsong'
020f3e56: call   0x182f609b0
020f3e5b: mov    byte ptr [rip + 0x1e25554], 1            ; [0x3f193b6] (bss)
020f3e62: mov    rbx, qword ptr [rbp + 0x230]
020f3e69: mov    qword ptr [rsp + 0x78], rsi
020f3e6e: mov    qword ptr [rsp + 0x50], rdi
020f3e73: mov    qword ptr [rsp + 0x48], r12
020f3e78: mov    qword ptr [rsp + 0x40], r13
020f3e7d: mov    qword ptr [rsp + 0x38], r14
020f3e82: mov    qword ptr [rsp + 0x30], r15
020f3e87: test   rbx, rbx
020f3e8a: je     0x1820f4304
020f3e90: mov    rdi, qword ptr [rip + 0x1bfdb59]         ; [0x3cf19f0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʼʿʲˀʼˀʵʹʼʳʾ>()
020f3e97: mov    rsi, qword ptr [rbx + 0xd8]
020f3e9e: cmp    qword ptr [rdi + 0x38], 0
020f3ea3: jne    0x1820f3ead
020f3ea5: mov    rcx, rdi
020f3ea8: call   0x182f657d0
020f3ead: mov    r8, qword ptr [rdi + 0x38]
020f3eb1: lea    rcx, [rsp + 0x20]
020f3eb6: mov    rdx, rsi
020f3eb9: mov    r8, qword ptr [r8 + 8]
020f3ebd: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
020f3ec2: mov    edi, dword ptr [rsp + 0x28]
020f3ec6: mov    dword ptr [rsp + 0x80], edi
020f3ecd: test   edi, edi
020f3ecf: jle    0x1820f3f6e
020f3ed5: mov    rdx, qword ptr [rbx + 0xd8]
020f3edc: test   rdx, rdx
020f3edf: je     0x1820f4304
020f3ee5: mov    edx, dword ptr [rdx + 0x18]
020f3ee8: mov    r8, qword ptr [rip + 0x1be23f1]          ; [0x3cd62e0] metamethod:Method$System.Collections.Generic.List<ʼʿʲˀʼˀʵʹʼʳʾ>.get_Item()
020f3eef: dec    edx
020f3ef1: mov    rcx, qword ptr [rbx + 0xd8]
020f3ef8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f3efd: test   rax, rax
020f3f00: je     0x1820f4304
020f3f06: mov    rcx, qword ptr [rax + 0x18]
020f3f0a: test   rcx, rcx
020f3f0d: je     0x1820f4304
020f3f13: mov    rdx, qword ptr [rip + 0x1ba0cde]         ; [0x3c94bf8] str:'_endofsong'
020f3f1a: xor    r9d, r9d
020f3f1d: mov    r8d, 5
020f3f23: call   0x181981640                              ; System.String$$Equals
020f3f28: test   al, al
020f3f2a: je     0x1820f3f6e
020f3f2c: mov    rax, qword ptr [rbx + 0xd8]
020f3f33: test   rax, rax
020f3f36: je     0x1820f4304
020f3f3c: mov    ebx, dword ptr [rax + 0x18]
020f3f3f: mov    rsi, qword ptr [rip + 0x1bb75aa]         ; [0x3cab4f0] metamethod:Method$System.Span<ʼʿʲˀʼˀʵʹʼʳʾ>.Slice()
020f3f46: dec    ebx
020f3f48: cmp    ebx, edi
020f3f4a: jbe    0x1820f3f53
020f3f4c: xor    ecx, ecx
020f3f4e: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
020f3f53: mov    rcx, qword ptr [rsi + 0x20]
020f3f57: test   byte ptr [rcx + 0x135], 1
020f3f5e: jne    0x1820f3f65
020f3f60: call   0x182f65750
020f3f65: mov    dword ptr [rsp + 0x80], ebx
020f3f6c: jmp    0x1820f3f72
020f3f6e: mov    ebx, dword ptr [rsp + 0x28]
020f3f72: mov    rcx, qword ptr [rip + 0x1bb7b27]         ; [0x3cabaa0] meta:ʼʽʶʿˀʵʶʻʴʽʼ[]_TypeInfo
020f3f79: mov    edx, ebx
020f3f7b: call   0x182f5fc80
020f3f80: xor    r12d, r12d
020f3f83: mov    r13, rax
020f3f86: mov    dword ptr [rsp + 0x70], r12d
020f3f8b: test   ebx, ebx
020f3f8d: jle    0x1820f40f1
020f3f93: xor    edi, edi
020f3f95: lea    eax, [rbx - 1]
020f3f98: xor    esi, esi
020f3f9a: nop    word ptr [rax + rax]
020f3fa0: xor    r15d, r15d
020f3fa3: cmp    edi, eax
020f3fa5: jge    0x1820f3fc9
020f3fa7: lea    eax, [rdi + 1]
020f3faa: cmp    eax, ebx
020f3fac: jae    0x1820f430a
020f3fb2: mov    r15, qword ptr [rsp + 0x20]
020f3fb7: mov    r15, qword ptr [r15 + rdi*8 + 8]
020f3fbc: test   r15, r15
020f3fbf: je     0x1820f4304
020f3fc5: mov    r15, qword ptr [r15 + 0x10]
020f3fc9: xor    r14d, r14d
020f3fcc: xor    r12d, r12d
020f3fcf: nop    
020f3fd0: mov    rax, qword ptr [rbp + 0x1e0]
020f3fd7: test   rax, rax
020f3fda: je     0x1820f4304
020f3fe0: cmp    esi, dword ptr [rax + 0x18]
020f3fe3: jge    0x1820f4050
020f3fe5: mov    r8, qword ptr [rip + 0x1be32b4]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f3fec: mov    edx, esi
020f3fee: mov    rcx, rax
020f3ff1: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f3ff6: mov    rbx, rax
020f3ff9: test   r15, r15
020f3ffc: jle    0x1820f400d
020f3ffe: test   rax, rax
020f4001: je     0x1820f4304
020f4007: cmp    qword ptr [rax + 0x40], r15
020f400b: jge    0x1820f4050
020f400d: test   rbx, rbx
020f4010: je     0x1820f4304
020f4016: xor    edx, edx
020f4018: mov    rcx, rbx
020f401b: call   0x18214b800                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʺʴʲʹʽʶʹʳʻʼʽ
020f4020: test   al, al
020f4022: je     0x1820f4032
020f4024: xor    edx, edx
020f4026: mov    rcx, rbx
020f4029: call   0x18214bcc0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʿˀʼʷʲʸʶʾˀʷʹ
020f402e: test   al, al
020f4030: jne    0x1820f404c
020f4032: xor    edx, edx
020f4034: mov    rcx, rbx
020f4037: inc    r12d
020f403a: call   0x18214b610                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʲʺʾʽʽʹʳʳʼʹ
020f403f: lea    ecx, [r14 + 1]
020f4043: test   al, al
020f4045: cmove  ecx, r14d
020f4049: mov    r14d, ecx
020f404c: inc    esi
020f404e: jmp    0x1820f3fd0
020f4050: test   r13, r13
020f4053: je     0x1820f4304
020f4059: mov    ebx, dword ptr [rsp + 0x80]
020f4060: cmp    edi, ebx
020f4062: jae    0x1820f430a
020f4068: mov    rcx, qword ptr [rsp + 0x20]
020f406d: mov    rcx, qword ptr [rcx + rdi*8]
020f4071: test   rcx, rcx
020f4074: je     0x1820f4304
020f407a: cmp    edi, dword ptr [r13 + 0x18]
020f407e: jae    0x1820f430a
020f4084: mov    rdx, qword ptr [rcx + 0x18]
020f4088: mov    eax, edi
020f408a: add    rax, 2
020f408e: add    rax, rax
020f4091: mov    qword ptr [r13 + rax*8], rdx
020f4096: cmp    edi, dword ptr [r13 + 0x18]
020f409a: jae    0x1820f430a
020f40a0: mov    ecx, edi
020f40a2: add    rcx, 2
020f40a6: shl    rcx, 4
020f40aa: add    rcx, r13
020f40ad: call   0x182f5fc00
020f40b2: cmp    edi, dword ptr [r13 + 0x18]
020f40b6: jae    0x1820f430a
020f40bc: mov    eax, edi
020f40be: add    rax, rax
020f40c1: mov    dword ptr [r13 + rax*8 + 0x2c], r14d
020f40c6: cmp    edi, dword ptr [r13 + 0x18]
020f40ca: jae    0x1820f430a
020f40d0: add    dword ptr [rsp + 0x70], r12d
020f40d5: mov    eax, edi
020f40d7: inc    edi
020f40d9: add    rax, rax
020f40dc: mov    dword ptr [r13 + rax*8 + 0x28], r12d
020f40e1: lea    eax, [rbx - 1]
020f40e4: cmp    edi, ebx
020f40e6: jl     0x1820f3fa0
020f40ec: mov    r12d, dword ptr [rsp + 0x70]
020f40f1: mov    rdx, qword ptr [rbp + 0xf8]
020f40f8: nop    dword ptr [rax + rax]
020f4100: xor    ebx, ebx
020f4102: test   rdx, rdx
020f4105: je     0x1820f4304
020f410b: nop    dword ptr [rax + rax]
020f4110: cmp    ebx, dword ptr [rdx + 0x18]
020f4113: jae    0x1820f430a
020f4119: movsxd rax, ebx
020f411c: mov    ecx, dword ptr [rdx + rax*4 + 0x20]
020f4120: cmp    dword ptr [rbp + 0x94], ecx
020f4126: jl     0x1820f412f
020f4128: inc    ebx
020f412a: cmp    ebx, 7
020f412d: jl     0x1820f4110
020f412f: mov    rcx, qword ptr [rip + 0x1c1a932]         ; [0x3d0ea68] meta:ʳʲʵʿʺʷˁʺˀʿʵ_TypeInfo
020f4136: mov    r14d, dword ptr [rbp + 0xdc]
020f413d: mov    r15d, dword ptr [rbp + 0xd8]
020f4144: mov    esi, dword ptr [rbp + 0xc0]
020f414a: call   0x182f60c00
020f414f: xor    edx, edx
020f4151: mov    rcx, rax
020f4154: mov    rdi, rax
020f4157: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020f415c: test   rdi, rdi
020f415f: je     0x1820f4304
020f4165: mov    ecx, dword ptr [rbp + 0x94]
020f416b: mov    dword ptr [rdi + 0x14], ecx
020f416e: mov    ecx, dword ptr [rbp + 0x1d0]
020f4174: mov    dword ptr [rdi + 0x10], ecx
020f4177: mov    rax, qword ptr [rbp + 0x228]
020f417e: test   rax, rax
020f4181: je     0x1820f4304
020f4187: mov    rax, qword ptr [rax + 0x70]
020f418b: test   rax, rax
020f418e: je     0x1820f4304
020f4194: mov    eax, dword ptr [rax + 0x18]
020f4197: lea    rcx, [rdi + 0xa0]
020f419e: mov    dword ptr [rdi + 0x4c], eax
020f41a1: mov    rdx, r13
020f41a4: mov    dword ptr [rdi + 0x94], ebx
020f41aa: mov    eax, dword ptr [rbp + 0xa0]
020f41b0: mov    dword ptr [rdi + 0x18], eax
020f41b3: mov    eax, dword ptr [rbp + 0xa4]
020f41b9: mov    dword ptr [rdi + 0x1c], eax
020f41bc: movzx  eax, byte ptr [rbp + 0xa8]
020f41c3: mov    byte ptr [rdi + 0xa8], al
020f41c9: mov    eax, dword ptr [rbp + 0x98]
020f41cf: mov    dword ptr [rdi + 0x20], eax
020f41d2: mov    eax, dword ptr [rbp + 0x88]
020f41d8: mov    dword ptr [rdi + 0x24], eax
020f41db: mov    eax, dword ptr [rbp + 0x9c]
020f41e1: mov    dword ptr [rdi + 0x40], eax
020f41e4: mov    dword ptr [rdi + 0x44], r12d
020f41e8: mov    eax, dword ptr [rbp + 0x4c]
020f41eb: mov    dword ptr [rdi + 0x48], eax
020f41ee: mov    qword ptr [rdi + 0xa0], r13
020f41f5: call   0x182f5fc00
020f41fa: mov    r13, qword ptr [rsp + 0x40]
020f41ff: xorps  xmm1, xmm1
020f4202: cvtsi2sd xmm1, qword ptr [rbp + 0x80]
020f420b: mov    r12, qword ptr [rsp + 0x48]
020f4210: xorps  xmm0, xmm0
020f4213: cvtsi2sd xmm0, qword ptr [rbp + 0x1b0]
020f421c: divsd  xmm1, xmm0
020f4220: cvtpd2ps xmm1, xmm1
020f4224: movss  dword ptr [rdi + 0x50], xmm1
020f4229: mov    rax, qword ptr [rbp + 0x80]
020f4230: mov    qword ptr [rdi + 0x58], rax
020f4234: mov    rax, qword ptr [rbp + 0x1b0]
020f423b: mov    qword ptr [rdi + 0x60], rax
020f423f: mov    rax, qword ptr [rbp + 0xb8]
020f4246: mov    qword ptr [rdi + 0x70], rax
020f424a: mov    eax, dword ptr [rbp + 0xb0]
020f4250: mov    dword ptr [rdi + 0x28], eax
020f4253: mov    eax, dword ptr [rbp + 0xb4]
020f4259: mov    dword ptr [rdi + 0x68], eax
020f425c: lea    eax, [r15 + r14]
020f4260: mov    r15, qword ptr [rsp + 0x30]
020f4265: mov    r14, qword ptr [rsp + 0x38]
020f426a: imul   ecx, eax, 0x32
020f426d: sub    esi, ecx
020f426f: mov    dword ptr [rdi + 0x2c], esi
020f4272: mov    eax, dword ptr [rbp + 0xc4]
020f4278: mov    rsi, qword ptr [rsp + 0x78]
020f427d: mov    dword ptr [rdi + 0x30], eax
020f4280: mov    eax, dword ptr [rbp + 0xcc]
020f4286: mov    dword ptr [rdi + 0x38], eax
020f4289: mov    eax, dword ptr [rbp + 0xd0]
020f428f: mov    dword ptr [rdi + 0x3c], eax
020f4292: mov    eax, dword ptr [rbp + 0xc8]
020f4298: mov    dword ptr [rdi + 0x34], eax
020f429b: mov    eax, dword ptr [rbp + 0xd4]
020f42a1: mov    dword ptr [rdi + 0x78], eax
020f42a4: mov    eax, dword ptr [rbp + 0xd8]
020f42aa: mov    dword ptr [rdi + 0x7c], eax
020f42ad: mov    eax, dword ptr [rbp + 0xdc]
020f42b3: mov    dword ptr [rdi + 0x84], eax
020f42b9: mov    eax, dword ptr [rbp + 0xe4]
020f42bf: mov    dword ptr [rdi + 0x88], eax
020f42c5: mov    eax, dword ptr [rbp + 0xe8]
020f42cb: mov    dword ptr [rdi + 0x8c], eax
020f42d1: mov    eax, dword ptr [rbp + 0xec]
020f42d7: mov    dword ptr [rdi + 0x90], eax
020f42dd: mov    eax, dword ptr [rbp + 0xe0]
020f42e3: mov    dword ptr [rdi + 0x80], eax
020f42e9: mov    eax, dword ptr [rbp + 0xac]
020f42ef: mov    dword ptr [rdi + 0x98], eax
020f42f5: mov    rax, rdi
020f42f8: mov    rdi, qword ptr [rsp + 0x50]
020f42fd: add    rsp, 0x58
020f4301: pop    rbp
020f4302: pop    rbx
020f4303: ret    
020f4304: call   0x182f60c50
020f4309: int3   
020f430a: call   0x182f60c40
020f430f: int3   
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
