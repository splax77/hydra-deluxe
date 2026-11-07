; function 0xb41a0 GameManager$$Awake next method starts at 0xb4500 bytes 0x360
000b41a0: mov    qword ptr [rsp + 8], rbx
000b41a5: push   rdi
000b41a6: sub    rsp, 0x20
000b41aa: cmp    byte ptr [rip + 0x3e57fe9], 0            ; [0x3f0c19a] (bss)
000b41b1: mov    rbx, rcx
000b41b4: jne    0x1800b4251
000b41ba: lea    rcx, [rip + 0x3c0e2af]                   ; [0x3cc2470] metamethod:Method$GameManager.ʷʻʶʶʴʶˀʼʽʽʺ()
000b41c1: call   0x182f609b0
000b41c6: lea    rcx, [rip + 0x3c3b6c3]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b41cd: call   0x182f609b0
000b41d2: lea    rcx, [rip + 0x3bec2b7]                   ; [0x3ca0490] metamethod:Method$System.Collections.Generic.List<int>.get_Item()
000b41d9: call   0x182f609b0
000b41de: lea    rcx, [rip + 0x3bfea83]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b41e5: call   0x182f609b0
000b41ea: lea    rcx, [rip + 0x3c167af]                   ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
000b41f1: call   0x182f609b0
000b41f6: lea    rcx, [rip + 0x3c427a3]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
000b41fd: call   0x182f609b0
000b4202: lea    rcx, [rip + 0x3c429c7]                   ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
000b4209: call   0x182f609b0
000b420e: lea    rcx, [rip + 0x3be410b]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b4215: call   0x182f609b0
000b421a: lea    rcx, [rip + 0x3be8e77]                   ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
000b4221: call   0x182f609b0
000b4226: lea    rcx, [rip + 0x3bdf12b]                   ; [0x3c93358] meta:UnityEngine.Events.UnityAction<Scene, Scene>_TypeInfo
000b422d: call   0x182f609b0
000b4232: lea    rcx, [rip + 0x3c177f7]                   ; [0x3ccba30] str:'Main Menu'
000b4239: call   0x182f609b0
000b423e: lea    rcx, [rip + 0x3c55523]                   ; [0x3d09768] str:'_OutputHue'
000b4245: call   0x182f609b0
000b424a: mov    byte ptr [rip + 0x3e57f49], 1            ; [0x3f0c19a] (bss)
000b4251: mov    rax, qword ptr [rip + 0x3c3b638]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b4258: cmp    dword ptr [rax + 0xe0], 0
000b425f: jne    0x1800b4270
000b4261: mov    rcx, rax
000b4264: call   0x182f60cf0
000b4269: mov    rax, qword ptr [rip + 0x3c3b620]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b4270: mov    rcx, qword ptr [rip + 0x3bfe9f1]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
000b4277: mov    rax, qword ptr [rax + 0xb8]
000b427e: cmp    dword ptr [rcx + 0xe0], 0
000b4285: mov    rdi, qword ptr [rax + 8]
000b4289: jne    0x1800b4290
000b428b: call   0x182f60cf0
000b4290: xor    r8d, r8d
000b4293: xor    edx, edx
000b4295: mov    rcx, rdi
000b4298: call   0x18287fd30                              ; UnityEngine.Object$$op_Equality
000b429d: test   al, al
000b429f: jne    0x1800b44c2
000b42a5: mov    rax, qword ptr [rip + 0x3c3b5e4]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b42ac: cmp    dword ptr [rax + 0xe0], 0
000b42b3: jne    0x1800b42c4
000b42b5: mov    rcx, rax
000b42b8: call   0x182f60cf0
000b42bd: mov    rax, qword ptr [rip + 0x3c3b5cc]         ; [0x3cef890] meta:GlobalVariables_TypeInfo
000b42c4: mov    rax, qword ptr [rax + 0xb8]
000b42cb: lea    rcx, [rbx + 0x28]
000b42cf: mov    rdx, qword ptr [rax + 8]
000b42d3: mov    qword ptr [rbx + 0x28], rdx
000b42d7: call   0x182f5fc00
000b42dc: mov    rcx, qword ptr [rip + 0x3bdf075]         ; [0x3c93358] meta:UnityEngine.Events.UnityAction<Scene, Scene>_TypeInfo
000b42e3: call   0x182f60c00
000b42e8: mov    r8, qword ptr [rip + 0x3c0e181]          ; [0x3cc2470] metamethod:Method$GameManager.ʷʻʶʶʴʶˀʼʽʽʺ()
000b42ef: xor    r9d, r9d
000b42f2: mov    rdx, rbx
000b42f5: mov    rcx, rax
000b42f8: mov    rdi, rax
000b42fb: call   0x180b96db0                              ; UnityEngine.Events.UnityAction<Scene, Scene>$$.ctor
000b4300: mov    rcx, qword ptr [rip + 0x3c16699]         ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
000b4307: cmp    dword ptr [rcx + 0xe0], 0
000b430e: jne    0x1800b4315
000b4310: call   0x182f60cf0
000b4315: xor    edx, edx
000b4317: mov    rcx, rdi
000b431a: call   0x18289c3b0                              ; UnityEngine.SceneManagement.SceneManager$$add_activeSceneChanged
000b431f: mov    rcx, qword ptr [rip + 0x3c55442]         ; [0x3d09768] str:'_OutputHue'
000b4326: xor    edx, edx
000b4328: call   0x182871ac0                              ; UnityEngine.Shader$$PropertyToID
000b432d: xor    r8d, r8d
000b4330: mov    dword ptr [rbx + 0x170], eax
000b4336: xorps  xmm1, xmm1
000b4339: mov    ecx, eax
000b433b: call   0x182871b00                              ; UnityEngine.Shader$$SetGlobalFloat
000b4340: mov    rax, qword ptr [rip + 0x3be3fd9]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b4347: mov    rcx, qword ptr [rax + 0xb8]
000b434e: mov    rcx, qword ptr [rcx + 0x10]
000b4352: test   rcx, rcx
000b4355: je     0x1800b44ef
000b435b: xor    edx, edx
000b435d: call   0x18210c090                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʸʺˀʳˁʽʸʺʲʻˁ
000b4362: movss  dword ptr [rbx + 0xd8], xmm0
000b436a: mov    rax, qword ptr [rip + 0x3be3faf]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b4371: mov    rcx, qword ptr [rax + 0xb8]
000b4378: mov    rax, qword ptr [rcx + 0x10]
000b437c: test   rax, rax
000b437f: je     0x1800b44ef
000b4385: mov    eax, dword ptr [rax + 0x10]
000b4388: sub    eax, 0x64
000b438b: movd   xmm0, eax
000b438f: mov    rax, qword ptr [rbx + 0x28]
000b4393: cvtdq2ps xmm0, xmm0
000b4396: movss  dword ptr [rbx + 0xdc], xmm0
000b439e: test   rax, rax
000b43a1: je     0x1800b44ef
000b43a7: cmp    byte ptr [rax + 0x74], 0
000b43ab: je     0x1800b4428
000b43ad: mov    rax, qword ptr [rip + 0x3be3f6c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b43b4: mov    rcx, qword ptr [rax + 0xb8]
000b43bb: mov    rax, qword ptr [rip + 0x3be8cd6]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
000b43c2: mov    rdi, qword ptr [rcx + 0x10]
000b43c6: cmp    dword ptr [rax + 0xe0], 0
000b43cd: jne    0x1800b43de
000b43cf: mov    rcx, rax
000b43d2: call   0x182f60cf0
000b43d7: mov    rax, qword ptr [rip + 0x3be8cba]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
000b43de: mov    rdx, qword ptr [rbx + 0x28]
000b43e2: test   rdx, rdx
000b43e5: je     0x1800b44ef
000b43eb: mov    rax, qword ptr [rax + 0xb8]
000b43f2: mov    rcx, qword ptr [rax + 0x28]
000b43f6: test   rcx, rcx
000b43f9: je     0x1800b44ef
000b43ff: mov    r8, qword ptr [rip + 0x3bec08a]          ; [0x3ca0490] metamethod:Method$System.Collections.Generic.List<int>.get_Item()
000b4406: mov    edx, dword ptr [rdx + 0x58]
000b4409: call   0x1807452d0                              ; System.Collections.Generic.List<RegexCharClass.SingleRange>$$get_Item
000b440e: test   rdi, rdi
000b4411: je     0x1800b44ef
000b4417: mov    r8, qword ptr [rip + 0x3c427b2]          ; [0x3cf6bd0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʸʾʵʽʳʾʾʻʷʲʽ()
000b441e: mov    edx, eax
000b4420: mov    rcx, rdi
000b4423: call   0x180fe5f50                              ; ʹʶʿʹʲʳʲʲʼʵʶ<int>$$ʸʾʵʽʳʾʾʻʷʲʽ
000b4428: mov    rcx, qword ptr [rbx + 0x28]
000b442c: test   rcx, rcx
000b442f: je     0x1800b44ef
000b4435: xor    r8d, r8d
000b4438: xor    edx, edx
000b443a: call   0x180236d10                              ; GlobalVariables$$ʽʵʴʲʺʵʽʿʾʼʻ
000b443f: mov    rcx, qword ptr [rbx + 0x38]
000b4443: xor    eax, eax
000b4445: mov    qword ptr [rbx + 0xf0], rax
000b444c: mov    qword ptr [rbx + 0xe0], rax
000b4453: test   rcx, rcx
000b4456: je     0x1800b44ef
000b445c: xor    edx, edx
000b445e: call   0x180185490                              ; SongBackground$$ʿˀʺʺʶʲʻʾʾʻʸ
000b4463: mov    rax, qword ptr [rip + 0x3be3eb6]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b446a: mov    rcx, qword ptr [rax + 0xb8]
000b4471: mov    rcx, qword ptr [rcx + 0x20]
000b4475: test   rcx, rcx
000b4478: je     0x1800b44ef
000b447a: xor    edx, edx
000b447c: call   0x18210c1f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʽʺʲʴʼʿˁʼʻˁʴ
000b4481: movss  dword ptr [rbx + 0x68], xmm0
000b4486: mov    rax, qword ptr [rip + 0x3be3e93]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000b448d: mov    rcx, qword ptr [rax + 0xb8]
000b4494: mov    rcx, qword ptr [rcx + 0x18]
000b4498: test   rcx, rcx
000b449b: je     0x1800b44ef
000b449d: xor    edx, edx
000b449f: call   0x18210c1f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʽʺʲʴʼʿˁʼʻˁʴ
000b44a4: movss  xmm1, dword ptr [rbx + 0x68]
000b44a9: subss  xmm1, xmm0
000b44ad: movss  dword ptr [rbx + 0x6c], xmm0
000b44b2: movss  dword ptr [rbx + 0x68], xmm1
000b44b7: mov    rbx, qword ptr [rsp + 0x30]
000b44bc: add    rsp, 0x20
000b44c0: pop    rdi
000b44c1: ret    
000b44c2: mov    rcx, qword ptr [rip + 0x3c164d7]         ; [0x3cca9a0] meta:UnityEngine.SceneManagement.SceneManager_TypeInfo
000b44c9: cmp    dword ptr [rcx + 0xe0], 0
000b44d0: jne    0x1800b44d7
000b44d2: call   0x182f60cf0
000b44d7: mov    rcx, qword ptr [rip + 0x3c17552]         ; [0x3ccba30] str:'Main Menu'
000b44de: xor    edx, edx
000b44e0: mov    rbx, qword ptr [rsp + 0x30]
000b44e5: add    rsp, 0x20
000b44e9: pop    rdi
000b44ea: jmp    0x18289c250                              ; UnityEngine.SceneManagement.SceneManager$$LoadScene
000b44ef: call   0x182f60c50
000b44f4: int3   
000b44f5: int3   
000b44f6: int3   
000b44f7: int3   
000b44f8: int3   
000b44f9: int3   
000b44fa: int3   
000b44fb: int3   
000b44fc: int3   
000b44fd: int3   
000b44fe: int3   
000b44ff: int3   
