02152b20: push   rbx
02152b22: push   rbp
02152b23: push   rsi
02152b24: push   rdi
02152b25: sub    rsp, 0x48
02152b29: cmp    byte ptr [rip + 0x1dc6c2a], 0            ; [0x3f1975a] (bss)
02152b30: mov    rdi, r8
02152b33: movsx  ebx, dl
02152b36: mov    rsi, rcx
02152b39: jne    0x182152b72
02152b3b: lea    rcx, [rip + 0x1b58616]                   ; [0x3cab158] metamethod:Method$System.Span<ʷʿʽʽʵʻʶʹʼʼʺ>.Slice()
02152b42: call   0x182f609b0
02152b47: lea    rcx, [rip + 0x1b586c2]                   ; [0x3cab210] metamethod:Method$System.Span<ʷʿʽʽʵʻʶʹʼʼʺ>.op_Implicit()
02152b4e: call   0x182f609b0
02152b53: lea    rcx, [rip + 0x1b4e516]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152b5a: call   0x182f609b0
02152b5f: lea    rcx, [rip + 0x1b9eb8a]                   ; [0x3cf16f0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʷʿʽʽʵʻʶʹʼʼʺ>()
02152b66: call   0x182f609b0
02152b6b: mov    byte ptr [rip + 0x1dc6be8], 1            ; [0x3f1975a] (bss)
02152b72: mov    rax, qword ptr [rip + 0x1b4e4f7]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152b79: cmp    dword ptr [rax + 0xe0], 0
02152b80: jne    0x182152b91
02152b82: mov    rcx, rax
02152b85: call   0x182f60cf0
02152b8a: mov    rax, qword ptr [rip + 0x1b4e4df]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152b91: mov    rax, qword ptr [rax + 0xb8]
02152b98: mov    rbp, qword ptr [rsi + 0xd0]
02152b9f: mov    rsi, qword ptr [rip + 0x1b9eb4a]         ; [0x3cf16f0] metamethod:Method$System.Collections.Generic.__ListXTension.AsSpan<ʷʿʽʽʵʻʶʹʼʼʺ>()
02152ba6: imul   ebx, dword ptr [rax + 0x60]
02152baa: cmp    qword ptr [rsi + 0x38], 0
02152baf: jne    0x182152bb9
02152bb1: mov    rcx, rsi
02152bb4: call   0x182f657d0
02152bb9: mov    r8, qword ptr [rsi + 0x38]
02152bbd: lea    rcx, [rsp + 0x20]
02152bc2: mov    rdx, rbp
02152bc5: mov    r8, qword ptr [r8 + 8]
02152bc9: call   0x180462d10                              ; System.Runtime.InteropServices.CollectionMarshal$$AsSpan<object>
02152bce: mov    rax, qword ptr [rip + 0x1b4e49b]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152bd5: mov    edx, dword ptr [rsp + 0x28]
02152bd9: mov    rbp, qword ptr [rip + 0x1b58578]         ; [0x3cab158] metamethod:Method$System.Span<ʷʿʽʽʵʻʶʹʼʼʺ>.Slice()
02152be0: mov    rcx, qword ptr [rax + 0xb8]
02152be7: mov    esi, dword ptr [rcx + 0x60]
02152bea: cmp    ebx, edx
02152bec: ja     0x182152bf4
02152bee: sub    edx, ebx
02152bf0: cmp    esi, edx
02152bf2: jbe    0x182152bfb
02152bf4: xor    ecx, ecx
02152bf6: call   0x181b75520                              ; System.ThrowHelper$$ThrowArgumentOutOfRangeException
02152bfb: mov    rax, qword ptr [rsp + 0x20]
02152c00: movsxd rcx, ebx
02152c03: mov    dword ptr [rsp + 0x2c], 0
02152c0b: lea    rbx, [rax + rcx*8]
02152c0f: mov    rcx, qword ptr [rbp + 0x20]
02152c13: test   byte ptr [rcx + 0x135], 1
02152c1a: jne    0x182152c21
02152c1c: call   0x182f65750
02152c21: mov    r8, qword ptr [rip + 0x1b585e8]          ; [0x3cab210] metamethod:Method$System.Span<ʷʿʽʽʵʻʶʹʼʼʺ>.op_Implicit()
02152c28: lea    rdx, [rsp + 0x20]
02152c2d: mov    qword ptr [rsp + 0x20], rbx
02152c32: lea    rcx, [rsp + 0x30]
02152c37: mov    dword ptr [rsp + 0x28], esi
02152c3b: movaps xmm0, xmmword ptr [rsp + 0x20]
02152c40: movdqa xmmword ptr [rsp + 0x20], xmm0
02152c46: call   0x1809b8190                              ; System.Span<ʼʽʶʿˀʵʶʻʴʽʼ>$$op_Implicit
02152c4b: movups xmm0, xmmword ptr [rsp + 0x30]
02152c50: xor    ebx, ebx
02152c52: movups xmmword ptr [rdi], xmm0
02152c55: mov    rdx, qword ptr [rip + 0x1b4e414]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152c5c: nop    dword ptr [rax]
02152c60: cmp    dword ptr [rdx + 0xe0], 0
02152c67: jne    0x182152c78
02152c69: mov    rcx, rdx
02152c6c: call   0x182f60cf0
02152c71: mov    rdx, qword ptr [rip + 0x1b4e3f8]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
02152c78: mov    rax, qword ptr [rdx + 0xb8]
02152c7f: cmp    ebx, dword ptr [rax + 0x60]
02152c82: jge    0x182152ca5
02152c84: cmp    ebx, dword ptr [rdi + 8]
02152c87: jae    0x182152cb6
02152c89: mov    rax, qword ptr [rdi]
02152c8c: movsxd rcx, ebx
02152c8f: cmp    qword ptr [rax + rcx*8], 0
02152c94: jne    0x182152c9a
02152c96: inc    ebx
02152c98: jmp    0x182152c60
02152c9a: mov    al, 1
02152c9c: add    rsp, 0x48
02152ca0: pop    rdi
02152ca1: pop    rsi
02152ca2: pop    rbp
02152ca3: pop    rbx
02152ca4: ret    
02152ca5: xorps  xmm0, xmm0
02152ca8: xor    al, al
02152caa: movups xmmword ptr [rdi], xmm0
02152cad: add    rsp, 0x48
02152cb1: pop    rdi
02152cb2: pop    rsi
02152cb3: pop    rbp
02152cb4: pop    rbx
02152cb5: ret    
02152cb6: call   0x182f60c40
02152cbb: int3   
02152cbc: int3   
