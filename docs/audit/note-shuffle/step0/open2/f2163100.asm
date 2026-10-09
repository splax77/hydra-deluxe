02163100: mov    qword ptr [rsp + 8], rbx
02163105: push   rdi
02163106: sub    rsp, 0x40
0216310a: movups xmm0, xmmword ptr [rdx]
0216310d: mov    rax, qword ptr [rsp + 0x70]
02163112: lea    rdx, [rsp + 0x30]
02163117: mov    qword ptr [rsp + 0x28], 0
02163120: movaps xmmword ptr [rsp + 0x30], xmm0
02163125: mov    qword ptr [rsp + 0x20], rax
0216312a: call   0x1821619c0                              ; ʺʹˁʿʺʼʼʷʳʴʶ$$ʴʼʶʲʼʿʿʽʻˀˁ
0216312f: lea    rcx, [rip + 0x1b37ada]                   ; [0x3c9ac10] meta:ʼʵʹʴˀʴʾʿʵʲʹ_TypeInfo
02163136: mov    rbx, rax
02163139: call   0x182f609d0
0216313e: mov    rcx, rax
02163141: call   0x182f60c00
02163146: xor    r8d, r8d
02163149: mov    rdx, rbx
0216314c: mov    rcx, rax
0216314f: mov    rdi, rax
02163152: call   0x182109520                              ; ʼʵʹʴˀʴʾʿʵʲʹ$$.ctor
02163157: lea    rcx, [rip + 0x1b2f22a]                   ; [0x3c92388] metamethod:Method$ʺʹˁʿʺʼʼʷʳʴʶ.ʽʻˀʺʳʲʴʳʶʿʵ()
0216315e: call   0x182f609d0
02163163: mov    rdx, rax
02163166: mov    rcx, rdi
02163169: call   0x182f60c10
0216316e: int3   
0216316f: int3   
