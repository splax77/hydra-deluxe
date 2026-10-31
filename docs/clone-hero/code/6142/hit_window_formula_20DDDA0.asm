020ddda0: sub    rsp, 0x48
020ddda4: cmp    byte ptr [rip + 0x1e3b50a], 0            ; [0x3f192b5] (bss)
020dddab: movaps xmmword ptr [rsp + 0x20], xmm7
020dddb0: movaps xmm7, xmm1
020dddb3: mov    qword ptr [rsp + 0x40], rbx
020dddb8: mov    rbx, rcx
020dddbb: jne    0x1820dddd0
020dddbd: lea    rcx, [rip + 0x1bc49a4]                   ; [0x3ca2768] meta:System.Math_TypeInfo
020dddc4: call   0x182f609b0
020dddc9: mov    byte ptr [rip + 0x1e3b4e5], 1            ; [0x3f192b5] (bss)
020dddd0: test   dword ptr [rbx + 0x198], 0x1000
020dddda: mulsd  xmm7, qword ptr [rip + 0xf86e46]         ; [0x3064c28] dbl=1000.0 q=0x408f400000000000
020ddde2: movaps xmmword ptr [rsp + 0x30], xmm6
020ddde7: mov    rbx, qword ptr [rsp + 0x40]
020dddec: mov    rcx, qword ptr [rip + 0x1bc4975]         ; [0x3ca2768] meta:System.Math_TypeInfo
020dddf3: ja     0x1820dde57
020dddf5: cmp    dword ptr [rcx + 0xe0], 0
020dddfc: jne    0x1820dde03
020dddfe: call   0x182f60cf0
020dde03: movsd  xmm1, qword ptr [rip + 0xf87a9d]         ; [0x30658a8] dbl=2.0 q=0x4000000000000000
020dde0b: movaps xmm6, xmm7
020dde0e: mulsd  xmm6, qword ptr [rip + 0x106289a]        ; [0x31406b0] dbl=0.0110924369747899 q=0x3f86b7a1b0513b41
020dde16: xor    r8d, r8d
020dde19: movaps xmm0, xmm7
020dde1c: call   0x181b52030                              ; System.Math$$Pow
020dde21: mulsd  xmm0, qword ptr [rip + 0x10627f7]        ; [0x3140620] dbl=2.628539e-05 q=0x3efb8fee3f2149c5
020dde29: subsd  xmm6, xmm0
020dde2d: mulsd  xmm6, qword ptr [rip + 0x10628d3]        ; [0x3140708] dbl=170.0 q=0x4065400000000000
020dde35: subsd  xmm6, qword ptr [rip + 0xf86183]         ; [0x3063fc0] dbl=20.0 q=0x4034000000000000
020dde3d: divsd  xmm6, qword ptr [rip + 0xf86de3]         ; [0x3064c28] dbl=1000.0 q=0x408f400000000000
020dde45: movaps xmm0, xmm6
020dde48: movaps xmm6, xmmword ptr [rsp + 0x30]
020dde4d: movaps xmm7, xmmword ptr [rsp + 0x20]
020dde52: add    rsp, 0x48
020dde56: ret    
020dde57: cmp    dword ptr [rcx + 0xe0], 0
020dde5e: jne    0x1820dde65
020dde60: call   0x182f60cf0
020dde65: movsd  xmm1, qword ptr [rip + 0xf87a3b]         ; [0x30658a8] dbl=2.0 q=0x4000000000000000
020dde6d: movaps xmm6, xmm7
020dde70: mulsd  xmm6, qword ptr [rip + 0x10628c0]        ; [0x3140738] dbl=-0.04 q=0xbfa47ae147ae147b
020dde78: xor    r8d, r8d
020dde7b: movaps xmm0, xmm7
020dde7e: call   0x181b52030                              ; System.Math$$Pow
020dde83: mulsd  xmm0, qword ptr [rip + 0x10627cd]        ; [0x3140658] dbl=0.0004 q=0x3f3a36e2eb1c432d
020dde8b: movaps xmm7, xmmword ptr [rsp + 0x20]
020dde90: subsd  xmm6, xmm0
020dde94: movsd  xmm0, qword ptr [rip + 0x1062864]        ; [0x3140700] dbl=13.0 q=0x402a000000000000
020dde9c: mulsd  xmm6, qword ptr [rip + 0x1062854]        ; [0x31406f8] dbl=12.4805915 q=0x4028f61016ce789e
020ddea4: subsd  xmm0, xmm6
020ddea8: movaps xmm6, xmmword ptr [rsp + 0x30]
020ddead: divsd  xmm0, qword ptr [rip + 0xf86d73]         ; [0x3064c28] dbl=1000.0 q=0x408f400000000000
020ddeb5: add    rsp, 0x48
020ddeb9: ret    
020ddeba: int3   
