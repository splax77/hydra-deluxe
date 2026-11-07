020cfd40: mov    qword ptr [rsp + 0x18], rbx
020cfd45: mov    qword ptr [rsp + 0x20], rbp
020cfd4a: push   r14
020cfd4c: sub    rsp, 0x30
020cfd50: cmp    byte ptr [rip + 0x1e495c6], 0            ; [0x3f1931d] (bss)
020cfd57: mov    rbx, rcx
020cfd5a: movsx  ebp, r8b
020cfd5e: movsx  r14d, dl
020cfd62: jne    0x1820cfd8f
020cfd64: lea    rcx, [rip + 0x1c016d5]                   ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
020cfd6b: call   0x182f609b0
020cfd70: lea    rcx, [rip + 0x1c01789]                   ; [0x3cd1500] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.set_Item()
020cfd77: call   0x182f609b0
020cfd7c: lea    rcx, [rip + 0x1c29b15]                   ; [0x3cf9898] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʷʿʽʽʵʻʶʹʼʼʺ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cfd83: call   0x182f609b0
020cfd88: mov    byte ptr [rip + 0x1e4958e], 1            ; [0x3f1931d] (bss)
020cfd8f: cmp    byte ptr [rip + 0x1e3e613], 0            ; [0x3f0e3a9] (bss)
020cfd96: jne    0x1820cfdab
020cfd98: lea    rcx, [rip + 0x1bd12d1]                   ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020cfd9f: call   0x182f609b0
020cfda4: mov    byte ptr [rip + 0x1e3e5fe], 1            ; [0x3f0e3a9] (bss)
020cfdab: mov    qword ptr [rsp + 0x48], rdi
020cfdb0: cmp    bpl, 0xff
020cfdb4: je     0x1820cfea3
020cfdba: mov    rax, qword ptr [rip + 0x1bd12af]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020cfdc1: cmp    dword ptr [rax + 0xe0], 0
020cfdc8: jne    0x1820cfdd9
020cfdca: mov    rcx, rax
020cfdcd: call   0x182f60cf0
020cfdd2: mov    rax, qword ptr [rip + 0x1bd1297]         ; [0x3ca1070] meta:ˁʿʺʲʲʹˀʴʾʻʻ_TypeInfo
020cfdd9: mov    rax, qword ptr [rax + 0xb8]
020cfde0: mov    edi, r14d
020cfde3: imul   edi, dword ptr [rax + 0x60]
020cfde7: add    edi, ebp
020cfde9: cmp    edi, -1
020cfdec: je     0x1820cfea3
020cfdf2: mov    qword ptr [rsp + 0x40], rsi
020cfdf7: mov    rsi, qword ptr [rbx + 0xd0]
020cfdfe: test   rsi, rsi
020cfe01: je     0x1820cfeec
020cfe07: mov    r8, qword ptr [rip + 0x1c01632]          ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
020cfe0e: mov    edx, edi
020cfe10: mov    rcx, rsi
020cfe13: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cfe18: test   rax, rax
020cfe1b: jne    0x1820cfe6f
020cfe1d: mov    rcx, qword ptr [rbx + 0x20]
020cfe21: test   rcx, rcx
020cfe24: je     0x1820cfeec
020cfe2a: mov    rdx, qword ptr [rip + 0x1c29a67]         ; [0x3cf9898] metamethod:Method$ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<ʷʿʽʽʵʻʶʹʼʼʺ>.ʺʹʽʳʵʼʺʾʾʷʿ()
020cfe31: call   0x180fe7380                              ; ˁʽˀʻʿʿʳʸʵʴʵ.ʺʳʵˀʺʻʳʳʲʼʸ<object>$$ʺʹʽʳʵʼʺʾʾʷʿ
020cfe36: test   rax, rax
020cfe39: je     0x1820cfeec
020cfe3f: movzx  r9d, bpl
020cfe43: mov    qword ptr [rsp + 0x20], 0
020cfe4c: movzx  r8d, r14b
020cfe50: mov    rdx, rbx
020cfe53: mov    rcx, rax
020cfe56: call   0x18215e6e0                              ; ʷʿʽʽʵʻʶʹʼʼʺ$$ʺʷʻʲʴʺʷʺʺʶʴ
020cfe5b: mov    r9, qword ptr [rip + 0x1c0169e]          ; [0x3cd1500] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.set_Item()
020cfe62: mov    r8, rax
020cfe65: mov    edx, edi
020cfe67: mov    rcx, rsi
020cfe6a: call   0x18077b2c0                              ; System.Collections.Generic.List<object>$$set_Item
020cfe6f: mov    rcx, qword ptr [rbx + 0xd0]
020cfe76: test   rcx, rcx
020cfe79: je     0x1820cfeec
020cfe7b: mov    r8, qword ptr [rip + 0x1c015be]          ; [0x3cd1440] metamethod:Method$System.Collections.Generic.List<ʷʿʽʽʵʻʶʹʼʼʺ>.get_Item()
020cfe82: mov    edx, edi
020cfe84: mov    rsi, qword ptr [rsp + 0x40]
020cfe89: mov    rdi, qword ptr [rsp + 0x48]
020cfe8e: mov    rbx, qword ptr [rsp + 0x50]
020cfe93: mov    rbp, qword ptr [rsp + 0x58]
020cfe98: add    rsp, 0x30
020cfe9c: pop    r14
020cfe9e: jmp    0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020cfea3: lea    rcx, [rip + 0x1be6636]                   ; [0x3cb64e0] meta:System.ArgumentException_TypeInfo
020cfeaa: call   0x182f609d0
020cfeaf: mov    rcx, rax
020cfeb2: call   0x182f60c00
020cfeb7: lea    rcx, [rip + 0x1bd1c2a]                   ; [0x3ca1ae8] str:'Invalid difficulty'
020cfebe: mov    rbx, rax
020cfec1: call   0x182f609d0
020cfec6: mov    rdx, rax
020cfec9: xor    r8d, r8d
020cfecc: mov    rcx, rbx
020cfecf: call   0x181a8b6d0                              ; System.ArgumentException$$.ctor
020cfed4: lea    rcx, [rip + 0x1bcf87d]                   ; [0x3c9f758] metamethod:Method$ˁʿʺʲʲʹˀʴʾʻʻ.ʼʶˁʴʶʴʶʳʷʷʹ()
020cfedb: call   0x182f609d0
020cfee0: mov    rdx, rax
020cfee3: mov    rcx, rbx
020cfee6: call   0x182f60c10
020cfeeb: int3   
020cfeec: call   0x182f60c50
020cfef1: int3   
020cfef2: int3   
