020ddc80: push   rbx
020ddc82: sub    rsp, 0x20
020ddc86: cmp    byte ptr [rcx + 0x2f2], 0
020ddc8d: mov    rbx, rcx
020ddc90: je     0x1820ddcad
020ddc92: cmp    byte ptr [rcx + 0x154], 0
020ddc99: je     0x1820ddcad
020ddc9b: cmp    byte ptr [rcx + 0x2f3], 0
020ddca2: jne    0x1820ddcb6
020ddca4: mov    byte ptr [rcx + 0x2f3], 1
020ddcab: jmp    0x1820ddcc6
020ddcad: cmp    byte ptr [rcx + 0x2f3], 0
020ddcb4: je     0x1820ddcc6
020ddcb6: cmp    dword ptr [rcx + 0x8c], 0
020ddcbd: je     0x1820ddccf
020ddcbf: mov    byte ptr [rcx + 0x2f3], 0
020ddcc6: cmp    dword ptr [rcx + 0x8c], 0
020ddccd: jne    0x1820ddcf3
020ddccf: cmp    byte ptr [rcx + 0x154], 0
020ddcd6: jne    0x1820ddce7
020ddcd8: cmp    byte ptr [rcx + 0x2f3], 0
020ddcdf: jne    0x1820ddce7
020ddce1: cmp    byte ptr [rcx + 0x40], 0
020ddce5: jne    0x1820ddcf3
020ddce7: xor    edx, edx
020ddce9: add    rsp, 0x20
020ddced: pop    rbx
020ddcee: jmp    0x1820de240                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʻʺʵʹʼʷʿʶˀʸʿ
020ddcf3: cmp    byte ptr [rcx + 0xf0], 0
020ddcfa: je     0x1820ddd12
020ddcfc: xor    edx, edx
020ddcfe: call   0x1820de240                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʻʺʵʹʼʷʿʶˀʸʿ
020ddd03: xor    edx, edx
020ddd05: mov    rcx, rbx
020ddd08: add    rsp, 0x20
020ddd0c: pop    rbx
020ddd0d: jmp    0x1820deaa0                              ; ʺʺʸʶʿʵˀʸʲˀʲ$$ʿʶʲʳʲʴʴʵʴʲʷ
020ddd12: add    rsp, 0x20
020ddd16: pop    rbx
020ddd17: ret    
020ddd18: int3   
