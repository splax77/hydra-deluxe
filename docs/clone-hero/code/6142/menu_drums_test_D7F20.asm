000d7f20: push   rbx
000d7f22: sub    rsp, 0x20
000d7f26: cmp    byte ptr [rip + 0x3e3429a], 0            ; [0x3f0c1c7] (bss)
000d7f2d: mov    rbx, rcx
000d7f30: jne    0x1800d7f45
000d7f32: lea    rcx, [rip + 0x3bc03e7]                   ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d7f39: call   0x182f609b0
000d7f3e: mov    byte ptr [rip + 0x3e34282], 1            ; [0x3f0c1c7] (bss)
000d7f45: xor    edx, edx
000d7f47: mov    rcx, rbx
000d7f4a: call   0x182123320                              ; ʾˀʷˁʻˀʴʳʽʻʽ$$ʼʾˁʺʴʻʵʳʾʲʺ
000d7f4f: sub    al, 2
000d7f51: cmp    al, 1
000d7f53: jbe    0x1800d7f5d
000d7f55: xor    al, al
000d7f57: add    rsp, 0x20
000d7f5b: pop    rbx
000d7f5c: ret    
000d7f5d: mov    rax, qword ptr [rip + 0x3bc03bc]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
000d7f64: mov    rcx, qword ptr [rax + 0xb8]
000d7f6b: mov    rcx, qword ptr [rcx + 0x140]
000d7f72: test   rcx, rcx
000d7f75: je     0x1800d7f83
000d7f77: xor    edx, edx
000d7f79: add    rsp, 0x20
000d7f7d: pop    rbx
000d7f7e: jmp    0x18210c2f0                              ; ʽʾʺʼʶʺʻʹʹʵʵ$$ʿˁʻʸʶʼˀʿʴʳʲ
000d7f83: call   0x182f60c50
000d7f88: int3   
000d7f89: int3   
