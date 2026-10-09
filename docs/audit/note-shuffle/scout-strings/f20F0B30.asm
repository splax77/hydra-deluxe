020f0b30: mov    qword ptr [rsp + 8], rbx
020f0b35: push   rdi
020f0b36: sub    rsp, 0x20
020f0b3a: xor    r10d, r10d
020f0b3d: xor    r11d, r11d
020f0b40: mov    r9, rdx
020f0b43: mov    edi, ecx
020f0b45: test   ecx, ecx
020f0b47: jle    0x1820f0c18
020f0b4d: movsd  xmm2, qword ptr [rip + 0x104fbeb]        ; [0x3140740] dbl=4.656612873077393e-10 q=0x3e00000000000000
020f0b55: nop    word ptr [rax + rax]
020f0b60: mov    r8, qword ptr [r9]
020f0b63: test   r8, r8
020f0b66: je     0x1820f0c27
020f0b6c: mov    ebx, dword ptr [r9 + 8]
020f0b70: xorps  xmm1, xmm1
020f0b73: mov    rax, qword ptr [r8 + 0x10]
020f0b77: dec    ebx
020f0b79: mov    rcx, qword ptr [r8 + 0x18]
020f0b7d: shr    rcx, 9
020f0b81: shl    rax, 0x17
020f0b85: xor    rax, qword ptr [r8 + 0x10]
020f0b89: xor    rcx, rax
020f0b8c: shr    rcx, 0x11
020f0b90: xor    rcx, qword ptr [r8 + 0x18]
020f0b94: xor    rcx, rax
020f0b97: mov    rax, qword ptr [r8 + 0x18]
020f0b9b: mov    qword ptr [r8 + 0x10], rax
020f0b9f: mov    qword ptr [r8 + 0x18], rcx
020f0ba3: lea    rdx, [rax + rcx]
020f0ba7: btr    edx, 0x1f
020f0bab: test   rdx, rdx
020f0bae: js     0x1820f0bb7
020f0bb0: cvtsi2sd xmm1, rdx
020f0bb5: jmp    0x1820f0bcc
020f0bb7: mov    rax, rdx
020f0bba: and    edx, 1
020f0bbd: shr    rax, 1
020f0bc0: or     rax, rdx
020f0bc3: cvtsi2sd xmm1, rax
020f0bc8: addsd  xmm1, xmm1
020f0bcc: movd   xmm0, ebx
020f0bd0: lea    eax, [r11 + 1]
020f0bd4: cvtdq2pd xmm0, xmm0
020f0bd8: mov    edx, 1
020f0bdd: mulsd  xmm1, xmm2
020f0be1: mulsd  xmm1, xmm0
020f0be5: cvttsd2si ecx, xmm1
020f0be9: inc    ecx
020f0beb: and    ecx, 0x1f
020f0bee: shl    dx, cl
020f0bf1: test   r10w, dx
020f0bf5: movzx  ecx, dx
020f0bf8: cmovne eax, r11d
020f0bfc: or     dx, r10w
020f0c00: test   r10w, cx
020f0c04: mov    r11d, eax
020f0c07: cmovne dx, r10w
020f0c0c: movzx  r10d, dx
020f0c10: cmp    eax, edi
020f0c12: jl     0x1820f0b60
020f0c18: mov    rbx, qword ptr [rsp + 0x30]
020f0c1d: movzx  eax, r10w
020f0c21: add    rsp, 0x20
020f0c25: pop    rdi
020f0c26: ret    
020f0c27: call   0x182f60c50
020f0c2c: int3   
020f0c2d: int3   
