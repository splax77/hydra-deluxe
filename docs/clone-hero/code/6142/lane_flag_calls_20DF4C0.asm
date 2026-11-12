020df4c0: push   rbx
020df4c2: sub    rsp, 0x30
020df4c6: cmp    byte ptr [rip + 0x1e39dee], 0            ; [0x3f192bb] (bss)
020df4cd: mov    rbx, rcx
020df4d0: jne    0x1820df4e5
020df4d2: lea    rcx, [rip + 0x1bb8577]                   ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df4d9: call   0x182f609b0
020df4de: mov    byte ptr [rip + 0x1e39dd6], 1            ; [0x3f192bb] (bss)
020df4e5: cmp    byte ptr [rbx + 0x290], 0
020df4ec: je     0x1820df672
020df4f2: movzx  eax, word ptr [rbx + 0x2f4]
020df4f9: test   eax, 0xffffff9d
020df4fe: je     0x1820df52b
020df500: mov    r8, qword ptr [rbx + 0x318]
020df507: test   r8, r8
020df50a: je     0x1820df52b
020df50c: movzx  eax, byte ptr [rbx + 0x30c]
020df513: xor    ecx, ecx
020df515: mov    rdx, qword ptr [rip + 0x1bb8534]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df51c: mov    r9d, 0x2c
020df522: mov    byte ptr [rsp + 0x20], al
020df526: call   0x182efbcf0
020df52b: test   byte ptr [rbx + 0x2f4], 3
020df532: je     0x1820df55f
020df534: mov    r8, qword ptr [rbx + 0x318]
020df53b: test   r8, r8
020df53e: je     0x1820df55f
020df540: movzx  eax, byte ptr [rbx + 0x30a]
020df547: xor    ecx, ecx
020df549: mov    rdx, qword ptr [rip + 0x1bb8500]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df550: mov    r9d, 8
020df556: mov    byte ptr [rsp + 0x20], al
020df55a: call   0x182efbcf0
020df55f: test   byte ptr [rbx + 0x2f4], 2
020df566: je     0x1820df593
020df568: mov    r8, qword ptr [rbx + 0x318]
020df56f: test   r8, r8
020df572: je     0x1820df593
020df574: movzx  eax, byte ptr [rbx + 0x30b]
020df57b: xor    ecx, ecx
020df57d: mov    rdx, qword ptr [rip + 0x1bb84cc]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df584: mov    r9d, 8
020df58a: mov    byte ptr [rsp + 0x20], al
020df58e: call   0x182efbcf0
020df593: test   byte ptr [rbx + 0x2f4], 4
020df59a: je     0x1820df5c7
020df59c: mov    r8, qword ptr [rbx + 0x318]
020df5a3: test   r8, r8
020df5a6: je     0x1820df5c7
020df5a8: movzx  eax, byte ptr [rbx + 0x309]
020df5af: xor    ecx, ecx
020df5b1: mov    rdx, qword ptr [rip + 0x1bb8498]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df5b8: mov    r9d, 3
020df5be: mov    byte ptr [rsp + 0x20], al
020df5c2: call   0x182efbcf0
020df5c7: movzx  eax, word ptr [rbx + 0x2f4]
020df5ce: test   eax, 0xffffffd7
020df5d3: je     0x1820df600
020df5d5: mov    r8, qword ptr [rbx + 0x318]
020df5dc: test   r8, r8
020df5df: je     0x1820df600
020df5e1: movzx  eax, byte ptr [rbx + 0x310]
020df5e8: xor    ecx, ecx
020df5ea: mov    rdx, qword ptr [rip + 0x1bb845f]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df5f1: mov    r9d, 0x3b
020df5f7: mov    byte ptr [rsp + 0x20], al
020df5fb: call   0x182efbcf0
020df600: movzx  eax, word ptr [rbx + 0x2f4]
020df607: test   eax, 0xffffffc3
020df60c: je     0x1820df639
020df60e: mov    r8, qword ptr [rbx + 0x318]
020df615: test   r8, r8
020df618: je     0x1820df639
020df61a: movzx  eax, byte ptr [rbx + 0x30e]
020df621: xor    ecx, ecx
020df623: mov    rdx, qword ptr [rip + 0x1bb8426]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df62a: mov    r9d, 0xffc2
020df630: mov    byte ptr [rsp + 0x20], al
020df634: call   0x182efbcf0
020df639: movzx  eax, word ptr [rbx + 0x2f4]
020df640: test   eax, 0xffffffc2
020df645: je     0x1820df672
020df647: mov    r8, qword ptr [rbx + 0x318]
020df64e: test   r8, r8
020df651: je     0x1820df672
020df653: movzx  eax, byte ptr [rbx + 0x30f]
020df65a: xor    ecx, ecx
020df65c: mov    rdx, qword ptr [rip + 0x1bb83ed]         ; [0x3c97a50] meta:ʹʲʻʵʲʻˀʿʵʽˁ_TypeInfo
020df663: mov    r9d, 0xff82
020df669: mov    byte ptr [rsp + 0x20], al
020df66d: call   0x182efbcf0
020df672: add    rsp, 0x30
020df676: pop    rbx
020df677: ret    
020df678: int3   
