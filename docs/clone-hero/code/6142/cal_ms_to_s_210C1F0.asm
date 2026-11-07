; function 0x210c1f0 ʽʾʺʼʶʺʻʹʹʵʵ$$ʽʺʲʴʼʿˁʼʻˁʴ next method starts at 0x210c230 bytes 0x40
0210c1f0: push   rbx
0210c1f2: sub    rsp, 0x20
0210c1f6: cmp    byte ptr [rip + 0x1e0d1f3], 0            ; [0x3f193f0] (bss)
0210c1fd: mov    rbx, rcx
0210c200: jne    0x18210c215
0210c202: lea    rcx, [rip + 0x1bea797]                   ; [0x3cf69a0] metamethod:Method$ʹʶʿʹʲʳʲʲʼʵʶ<int>.ʴʾʶʹʳʷʸˀʳʼʽ()
0210c209: call   0x182f609b0
0210c20e: mov    byte ptr [rip + 0x1e0d1db], 1            ; [0x3f193f0] (bss)
0210c215: movd   xmm0, dword ptr [rbx + 0x10]
0210c21a: cvtdq2ps xmm0, xmm0
0210c21d: divss  xmm0, dword ptr [rip + 0xf586df]         ; [0x3064904] flt=1000.0 i=1148846080
0210c225: add    rsp, 0x20
0210c229: pop    rbx
0210c22a: ret    
0210c22b: int3   
0210c22c: int3   
0210c22d: int3   
0210c22e: int3   
0210c22f: int3   
