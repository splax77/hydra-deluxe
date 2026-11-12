02c49a60: sub    rsp, 0x38
02c49a64: xorps  xmm1, xmm1
02c49a67: movaps xmmword ptr [rsp + 0x20], xmm6
02c49a6c: comisd xmm0, xmm1
02c49a70: lea    rdx, [rsp + 0x40]
02c49a75: movaps xmm6, xmm0
02c49a78: jb     0x182c49ac5
02c49a7a: call   0x183031200
02c49a7f: movsd  xmm1, qword ptr [rip + 0x41a529]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
02c49a87: ucomisd xmm0, xmm1
02c49a8b: jp     0x182c49ab0
02c49a8d: jne    0x182c49ab0
02c49a8f: movsd  xmm0, qword ptr [rsp + 0x40]
02c49a95: cvttsd2si rax, xmm0
02c49a9a: test   al, 1
02c49a9c: je     0x182c49b07
02c49a9e: addsd  xmm0, qword ptr [rip + 0x41a31a]         ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
02c49aa6: movaps xmm6, xmmword ptr [rsp + 0x20]
02c49aab: add    rsp, 0x38
02c49aaf: ret    
02c49ab0: addsd  xmm6, xmm1
02c49ab4: movaps xmm0, xmm6
02c49ab7: movaps xmm6, xmmword ptr [rsp + 0x20]
02c49abc: add    rsp, 0x38
02c49ac0: jmp    0x1830312d0
02c49ac5: call   0x183031200
02c49aca: ucomisd xmm0, qword ptr [rip + 0x41a756]        ; [0x3064228] dbl=-0.5 q=0xbfe0000000000000
02c49ad2: jp     0x182c49af7
02c49ad4: jne    0x182c49af7
02c49ad6: movsd  xmm0, qword ptr [rsp + 0x40]
02c49adc: cvttsd2si rax, xmm0
02c49ae1: test   al, 1
02c49ae3: je     0x182c49b07
02c49ae5: subsd  xmm0, qword ptr [rip + 0x41a2d3]         ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
02c49aed: movaps xmm6, xmmword ptr [rsp + 0x20]
02c49af2: add    rsp, 0x38
02c49af6: ret    
02c49af7: subsd  xmm6, qword ptr [rip + 0x41a4b1]         ; [0x3063fb0] dbl=0.5 q=0x3fe0000000000000
02c49aff: movaps xmm0, xmm6
02c49b02: call   0x183031130
02c49b07: movaps xmm6, xmmword ptr [rsp + 0x20]
02c49b0c: add    rsp, 0x38
02c49b10: ret    
02c49b11: int3   
