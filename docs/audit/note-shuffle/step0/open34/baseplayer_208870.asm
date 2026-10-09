00208870: push   rbx
00208872: sub    rsp, 0x130
00208879: cmp    byte ptr [rip + 0x3d04468], 0            ; [0x3f0cce8] (bss)
00208880: mov    rbx, rcx
00208883: jne    0x1802088a4
00208885: lea    rcx, [rip + 0x3ae7004]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
0020888c: call   0x182f609b0
00208891: lea    rcx, [rip + 0x3a8fa88]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
00208898: call   0x182f609b0
0020889d: mov    byte ptr [rip + 0x3d04444], 1            ; [0x3f0cce8] (bss)
002088a4: mov    rax, qword ptr [rip + 0x3ae6fe5]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002088ab: cmp    dword ptr [rax + 0xe0], 0
002088b2: jne    0x1802088c3
002088b4: mov    rcx, rax
002088b7: call   0x182f60cf0
002088bc: mov    rax, qword ptr [rip + 0x3ae6fcd]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
002088c3: mov    rax, qword ptr [rax + 0xb8]
002088ca: mov    qword ptr [rsp + 0x140], rsi
002088d2: mov    qword ptr [rsp + 0x148], rdi
002088da: mov    rcx, qword ptr [rax + 8]
002088de: test   rcx, rcx
002088e1: je     0x180208b41
002088e7: movzx  esi, byte ptr [rcx + 0x72]
002088eb: xor    edx, edx
002088ed: mov    rax, qword ptr [rip + 0x3a8fa2c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
002088f4: mov    rcx, qword ptr [rax + 0xb8]
002088fb: mov    rcx, qword ptr [rcx + 0x118]
00208902: call   0x18210bf30                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʴˁʺʸʲʶʸʵʹʹʿ
00208907: test   sil, sil
0020890a: jne    0x180208911
0020890c: mov    dil, 1
0020890f: jmp    0x180208918
00208911: movzx  edi, sil
00208915: and    dil, al
00208918: xor    r8d, r8d
0020891b: lea    rcx, [rsp + 0x40]
00208920: mov    rdx, rbx
00208923: call   0x1800b83b0                              ; GameManager$$ʶˁʳʲʺʻʷʴʿʴˀ
00208928: mov    r8, qword ptr [rbx + 0x20]
0020892c: test   r8, r8
0020892f: je     0x180208b41
00208935: movsd  xmm0, qword ptr [rax]
00208939: lea    rdx, [rsp + 0x30]
0020893e: mov    eax, dword ptr [rax + 8]
00208941: lea    rcx, [rsp + 0x50]
00208946: mov    r8, qword ptr [r8 + 0xa8]
0020894d: movzx  r9d, dil
00208951: movaps xmmword ptr [rsp + 0x120], xmm6
00208959: movaps xmmword ptr [rsp + 0x110], xmm7
00208961: movaps xmmword ptr [rsp + 0x100], xmm8
0020896a: movaps xmmword ptr [rsp + 0xf0], xmm9
00208973: movsd  qword ptr [rsp + 0x30], xmm0
00208979: mov    dword ptr [rsp + 0x38], eax
0020897d: mov    qword ptr [rsp + 0x20], 0
00208986: call   0x1820d2ea0                              ; ʲʽʶʳʺʹʳˀʾʴʹ$$ʹʹʾʴʳʶʼʵʳʼʼ
0020898b: lea    rcx, [rbx + 0x140]
00208992: xor    edx, edx
00208994: movups xmm0, xmmword ptr [rax]
00208997: movups xmm1, xmmword ptr [rax + 0x10]
0020899b: movups xmm2, xmmword ptr [rax + 0x20]
0020899f: movups xmm3, xmmword ptr [rax + 0x30]
002089a3: movups xmm4, xmmword ptr [rax + 0x40]
002089a7: movups xmm5, xmmword ptr [rax + 0x50]
002089ab: movups xmm6, xmmword ptr [rax + 0x60]
002089af: movups xmm7, xmmword ptr [rax + 0x70]
002089b3: movups xmm8, xmmword ptr [rax + 0x80]
002089bb: movups xmm9, xmmword ptr [rax + 0x90]
002089c3: movups xmmword ptr [rbx + 0x140], xmm0
002089ca: movups xmmword ptr [rbx + 0x150], xmm1
002089d1: movups xmmword ptr [rbx + 0x160], xmm2
002089d8: movups xmmword ptr [rbx + 0x170], xmm3
002089df: movups xmmword ptr [rbx + 0x180], xmm4
002089e6: movups xmmword ptr [rbx + 0x190], xmm5
002089ed: movups xmmword ptr [rbx + 0x1a0], xmm6
002089f4: movups xmmword ptr [rbx + 0x1b0], xmm7
002089fb: movups xmmword ptr [rbx + 0x1c0], xmm8
00208a03: movups xmmword ptr [rbx + 0x1d0], xmm9
00208a0b: call   0x182f5fc00
00208a10: movups xmm0, xmmword ptr [rbx + 0x140]
00208a17: mov    rax, qword ptr [rbx]
00208a1a: lea    rdx, [rsp + 0x50]
00208a1f: movups xmm1, xmmword ptr [rbx + 0x150]
00208a26: mov    rcx, rbx
00208a29: movups xmmword ptr [rsp + 0x50], xmm0
00208a2e: mov    r8, qword ptr [rax + 0x3d0]
00208a35: movups xmm0, xmmword ptr [rbx + 0x160]
00208a3c: movups xmmword ptr [rsp + 0x60], xmm1
00208a41: movups xmm1, xmmword ptr [rbx + 0x170]
00208a48: movups xmmword ptr [rsp + 0x70], xmm0
00208a4d: movups xmm0, xmmword ptr [rbx + 0x180]
00208a54: movups xmmword ptr [rsp + 0x80], xmm1
00208a5c: movups xmm1, xmmword ptr [rbx + 0x190]
00208a63: movups xmmword ptr [rsp + 0x90], xmm0
00208a6b: movups xmm0, xmmword ptr [rbx + 0x1a0]
00208a72: movups xmmword ptr [rsp + 0xa0], xmm1
00208a7a: movups xmm1, xmmword ptr [rbx + 0x1b0]
00208a81: movups xmmword ptr [rsp + 0xb0], xmm0
00208a89: movups xmm0, xmmword ptr [rbx + 0x1c0]
00208a90: movups xmmword ptr [rsp + 0xc0], xmm1
00208a98: movups xmm1, xmmword ptr [rbx + 0x1d0]
00208a9f: movups xmmword ptr [rsp + 0xd0], xmm0
00208aa7: movups xmmword ptr [rsp + 0xe0], xmm1
00208aaf: call   qword ptr [rax + 0x3c8]
00208ab5: mov    rdx, rax
00208ab8: mov    qword ptr [rbx + 0x120], rax
00208abf: lea    rcx, [rbx + 0x120]
00208ac6: call   0x182f5fc00
00208acb: movaps xmm9, xmmword ptr [rsp + 0xf0]
00208ad4: movaps xmm8, xmmword ptr [rsp + 0x100]
00208add: movaps xmm7, xmmword ptr [rsp + 0x110]
00208ae5: movaps xmm6, xmmword ptr [rsp + 0x120]
00208aed: test   sil, sil
00208af0: je     0x180208b15
00208af2: mov    rcx, qword ptr [rbx + 0x20]
00208af6: test   rcx, rcx
00208af9: je     0x180208b41
00208afb: mov    rdx, qword ptr [rbx + 0x120]
00208b02: mov    qword ptr [rcx + 0x188], rdx
00208b09: add    rcx, 0x188
00208b10: call   0x182f5fc00
00208b15: mov    rax, qword ptr [rbx]
00208b18: mov    rcx, rbx
00208b1b: mov    rdx, qword ptr [rax + 0x410]
00208b22: mov    rdi, qword ptr [rsp + 0x148]
00208b2a: mov    rsi, qword ptr [rsp + 0x140]
00208b32: add    rsp, 0x130
00208b39: pop    rbx
00208b3a: jmp    qword ptr [rax + 0x408]
00208b41: call   0x182f60c50
00208b46: int3   
00208b47: int3   
