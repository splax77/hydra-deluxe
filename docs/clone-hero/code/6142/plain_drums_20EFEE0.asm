020efee0: mov    qword ptr [rsp + 8], rbx
020efee5: push   rdi
020efee6: sub    rsp, 0x20
020efeea: cmp    byte ptr [rip + 0x1e29484], 0            ; [0x3f19375] (bss)
020efef1: mov    rdi, rcx
020efef4: jne    0x1820eff15
020efef6: lea    rcx, [rip + 0x1be72e3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020efefd: call   0x182f609b0
020eff02: lea    rcx, [rip + 0x1be7397]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020eff09: call   0x182f609b0
020eff0e: mov    byte ptr [rip + 0x1e29460], 1            ; [0x3f19375] (bss)
020eff15: xor    ebx, ebx
020eff17: xor    eax, eax
020eff19: test   rdi, rdi
020eff1c: je     0x1820eff53
020eff1e: nop    
020eff20: cmp    eax, dword ptr [rdi + 0x18]
020eff23: jge    0x1820eff48
020eff25: mov    r8, qword ptr [rip + 0x1be7374]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020eff2c: mov    edx, ebx
020eff2e: mov    rcx, rdi
020eff31: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020eff36: test   rax, rax
020eff39: je     0x1820eff53
020eff3b: and    dword ptr [rax + 0x7c], 0xfff3ffff
020eff42: inc    ebx
020eff44: mov    eax, ebx
020eff46: jmp    0x1820eff20
020eff48: mov    rbx, qword ptr [rsp + 0x30]
020eff4d: add    rsp, 0x20
020eff51: pop    rdi
020eff52: ret    
020eff53: call   0x182f60c50
020eff58: int3   
020eff59: int3   
