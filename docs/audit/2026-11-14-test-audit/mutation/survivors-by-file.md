# Survivors of mutation run 20261116-123523, by source file

No tool can say whether a survivor is an equivalent bug, one that changes nothing a user could see.

Mutation run evidence, with four limits: (1) the tool plants its own kinds of bug, not the change an auditor named, so caught nothing is not proof a test is empty; (2) the run went several bugs at once, so a timed-out bug lists its red tests but credits none of them as catches; (3) each verdict comes from one run of its bug; (4) the run held only the C++ tests in hydra_tests, so a test outside it has no evidence here, which is not the same as catching nothing. See docs/audit/2026-11-14-test-audit/mutation/README.md.

A survivor is a bug some test reached and no test caught. These are the 691 lines of report.txt's list "Survivors: bugs reached but caught by no test", copied word for word and grouped by the source file each names, in 74 files. The files are in name order and the bugs in the report's order. This page holds no verdict and ranks nothing. It is an input for the missing-coverage pass that comes after the audit, and T8 judges none of it.

## src/app/analysis.cpp (9)

```
bug 31: src/app/analysis.cpp line 343, `&&` becomes `||`
bug 38: src/app/analysis.cpp line 486, `!` becomes `(bool)`
bug 47: src/app/analysis.cpp line 520, `>` becomes `>=`
bug 48: src/app/analysis.cpp line 531, `<` becomes `<=`
bug 50: src/app/analysis.cpp line 545, `>` becomes `>=`
bug 55: src/app/analysis.cpp line 589, `>` becomes `>=`
bug 56: src/app/analysis.cpp line 589, `&&` becomes `||`
bug 59: src/app/analysis.cpp line 601, `>` becomes `>=`
bug 68: src/app/analysis.cpp line 714, `>` becomes `>=`
```

## src/app/config.cpp (5)

```
bug 82: src/app/config.cpp line 50, `!=` becomes `==`
bug 83: src/app/config.cpp line 55, `==` becomes `!=`
bug 84: src/app/config.cpp line 56, `&&` becomes `||`
bug 85: src/app/config.cpp line 57, `>=` becomes `>`
bug 86: src/app/config.cpp line 59, `==` becomes `!=`
```

## src/app/display_format.cpp (2)

```
bug 118: src/app/display_format.cpp line 56, `<` becomes `<=`
bug 119: src/app/display_format.cpp line 59, `<` becomes `<=`
```

## src/app/dm_report.cpp (1)

```
bug 132: src/app/dm_report.cpp line 133, `>` becomes `>=`
```

## src/app/dynamics_breakdown.cpp (2)

```
bug 166: src/app/dynamics_breakdown.cpp line 31, `<` becomes `<=`
bug 184: src/app/dynamics_breakdown.cpp line 209, `<` becomes `<=`
```

## src/app/fill_report.cpp (2)

```
bug 188: src/app/fill_report.cpp line 277, `>` becomes `>=`
bug 197: src/app/fill_report.cpp line 332, `!` becomes `(bool)`
```

## src/app/html_page.cpp (11)

```
bug 199: src/app/html_page.cpp line 33, `<` becomes `<=`
bug 201: src/app/html_page.cpp line 59, `==` becomes `!=`
bug 202: src/app/html_page.cpp line 59, `&&` becomes `||`
bug 203: src/app/html_page.cpp line 59, `+` becomes `-`
bug 204: src/app/html_page.cpp line 59, `<` becomes `<=`
bug 205: src/app/html_page.cpp line 60, `+` becomes `-`
bug 206: src/app/html_page.cpp line 62, `==` becomes `!=`
bug 207: src/app/html_page.cpp line 62, `&&` becomes `||`
bug 208: src/app/html_page.cpp line 62, `<` becomes `<=`
bug 209: src/app/html_page.cpp line 63, `+` becomes `-`
bug 214: src/app/html_page.cpp line 75, `>=` becomes `>`
```

## src/app/launch.cpp (1)

```
bug 224: src/app/launch.cpp line 100, `!` becomes `(bool)`
```

## src/app/library_query.cpp (16)

```
bug 225: src/app/library_query.cpp line 70, `<` becomes `<=`
bug 229: src/app/library_query.cpp line 81, `<=` becomes `<`
bug 230: src/app/library_query.cpp line 82, `>=` becomes `>`
bug 235: src/app/library_query.cpp line 83, `<=` becomes `<`
bug 245: src/app/library_query.cpp line 173, `&&` becomes `||`
bug 247: src/app/library_query.cpp line 173, `&&` becomes `||`
bug 251: src/app/library_query.cpp line 183, `<` becomes `<=`
bug 256: src/app/library_query.cpp line 206, `&&` becomes `||`
bug 257: src/app/library_query.cpp line 206, `&&` becomes `||`
bug 263: src/app/library_query.cpp line 250, `!` becomes `(bool)`
bug 266: src/app/library_query.cpp line 251, `!` becomes `(bool)`
bug 294: src/app/library_query.cpp line 379, `<` becomes `<=`
bug 300: src/app/library_query.cpp line 387, `>` becomes `>=`
bug 322: src/app/library_query.cpp line 481, `||` becomes `&&`
bug 326: src/app/library_query.cpp line 496, `!=` becomes `==`
bug 327: src/app/library_query.cpp line 496, `<` becomes `<=`
```

## src/app/path_report_view.cpp (1)

```
bug 340: src/app/path_report_view.cpp line 110, `<` becomes `<=`
```

## src/app/path_view.cpp (6)

```
bug 354: src/app/path_view.cpp line 92, `>` becomes `>=`
bug 357: src/app/path_view.cpp line 141, `<` becomes `<=`
bug 383: src/app/path_view.cpp line 375, `>` becomes `>=`
bug 384: src/app/path_view.cpp line 376, `<` becomes `<=`
bug 399: src/app/path_view.cpp line 617, `>=` becomes `>`
bug 406: src/app/path_view.cpp line 634, `||` becomes `&&`
```

## src/app/preview_source.cpp (7)

```
bug 438: src/app/preview_source.cpp line 169, `<` becomes `<=`
bug 441: src/app/preview_source.cpp line 200, `!` becomes `(bool)`
bug 442: src/app/preview_source.cpp line 202, `<=` becomes `<`
bug 443: src/app/preview_source.cpp line 217, `<` becomes `<=`
bug 444: src/app/preview_source.cpp line 222, `<=` becomes `<`
bug 445: src/app/preview_source.cpp line 240, `!` becomes `(bool)`
bug 446: src/app/preview_source.cpp line 251, `!` becomes `(bool)`
```

## src/app/preview_view.cpp (21)

```
bug 461: src/app/preview_view.cpp line 86, `<=` becomes `<`
bug 464: src/app/preview_view.cpp line 181, `<` becomes `<=`
bug 465: src/app/preview_view.cpp line 188, `<` becomes `<=`
bug 467: src/app/preview_view.cpp line 188, `<=` becomes `<`
bug 470: src/app/preview_view.cpp line 189, `<` becomes `<=`
bug 471: src/app/preview_view.cpp line 195, `<` becomes `<=`
bug 473: src/app/preview_view.cpp line 195, `<=` becomes `<`
bug 476: src/app/preview_view.cpp line 196, `<` becomes `<=`
bug 477: src/app/preview_view.cpp line 199, `<` becomes `<=`
bug 491: src/app/preview_view.cpp line 315, `&&` becomes `||`
bug 496: src/app/preview_view.cpp line 394, `<=` becomes `<`
bug 507: src/app/preview_view.cpp line 524, `<` becomes `<=`
bug 508: src/app/preview_view.cpp line 525, `>=` becomes `>`
bug 510: src/app/preview_view.cpp line 532, `<=` becomes `<`
bug 512: src/app/preview_view.cpp line 557, `<` becomes `<=`
bug 513: src/app/preview_view.cpp line 565, `<` becomes `<=`
bug 514: src/app/preview_view.cpp line 565, `>` becomes `>=`
bug 529: src/app/preview_view.cpp line 744, `>=` becomes `>`
bug 532: src/app/preview_view.cpp line 785, `>` becomes `>=`
bug 533: src/app/preview_view.cpp line 787, `>` becomes `>=`
bug 535: src/app/preview_view.cpp line 791, `<` becomes `<=`
```

