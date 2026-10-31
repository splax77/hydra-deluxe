020f6800: mov    qword ptr [rsp + 8], rbx
020f6805: mov    qword ptr [rsp + 0x10], rsi
020f680a: push   rdi
020f680b: sub    rsp, 0x40
020f680f: cmp    byte ptr [rcx + 0x42], 0
020f6813: mov    rdi, rcx
020f6816: movaps xmmword ptr [rsp + 0x30], xmm6
020f681b: jne    0x1820f685c
020f681d: mov    rcx, qword ptr [rcx + 0x68]
020f6821: sub    rcx, qword ptr [rdi + 0x1b0]
020f6828: test   rcx, rcx
020f682b: jle    0x1820f683f
020f682d: mov    rax, qword ptr [rdi + 0x1b0]
020f6834: sub    qword ptr [rdi + 0x80], rcx
020f683b: mov    qword ptr [rdi + 0x68], rax
020f683f: mov    rax, qword ptr [rdi + 0x68]
020f6843: mov    rbx, qword ptr [rsp + 0x50]
020f6848: mov    rsi, qword ptr [rsp + 0x58]
020f684d: movaps xmm6, xmmword ptr [rsp + 0x30]
020f6852: mov    qword ptr [rdi + 0x60], rax
020f6856: add    rsp, 0x40
020f685a: pop    rdi
020f685b: ret    
020f685c: mov    rax, qword ptr [rcx + 0x140]
020f6863: xor    edx, edx
020f6865: mov    qword ptr [rcx + 0x138], rax
020f686c: call   0x1820f3750                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʳʻʷʵˁʴʾʻʿʴʺ
020f6871: mov    rsi, qword ptr [rdi + 0x138]
020f6878: mov    rbx, rax
020f687b: sub    rbx, qword ptr [rdi + 0x118]
020f6882: cmp    byte ptr [rip + 0x1e22b2a], 0            ; [0x3f193b3] (bss)
020f6889: mov    qword ptr [rdi + 0x140], rbx
020f6890: jne    0x1820f68a5
020f6892: lea    rcx, [rip + 0x1c18407]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f6899: call   0x182f609b0
020f689e: mov    byte ptr [rip + 0x1e22b0e], 1            ; [0x3f193b3] (bss)
020f68a5: cmp    dword ptr [rdi + 0x190], 1
020f68ac: je     0x1820f6932
020f68b2: sub    rbx, rsi
020f68b5: jns    0x1820f68e0
020f68b7: mov    rcx, qword ptr [rdi + 0x228]
020f68be: test   rcx, rcx
020f68c1: je     0x1820f69e5
020f68c7: mov    rbx, qword ptr [rdi + 0x108]
020f68ce: xor    edx, edx
020f68d0: call   0x18215fed0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʿʼʿʿʵʵʲʿʷˁʻ
020f68d5: cmp    rbx, rax
020f68d8: jle    0x1820f69eb
020f68de: xor    ebx, ebx
020f68e0: cmp    rbx, qword ptr [rdi + 0x68]
020f68e4: jle    0x1820f68ea
020f68e6: mov    rbx, qword ptr [rdi + 0x68]
020f68ea: mov    rax, qword ptr [rdi + 0x68]
020f68ee: mov    r8, qword ptr [rdi + 0x1a0]
020f68f5: sub    rax, rbx
020f68f8: mov    qword ptr [rdi + 0x68], rax
020f68fc: test   r8, r8
020f68ff: je     0x1820f6932
020f6901: mov    rdx, qword ptr [rip + 0x1c18398]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f6908: xorps  xmm3, xmm3
020f690b: cvtsi2ss xmm3, rax
020f6910: movzx  eax, byte ptr [rdi + 0x42]
020f6914: mov    ecx, 0xf
020f6919: mov    byte ptr [rsp + 0x20], al
020f691d: xorps  xmm0, xmm0
020f6920: cvtsi2ss xmm0, qword ptr [rdi + 0x1b0]
020f6929: divss  xmm3, xmm0
020f692d: call   0x182efc240
020f6932: mov    rcx, qword ptr [rdi + 0x68]
020f6936: sub    rcx, qword ptr [rdi + 0x1b0]
020f693d: test   rcx, rcx
020f6940: jle    0x1820f695b
020f6942: mov    rax, qword ptr [rdi + 0x1b0]
020f6949: sub    qword ptr [rdi + 0x80], rcx
020f6950: mov    qword ptr [rdi + 0x68], rax
020f6954: mov    byte ptr [rdi + 0x130], 1
020f695b: cmp    byte ptr [rdi + 0x130], 0
020f6962: je     0x1820f683f
020f6968: mov    rdx, qword ptr [rdi + 0x140]
020f696f: xor    r9d, r9d
020f6972: add    rdx, qword ptr [rdi + 0x68]
020f6976: mov    rcx, rdi
020f6979: movsd  xmm2, qword ptr [rdi + 0x110]
020f6981: call   0x1820f37e0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʳʼʽʽʸʽʺʲʼʹʻ
020f6986: movsd  qword ptr [rdi + 0x120], xmm0
020f698e: xor    r8d, r8d
020f6991: mov    rcx, qword ptr [rdi + 0x208]
020f6998: movaps xmm1, xmm0
020f699b: movaps xmm6, xmm0
020f699e: call   0x18216a720                              ; ʿʴʴʾʾˁʲʹʲʾʳ$$ʳʴʼʼʽʸʲʹʼʹˀ
020f69a3: test   rax, rax
020f69a6: je     0x1820f69e5
020f69a8: mov    rcx, qword ptr [rdi + 0x230]
020f69af: test   rcx, rcx
020f69b2: je     0x1820f69e5
020f69b4: subsd  xmm6, qword ptr [rax + 0x18]
020f69b9: movsd  xmm2, qword ptr [rcx + 0xb0]
020f69c1: xor    r9d, r9d
020f69c4: mov    rdx, rax
020f69c7: movaps xmm0, xmm6
020f69ca: call   0x1820d1d20                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʷʾʻʿʵʻʸʵʺʹʺ
020f69cf: xor    edx, edx
020f69d1: mov    rcx, rdi
020f69d4: call   0x1820f3750                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʳʻʷʵˁʴʾʻʿʴʺ
020f69d9: mov    byte ptr [rdi + 0x130], 0
020f69e0: jmp    0x1820f683f
020f69e5: call   0x182f60c50
020f69ea: int3   
020f69eb: lea    rcx, [rip + 0x1bbfc56]                   ; [0x3cb6648] meta:System.ArgumentOutOfRangeException_TypeInfo
020f69f2: call   0x182f609d0
020f69f7: mov    rcx, rax
020f69fa: call   0x182f60c00
020f69ff: lea    rcx, [rip + 0x1bf68fa]                   ; [0x3ced300] str:'Star power drain amount must be greater than 0'
020f6a06: mov    rdi, rax
020f6a09: call   0x182f609d0
020f6a0e: lea    rcx, [rip + 0x1ba71b3]                   ; [0x3c9dbc8] str:'amount'
020f6a15: mov    rbx, rax
020f6a18: call   0x182f609d0
020f6a1d: xor    r9d, r9d
020f6a20: mov    r8, rbx
020f6a23: mov    rdx, rax
020f6a26: mov    rcx, rdi
020f6a29: call   0x181a8bdd0                              ; System.ArgumentOutOfRangeException$$.ctor
020f6a2e: lea    rcx, [rip + 0x1ba5beb]                   ; [0x3c9c620] metamethod:Method$ʿʶʴˀʴʾʵʵʶʳʲ.ʼʲʴʲˀʺʲʸʺʹʵ()
020f6a35: call   0x182f609d0
020f6a3a: mov    rdx, rax
020f6a3d: mov    rcx, rdi
020f6a40: call   0x182f60c10
020f6a45: int3   
020f6a46: int3   
