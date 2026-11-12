00233198: call   0x181f73220                              ; Rewired.ReInput$$get_configuration
0023319d: mov    rcx, qword ptr [rip + 0x3a6517c]         ; [0x3c98320] meta:ʹʺʽˁʽˁˀʼʶʷʼ_TypeInfo
002331a4: mov    rdx, qword ptr [rcx + 0xb8]
002331ab: mov    rdx, qword ptr [rdx + 0xc0]
002331b2: test   rdx, rdx
002331b5: je     0x180233634
002331bb: test   rax, rax
002331be: je     0x180233634
002331c4: mov    edx, dword ptr [rdx + 0x10]
002331c7: xor    r8d, r8d
002331ca: mov    rcx, rax
002331cd: call   0x181f621b0                              ; Rewired.ReInput.ConfigHelper$$set_joystickRefreshRate
