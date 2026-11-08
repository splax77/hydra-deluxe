02158f60: push   rbx
02158f62: push   rbp
02158f63: push   rsi
02158f64: push   rdi
02158f65: sub    rsp, 0x28
02158f69: mov    rbx, rdx
02158f6c: mov    rbp, r9
02158f6f: xor    edx, edx
02158f71: movzx  edi, r8b
02158f75: mov    rsi, rcx
02158f78: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
02158f7d: lea    rcx, [rsi + 0x20]
02158f81: mov    qword ptr [rsi + 0x20], rbx
02158f85: mov    rdx, rbx
02158f88: call   0x182f5fc00
02158f8d: lea    rcx, [rsi + 0x30]
02158f91: mov    byte ptr [rsi + 0x28], dil
02158f95: mov    rdx, rbp
02158f98: mov    qword ptr [rsi + 0x30], rbp
02158f9c: add    rsp, 0x28
02158fa0: pop    rdi
02158fa1: pop    rsi
02158fa2: pop    rbp
02158fa3: pop    rbx
02158fa4: jmp    0x182f5fc00
02158fa9: int3   
