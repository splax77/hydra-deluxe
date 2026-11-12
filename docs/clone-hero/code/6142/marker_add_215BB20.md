# marker_add_215BB20 (build 6142, RVA 0x215BB20)

A public instance method of the chart track class (dump.cs line 552027). It adds one marker, passed by reference, to the track. Line 11 reads the marker's kind at +0x18. Line 14 calls 0x2160210, which returns the track's marker list for that kind and makes it if needed. Lines 18 to 51 append the marker to that list (the body of List.Add, compiled in place, with `AddWithResize` at line 35 when the list is full). The kind is the caller's, so the method adds a marker of any kind.

`marker_add_215CC80.asm` and `marker_add_215E420.asm` are the same instructions at other addresses; only the jump targets, the RIP offsets and the start-up flag byte differ. The code at 0x20CEB90, which script.json gives no name, is a fourth copy of the same body; it is the one the helpers in `marker_make_from_event_20E6F00.md` and `marker_copy_to_difficulties_20E71A0.md` and the method 0x20CFB80 call.

How it is reached: no way was found for any of the three named copies. None has a vtable slot in dump.cs (lines 552027, 552055, 552092), none has a direct call or jump, none has a `ScriptMetadataMethod` entry, and each has one pointer, in the method pointer table (RVAs 0x3E910D8, 0x3E91170 and 0x3E91120). No string literal holds their names. Direct references only (byte scan); see `docs/audit/2026-11-08-clone-hero-unknowns/W3.md`, block NL-39, third pass.
