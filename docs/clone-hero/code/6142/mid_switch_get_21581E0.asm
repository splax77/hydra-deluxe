021581e0: sub    rsp, 0x28
021581e4: cmp    byte ptr [rip + 0x1dc14ac], 0            ; [0x3f19697] (bss)
021581eb: jne    0x18215820c
021581ed: lea    rcx, [rip + 0x1b74794]                   ; [0x3ccc988] metamethod:Method$System.Threading.ThreadLocal<bool>.get_Value()
021581f4: call   0x182f609b0
021581f9: lea    rcx, [rip + 0x1b3c140]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02158200: call   0x182f609b0
02158205: mov    byte ptr [rip + 0x1dc148b], 1            ; [0x3f19697] (bss)
0215820c: mov    rax, qword ptr [rip + 0x1b3c12d]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02158213: cmp    dword ptr [rax + 0xe0], 0
0215821a: jne    0x18215822b
0215821c: mov    rcx, rax
0215821f: call   0x182f60cf0
02158224: mov    rax, qword ptr [rip + 0x1b3c115]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215822b: mov    rax, qword ptr [rax + 0xb8]
02158232: mov    rcx, qword ptr [rax]
02158235: test   rcx, rcx
02158238: je     0x18215824a
0215823a: mov    rdx, qword ptr [rip + 0x1b74747]         ; [0x3ccc988] metamethod:Method$System.Threading.ThreadLocal<bool>.get_Value()
02158241: add    rsp, 0x28
02158245: jmp    0x180b0b810                              ; System.Threading.ThreadLocal<bool>$$get_Value
0215824a: call   0x182f60c50
0215824f: int3   
02158250: cmp    ecx, 0x24
02158253: jge    0x182158258
02158255: xor    al, al
02158257: ret    
02158258: cmp    ecx, 0x54
0215825b: setle  al
0215825e: ret    
0215825f: int3   
