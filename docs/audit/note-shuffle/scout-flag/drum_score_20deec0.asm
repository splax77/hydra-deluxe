020deec0: mov    qword ptr [rsp + 0x10], rbp
020deec5: mov    qword ptr [rsp + 0x18], rsi
020deeca: push   rdi
020deecb: sub    rsp, 0x20
020deecf: mov    esi, r8d
020deed2: mov    rdi, rdx
020deed5: mov    rbp, rcx
020deed8: test   rdx, rdx
020deedb: je     0x1820def2a
020deedd: mov    qword ptr [rsp + 0x30], rbx
020deee2: xor    edx, edx
020deee4: mov    rcx, rdi
020deee7: call   0x18214be30                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˁʸʷʽʸʸʼʼʲʾʼ
020deeec: test   al, al
020deeee: lea    ebx, [rsi + 0xf]
020deef1: cmove  ebx, esi
020deef4: xor    edx, edx
020deef6: mov    rcx, rdi
020deef9: call   0x18214bb40                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʾʽʾʾʵʸʳʶʽˀʻ
020deefe: test   al, al
020def00: lea    r8d, [rbx + rbx]
020def04: mov    rdx, rdi
020def07: mov    rcx, rbp
020def0a: cmove  r8d, ebx
020def0e: xor    r9d, r9d
020def11: mov    rbx, qword ptr [rsp + 0x30]
020def16: mov    rbp, qword ptr [rsp + 0x38]
020def1b: mov    rsi, qword ptr [rsp + 0x40]
020def20: add    rsp, 0x20
020def24: pop    rdi
020def25: jmp    0x1820f6eb0                              ; ʿʶʴˀʴʾʵʵʶʳʲ$$ʿʾʹʲˀʶʷʵʶʶˀ
020def2a: call   0x182f60c50
020def2f: int3   
020def30: sub    rsp, 0x28
020def34: test   rdx, rdx
020def37: je     0x1820defdb
020def3d: cmp    word ptr [rdx + 0x80], 0
020def45: je     0x1820defcf
020def4b: cmp    word ptr [rdx + 0x80], 4
020def53: je     0x1820defc3
020def55: cmp    word ptr [rdx + 0x80], 7
020def5d: je     0x1820defb7
020def5f: cmp    word ptr [rdx + 0x80], 1
020def67: je     0x1820defab
020def69: cmp    word ptr [rdx + 0x80], 0x49
020def71: je     0x1820def9f
020def73: cmp    word ptr [rdx + 0x80], 0x5a
020def7b: je     0x1820def93
020def7d: cmp    word ptr [rdx + 0x80], 0x6e
020def85: jne    0x1820defcf
020def87: movzx  eax, byte ptr [rcx + 0x310]
020def8e: add    rsp, 0x28
020def92: ret    
020def93: movzx  eax, byte ptr [rcx + 0x30f]
020def9a: add    rsp, 0x28
020def9e: ret    
020def9f: movzx  eax, byte ptr [rcx + 0x30e]
020defa6: add    rsp, 0x28
020defaa: ret    
020defab: movzx  eax, byte ptr [rcx + 0x30d]
020defb2: add    rsp, 0x28
020defb6: ret    
020defb7: movzx  eax, byte ptr [rcx + 0x30b]
020defbe: add    rsp, 0x28
020defc2: ret    
020defc3: movzx  eax, byte ptr [rcx + 0x30a]
020defca: add    rsp, 0x28
020defce: ret    
020defcf: movzx  eax, byte ptr [rcx + 0x309]
020defd6: add    rsp, 0x28
020defda: ret    
020defdb: call   0x182f60c50
