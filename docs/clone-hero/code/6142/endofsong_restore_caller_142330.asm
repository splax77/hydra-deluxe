00142330: mov    qword ptr [rsp + 8], rbx
00142335: push   rdi
00142336: sub    rsp, 0x20
0014233a: cmp    byte ptr [rip + 0x3dca1f0], 0            ; [0x3f0c531] (bss)
00142341: movzx  ebx, dl
00142344: mov    rdi, rcx
00142347: jne    0x180142380
00142349: lea    rcx, [rip + 0x3bad540]                   ; [0x3cef890] meta:GlobalVariables_TypeInfo
00142350: call   0x182f609b0
00142355: lea    rcx, [rip + 0x3b6f634]                   ; [0x3cb1990] metamethod:Method$UnityEngine.Object.FindObjectOfType<EndOfSong>()
0014235c: call   0x182f609b0
00142361: lea    rcx, [rip + 0x3b70900]                   ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
00142368: call   0x182f609b0
0014236d: lea    rcx, [rip + 0x3b5ad24]                   ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
00142374: call   0x182f609b0
00142379: mov    byte ptr [rip + 0x3dca1b1], 1            ; [0x3f0c531] (bss)
00142380: cmp    bl, 1
00142383: je     0x180142446
00142389: cmp    bl, 3
0014238c: je     0x1801423f4
0014238e: cmp    bl, 5
00142391: je     0x1801423a0
00142393: xor    al, al
00142395: mov    rbx, qword ptr [rsp + 0x30]
0014239a: add    rsp, 0x20
0014239e: pop    rdi
0014239f: ret    
001423a0: mov    rcx, qword ptr [rip + 0x3b708c1]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
001423a7: cmp    dword ptr [rcx + 0xe0], 0
001423ae: jne    0x1801423b5
001423b0: call   0x182f60cf0
001423b5: mov    rcx, qword ptr [rip + 0x3b6f5d4]         ; [0x3cb1990] metamethod:Method$UnityEngine.Object.FindObjectOfType<EndOfSong>()
001423bc: call   0x1804e6370                              ; UnityEngine.Object$$FindObjectOfType<object>
001423c1: test   rax, rax
001423c4: je     0x1801424d1
001423ca: xor    edx, edx
001423cc: mov    rcx, rax
001423cf: call   0x180160650                              ; EndOfSong$$ʹʵʵʽˀʼʺʽʽʷʹ
001423d4: lea    rcx, [rdi + 0x20]
001423d8: mov    qword ptr [rdi + 0x20], 0
001423e0: xor    edx, edx
001423e2: call   0x182f5fc00
001423e7: mov    al, 1
001423e9: mov    rbx, qword ptr [rsp + 0x30]
001423ee: add    rsp, 0x20
001423f2: pop    rdi
001423f3: ret    
001423f4: mov    rcx, qword ptr [rip + 0x3b7086d]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
001423fb: cmp    dword ptr [rcx + 0xe0], 0
00142402: jne    0x180142409
00142404: call   0x182f60cf0
00142409: mov    rcx, qword ptr [rip + 0x3b6f580]         ; [0x3cb1990] metamethod:Method$UnityEngine.Object.FindObjectOfType<EndOfSong>()
00142410: call   0x1804e6370                              ; UnityEngine.Object$$FindObjectOfType<object>
00142415: test   rax, rax
00142418: je     0x1801424d1
0014241e: xor    edx, edx
00142420: mov    rcx, rax
00142423: call   0x180160ab0                              ; EndOfSong$$ʺʷʷʾˀʼˁʹʼʻʸ
00142428: mov    rcx, qword ptr [rip + 0x3b5ac69]         ; [0x3c9d098] meta:ʾʲʽʻʺʶʺʺʷʿʺ_TypeInfo
0014242f: cmp    dword ptr [rcx + 0xe0], 0
00142436: jne    0x18014243d
00142438: call   0x182f60cf0
0014243d: xor    ecx, ecx
0014243f: call   0x180171ff0                              ; ʾʲʽʻʺʶʺʺʷʿʺ$$ʴʿʽˁˀʼʷʴˀʴˁ
00142444: jmp    0x1801423d4
00142446: mov    rcx, qword ptr [rip + 0x3b7081b]         ; [0x3cb2c68] meta:UnityEngine.Object_TypeInfo