## src/app/report.cpp (1)

```
bug 558: src/app/report.cpp line 255, `>` becomes `>=`
```

## src/app/report_files.cpp (2)

```
bug 572: src/app/report_files.cpp line 65, `||` becomes `&&`
bug 573: src/app/report_files.cpp line 65, `>` becomes `>=`
```

## src/app/report_view.cpp (1)

```
bug 589: src/app/report_view.cpp line 14, `<=` becomes `<`
```

## src/app/rules_file.cpp (1)

```
bug 605: src/app/rules_file.cpp line 40, `==` becomes `!=`
```

## src/app/saved_figure_fill.cpp (4)

```
bug 625: src/app/saved_figure_fill.cpp line 42, `!` becomes `(bool)`
bug 626: src/app/saved_figure_fill.cpp line 42, `&&` becomes `||`
bug 627: src/app/saved_figure_fill.cpp line 42, `!` becomes `(bool)`
bug 630: src/app/saved_figure_fill.cpp line 63, `!=` becomes `==`
```

## src/app/update_release.cpp (3)

```
bug 669: src/app/update_release.cpp line 30, `!` becomes `(bool)`
bug 681: src/app/update_release.cpp line 67, `||` becomes `&&`
bug 684: src/app/update_release.cpp line 74, `||` becomes `&&`
```

## src/audio/decode.cpp (1)

```
bug 716: src/audio/decode.cpp line 24, `<=` becomes `<`
```

## src/audio/device.cpp (2)

```
bug 722: src/audio/device.cpp line 74, `||` becomes `&&`
bug 724: src/audio/device.cpp line 82, `&&` becomes `||`
```

## src/audio/ma_reader.cpp (47)

```
bug 726: src/audio/ma_reader.cpp line 177, `<=` becomes `<`
bug 727: src/audio/ma_reader.cpp line 182, `!=` becomes `==`
bug 735: src/audio/ma_reader.cpp line 214, `||` becomes `&&`
bug 739: src/audio/ma_reader.cpp line 244, `&&` becomes `||`
bug 741: src/audio/ma_reader.cpp line 245, `!=` becomes `==`
bug 742: src/audio/ma_reader.cpp line 245, `&&` becomes `||`
bug 743: src/audio/ma_reader.cpp line 245, `!=` becomes `==`
bug 745: src/audio/ma_reader.cpp line 264, `&&` becomes `||`
bug 748: src/audio/ma_reader.cpp line 264, `==` becomes `!=`
bug 749: src/audio/ma_reader.cpp line 264, `&&` becomes `||`
bug 751: src/audio/ma_reader.cpp line 265, `&&` becomes `||`
bug 753: src/audio/ma_reader.cpp line 265, `&&` becomes `||`
bug 759: src/audio/ma_reader.cpp line 274, `&&` becomes `||`
bug 761: src/audio/ma_reader.cpp line 274, `&&` becomes `||`
bug 763: src/audio/ma_reader.cpp line 274, `&&` becomes `||`
bug 767: src/audio/ma_reader.cpp line 333, `>` becomes `>=`
bug 768: src/audio/ma_reader.cpp line 336, `<=` becomes `<`
bug 770: src/audio/ma_reader.cpp line 338, `||` becomes `&&`
bug 773: src/audio/ma_reader.cpp line 341, `<` becomes `<=`
bug 774: src/audio/ma_reader.cpp line 341, `||` becomes `&&`
bug 775: src/audio/ma_reader.cpp line 341, `>` becomes `>=`
bug 776: src/audio/ma_reader.cpp line 342, `!=` becomes `==`
bug 778: src/audio/ma_reader.cpp line 342, `>` becomes `>=`
bug 779: src/audio/ma_reader.cpp line 342, `||` becomes `&&`
bug 781: src/audio/ma_reader.cpp line 348, `>` becomes `>=`
bug 785: src/audio/ma_reader.cpp line 392, `<=` becomes `<`
bug 786: src/audio/ma_reader.cpp line 392, `||` becomes `&&`
bug 787: src/audio/ma_reader.cpp line 392, `<=` becomes `<`
bug 791: src/audio/ma_reader.cpp line 416, `<=` becomes `<`
bug 794: src/audio/ma_reader.cpp line 426, `!` becomes `(bool)`
bug 795: src/audio/ma_reader.cpp line 429, `>` becomes `>=`
bug 797: src/audio/ma_reader.cpp line 429, `!` becomes `(bool)`
bug 799: src/audio/ma_reader.cpp line 429, `<=` becomes `<`
bug 800: src/audio/ma_reader.cpp line 435, `>` becomes `>=`
bug 804: src/audio/ma_reader.cpp line 435, `>=` becomes `>`
bug 806: src/audio/ma_reader.cpp line 436, `&&` becomes `||`
bug 807: src/audio/ma_reader.cpp line 454, `<` becomes `<=`
bug 808: src/audio/ma_reader.cpp line 454, `||` becomes `&&`
bug 810: src/audio/ma_reader.cpp line 459, `<` becomes `<=`
bug 812: src/audio/ma_reader.cpp line 466, `>=` becomes `>`
bug 813: src/audio/ma_reader.cpp line 472, `<` becomes `<=`
bug 814: src/audio/ma_reader.cpp line 476, `<` becomes `<=`
bug 815: src/audio/ma_reader.cpp line 479, `>=` becomes `>`
bug 817: src/audio/ma_reader.cpp line 488, `||` becomes `&&`
bug 818: src/audio/ma_reader.cpp line 488, `>=` becomes `>`
bug 820: src/audio/ma_reader.cpp line 501, `||` becomes `&&`
bug 823: src/audio/ma_reader.cpp line 503, `>=` becomes `>`
```

## src/audio/mapped_file.cpp (2)

```
bug 834: src/audio/mapped_file.cpp line 31, `||` becomes `&&`
bug 835: src/audio/mapped_file.cpp line 31, `>` becomes `>=`
```

## src/audio/mixer.cpp (1)

```
bug 847: src/audio/mixer.cpp line 63, `>` becomes `>=`
```

## src/audio/opus_reader.cpp (30)

