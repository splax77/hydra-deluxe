020f16e0: mov    qword ptr [rsp + 8], rbx
020f16e5: push   rdi
020f16e6: sub    rsp, 0x20
020f16ea: cmp    byte ptr [rip + 0x1e27c87], 0            ; [0x3f19378] (bss)
020f16f1: mov    rdi, rcx
020f16f4: jne    0x1820f1715
020f16f6: lea    rcx, [rip + 0x1be5ae3]                   ; [0x3cd71e0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Count()
020f16fd: call   0x182f609b0
020f1702: lea    rcx, [rip + 0x1be5b97]                   ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f1709: call   0x182f609b0
020f170e: mov    byte ptr [rip + 0x1e27c63], 1            ; [0x3f19378] (bss)
020f1715: xor    ebx, ebx
020f1717: xor    eax, eax
020f1719: test   rdi, rdi
020f171c: je     0x1820f177d
020f171e: nop    
020f1720: cmp    eax, dword ptr [rdi + 0x18]
020f1723: jge    0x1820f1772
020f1725: mov    r8, qword ptr [rip + 0x1be5b74]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f172c: mov    edx, ebx
020f172e: mov    rcx, rdi
020f1731: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1736: test   rax, rax
020f1739: je     0x1820f177d
020f173b: test   byte ptr [rax + 0x80], 0xe0
020f1742: je     0x1820f176c
020f1744: mov    r8, qword ptr [rip + 0x1be5b55]          ; [0x3cd72a0] metamethod:Method$System.Collections.Generic.List<ʽʿʸʸʾʶʶʾʶʹʲ>.get_Item()
020f174b: mov    edx, ebx
020f174d: mov    rcx, rdi
020f1750: call   0x180726570                              ; System.Collections.Generic.List<object>$$System.Collections.IList.get_Item
020f1755: test   rax, rax
020f1758: je     0x1820f177d
020f175a: movzx  ecx, word ptr [rax + 0x80]
020f1761: shr    cx, 3
020f1765: mov    word ptr [rax + 0x80], cx
020f176c: inc    ebx
020f176e: mov    eax, ebx
020f1770: jmp    0x1820f1720
020f1772: mov    rbx, qword ptr [rsp + 0x30]
020f1777: add    rsp, 0x20
020f177b: pop    rdi
020f177c: ret    
020f177d: call   0x182f60c50
020f1782: int3   
020f1783: int3   
