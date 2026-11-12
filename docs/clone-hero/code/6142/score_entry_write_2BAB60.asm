002bab60: mov    qword ptr [rsp + 8], rbx
002bab65: push   rdi
002bab66: sub    rsp, 0x20
002bab6a: mov    rbx, rdx
002bab6d: mov    rdi, rcx
002bab70: test   rdx, rdx
002bab73: je     0x1802bac39
002bab79: mov    rax, qword ptr [rdx]
002bab7c: movsx  edx, byte ptr [rcx]
002bab7f: mov    rcx, rbx
002bab82: mov    r8, qword ptr [rax + 0x2a0]
002bab89: call   qword ptr [rax + 0x298]
002bab8f: mov    rax, qword ptr [rbx]
002bab92: mov    rcx, rbx
002bab95: movzx  edx, byte ptr [rdi + 1]
002bab99: mov    r8, qword ptr [rax + 0x220]
002baba0: call   qword ptr [rax + 0x218]
002baba6: mov    rax, qword ptr [rbx]
002baba9: mov    rcx, rbx
002babac: movzx  edx, byte ptr [rdi + 0x14]
002babb0: mov    r8, qword ptr [rax + 0x220]
002babb7: call   qword ptr [rax + 0x218]
002babbd: mov    rax, qword ptr [rbx]
002babc0: mov    rcx, rbx
002babc3: movzx  edx, byte ptr [rdi + 8]
002babc7: mov    r8, qword ptr [rax + 0x210]
002babce: call   qword ptr [rax + 0x208]
002babd4: mov    rax, qword ptr [rbx]
002babd7: mov    rcx, rbx
002babda: movzx  edx, word ptr [rdi + 0xa]
002babde: mov    r8, qword ptr [rax + 0x290]
002babe5: call   qword ptr [rax + 0x288]
002babeb: mov    rax, qword ptr [rbx]
002babee: mov    rcx, rbx
002babf1: movzx  edx, byte ptr [rdi + 0xc]
002babf5: mov    r8, qword ptr [rax + 0x220]
002babfc: call   qword ptr [rax + 0x218]
002bac02: mov    rax, qword ptr [rbx]
002bac05: mov    rcx, rbx
002bac08: mov    edx, dword ptr [rdi + 0x10]
002bac0b: mov    r8, qword ptr [rax + 0x2c0]
002bac12: call   qword ptr [rax + 0x2b8]
002bac18: mov    rax, qword ptr [rbx]
002bac1b: mov    rcx, rbx
002bac1e: mov    edx, dword ptr [rdi + 4]
002bac21: mov    r8, qword ptr [rax + 0x2b0]
002bac28: mov    rbx, qword ptr [rsp + 0x30]
002bac2d: add    rsp, 0x20
002bac31: pop    rdi
002bac32: jmp    qword ptr [rax + 0x2a8]
002bac39: call   0x182f60c50
002bac3e: int3   
002bac3f: int3   