```
bug 849: src/audio/opus_reader.cpp line 66, `<=` becomes `<`
bug 850: src/audio/opus_reader.cpp line 68, `<` becomes `<=`
bug 853: src/audio/opus_reader.cpp line 108, `<=` becomes `<`
bug 854: src/audio/opus_reader.cpp line 109, `&&` becomes `||`
bug 855: src/audio/opus_reader.cpp line 109, `>=` becomes `>`
bug 856: src/audio/opus_reader.cpp line 110, `!` becomes `(bool)`
bug 857: src/audio/opus_reader.cpp line 111, `<=` becomes `<`
bug 865: src/audio/opus_reader.cpp line 128, `>` becomes `>=`
bug 871: src/audio/opus_reader.cpp line 141, `||` becomes `&&`
bug 872: src/audio/opus_reader.cpp line 141, `!=` becomes `==`
bug 874: src/audio/opus_reader.cpp line 153, `||` becomes `&&`
bug 876: src/audio/opus_reader.cpp line 153, `||` becomes `&&`
bug 883: src/audio/opus_reader.cpp line 172, `!` becomes `(bool)`
bug 884: src/audio/opus_reader.cpp line 172, `&&` becomes `||`
bug 889: src/audio/opus_reader.cpp line 179, `||` becomes `&&`
bug 900: src/audio/opus_reader.cpp line 217, `||` becomes `&&`
bug 901: src/audio/opus_reader.cpp line 217, `>` becomes `>=`
bug 903: src/audio/opus_reader.cpp line 224, `||` becomes `&&`
bug 905: src/audio/opus_reader.cpp line 236, `<=` becomes `<`
bug 909: src/audio/opus_reader.cpp line 249, `!=` becomes `==`
bug 913: src/audio/opus_reader.cpp line 269, `<` becomes `<=`
bug 919: src/audio/opus_reader.cpp line 288, `+` becomes `-`
bug 920: src/audio/opus_reader.cpp line 288, `<=` becomes `<`
bug 923: src/audio/opus_reader.cpp line 328, `<` becomes `<=`
bug 933: src/audio/opus_reader.cpp line 360, `<` becomes `<=`
bug 938: src/audio/opus_reader.cpp line 367, `false` becomes `true`
bug 940: src/audio/opus_reader.cpp line 376, `<` becomes `<=`
bug 941: src/audio/opus_reader.cpp line 379, `false` becomes `true`
bug 942: src/audio/opus_reader.cpp line 385, `>` becomes `>=`
bug 944: src/audio/opus_reader.cpp line 390, `>=` becomes `>`
```

## src/audio/player.cpp (6)

```
bug 945: src/audio/player.cpp line 18, `&&` becomes `||`
bug 946: src/audio/player.cpp line 18, `>` becomes `>=`
bug 948: src/audio/player.cpp line 29, `<=` becomes `<`
bug 949: src/audio/player.cpp line 38, `<=` becomes `<`
bug 951: src/audio/player.cpp line 51, `>` becomes `>=`
bug 955: src/audio/player.cpp line 67, `<` becomes `<=`
```

## src/audio/song_audio.cpp (1)

```
bug 960: src/audio/song_audio.cpp line 73, `<` becomes `<=`
```

## src/audio/stream_mix.cpp (18)

```
bug 961: src/audio/stream_mix.cpp line 24, `>` becomes `>=`
bug 967: src/audio/stream_mix.cpp line 104, `&&` becomes `||`
bug 970: src/audio/stream_mix.cpp line 116, `<` becomes `<=`
bug 974: src/audio/stream_mix.cpp line 120, `>` becomes `>=`
bug 978: src/audio/stream_mix.cpp line 133, `&&` becomes `||`
bug 980: src/audio/stream_mix.cpp line 133, `&&` becomes `||`
bug 982: src/audio/stream_mix.cpp line 133, `<` becomes `<=`
bug 983: src/audio/stream_mix.cpp line 145, `>=` becomes `>`
bug 984: src/audio/stream_mix.cpp line 161, `<=` becomes `<`
bug 988: src/audio/stream_mix.cpp line 188, `<=` becomes `<`
bug 989: src/audio/stream_mix.cpp line 188, `||` becomes `&&`
bug 990: src/audio/stream_mix.cpp line 188, `<=` becomes `<`
bug 992: src/audio/stream_mix.cpp line 192, `||` becomes `&&`
bug 993: src/audio/stream_mix.cpp line 192, `<=` becomes `<`
bug 994: src/audio/stream_mix.cpp line 192, `||` becomes `&&`
bug 995: src/audio/stream_mix.cpp line 192, `<=` becomes `<`
bug 997: src/audio/stream_mix.cpp line 236, `>` becomes `>=`
bug 998: src/audio/stream_mix.cpp line 241, `<=` becomes `<`
```

## src/audio/vorbis_reader.cpp (4)

```
bug 1002: src/audio/vorbis_reader.cpp line 46, `<=` becomes `<`
bug 1003: src/audio/vorbis_reader.cpp line 46, `||` becomes `&&`
bug 1004: src/audio/vorbis_reader.cpp line 46, `<=` becomes `<`
bug 1009: src/audio/vorbis_reader.cpp line 77, `<` becomes `<=`
```

## src/core/audio_sniff.cpp (8)

```
bug 1016: src/core/audio_sniff.cpp line 22, `<` becomes `<=`
bug 1018: src/core/audio_sniff.cpp line 23, `||` becomes `&&`
bug 1019: src/core/audio_sniff.cpp line 23, `<` becomes `<=`
bug 1027: src/core/audio_sniff.cpp line 39, `<` becomes `<=`
bug 1031: src/core/audio_sniff.cpp line 50, `||` becomes `&&`
bug 1032: src/core/audio_sniff.cpp line 50, `<` becomes `<=`
bug 1034: src/core/audio_sniff.cpp line 68, `<` becomes `<=`
bug 1037: src/core/audio_sniff.cpp line 76, `&&` becomes `||`
```

## src/core/model.cpp (6)

```
bug 1095: src/core/model.cpp line 331, `&&` becomes `||`
bug 1097: src/core/model.cpp line 331, `&&` becomes `||`
bug 1099: src/core/model.cpp line 331, `&&` becomes `||`
bug 1101: src/core/model.cpp line 332, `&&` becomes `||`
bug 1126: src/core/model.cpp line 508, `>` becomes `>=`
bug 1157: src/core/model.cpp line 678, `>` becomes `>=`
```

## src/core/note_window.cpp (1)

```
bug 1218: src/core/note_window.cpp line 114, `>=` becomes `>`
```

## src/core/replay.cpp (3)

```
bug 1228: src/core/replay.cpp line 65, `<` becomes `<=`
bug 1254: src/core/replay.cpp line 316, `<` becomes `<=`
bug 1268: src/core/replay.cpp line 421, `||` becomes `&&`
```

## src/core/song.cpp (4)

```
bug 1297: src/core/song.cpp line 151, `+` becomes `-`
bug 1300: src/core/song.cpp line 154, `<=` becomes `<`
bug 1303: src/core/song.cpp line 165, `>` becomes `>=`
bug 1307: src/core/song.cpp line 205, `<` becomes `<=`
```

## src/core/strutil.cpp (4)

```
bug 1374: src/core/strutil.cpp line 73, `&&` becomes `||`
bug 1375: src/core/strutil.cpp line 73, `>` becomes `>=`
bug 1376: src/core/strutil.cpp line 73, `&&` becomes `||`
bug 1395: src/core/strutil.cpp line 116, `<` becomes `<=`
```

## src/core/timing.cpp (6)

```
bug 1427: src/core/timing.cpp line 181, `>` becomes `>=`
bug 1430: src/core/timing.cpp line 197, `<=` becomes `<`
bug 1441: src/core/timing.cpp line 259, `>` becomes `>=`
bug 1443: src/core/timing.cpp line 270, `>` becomes `>=`
bug 1444: src/core/timing.cpp line 272, `>` becomes `>=`
bug 1447: src/core/timing.cpp line 351, `<` becomes `<=`
```

## src/core/winstr.cpp (9)

```
bug 1450: src/core/winstr.cpp line 60, `>=` becomes `>`
bug 1456: src/core/winstr.cpp line 78, `||` becomes `&&`
bug 1457: src/core/winstr.cpp line 78, `>=` becomes `>`
bug 1465: src/core/winstr.cpp line 126, `||` becomes `&&`
bug 1466: src/core/winstr.cpp line 126, `>=` becomes `>`
bug 1485: src/core/winstr.cpp line 232, `==` becomes `!=`
bug 1486: src/core/winstr.cpp line 233, `<` becomes `<=`
bug 1498: src/core/winstr.cpp line 263, `<` becomes `<=`
bug 1499: src/core/winstr.cpp line 275, `!=` becomes `==`
```

## src/image/decode.cpp (1)

```
bug 1508: src/image/decode.cpp line 15, `||` becomes `&&`
```

