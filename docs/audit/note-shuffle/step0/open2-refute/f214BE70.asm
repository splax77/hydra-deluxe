0214be70: mov    qword ptr [rsp + 8], rbx
0214be75: push   rdi
0214be76: sub    rsp, 0x20
0214be7a: mov    rdi, rdx
0214be7d: mov    dword ptr [rcx + 0x50], 0xffffffff
0214be84: xor    edx, edx
0214be86: mov    rbx, rcx
0214be89: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0214be8e: test   rdi, rdi
0214be91: je     0x18214bf02
0214be93: mov    rax, qword ptr [rdi + 0x28]
0214be97: lea    rcx, [rbx + 0x18]
0214be9b: mov    qword ptr [rbx + 0x28], rax
0214be9f: mov    rax, qword ptr [rdi + 0x30]
0214bea3: mov    qword ptr [rbx + 0x30], rax
0214bea7: mov    eax, dword ptr [rdi + 0x38]
0214beaa: mov    dword ptr [rbx + 0x38], eax
0214bead: movzx  eax, word ptr [rdi + 0x80]
0214beb4: mov    word ptr [rbx + 0x80], ax
0214bebb: mov    eax, dword ptr [rdi + 0x7c]
0214bebe: mov    dword ptr [rbx + 0x7c], eax
0214bec1: mov    rax, qword ptr [rdi + 0x40]
0214bec5: mov    qword ptr [rbx + 0x40], rax
0214bec9: mov    rax, qword ptr [rdi + 0x48]
0214becd: mov    qword ptr [rbx + 0x48], rax
0214bed1: mov    rdx, qword ptr [rdi + 0x18]
0214bed5: mov    qword ptr [rbx + 0x18], rdx
0214bed9: call   0x182f5fc00
0214bede: mov    rdx, qword ptr [rdi + 0x10]
0214bee2: lea    rcx, [rbx + 0x10]
0214bee6: mov    qword ptr [rbx + 0x10], rdx
0214beea: call   0x182f5fc00
0214beef: mov    rax, qword ptr [rdi + 0x60]
0214bef3: mov    qword ptr [rbx + 0x60], rax
0214bef7: mov    rbx, qword ptr [rsp + 0x30]
0214befc: add    rsp, 0x20
0214bf00: pop    rdi
0214bf01: ret    
0214bf02: call   0x182f60c50
0214bf07: int3   
0214bf08: int3   
