# The spreadsheet writer keeps to the file formats' numbers

The report export (src/app/xlsx.cpp) writes an .xlsx workbook. Nine numbers in
it are not Hydra's choices. Each is a limit or an id that a published file
format fixes, and the user asked for no value of their own for any of them
("i want to add an export to excel option for the path report and
dmleaderboard report"). A different number would make Excel call the file
damaged.

Excel's sheet holds 1048576 rows and 16384 columns (column XFD), from Excel's
specifications and limits. The writer refuses a sheet past either, so it never
writes a file Excel would call damaged. The
last of the 16384 columns is also where the sheet's default style ends.

In a workbook's style part, the first custom number format takes the id 164.
Excel numbers its own built-in formats below that (ECMA-376 Part 1, 18.8.30).

A text cell holds only characters XML 1.0 allows, its Char production (XML 1.0,
section 2.2): the tab, the line feed, the carriage return, 0x20 to 0xD7FF,
0xE000 to 0xFFFD, and 0x10000 to 0x10FFFF. The writer drops any other
character from a cell, so a control character in a song name cannot make the
file unreadable.