## src/net/dmbot_client.cpp (7)

```
bug 1512: src/net/dmbot_client.cpp line 21, `||` becomes `&&`
bug 1514: src/net/dmbot_client.cpp line 27, `||` becomes `&&`
bug 1515: src/net/dmbot_client.cpp line 27, `||` becomes `&&`
bug 1518: src/net/dmbot_client.cpp line 33, `||` becomes `&&`
bug 1519: src/net/dmbot_client.cpp line 33, `||` becomes `&&`
bug 1522: src/net/dmbot_client.cpp line 39, `||` becomes `&&`
bug 1532: src/net/dmbot_client.cpp line 103, `||` becomes `&&`
```

## src/net/http.cpp (2)

```
bug 1538: src/net/http.cpp line 121, `!` becomes `(bool)`
bug 1545: src/net/http.cpp line 190, `||` becomes `&&`
```

## src/parse/midi.cpp (12)

```
bug 1588: src/parse/midi.cpp line 77, `<` becomes `<=`
bug 1591: src/parse/midi.cpp line 102, `<` becomes `<=`
bug 1610: src/parse/midi.cpp line 336, `&&` becomes `||`
bug 1612: src/parse/midi.cpp line 341, `false` becomes `true`
bug 1623: src/parse/midi.cpp line 417, `<` becomes `<=`
bug 1630: src/parse/midi.cpp line 432, `false` becomes `true`
bug 1640: src/parse/midi.cpp line 471, `!` becomes `(bool)`
bug 1642: src/parse/midi.cpp line 471, `>` becomes `>=`
bug 1643: src/parse/midi.cpp line 473, `&&` becomes `||`
bug 1646: src/parse/midi.cpp line 473, `>` becomes `>=`
bug 1650: src/parse/midi.cpp line 491, `<` becomes `<=`
bug 1652: src/parse/midi.cpp line 515, `false` becomes `true`
```

## src/parse/note_shuffle.cpp (3)

```
bug 1657: src/parse/note_shuffle.cpp line 81, `&&` becomes `||`
bug 1664: src/parse/note_shuffle.cpp line 135, `<` becomes `<=`
bug 1674: src/parse/note_shuffle.cpp line 157, `||` becomes `&&`
```

## src/parse/sng.cpp (3)

```
bug 1690: src/parse/sng.cpp line 24, `>` becomes `>=`
bug 1698: src/parse/sng.cpp line 64, `<` becomes `<=`
bug 1711: src/parse/sng.cpp line 122, `<=` becomes `<`
```

## src/parse/song.cpp (52)

```
bug 1737: src/parse/song.cpp line 116, `<` becomes `<=`
bug 1738: src/parse/song.cpp line 120, `==` becomes `!=`
bug 1740: src/parse/song.cpp line 120, `>` becomes `>=`
bug 1746: src/parse/song.cpp line 155, `>` becomes `>=`
bug 1756: src/parse/song.cpp line 209, `<` becomes `<=`
bug 1757: src/parse/song.cpp line 239, `<` becomes `<=`
bug 1811: src/parse/song.cpp line 386, `||` becomes `&&`
bug 1813: src/parse/song.cpp line 409, `<` becomes `<=`
bug 1819: src/parse/song.cpp line 417, `<=` becomes `<`
bug 1834: src/parse/song.cpp line 579, `||` becomes `&&`
bug 1838: src/parse/song.cpp line 621, `>=` becomes `>`
bug 1842: src/parse/song.cpp line 621, `>=` becomes `>`
bug 1844: src/parse/song.cpp line 621, `<=` becomes `<`
bug 1846: src/parse/song.cpp line 631, `<` becomes `<=`
bug 1849: src/parse/song.cpp line 632, `||` becomes `&&`
bug 1850: src/parse/song.cpp line 632, `>=` becomes `>`
bug 1857: src/parse/song.cpp line 639, `&&` becomes `||`
bug 1858: src/parse/song.cpp line 639, `&&` becomes `||`
bug 1861: src/parse/song.cpp line 642, `||` becomes `&&`
bug 1862: src/parse/song.cpp line 642, `<` becomes `<=`
bug 1867: src/parse/song.cpp line 789, `<` becomes `<=`
bug 1868: src/parse/song.cpp line 839, `>=` becomes `>`
bug 1870: src/parse/song.cpp line 839, `<=` becomes `<`
bug 1871: src/parse/song.cpp line 840, `>=` becomes `>`
bug 1873: src/parse/song.cpp line 840, `<=` becomes `<`
bug 1874: src/parse/song.cpp line 841, `>=` becomes `>`
bug 1876: src/parse/song.cpp line 841, `<=` becomes `<`
bug 1879: src/parse/song.cpp line 889, `&&` becomes `||`
bug 1880: src/parse/song.cpp line 889, `<=` becomes `<`
bug 1884: src/parse/song.cpp line 901, `&&` becomes `||`
bug 1890: src/parse/song.cpp line 925, `||` becomes `&&`
bug 1909: src/parse/song.cpp line 1078, `>` becomes `>=`
bug 1924: src/parse/song.cpp line 1256, `&&` becomes `||`
bug 1926: src/parse/song.cpp line 1271, `>=` becomes `>`
bug 1929: src/parse/song.cpp line 1271, `&&` becomes `||`
bug 1932: src/parse/song.cpp line 1328, `<` becomes `<=`
bug 1933: src/parse/song.cpp line 1328, `||` becomes `&&`
bug 1935: src/parse/song.cpp line 1328, `||` becomes `&&`
bug 1940: src/parse/song.cpp line 1387, `>` becomes `>=`
bug 1941: src/parse/song.cpp line 1388, `<` becomes `<=`
bug 1972: src/parse/song.cpp line 1463, `>=` becomes `>`
bug 1981: src/parse/song.cpp line 1471, `&&` becomes `||`
bug 1987: src/parse/song.cpp line 1686, `||` becomes `&&`
bug 1988: src/parse/song.cpp line 1686, `||` becomes `&&`
bug 1990: src/parse/song.cpp line 1694, `==` becomes `!=`
bug 1992: src/parse/song.cpp line 1695, `false` becomes `true`
bug 2022: src/parse/song.cpp line 1836, `>=` becomes `>`
bug 2056: src/parse/song.cpp line 1892, `>` becomes `>=`
bug 2078: src/parse/song.cpp line 2092, `&&` becomes `||`
bug 2121: src/parse/song.cpp line 2261, `==` becomes `!=`
bug 2122: src/parse/song.cpp line 2267, `<` becomes `<=`
bug 2124: src/parse/song.cpp line 2267, `>` becomes `>=`
```

## src/parse/srb.cpp (4)

```
bug 2137: src/parse/srb.cpp line 75, `>` becomes `>=`
bug 2145: src/parse/srb.cpp line 138, `<` becomes `<=`
bug 2153: src/parse/srb.cpp line 155, `false` becomes `true`
bug 2175: src/parse/srb.cpp line 189, `<=` becomes `<`
```

## src/parse/text_decode.cpp (23)

