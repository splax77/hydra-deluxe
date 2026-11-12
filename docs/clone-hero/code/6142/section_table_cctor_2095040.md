# section_table_cctor_2095040.asm  (build 6142, RVA 0x2095040, the static constructor of the section-name table class; dump.cs line 527009)
Made with tools/ch_mcp/chdis.py with maxbytes 40000 on 2026-11-08, session 77057df2 (task CH-UNK-W2). The listing ends at the method's ret (its last two lines).
Lines 1 to about 10870: first call only: touch every string literal.
From line 10876 to the end: 2,718 calls to 0x182C503E0 (the dictionary Add), each with the key in rdx and the shown name in r8, both string literals.
Read in order, the 2,718 (key, name) pairs equal the 2,718 rows of data/clone-hero-1.1.0.6142-section-names.tsv, row for row. One name holds a single quote, so the listing prints it in double quotes (line 25024, "Breaking 'n' Entering").
