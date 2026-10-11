def simulate(times_pads, lit, dbl_flag_note, T_BREAK=0.75, T_END=0.5):
    """times_pads: list of (t, colour, is_flagged, is_last). Perfect play: every note pressed at its time.
    returns list of (break_time, reason)."""
    mask = set(); mask2 = set(); dbl = False; fin = False
    t2e0 = 0.0; t2d8 = 0.0; c2ec = 0
    events = []
    for (t, col, flagged, last) in times_pads:
        # continuous timeouts before this note
        if mask and fin and t - t2d8 > T_END:
            mask = set(); mask2 = set(); dbl = False; fin = False; c2ec = 0
        if mask and dbl and not fin and t - t2e0 > T_BREAK:
            events.append((t2e0 + T_BREAK, "0.75s no full round"))
            mask = set(); mask2 = set(); dbl = False; c2ec = 0
        if flagged:
            c2ec = 0  # the press handler's qword write at 0x20DECF3 zeroes 0x2E8 and 0x2EC together
            mask2.discard(col)
            if mask and not mask2:
                t2e0 = t; mask2 = set(mask)
            elif not mask:
                # mask empty: and-result 0 -> ZF -> round reset path (mask2=mask=empty)
                t2e0 = t
            if col not in mask:
                dbl = dbl_flag_note
                mask.add(col); mask2.add(col); t2e0 = t
            if last:
                fin = True; t2d8 = t
        else:
            if mask:
                c2ec += 1
                if c2ec >= 3:
                    mask = set(); mask2 = set(); dbl = False; fin = False; c2ec = 0
    return events

