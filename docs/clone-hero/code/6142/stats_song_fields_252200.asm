00252200: push   rbx
00252202: push   rbp
00252203: push   rsi
00252204: push   rdi
00252205: sub    rsp, 0x38
00252209: cmp    byte ptr [rip + 0x3cbad44], 0            ; [0x3f0cf54] (bss)
00252210: mov    esi, r9d
00252213: mov    ebp, r8d
00252216: mov    rdi, rdx
00252219: mov    rbx, rcx
0025221c: jne    0x18025223d
0025221e: lea    rcx, [rip + 0x3a852eb]                   ; [0x3cd7510] meta:System.DateTime_TypeInfo
00252225: call   0x182f609b0
0025222a: lea    rcx, [rip + 0x3a9d65f]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00252231: call   0x182f609b0
00252236: mov    byte ptr [rip + 0x3cbad17], 1            ; [0x3f0cf54] (bss)
0025223d: mov    rax, qword ptr [rip + 0x3a9d64c]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00252244: mov    qword ptr [rsp + 0x20], 0
0025224d: cmp    dword ptr [rax + 0xe0], 0
00252254: jne    0x180252265
00252256: mov    rcx, rax
00252259: call   0x182f60cf0
0025225e: mov    rax, qword ptr [rip + 0x3a9d62b]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00252265: mov    rax, qword ptr [rax + 0xb8]
0025226c: mov    rdx, qword ptr [rax + 8]
00252270: test   rdx, rdx
00252273: je     0x18025237c
00252279: mov    rdx, qword ptr [rdx + 0x60]
0025227d: lea    rcx, [rbx + 0x10]
00252281: mov    qword ptr [rbx + 0x10], rdx
00252285: call   0x182f5fc00
0025228a: mov    rax, qword ptr [rip + 0x3a9d5ff]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
00252291: mov    rcx, qword ptr [rax + 0xb8]
00252298: mov    rax, qword ptr [rcx + 8]
0025229c: test   rax, rax
0025229f: je     0x18025237c
002522a5: mov    eax, dword ptr [rax + 0x50]
002522a8: mov    dword ptr [rbx + 0x3c], eax
002522ab: test   rdi, rdi
002522ae: je     0x18025237c
002522b4: xor    edx, edx
002522b6: mov    rcx, rdi
002522b9: call   0x1803603f0                              ; SongEntry$$get_ChecksumString
002522be: lea    rcx, [rbx + 0x18]
002522c2: mov    qword ptr [rbx + 0x18], rax
002522c6: mov    rdx, rax
002522c9: call   0x182f5fc00
002522ce: xor    edx, edx
002522d0: mov    rcx, rdi
002522d3: call   0x18035a5f0                              ; SongEntry$$ʷˁʼˁʴʸʾʻʿʿʿ
002522d8: test   rax, rax
002522db: je     0x18025237c
002522e1: mov    rdx, qword ptr [rax + 0x10]
002522e5: lea    rcx, [rbx + 0x20]
002522e9: mov    qword ptr [rbx + 0x20], rdx
002522ed: call   0x182f5fc00
002522f2: xor    edx, edx
002522f4: mov    rcx, rdi
002522f7: call   0x1803600c0                              ; SongEntry$$get_Artist
002522fc: test   rax, rax
002522ff: je     0x18025237c
00252301: mov    rdx, qword ptr [rax + 0x10]
00252305: lea    rcx, [rbx + 0x28]
00252309: mov    qword ptr [rbx + 0x28], rdx
0025230d: call   0x182f5fc00
00252312: xor    edx, edx
00252314: mov    rcx, rdi
00252317: call   0x1803603c0                              ; SongEntry$$get_Charter
0025231c: test   rax, rax
0025231f: je     0x18025237c
00252321: mov    rdx, qword ptr [rax + 0x10]
00252325: lea    rcx, [rbx + 0x30]
00252329: mov    qword ptr [rbx + 0x30], rdx
0025232d: call   0x182f5fc00
00252332: mov    eax, dword ptr [rsp + 0x80]
00252339: mov    dword ptr [rbx + 0x4c], eax
0025233c: mov    dword ptr [rbx + 0x38], ebp
0025233f: mov    dword ptr [rbx + 0x48], esi
00252342: mov    rcx, qword ptr [rip + 0x3a851c7]         ; [0x3cd7510] meta:System.DateTime_TypeInfo
00252349: cmp    dword ptr [rcx + 0xe0], 0
00252350: jne    0x180252357
00252352: call   0x182f60cf0
00252357: xor    ecx, ecx
00252359: call   0x181b46c70                              ; System.DateTime$$get_Now
0025235e: xor    edx, edx