```
bug 2176: src/parse/text_decode.cpp line 28, `<` becomes `<=`
bug 2177: src/parse/text_decode.cpp line 30, `<` becomes `<=`
bug 2178: src/parse/text_decode.cpp line 33, `<` becomes `<=`
bug 2181: src/parse/text_decode.cpp line 50, `<=` becomes `<`
bug 2184: src/parse/text_decode.cpp line 70, `>=` becomes `>`
bug 2185: src/parse/text_decode.cpp line 70, `&&` becomes `||`
bug 2186: src/parse/text_decode.cpp line 70, `<=` becomes `<`
bug 2187: src/parse/text_decode.cpp line 70, `&&` becomes `||`
bug 2188: src/parse/text_decode.cpp line 70, `+` becomes `-`
bug 2189: src/parse/text_decode.cpp line 70, `<` becomes `<=`
bug 2190: src/parse/text_decode.cpp line 72, `>=` becomes `>`
bug 2192: src/parse/text_decode.cpp line 72, `<=` becomes `<`
bug 2193: src/parse/text_decode.cpp line 78, `>=` becomes `>`
bug 2195: src/parse/text_decode.cpp line 78, `<=` becomes `<`
bug 2197: src/parse/text_decode.cpp line 90, `<` becomes `<=`
bug 2198: src/parse/text_decode.cpp line 90, `&&` becomes `||`
bug 2202: src/parse/text_decode.cpp line 102, `>=` becomes `>`
bug 2206: src/parse/text_decode.cpp line 104, `>=` becomes `>`
bug 2208: src/parse/text_decode.cpp line 104, `<=` becomes `<`
bug 2210: src/parse/text_decode.cpp line 106, `>=` becomes `>`
bug 2214: src/parse/text_decode.cpp line 108, `>=` becomes `>`
bug 2216: src/parse/text_decode.cpp line 108, `<=` becomes `<`
bug 2223: src/parse/text_decode.cpp line 111, `<` becomes `<=`
```

## src/pathbuilder/pb_coach.cpp (22)

```
bug 2239: src/pathbuilder/pb_coach.cpp line 38, `&&` becomes `||`
bug 2240: src/pathbuilder/pb_coach.cpp line 38, `<` becomes `<=`
bug 2245: src/pathbuilder/pb_coach.cpp line 55, `!` becomes `(bool)`
bug 2246: src/pathbuilder/pb_coach.cpp line 58, `==` becomes `!=`
bug 2249: src/pathbuilder/pb_coach.cpp line 89, `<=` becomes `<`
bug 2261: src/pathbuilder/pb_coach.cpp line 203, `<=` becomes `<`
bug 2263: src/pathbuilder/pb_coach.cpp line 290, `true` becomes `false`
bug 2280: src/pathbuilder/pb_coach.cpp line 351, `||` becomes `&&`
bug 2286: src/pathbuilder/pb_coach.cpp line 365, `||` becomes `&&`
bug 2293: src/pathbuilder/pb_coach.cpp line 375, `!` becomes `(bool)`
bug 2297: src/pathbuilder/pb_coach.cpp line 395, `&&` becomes `||`
bug 2298: src/pathbuilder/pb_coach.cpp line 395, `&&` becomes `||`
bug 2299: src/pathbuilder/pb_coach.cpp line 395, `!` becomes `(bool)`
bug 2302: src/pathbuilder/pb_coach.cpp line 396, `!` becomes `(bool)`
bug 2311: src/pathbuilder/pb_coach.cpp line 428, `==` becomes `!=`
bug 2314: src/pathbuilder/pb_coach.cpp line 468, `||` becomes `&&`
bug 2316: src/pathbuilder/pb_coach.cpp line 469, `!` becomes `(bool)`
bug 2318: src/pathbuilder/pb_coach.cpp line 484, `||` becomes `&&`
bug 2320: src/pathbuilder/pb_coach.cpp line 484, `||` becomes `&&`
bug 2335: src/pathbuilder/pb_coach.cpp line 544, `||` becomes `&&`
bug 2341: src/pathbuilder/pb_coach.cpp line 636, `>=` becomes `>`
bug 2342: src/pathbuilder/pb_coach.cpp line 636, `||` becomes `&&`
```

## src/pathbuilder/pb_files.cpp (3)

```
bug 2347: src/pathbuilder/pb_files.cpp line 63, `||` becomes `&&`
bug 2349: src/pathbuilder/pb_files.cpp line 63, `||` becomes `&&`
bug 2351: src/pathbuilder/pb_files.cpp line 63, `||` becomes `&&`
```

## src/pathbuilder/pb_state.cpp (1)

```
bug 2369: src/pathbuilder/pb_state.cpp line 71, `||` becomes `&&`
```

## src/pathbuilder/pb_text.cpp (2)

```
bug 2385: src/pathbuilder/pb_text.cpp line 61, `>` becomes `>=`
bug 2391: src/pathbuilder/pb_text.cpp line 66, `<` becomes `<=`
```

## src/render/highway_draw.cpp (8)

```
bug 2523: src/render/highway_draw.cpp line 266, `<=` becomes `<`
bug 2525: src/render/highway_draw.cpp line 266, `<` becomes `<=`
bug 2528: src/render/highway_draw.cpp line 272, `<` becomes `<=`
bug 2529: src/render/highway_draw.cpp line 274, `<=` becomes `<`
bug 2537: src/render/highway_draw.cpp line 382, `<=` becomes `<`
bug 2541: src/render/highway_draw.cpp line 402, `<=` becomes `<`
bug 2542: src/render/highway_draw.cpp line 404, `<=` becomes `<`
bug 2544: src/render/highway_draw.cpp line 424, `>=` becomes `>`
```

## src/render/obj_loader.cpp (10)

```
bug 2551: src/render/obj_loader.cpp line 24, `<` becomes `<=`
bug 2556: src/render/obj_loader.cpp line 26, `<` becomes `<=`
bug 2561: src/render/obj_loader.cpp line 27, `>` becomes `>=`
bug 2571: src/render/obj_loader.cpp line 74, `>` becomes `>=`
bug 2573: src/render/obj_loader.cpp line 95, `&&` becomes `||`
bug 2578: src/render/obj_loader.cpp line 110, `<=` becomes `<`
bug 2583: src/render/obj_loader.cpp line 115, `!` becomes `(bool)`
bug 2604: src/render/obj_loader.cpp line 203, `<` becomes `<=`
bug 2605: src/render/obj_loader.cpp line 205, `<` becomes `<=`
bug 2606: src/render/obj_loader.cpp line 207, `<` becomes `<=`
```

## src/render/overlay_layout.cpp (17)

```
bug 2609: src/render/overlay_layout.cpp line 76, `>` becomes `>=`
bug 2611: src/render/overlay_layout.cpp line 81, `>=` becomes `>`
bug 2613: src/render/overlay_layout.cpp line 81, `<=` becomes `<`
bug 2614: src/render/overlay_layout.cpp line 91, `<` becomes `<=`
bug 2616: src/render/overlay_layout.cpp line 144, `>` becomes `>=`
bug 2618: src/render/overlay_layout.cpp line 170, `>` becomes `>=`
bug 2622: src/render/overlay_layout.cpp line 172, `<` becomes `<=`
bug 2628: src/render/overlay_layout.cpp line 174, `+` becomes `-`
bug 2630: src/render/overlay_layout.cpp line 175, `>` becomes `>=`
bug 2633: src/render/overlay_layout.cpp line 188, `<` becomes `<=`
bug 2637: src/render/overlay_layout.cpp line 193, `<` becomes `<=`
bug 2639: src/render/overlay_layout.cpp line 197, `>` becomes `>=`
bug 2643: src/render/overlay_layout.cpp line 197, `<` becomes `<=`
bug 2650: src/render/overlay_layout.cpp line 206, `<` becomes `<=`
bug 2653: src/render/overlay_layout.cpp line 230, `<` becomes `<=`
bug 2656: src/render/overlay_layout.cpp line 253, `<` becomes `<=`
bug 2657: src/render/overlay_layout.cpp line 258, `>` becomes `>=`
```

## src/render/preview_config.cpp (6)

```
bug 2667: src/render/preview_config.cpp line 24, `>=` becomes `>`
bug 2670: src/render/preview_config.cpp line 25, `>=` becomes `>`
bug 2671: src/render/preview_config.cpp line 25, `&&` becomes `||`
bug 2673: src/render/preview_config.cpp line 28, `||` becomes `&&`
bug 2675: src/render/preview_config.cpp line 28, `||` becomes `&&`
bug 2682: src/render/preview_config.cpp line 33, `||` becomes `&&`
```

