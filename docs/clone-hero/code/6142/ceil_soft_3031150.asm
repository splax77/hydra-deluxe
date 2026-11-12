03031150: xor    r9d, r9d
03031153: xorps  xmm2, xmm2
03031156: movsd  xmm2, xmm0
0303115a: movabs rcx, 0x7fffffffffffffff
03031164: movq   rdx, xmm2
03031169: mov    rax, rdx
0303116c: and    rax, rcx
0303116f: cmp    rdx, rax
03031172: mov    r8, rax
03031175: setne  r9b
03031179: shr    r8, 0x34
0303117d: cmp    r8, 0x433
03031184: jb     0x18303119d
03031186: movabs rcx, 0x7ff0000000000000
03031190: cmp    rax, rcx
03031193: jbe    0x1830311f9
03031195: mov    rcx, rdx
03031198: jmp    0x18304b9c0
0303119d: cmp    r8, 0x3ff
030311a4: jae    0x1830311c9
030311a6: test   rax, rax
030311a9: je     0x1830311f9
030311ab: test   r9, r9
030311ae: je     0x1830311c0
030311b0: movabs rax, 0x8000000000000000
030311ba: movq   xmm0, rax
030311bf: ret    
030311c0: movsd  xmm0, qword ptr [rip + 0x32bf8]          ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
030311c8: ret    
030311c9: mov    ecx, 0x33
030311ce: mov    eax, 1
030311d3: sub    cl, r8b
030311d6: shl    rax, cl
030311d9: dec    rax
030311dc: not    rax
030311df: and    rax, rdx
030311e2: movq   xmm0, rax
030311e7: test   r9, r9
030311ea: jne    0x1830311f9
030311ec: cmp    rax, rdx
030311ef: je     0x1830311f9
030311f1: addsd  xmm0, qword ptr [rip + 0x32bc7]          ; [0x3063dc0] dbl=1.0 q=0x3ff0000000000000
030311f9: ret    
030311fa: int3   
