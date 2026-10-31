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
