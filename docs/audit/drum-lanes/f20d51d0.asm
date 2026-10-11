020d51d0: mov    qword ptr [rsp + 0x10], rbx
020d51d5: push   rdi
020d51d6: sub    rsp, 0x70
020d51da: cmp    byte ptr [rip + 0x1e4412d], 0            ; [0x3f1930e] (bss)
020d51e1: mov    ebx, edx
020d51e3: mov    rdi, rcx
020d51e6: jne    0x1820d51fb
020d51e8: lea    rcx, [rip + 0x1bc3659]                   ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d51ef: call   0x182f609b0
020d51f4: mov    byte ptr [rip + 0x1e44113], 1            ; [0x3f1930e] (bss)
020d51fb: xor    eax, eax
020d51fd: xorps  xmm0, xmm0
020d5200: mov    qword ptr [rsp + 0x30], rax
020d5205: xorps  xmm1, xmm1
020d5208: mov    qword ptr [rsp + 0x48], rax
020d520d: movups xmmword ptr [rsp + 0x20], xmm0
020d5212: movups xmmword ptr [rsp + 0x38], xmm1
020d5217: test   rdi, rdi
020d521a: je     0x1820d5307
020d5220: xor    edx, edx
020d5222: mov    qword ptr [rsp + 0x80], rsi
020d522a: mov    rcx, rdi
020d522d: call   0x18214bcd0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ˀʶˀʼʲʾʸʴʴʶʿ
020d5232: mov    rcx, qword ptr [rip + 0x1bc360f]         ; [0x3c98848] meta:ʹʾʽʼʷʲʴʸʷʼˁ_TypeInfo
020d5239: movzx  esi, ax
020d523c: cmp    dword ptr [rcx + 0xe0], 0
020d5243: jne    0x1820d524a
020d5245: call   0x182f60cf0
020d524a: movzx  ecx, bx
020d524d: xor    r8d, r8d
020d5250: and    ecx, esi
020d5252: mov    rdx, rdi
020d5255: mov    eax, ecx
020d5257: shr    eax, 1
020d5259: or     eax, ecx
020d525b: mov    ecx, eax
020d525d: shr    ecx, 2
020d5260: or     ecx, eax
020d5262: mov    eax, ecx
020d5264: shr    eax, 4
020d5267: or     eax, ecx
020d5269: lea    rcx, [rsp + 0x50]
020d526e: mov    ebx, eax
020d5270: shr    ebx, 8
020d5273: or     ebx, eax
020d5275: inc    ebx
020d5277: shr    ebx, 1
020d5279: call   0x18214b9c0                              ; ʽʿʸʸʾʶʶʾʶʹʲ$$ʽʸʷʶʸʿʶˀʹˁʻ
020d527e: xor    r8d, r8d
020d5281: lea    rdx, [rsp + 0x38]
020d5286: lea    rcx, [rsp + 0x50]
020d528b: movups xmm0, xmmword ptr [rax]
020d528e: movups xmmword ptr [rsp + 0x38], xmm0
020d5293: movsd  xmm1, qword ptr [rax + 0x10]
020d5298: movsd  qword ptr [rsp + 0x48], xmm1
020d529e: call   0x1800b2520                              ; System.IO.Pipelines.ReadResult$$get_Buffer
020d52a3: mov    rsi, qword ptr [rsp + 0x80]
020d52ab: movups xmm0, xmmword ptr [rax]
020d52ae: movups xmmword ptr [rsp + 0x20], xmm0
020d52b3: movsd  xmm1, qword ptr [rax + 0x10]
020d52b8: movsd  qword ptr [rsp + 0x30], xmm1
020d52be: nop    
020d52c0: xor    edx, edx
020d52c2: lea    rcx, [rsp + 0x20]
020d52c7: call   0x182141df0                              ; ʽʿʸʸʾʶʶʾʶʹʲ.ʺʲʹʿʴʸʴʲʹʵʷ$$ʿʵʷʻʵʻʷʵʶʺʼ
020d52cc: test   al, al
020d52ce: je     0x1820d52f7
020d52d0: mov    rax, qword ptr [rsp + 0x28]
020d52d5: test   rax, rax
020d52d8: je     0x1820d5307
020d52da: movzx  ecx, word ptr [rax + 0x80]
020d52e1: and    cx, bx
020d52e4: cmp    cx, bx
020d52e7: jne    0x1820d52c0
020d52e9: mov    rbx, qword ptr [rsp + 0x88]
020d52f1: add    rsp, 0x70
020d52f5: pop    rdi
020d52f6: ret    
020d52f7: mov    rbx, qword ptr [rsp + 0x88]
020d52ff: xor    eax, eax
020d5301: add    rsp, 0x70
020d5305: pop    rdi
020d5306: ret    
020d5307: call   0x182f60c50
020d530c: int3   
020d530d: int3   
