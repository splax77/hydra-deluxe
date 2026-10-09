0212c0b0: unpcklpd xmm1, xmm1
0212c0b4: sub    rsp, 0x28
0212c0b8: xorps  xmm0, xmm0
0212c0bb: comisd xmm0, xmm1
0212c0bf: jae    0x18212c10e
0212c0c1: movabs rdx, 0x7fffffffffffffff
0212c0cb: movq   rax, xmm1
0212c0d0: and    rax, rdx
0212c0d3: movabs rdx, 0x7ff0000000000000
0212c0dd: cmp    rax, rdx
0212c0e0: jae    0x18212c10e
0212c0e2: mov    rax, qword ptr [rcx + 0xc8]
0212c0e9: movups xmmword ptr [rcx + 0xb0], xmm1
0212c0f0: test   rax, rax
0212c0f3: je     0x18212c160
0212c0f5: movss  xmm0, dword ptr [rax + 0x14]
0212c0fa: cvtps2pd xmm0, xmm0
0212c0fd: mulsd  xmm0, xmm1
0212c101: movsd  qword ptr [rcx + 0xb0], xmm0
0212c109: add    rsp, 0x28
0212c10d: ret    
0212c10e: lea    rcx, [rip + 0x1b8a3cb]                   ; [0x3cb64e0] meta:System.ArgumentException_TypeInfo
0212c115: mov    qword ptr [rsp + 0x20], rbx
0212c11a: call   0x182f609d0
0212c11f: mov    rcx, rax
0212c122: call   0x182f60c00
0212c127: lea    rcx, [rip + 0x1bb52c2]                   ; [0x3ce13f0] str:'Song Resolution must be greater than 0'
0212c12e: mov    rbx, rax
0212c131: call   0x182f609d0
0212c136: mov    rdx, rax
0212c139: xor    r8d, r8d
0212c13c: mov    rcx, rbx
0212c13f: call   0x181a8b6d0                              ; System.ArgumentException$$.ctor
0212c144: lea    rcx, [rip + 0x1b7328d]                   ; [0x3c9f3d8] metamethod:Method$ˁʿʺʲʲʹˀʴʾʻʻ.ʴˁʹˀʵʲʾʺʴʼʵ()
0212c14b: call   0x182f609d0
0212c150: mov    rdx, rax
0212c153: mov    rcx, rbx
0212c156: call   0x182f60c10
0212c15b: mov    rbx, qword ptr [rsp + 0x20]
0212c160: call   0x182f60c50
0212c165: int3   
0212c166: int3   
