0014d0d0: mov    qword ptr [rsp + 8], rbx
0014d0d5: push   rdi
0014d0d6: sub    rsp, 0x70
0014d0da: cmp    byte ptr [rip + 0x3dbf5e0], 0            ; [0x3f0c6c1] (bss)
0014d0e1: movzx  edi, r8b
0014d0e5: movaps xmmword ptr [rsp + 0x60], xmm6
0014d0ea: mov    rbx, rcx
0014d0ed: movaps xmm6, xmm1
0014d0f0: jne    0x18014d111
0014d0f2: lea    rcx, [rip + 0x3b49337]                   ; [0x3c96430] str:'_uElectricityEffect'
0014d0f9: call   0x182f609b0
0014d0fe: lea    rcx, [rip + 0x3b493eb]                   ; [0x3c964f0] str:'_uFillHeight'
0014d105: call   0x182f609b0
0014d10a: mov    byte ptr [rip + 0x3dbf5b0], 1            ; [0x3f0c6c1] (bss)
0014d111: ucomiss xmm6, dword ptr [rbx + 0x60]
0014d115: jp     0x18014d11d
0014d117: je     0x18014d311
0014d11d: movaps xmmword ptr [rsp + 0x50], xmm7
0014d122: movaps xmmword ptr [rsp + 0x40], xmm8
0014d128: movaps xmmword ptr [rsp + 0x30], xmm9
0014d12e: mov    rcx, qword ptr [rbx + 0x20]
0014d132: movss  dword ptr [rbx + 0x60], xmm6
0014d137: test   rcx, rcx
0014d13a: je     0x18014d34c
0014d140: xor    edx, edx
0014d142: call   0x182870d10                              ; UnityEngine.Renderer$$get_material
0014d147: test   rax, rax
0014d14a: je     0x18014d34c
0014d150: mov    rdx, qword ptr [rip + 0x3b49399]         ; [0x3c964f0] str:'_uFillHeight'
0014d157: xor    r9d, r9d
0014d15a: movaps xmm2, xmm6
0014d15d: mov    rcx, rax
0014d160: call   0x1828668c0                              ; UnityEngine.Material$$SetFloat
0014d165: mov    rcx, qword ptr [rbx + 0x20]
0014d169: test   rcx, rcx
0014d16c: je     0x18014d34c
0014d172: xor    edx, edx
0014d174: call   0x182870d10                              ; UnityEngine.Renderer$$get_material
0014d179: movss  xmm9, dword ptr [rip + 0x2f17232]        ; [0x30643b4] flt=0.5 i=1056964608
0014d182: xorps  xmm8, xmm8
0014d186: movss  xmm7, dword ptr [rip + 0x2f16c2e]        ; [0x3063dbc] flt=1.0 i=1065353216
0014d18e: mov    rdx, qword ptr [rip + 0x3b4929b]         ; [0x3c96430] str:'_uElectricityEffect'
0014d195: test   dil, dil
0014d198: je     0x18014d19f
0014d19a: movaps xmm2, xmm7
0014d19d: jmp    0x18014d1ad
0014d19f: comiss xmm6, xmm9
0014d1a3: jb     0x18014d1aa
0014d1a5: movaps xmm2, xmm7
0014d1a8: jmp    0x18014d1ad
0014d1aa: xorps  xmm2, xmm2
0014d1ad: test   rax, rax
0014d1b0: je     0x18014d34c
0014d1b6: xor    r9d, r9d
0014d1b9: mov    rcx, rax
0014d1bc: call   0x1828668c0                              ; UnityEngine.Material$$SetFloat
0014d1c1: comiss xmm8, xmm6
0014d1c5: jae    0x18014d324
0014d1cb: test   dil, dil
0014d1ce: je     0x18014d1d7
0014d1d0: mov    edx, 1
0014d1d5: jmp    0x18014d1de
0014d1d7: comiss xmm6, xmm9
0014d1db: setae  dl
0014d1de: mov    rcx, qword ptr [rbx + 0x50]
0014d1e2: test   rcx, rcx
0014d1e5: je     0x18014d34c
0014d1eb: xor    r8d, r8d
0014d1ee: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0014d1f3: mov    rcx, qword ptr [rbx + 0x58]
0014d1f7: test   rcx, rcx
0014d1fa: je     0x18014d34c
0014d200: comiss xmm6, xmm8
0014d204: seta   dl
0014d207: xor    r8d, r8d
0014d20a: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0014d20f: mov    rcx, qword ptr [rbx + 0x50]
0014d213: test   rcx, rcx
0014d216: je     0x18014d34c
0014d21c: xor    edx, edx
0014d21e: call   0x18287b3e0                              ; UnityEngine.GameObject$$get_transform
0014d223: comiss xmm9, xmm6
0014d227: movss  xmm0, dword ptr [rbx + 0x34]
0014d22c: movss  xmm1, dword ptr [rbx + 0x38]
0014d231: movss  xmm2, dword ptr [rbx + 0x3c]
0014d236: jb     0x18014d284
0014d238: movss  xmm3, dword ptr [rbx + 0x40]
0014d23d: addss  xmm6, xmm6
0014d241: movss  xmm4, dword ptr [rbx + 0x44]
0014d246: movss  xmm5, dword ptr [rbx + 0x48]
0014d24b: comiss xmm8, xmm6
0014d24f: ja     0x18014d25b
0014d251: comiss xmm6, xmm7
0014d254: jbe    0x18014d25e
0014d256: movaps xmm6, xmm7
0014d259: jmp    0x18014d25e
0014d25b: xorps  xmm6, xmm6
0014d25e: subss  xmm0, xmm3
0014d262: subss  xmm1, xmm4
0014d266: subss  xmm2, xmm5
0014d26a: mulss  xmm0, xmm6
0014d26e: mulss  xmm1, xmm6
0014d272: mulss  xmm2, xmm6
0014d276: addss  xmm0, xmm3
0014d27a: addss  xmm1, xmm4
0014d27e: addss  xmm2, xmm5
0014d282: jmp    0x18014d2dc
0014d284: movss  xmm3, dword ptr [rbx + 0x28]
0014d289: subss  xmm6, xmm9
0014d28e: movss  xmm4, dword ptr [rbx + 0x2c]
0014d293: movss  xmm5, dword ptr [rbx + 0x30]
0014d298: addss  xmm6, xmm6
0014d29c: comiss xmm8, xmm6
0014d2a0: ja     0x18014d2ac
0014d2a2: comiss xmm6, xmm7
0014d2a5: jbe    0x18014d2af
0014d2a7: movaps xmm6, xmm7
0014d2aa: jmp    0x18014d2af
0014d2ac: xorps  xmm6, xmm6
0014d2af: subss  xmm3, xmm0
0014d2b3: subss  xmm4, xmm1
0014d2b7: subss  xmm5, xmm2
0014d2bb: mulss  xmm3, xmm6
0014d2bf: mulss  xmm4, xmm6
0014d2c3: mulss  xmm5, xmm6
0014d2c7: addss  xmm3, xmm0
0014d2cb: addss  xmm4, xmm1
0014d2cf: addss  xmm5, xmm2
0014d2d3: movaps xmm0, xmm3
0014d2d6: movaps xmm1, xmm4
0014d2d9: movaps xmm2, xmm5
0014d2dc: test   rax, rax
0014d2df: je     0x18014d34c
0014d2e1: unpcklps xmm0, xmm1
0014d2e4: lea    rdx, [rsp + 0x20]
0014d2e9: xor    r8d, r8d
0014d2ec: movsd  qword ptr [rsp + 0x20], xmm0
0014d2f2: mov    rcx, rax
0014d2f5: movss  dword ptr [rsp + 0x28], xmm2
0014d2fb: call   0x18288a510                              ; UnityEngine.Transform$$set_localPosition
0014d300: movaps xmm8, xmmword ptr [rsp + 0x40]
0014d306: movaps xmm7, xmmword ptr [rsp + 0x50]
0014d30b: movaps xmm9, xmmword ptr [rsp + 0x30]
0014d311: mov    rbx, qword ptr [rsp + 0x80]
0014d319: movaps xmm6, xmmword ptr [rsp + 0x60]
0014d31e: add    rsp, 0x70
0014d322: pop    rdi
0014d323: ret    
0014d324: mov    rcx, qword ptr [rbx + 0x58]
0014d328: test   rcx, rcx
0014d32b: je     0x18014d34c
0014d32d: xor    r8d, r8d
0014d330: xor    edx, edx
0014d332: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0014d337: mov    rcx, qword ptr [rbx + 0x50]
0014d33b: test   rcx, rcx
0014d33e: je     0x18014d34c
0014d340: xor    r8d, r8d
0014d343: xor    edx, edx
0014d345: call   0x18287aee0                              ; UnityEngine.GameObject$$SetActive
0014d34a: jmp    0x18014d300
0014d34c: call   0x182f60c50
0014d351: int3   
0014d352: int3   
