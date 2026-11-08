0035f6e0: mov    byte ptr [rsp + 0x10], dl
0035f6e4: mov    qword ptr [rsp + 8], rcx
0035f6e9: push   rbx
0035f6ea: sub    rsp, 0x60
0035f6ee: cmp    byte ptr [rip + 0x3bae952], 0            ; [0x3f0e047] (bss)
0035f6f5: jne    0x18035f70a
0035f6f7: lea    rcx, [rip + 0x3940cd2]                   ; [0x3ca03d0] meta:ˁʴʻʹʵʶʲʺʸˁʾ_TypeInfo
0035f6fe: call   0x182f609b0
0035f703: mov    byte ptr [rip + 0x3bae93d], 1            ; [0x3f0e047] (bss)
0035f70a: mov    byte ptr [rsp + 0x88], 0
0035f712: mov    byte ptr [rsp + 0x20], 0
0035f717: mov    qword ptr [rsp + 0x40], 0
0035f720: lea    rax, [rsp + 0x78]
0035f725: mov    qword ptr [rsp + 0x48], rax
0035f72a: lea    rax, [rsp + 0x20]
0035f72f: mov    qword ptr [rsp + 0x50], rax
0035f734: lea    rax, [rsp + 0x70]
0035f739: mov    qword ptr [rsp + 0x58], rax
0035f73e: mov    rcx, qword ptr [rsp + 0x70]
0035f743: xor    edx, edx
0035f745: cmp    byte ptr [rcx + 0xb3], dl
0035f74b: jne    0x18035f7ab
0035f74d: call   0x1803602c0                              ; SongEntry$$get_ChartPath
0035f752: mov    rbx, rax
0035f755: mov    rcx, qword ptr [rip + 0x3940c74]         ; [0x3ca03d0] meta:ˁʴʻʹʵʶʲʺʸˁʾ_TypeInfo
0035f75c: cmp    dword ptr [rcx + 0xe0], 0
0035f763: jne    0x18035f76a
0035f765: call   0x182f60cf0
0035f76a: xor    r8d, r8d
0035f76d: mov    rdx, rbx
0035f770: lea    rcx, [rsp + 0x30]
0035f775: call   0x1801313f0                              ; ˁʴʻʹʵʶʲʺʸˁʾ$$ˁʹʷʼʴʿʶʴʹʽʽ
0035f77a: mov    r8, rax
0035f77d: mov    rax, qword ptr [rax]
0035f780: mov    r8d, dword ptr [r8 + 8]
0035f784: mov    byte ptr [rsp + 0x20], 1
0035f789: test   rax, rax
0035f78c: je     0x18035f793
0035f78e: test   r8d, r8d
0035f791: jne    0x18035f7da
0035f793: xor    bl, bl
0035f795: mov    byte ptr [rsp + 0x88], bl
0035f79c: lea    rcx, [rsp + 0x48]
0035f7a1: call   0x18034ddf0
0035f7a6: jmp    0x18035f848
0035f7ab: call   0x180360a20                              ; SongEntry$$get_Sng
0035f7b0: test   rax, rax
0035f7b3: je     0x18035f85c
0035f7b9: xor    r8d, r8d
0035f7bc: mov    rdx, qword ptr [rsp + 0x70]
0035f7c1: mov    rdx, qword ptr [rdx + 0x78]
0035f7c5: mov    rcx, rax
0035f7c8: call   0x180262100                              ; ʵʻʾˁʸʶʿʸʸʽʾ$$ʲʻʺʹʴʷˁʽʲʷʺ
0035f7cd: test   rax, rax
0035f7d0: je     0x18035f857
0035f7d6: mov    r8d, dword ptr [rax + 0x18]
0035f7da: mov    rbx, qword ptr [rsp + 0x70]
0035f7df: lea    rcx, [rbx + 0x38]
0035f7e3: xor    r9d, r9d
0035f7e6: mov    rdx, rax
0035f7e9: call   0x1820fa8f0                              ; ˁʶʾʿʲʵʷʲʼʵʻ$$ʽʳʸʿʷʽʼˁʸʻʼ
0035f7ee: mov    rax, qword ptr [rbx + 0x60]
0035f7f2: test   rax, rax
0035f7f5: je     0x18035f851
0035f7f7: movups xmm0, xmmword ptr [rbx + 0x38]
0035f7fb: movups xmmword ptr [rax + 0x10], xmm0
0035f7ff: mov    rcx, qword ptr [rsp + 0x70]
0035f804: mov    qword ptr [rcx + 0x48], 0
0035f80c: add    rcx, 0x48
0035f810: xor    edx, edx
0035f812: call   0x182f5fc00
0035f817: mov    bl, 1
0035f819: mov    byte ptr [rsp + 0x88], bl
0035f820: lea    rcx, [rsp + 0x48]
0035f825: call   0x18034ddf0
0035f82a: jmp    0x18035f848
0035f82c: lea    rcx, [rsp + 0x48]
0035f831: call   0x18034ddf0
0035f836: mov    rcx, qword ptr [rsp + 0x40]
0035f83b: test   rcx, rcx
0035f83e: jne    0x18035f862
0035f840: movzx  ebx, byte ptr [rsp + 0x88]
0035f848: movzx  eax, bl
0035f84b: add    rsp, 0x60
0035f84f: pop    rbx
0035f850: ret    
0035f851: call   0x182f60c50
0035f856: nop    
0035f857: call   0x182f60c50
0035f85c: call   0x182f60c50
0035f861: nop    
0035f862: call   0x182f60ce0
0035f867: int3   
0035f868: int3   
