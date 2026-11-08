020d3f30: mov    qword ptr [rsp + 8], rcx
020d3f35: push   rbp
020d3f36: push   rbx
020d3f37: push   rsi
020d3f38: lea    rbp, [rsp - 0x47]
020d3f3d: sub    rsp, 0x90
020d3f44: cmp    byte ptr [rip + 0x1e453bd], 0            ; [0x3f19308] (bss)
020d3f4b: mov    rbx, rcx
020d3f4e: jne    0x1820d3fdf
020d3f54: lea    rcx, [rip + 0x1bf8dad]                   ; [0x3cccd08] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.Add()
020d3f5b: call   0x182f609b0
020d3f60: lea    rcx, [rip + 0x1bf8ce1]                   ; [0x3cccc48] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>..ctor()
020d3f67: call   0x182f609b0
020d3f6c: lea    rcx, [rip + 0x1bfb54d]                   ; [0x3ccf4c0] metamethod:Method$System.Collections.Generic.List<ʵʴʻʵʶʾʵʾʳʵʺ>.get_Count()
020d3f73: call   0x182f609b0
020d3f78: lea    rcx, [rip + 0x1c00941]                   ; [0x3cd48c0] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Count()
020d3f7f: call   0x182f609b0
020d3f84: lea    rcx, [rip + 0x1c009f5]                   ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
020d3f8b: call   0x182f609b0
020d3f90: lea    rcx, [rip + 0x1bfb5e9]                   ; [0x3ccf580] metamethod:Method$System.Collections.Generic.List<ʵʴʻʵʶʾʵʾʳʵʺ>.get_Item()
020d3f97: call   0x182f609b0
020d3f9c: lea    rcx, [rip + 0x1c2022d]                   ; [0x3cf41d0] meta:System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>_TypeInfo
020d3fa3: call   0x182f609b0
020d3fa8: lea    rcx, [rip + 0x1c2f089]                   ; [0x3d03038] metamethod:Method$System.Nullable<long>.GetValueOrDefault()
020d3faf: call   0x182f609b0
020d3fb4: lea    rcx, [rip + 0x1c2ef05]                   ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d3fbb: call   0x182f609b0
020d3fc0: lea    rcx, [rip + 0x1c2f1e9]                   ; [0x3d031b0] metamethod:Method$System.Nullable<long>.get_HasValue()
020d3fc7: call   0x182f609b0
020d3fcc: lea    rcx, [rip + 0x1c39a75]                   ; [0x3d0da48] meta:ʲʲʺʴʺʼʷʸʾʷʹ_TypeInfo
020d3fd3: call   0x182f609b0
020d3fd8: mov    byte ptr [rip + 0x1e45329], 1            ; [0x3f19308] (bss)
020d3fdf: mov    rcx, qword ptr [rip + 0x1c201ea]         ; [0x3cf41d0] meta:System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>_TypeInfo
020d3fe6: xorps  xmm0, xmm0
020d3fe9: mov    qword ptr [rsp + 0xb8], rdi
020d3ff1: mov    qword ptr [rsp + 0x88], r12
020d3ff9: mov    qword ptr [rsp + 0x80], r13
020d4001: mov    qword ptr [rsp + 0x78], r14
020d4006: mov    qword ptr [rsp + 0x70], r15
020d400b: movaps xmmword ptr [rsp + 0x60], xmm6
020d4010: movaps xmmword ptr [rsp + 0x50], xmm7
020d4015: movups xmmword ptr [rbp - 9], xmm0
020d4019: call   0x182f60c00
020d401e: mov    rdx, qword ptr [rip + 0x1bf8c23]         ; [0x3cccc48] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>..ctor()
020d4025: mov    rcx, rax
020d4028: mov    rsi, rax
020d402b: mov    qword ptr [rbp - 0x19], rax
020d402f: call   0x180706cb0                              ; System.Collections.Generic.LowLevelList<__Il2CppFullySharedGenericType>$$.ctor
020d4034: test   rbx, rbx
020d4037: je     0x1820d43a5
020d403d: mov    rcx, qword ptr [rbx + 0xe0]
020d4044: mov    qword ptr [rbp - 0x11], rcx
020d4048: test   rcx, rcx
020d404b: je     0x1820d43a5
020d4051: mov    r13, qword ptr [rbx + 0xe8]
020d4058: mov    r14d, 4
020d405e: movsd  xmm7, qword ptr [rip + 0xf8ff4a]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
020d4066: xor    r12d, r12d
020d4069: xor    r15d, r15d
020d406c: mov    dword ptr [rbp + 0x77], r14d
020d4070: xor    eax, eax
020d4072: mov    dword ptr [rbp + 0x7f], r14d
020d4076: mov    qword ptr [rbp - 0x21], rax
020d407a: xorps  xmm6, xmm6
020d407d: mov    dword ptr [rbp - 0x29], eax
020d4080: cmp    eax, dword ptr [rcx + 0x18]
020d4083: jl     0x1820d4098
020d4085: test   r13, r13
020d4088: je     0x1820d43a5
020d408e: cmp    r15d, dword ptr [r13 + 0x18]
020d4092: jge    0x1820d4354
020d4098: cmp    r12d, dword ptr [rcx + 0x18]
020d409c: jge    0x1820d40b2
020d409e: mov    r8, qword ptr [rip + 0x1c008db]          ; [0x3cd4980] metamethod:Method$System.Collections.Generic.List<ʼʳʿʼʼʾʺʿʺʸʸ>.get_Item()
020d40a5: mov    edx, r12d
020d40a8: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d40ad: mov    rdi, rax
020d40b0: jmp    0x1820d40b4
020d40b2: xor    edi, edi
020d40b4: test   r13, r13
020d40b7: je     0x1820d43a5
020d40bd: cmp    r15d, dword ptr [r13 + 0x18]
020d40c1: jge    0x1820d40da
020d40c3: mov    r8, qword ptr [rip + 0x1bfb4b6]          ; [0x3ccf580] metamethod:Method$System.Collections.Generic.List<ʵʴʻʵʶʾʵʾʳʵʺ>.get_Item()
020d40ca: mov    edx, r15d
020d40cd: mov    rcx, r13
020d40d0: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020d40d5: mov    rbx, rax
020d40d8: jmp    0x1820d40dc
020d40da: xor    ebx, ebx
020d40dc: test   rdi, rdi
020d40df: jne    0x1820d40fd
020d40e1: test   rbx, rbx
020d40e4: je     0x1820d4354
020d40ea: xorps  xmm0, xmm0
020d40ed: movd   esi, xmm0
020d40f1: psrldq xmm0, 8
020d40f6: movq   r14, xmm0
020d40fb: jmp    0x1820d4125
020d40fd: test   rbx, rbx
020d4100: je     0x1820d415d
020d4102: mov    r8, qword ptr [rip + 0x1c2edb7]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d4109: lea    rcx, [rbp - 9]
020d410d: mov    rdx, qword ptr [rdi + 0x10]
020d4111: xorps  xmm0, xmm0
020d4114: movups xmmword ptr [rbp - 9], xmm0
020d4118: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d411d: movzx  esi, byte ptr [rbp - 9]
020d4121: mov    r14, qword ptr [rbp - 1]
020d4125: mov    r8, qword ptr [rip + 0x1c2ed94]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d412c: lea    rcx, [rbp - 9]
020d4130: mov    rdx, qword ptr [rbx + 0x10]
020d4134: xorps  xmm0, xmm0
020d4137: movups xmmword ptr [rbp - 9], xmm0
020d413b: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d4140: movzx  eax, byte ptr [rbp - 9]
020d4144: xor    ecx, ecx
020d4146: cmp    r14, qword ptr [rbp - 1]
020d414a: setl   cl
020d414d: and    ecx, eax
020d414f: test   sil, cl
020d4152: je     0x1820d4172
020d4154: test   rdi, rdi
020d4157: je     0x1820d43a5
020d415d: mov    rax, qword ptr [rdi + 0x10]
020d4161: inc    r12d
020d4164: movsd  xmm7, qword ptr [rdi + 0x18]
020d4169: movsd  xmm6, qword ptr [rdi + 0x20]
020d416e: mov    qword ptr [rbp - 0x21], rax
020d4172: test   rbx, rbx
020d4175: je     0x1820d41a6
020d4177: test   rdi, rdi
020d417a: je     0x1820d4204
020d4180: mov    r8, qword ptr [rip + 0x1c2ed39]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d4187: lea    rcx, [rbp - 9]
020d418b: mov    rdx, qword ptr [rbx + 0x10]
020d418f: xorps  xmm0, xmm0
020d4192: movups xmmword ptr [rbp - 9], xmm0
020d4196: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d419b: movzx  r14d, byte ptr [rbp - 9]
020d41a0: mov    rsi, qword ptr [rbp - 1]
020d41a4: jmp    0x1820d41bd
020d41a6: xorps  xmm0, xmm0
020d41a9: movd   r14d, xmm0
020d41ae: psrldq xmm0, 8
020d41b3: movq   rsi, xmm0
020d41b8: test   rdi, rdi
020d41bb: je     0x1820d41e2
020d41bd: mov    r8, qword ptr [rip + 0x1c2ecfc]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d41c4: lea    rcx, [rbp - 9]
020d41c8: mov    rdx, qword ptr [rdi + 0x10]
020d41cc: xorps  xmm0, xmm0
020d41cf: movups xmmword ptr [rbp - 9], xmm0
020d41d3: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d41d8: movzx  eax, byte ptr [rbp - 9]
020d41dc: mov    rdx, qword ptr [rbp - 1]
020d41e0: jmp    0x1820d41e9
020d41e2: movzx  eax, r14b
020d41e6: mov    rdx, rsi
020d41e9: xor    ecx, ecx
020d41eb: movzx  eax, al
020d41ee: cmp    rsi, rdx
020d41f1: setl   cl
020d41f4: and    ecx, eax
020d41f6: test   r14b, cl
020d41f9: je     0x1820d4220
020d41fb: test   rbx, rbx
020d41fe: je     0x1820d43a5
020d4204: mov    rax, qword ptr [rbx + 0x10]
020d4208: inc    r15d
020d420b: mov    ecx, dword ptr [rbx + 0x18]
020d420e: movsd  xmm6, qword ptr [rbx + 0x20]
020d4213: mov    qword ptr [rbp - 0x21], rax
020d4217: mov    eax, dword ptr [rbx + 0x1c]
020d421a: mov    dword ptr [rbp + 0x7f], eax
020d421d: mov    dword ptr [rbp + 0x77], ecx
020d4220: xorps  xmm0, xmm0
020d4223: test   rdi, rdi
020d4226: je     0x1820d424b
020d4228: mov    r8, qword ptr [rip + 0x1c2ec91]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d422f: lea    rcx, [rbp - 9]
020d4233: mov    rdx, qword ptr [rdi + 0x10]
020d4237: movups xmmword ptr [rbp - 9], xmm0
020d423b: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d4240: movzx  r14d, byte ptr [rbp - 9]
020d4245: mov    rsi, qword ptr [rbp - 1]
020d4249: jmp    0x1820d425a
020d424b: movd   r14d, xmm0
020d4250: psrldq xmm0, 8
020d4255: movq   rsi, xmm0
020d425a: xorps  xmm0, xmm0
020d425d: test   rbx, rbx
020d4260: je     0x1820d4284
020d4262: mov    r8, qword ptr [rip + 0x1c2ec57]          ; [0x3d02ec0] metamethod:Method$System.Nullable<long>..ctor()
020d4269: lea    rcx, [rbp - 9]
020d426d: mov    rdx, qword ptr [rbx + 0x10]
020d4271: movups xmmword ptr [rbp - 9], xmm0
020d4275: call   0x180868470                              ; System.Data.SqlTypes.SqlMoney$$.ctor
020d427a: movzx  edx, byte ptr [rbp - 9]
020d427e: mov    rax, qword ptr [rbp - 1]
020d4282: jmp    0x1820d4292
020d4284: movd   edx, xmm0
020d4288: psrldq xmm0, 8
020d428d: movq   rax, xmm0
020d4292: xor    ecx, ecx
020d4294: cmp    rsi, rax
020d4297: sete   cl
020d429a: xor    eax, eax
020d429c: cmp    r14b, dl
020d429f: sete   al
020d42a2: test   eax, ecx
020d42a4: je     0x1820d42e0
020d42a6: test   rbx, rbx
020d42a9: je     0x1820d43a5
020d42af: test   rdi, rdi
020d42b2: je     0x1820d43a5
020d42b8: mov    rsi, qword ptr [rdi + 0x10]
020d42bc: inc    r12d
020d42bf: mov    r14d, dword ptr [rbx + 0x18]
020d42c3: inc    r15d
020d42c6: mov    eax, dword ptr [rbx + 0x1c]
020d42c9: movsd  xmm7, qword ptr [rdi + 0x18]
020d42ce: movsd  xmm6, qword ptr [rdi + 0x20]
020d42d3: mov    qword ptr [rbp - 0x21], rsi
020d42d7: mov    dword ptr [rbp + 0x77], r14d
020d42db: mov    dword ptr [rbp + 0x7f], eax
020d42de: jmp    0x1820d42e8
020d42e0: mov    rsi, qword ptr [rbp - 0x21]
020d42e4: mov    r14d, dword ptr [rbp + 0x77]
020d42e8: mov    rcx, qword ptr [rip + 0x1c39759]         ; [0x3d0da48] meta:ʲʲʺʴʺʼʷʸʾʷʹ_TypeInfo
020d42ef: call   0x182f60c00
020d42f4: xor    edx, edx
020d42f6: mov    rcx, rax
020d42f9: mov    rbx, rax
020d42fc: call   0x180005570                              ; UnitySourceGeneratedAssemblyMonoScriptTypes_v1$$.ctor
020d4301: test   rbx, rbx
020d4304: je     0x1820d43a5
020d430a: mov    edi, dword ptr [rbp - 0x29]
020d430d: mov    eax, dword ptr [rbp + 0x7f]
020d4310: mov    qword ptr [rbx + 0x10], rsi
020d4314: mov    rsi, qword ptr [rbp - 0x19]
020d4318: movsd  qword ptr [rbx + 0x18], xmm6
020d431d: movsd  qword ptr [rbx + 0x30], xmm7
020d4322: mov    dword ptr [rbx + 0x20], edi
020d4325: mov    dword ptr [rbx + 0x24], r14d
020d4329: mov    dword ptr [rbx + 0x28], eax
020d432c: test   rsi, rsi
020d432f: je     0x1820d43a5
020d4331: mov    r8, qword ptr [rip + 0x1bf89d0]          ; [0x3cccd08] metamethod:Method$System.Collections.Generic.List<ʲʲʺʴʺʼʷʸʾʷʹ>.Add()
020d4338: inc    edi
020d433a: mov    rdx, rbx
020d433d: mov    dword ptr [rbp - 0x29], edi
020d4340: mov    rcx, rsi
020d4343: call   0x182c425f0
020d4348: mov    rcx, qword ptr [rbp - 0x11]
020d434c: mov    eax, r12d
020d434f: jmp    0x1820d4080
020d4354: mov    rax, qword ptr [rbp + 0x67]
020d4358: xor    r8d, r8d
020d435b: mov    rcx, rsi
020d435e: movsd  xmm1, qword ptr [rax + 0xb0]
020d4366: call   0x1820d2930                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʸʺˁʵʳʴʺʸˀʴʴ
020d436b: movaps xmm7, xmmword ptr [rsp + 0x50]
020d4370: mov    rax, rsi
020d4373: movaps xmm6, xmmword ptr [rsp + 0x60]
020d4378: mov    r15, qword ptr [rsp + 0x70]
020d437d: mov    r14, qword ptr [rsp + 0x78]
020d4382: mov    r13, qword ptr [rsp + 0x80]
020d438a: mov    r12, qword ptr [rsp + 0x88]
020d4392: mov    rdi, qword ptr [rsp + 0xb8]
020d439a: add    rsp, 0x90
020d43a1: pop    rsi
020d43a2: pop    rbx
020d43a3: pop    rbp
020d43a4: ret    
020d43a5: call   0x182f60c50
020d43aa: int3   
020d43ab: int3   
