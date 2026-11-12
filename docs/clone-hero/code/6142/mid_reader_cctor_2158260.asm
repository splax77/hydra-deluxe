02158260: mov    qword ptr [rsp + 8], rbx
02158265: push   rdi
02158266: sub    rsp, 0x20
0215826a: cmp    byte ptr [rip + 0x1dc142f], 0            ; [0x3f196a0] (bss)
02158271: jne    0x1821582c2
02158273: lea    rcx, [rip + 0x1b54d76]                   ; [0x3cacff0] meta:System.Func<bool>_TypeInfo
0215827a: call   0x182f609b0
0215827f: lea    rcx, [rip + 0x1b74642]                   ; [0x3ccc8c8] metamethod:Method$System.Threading.ThreadLocal<bool>..ctor()
02158286: call   0x182f609b0
0215828b: lea    rcx, [rip + 0x1bb3046]                   ; [0x3d0b2d8] meta:System.Threading.ThreadLocal<bool>_TypeInfo
02158292: call   0x182f609b0
02158297: lea    rcx, [rip + 0x1b3c0a2]                   ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215829e: call   0x182f609b0
021582a3: lea    rcx, [rip + 0x1b7f586]                   ; [0x3cd7830] metamethod:Method$ʶʲʻʾʾʺʴˀʷʼˀ.<>c.ʼˁʾʳʳʽʲʺʶʸʴ()
021582aa: call   0x182f609b0
021582af: lea    rcx, [rip + 0x1ba8a3a]                   ; [0x3d00cf0] meta:ʶʲʻʾʾʺʴˀʷʼˀ.<>c_TypeInfo
021582b6: call   0x182f609b0
021582bb: mov    byte ptr [rip + 0x1dc13de], 1            ; [0x3f196a0] (bss)
021582c2: mov    rax, qword ptr [rip + 0x1ba8a27]         ; [0x3d00cf0] meta:ʶʲʻʾʾʺʴˀʷʼˀ.<>c_TypeInfo
021582c9: cmp    dword ptr [rax + 0xe0], 0
021582d0: jne    0x1821582e1
021582d2: mov    rcx, rax
021582d5: call   0x182f60cf0
021582da: mov    rax, qword ptr [rip + 0x1ba8a0f]         ; [0x3d00cf0] meta:ʶʲʻʾʾʺʴˀʷʼˀ.<>c_TypeInfo
021582e1: mov    rax, qword ptr [rax + 0xb8]
021582e8: mov    rcx, qword ptr [rip + 0x1b54d01]         ; [0x3cacff0] meta:System.Func<bool>_TypeInfo
021582ef: mov    rbx, qword ptr [rax]
021582f2: call   0x182f60c00
021582f7: mov    r8, qword ptr [rip + 0x1b7f532]          ; [0x3cd7830] metamethod:Method$ʶʲʻʾʾʺʴˀʷʼˀ.<>c.ʼˁʾʳʳʽʲʺʶʸʴ()
021582fe: xor    r9d, r9d
02158301: mov    rdx, rbx
02158304: mov    rcx, rax
02158307: mov    rdi, rax
0215830a: call   0x1808063c0                              ; Unity.Burst.BurstCompiler.BurstCompilerHelper.IsBurstEnabledDelegate$$.ctor
0215830f: mov    rcx, qword ptr [rip + 0x1bb2fc2]         ; [0x3d0b2d8] meta:System.Threading.ThreadLocal<bool>_TypeInfo
02158316: call   0x182f60c00
0215831b: mov    r8, qword ptr [rip + 0x1b745a6]          ; [0x3ccc8c8] metamethod:Method$System.Threading.ThreadLocal<bool>..ctor()
02158322: mov    rdx, rdi
02158325: mov    rcx, rax
02158328: mov    rbx, rax
0215832b: call   0x180b0b150                              ; System.Threading.ThreadLocal<bool>$$.ctor
02158330: mov    rcx, qword ptr [rip + 0x1b3c009]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
02158337: mov    rdx, qword ptr [rcx + 0xb8]
0215833e: mov    qword ptr [rdx], rbx
02158341: mov    rdx, rbx
02158344: mov    rcx, qword ptr [rip + 0x1b3bff5]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215834b: mov    rcx, qword ptr [rcx + 0xb8]
02158352: call   0x182f5fc00
02158357: mov    rax, qword ptr [rip + 0x1b3bfe2]         ; [0x3c94340] meta:ʶʲʻʾʾʺʴˀʷʼˀ_TypeInfo
0215835e: mov    rbx, qword ptr [rsp + 0x30]
02158363: mov    rcx, qword ptr [rax + 0xb8]
0215836a: mov    dword ptr [rcx + 8], 0xf
02158371: add    rsp, 0x20
02158375: pop    rdi
02158376: ret    
02158377: int3   