## src/render/preview_renderer.cpp (3)

```
bug 2690: src/render/preview_renderer.cpp line 280, `==` becomes `!=`
bug 2692: src/render/preview_renderer.cpp line 313, `||` becomes `&&`
bug 2695: src/render/preview_renderer.cpp line 475, `>` becomes `>=`
```

## src/render/track_state.cpp (9)

```
bug 2699: src/render/track_state.cpp line 67, `<=` becomes `<`
bug 2705: src/render/track_state.cpp line 108, `<` becomes `<=`
bug 2709: src/render/track_state.cpp line 109, `&&` becomes `||`
bug 2724: src/render/track_state.cpp line 296, `<` becomes `<=`
bug 2725: src/render/track_state.cpp line 297, `==` becomes `!=`
bug 2726: src/render/track_state.cpp line 297, `||` becomes `&&`
bug 2727: src/render/track_state.cpp line 297, `<` becomes `<=`
bug 2734: src/render/track_state.cpp line 310, `<` becomes `<=`
bug 2735: src/render/track_state.cpp line 311, `==` becomes `!=`
```

## src/search/engine.cpp (67)

```
bug 2790: src/search/engine.cpp line 363, `<` becomes `<=`
bug 2791: src/search/engine.cpp line 364, `>` becomes `>=`
bug 2793: src/search/engine.cpp line 372, `==` becomes `!=`
bug 2798: src/search/engine.cpp line 397, `+` becomes `-`
bug 2799: src/search/engine.cpp line 468, `<` becomes `<=`
bug 2821: src/search/engine.cpp line 554, `>=` becomes `>`
bug 2826: src/search/engine.cpp line 562, `<` becomes `<=`
bug 2829: src/search/engine.cpp line 568, `||` becomes `&&`
bug 2830: src/search/engine.cpp line 568, `<` becomes `<=`
bug 2831: src/search/engine.cpp line 571, `!` becomes `(bool)`
bug 2845: src/search/engine.cpp line 596, `>=` becomes `>`
bug 2847: src/search/engine.cpp line 640, `||` becomes `&&`
bug 2851: src/search/engine.cpp line 670, `>=` becomes `>`
bug 2860: src/search/engine.cpp line 806, `<` becomes `<=`
bug 2864: src/search/engine.cpp line 853, `||` becomes `&&`
bug 2866: src/search/engine.cpp line 855, `<` becomes `<=`
bug 2868: src/search/engine.cpp line 864, `+` becomes `-`
bug 2869: src/search/engine.cpp line 868, `&&` becomes `||`
bug 2870: src/search/engine.cpp line 868, `!` becomes `(bool)`
bug 2871: src/search/engine.cpp line 868, `&&` becomes `||`
bug 2872: src/search/engine.cpp line 869, `!` becomes `(bool)`
bug 2873: src/search/engine.cpp line 872, `>` becomes `>=`
bug 2874: src/search/engine.cpp line 872, `||` becomes `&&`
bug 2875: src/search/engine.cpp line 872, `>` becomes `>=`
bug 2879: src/search/engine.cpp line 915, `>` becomes `>=`
bug 2885: src/search/engine.cpp line 970, `>` becomes `>=`
bug 2892: src/search/engine.cpp line 984, `||` becomes `&&`
bug 2893: src/search/engine.cpp line 984, `>=` becomes `>`
bug 2894: src/search/engine.cpp line 1000, `<` becomes `<=`
bug 2900: src/search/engine.cpp line 1016, `>=` becomes `>`
bug 2901: src/search/engine.cpp line 1016, `&&` becomes `||`
bug 2905: src/search/engine.cpp line 1024, `||` becomes `&&`
bug 2912: src/search/engine.cpp line 1117, `>=` becomes `>`
bug 2914: src/search/engine.cpp line 1164, `<` becomes `<=`
bug 2920: src/search/engine.cpp line 1182, `>=` becomes `>`
bug 2924: src/search/engine.cpp line 1215, `<` becomes `<=`
bug 2936: src/search/engine.cpp line 1248, `<` becomes `<=`
bug 2956: src/search/engine.cpp line 1331, `>=` becomes `>`
bug 2959: src/search/engine.cpp line 1331, `>=` becomes `>`
bug 2960: src/search/engine.cpp line 1332, `>=` becomes `>`
bug 2961: src/search/engine.cpp line 1335, `<` becomes `<=`
bug 2980: src/search/engine.cpp line 1482, `>=` becomes `>`
bug 2981: src/search/engine.cpp line 1482, `||` becomes `&&`
bug 2983: src/search/engine.cpp line 1483, `||` becomes `&&`
bug 2984: src/search/engine.cpp line 1483, `>` becomes `>=`
bug 2987: src/search/engine.cpp line 1488, `||` becomes `&&`
bug 2988: src/search/engine.cpp line 1488, `>=` becomes `>`
bug 2995: src/search/engine.cpp line 1526, `>` becomes `>=`
bug 3004: src/search/engine.cpp line 1584, `<` becomes `<=`
bug 3009: src/search/engine.cpp line 1627, `&&` becomes `||`
bug 3010: src/search/engine.cpp line 1627, `>=` becomes `>`
bug 3014: src/search/engine.cpp line 1652, `<=` becomes `<`
bug 3024: src/search/engine.cpp line 1704, `>=` becomes `>`
bug 3025: src/search/engine.cpp line 1704, `||` becomes `&&`
bug 3029: src/search/engine.cpp line 1734, `<=` becomes `<`
bug 3030: src/search/engine.cpp line 1734, `||` becomes `&&`
bug 3032: src/search/engine.cpp line 1734, `||` becomes `&&`
bug 3034: src/search/engine.cpp line 1752, `>=` becomes `>`
bug 3045: src/search/engine.cpp line 1856, `>` becomes `>=`
bug 3049: src/search/engine.cpp line 1869, `>=` becomes `>`
bug 3053: src/search/engine.cpp line 1883, `<` becomes `<=`
bug 3056: src/search/engine.cpp line 1896, `>=` becomes `>`
bug 3068: src/search/engine.cpp line 1963, `>` becomes `>=`
bug 3072: src/search/engine.cpp line 2031, `>=` becomes `>`
bug 3077: src/search/engine.cpp line 2076, `&&` becomes `||`
bug 3078: src/search/engine.cpp line 2076, `>=` becomes `>`
bug 3079: src/search/engine.cpp line 2076, `&&` becomes `||`
```

## src/search/graph.cpp (4)

```
bug 3087: src/search/graph.cpp line 23, `>` becomes `>=`
bug 3088: src/search/graph.cpp line 36, `<=` becomes `<`
bug 3110: src/search/graph.cpp line 404, `==` becomes `!=`
bug 3112: src/search/graph.cpp line 443, `&&` becomes `||`
```

## src/search/pather.cpp (9)

```
bug 3131: src/search/pather.cpp line 222, `>=` becomes `>`
bug 3132: src/search/pather.cpp line 222, `||` becomes `&&`
bug 3140: src/search/pather.cpp line 319, `<` becomes `<=`
bug 3152: src/search/pather.cpp line 329, `<` becomes `<=`
bug 3158: src/search/pather.cpp line 365, `||` becomes `&&`
bug 3161: src/search/pather.cpp line 451, `||` becomes `&&`
bug 3167: src/search/pather.cpp line 506, `<` becomes `<=`
bug 3168: src/search/pather.cpp line 506, `&&` becomes `||`
bug 3169: src/search/pather.cpp line 506, `<` becomes `<=`
```

## src/search/pathstring_read.cpp (2)

```
bug 3174: src/search/pathstring_read.cpp line 36, `!` becomes `(bool)`
bug 3182: src/search/pathstring_read.cpp line 128, `<` becomes `<=`
```

