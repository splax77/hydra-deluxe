0214c9e0: mov    qword ptr [rsp + 8], rbx
0214c9e5: push   rdi
0214c9e6: sub    rsp, 0x20
0214c9ea: cmp    byte ptr [rip + 0x1dccc1f], 0            ; [0x3f19610] (bss)
0214c9f1: mov    rdi, rdx
0214c9f4: mov    rbx, rcx
0214c9f7: jne    0x18214ca0c
0214c9f9: lea    rcx, [rip + 0x1b8ade0]                   ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0214ca00: call   0x182f609b0
0214ca05: mov    byte ptr [rip + 0x1dccc04], 1            ; [0x3f19610] (bss)
0214ca0c: cmp    byte ptr [rbx + 0x34], 0
0214ca10: jne    0x18214ca39
0214ca12: test   rdi, rdi
0214ca15: je     0x18214ca47
0214ca17: mov    rcx, qword ptr [rdi + 0x60]
0214ca1b: test   rcx, rcx
0214ca1e: je     0x18214ca47
0214ca20: mov    r8, qword ptr [rip + 0x1b8adb9]          ; [0x3cd77e0] metamethod:Method$System.Collections.Generic.List<ʾʳʿʽʸʸʷʾʼʴʿ>.get_Item()
0214ca27: mov    edx, dword ptr [rbx + 0x2c]
0214ca2a: mov    rbx, qword ptr [rsp + 0x30]
0214ca2f: add    rsp, 0x20
0214ca33: pop    rdi
0214ca34: jmp    0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
0214ca39: mov    rax, rbx
0214ca3c: mov    rbx, qword ptr [rsp + 0x30]
0214ca41: add    rsp, 0x20
0214ca45: pop    rdi
0214ca46: ret    
0214ca47: call   0x182f60c50
0214ca4c: int3   
0214ca4d: int3   
