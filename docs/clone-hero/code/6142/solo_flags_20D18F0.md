# solo_flags_20D18F0 (build 6142, RVA 0x20D18F0)
Static method of class #8265; rdi = the track (#8661), r12 = the play-note list. Called from the note builder at 0x20D3259.
Lines 30 to 39: r13b = 1 when the track's instrument byte (+0x58) is 6 or 9 (drums).
Lines 40 to 53: loop over the solo list at track +0x78 (index ebx); r14 = solo entry (start at +0x10, length at +0x18).
Lines 56 to 90: scan all notes (esi). Lines 72 to 74: a note whose tick (+0x40) is before the start is skipped. Lines 77 to 81: a note for which 0x214BCC0 is true is skipped unless r13b is set (drums); what that test reads was not traced. Lines 82 to 85: a note at or after start+length ends the scan. Lines 86 to 88: remember the first covered note (rdi), count covered notes (r15d).
Lines 91 to 92: no covered notes: jump to line 178. Lines 178 to 194 build a System.Exception with the text "Solo sections should always have atleast 1 note!" and end with a call (line 193) and an int3, the same ending as the exception path in `sp_gain_20F4FA0.asm`. No flag is set and the walk does not go on to the next solo.
Lines 93 to 125: for the first covered note and its chord, or 0x200 (SoloBegin) into the flags at +0x7C (line 120).
Lines 126 to 162: for the note at index esi minus 1 and its chord, or 0x400 (SoloEnd) (line 157). That is the last note the scan walked before it ended. On drums it is the last covered note; off drums, a note skipped by the 0x214BCC0 test can be the one at that index.
Lines 163 to 167: next solo entry.
