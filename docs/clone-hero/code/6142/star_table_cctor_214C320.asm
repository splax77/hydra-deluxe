0214c320: push   rbx
0214c322: sub    rsp, 0x20
0214c326: cmp    byte ptr [rip + 0x1dcd32f], 0            ; [0x3f1965c] (bss)
0214c32d: jne    0x18214c35a
0214c32f: lea    rcx, [rip + 0x1b59c3a]                   ; [0x3ca5f70] meta:float[]_TypeInfo
0214c336: call   0x182f609b0
0214c33b: lea    rcx, [rip + 0x1b50c9e]                   ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0214c342: call   0x182f609b0
0214c347: lea    rcx, [rip + 0x1bbc9c2]                   ; [0x3d08d10] meta:Field$<PrivateImplementationDetails>.6C5DB40D03F03CEC0F0118D6E9EBCC0AFF35BC80669E4B618B21CEE03E6AAA9B
0214c34e: call   0x182f609b0
0214c353: mov    byte ptr [rip + 0x1dcd302], 1            ; [0x3f1965c] (bss)
0214c35a: mov    rcx, qword ptr [rip + 0x1b59c0f]         ; [0x3ca5f70] meta:float[]_TypeInfo
0214c361: mov    edx, 9
0214c366: call   0x182f5fc80
0214c36b: mov    rdx, qword ptr [rip + 0x1bbc99e]         ; [0x3d08d10] meta:Field$<PrivateImplementationDetails>.6C5DB40D03F03CEC0F0118D6E9EBCC0AFF35BC80669E4B618B21CEE03E6AAA9B
0214c372: xor    r8d, r8d
0214c375: mov    rcx, rax
0214c378: mov    rbx, rax
0214c37b: call   0x181a47220                              ; System.Runtime.CompilerServices.RuntimeHelpers$$InitializeArray
0214c380: mov    rcx, qword ptr [rip + 0x1b50c59]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0214c387: mov    rdx, qword ptr [rcx + 0xb8]
0214c38e: mov    qword ptr [rdx], rbx
0214c391: mov    rdx, rbx
0214c394: mov    rcx, qword ptr [rip + 0x1b50c45]         ; [0x3c9cfe0] meta:ʾʲʷˁʸˀʵʾʺˀˁ_TypeInfo
0214c39b: mov    rcx, qword ptr [rcx + 0xb8]
0214c3a2: add    rsp, 0x20
0214c3a6: pop    rbx
0214c3a7: jmp    0x182f5fc00
0214c3ac: int3   
