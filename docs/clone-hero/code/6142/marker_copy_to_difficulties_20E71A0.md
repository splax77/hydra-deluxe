# marker_copy_to_difficulties_20E71A0 (build 6142, RVA 0x20E71A0)

A public static extension method of the class at dump.cs line 527397 (TypeDefIndex 8254). It copies one marker onto every difficulty of an instrument. Its arguments are the song (rcx), an instrument byte (dl) and a marker passed by reference (r8).

Lines 18 to 26 loop over the difficulties, up to a count kept in a static field of the class. For each one, lines 27 to 38 build a new marker from the given marker's +0x0, +0x8, +0x14 and +0x18 fields (constructor 0x213E320), with the difficulty as the constructor's fifth argument. The kind is the given marker's byte at +0x18 (line 29), so the copy keeps whatever kind it was given. Lines 49 to 59 find the track with 0x20CFD40 and add the marker with 0x20CEB90.

How it is reached: no way was found, by the same searches as `marker_make_from_event_20E6F00.md`: no vtable slot (dump.cs line 527409), no direct call or jump, no `ScriptMetadataMethod` entry, one pointer in the method pointer table (RVA 0x3E89758), no string literal holding its name. Direct references only (byte scan); see `docs/audit/2026-11-08-clone-hero-unknowns/W3.md`, block NL-39, third pass.
