0214cee0: mov    qword ptr [rsp + 8], rbx
0214cee5: mov    qword ptr [rsp + 0x10], rsi
0214ceea: push   rdi
0214ceeb: sub    rsp, 0x30
0214ceef: cmp    byte ptr [rip + 0x1dcc71b], 0            ; [0x3f19611] (bss)
0214cef6: mov    rbx, rcx
0214cef9: jne    0x18214cf1a
0214cefb: lea    rcx, [rip + 0x1b5e3c6]                   ; [0x3cab2c8] metamethod:Method$System.Span<ʸʻˁʴʿʶʶʳʸʶʳ>.get_Length()
0214cf02: call   0x182f609b0
0214cf07: lea    rcx, [rip + 0x1ba4962]                   ; [0x3cf1870] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>()
0214cf0e: call   0x182f609b0
0214cf13: mov    byte ptr [rip + 0x1dcc6f7], 1            ; [0x3f19611] (bss)
0214cf1a: mov    rdi, qword ptr [rip + 0x1ba494f]         ; [0x3cf1870] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>()
0214cf21: mov    rsi, qword ptr [rbx + 0x38]
0214cf25: cmp    qword ptr [rdi + 0x38], 0
0214cf2a: jne    0x18214cf34
0214cf2c: mov    rcx, rdi
0214cf2f: call   0x182f657d0
0214cf34: mov    r8, qword ptr [rdi + 0x38]
0214cf38: lea    rcx, [rsp + 0x20]
0214cf3d: mov    rdx, rsi
0214cf40: mov    r8, qword ptr [r8 + 8]
0214cf44: call   0x180462bf0                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<ʸʻˁʴʿʶʶʳʸʶʳ>
0214cf49: mov    r9d, dword ptr [rsp + 0x28]
0214cf4e: test   r9d, r9d
0214cf51: jle    0x18214cffb
0214cf57: mov    r10, qword ptr [rsp + 0x20]
0214cf5c: xor    r8d, r8d
0214cf5f: nop    
0214cf60: cmp    r8d, r9d
0214cf63: jae    0x18214d00b
0214cf69: mov    edx, r8d
0214cf6c: shl    rdx, 5
0214cf70: add    rdx, r10
0214cf73: movzx  ecx, byte ptr [rdx + 0x18]
0214cf77: sub    ecx, 6
0214cf7a: je     0x18214cfdb
0214cf7c: sub    ecx, 1
0214cf7f: je     0x18214cfc8
0214cf81: sub    ecx, 1
0214cf84: je     0x18214cfbf
0214cf86: sub    ecx, 1
0214cf89: je     0x18214cfaa
0214cf8b: cmp    ecx, 1
0214cf8e: jne    0x18214cfef
0214cf90: mov    eax, dword ptr [rbx + 0x20]
0214cf93: cmp    dword ptr [rdx + 0x14], eax
0214cf96: jne    0x18214cfef
0214cf98: test   dword ptr [rbx + 0x24], 0x400
0214cf9f: jne    0x18214cfef
0214cfa1: or     dword ptr [rbx + 0x24], 0x200
0214cfa8: jmp    0x18214cfef
0214cfaa: mov    eax, dword ptr [rbx + 0x20]
0214cfad: cmp    dword ptr [rdx + 0x14], eax
0214cfb0: jne    0x18214cfef
0214cfb2: mov    eax, dword ptr [rbx + 0x24]
0214cfb5: btr    eax, 9
0214cfb9: bts    eax, 0xa
0214cfbd: jmp    0x18214cfec
0214cfbf: or     dword ptr [rbx + 0x24], 0x100
0214cfc6: jmp    0x18214cfef
0214cfc8: mov    eax, dword ptr [rbx + 0x20]
0214cfcb: cmp    dword ptr [rdx + 0x14], eax
0214cfce: jne    0x18214cfef
0214cfd0: mov    eax, dword ptr [rbx + 0x24]
0214cfd3: and    eax, 0xffffffef
0214cfd6: or     eax, 0x20
0214cfd9: jmp    0x18214cfec
0214cfdb: mov    eax, dword ptr [rbx + 0x20]
0214cfde: cmp    dword ptr [rdx + 0x14], eax
0214cfe1: jne    0x18214cfef
0214cfe3: mov    eax, dword ptr [rbx + 0x24]
0214cfe6: and    eax, 0xffffffdf
0214cfe9: or     eax, 0x10
0214cfec: mov    dword ptr [rbx + 0x24], eax
0214cfef: inc    r8d
0214cff2: cmp    r8d, r9d
0214cff5: jl     0x18214cf63
0214cffb: mov    rbx, qword ptr [rsp + 0x40]
0214d000: mov    rsi, qword ptr [rsp + 0x48]
0214d005: add    rsp, 0x30
0214d009: pop    rdi
0214d00a: ret    
0214d00b: call   0x182f60c40
0214d010: int3   
0214d011: int3   
