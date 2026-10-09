02152970: mov    qword ptr [rsp + 0x18], rbx
02152975: push   rbp
02152976: push   rsi
02152977: push   r15
02152979: sub    rsp, 0x80
02152980: cmp    byte ptr [rip + 0x1dc6dd2], 0            ; [0x3f19759] (bss)
02152987: mov    rbx, r9
0215298a: movzx  r15d, r8b
0215298e: movzx  esi, dl
02152991: mov    rbp, rcx
02152994: jne    0x1821529a9
02152996: lea    rcx, [rip + 0x1b4d5fb]                   ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
0215299d: call   0x182f609b0
021529a2: mov    byte ptr [rip + 0x1dc6db0], 1            ; [0x3f19759] (bss)
021529a9: test   rbx, rbx
021529ac: je     0x182152b01
021529b2: cmp    byte ptr [rbx + 0x21], 0xff
021529b6: mov    qword ptr [rsp + 0xa0], rdi
021529be: je     0x182152a80
021529c4: mov    qword ptr [rsp + 0xa8], r14
021529cc: movzx  ecx, byte ptr [rbx + 0x21]
021529d0: xor    edx, edx
021529d2: call   0x18210da00                              ; ʿʶʻʺʼʾʺʵʴʺʵ$$ʼʺʷʻʴʵˁʸˁʾʿ
021529d7: mov    r8, qword ptr [rbx + 0x28]
021529db: movzx  r14d, al
021529df: test   r8, r8
021529e2: je     0x182152b07
021529e8: mov    r8, qword ptr [r8 + 0x10]
021529ec: lea    rcx, [rsp + 0x60]
021529f1: mov    rdx, qword ptr [rbx + 0x10]
021529f5: xorps  xmm0, xmm0
021529f8: xor    edi, edi
021529fa: movzx  r9d, r15b
021529fe: mov    qword ptr [rsp + 0x30], rdi
02152a03: mov    dword ptr [rsp + 0x28], edi
02152a07: mov    byte ptr [rsp + 0x20], al
02152a0b: movups xmmword ptr [rsp + 0x60], xmm0
02152a10: movups xmmword ptr [rsp + 0x70], xmm0
02152a15: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
02152a1a: mov    rcx, qword ptr [rip + 0x1b4d577]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02152a21: movups xmm0, xmmword ptr [rsp + 0x60]
02152a26: movups xmm1, xmmword ptr [rsp + 0x70]
02152a2b: movups xmmword ptr [rsp + 0x40], xmm0
02152a30: movups xmmword ptr [rsp + 0x50], xmm1
02152a35: cmp    dword ptr [rcx + 0xe0], edi
02152a3b: jne    0x182152a42
02152a3d: call   0x182f60cf0
02152a42: lea    r9, [rsp + 0x40]
02152a47: mov    qword ptr [rsp + 0x20], rdi
02152a4c: movzx  r8d, r14b
02152a50: movzx  edx, sil
02152a54: mov    rcx, rbp
02152a57: call   0x1820cfb80                              ; ˀˀʴʸʵʷʾʵʻʼʹ$$ʷʶʺʺˁʿʾʹʽʼˁ
02152a5c: mov    r14, qword ptr [rsp + 0xa8]
02152a64: mov    rdi, qword ptr [rsp + 0xa0]
02152a6c: mov    rbx, qword ptr [rsp + 0xb0]
02152a74: add    rsp, 0x80
02152a7b: pop    r15
02152a7d: pop    rsi
02152a7e: pop    rbp
02152a7f: ret    
02152a80: mov    r8, qword ptr [rbx + 0x28]
02152a84: test   r8, r8
02152a87: je     0x182152b0d
02152a8d: mov    r8, qword ptr [r8 + 0x10]
02152a91: lea    rcx, [rsp + 0x60]
02152a96: mov    rdx, qword ptr [rbx + 0x10]
02152a9a: xorps  xmm0, xmm0
02152a9d: xor    edi, edi
02152a9f: movzx  r9d, r15b
02152aa3: mov    qword ptr [rsp + 0x30], rdi
02152aa8: mov    dword ptr [rsp + 0x28], edi
02152aac: mov    byte ptr [rsp + 0x20], 0xff
02152ab1: movups xmmword ptr [rsp + 0x60], xmm0
02152ab6: movups xmmword ptr [rsp + 0x70], xmm0
02152abb: call   0x18213e320                              ; ʸʻˁʴʿʶʶʳʸʶʳ$$.ctor
02152ac0: mov    rcx, qword ptr [rip + 0x1b4d4d1]         ; [0x3c9ff98] meta:ˀˀʴʸʵʷʾʵʻʼʹ_TypeInfo
02152ac7: movups xmm0, xmmword ptr [rsp + 0x60]
02152acc: movups xmm1, xmmword ptr [rsp + 0x70]
02152ad1: movups xmmword ptr [rsp + 0x40], xmm0
02152ad6: movups xmmword ptr [rsp + 0x50], xmm1
02152adb: cmp    dword ptr [rcx + 0xe0], edi
02152ae1: jne    0x182152ae8
02152ae3: call   0x182f60cf0
02152ae8: xor    r9d, r9d
02152aeb: lea    r8, [rsp + 0x40]
02152af0: movzx  edx, sil
02152af4: mov    rcx, rbp
02152af7: call   0x1820cfbc0
02152afc: jmp    0x182152a64
02152b01: call   0x182f60c50
02152b06: int3   
02152b07: call   0x182f60c50
02152b0c: int3   
02152b0d: call   0x182f60c50
02152b12: int3   
02152b13: int3   