## src/store/record_store.cpp (14)

```
bug 3251: src/store/record_store.cpp line 618, `>` becomes `>=`
bug 3253: src/store/record_store.cpp line 624, `&&` becomes `||`
bug 3254: src/store/record_store.cpp line 624, `!` becomes `(bool)`
bug 3255: src/store/record_store.cpp line 624, `||` becomes `&&`
bug 3256: src/store/record_store.cpp line 624, `>` becomes `>=`
bug 3264: src/store/record_store.cpp line 690, `!=` becomes `==`
bug 3301: src/store/record_store.cpp line 1316, `==` becomes `!=`
bug 3320: src/store/record_store.cpp line 1571, `!` becomes `(bool)`
bug 3321: src/store/record_store.cpp line 1571, `&&` becomes `||`
bug 3322: src/store/record_store.cpp line 1571, `!=` becomes `==`
bug 3323: src/store/record_store.cpp line 1572, `>` becomes `>=`
bug 3324: src/store/record_store.cpp line 1573, `<` becomes `<=`
bug 3326: src/store/record_store.cpp line 1643, `<` becomes `<=`
bug 3355: src/store/record_store.cpp line 2118, `false` becomes `true`
```

## src/store/upgrade_files.cpp (2)

```
bug 3372: src/store/upgrade_files.cpp line 22, `<` becomes `<=`
bug 3380: src/store/upgrade_files.cpp line 60, `<` becomes `<=`
```

## src/ui/app_shell.cpp (23)

```
bug 3388: src/ui/app_shell.cpp line 68, `||` becomes `&&`
bug 3390: src/ui/app_shell.cpp line 68, `false` becomes `true`
bug 3392: src/ui/app_shell.cpp line 70, `&&` becomes `||`
bug 3397: src/ui/app_shell.cpp line 80, `&&` becomes `||`
bug 3398: src/ui/app_shell.cpp line 81, `>` becomes `>=`
bug 3401: src/ui/app_shell.cpp line 134, `<` becomes `<=`
bug 3402: src/ui/app_shell.cpp line 134, `||` becomes `&&`
bug 3403: src/ui/app_shell.cpp line 134, `<` becomes `<=`
bug 3405: src/ui/app_shell.cpp line 138, `>=` becomes `>`
bug 3407: src/ui/app_shell.cpp line 138, `>=` becomes `>`
bug 3416: src/ui/app_shell.cpp line 162, `>` becomes `>=`
bug 3417: src/ui/app_shell.cpp line 162, `&&` becomes `||`
bug 3418: src/ui/app_shell.cpp line 162, `>` becomes `>=`
bug 3429: src/ui/app_shell.cpp line 233, `<=` becomes `<`
bug 3430: src/ui/app_shell.cpp line 233, `||` becomes `&&`
bug 3431: src/ui/app_shell.cpp line 233, `<=` becomes `<`
bug 3432: src/ui/app_shell.cpp line 235, `>` becomes `>=`
bug 3433: src/ui/app_shell.cpp line 249, `>` becomes `>=`
bug 3434: src/ui/app_shell.cpp line 250, `<` becomes `<=`
bug 3435: src/ui/app_shell.cpp line 251, `>` becomes `>=`
bug 3436: src/ui/app_shell.cpp line 252, `<` becomes `<=`
bug 3439: src/ui/app_shell.cpp line 265, `!` becomes `(bool)`
bug 3455: src/ui/app_shell.cpp line 316, `||` becomes `&&`
```

## src/ui/app_state.cpp (40)

```
bug 3490: src/ui/app_state.cpp line 217, `>` becomes `>=`
bug 3494: src/ui/app_state.cpp line 250, `||` becomes `&&`
bug 3495: src/ui/app_state.cpp line 250, `||` becomes `&&`
bug 3497: src/ui/app_state.cpp line 250, `false` becomes `true`
bug 3510: src/ui/app_state.cpp line 312, `||` becomes `&&`
bug 3512: src/ui/app_state.cpp line 314, `<` becomes `<=`
bug 3522: src/ui/app_state.cpp line 441, `||` becomes `&&`
bug 3524: src/ui/app_state.cpp line 442, `==` becomes `!=`
bug 3536: src/ui/app_state.cpp line 502, `!` becomes `(bool)`
bug 3557: src/ui/app_state.cpp line 574, `!` becomes `(bool)`
bug 3563: src/ui/app_state.cpp line 575, `!` becomes `(bool)`
bug 3564: src/ui/app_state.cpp line 575, `||` becomes `&&`
bug 3566: src/ui/app_state.cpp line 576, `!` becomes `(bool)`
bug 3567: src/ui/app_state.cpp line 576, `||` becomes `&&`
bug 3569: src/ui/app_state.cpp line 577, `!` becomes `(bool)`
bug 3570: src/ui/app_state.cpp line 577, `||` becomes `&&`
bug 3572: src/ui/app_state.cpp line 577, `||` becomes `&&`
bug 3574: src/ui/app_state.cpp line 578, `!` becomes `(bool)`
bug 3575: src/ui/app_state.cpp line 578, `||` becomes `&&`
bug 3577: src/ui/app_state.cpp line 579, `!` becomes `(bool)`
bug 3586: src/ui/app_state.cpp line 685, `>` becomes `>=`
bug 3600: src/ui/app_state.cpp line 774, `!` becomes `(bool)`
bug 3602: src/ui/app_state.cpp line 789, `!` becomes `(bool)`
bug 3604: src/ui/app_state.cpp line 804, `||` becomes `&&`
bug 3606: src/ui/app_state.cpp line 805, `!` becomes `(bool)`
bug 3616: src/ui/app_state.cpp line 841, `||` becomes `&&`
bug 3623: src/ui/app_state.cpp line 854, `||` becomes `&&`
bug 3631: src/ui/app_state.cpp line 912, `!` becomes `(bool)`
bug 3636: src/ui/app_state.cpp line 932, `&&` becomes `||`
bug 3645: src/ui/app_state.cpp line 964, `==` becomes `!=`
bug 3646: src/ui/app_state.cpp line 964, `||` becomes `&&`
bug 3661: src/ui/app_state.cpp line 1034, `&&` becomes `||`
bug 3664: src/ui/app_state.cpp line 1037, `==` becomes `!=`
bug 3668: src/ui/app_state.cpp line 1092, `&&` becomes `||`
bug 3683: src/ui/app_state.cpp line 1200, `&&` becomes `||`
bug 3711: src/ui/app_state.cpp line 1289, `==` becomes `!=`
bug 3713: src/ui/app_state.cpp line 1318, `||` becomes `&&`
bug 3714: src/ui/app_state.cpp line 1318, `||` becomes `&&`
bug 3724: src/ui/app_state.cpp line 1346, `||` becomes `&&`
bug 3727: src/ui/app_state.cpp line 1351, `||` becomes `&&`
```

## src/ui/column_widths.cpp (8)

```
bug 3731: src/ui/column_widths.cpp line 38, `>` becomes `>=`
bug 3749: src/ui/column_widths.cpp line 155, `<=` becomes `<`
bug 3751: src/ui/column_widths.cpp line 155, `<` becomes `<=`
bug 3753: src/ui/column_widths.cpp line 223, `!=` becomes `==`
bug 3755: src/ui/column_widths.cpp line 240, `>=` becomes `>`
bug 3756: src/ui/column_widths.cpp line 244, `>` becomes `>=`
bug 3769: src/ui/column_widths.cpp line 322, `&&` becomes `||`
bug 3772: src/ui/column_widths.cpp line 330, `<` becomes `<=`
```

## src/ui/dm_jobs.cpp (1)

```
bug 3814: src/ui/dm_jobs.cpp line 22, `!` becomes `(bool)`
```

