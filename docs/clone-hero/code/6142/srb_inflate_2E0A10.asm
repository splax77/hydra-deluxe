002e0a10: mov    qword ptr [rsp + 0x10], rbx
002e0a15: mov    qword ptr [rsp + 0x20], rsi
002e0a1a: push   rdi
002e0a1b: sub    rsp, 0x40
002e0a1f: mov    rbx, rcx
002e0a22: cmp    byte ptr [rip + 0x3c2cebf], 0            ; [0x3f0d8e8] (bss)
002e0a29: jne    0x1802e0a56
002e0a2b: lea    rcx, [rip + 0x39fa256]                   ; [0x3cdac88] meta:System.IO.Compression.DeflateStream_TypeInfo
002e0a32: call   0x182f609b0
002e0a37: lea    rcx, [rip + 0x3a1d292]                   ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002e0a3e: call   0x182f609b0
002e0a43: lea    rcx, [rip + 0x39c34e6]                   ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
002e0a4a: call   0x182f609b0
002e0a4f: mov    byte ptr [rip + 0x3c2ce92], 1            ; [0x3f0d8e8] (bss)
002e0a56: mov    qword ptr [rsp + 0x50], 0
002e0a5f: mov    rcx, qword ptr [rip + 0x39c34ca]         ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
002e0a66: call   0x182f60c00
002e0a6b: mov    rdi, rax
002e0a6e: xor    r8d, r8d
002e0a71: mov    rdx, rbx
002e0a74: mov    rcx, rax
002e0a77: call   0x181af8ec0                              ; System.IO.MemoryStream$$.ctor
002e0a7c: mov    rcx, qword ptr [rip + 0x39c34ad]         ; [0x3ca3f30] meta:System.IO.MemoryStream_TypeInfo
002e0a83: call   0x182f60c00
002e0a88: mov    rsi, rax
002e0a8b: mov    qword ptr [rsp + 0x60], rax
002e0a90: xor    edx, edx
002e0a92: mov    rcx, rax
002e0a95: call   0x181af8fa0                              ; System.IO.MemoryStream$$.ctor
002e0a9a: mov    rcx, qword ptr [rip + 0x39fa1e7]         ; [0x3cdac88] meta:System.IO.Compression.DeflateStream_TypeInfo
002e0aa1: call   0x182f60c00
002e0aa6: mov    rbx, rax
002e0aa9: xor    r9d, r9d
002e0aac: xor    r8d, r8d
002e0aaf: mov    rdx, rdi
002e0ab2: mov    rcx, rax
002e0ab5: call   0x1826c8430                              ; System.IO.Compression.DeflateStream$$.ctor
002e0aba: mov    qword ptr [rsp + 0x50], rbx
002e0abf: mov    qword ptr [rsp + 0x28], 0
002e0ac8: lea    rax, [rsp + 0x50]
002e0acd: mov    qword ptr [rsp + 0x30], rax
002e0ad2: mov    rcx, qword ptr [rsp + 0x50]
002e0ad7: test   rcx, rcx
002e0ada: je     0x1802e0b57
002e0adc: xor    r8d, r8d
002e0adf: mov    rdx, rsi
002e0ae2: call   0x181b246c0                              ; System.IO.Stream$$CopyTo
002e0ae7: nop    
002e0ae8: mov    r8, qword ptr [rsp + 0x50]
002e0aed: test   r8, r8
002e0af0: je     0x1802e0b00
002e0af2: xor    ecx, ecx
002e0af4: mov    rdx, qword ptr [rip + 0x3a1d1d5]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002e0afb: call   0x182c42550
002e0b00: jmp    0x1802e0b29
002e0b02: mov    r8, qword ptr [rsp + 0x50]
002e0b07: test   r8, r8
002e0b0a: je     0x1802e0b1a
002e0b0c: xor    ecx, ecx
002e0b0e: mov    rdx, qword ptr [rip + 0x3a1d1bb]         ; [0x3cfdcd0] meta:System.IDisposable_TypeInfo
002e0b15: call   0x182c42550
002e0b1a: mov    rcx, qword ptr [rsp + 0x28]
002e0b1f: test   rcx, rcx
002e0b22: jne    0x1802e0b5d
002e0b24: mov    rsi, qword ptr [rsp + 0x60]
002e0b29: test   rsi, rsi
002e0b2c: je     0x1802e0b51
002e0b2e: mov    rax, qword ptr [rsi]
002e0b31: mov    rdx, qword ptr [rax + 0x420]
002e0b38: mov    rcx, rsi
002e0b3b: call   qword ptr [rax + 0x418]
002e0b41: mov    rbx, qword ptr [rsp + 0x58]
002e0b46: mov    rsi, qword ptr [rsp + 0x68]
002e0b4b: add    rsp, 0x40
002e0b4f: pop    rdi
002e0b50: ret    
002e0b51: call   0x182f60c50
002e0b56: nop    
002e0b57: call   0x182f60c50
002e0b5c: nop    
002e0b5d: call   0x182f60ce0
002e0b62: int3   
002e0b63: int3   
