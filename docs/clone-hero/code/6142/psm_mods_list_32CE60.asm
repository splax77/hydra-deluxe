0032ce60: push   rbx
0032ce62: sub    rsp, 0x20
0032ce66: cmp    byte ptr [rip + 0x3be0ebe], 0            ; [0x3f0dd2b] (bss)
0032ce6d: mov    rbx, rcx
0032ce70: jne    0x18032cecd
0032ce72: lea    rcx, [rip + 0x3979b97]                   ; [0x3ca6a10] metamethod:Method$System.Linq.Enumerable.ToArray<ʷʴʼʴʴʷˁʵʺʴˁ>()
0032ce79: call   0x182f609b0
0032ce7e: lea    rcx, [rip + 0x397d44b]                   ; [0x3caa2d0] metamethod:Method$System.Linq.Enumerable.Where<ʷʴʼʴʴʷˁʵʺʴˁ>()
0032ce85: call   0x182f609b0
0032ce8a: lea    rcx, [rip + 0x3991f0f]                   ; [0x3cbeda0] meta:System.Func<ʷʴʼʴʴʷˁʵʺʴˁ, bool>_TypeInfo
0032ce91: call   0x182f609b0
0032ce96: lea    rcx, [rip + 0x3982aa3]                   ; [0x3caf940] metamethod:Method$PlayerSelectionMenu.ʶʴʿʺʽʽʳʳʻʼʲ.ʲʹˁʶˁʲʵʺʿʾʹ()
0032ce9d: call   0x182f609b0
0032cea2: lea    rcx, [rip + 0x39aaf2f]                   ; [0x3cd7dd8] meta:PlayerSelectionMenu.ʶʴʿʺʽʽʳʳʻʼʲ_TypeInfo
0032cea9: call   0x182f609b0
0032ceae: lea    rcx, [rip + 0x397e853]                   ; [0x3cab708] meta:ʷʴʼʴʴʷˁʵʺʴˁ[]_TypeInfo
0032ceb5: call   0x182f609b0
0032ceba: lea    rcx, [rip + 0x39ca767]                   ; [0x3cf7628] meta:Field$<PrivateImplementationDetails>.7FCC070037949F0E0370384585837FB80BC92ADA1017FC501AA3885C2D4067E5
0032cec1: call   0x182f609b0
0032cec6: mov    byte ptr [rip + 0x3be0e5e], 1            ; [0x3f0dd2b] (bss)
0032cecd: cmp    byte ptr [rbx + 0x242], 0
0032ced4: mov    qword ptr [rsp + 0x38], rdi
0032ced9: je     0x18032cee4
0032cedb: mov    rdi, qword ptr [rbx + 0x258]
0032cee2: jmp    0x18032cefd
0032cee4: cmp    byte ptr [rbx + 0x243], 0
0032ceeb: je     0x18032cef6
0032ceed: mov    rdi, qword ptr [rbx + 0x250]
0032cef4: jmp    0x18032cefd
0032cef6: mov    rdi, qword ptr [rbx + 0x248]
0032cefd: xor    ecx, ecx
0032ceff: mov    qword ptr [rsp + 0x30], rsi
0032cf04: call   0x180139ad0                              ; LeaderboardsOnlineManager$$ʿʲʳˀʶʿʷʻʵˀʳ
0032cf09: test   al, al
0032cf0b: je     0x18032cfdf
0032cf11: mov    rcx, qword ptr [rip + 0x39aaec0]         ; [0x3cd7dd8] meta:PlayerSelectionMenu.ʶʴʿʺʽʽʳʳʻʼʲ_TypeInfo
0032cf18: call   0x182f60c00
0032cf1d: xor    edx, edx
0032cf1f: mov    rcx, rax
0032cf22: mov    rsi, rax
0032cf25: call   0x180006d60                              ; System.Configuration.ConfigurationCollectionAttribute$$.ctor
0032cf2a: cmp    byte ptr [rbx + 0x242], 0
0032cf31: mov    rcx, qword ptr [rip + 0x397e7d0]         ; [0x3cab708] meta:ʷʴʼʴʴʷˁʵʺʴˁ[]_TypeInfo
0032cf38: je     0x18032cf63
0032cf3a: mov    edx, 1
0032cf3f: call   0x182f5fc80
0032cf44: mov    rbx, rax
0032cf47: test   rax, rax
0032cf4a: je     0x18032cff2
0032cf50: cmp    dword ptr [rax + 0x18], 0
0032cf54: jbe    0x18032cff8
0032cf5a: mov    dword ptr [rax + 0x20], 8
0032cf61: jmp    0x18032cf82
0032cf63: mov    edx, 3
0032cf68: call   0x182f5fc80
0032cf6d: mov    rdx, qword ptr [rip + 0x39ca6b4]         ; [0x3cf7628] meta:Field$<PrivateImplementationDetails>.7FCC070037949F0E0370384585837FB80BC92ADA1017FC501AA3885C2D4067E5
0032cf74: xor    r8d, r8d
0032cf77: mov    rcx, rax
0032cf7a: mov    rbx, rax
0032cf7d: call   0x181a47220                              ; System.Runtime.CompilerServices.RuntimeHelpers$$InitializeArray
0032cf82: test   rsi, rsi
0032cf85: je     0x18032cff2
0032cf87: lea    rcx, [rsi + 0x10]
0032cf8b: mov    qword ptr [rsi + 0x10], rbx
0032cf8f: mov    rdx, rbx
0032cf92: call   0x182f5fc00
0032cf97: mov    rcx, qword ptr [rip + 0x3991e02]         ; [0x3cbeda0] meta:System.Func<ʷʴʼʴʴʷˁʵʺʴˁ, bool>_TypeInfo
0032cf9e: call   0x182f60c00
0032cfa3: mov    r8, qword ptr [rip + 0x3982996]          ; [0x3caf940] metamethod:Method$PlayerSelectionMenu.ʶʴʿʺʽʽʳʳʻʼʲ.ʲʹˁʶˁʲʵʺʿʾʹ()
0032cfaa: xor    r9d, r9d
0032cfad: mov    rdx, rsi
0032cfb0: mov    rcx, rax
0032cfb3: mov    rbx, rax
0032cfb6: call   0x1808ae210                              ; Rewired.HID.Drivers.NintendoSwitchGamepadDriver.ysRDsSkqcdNGJqUiqnAsjZsZychr$$.ctor
0032cfbb: mov    r8, qword ptr [rip + 0x397d30e]          ; [0x3caa2d0] metamethod:Method$System.Linq.Enumerable.Where<ʷʴʼʴʴʷˁʵʺʴˁ>()
0032cfc2: mov    rdx, rbx
0032cfc5: mov    rcx, rdi
0032cfc8: call   0x18048ee00                              ; System.Linq.Enumerable$$Where<object>
0032cfcd: mov    rdx, qword ptr [rip + 0x3979a3c]         ; [0x3ca6a10] metamethod:Method$System.Linq.Enumerable.ToArray<ʷʴʼʴʴʷˁʵʺʴˁ>()
0032cfd4: mov    rcx, rax
0032cfd7: call   0x18048bf70                              ; System.Linq.Enumerable$$ToArray<Int32Enum>
0032cfdc: mov    rdi, rax
0032cfdf: mov    rsi, qword ptr [rsp + 0x30]
0032cfe4: mov    rax, rdi
0032cfe7: mov    rdi, qword ptr [rsp + 0x38]
0032cfec: add    rsp, 0x20
0032cff0: pop    rbx
0032cff1: ret    
0032cff2: call   0x182f60c50
0032cff7: int3   
0032cff8: call   0x182f60c40
0032cffd: int3   
0032cffe: int3   