## src/ui/imgui_setup.cpp (1)

```
bug 3848: src/ui/imgui_setup.cpp line 77, `<=` becomes `<`
```

## src/ui/library_jobs.cpp (13)

```
bug 3931: src/ui/library_jobs.cpp line 51, `>` becomes `>=`
bug 3935: src/ui/library_jobs.cpp line 114, `<` becomes `<=`
bug 3946: src/ui/library_jobs.cpp line 346, `&&` becomes `||`
bug 3948: src/ui/library_jobs.cpp line 346, `&&` becomes `||`
bug 3951: src/ui/library_jobs.cpp line 350, `||` becomes `&&`
bug 3953: src/ui/library_jobs.cpp line 356, `||` becomes `&&`
bug 3965: src/ui/library_jobs.cpp line 526, `<` becomes `<=`
bug 3966: src/ui/library_jobs.cpp line 526, `&&` becomes `||`
bug 3967: src/ui/library_jobs.cpp line 526, `<` becomes `<=`
bug 3970: src/ui/library_jobs.cpp line 629, `&&` becomes `||`
bug 3972: src/ui/library_jobs.cpp line 630, `&&` becomes `||`
bug 3975: src/ui/library_jobs.cpp line 631, `&&` becomes `||`
bug 3981: src/ui/library_jobs.cpp line 740, `false` becomes `true`
```

## src/ui/library_model.cpp (14)

```
bug 3991: src/ui/library_model.cpp line 48, `||` becomes `&&`
bug 4017: src/ui/library_model.cpp line 158, `||` becomes `&&`
bug 4018: src/ui/library_model.cpp line 158, `>=` becomes `>`
bug 4020: src/ui/library_model.cpp line 212, `<` becomes `<=`
bug 4024: src/ui/library_model.cpp line 237, `>` becomes `>=`
bug 4027: src/ui/library_model.cpp line 278, `>` becomes `>=`
bug 4042: src/ui/library_model.cpp line 355, `<` becomes `<=`
bug 4043: src/ui/library_model.cpp line 356, `<` becomes `<=`
bug 4044: src/ui/library_model.cpp line 357, `<` becomes `<=`
bug 4048: src/ui/library_model.cpp line 371, `<` becomes `<=`
bug 4049: src/ui/library_model.cpp line 372, `>` becomes `>=`
bug 4051: src/ui/library_model.cpp line 375, `<` becomes `<=`
bug 4052: src/ui/library_model.cpp line 385, `<` becomes `<=`
bug 4053: src/ui/library_model.cpp line 385, `>` becomes `>=`
```

## src/ui/preview_controller.cpp (36)

```
bug 4207: src/ui/preview_controller.cpp line 44, `||` becomes `&&`
bug 4209: src/ui/preview_controller.cpp line 47, `>` becomes `>=`
bug 4210: src/ui/preview_controller.cpp line 56, `!` becomes `(bool)`
bug 4212: src/ui/preview_controller.cpp line 56, `!` becomes `(bool)`
bug 4214: src/ui/preview_controller.cpp line 56, `!` becomes `(bool)`
bug 4216: src/ui/preview_controller.cpp line 56, `!` becomes `(bool)`
bug 4218: src/ui/preview_controller.cpp line 56, `!` becomes `(bool)`
bug 4221: src/ui/preview_controller.cpp line 57, `!` becomes `(bool)`
bug 4223: src/ui/preview_controller.cpp line 61, `||` becomes `&&`
bug 4227: src/ui/preview_controller.cpp line 61, `||` becomes `&&`
bug 4229: src/ui/preview_controller.cpp line 62, `||` becomes `&&`
bug 4231: src/ui/preview_controller.cpp line 62, `||` becomes `&&`
bug 4237: src/ui/preview_controller.cpp line 95, `&&` becomes `||`
bug 4238: src/ui/preview_controller.cpp line 95, `&&` becomes `||`
bug 4240: src/ui/preview_controller.cpp line 95, `&&` becomes `||`
bug 4245: src/ui/preview_controller.cpp line 128, `&&` becomes `||`
bug 4247: src/ui/preview_controller.cpp line 128, `&&` becomes `||`
bug 4248: src/ui/preview_controller.cpp line 128, `&&` becomes `||`
bug 4253: src/ui/preview_controller.cpp line 272, `!=` becomes `==`
bug 4256: src/ui/preview_controller.cpp line 291, `&&` becomes `||`
bug 4257: src/ui/preview_controller.cpp line 291, `!` becomes `(bool)`
bug 4261: src/ui/preview_controller.cpp line 348, `&&` becomes `||`
bug 4262: src/ui/preview_controller.cpp line 348, `!=` becomes `==`
bug 4263: src/ui/preview_controller.cpp line 352, `<=` becomes `<`
bug 4264: src/ui/preview_controller.cpp line 352, `||` becomes `&&`
bug 4265: src/ui/preview_controller.cpp line 352, `<=` becomes `<`
bug 4268: src/ui/preview_controller.cpp line 362, `!=` becomes `==`
bug 4269: src/ui/preview_controller.cpp line 362, `||` becomes `&&`
bug 4270: src/ui/preview_controller.cpp line 362, `!=` becomes `==`
bug 4271: src/ui/preview_controller.cpp line 373, `&&` becomes `||`
bug 4272: src/ui/preview_controller.cpp line 373, `==` becomes `!=`
bug 4278: src/ui/preview_controller.cpp line 404, `||` becomes `&&`
bug 4280: src/ui/preview_controller.cpp line 430, `||` becomes `&&`
bug 4285: src/ui/preview_controller.cpp line 480, `!` becomes `(bool)`
bug 4286: src/ui/preview_controller.cpp line 480, `||` becomes `&&`
bug 4287: src/ui/preview_controller.cpp line 480, `!=` becomes `==`
```

## src/ui/preview_load_job.cpp (3)

```
bug 4315: src/ui/preview_load_job.cpp line 221, `!` becomes `(bool)`
bug 4316: src/ui/preview_load_job.cpp line 223, `!` becomes `(bool)`
bug 4317: src/ui/preview_load_job.cpp line 231, `==` becomes `!=`
```

## src/ui/preview_timeline.cpp (7)

```
bug 4391: src/ui/preview_timeline.cpp line 427, `>` becomes `>=`
bug 4394: src/ui/preview_timeline.cpp line 450, `<` becomes `<=`
bug 4396: src/ui/preview_timeline.cpp line 450, `>` becomes `>=`
bug 4400: src/ui/preview_timeline.cpp line 500, `<` becomes `<=`
bug 4406: src/ui/preview_timeline.cpp line 510, `>` becomes `>=`
bug 4412: src/ui/preview_timeline.cpp line 556, `<` becomes `<=`
bug 4418: src/ui/preview_timeline.cpp line 580, `>` becomes `>=`
```

## src/ui/preview_transport.cpp (7)

```
bug 4445: src/ui/preview_transport.cpp line 60, `<` becomes `<=`
bug 4448: src/ui/preview_transport.cpp line 61, `>` becomes `>=`
bug 4451: src/ui/preview_transport.cpp line 83, `>` becomes `>=`
bug 4452: src/ui/preview_transport.cpp line 83, `&&` becomes `||`
bug 4453: src/ui/preview_transport.cpp line 83, `>` becomes `>=`
bug 4455: src/ui/preview_transport.cpp line 88, `>` becomes `>=`
bug 4459: src/ui/preview_transport.cpp line 94, `>=` becomes `>`
```

## src/ui/timeline_view.cpp (3)

```
bug 4523: src/ui/timeline_view.cpp line 58, `<` becomes `<=`
bug 4525: src/ui/timeline_view.cpp line 58, `>` becomes `>=`
bug 4527: src/ui/timeline_view.cpp line 68, `<=` becomes `<`
```
