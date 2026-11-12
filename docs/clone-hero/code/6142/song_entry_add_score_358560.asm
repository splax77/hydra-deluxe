00358560: mov    qword ptr [rsp + 8], rbx
00358565: push   rdi
00358566: sub    rsp, 0x60
0035856a: cmp    byte ptr [rip + 0x3bb5ae6], 0            ; [0x3f0e057] (bss)
00358571: mov    rbx, rdx
00358574: mov    rdi, rcx
00358577: jne    0x18035858c
00358579: lea    rcx, [rip + 0x39457c0]                   ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
00358580: call   0x182f609b0
00358585: mov    byte ptr [rip + 0x3bb5acb], 1            ; [0x3f0e057] (bss)
0035858c: mov    rdx, qword ptr [rdi + 0x60]
00358590: xorps  xmm0, xmm0
00358593: movups xmmword ptr [rsp + 0x20], xmm0
00358598: movups xmmword ptr [rsp + 0x30], xmm0
0035859d: test   rdx, rdx
003585a0: je     0x180358633
003585a6: movzx  r8d, byte ptr [rbx]
003585aa: lea    rcx, [rsp + 0x40]
003585af: xor    r9d, r9d
003585b2: call   0x1802165a0                              ; ʽʵʳʲʺˀˀʷʵʸˀ$$ʼʻʹʵʵʻʸʹʹʼʼ
003585b7: mov    rcx, qword ptr [rdi + 0x60]
003585bb: movups xmm0, xmmword ptr [rax]
003585be: movups xmm1, xmmword ptr [rax + 0x10]
003585c2: movups xmmword ptr [rsp + 0x20], xmm0
003585c7: movups xmmword ptr [rsp + 0x30], xmm1
003585cc: test   rcx, rcx
003585cf: je     0x180358633
003585d1: movups xmm0, xmmword ptr [rbx]
003585d4: xor    r8d, r8d
003585d7: lea    rdx, [rsp + 0x40]
003585dc: movups xmm1, xmmword ptr [rbx + 0x10]
003585e0: movaps xmmword ptr [rsp + 0x40], xmm0
003585e5: movaps xmmword ptr [rsp + 0x50], xmm1
003585ea: call   0x1802161e0                              ; ʽʵʳʲʺˀˀʷʵʸˀ$$ʺʻˀˁʸˀʸʻˀʸʲ
003585ef: mov    rcx, qword ptr [rip + 0x394574a]         ; [0x3c9dd40] meta:ʿʲʽʿʶʾʻʾʺʸʽ_TypeInfo
003585f6: cmp    dword ptr [rcx + 0xe0], 0
003585fd: jne    0x180358604
003585ff: call   0x182f60cf0
00358604: xor    ecx, ecx
00358606: call   0x1802f5a40                              ; ʿʲʽʿʶʾʻʾʺʸʽ$$ʸʾʶˁʲʲʹʼʻʺʶ
0035860b: xor    edx, edx
0035860d: mov    rcx, rbx
00358610: call   0x1802ba7a0                              ; ˀʵʳʲʺʽʵˁʹʺʼ$$ʳʼʿʻʼʶʽʴʷʸʴ
00358615: xor    edx, edx
00358617: lea    rcx, [rsp + 0x20]
0035861c: mov    ebx, eax
0035861e: call   0x1802ba7a0                              ; ˀʵʳʲʺʽʵˁʹʺʼ$$ʳʼʿʻʼʶʽʴʷʸʴ
00358623: cmp    ebx, eax
00358625: mov    rbx, qword ptr [rsp + 0x70]
0035862a: setg   al
0035862d: add    rsp, 0x60
00358631: pop    rdi
00358632: ret    
00358633: call   0x182f60c50
00358638: int3   
00358639: int3   
