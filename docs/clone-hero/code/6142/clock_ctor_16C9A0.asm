; function 0x16c9a0 ʴʶʺˀˀʺʹʴʿʹʷ$$.ctor next method starts at 0x16ca70 bytes 0xd0
0016c9a0: push   rbx
0016c9a2: sub    rsp, 0x50
0016c9a6: movaps xmmword ptr [rsp + 0x40], xmm6
0016c9ab: xor    edx, edx
0016c9ad: movaps xmmword ptr [rsp + 0x30], xmm7
0016c9b2: movaps xmm6, xmm1
0016c9b5: movaps xmmword ptr [rsp + 0x20], xmm8
0016c9bb: movaps xmm7, xmm3
0016c9be: movaps xmm8, xmm2
0016c9c2: mov    byte ptr [rcx + 0x28], 1
0016c9c6: mov    rbx, rcx
0016c9c9: mov    dword ptr [rcx + 0x30], 0x3f800000
0016c9d0: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0016c9d5: movss  xmm0, dword ptr [rsp + 0x80]
0016c9de: xorps  xmm1, xmm1
0016c9e1: movss  xmm4, dword ptr [rsp + 0x88]
0016c9ea: cvtps2pd xmm0, xmm0
0016c9ed: cvtss2sd xmm1, xmm7
0016c9f1: movsd  qword ptr [rbx + 0x48], xmm0
0016c9f6: xorps  xmm0, xmm0
0016c9f9: cvtss2sd xmm0, xmm7
0016c9fd: divsd  xmm8, xmm1
0016ca02: divsd  xmm6, xmm0
0016ca06: xorps  xmm8, xmmword ptr [rip + 0x2ef7f22]      ; [0x3064930] xmm dbl=(-0.0, -0.0) flt=(0.0, -0.0, 0.0, -0.0)
0016ca0e: movaps xmm0, xmmword ptr [rip + 0x2ef7f2b]      ; [0x3064940] xmm dbl=(-1.0, -1.0) flt=(0.0, -1.875, 0.0, -1.875)
0016ca15: comisd xmm0, xmm8
0016ca1a: cvtps2pd xmm4, xmm4
0016ca1d: movsd  qword ptr [rbx + 0x10], xmm6
0016ca22: movss  dword ptr [rbx + 0x2c], xmm7
0016ca27: movsd  qword ptr [rbx + 0x50], xmm4
0016ca2c: movsd  qword ptr [rbx + 0x58], xmm8
0016ca32: jbe    0x18016ca49
0016ca34: subsd  xmm8, qword ptr [rip + 0x2ef7383]        ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
0016ca3d: unpcklpd xmm8, xmm8
0016ca42: movups xmmword ptr [rbx + 0x38], xmm8
0016ca47: jmp    0x18016ca4d
0016ca49: movups xmmword ptr [rbx + 0x38], xmm0
0016ca4d: movaps xmm6, xmmword ptr [rsp + 0x40]
0016ca52: movaps xmm7, xmmword ptr [rsp + 0x30]
0016ca57: movaps xmm8, xmmword ptr [rsp + 0x20]
0016ca5d: add    rsp, 0x50
0016ca61: pop    rbx
0016ca62: ret    
0016ca63: int3   
0016ca64: int3   
0016ca65: int3   
0016ca66: int3   
0016ca67: int3   
0016ca68: int3   
0016ca69: int3   
0016ca6a: int3   
0016ca6b: int3   
0016ca6c: int3   
0016ca6d: int3   
0016ca6e: int3   
0016ca6f: int3   
