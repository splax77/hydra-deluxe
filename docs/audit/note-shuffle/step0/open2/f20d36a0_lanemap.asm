020d36a0: mov    qword ptr [rsp + 0x10], rbx
020d36a5: mov    qword ptr [rsp + 0x18], rbp
020d36aa: push   rsi
020d36ab: push   r12
020d36ad: push   r13
020d36af: push   r14
020d36b1: push   r15
020d36b3: sub    rsp, 0x70
020d36b7: mov    r10, rdx
020d36ba: cmp    r8b, 0xe
020d36be: ja     0x1820d36d7
020d36c0: lea    rdx, [rip - 0x20d36c7]                   ; [0x0] (bss)
020d36c7: movsx  rax, r8b
020d36cb: mov    eax, dword ptr [rdx + rax*4 + 0x20d3914]
020d36d2: add    rax, rdx
020d36d5: jmp    rax
020d36d7: test   r10, r10
020d36da: je     0x1820d3907
020d36e0: mov    rdx, rcx
020d36e3: xor    r8d, r8d
020d36e6: mov    rcx, r10
020d36e9: call   0x18214c680                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ʾʵʼˁʽʲʹʳʳʻʹ
020d36ee: jmp    0x1820d38ed
020d36f3: test   r10, r10
020d36f6: je     0x1820d3907
020d36fc: mov    ecx, dword ptr [r10 + 0x20]
020d3700: sub    ecx, 0xd
020d3703: je     0x1820d37b0
020d3709: sub    ecx, 1
020d370c: je     0x1820d37a3
020d3712: sub    ecx, 1
020d3715: je     0x1820d377b
020d3717: sub    ecx, 1
020d371a: je     0x1820d3753
020d371c: cmp    ecx, 1
020d371f: je     0x1820d372b
020d3721: xor    ebx, ebx
020d3723: movzx  eax, bx
020d3726: jmp    0x1820d38ed
020d372b: xor    edx, edx
020d372d: mov    rcx, r10
020d3730: call   0x18214dca0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʸʷʽʸʸʼʼʲʾʼ
020d3735: test   al, al
020d3737: jne    0x1820d3746
020d3739: mov    ebx, 0x10
020d373e: movzx  eax, bx
020d3741: jmp    0x1820d38ed
020d3746: mov    ebx, 0x80
020d374b: movzx  eax, bx
020d374e: jmp    0x1820d38ed
020d3753: xor    edx, edx
020d3755: mov    rcx, r10
020d3758: call   0x18214dca0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʸʷʽʸʸʼʼʲʾʼ
020d375d: test   al, al
020d375f: jne    0x1820d376e
020d3761: mov    ebx, 8
020d3766: movzx  eax, bx
020d3769: jmp    0x1820d38ed
020d376e: mov    ebx, 0x40
020d3773: movzx  eax, bx
020d3776: jmp    0x1820d38ed
020d377b: xor    edx, edx
020d377d: mov    rcx, r10
020d3780: call   0x18214dca0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʸʷʽʸʸʼʼʲʾʼ
020d3785: test   al, al
020d3787: jne    0x1820d3796
020d3789: mov    ebx, 4
020d378e: movzx  eax, bx
020d3791: jmp    0x1820d38ed
020d3796: mov    ebx, 0x20
020d379b: movzx  eax, bx
020d379e: jmp    0x1820d38ed
020d37a3: mov    ebx, 2
020d37a8: movzx  eax, bx
020d37ab: jmp    0x1820d38ed
020d37b0: mov    ebx, 1
020d37b5: movzx  eax, bx
020d37b8: jmp    0x1820d38ed
020d37bd: xor    eax, eax
020d37bf: mov    qword ptr [rsp + 0xa0], rdi
020d37c7: mov    qword ptr [rsp + 0x30], rax
020d37cc: xorps  xmm0, xmm0
020d37cf: mov    qword ptr [rsp + 0x48], rax
020d37d4: xorps  xmm1, xmm1
020d37d7: movups xmmword ptr [rsp + 0x20], xmm0
020d37dc: movups xmmword ptr [rsp + 0x38], xmm1
020d37e1: test   r10, r10
020d37e4: je     0x1820d390d
020d37ea: mov    r8, rcx
020d37ed: xor    ebx, ebx
020d37ef: lea    rcx, [rsp + 0x50]
020d37f4: xor    r9d, r9d
020d37f7: mov    rdx, r10
020d37fa: movzx  edi, bx
020d37fd: call   0x18214c5e0                              ; ʾʳʿʽʸʸʷʾʼʴʿ$$ˁʳʿʴʾʲˁʾʼʹʿ
020d3802: xor    r8d, r8d
020d3805: lea    rdx, [rsp + 0x38]
020d380a: lea    rcx, [rsp + 0x50]
020d380f: movups xmm0, xmmword ptr [rax]
020d3812: movups xmmword ptr [rsp + 0x38], xmm0
020d3817: movsd  xmm1, qword ptr [rax + 0x10]
020d381c: movsd  qword ptr [rsp + 0x48], xmm1
020d3822: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d3827: xor    edx, edx
020d3829: lea    rcx, [rsp + 0x20]
020d382e: movups xmm0, xmmword ptr [rax]
020d3831: movups xmmword ptr [rsp + 0x20], xmm0
020d3836: movsd  xmm1, qword ptr [rax + 0x10]
020d383b: movsd  qword ptr [rsp + 0x30], xmm1
020d3841: call   0x18213b870                              ; ʾʳʿʽʸʸʷʾʼʴʿ.ʴʷˀʹʹʵʿʳʻˁʼ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d3846: test   al, al
020d3848: je     0x1820d38e2
020d384e: mov    esi, 2
020d3853: mov    ebp, 1
020d3858: mov    r12d, 0x10
020d385e: mov    r15d, 8
020d3864: mov    r14d, 4
020d386a: mov    r13d, 0x20
020d3870: xor    edx, edx
020d3872: lea    rcx, [rsp + 0x20]
020d3877: call   0x18213b790                              ; ʾʳʿʽʸʸʷʾʼʴʿ.ʴʷˀʹʹʵʿʳʻˁʼ$$ʶʼʵʲʸʳʻʴʴʹʹ
020d387c: test   rax, rax
020d387f: je     0x1820d390d
020d3885: mov    eax, dword ptr [rax + 0x20]
020d3888: dec    eax
020d388a: cmp    eax, 0xb
020d388d: ja     0x1820d38cc
020d388f: lea    rdx, [rip - 0x20d3896]                   ; [0x0] (bss)
020d3896: cdqe   
020d3898: mov    ecx, dword ptr [rdx + rax*4 + 0x20d3950]
020d389f: add    rcx, rdx
020d38a2: jmp    rcx
020d38a4: movzx  eax, bp
020d38a7: jmp    0x1820d38cf
020d38a9: mov    eax, esi
020d38ab: jmp    0x1820d38cf
020d38ad: movzx  eax, r14w
020d38b1: jmp    0x1820d38cf
020d38b3: movzx  eax, r15w
020d38b7: jmp    0x1820d38cf
020d38b9: movzx  eax, r12w
020d38bd: jmp    0x1820d38cf
020d38bf: movzx  eax, r13w
020d38c3: jmp    0x1820d38cf
020d38c5: mov    eax, 0x40
020d38ca: jmp    0x1820d38cf
020d38cc: movzx  eax, bx
020d38cf: xor    edx, edx
020d38d1: lea    rcx, [rsp + 0x20]
020d38d6: or     di, ax
020d38d9: call   0x18213b870                              ; ʾʳʿʽʸʸʷʾʼʴʿ.ʴʷˀʹʹʵʿʳʻˁʼ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d38de: test   al, al
020d38e0: jne    0x1820d3870
020d38e2: movzx  eax, di
020d38e5: mov    rdi, qword ptr [rsp + 0xa0]
020d38ed: lea    r11, [rsp + 0x70]
020d38f2: mov    rbx, qword ptr [r11 + 0x38]
020d38f6: mov    rbp, qword ptr [r11 + 0x40]
020d38fa: mov    rsp, r11
020d38fd: pop    r15
020d38ff: pop    r14
020d3901: pop    r13
020d3903: pop    r12
020d3905: pop    rsi
020d3906: ret    
020d3907: call   0x182f60c50
020d390c: int3   
020d390d: call   0x182f60c50
020d3912: int3   
020d3913: nop    
020d3914: xlatb  
020d3915: or     eax, 0xd36d702
020d391b: add    dl, bh
020d391d: or     eax, 0xd36d702
020d3923: add    bh, byte ptr [rbp - 0x42fdf2c9]
