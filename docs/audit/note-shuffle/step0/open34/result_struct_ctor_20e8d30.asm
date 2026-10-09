020e8d30: mov    qword ptr [rsp + 8], rbx
020e8d35: mov    qword ptr [rsp + 0x10], rsi
020e8d3a: push   rdi
020e8d3b: sub    rsp, 0x20
020e8d3f: mov    rdi, r9
020e8d42: mov    qword ptr [rcx], rdx
020e8d45: mov    rbx, r8
020e8d48: mov    rsi, rcx
020e8d4b: call   0x182f5fc00
020e8d50: lea    rcx, [rsi + 8]
020e8d54: mov    qword ptr [rsi + 8], rbx
020e8d58: mov    rdx, rbx
020e8d5b: call   0x182f5fc00
020e8d60: lea    rcx, [rsi + 0x10]
020e8d64: mov    qword ptr [rsi + 0x10], rdi
020e8d68: mov    rdx, rdi
020e8d6b: call   0x182f5fc00
020e8d70: mov    rdx, qword ptr [rsp + 0x58]
020e8d75: lea    rcx, [rsi + 0x18]
020e8d79: mov    qword ptr [rsi + 0x18], rdx
020e8d7d: call   0x182f5fc00
020e8d82: mov    rdx, qword ptr [rsp + 0x50]
020e8d87: lea    rcx, [rsi + 0x20]
020e8d8b: mov    qword ptr [rsi + 0x20], rdx
020e8d8f: call   0x182f5fc00
020e8d94: movzx  eax, byte ptr [rsp + 0x68]
020e8d99: lea    rcx, [rsi + 0x28]
020e8d9d: mov    rdx, qword ptr [rsp + 0x60]
020e8da2: mov    byte ptr [rsi + 0x38], al
020e8da5: movzx  eax, byte ptr [rsp + 0x70]
020e8daa: mov    byte ptr [rsi + 0x39], al
020e8dad: mov    qword ptr [rsi + 0x28], rdx
020e8db1: call   0x182f5fc00
020e8db6: mov    rdx, qword ptr [rsp + 0x80]
020e8dbe: lea    rcx, [rsi + 0x30]
020e8dc2: mov    qword ptr [rsi + 0x30], rdx
020e8dc6: call   0x182f5fc00
020e8dcb: mov    rdx, qword ptr [rsp + 0x88]
020e8dd3: lea    rcx, [rsi + 0x58]
020e8dd7: mov    qword ptr [rsi + 0x58], rdx
020e8ddb: call   0x182f5fc00
020e8de0: mov    rbx, qword ptr [rsp + 0xd0]
020e8de8: lea    rcx, [rsi + 0x40]
020e8dec: mov    rdx, qword ptr [rbx]
020e8def: mov    qword ptr [rsi + 0x40], rdx
020e8df3: call   0x182f5fc00
020e8df8: mov    rdx, qword ptr [rbx]
020e8dfb: test   rdx, rdx
020e8dfe: je     0x1820e8eeb
020e8e04: mov    rdx, qword ptr [rdx + 0x30]
020e8e08: lea    rcx, [rsi + 0x50]
020e8e0c: mov    qword ptr [rsi + 0x50], rdx
020e8e10: call   0x182f5fc00
020e8e15: mov    rdx, qword ptr [rsp + 0x78]
020e8e1a: lea    rcx, [rsi + 0x48]
020e8e1e: mov    qword ptr [rsi + 0x48], rdx
020e8e22: call   0x182f5fc00
020e8e27: mov    rdx, qword ptr [rsp + 0x90]
020e8e2f: lea    rcx, [rsi + 0x60]
020e8e33: mov    qword ptr [rsi + 0x60], rdx
020e8e37: call   0x182f5fc00
020e8e3c: mov    rdx, qword ptr [rsp + 0x98]
020e8e44: lea    rcx, [rsi + 0x68]
020e8e48: mov    qword ptr [rsi + 0x68], rdx
020e8e4c: call   0x182f5fc00
020e8e51: mov    rdx, qword ptr [rsp + 0xa8]
020e8e59: lea    rcx, [rsi + 0x70]
020e8e5d: mov    qword ptr [rsi + 0x70], rdx
020e8e61: call   0x182f5fc00
020e8e66: mov    rdx, qword ptr [rsp + 0xb0]
020e8e6e: lea    rcx, [rsi + 0x78]
020e8e72: mov    qword ptr [rsi + 0x78], rdx
020e8e76: call   0x182f5fc00
020e8e7b: mov    rdx, qword ptr [rsp + 0xb8]
020e8e83: lea    rcx, [rsi + 0x80]
020e8e8a: mov    qword ptr [rsi + 0x80], rdx
020e8e91: call   0x182f5fc00
020e8e96: mov    rdx, qword ptr [rsp + 0xa0]
020e8e9e: lea    rcx, [rsi + 0x88]
020e8ea5: mov    qword ptr [rsi + 0x88], rdx
020e8eac: call   0x182f5fc00
020e8eb1: mov    rdx, qword ptr [rsp + 0xc0]
020e8eb9: lea    rcx, [rsi + 0x90]
020e8ec0: mov    qword ptr [rsi + 0x90], rdx
020e8ec7: call   0x182f5fc00
020e8ecc: mov    rax, qword ptr [rsp + 0xc8]
020e8ed4: mov    rbx, qword ptr [rsp + 0x30]
020e8ed9: mov    qword ptr [rsi + 0x98], rax
020e8ee0: mov    rsi, qword ptr [rsp + 0x38]
020e8ee5: add    rsp, 0x20
020e8ee9: pop    rdi
020e8eea: ret    
020e8eeb: call   0x182f60c50
020e8ef0: int3   
020e8ef1: int3   
