020f63f0: mov    qword ptr [rsp + 0x20], rbx
020f63f5: push   rdi
020f63f6: sub    rsp, 0x80
020f63fd: cmp    byte ptr [rip + 0x1e22fad], 0            ; [0x3f193b1] (bss)
020f6404: mov    rdi, rdx
020f6407: movaps xmmword ptr [rsp + 0x70], xmm6
020f640c: mov    rbx, rcx
020f640f: jne    0x1820f6430
020f6411: lea    rcx, [rip + 0x1c18888]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f6418: call   0x182f609b0
020f641d: lea    rcx, [rip + 0x1b9cb54]                   ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6424: call   0x182f609b0
020f6429: mov    byte ptr [rip + 0x1e22f81], 1            ; [0x3f193b1] (bss)
020f6430: mov    rcx, qword ptr [rip + 0x1b9cb41]         ; [0x3c92f78] meta:ʴˀʽʺˀʲʲʻʷʾʻ_TypeInfo
020f6437: xor    eax, eax
020f6439: movsd  xmm6, qword ptr [rbx + 0x100]
020f6441: xorps  xmm0, xmm0
020f6444: xorps  xmm1, xmm1
020f6447: mov    qword ptr [rsp + 0x30], rax
020f644c: movups xmmword ptr [rsp + 0x20], xmm0
020f6451: mov    qword ptr [rsp + 0x48], rax
020f6456: movups xmmword ptr [rsp + 0x38], xmm1
020f645b: cmp    dword ptr [rcx + 0xe0], eax
020f6461: jne    0x1820f6468
020f6463: call   0x182f60cf0
020f6468: xor    r8d, r8d
020f646b: mov    qword ptr [rsp + 0xa0], r14
020f6473: mov    edx, 1
020f6478: movaps xmm0, xmm6
020f647b: call   0x180f96aa0
020f6480: movaps xmm6, xmmword ptr [rsp + 0x70]
020f6485: movsd  qword ptr [rbx + 0x28], xmm0
020f648a: mov    byte ptr [rbx + 0x45], 0
020f648e: test   rdi, rdi
020f6491: je     0x1820f67f9
020f6497: xor    r8d, r8d
020f649a: lea    rcx, [rsp + 0x50]
020f649f: mov    rdx, rdi
020f64a2: mov    r14b, 1
020f64a5: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020f64aa: xor    r8d, r8d
020f64ad: lea    rdx, [rsp + 0x38]
020f64b2: lea    rcx, [rsp + 0x50]
020f64b7: movups xmm0, xmmword ptr [rax]
020f64ba: movups xmmword ptr [rsp + 0x38], xmm0
020f64bf: movsd  xmm1, qword ptr [rax + 0x10]
020f64c4: movsd  qword ptr [rsp + 0x48], xmm1
020f64ca: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020f64cf: movups xmm0, xmmword ptr [rax]
020f64d2: movups xmmword ptr [rsp + 0x20], xmm0
020f64d7: movsd  xmm1, qword ptr [rax + 0x10]
020f64dc: movsd  qword ptr [rsp + 0x30], xmm1
020f64e2: xor    edx, edx
020f64e4: lea    rcx, [rsp + 0x20]
020f64e9: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020f64ee: test   al, al
020f64f0: je     0x1820f650e
020f64f2: mov    rcx, qword ptr [rsp + 0x28]
020f64f7: test   rcx, rcx
020f64fa: je     0x1820f67f9
020f6500: xor    edx, edx
020f6502: call   0x18214b610                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʲʺʾʽʽʹʳʳʼʹ
020f6507: test   al, al
020f6509: jne    0x1820f64e2
020f650b: xor    r14b, r14b
020f650e: cmp    dword ptr [rbx + 0x190], 1
020f6515: je     0x1820f673d
020f651b: inc    dword ptr [rbx + 0x9c]
020f6521: xor    r8d, r8d
020f6524: inc    dword ptr [rbx + 0x88]
020f652a: mov    dl, 1
020f652c: mov    rcx, rbx
020f652f: call   0x1820f4e30                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʷʸˁʾʿʹʿʽʲˀʾ
020f6534: mov    rax, qword ptr [rbx]
020f6537: mov    rcx, rbx
020f653a: mov    rdx, qword ptr [rax + 0x1e0]
020f6541: call   qword ptr [rax + 0x1d8]
020f6547: movsd  xmm0, qword ptr [rbx + 0x110]
020f654f: comisd xmm0, xmmword ptr [rdi + 0x28]
020f6554: jb     0x1820f6581
020f6556: cmp    byte ptr [rbx + 0x42], 0
020f655a: je     0x1820f6581
020f655c: movsd  xmm0, qword ptr [rbx + 0x100]
020f6564: comisd xmm0, xmmword ptr [rbx + 0x110]
020f656c: jb     0x1820f6581
020f656e: inc    dword ptr [rbx + 0xe8]
020f6574: xor    r8d, r8d
020f6577: mov    dl, 1
020f6579: mov    rcx, rdi
020f657c: call   0x18214b6e0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵˁʾˁʻˁʹʷʻʷʿ
020f6581: movsd  xmm0, qword ptr [rdi + 0x28]
020f6586: comisd xmm0, xmmword ptr [rbx + 0x120]
020f658e: jbe    0x1820f65bb
020f6590: cmp    byte ptr [rbx + 0x42], 0
020f6594: je     0x1820f65bb
020f6596: movsd  xmm0, qword ptr [rbx + 0x120]
020f659e: comisd xmm0, xmmword ptr [rbx + 0x100]
020f65a6: jbe    0x1820f65bb
020f65a8: inc    dword ptr [rbx + 0xe8]
020f65ae: xor    r8d, r8d
020f65b1: mov    dl, 1
020f65b3: mov    rcx, rdi
020f65b6: call   0x18214b6e0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵˁʾˁʻˁʹʷʻʷʿ
020f65bb: movsd  xmm0, qword ptr [rdi + 0x28]
020f65c0: addsd  xmm0, qword ptr [rbx + 0x10]
020f65c5: comisd xmm0, xmmword ptr [rbx + 0x128]
020f65cd: jbe    0x1820f6600
020f65cf: movsd  xmm1, qword ptr [rdi + 0x28]
020f65d4: subsd  xmm1, qword ptr [rbx + 0x10]
020f65d9: movsd  xmm0, qword ptr [rbx + 0x128]
020f65e1: comisd xmm0, xmm1
020f65e5: jb     0x1820f6600
020f65e7: cmp    byte ptr [rbx + 0x42], 0
020f65eb: jne    0x1820f6600
020f65ed: inc    dword ptr [rbx + 0xec]
020f65f3: xor    r8d, r8d
020f65f6: mov    dl, 1
020f65f8: mov    rcx, rdi
020f65fb: call   0x18214b710                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʵˁˁˀʽʴʹʳʲʴˀ
020f6600: mov    eax, dword ptr [rbx + 0x98]
020f6606: cmp    dword ptr [rbx + 0x88], eax
020f660c: jle    0x1820f661a
020f660e: mov    eax, dword ptr [rbx + 0x88]
020f6614: mov    dword ptr [rbx + 0x98], eax
020f661a: xor    edx, edx
020f661c: mov    rcx, rdi
020f661f: call   0x18214b630                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʴʽʻʻʶˁʼʾˀʽʽ
020f6624: test   al, al
020f6626: je     0x1820f6638
020f6628: cmp    byte ptr [rbx + 0x43], 0
020f662c: jne    0x1820f6642
020f662e: xor    edx, edx
020f6630: mov    rcx, rbx
020f6633: call   0x1820f62c0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʽˀʺʺʺʷʶʸʶʲʺ
020f6638: cmp    byte ptr [rbx + 0x43], 0
020f663c: je     0x1820f673d
020f6642: cmp    byte ptr [rip + 0x1e22d62], 0            ; [0x3f193ab] (bss)
020f6649: mov    qword ptr [rsp + 0x90], rbp
020f6651: mov    qword ptr [rsp + 0x98], rsi
020f6659: jne    0x1820f666e
020f665b: lea    rcx, [rip + 0x1c1863e]                   ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f6662: call   0x182f609b0
020f6667: mov    byte ptr [rip + 0x1e22d3d], 1            ; [0x3f193ab] (bss)
020f666e: mov    eax, dword ptr [rbx + 0x168]
020f6674: movd   xmm0, dword ptr [rbx + 0x16c]
020f667c: inc    eax
020f667e: mov    rsi, qword ptr [rbx + 0x1a0]
020f6685: mov    dword ptr [rbx + 0x168], eax
020f668b: cvtdq2ps xmm0, xmm0
020f668e: movd   xmm1, eax
020f6692: cvtdq2ps xmm1, xmm1
020f6695: divss  xmm1, xmm0
020f6699: mulss  xmm1, dword ptr [rip + 0xf6d95b]         ; [0x3063ffc] flt=100.0 i=1120403456
020f66a1: cvttss2si ebp, xmm1
020f66a5: mov    dword ptr [rbx + 0x164], ebp
020f66ab: test   rsi, rsi
020f66ae: je     0x1820f6714
020f66b0: mov    r10, qword ptr [rsi]
020f66b3: xor    eax, eax
020f66b5: mov    r9, qword ptr [rip + 0x1c185e4]          ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f66bc: movzx  edx, word ptr [r10 + 0x12e]
020f66c4: cmp    ax, dx
020f66c7: jae    0x1820f66f8
020f66c9: mov    r8, qword ptr [r10 + 0xb0]
020f66d0: xor    ecx, ecx
020f66d2: nop    dword ptr [rax]
020f66d6: nop    word ptr [rax + rax]
020f66e0: movzx  eax, cx
020f66e3: add    rax, rax
020f66e6: cmp    qword ptr [r8 + rax*8], r9
020f66ea: je     0x1820f67d7
020f66f0: inc    cx
020f66f3: cmp    cx, dx
020f66f6: jb     0x1820f66e0
020f66f8: mov    r8d, 7
020f66fe: mov    rdx, r9
020f6701: mov    rcx, rsi
020f6704: call   0x182f65070
020f6709: mov    r8, qword ptr [rax + 8]
020f670d: mov    edx, ebp
020f670f: mov    rcx, rsi
020f6712: call   qword ptr [rax]
020f6714: xor    edx, edx
020f6716: mov    rcx, rdi
020f6719: call   0x18214b7d0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʸʹʲʳʻʾʺʷʲˁʶ
020f671e: mov    rsi, qword ptr [rsp + 0x98]
020f6726: mov    rbp, qword ptr [rsp + 0x90]
020f672e: test   r14b, al
020f6731: je     0x1820f673d
020f6733: xor    edx, edx
020f6735: mov    rcx, rbx
020f6738: call   0x1820f7610                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ˁʺʸʿʽʼʸʴʺʹˁ
020f673d: cmp    byte ptr [rbx + 0x46], 0
020f6741: jne    0x1820f6750
020f6743: xor    edx, edx
020f6745: mov    rcx, rdi
020f6748: call   0x18214be20                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʶˀʼʼʾʿʽʺʻʵ
020f674d: mov    byte ptr [rbx + 0x46], al
020f6750: xor    edx, edx
020f6752: mov    rcx, rdi
020f6755: call   0x18214b5f0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʲˁʹʸʸʺʶʼʽʸʸ
020f675a: test   al, al
020f675c: je     0x1820f679e
020f675e: cmp    byte ptr [rbx + 0x46], 0
020f6762: je     0x1820f679e
020f6764: test   r14b, r14b
020f6767: je     0x1820f679e
020f6769: cmp    byte ptr [rbx + 0x196], 0
020f6770: je     0x1820f679a
020f6772: and    dword ptr [rdi + 0x7c], 0xfffffeff
020f6779: mov    r8b, 1
020f677c: mov    rax, qword ptr [rbx]
020f677f: mov    rcx, rbx
020f6782: mov    rdx, qword ptr [rbx + 0x1c0]
020f6789: mov    r9, qword ptr [rax + 0x1b0]
020f6790: call   qword ptr [rax + 0x1a8]
020f6796: mov    byte ptr [rbx + 0x45], 1
020f679a: mov    byte ptr [rbx + 0x46], 0
020f679e: mov    r8, qword ptr [rbx + 0x1a0]
020f67a5: test   r8, r8
020f67a8: je     0x1820f67be
020f67aa: mov    rdx, qword ptr [rip + 0x1c184ef]         ; [0x3d0eca0] meta:ʳʴʹʻʾʲʹʹʽʽʷ_TypeInfo
020f67b1: mov    ecx, 5
020f67b6: mov    r9, rdi
020f67b9: call   0x182c42260
020f67be: mov    r14, qword ptr [rsp + 0xa0]
020f67c6: mov    rbx, qword ptr [rsp + 0xa8]
020f67ce: add    rsp, 0x80
020f67d5: pop    rdi
020f67d6: ret    
020f67d7: movzx  ecx, cx
020f67da: add    rcx, rcx
020f67dd: mov    eax, dword ptr [r8 + rcx*8 + 8]
020f67e2: add    eax, 7
020f67e5: cdqe   
020f67e7: shl    rax, 4
020f67eb: add    rax, 0x138
020f67f1: add    rax, r10
020f67f4: jmp    0x1820f6709
020f67f9: call   0x182f60c50
020f67fe: int3   
020f67ff: int3   
