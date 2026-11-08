0032aa80: mov    qword ptr [rsp + 0x18], r8
0032aa85: mov    qword ptr [rsp + 0x10], rdx
0032aa8a: push   rbx
0032aa8b: push   rbp
0032aa8c: push   rsi
0032aa8d: push   rdi
0032aa8e: push   r12
0032aa90: push   r13
0032aa92: push   r14
0032aa94: push   r15
0032aa96: sub    rsp, 0x28
0032aa9a: mov    ebp, dword ptr [rcx]
0032aa9c: mov    r13d, 1
0032aaa2: xor    ebp, dword ptr [r8]
0032aaa5: mov    r14d, dword ptr [rcx + 4]
0032aaa9: mov    r15d, dword ptr [rcx + 8]
0032aaad: xor    r14d, dword ptr [r8 + 4]
0032aab1: xor    r15d, dword ptr [r8 + 8]
0032aab5: mov    r10d, dword ptr [rcx + 0xc]
0032aab9: xor    r10d, dword ptr [r8 + 0xc]
0032aabd: mov    dword ptr [rsp + 0x88], ebp
0032aac4: mov    dword ptr [rsp + 0x70], r13d
0032aac9: nop    dword ptr [rax]
0032aad0: lea    eax, [r13*4]
0032aad8: mov    edi, r14d
0032aadb: movsxd rcx, eax
0032aade: mov    r13d, r10d
0032aae1: mov    eax, r14d
0032aae4: shr    rax, 8
0032aae8: movzx  eax, al
0032aaeb: lea    rcx, [rcx*4]
0032aaf3: mov    qword ptr [rsp + 0x10], rcx
0032aaf8: mov    r12d, r15d
0032aafb: mov    ecx, dword ptr [r9 + rax*4]
0032aaff: mov    dword ptr [rsp + 8], ecx
0032ab03: mov    eax, r15d
0032ab06: shr    rax, 0x10
0032ab0a: movzx  eax, al
0032ab0d: mov    ecx, dword ptr [r9 + rax*4]
0032ab11: mov    dword ptr [rsp + 4], ecx
0032ab15: mov    eax, r10d
0032ab18: shr    rax, 0x18
0032ab1c: mov    ecx, dword ptr [r9 + rax*4]
0032ab20: mov    dword ptr [rsp], ecx
0032ab23: mov    eax, r15d
0032ab26: shr    rax, 8
0032ab2a: movzx  eax, al
0032ab2d: mov    r10d, dword ptr [r9 + rax*4]
0032ab31: mov    eax, r13d
0032ab34: shr    rax, 0x10
0032ab38: movzx  eax, al
0032ab3b: mov    r8d, dword ptr [r9 + rax*4]
0032ab3f: mov    eax, ebp
0032ab41: shr    rax, 0x18
0032ab45: mov    edx, dword ptr [r9 + rax*4]
0032ab49: mov    eax, r13d
0032ab4c: shr    rax, 8
0032ab50: movzx  eax, al
0032ab53: mov    esi, dword ptr [r9 + rax*4]
0032ab57: mov    eax, ebp
0032ab59: shr    rax, 0x10
0032ab5d: movzx  eax, al
0032ab60: mov    ebx, dword ptr [r9 + rax*4]
0032ab64: mov    eax, r14d
0032ab67: shr    rax, 0x18
0032ab6b: mov    r11d, dword ptr [r9 + rax*4]
0032ab6f: mov    eax, ebp
0032ab71: shr    rax, 8
0032ab75: movzx  eax, al
0032ab78: mov    r15d, dword ptr [r9 + rax*4]
0032ab7c: mov    eax, r14d
0032ab7f: shr    rax, 0x10
0032ab83: movzx  eax, al
0032ab86: mov    r14d, dword ptr [r9 + rax*4]
0032ab8a: mov    eax, r12d
0032ab8d: shr    rax, 0x18
0032ab91: mov    ebp, dword ptr [r9 + rax*4]
0032ab95: mov    eax, dword ptr [rsp + 0x70]
0032ab99: lea    eax, [rax*4]
0032aba0: movsxd rcx, eax
0032aba3: mov    eax, r8d
0032aba6: shr    eax, 0x10
0032aba9: shl    r8d, 0x10
0032abad: or     eax, r8d
0032abb0: lea    rcx, [rcx*4 + 0x10]
0032abb8: mov    qword ptr [rsp + 0x18], rcx
0032abbd: mov    ecx, edx
0032abbf: shl    edx, 0x18
0032abc2: shr    ecx, 8
0032abc5: or     edx, ecx
0032abc7: xor    edx, eax
0032abc9: mov    eax, r10d
0032abcc: shr    eax, 0x18
0032abcf: shl    r10d, 8
0032abd3: or     eax, r10d
0032abd6: xor    edx, eax
0032abd8: movzx  eax, dil
0032abdc: mov    rdi, qword ptr [rsp + 0x10]
0032abe1: mov    ecx, dword ptr [r9 + rax*4]
0032abe5: mov    rax, qword ptr [rsp + 0x80]
0032abed: xor    rdx, rcx
0032abf0: mov    eax, dword ptr [rdi + rax + 4]
0032abf4: xor    rdx, rax
0032abf7: mov    qword ptr [rsp + 0x10], rdx
0032abfc: mov    ecx, r11d
0032abff: mov    rax, rdx
0032ac02: shr    rax, 8
0032ac06: movzx  eax, al
0032ac09: shr    ecx, 8
0032ac0c: shl    r11d, 0x18
0032ac10: or     ecx, r11d
0032ac13: mov    r11d, dword ptr [rsp + 4]
0032ac18: mov    r10d, dword ptr [r9 + rax*4]
0032ac1c: mov    eax, ebx
0032ac1e: shr    eax, 0x10
0032ac21: shl    ebx, 0x10
0032ac24: or     eax, ebx
0032ac26: mov    ebx, ebp
0032ac28: xor    ecx, eax
0032ac2a: shr    ebx, 8
0032ac2d: mov    eax, esi
0032ac2f: shl    ebp, 0x18
0032ac32: shr    eax, 0x18
0032ac35: or     ebx, ebp
0032ac37: shl    esi, 8
0032ac3a: or     esi, eax
0032ac3c: movzx  eax, r12b
0032ac40: mov    r12, qword ptr [rsp + 0x80]
0032ac48: xor    esi, ecx
0032ac4a: mov    ecx, dword ptr [r9 + rax*4]
0032ac4e: mov    eax, dword ptr [rdi + r12 + 8]
0032ac53: xor    rsi, rcx
0032ac56: xor    rsi, rax
0032ac59: mov    rax, rsi
0032ac5c: shr    rax, 0x10
0032ac60: movzx  eax, al
0032ac63: mov    r8d, dword ptr [r9 + rax*4]
0032ac67: mov    eax, r14d
0032ac6a: shr    eax, 0x10
0032ac6d: shl    r14d, 0x10
0032ac71: or     eax, r14d
0032ac74: xor    ebx, eax
0032ac76: mov    eax, r15d
0032ac79: shr    eax, 0x18
0032ac7c: shl    r15d, 8
0032ac80: or     eax, r15d
0032ac83: xor    ebx, eax
0032ac85: movzx  eax, r13b
0032ac89: mov    ecx, dword ptr [r9 + rax*4]
0032ac8d: mov    eax, dword ptr [rdi + r12 + 0xc]
0032ac92: xor    rbx, rcx
0032ac95: xor    rbx, rax
0032ac98: mov    rax, rbx
0032ac9b: shr    rax, 0x18
0032ac9f: mov    edx, dword ptr [r9 + rax*4]
0032aca3: mov    ebp, edx
0032aca5: mov    eax, dword ptr [rsp]
0032aca8: mov    ecx, eax
0032acaa: shl    eax, 0x18
0032acad: shr    ecx, 8
0032acb0: or     ecx, eax
0032acb2: shr    ebp, 8
0032acb5: mov    eax, r11d
0032acb8: shl    edx, 0x18
0032acbb: shr    eax, 0x10
0032acbe: or     ebp, edx
0032acc0: shl    r11d, 0x10
0032acc4: or     eax, r11d
0032acc7: mov    r11d, dword ptr [rsp + 8]
0032accc: xor    ecx, eax
0032acce: mov    eax, r11d
0032acd1: shr    eax, 0x18
0032acd4: shl    r11d, 8
0032acd8: or     r11d, eax
0032acdb: xor    r11d, ecx
0032acde: mov    ecx, dword ptr [rsp + 0x88]
0032ace5: movzx  eax, cl
0032ace8: mov    ecx, dword ptr [r9 + rax*4]
0032acec: mov    eax, dword ptr [rdi + r12]
0032acf0: xor    r11, rcx
0032acf3: xor    r11, rax
0032acf6: mov    eax, r8d
0032acf9: shr    eax, 0x10
0032acfc: shl    r8d, 0x10
0032ad00: or     eax, r8d
0032ad03: xor    ebp, eax
0032ad05: mov    rdi, qword ptr [rsp + 0x18]
0032ad0a: mov    eax, r10d
0032ad0d: shl    r10d, 8
0032ad11: shr    eax, 0x18
0032ad14: or     eax, r10d
0032ad17: mov    r10, qword ptr [rsp + 0x10]
0032ad1c: xor    ebp, eax
0032ad1e: movzx  eax, r11b
0032ad22: xor    ebp, dword ptr [r9 + rax*4]
0032ad26: mov    rax, rsi
0032ad29: shr    rax, 8
0032ad2d: movzx  eax, al
0032ad30: xor    ebp, dword ptr [rdi + r12]
0032ad34: mov    dword ptr [rsp + 0x88], ebp
0032ad3b: mov    r8d, dword ptr [r9 + rax*4]
0032ad3f: mov    rax, rbx
0032ad42: shr    rax, 0x10
0032ad46: movzx  eax, al
0032ad49: mov    edx, dword ptr [r9 + rax*4]
0032ad4d: mov    rax, r11
0032ad50: shr    rax, 0x18
0032ad54: mov    ecx, dword ptr [r9 + rax*4]
0032ad58: mov    eax, edx
0032ad5a: shr    eax, 0x10
0032ad5d: mov    r14d, ecx
0032ad60: shr    r14d, 8
0032ad64: shl    ecx, 0x18
0032ad67: or     r14d, ecx
0032ad6a: shl    edx, 0x10
0032ad6d: or     eax, edx
0032ad6f: xor    r14d, eax
0032ad72: mov    eax, r8d
0032ad75: shr    eax, 0x18
0032ad78: shl    r8d, 8
0032ad7c: or     eax, r8d
0032ad7f: xor    r14d, eax
0032ad82: movzx  eax, r10b
0032ad86: xor    r14d, dword ptr [r9 + rax*4]
0032ad8a: mov    rax, rbx
0032ad8d: xor    r14d, dword ptr [rdi + r12 + 4]
0032ad92: shr    rax, 8
0032ad96: movzx  eax, al
0032ad99: mov    r8d, dword ptr [r9 + rax*4]
0032ad9d: mov    rax, r11
0032ada0: shr    rax, 0x10
0032ada4: movzx  eax, al
0032ada7: shr    r11, 8
0032adab: mov    edx, dword ptr [r9 + rax*4]
0032adaf: mov    rax, r10
0032adb2: shr    rax, 0x18
0032adb6: shr    r10, 0x10
0032adba: mov    ecx, dword ptr [r9 + rax*4]
0032adbe: mov    eax, edx
0032adc0: shr    eax, 0x10
0032adc3: mov    r15d, ecx
0032adc6: shr    r15d, 8
0032adca: shl    ecx, 0x18
0032adcd: or     r15d, ecx
0032add0: shl    edx, 0x10
0032add3: or     eax, edx
0032add5: xor    r15d, eax
0032add8: mov    eax, r8d
0032addb: shr    eax, 0x18
0032adde: shl    r8d, 8
0032ade2: or     eax, r8d
0032ade5: xor    r15d, eax
0032ade8: movzx  eax, sil
0032adec: shr    rsi, 0x18
0032adf0: xor    r15d, dword ptr [r9 + rax*4]
0032adf4: xor    r15d, dword ptr [rdi + r12 + 8]
0032adf9: movzx  eax, r11b
0032adfd: mov    edx, dword ptr [r9 + rax*4]
0032ae01: movzx  eax, r10b
0032ae05: mov    ecx, dword ptr [r9 + rax*4]
0032ae09: mov    eax, dword ptr [r9 + rsi*4]
0032ae0d: mov    r10d, eax
0032ae10: shl    eax, 0x18
0032ae13: shr    r10d, 8
0032ae17: or     r10d, eax
0032ae1a: mov    eax, ecx
0032ae1c: mov    r13d, dword ptr [rsp + 0x70]
0032ae21: shr    eax, 0x10
0032ae24: add    r13d, 2
0032ae28: shl    ecx, 0x10
0032ae2b: or     eax, ecx
0032ae2d: mov    dword ptr [rsp + 0x70], r13d
0032ae32: xor    r10d, eax
0032ae35: mov    eax, edx
0032ae37: shr    eax, 0x18
0032ae3a: shl    edx, 8
0032ae3d: or     eax, edx
0032ae3f: xor    r10d, eax
0032ae42: movzx  eax, bl
0032ae45: xor    r10d, dword ptr [r9 + rax*4]
0032ae49: xor    r10d, dword ptr [rdi + r12 + 0xc]
0032ae4e: cmp    r13d, 9
0032ae52: jl     0x18032aad0
0032ae58: mov    ebx, r15d
0032ae5b: lea    eax, [r13*4]
0032ae63: movsxd rcx, eax
0032ae66: mov    eax, r14d
0032ae69: lea    r12, [r12 + rcx*4]
0032ae6d: shr    rax, 8
0032ae71: movzx  eax, al
0032ae74: mov    edi, r14d
0032ae77: mov    esi, r10d
0032ae7a: mov    r11d, ebp
0032ae7d: mov    r8d, dword ptr [r9 + rax*4]
0032ae81: mov    eax, r15d
0032ae84: shr    rax, 0x10
0032ae88: movzx  eax, al
0032ae8b: mov    ecx, dword ptr [r9 + rax*4]
0032ae8f: mov    r15d, ecx
0032ae92: shr    r15d, 0x10
0032ae96: shl    ecx, 0x10
0032ae99: or     r15d, ecx
0032ae9c: mov    eax, r10d
0032ae9f: shr    rax, 0x18
0032aea3: mov    edx, dword ptr [r9 + rax*4]
0032aea7: mov    eax, r8d
0032aeaa: shr    eax, 0x18
0032aead: shl    r8d, 8
0032aeb1: or     eax, r8d
0032aeb4: xor    r15d, eax
0032aeb7: mov    eax, edx
0032aeb9: shr    eax, 8
0032aebc: shl    edx, 0x18
0032aebf: or     eax, edx
0032aec1: xor    r15d, eax
0032aec4: movzx  eax, bpl
0032aec8: xor    r15d, dword ptr [r9 + rax*4]
0032aecc: mov    eax, ebx
0032aece: xor    r15d, dword ptr [r12]
0032aed2: shr    rax, 8
0032aed6: movzx  eax, al
0032aed9: mov    r8d, dword ptr [r9 + rax*4]
0032aedd: mov    eax, r10d
0032aee0: shr    rax, 0x10
0032aee4: movzx  eax, al
0032aee7: mov    edx, dword ptr [r9 + rax*4]
0032aeeb: mov    eax, ebp
0032aeed: shr    rax, 0x18
0032aef1: mov    ecx, dword ptr [r9 + rax*4]
0032aef5: mov    eax, edx
0032aef7: shr    eax, 0x10
0032aefa: mov    r14d, ecx
0032aefd: shr    r14d, 8
0032af01: shl    ecx, 0x18
0032af04: or     r14d, ecx
0032af07: shl    edx, 0x10
0032af0a: or     eax, edx
0032af0c: xor    r14d, eax
0032af0f: mov    eax, r8d
0032af12: shr    eax, 0x18
0032af15: shl    r8d, 8
0032af19: or     eax, r8d
0032af1c: xor    r14d, eax
0032af1f: movzx  eax, dil
0032af23: xor    r14d, dword ptr [r9 + rax*4]
0032af27: xor    r14d, dword ptr [r12 + 4]
0032af2c: mov    eax, r10d
0032af2f: shr    rax, 8
0032af33: movzx  eax, al
0032af36: mov    r8d, dword ptr [r9 + rax*4]
0032af3a: mov    eax, ebp
0032af3c: shr    rax, 0x10
0032af40: movzx  eax, al
0032af43: mov    edx, dword ptr [r9 + rax*4]
0032af47: mov    eax, edi
0032af49: shr    rax, 0x18
0032af4d: mov    ecx, dword ptr [r9 + rax*4]
0032af51: mov    r10d, ecx
0032af54: shr    r10d, 8
0032af58: mov    eax, edx
0032af5a: shl    ecx, 0x18
0032af5d: or     r10d, ecx
0032af60: shr    eax, 0x10
0032af63: shl    edx, 0x10
0032af66: or     eax, edx
0032af68: shr    r11, 8
0032af6c: xor    r10d, eax
0032af6f: shr    rdi, 0x10
0032af73: mov    eax, r8d
0032af76: shl    r8d, 8
0032af7a: shr    eax, 0x18
0032af7d: or     eax, r8d
0032af80: xor    r10d, eax
0032af83: movzx  eax, bl
0032af86: shr    rbx, 0x18
0032af8a: xor    r10d, dword ptr [r9 + rax*4]
0032af8e: xor    r10d, dword ptr [r12 + 8]
0032af93: movzx  eax, r11b
0032af97: mov    ebp, r10d
0032af9a: mov    r11d, r10d
0032af9d: mov    r10d, r14d
0032afa0: mov    edx, dword ptr [r9 + rax*4]
0032afa4: movzx  eax, dil
0032afa8: mov    ecx, dword ptr [r9 + rax*4]
0032afac: mov    eax, dword ptr [r9 + rbx*4]
0032afb0: mov    r8d, eax
0032afb3: mov    rbx, qword ptr [rsp + 0x90]
0032afbb: shl    eax, 0x18
0032afbe: shr    r8d, 8
0032afc2: or     r8d, eax
0032afc5: mov    eax, ecx
0032afc7: shr    eax, 0x10
0032afca: shl    ecx, 0x10
0032afcd: or     eax, ecx
0032afcf: xor    r8d, eax
0032afd2: mov    eax, edx
0032afd4: shr    eax, 0x18
0032afd7: shl    edx, 8
0032afda: or     eax, edx
0032afdc: xor    r8d, eax
0032afdf: movzx  eax, sil
0032afe3: xor    r8d, dword ptr [r9 + rax*4]
0032afe7: lea    eax, [r13*4]
0032afef: xor    r8d, dword ptr [r12 + 0xc]
0032aff4: mov    r12, qword ptr [rsp + 0x78]
0032aff9: movsxd rcx, eax
0032affc: mov    rax, qword ptr [rsp + 0x80]
0032b004: lea    rsi, [rax + rcx*4]
0032b008: mov    edi, r8d
0032b00b: mov    eax, ebp
0032b00d: mov    r9d, r8d
0032b010: shr    rax, 0x10
0032b014: movzx  ecx, al
0032b017: mov    eax, r8d
0032b01a: mov    r8d, r15d
0032b01d: shr    rax, 0x18
0032b021: movzx  edx, byte ptr [rcx + rbx]
0032b025: movzx  ecx, byte ptr [rax + rbx]
0032b029: mov    eax, r14d
0032b02c: shr    rax, 8
0032b030: movzx  eax, al
0032b033: shl    ecx, 8
0032b036: xor    edx, ecx
0032b038: shl    edx, 8
0032b03b: movzx  ecx, byte ptr [rax + rbx]
0032b03f: xor    edx, ecx
0032b041: movzx  eax, r15b
0032b045: shl    edx, 8
0032b048: movzx  ecx, byte ptr [rax + rbx]
0032b04c: mov    eax, r15d
0032b04f: shr    rax, 0x18
0032b053: xor    edx, ecx
0032b055: xor    edx, dword ptr [rsi + 0x10]
0032b058: mov    dword ptr [r12], edx
0032b05c: movzx  edx, byte ptr [rax + rbx]
0032b060: mov    eax, edi
0032b062: shr    rax, 0x10
0032b066: movzx  eax, al
0032b069: shl    edx, 8
0032b06c: movzx  ecx, byte ptr [rax + rbx]
0032b070: mov    eax, ebp
0032b072: xor    edx, ecx
0032b074: shl    edx, 8
0032b077: shr    rax, 8
0032b07b: movzx  eax, al
0032b07e: movzx  ecx, byte ptr [rax + rbx]
0032b082: xor    edx, ecx
0032b084: movzx  eax, r14b
0032b088: shl    edx, 8
0032b08b: shr    r9, 8
0032b08f: shr    r10, 0x10
0032b093: movzx  ecx, byte ptr [rax + rbx]
0032b097: mov    eax, r15d
0032b09a: xor    edx, ecx
0032b09c: shr    rax, 0x10
0032b0a0: xor    edx, dword ptr [rsi + 0x14]
0032b0a3: mov    dword ptr [r12 + 4], edx
0032b0a8: movzx  eax, al
0032b0ab: shr    r11, 0x18
0032b0af: shr    r8, 8
0032b0b3: movzx  edx, byte ptr [rax + rbx]
0032b0b7: mov    eax, r14d
0032b0ba: shr    rax, 0x18
0032b0be: movzx  ecx, byte ptr [rax + rbx]
0032b0c2: shl    ecx, 8
0032b0c5: xor    edx, ecx
0032b0c7: movzx  eax, r9b
0032b0cb: shl    edx, 8
0032b0ce: movzx  ecx, byte ptr [rax + rbx]
0032b0d2: xor    edx, ecx
0032b0d4: movzx  eax, bpl
0032b0d8: shl    edx, 8
0032b0db: movzx  ecx, byte ptr [rax + rbx]
0032b0df: xor    edx, ecx
0032b0e1: movzx  eax, r10b
0032b0e5: xor    edx, dword ptr [rsi + 0x18]
0032b0e8: mov    dword ptr [r12 + 8], edx
0032b0ed: movzx  edx, byte ptr [rax + rbx]
0032b0f1: movzx  eax, byte ptr [r11 + rbx]
0032b0f6: shl    eax, 8
0032b0f9: xor    edx, eax
0032b0fb: movzx  eax, r8b
0032b0ff: shl    edx, 8
0032b102: movzx  ecx, byte ptr [rax + rbx]
0032b106: xor    edx, ecx
0032b108: movzx  eax, dil
0032b10c: shl    edx, 8
0032b10f: movzx  ecx, byte ptr [rax + rbx]
0032b113: xor    edx, ecx
0032b115: xor    edx, dword ptr [rsi + 0x1c]
0032b118: mov    dword ptr [r12 + 0xc], edx
0032b11d: add    rsp, 0x28
0032b121: pop    r15
0032b123: pop    r14
0032b125: pop    r13
0032b127: pop    r12
0032b129: pop    rdi
0032b12a: pop    rsi
0032b12b: pop    rbp
0032b12c: pop    rbx
0032b12d: ret    
0032b12e: int3   
