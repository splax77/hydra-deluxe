; function 0x210c090 ʽʾʺʼʶʺʻʹʹʵʵ$$ʸʺˀʳˁʽʸʺʲʻˁ next method starts at 0x210c0d0 bytes 0x40
0210c090: push   rbx
0210c092: sub    rsp, 0x20
0210c096: cmp    byte ptr [rip + 0x1e0d350], 0            ; [0x3f193ed] (bss)
0210c09d: mov    rbx, rcx
0210c0a0: jne    0x18210c0b5
0210c0a2: lea    rcx, [rip + 0x1bea8f7]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0210c0a9: call   0x182f609b0
0210c0ae: mov    byte ptr [rip + 0x1e0d338], 1            ; [0x3f193ed] (bss)
0210c0b5: movd   xmm0, dword ptr [rbx + 0x10]
0210c0ba: cvtdq2ps xmm0, xmm0
0210c0bd: divss  xmm0, dword ptr [rip + 0xf57f37]         ; [0x3063ffc] flt=100.0 i=1120403456
0210c0c5: add    rsp, 0x20
0210c0c9: pop    rbx
0210c0ca: ret    
0210c0cb: int3   
0210c0cc: int3   
0210c0cd: int3   
0210c0ce: int3   
0210c0cf: int3   
