## 19:43:03 engine-baseline-lib-fresh (exit 0, 43.9 s, compilers busy at start: 0)
```nFound 19,436 charts.
[1685/19436]    498,240  Buckner & Garcia - Found Me the Bomb                 3 0 1                               
[1889/19436]    321,155  an Unkindness - Foundation                           E2 1 0 E2                           
[2227/19436]    146,920  Stephen Sanchez - Until I Found You                  0 2                                 
[2380/19436]    429,780  New Found Glory - My Friends Over You                3 3 3 0                             
[2381/19436]    315,330  New Found Glory - Iris                               4 0 6 0                             
[2382/19436]    332,425  New Found Glory - Greatest of All Time               3 0 0 0 3                           
[2383/19436]    329,295  New Found Glory - All Downhill from Here             1 1 3 0                             
[2394/19436]    455,560  My Chemical Romance - The Foundations of Decay       0 2 6                               
[2569/19436]    459,065  Gin Blossoms - Found Out About You                   1 2 3 1                             
[8860/19436]    885,480  sasakure.UK - Lost and Found (Band Remaster)         0- 1 1+ 3 2 0                       
[12322/19436]    106,200  Killitorous - Have You Found Jesus Yet Gump?         (No activations.)                   
[13815/19436]    521,760  Brandon Burkhalter - Newfound Rapture                (No activations.)                   
[17491/19436]    312,590  New Found Glory - Part of Your World                 1- 3+                               
[17584/19436]    407,980  My Chemical Romance - The Foundations of Decay       1 1- 5                              
Analyzed 19,378, skipped 0 already stored, 58 failed in 15.8s.
Failures:
baseline library fresh db wall: 43.6111918 s
```

## 19:44:12 engine-baseline-split (exit 0, 5.4 s, compilers busy at start: 0)
```ncharts 1 parse 0 search 0 store 0 wall 5.1098043
```

## 19:44:18 scan-correctness1 (exit 0, 17.4 s, compilers busy at start: 0)
```n--- A baseline
Scanning C:\Clone Hero
  folders 23160 | charts 19436 | cached 0 | errors 0
  enumerate     :    0.92s
  read+hash     :    3.98s
  discover total:    4.90s
  library write :    0.16s (19436 rows)
--- B all-on
Scanning C:\Clone Hero
  folders 23160 | charts 19436 | cached 0 | errors 0
  enumerate     :    0.19s
  read+hash     :    2.37s
  discover total:    2.56s
  library write :    0.15s (19436 rows)
--- C reusebuf
Scanning C:\Clone Hero
  folders 23160 | charts 19436 | cached 0 | errors 0
  enumerate     :    0.85s
  read+hash     :    2.81s
  discover total:    3.66s
  library write :    0.29s (19436 rows)
--- D exp, all off
Scanning C:\Clone Hero
  folders 23160 | charts 19436 | cached 0 | errors 0
  enumerate     :    1.09s
  read+hash     :    4.03s
  discover total:    5.12s
  library write :    0.16s (19436 rows)
```

## 19:45:21 scan-walk (exit 0, 122.5 s, compilers busy at start: 0)
```nrep 1  A baseline                   enum   0.96  read   3.83  total   4.78  wall   4.86  charts 19436 cached 0
rep 1  findex                       enum   1.11  read   4.17  total   5.28  wall   5.30  charts 19436 cached 0
rep 1  pwalk4                       enum   0.28  read   3.90  total   4.18  wall   4.20  charts 19436 cached 0
rep 1  pwalk8                       enum   0.20  read   3.66  total   3.85  wall   3.87  charts 19436 cached 0
rep 1  pwalk16                      enum   0.16  read   3.84  total   3.99  wall   4.02  charts 19436 cached 0
rep 1  findex+pwalk8                enum   0.29  read   4.73  total   5.02  wall   5.04  charts 19436 cached 0
rep 2  A baseline                   enum   0.98  read   3.56  total   4.54  wall   4.56  charts 19436 cached 0
rep 2  findex                       enum   0.77  read   3.52  total   4.30  wall   4.31  charts 19436 cached 0
rep 2  pwalk4                       enum   0.31  read   3.54  total   3.85  wall   3.87  charts 19436 cached 0
rep 2  pwalk8                       enum   0.18  read   3.46  total   3.64  wall   3.65  charts 19436 cached 0
rep 2  pwalk16                      enum   0.19  read   3.67  total   3.87  wall   3.88  charts 19436 cached 0
rep 2  findex+pwalk8                enum   0.18  read   3.48  total   3.66  wall   3.68  charts 19436 cached 0
rep 3  A baseline                   enum   0.87  read   3.39  total   4.26  wall   4.28  charts 19436 cached 0
rep 3  findex                       enum   0.85  read   3.63  total   4.48  wall   4.49  charts 19436 cached 0
rep 3  pwalk4                       enum   0.27  read   3.63  total   3.90  wall   3.91  charts 19436 cached 0
rep 3  pwalk8                       enum   0.19  read   3.54  total   3.73  wall   3.74  charts 19436 cached 0
rep 3  pwalk16                      enum   0.16  read   3.66  total   3.82  wall   3.84  charts 19436 cached 0
rep 3  findex+pwalk8                enum   0.22  read   3.71  total   3.93  wall   3.95  charts 19436 cached 0
rep 4  A baseline                   enum   0.87  read   3.54  total   4.41  wall   4.42  charts 19436 cached 0
rep 4  findex                       enum   0.79  read   3.50  total   4.29  wall   4.30  charts 19436 cached 0
rep 4  pwalk4                       enum   0.29  read   3.49  total   3.78  wall   3.79  charts 19436 cached 0
rep 4  pwalk8                       enum   0.18  read   3.71  total   3.89  wall   3.90  charts 19436 cached 0
rep 4  pwalk16                      enum   0.16  read   3.44  total   3.61  wall   3.62  charts 19436 cached 0
rep 4  findex+pwalk8                enum   0.19  read   3.46  total   3.65  wall   3.66  charts 19436 cached 0
rep 5  A baseline                   enum   0.87  read   3.41  total   4.28  wall   4.30  charts 19436 cached 0
rep 5  findex                       enum   0.86  read   3.36  total   4.22  wall   4.23  charts 19436 cached 0
rep 5  pwalk4                       enum   0.29  read   3.42  total   3.71  wall   3.72  charts 19436 cached 0
rep 5  pwalk8                       enum   0.19  read   3.38  total   3.57  wall   3.59  charts 19436 cached 0
rep 5  pwalk16                      enum   0.16  read   3.45  total   3.61  wall   3.63  charts 19436 cached 0
rep 5  findex+pwalk8                enum   0.19  read   3.35  total   3.54  wall   3.56  charts 19436 cached 0

MEDIANS over 5 runs
  A baseline                   enum   0.87  read   3.54  total   4.41  wall   4.42
  findex                       enum   0.85  read   3.52  total   4.30  wall   4.31
  pwalk4                       enum   0.29  read   3.54  total   3.85  wall   3.87
  pwalk8                       enum   0.19  read   3.54  total   3.73  wall   3.74
  pwalk16                      enum   0.16  read   3.66  total   3.82  wall   3.84
  findex+pwalk8                enum   0.19  read   3.48  total   3.66  wall   3.68
```

## 19:47:24 prof-fresh-first (exit 0, 34.3 s, compilers busy at start: 0)
```nA fresh exit 0 wall 17.1 s

Found 19,436 charts.
Analyzed 19,378, skipped 0 already stored, 58 failed in 12.4s.
B fresh exit 0 wall 16.8 s
Found 19,436 charts.
Analyzed 19,378, skipped 0 already stored, 58 failed in 12.2s.
```

## 19:48:00 algo-lib-xdom-correctness (exit 0, 35.1 s, compilers busy at start: 0)
```nHalftime: No Expert Pro Drums notes in this chart.
  ...and 38 more.
A baseline wall: 16.6 s
  Halftime: No Expert Pro Drums notes in this chart.
  ...and 38 more.
X dominance wall: 18.3 s
```

## 19:48:36 engine-baseline-libsplit (exit 0, 52.2 s, compilers busy at start: 0)
```ncharts 18811 failed 58 | parse 16.561s | graph 14.321s | analyze x1 18.822s | prepare 0.555s | hash 8d17f958172bd6f4
wall 52.0015365
```

## 19:49:29 prof-real-first (exit 0, 31.3 s, compilers busy at start: 0)
```nA real --redo exit 0 wall 15.5 s

Found 19,436 charts.
Analyzed 19,378, skipped 0 already stored, 58 failed in 14.1s.
B real --redo exit 0 wall 15.2 s
Found 19,436 charts.
Analyzed 19,378, skipped 0 already stored, 58 failed in 13.6s.
```

## 19:50:02 scan-hash (exit 0, 133.4 s, compilers busy at start: 0)
```nrep 1  A baseline                   enum   0.78  read   3.51  total   4.29  wall   4.34  charts 19436 cached 0
rep 1  reusebuf                     enum   1.01  read   2.71  total   3.73  wall   3.75  charts 19436 cached 0
rep 1  hashio 1MB                   enum   0.85  read   2.77  total   3.62  wall   3.64  charts 19436 cached 0
rep 1  hashio 256KB                 enum   1.01  read   2.66  total   3.66  wall   3.69  charts 19436 cached 0
rep 1  workers12                    enum   0.86  read   3.23  total   4.09  wall   4.10  charts 19436 cached 0
rep 1  workers16                    enum   0.85  read   2.99  total   3.84  wall   3.85  charts 19436 cached 0
rep 1  hashio+workers16             enum   0.85  read   2.51  total   3.37  wall   3.38  charts 19436 cached 0
rep 2  A baseline                   enum   0.87  read   3.55  total   4.42  wall   4.44  charts 19436 cached 0
rep 2  reusebuf                     enum   0.84  read   2.68  total   3.52  wall   3.54  charts 19436 cached 0
rep 2  hashio 1MB                   enum   0.88  read   2.76  total   3.64  wall   3.66  charts 19436 cached 0
rep 2  hashio 256KB                 enum   0.91  read   2.62  total   3.53  wall   3.55  charts 19436 cached 0
rep 2  workers12                    enum   0.82  read   3.08  total   3.89  wall   3.90  charts 19436 cached 0
rep 2  workers16                    enum   0.77  read   3.19  total   3.97  wall   3.98  charts 19436 cached 0
rep 2  hashio+workers16             enum   0.95  read   2.28  total   3.22  wall   3.24  charts 19436 cached 0
rep 3  A baseline                   enum   0.89  read   3.72  total   4.62  wall   4.63  charts 19436 cached 0
rep 3  reusebuf                     enum   0.80  read   2.62  total   3.42  wall   3.44  charts 19436 cached 0
rep 3  hashio 1MB                   enum   0.98  read   2.63  total   3.61  wall   3.63  charts 19436 cached 0
rep 3  hashio 256KB                 enum   0.83  read   2.68  total   3.51  wall   3.53  charts 19436 cached 0
rep 3  workers12                    enum   0.80  read   3.16  total   3.95  wall   3.97  charts 19436 cached 0
rep 3  workers16                    enum   0.80  read   3.36  total   4.16  wall   4.17  charts 19436 cached 0
rep 3  hashio+workers16             enum   0.84  read   2.32  total   3.16  wall   3.18  charts 19436 cached 0
rep 4  A baseline                   enum   0.96  read   3.45  total   4.41  wall   4.42  charts 19436 cached 0
rep 4  reusebuf                     enum   0.76  read   2.65  total   3.41  wall   3.43  charts 19436 cached 0
rep 4  hashio 1MB                   enum   0.85  read   2.59  total   3.44  wall   3.45  charts 19436 cached 0
rep 4  hashio 256KB                 enum   0.76  read   2.65  total   3.41  wall   3.42  charts 19436 cached 0
rep 4  workers12                    enum   0.79  read   3.12  total   3.91  wall   3.93  charts 19436 cached 0
rep 4  workers16                    enum   0.79  read   2.97  total   3.76  wall   3.77  charts 19436 cached 0
rep 4  hashio+workers16             enum   0.78  read   2.29  total   3.08  wall   3.09  charts 19436 cached 0
rep 5  A baseline                   enum   0.86  read   3.65  total   4.52  wall   4.53  charts 19436 cached 0
rep 5  reusebuf                     enum   0.83  read   3.69  total   4.51  wall   4.59  charts 19436 cached 0
rep 5  hashio 1MB                   enum   0.94  read   2.60  total   3.54  wall   3.79  charts 19436 cached 0
rep 5  hashio 256KB                 enum   0.79  read   2.70  total   3.49  wall   3.51  charts 19436 cached 0
rep 5  workers12                    enum   0.81  read   3.34  total   4.15  wall   4.16  charts 19436 cached 0
rep 5  workers16                    enum   0.87  read   3.25  total   4.12  wall   4.14  charts 19436 cached 0
rep 5  hashio+workers16             enum   0.95  read   2.29  total   3.24  wall   3.25  charts 19436 cached 0

MEDIANS over 5 runs
  A baseline                   enum   0.87  read   3.55  total   4.42  wall   4.44
  reusebuf                     enum   0.83  read   2.68  total   3.52  wall   3.54
  hashio 1MB                   enum   0.88  read   2.63  total   3.61  wall   3.64
  hashio 256KB                 enum   0.83  read   2.66  total   3.51  wall   3.53
  workers12                    enum   0.81  read   3.16  total   3.95  wall   3.97
  workers16                    enum   0.80  read   3.19  total   3.97  wall   3.98
  hashio+workers16             enum   0.85  read   2.29  total   3.22  wall   3.24
```

## 19:52:21 prof-overhead-ab (exit 0, 101.4 s, compilers busy at start: 0)
```nA run 1 fresh exit 0 wall 16.95 s
B run 1 fresh exit 0 wall 17.04 s
A run 2 fresh exit 0 wall 16.81 s
B run 2 fresh exit 0 wall 16.81 s
A run 3 fresh exit 0 wall 16.72 s
B run 3 fresh exit 0 wall 16.75 s
A median 16.81 s
B median 16.81 s
```

## 19:54:04 write-first (exit 0, 35 s, compilers busy at start: 0)
```nA1 exit=0 wall_s=16.9 analyzed=19,378 failed=58 analysis_s=15.4 peakWS_MB=514 peakCommit_MB=639 
B0 exit=0 wall_s=17.6 analyzed=19,378 failed=58 analysis_s=16.2 peakWS_MB=503 peakCommit_MB=634 PERF wall_ms=16135 workers=8 charts=18869 worker_busy_ms=49971 util=0.387 consume_ms=15248 save_ms=12766 commit_ms=0 commits=0 save_per_chart_ms=0.677 consume_per_chart_ms=0.808 last_start_ms=15908 nth_last_end_ms=15909 group=0 cache=0 order=
```

## 19:54:41 engine-probe-lib (exit 0, 62 s, compilers busy at start: 0)
```ncharts 18811 failed 58 | parse 15.084s | graph 20.112s | analyze x1 24.413s | prepare 0.514s | hash 8d17f958172bd6f4
graph destroy 0.242s | probes: [0]39.887 [1]1.220 [2]13.169 [3]9.369 [4]3.552 [5]1.678 [6]3.467 [7]0.000 [8]0.760 [9]0.000 [10]0.987 [11]2.802 [12]0.354 [13]0.768 [14]0.000 [15]0.000
```

## 19:55:44 prof-coldhash (exit 0, 109.6 s, compilers busy at start: 0)
```nbuf           threads  8: 19436 files 7.83 GB in 3.00 s = 2.61 GB/s; md5 mismatches/errors 0
nobuf         threads  8: 19436 files 7.83 GB in 4.09 s = 1.92 GB/s; md5 mismatches/errors 0
nobuf         threads 16: 19436 files 7.83 GB in 8.30 s = 0.94 GB/s; md5 mismatches/errors 0
buf           threads 16: 19436 files 7.83 GB in 3.58 s = 2.19 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads  8: 19436 files 7.83 GB in 2.90 s = 2.70 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads 16: 19436 files 7.83 GB in 7.01 s = 1.12 GB/s; md5 mismatches/errors 0
buf           threads  8: 19436 files 7.83 GB in 3.21 s = 2.44 GB/s; md5 mismatches/errors 0
nobuf         threads  8: 19436 files 7.83 GB in 3.94 s = 1.99 GB/s; md5 mismatches/errors 0
nobuf         threads 16: 19436 files 7.83 GB in 7.87 s = 1.00 GB/s; md5 mismatches/errors 0
buf           threads 16: 19436 files 7.83 GB in 3.41 s = 2.30 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads  8: 19436 files 7.83 GB in 3.11 s = 2.52 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads 16: 19436 files 7.83 GB in 8.04 s = 0.97 GB/s; md5 mismatches/errors 0
buf           threads  8: 19436 files 7.83 GB in 5.15 s = 1.52 GB/s; md5 mismatches/errors 0
nobuf         threads  8: 19436 files 7.83 GB in 10.60 s = 0.74 GB/s; md5 mismatches/errors 0
nobuf         threads 16: 19436 files 7.83 GB in 12.11 s = 0.65 GB/s; md5 mismatches/errors 0
buf           threads 16: 19436 files 7.83 GB in 4.05 s = 1.93 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads  8: 19436 files 7.83 GB in 6.64 s = 1.18 GB/s; md5 mismatches/errors 0
nobuf-nohash  threads 16: 19436 files 7.83 GB in 10.31 s = 0.76 GB/s; md5 mismatches/errors 0
```

## 19:57:37 algo-lib-shadow-passes (exit 0, 47.3 s, compilers busy at start: 0)
```nAnalyzed 19,378, skipped 0 already stored, 58 failed in 41.4s.
log run wall: 46.9 s
```

## 19:58:28 write-cache-group (exit 0, 194.1 s, compilers busy at start: 0)
```nA_1 exit=0 wall_s=19.4 analyzed=19,378 failed=58 analysis_s=17.9 peakWS_MB=514 peakCommit_MB=650 
C_1 exit=0 wall_s=17.2 analyzed=19,378 failed=58 analysis_s=15.5 peakWS_MB=506 peakCommit_MB=639 PERF wall_ms=15537 workers=8 charts=18869 worker_busy_ms=53466 util=0.430 consume_ms=14530 save_ms=11775 commit_ms=0 commits=0 save_per_chart_ms=0.624 consume_per_chart_ms=0.770 last_start_ms=15247 nth_last_end_ms=15248 group=0 cache=1 order=
CG16_1 exit=0 wall_s=16.3 analyzed=19,378 failed=58 analysis_s=14.9 peakWS_MB=463 peakCommit_MB=598 PERF wall_ms=14855 workers=8 charts=18869 worker_busy_ms=56176 util=0.473 consume_ms=13994 save_ms=5018 commit_ms=6711 commits=1360 save_per_chart_ms=0.622 consume_per_chart_ms=0.742 last_start_ms=14561 nth_last_end_ms=14561 group=16 cache=1 order=
CG64_1 exit=0 wall_s=20.7 analyzed=19,378 failed=58 analysis_s=19.2 peakWS_MB=515 peakCommit_MB=651 PERF wall_ms=19167 workers=8 charts=18869 worker_busy_ms=60616 util=0.395 consume_ms=18162 save_ms=7932 commit_ms=7961 commits=509 save_per_chart_ms=0.842 consume_per_chart_ms=0.963 last_start_ms=18935 nth_last_end_ms=18936 group=64 cache=1 order=
A_2 exit=0 wall_s=17.1 analyzed=19,378 failed=58 analysis_s=15.6 peakWS_MB=506 peakCommit_MB=637 
C_2 exit=0 wall_s=15.2 analyzed=19,378 failed=58 analysis_s=13.9 peakWS_MB=485 peakCommit_MB=625 PERF wall_ms=13921 workers=8 charts=18869 worker_busy_ms=50881 util=0.457 consume_ms=12967 save_ms=10616 commit_ms=0 commits=0 save_per_chart_ms=0.563 consume_per_chart_ms=0.687 last_start_ms=13648 nth_last_end_ms=13650 group=0 cache=1 order=
CG16_2 exit=0 wall_s=13.9 analyzed=19,378 failed=58 analysis_s=12.6 peakWS_MB=513 peakCommit_MB=647 PERF wall_ms=12644 workers=8 charts=18869 worker_busy_ms=51550 util=0.510 consume_ms=11942 save_ms=4419 commit_ms=5717 commits=1399 save_per_chart_ms=0.537 consume_per_chart_ms=0.633 last_start_ms=12413 nth_last_end_ms=12414 group=16 cache=1 order=
CG64_2 exit=0 wall_s=15.5 analyzed=19,378 failed=58 analysis_s=14.2 peakWS_MB=501 peakCommit_MB=638 PERF wall_ms=14201 workers=8 charts=18869 worker_busy_ms=58982 util=0.519 consume_ms=13336 save_ms=5339 commit_ms=6275 commits=808 save_per_chart_ms=0.616 consume_per_chart_ms=0.707 last_start_ms=13965 nth_last_end_ms=13965 group=64 cache=1 order=
A_3 exit=0 wall_s=16.3 analyzed=19,378 failed=58 analysis_s=15.0 peakWS_MB=503 peakCommit_MB=631 
C_3 exit=0 wall_s=14.1 analyzed=19,378 failed=58 analysis_s=12.8 peakWS_MB=509 peakCommit_MB=639 PERF wall_ms=12780 workers=8 charts=18869 worker_busy_ms=49426 util=0.483 consume_ms=11878 save_ms=9705 commit_ms=0 commits=0 save_per_chart_ms=0.514 consume_per_chart_ms=0.629 last_start_ms=12500 nth_last_end_ms=12502 group=0 cache=1 order=
CG16_3 exit=0 wall_s=13.5 analyzed=19,378 failed=58 analysis_s=12.2 peakWS_MB=506 peakCommit_MB=641 PERF wall_ms=12160 workers=8 charts=18869 worker_busy_ms=51130 util=0.526 consume_ms=11443 save_ms=4270 commit_ms=5566 commits=1424 save_per_chart_ms=0.521 consume_per_chart_ms=0.606 last_start_ms=11916 nth_last_end_ms=11918 group=16 cache=1 order=
CG64_3 exit=0 wall_s=13.0 analyzed=19,378 failed=58 analysis_s=11.6 peakWS_MB=507 peakCommit_MB=643 PERF wall_ms=11606 workers=8 charts=18869 worker_busy_ms=50687 util=0.546 consume_ms=10933 save_ms=4336 commit_ms=5175 commits=718 save_per_chart_ms=0.504 consume_per_chart_ms=0.579 last_start_ms=11377 nth_last_end_ms=11378 group=64 cache=1 order=
```

## 20:01:44 prof-vsdiag (exit 0, 1.6 s, compilers busy at start: 0)
```nMicrosoft (R) VS Standard Collector


Unhandled Exception: System.IO.FileLoadException: Could not load file or assembly 'Newtonsoft.Json, Version=13.0.0.0, Culture=neutral, PublicKeyToken=30ad4fe6b2a6aeed' or one of its dependencies. The located assembly's manifest definition does not match the assembly reference. (Exception from HRESULT: 0x80131040)
   at Microsoft.DiagnosticsHub.StandardCollector.RuntimeOptions.LoadConfigFromPath(List`1 agents, String configFile)
   at Microsoft.DiagnosticsHub.StandardCollector.RuntimeOptions.LoadConfigWithName(String configName)
   at Microsoft.DiagnosticsHub.StandardCollector.CommandLine.Parser.<>c.<.ctor>b__23_1(IDictionary`2 flags, IList`1 args, RuntimeOptions options)
   at Microsoft.DiagnosticsHub.StandardCollector.CommandLine.ParserBase`1.ParseArgs(String[] arguments)
   at Microsoft.DiagnosticsHub.StandardCollector.Program.<Main>d__0.MoveNext()
--- End of stack trace from previous location where exception was thrown ---
   at System.Runtime.ExceptionServices.ExceptionDispatchInfo.Throw()
   at System.Runtime.CompilerServices.TaskAwaiter.HandleNonSuccessAndDebuggerNotification(Task task)
   at Microsoft.DiagnosticsHub.StandardCollector.Program.<Main>(String[] args)

start exit -532462766

Microsoft (R) VS Standard Collector


Session 22 does not exist.

stop exit 0 after 0.6 s

Microsoft (R) VS Standard Collector


Unhandled Exception: System.IO.FileLoadException: Could not load file or assembly 'Newtonsoft.Json, Version=13.0.0.0, Culture=neutral, PublicKeyToken=30ad4fe6b2a6aeed' or one of its dependencies. The located assembly's manifest definition does not match the assembly reference. (Exception from HRESULT: 0x80131040)
   at Microsoft.DiagnosticsHub.StandardCollector.RuntimeOptions.LoadConfigFromPath(List`1 agents, String configFile)
   at Microsoft.DiagnosticsHub.StandardCollector.RuntimeOptions.LoadConfigWithName(String configName)
   at Microsoft.DiagnosticsHub.StandardCollector.CommandLine.Parser.<>c.<.ctor>b__23_1(IDictionary`2 flags, IList`1 args, RuntimeOptions options)
   at Microsoft.DiagnosticsHub.StandardCollector.CommandLine.ParserBase`1.ParseArgs(String[] arguments)
   at Microsoft.DiagnosticsHub.StandardCollector.Program.<Main>d__0.MoveNext()
--- End of stack trace from previous location where exception was thrown ---
   at System.Runtime.ExceptionServices.ExceptionDispatchInfo.Throw()
   at System.Runtime.CompilerServices.TaskAwaiter.HandleNonSuccessAndDebuggerNotification(Task task)
   at Microsoft.DiagnosticsHub.StandardCollector.Program.<Main>(String[] args)

start exit -532462766

Microsoft (R) VS Standard Collector


Session 23 does not exist.

stop exit 0 after 0.6 s
```

## 20:01:49 scan-combo (exit 0, 111 s, compilers busy at start: 0)
```nrep 1  A baseline                   enum   0.79  read   3.25  total   4.05  wall   4.09  charts 19436 cached 0
rep 1  reusebuf                     enum   0.81  read   2.59  total   3.40  wall   3.45  charts 19436 cached 0
rep 1  bigfirst                     enum   0.78  read   2.68  total   3.46  wall   3.48  charts 19436 cached 0
rep 1  reusebuf+bigfirst            enum   0.87  read   1.78  total   2.65  wall   2.67  charts 19436 cached 0
rep 1  reusebuf+bigfirst+w16        enum   0.82  read   1.85  total   2.67  wall   2.68  charts 19436 cached 0
rep 1  pwalk8+reusebuf+bigfirst     enum   0.17  read   1.89  total   2.06  wall   2.08  charts 19436 cached 0
rep 1  pwalk8+hashio+bigfirst+w16   enum   0.17  read   1.99  total   2.16  wall   2.18  charts 19436 cached 0
rep 2  A baseline                   enum   0.85  read   3.36  total   4.20  wall   4.22  charts 19436 cached 0
rep 2  reusebuf                     enum   0.93  read   2.65  total   3.58  wall   3.60  charts 19436 cached 0
rep 2  bigfirst                     enum   0.84  read   2.73  total   3.57  wall   3.59  charts 19436 cached 0
rep 2  reusebuf+bigfirst            enum   0.89  read   1.88  total   2.76  wall   2.78  charts 19436 cached 0
rep 2  reusebuf+bigfirst+w16        enum   1.07  read   2.07  total   3.13  wall   3.15  charts 19436 cached 0
rep 2  pwalk8+reusebuf+bigfirst     enum   0.19  read   1.98  total   2.18  wall   2.19  charts 19436 cached 0
rep 2  pwalk8+hashio+bigfirst+w16   enum   0.25  read   2.14  total   2.39  wall   2.41  charts 19436 cached 0
rep 3  A baseline                   enum   0.79  read   3.42  total   4.22  wall   4.23  charts 19436 cached 0
rep 3  reusebuf                     enum   0.88  read   2.80  total   3.68  wall   3.69  charts 19436 cached 0
rep 3  bigfirst                     enum   0.98  read   2.69  total   3.67  wall   3.68  charts 19436 cached 0
rep 3  reusebuf+bigfirst            enum   0.87  read   1.96  total   2.83  wall   2.84  charts 19436 cached 0
rep 3  reusebuf+bigfirst+w16        enum   0.80  read   2.13  total   2.93  wall   2.95  charts 19436 cached 0
rep 3  pwalk8+reusebuf+bigfirst     enum   0.17  read   1.81  total   1.98  wall   2.00  charts 19436 cached 0
rep 3  pwalk8+hashio+bigfirst+w16   enum   0.19  read   2.21  total   2.40  wall   2.42  charts 19436 cached 0
rep 4  A baseline                   enum   0.90  read   3.53  total   4.43  wall   4.45  charts 19436 cached 0
rep 4  reusebuf                     enum   1.04  read   2.79  total   3.83  wall   3.85  charts 19436 cached 0
rep 4  bigfirst                     enum   1.89  read   3.09  total   4.98  wall   4.99  charts 19436 cached 0
rep 4  reusebuf+bigfirst            enum   0.86  read   2.12  total   2.98  wall   3.01  charts 19436 cached 0
rep 4  reusebuf+bigfirst+w16        enum   0.81  read   2.09  total   2.90  wall   2.92  charts 19436 cached 0
rep 4  pwalk8+reusebuf+bigfirst     enum   0.19  read   1.87  total   2.06  wall   2.08  charts 19436 cached 0
rep 4  pwalk8+hashio+bigfirst+w16   enum   0.21  read   2.00  total   2.21  wall   2.23  charts 19436 cached 0
rep 5  A baseline                   enum   0.81  read   3.40  total   4.22  wall   4.23  charts 19436 cached 0
rep 5  reusebuf                     enum   0.77  read   2.77  total   3.53  wall   3.55  charts 19436 cached 0
rep 5  bigfirst                     enum   0.79  read   2.52  total   3.31  wall   3.32  charts 19436 cached 0
rep 5  reusebuf+bigfirst            enum   0.92  read   1.79  total   2.71  wall   2.73  charts 19436 cached 0
rep 5  reusebuf+bigfirst+w16        enum   0.90  read   2.03  total   2.93  wall   2.95  charts 19436 cached 0
rep 5  pwalk8+reusebuf+bigfirst     enum   0.36  read   2.89  total   3.25  wall   3.33  charts 19436 cached 0
rep 5  pwalk8+hashio+bigfirst+w16   enum   0.26  read   2.36  total   2.62  wall   2.66  charts 19436 cached 0

MEDIANS over 5 runs
  A baseline                   enum   0.81  read   3.40  total   4.22  wall   4.23
  reusebuf                     enum   0.88  read   2.77  total   3.58  wall   3.60
  bigfirst                     enum   0.84  read   2.69  total   3.57  wall   3.59
  reusebuf+bigfirst            enum   0.87  read   1.88  total   2.76  wall   2.78
  reusebuf+bigfirst+w16        enum   0.82  read   2.07  total   2.93  wall   2.95
  pwalk8+reusebuf+bigfirst     enum   0.19  read   1.89  total   2.06  wall   2.08
  pwalk8+hashio+bigfirst+w16   enum   0.21  read   2.14  total   2.39  wall   2.41
```

## 20:04:00 algo-bench-split (exit 0, 4.4 s, compilers busy at start: 0)
```n### run 1 : C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182
Discovered 8 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Wildfire\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 473310 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Only Thing That Matters\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 322070 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Mark, Tom and Travis Show (The Enema Strikes Back!)\notes.mid
  parse 0.01s | search 0.02s | store 0.00s  => TOTAL 0.02s
  best score 7873975 | 12 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Fallen Interlude\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 198105 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Natives\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 674720 | 6 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Home Is Such A Lonely Place\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 248065 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Here's Your Letter\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 334755 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - All Of This\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 421590 | 14 paths | sp_cap 4
### run 2 : C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182
Discovered 8 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Wildfire\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 473310 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Only Thing That Matters\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 322070 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Mark, Tom and Travis Show (The Enema Strikes Back!)\notes.mid
  parse 0.01s | search 0.02s | store 0.00s  => TOTAL 0.02s
  best score 7873975 | 12 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - The Fallen Interlude\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 198105 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Natives\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 674720 | 6 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Home Is Such A Lonely Place\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 248065 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - Here's Your Letter\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 334755 | 5 paths | sp_cap 4
C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\BrianTD9192\Blink-182\Blink-182 - All Of This\notes.mid
  parse 0.00s | search 0.00s | store 0.00s  => TOTAL 0.00s
  best score 421590 | 14 paths | sp_cap 4
### run 1 : C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\notes.mid
  parse 0.04s | search 0.14s | store 0.01s  => TOTAL 0.18s
  best score 70757755 | 20 paths | sp_cap 4
### run 2 : C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\blink-182 - Discography\notes.mid
  parse 0.04s | search 0.14s | store 0.01s  => TOTAL 0.18s
  best score 70757755 | 20 paths | sp_cap 4
### run 1 : C:\Clone Hero\songs\Misc Downloads\Rise Against - Discography (2024)
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\Rise Against - Discography (2024)\notes.mid
  parse 0.03s | search 0.11s | store 0.00s  => TOTAL 0.14s
  best score 53618415 | 20 paths | sp_cap 4
### run 2 : C:\Clone Hero\songs\Misc Downloads\Rise Against - Discography (2024)
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\Rise Against - Discography (2024)\notes.mid
  parse 0.03s | search 0.13s | store 0.00s  => TOTAL 0.16s
  best score 53618415 | 20 paths | sp_cap 4
### run 1 : C:\Clone Hero\songs\Misc Downloads\Hail The Sun - Discography (2025)
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\Hail The Sun - Discography (2025)\notes.mid
  parse 0.02s | search 0.08s | store 0.00s  => TOTAL 0.11s
  best score 38843975 | 20 paths | sp_cap 4
### run 2 : C:\Clone Hero\songs\Misc Downloads\Hail The Sun - Discography (2025)
Discovered 1 chart(s) (0 folder error(s)).
C:\Clone Hero\songs\Misc Downloads\Hail The Sun - Discography (2025)\notes.mid
  parse 0.02s | search 0.09s | store 0.00s  => TOTAL 0.11s
  best score 38843975 | 20 paths | sp_cap 4
### run 1 : C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\algo\giants_sng
Discovered 1 chart(s) (0 folder error(s)).
C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\algo\giants_sng\endless1.sng
  parse 0.04s | search 0.07s | store 0.00s  => TOTAL 0.10s
  best score 26577650 | 20 paths | sp_cap 4
### run 2 : C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\algo\giants_sng
Discovered 1 chart(s) (0 folder error(s)).
C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\algo\giants_sng\endless1.sng
  parse 0.04s | search 0.07s | store 0.00s  => TOTAL 0.11s
  best score 26577650 | 20 paths | sp_cap 4
```

## 20:04:05 write-breakdown (exit 0, 109.5 s, compilers busy at start: 0)
```nA_d exit=0 wall_s=18.9 analyzed=19,378 failed=58 analysis_s=17.5 peakWS_MB=498 peakCommit_MB=626 
off exit=0 wall_s=17.2 analyzed=19,378 failed=58 analysis_s=15.7 peakWS_MB=492 peakCommit_MB=627 PERF wall_ms=15735 workers=8 charts=18869 worker_busy_ms=48580 util=0.386 consume_ms=14896 save_ms=12428 commit_ms=0 commits=0 save_per_chart_ms=0.659 consume_per_chart_ms=0.789 last_start_ms=15502 nth_last_end_ms=15503 group=0 cache=0 order= PERFSAVE tempomap=142 begin=48 upsert=1473 length=134 purge1=941 purge2=1243 insert=261 nodes=548 gc=302 dynamics=374 commit=6949 
C exit=0 wall_s=14.6 analyzed=19,378 failed=58 analysis_s=13.1 peakWS_MB=499 peakCommit_MB=637 PERF wall_ms=13093 workers=8 charts=18869 worker_busy_ms=49879 util=0.476 consume_ms=12211 save_ms=9968 commit_ms=0 commits=0 save_per_chart_ms=0.528 consume_per_chart_ms=0.647 last_start_ms=12861 nth_last_end_ms=12861 group=0 cache=1 order= PERFSAVE tempomap=135 begin=20 upsert=719 length=67 purge1=535 purge2=863 insert=107 nodes=509 gc=169 dynamics=306 commit=6525 
C_ckpt exit=0 wall_s=18.6 analyzed=19,378 failed=58 analysis_s=15.0 peakWS_MB=514 peakCommit_MB=634 PERF wall_ms=15009 workers=8 charts=18869 worker_busy_ms=50395 util=0.420 consume_ms=14026 save_ms=10551 commit_ms=0 commits=0 save_per_chart_ms=0.559 consume_per_chart_ms=0.743 last_start_ms=14753 nth_last_end_ms=14756 group=0 cache=1 order= PERFSAVE tempomap=136 begin=20 upsert=784 length=69 purge1=548 purge2=871 insert=108 nodes=543 gc=165 dynamics=289 commit=7004 
C_syncoff exit=0 wall_s=13.2 analyzed=19,378 failed=58 analysis_s=10.2 peakWS_MB=499 peakCommit_MB=619 PERF wall_ms=10150 workers=8 charts=18869 worker_busy_ms=50071 util=0.617 consume_ms=9189 save_ms=5892 commit_ms=0 commits=0 save_per_chart_ms=0.312 consume_per_chart_ms=0.487 last_start_ms=9902 nth_last_end_ms=9903 group=0 cache=1 order= PERFSAVE tempomap=126 begin=20 upsert=727 length=68 purge1=513 purge2=853 insert=109 nodes=516 gc=171 dynamics=298 commit=2475 
CG16_ckpt exit=0 wall_s=10.6 analyzed=19,378 failed=58 analysis_s=9.0 peakWS_MB=502 peakCommit_MB=632 PERF wall_ms=8986 workers=8 charts=18869 worker_busy_ms=50500 util=0.702 consume_ms=8185 save_ms=4375 commit_ms=1944 commits=1616 save_per_chart_ms=0.335 consume_per_chart_ms=0.434 last_start_ms=8718 nth_last_end_ms=8719 group=16 cache=1 order= PERFSAVE tempomap=126 begin=6 upsert=511 length=53 purge1=459 purge2=1195 insert=167 nodes=837 gc=177 dynamics=825 commit=10 
C_cache64MB exit=0 wall_s=15.2 analyzed=19,378 failed=58 analysis_s=13.8 peakWS_MB=591 peakCommit_MB=723 PERF wall_ms=13810 workers=8 charts=18869 worker_busy_ms=50097 util=0.453 consume_ms=12899 save_ms=10437 commit_ms=0 commits=0 save_per_chart_ms=0.553 consume_per_chart_ms=0.684 last_start_ms=13568 nth_last_end_ms=13571 group=0 cache=1 order= PERFSAVE tempomap=141 begin=22 upsert=542 length=66 purge1=411 purge2=639 insert=113 nodes=397 gc=195 dynamics=205 commit=7695
```

## 20:05:55 scan-rescan (exit 0, 16 s, compilers busy at start: 0)
```nrep 1  A baseline                   enum   0.76  read   0.03  total   0.79  wall   1.01  charts 19436 cached 19436
rep 1  findex                       enum   0.75  read   0.03  total   0.78  wall   0.97  charts 19436 cached 19436
rep 1  pwalk8                       enum   0.16  read   0.03  total   0.19  wall   0.39  charts 19436 cached 19436
rep 1  pwalk8+findex                enum   0.16  read   0.03  total   0.19  wall   0.39  charts 19436 cached 19436
rep 1  leafsim (estimate)           enum   0.22  read   0.02  total   0.24  wall   0.45  charts 19319 cached 19319
rep 2  A baseline                   enum   0.79  read   0.03  total   0.82  wall   1.01  charts 19436 cached 19436
rep 2  findex                       enum   0.77  read   0.03  total   0.80  wall   0.98  charts 19436 cached 19436
rep 2  pwalk8                       enum   0.17  read   0.03  total   0.19  wall   0.38  charts 19436 cached 19436
rep 2  pwalk8+findex                enum   0.16  read   0.03  total   0.19  wall   0.39  charts 19436 cached 19436
rep 2  leafsim (estimate)           enum   0.18  read   0.03  total   0.21  wall   0.39  charts 19319 cached 19319
rep 3  A baseline                   enum   0.77  read   0.03  total   0.80  wall   0.99  charts 19436 cached 19436
rep 3  findex                       enum   0.75  read   0.03  total   0.78  wall   0.96  charts 19436 cached 19436
rep 3  pwalk8                       enum   0.17  read   0.03  total   0.20  wall   0.39  charts 19436 cached 19436
rep 3  pwalk8+findex                enum   0.17  read   0.03  total   0.20  wall   0.38  charts 19436 cached 19436
rep 3  leafsim (estimate)           enum   0.19  read   0.03  total   0.21  wall   0.40  charts 19319 cached 19319
rep 4  A baseline                   enum   0.77  read   0.03  total   0.80  wall   0.99  charts 19436 cached 19436
rep 4  findex                       enum   0.76  read   0.03  total   0.78  wall   0.97  charts 19436 cached 19436
rep 4  pwalk8                       enum   0.17  read   0.03  total   0.20  wall   0.38  charts 19436 cached 19436
rep 4  pwalk8+findex                enum   0.17  read   0.03  total   0.19  wall   0.38  charts 19436 cached 19436
rep 4  leafsim (estimate)           enum   0.19  read   0.02  total   0.22  wall   0.40  charts 19319 cached 19319
rep 5  A baseline                   enum   0.77  read   0.03  total   0.80  wall   0.99  charts 19436 cached 19436
rep 5  findex                       enum   0.76  read   0.03  total   0.79  wall   0.98  charts 19436 cached 19436
rep 5  pwalk8                       enum   0.16  read   0.03  total   0.19  wall   0.38  charts 19436 cached 19436
rep 5  pwalk8+findex                enum   0.17  read   0.03  total   0.19  wall   0.38  charts 19436 cached 19436
rep 5  leafsim (estimate)           enum   0.20  read   0.03  total   0.22  wall   0.42  charts 19319 cached 19319

MEDIANS over 5 runs
  A baseline                   enum   0.77  read   0.03  total   0.80  wall   0.99
  findex                       enum   0.76  read   0.03  total   0.78  wall   0.97
  pwalk8                       enum   0.17  read   0.03  total   0.19  wall   0.38
  pwalk8+findex                enum   0.17  read   0.03  total   0.19  wall   0.38
  leafsim (estimate)           enum   0.19  read   0.03  total   0.22  wall   0.40
```

## 20:06:13 engine-lib-hash-verify6c-combo-base (exit 0, 121.1 s, compilers busy at start: 0)
```ncharts 18811 failed 58 | parse 14.617s | graph 13.749s | analyze x1 17.856s | prepare 0.497s | hash 8d17f958172bd6f4
graph destroy 0.251s
charts 18811 failed 58 | parse 13.375s | graph 2.739s | analyze x1 6.557s | prepare 0.459s | hash 8d17f958172bd6f4
graph destroy 0.213s
charts 18811 failed 58 | parse 15.069s | graph 13.582s | analyze x1 17.597s | prepare 0.504s | hash 8d17f958172bd6f4
```

## 20:08:15 algo-lib-cpu (exit 0, 42.5 s, compilers busy at start: 0)
```nlogical CPUs: 16
run 1 wall 19.8 s, CPU 81.5 s (user 58.6 s)
Analyzed 19,378, skipped 0 already stored, 58 failed in 15.5s.
run 2 wall 22.5 s, CPU 73.4 s (user 50.7 s)
Analyzed 19,378, skipped 0 already stored, 58 failed in 18.1s.
```

## 20:08:58 engine-giants-and-redo-AB (exit 0, 1.2 s, compilers busy at start: 0)
```n&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:10
Line |
  10 |        $o = & (Bench $v) --engine "$s\giants" --reps 5 | Select-Object …
     |               ~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,6a,6b,6c,seg,combo\hydra_bench.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 1 base,6a,6b,6c,seg,combo | 
&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:10
Line |
  10 |        $o = & (Bench $v) --engine "$s\giants" --reps 5 | Select-Object …
     |               ~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,6a,6b,6c,seg,combo\hydra_bench.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 2 base,6a,6b,6c,seg,combo | 
&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:10
Line |
  10 |        $o = & (Bench $v) --engine "$s\giants" --reps 5 | Select-Object …
     |               ~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,6a,6b,6c,seg,combo\hydra_bench.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 3 base,6a,6b,6c,seg,combo | 
MEDIAN giants base,6a,6b,6c,seg,combo -1 (runs: -1, -1, -1)
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-wal' because it does not exist.
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-shm' because it does not exist.
&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:19
Line |
  19 |        $out = & "$(VDir $v)\hydra_batch.exe" --db $db --redo 'C:\Clone …
     |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,combo\hydra_batch.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 1 base,combo redo analysis -1 s wall 0 s | 
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-wal' because it does not exist.
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-shm' because it does not exist.
&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:19
Line |
  19 |        $out = & "$(VDir $v)\hydra_batch.exe" --db $db --redo 'C:\Clone …
     |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,combo\hydra_batch.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 2 base,combo redo analysis -1 s wall 0 s | 
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-wal' because it does not exist.
Copy-Item: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:17
Line |
  17 |  … '-wal', '-shm') { Copy-Item "$s\pristine\hydra.db$x" "$db$x" -Force }
     |                      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | Cannot find path
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\pristine\hydra.db-shm' because it does not exist.
&: C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\t_ab.ps1:19
Line |
  19 |        $out = & "$(VDir $v)\hydra_batch.exe" --db $db --redo 'C:\Clone …
     |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     | The term
     | 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\exp\base,combo\hydra_batch.exe' is not recognized as a name of a cmdlet, function, script file, or executable program. Check the spelling of the name, or if a path was included, verify that the path is correct and try again.
round 3 base,combo redo analysis -1 s wall 0 s | 
MEDIAN redo base,combo -1 (runs: -1, -1, -1)
```

## 20:09:01 prof-vsdiag2 (exit 0, 24.9 s, compilers busy at start: 0)
```nMicrosoft (R) VS Standard Collector

Session 32: {2097e42f-003d-4f91-a845-6404cf289e84}
  Running

start exit 0

Microsoft (R) VS Standard Collector

Collection result moved to 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\prof\vsdiag\blink.diagsession'.
Session 32: {2097e42f-003d-4f91-a845-6404cf289e84}
  Stopped

stop exit 0 after 11.3 s

Microsoft (R) VS Standard Collector

Session 33: {2197e42f-003d-4f91-a845-6404cf289e84}
  Running

start exit 0

Microsoft (R) VS Standard Collector

Collection result moved to 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\prof\vsdiag\es3.diagsession'.
Session 33: {2197e42f-003d-4f91-a845-6404cf289e84}
  Stopped

stop exit 0 after 10 s
```

## 20:09:28 engine-giants-and-redo-AB (exit 0, 150.4 s, compilers busy at start: 0)
```nround 1 base | charts 3 failed 0 | parse 0.095s | graph 0.174s | analyze x5 1.561s | prepare 0.009s | hash a97cf6353c01c2c9
round 1 6a | charts 3 failed 0 | parse 0.107s | graph 0.090s | analyze x5 1.286s | prepare 0.010s | hash a97cf6353c01c2c9
round 1 6b | charts 3 failed 0 | parse 0.109s | graph 0.137s | analyze x5 1.651s | prepare 0.009s | hash a97cf6353c01c2c9
round 1 6c | charts 3 failed 0 | parse 0.105s | graph 0.105s | analyze x5 1.333s | prepare 0.009s | hash a97cf6353c01c2c9
round 1 seg | charts 3 failed 0 | parse 0.107s | graph 0.165s | analyze x5 1.665s | prepare 0.011s | hash a97cf6353c01c2c9
round 1 combo | charts 3 failed 0 | parse 0.266s | graph 0.133s | analyze x5 2.083s | prepare 0.013s | hash a97cf6353c01c2c9
round 2 base | charts 3 failed 0 | parse 0.105s | graph 0.206s | analyze x5 1.731s | prepare 0.010s | hash a97cf6353c01c2c9
round 2 6a | charts 3 failed 0 | parse 0.107s | graph 0.101s | analyze x5 1.335s | prepare 0.011s | hash a97cf6353c01c2c9
round 2 6b | charts 3 failed 0 | parse 0.105s | graph 0.137s | analyze x5 1.505s | prepare 0.010s | hash a97cf6353c01c2c9
round 2 6c | charts 3 failed 0 | parse 0.101s | graph 0.109s | analyze x5 1.372s | prepare 0.009s | hash a97cf6353c01c2c9
round 2 seg | charts 3 failed 0 | parse 0.100s | graph 0.161s | analyze x5 1.421s | prepare 0.009s | hash a97cf6353c01c2c9
round 2 combo | charts 3 failed 0 | parse 0.100s | graph 0.059s | analyze x5 0.879s | prepare 0.008s | hash a97cf6353c01c2c9
round 3 base | charts 3 failed 0 | parse 0.097s | graph 0.185s | analyze x5 1.627s | prepare 0.010s | hash a97cf6353c01c2c9
round 3 6a | charts 3 failed 0 | parse 0.102s | graph 0.101s | analyze x5 1.250s | prepare 0.009s | hash a97cf6353c01c2c9
round 3 6b | charts 3 failed 0 | parse 0.103s | graph 0.125s | analyze x5 1.469s | prepare 0.009s | hash a97cf6353c01c2c9
round 3 6c | charts 3 failed 0 | parse 0.099s | graph 0.101s | analyze x5 1.202s | prepare 0.008s | hash a97cf6353c01c2c9
round 3 seg | charts 3 failed 0 | parse 0.102s | graph 0.159s | analyze x5 1.431s | prepare 0.008s | hash a97cf6353c01c2c9
round 3 combo | charts 3 failed 0 | parse 0.111s | graph 0.077s | analyze x5 1.085s | prepare 0.009s | hash a97cf6353c01c2c9
MEDIAN giants base 1.627 (runs: 1.561, 1.627, 1.731)
MEDIAN giants 6a 1.286 (runs: 1.25, 1.286, 1.335)
MEDIAN giants 6b 1.505 (runs: 1.469, 1.505, 1.651)
MEDIAN giants 6c 1.333 (runs: 1.202, 1.333, 1.372)
MEDIAN giants seg 1.431 (runs: 1.421, 1.431, 1.665)
MEDIAN giants combo 1.085 (runs: 0.879, 1.085, 2.083)
round 1 base redo analysis 12 s wall 13.1 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.0s.
round 1 combo redo analysis 12.5 s wall 13.4 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.5s.
round 2 base redo analysis 12.6 s wall 13.5 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.6s.
round 2 combo redo analysis 12.9 s wall 13.8 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.9s.
round 3 base redo analysis 13.7 s wall 14.8 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 13.7s.
round 3 combo redo analysis 22.9 s wall 24 s | Analyzed 19,378, skipped 0 already stored, 58 failed in 22.9s.
MEDIAN redo base 12.6 (runs: 12, 12.6, 13.7)
MEDIAN redo combo 12.9 (runs: 12.5, 12.9, 22.9)
```

## 20:12:00 scan-correctness2 (exit 0, 8.5 s, compilers busy at start: 0)
```nrep 1  E pwalk8+reusebuf+bigfirst   enum   0.18  read   1.85  total   2.03  wall   2.27  charts 19436 cached 0
rep 1  F pwalk8+hashio+bigfirst+w16 enum   0.18  read   2.02  total   2.19  wall   2.37  charts 19436 cached 0
rep 1  G findex+pwalk16 w12         enum   0.16  read   3.14  total   3.30  wall   3.49  charts 19436 cached 0

MEDIANS over 1 runs
  E pwalk8+reusebuf+bigfirst   enum   0.18  read   1.85  total   2.03  wall   2.27
  F pwalk8+hashio+bigfirst+w16 enum   0.18  read   2.02  total   2.19  wall   2.37
  G findex+pwalk16 w12         enum   0.16  read   3.14  total   3.30  wall   3.49
```

## 20:12:10 write-group-ckpt (exit 0, 160.7 s, compilers busy at start: 0)
```nA_1 exit=0 wall_s=16.3 analyzed=19,378 failed=58 analysis_s=15.0 peakWS_MB=510 peakCommit_MB=638 
CG16k10_1 exit=0 wall_s=11.3 analyzed=19,378 failed=58 analysis_s=9.8 peakWS_MB=499 peakCommit_MB=632 PERF wall_ms=9803 workers=8 charts=18869 worker_busy_ms=46723 util=0.596 consume_ms=9040 save_ms=4051 commit_ms=3241 commits=1718 save_per_chart_ms=0.386 consume_per_chart_ms=0.479 last_start_ms=9570 nth_last_end_ms=9570 group=16 cache=1 order= PERFSAVE tempomap=103 begin=6 upsert=450 length=47 purge1=423 purge2=1121 insert=153 nodes=778 gc=173 dynamics=777 commit=10 
CG64k10_1 exit=0 wall_s=11.1 analyzed=19,378 failed=58 analysis_s=9.7 peakWS_MB=523 peakCommit_MB=647 PERF wall_ms=9711 workers=8 charts=18869 worker_busy_ms=48322 util=0.622 consume_ms=8856 save_ms=4144 commit_ms=3159 commits=1087 save_per_chart_ms=0.387 consume_per_chart_ms=0.469 last_start_ms=9429 nth_last_end_ms=9433 group=64 cache=1 order= PERFSAVE tempomap=97 begin=5 upsert=485 length=47 purge1=439 purge2=1232 insert=163 nodes=807 gc=179 dynamics=670 commit=7 
CG64k10L_1 exit=0 wall_s=11.3 analyzed=19,378 failed=58 analysis_s=9.9 peakWS_MB=510 peakCommit_MB=639 PERF wall_ms=9854 workers=8 charts=18869 worker_busy_ms=49583 util=0.629 consume_ms=9347 save_ms=4308 commit_ms=3196 commits=1188 save_per_chart_ms=0.398 consume_per_chart_ms=0.495 last_start_ms=9627 nth_last_end_ms=9627 group=64 cache=1 order= PERFSAVE tempomap=0 begin=5 upsert=510 length=51 purge1=483 purge2=1300 insert=172 nodes=846 gc=213 dynamics=711 commit=8 
A_2 exit=0 wall_s=15.6 analyzed=19,378 failed=58 analysis_s=14.3 peakWS_MB=522 peakCommit_MB=646 
CG16k10_2 exit=0 wall_s=11.5 analyzed=19,378 failed=58 analysis_s=10.1 peakWS_MB=498 peakCommit_MB=620 PERF wall_ms=10071 workers=8 charts=18869 worker_busy_ms=47251 util=0.586 consume_ms=9294 save_ms=4142 commit_ms=3368 commits=1681 save_per_chart_ms=0.398 consume_per_chart_ms=0.493 last_start_ms=9830 nth_last_end_ms=9834 group=16 cache=1 order= PERFSAVE tempomap=113 begin=6 upsert=460 length=47 purge1=434 purge2=1136 insert=160 nodes=779 gc=185 dynamics=799 commit=10 
CG64k10_2 exit=0 wall_s=14.3 analyzed=19,378 failed=58 analysis_s=12.7 peakWS_MB=482 peakCommit_MB=614 PERF wall_ms=12742 workers=8 charts=18869 worker_busy_ms=47132 util=0.462 consume_ms=12021 save_ms=4253 commit_ms=5259 commits=762 save_per_chart_ms=0.504 consume_per_chart_ms=0.637 last_start_ms=12491 nth_last_end_ms=12493 group=64 cache=1 order= PERFSAVE tempomap=111 begin=5 upsert=499 length=47 purge1=459 purge2=1286 insert=168 nodes=835 gc=180 dynamics=644 commit=7 
CG64k10L_2 exit=0 wall_s=15.3 analyzed=19,378 failed=58 analysis_s=12.6 peakWS_MB=495 peakCommit_MB=632 PERF wall_ms=12597 workers=8 charts=18869 worker_busy_ms=48053 util=0.477 consume_ms=12109 save_ms=4176 commit_ms=5446 commits=1128 save_per_chart_ms=0.510 consume_per_chart_ms=0.642 last_start_ms=12373 nth_last_end_ms=12374 group=64 cache=1 order= PERFSAVE tempomap=0 begin=4 upsert=493 length=48 purge1=472 purge2=1262 insert=170 nodes=828 gc=207 dynamics=675 commit=7 
A_3 exit=0 wall_s=17.3 analyzed=19,378 failed=58 analysis_s=15.8 peakWS_MB=516 peakCommit_MB=648 
CG16k10_3 exit=0 wall_s=11.7 analyzed=19,378 failed=58 analysis_s=10.3 peakWS_MB=483 peakCommit_MB=619 PERF wall_ms=10298 workers=8 charts=18869 worker_busy_ms=47529 util=0.577 consume_ms=9511 save_ms=4327 commit_ms=3429 commits=1705 save_per_chart_ms=0.411 consume_per_chart_ms=0.504 last_start_ms=10045 nth_last_end_ms=10050 group=16 cache=1 order= PERFSAVE tempomap=117 begin=6 upsert=471 length=48 purge1=448 purge2=1201 insert=164 nodes=860 gc=188 dynamics=801 commit=12 
CG64k10_3 exit=0 wall_s=11.7 analyzed=19,378 failed=58 analysis_s=10.3 peakWS_MB=500 peakCommit_MB=625 PERF wall_ms=10323 workers=8 charts=18869 worker_busy_ms=50355 util=0.610 consume_ms=9601 save_ms=4560 commit_ms=3221 commits=859 save_per_chart_ms=0.412 consume_per_chart_ms=0.509 last_start_ms=10095 nth_last_end_ms=10101 group=64 cache=1 order= PERFSAVE tempomap=121 begin=6 upsert=536 length=50 purge1=482 purge2=1367 insert=182 nodes=904 gc=200 dynamics=691 commit=8 
CG64k10L_3 exit=0 wall_s=11.5 analyzed=19,378 failed=58 analysis_s=9.9 peakWS_MB=504 peakCommit_MB=633 PERF wall_ms=9933 workers=8 charts=18869 worker_busy_ms=48502 util=0.610 consume_ms=9431 save_ms=4182 commit_ms=3467 commits=1197 save_per_chart_ms=0.405 consume_per_chart_ms=0.500 last_start_ms=9688 nth_last_end_ms=9692 group=64 cache=1 order= PERFSAVE tempomap=0 begin=5 upsert=501 length=49 purge1=464 purge2=1250 insert=170 nodes=838 gc=186 dynamics=703 commit=8
```

## 20:14:51 prof-vsdiag-blink20 (exit 0, 18 s, compilers busy at start: 0)
```nMicrosoft (R) VS Standard Collector

Session 42: {2a97e42f-003d-4f91-a845-6404cf289e84}
  Running

start exit 0

Microsoft (R) VS Standard Collector

Collection result moved to 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\prof\vsdiag\b.diagsession'.
Session 42: {2a97e42f-003d-4f91-a845-6404cf289e84}
  Stopped

stop exit 0 after 7.6 s

Microsoft (R) VS Standard Collector

Session 43: {2b97e42f-003d-4f91-a845-6404cf289e84}
  Running

start exit 0

Microsoft (R) VS Standard Collector

Collection result moved to 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\prof\vsdiag\C.diagsession'.
Session 43: {2b97e42f-003d-4f91-a845-6404cf289e84}
  Stopped

stop exit 0 after 7.4 s
```

## 20:15:11 engine-correctness-combo (exit 0, 20.4 s, compilers busy at start: 0)
```nFound 19,436 charts.
[1685/19436]    498,240  Buckner & Garcia - Found Me the Bomb                 3 0 1                               
[1891/19436]    321,155  an Unkindness - Foundation                           E2 1 0 E2                           
[2227/19436]    146,920  Stephen Sanchez - Until I Found You                  0 2                                 
[2379/19436]    429,780  New Found Glory - My Friends Over You                3 3 3 0                             
[2381/19436]    315,330  New Found Glory - Iris                               4 0 6 0                             
[2382/19436]    332,425  New Found Glory - Greatest of All Time               3 0 0 0 3                           
[2383/19436]    329,295  New Found Glory - All Downhill from Here             1 1 3 0                             
[2394/19436]    455,560  My Chemical Romance - The Foundations of Decay       0 2 6                               
[2571/19436]    459,065  Gin Blossoms - Found Out About You                   1 2 3 1                             
[8859/19436]    885,480  sasakure.UK - Lost and Found (Band Remaster)         0- 1 1+ 3 2 0                       
[12326/19436]    106,200  Killitorous - Have You Found Jesus Yet Gump?         (No activations.)                   
[13814/19436]    521,760  Brandon Burkhalter - Newfound Rapture                (No activations.)                   
[17490/19436]    312,590  New Found Glory - Part of Your World                 1- 3+                               
[17583/19436]    407,980  My Chemical Romance - The Foundations of Decay       1 1- 5                              
Analyzed 19,378, skipped 0 already stored, 58 failed in 12.2s.
tables A: ['charts', 'dynamics', 'meta', 'path_refs', 'paths', 'results', 'songmeta']
charts: rows A 0 B 0 | key md5 | keys 0 | differing keys 0 | columns {}
dynamics: rows A 18811 B 18811 | key md5 | keys 18811 | differing keys 0 | columns {}
meta: rows A 2 B 2 | key key | keys 2 | differing keys 0 | columns {}
path_refs: rows A 90674 B 90674 | key hyhash | keys 18811 | differing keys 12573 | columns {'result_id': 59537}
paths: rows A 90674 B 90674 | key hyhash | keys 18811 | differing keys 0 | columns {}
results: rows A 18811 B 18811 | key hyhash | keys 18811 | differing keys 12573 | columns {'result_id': 12573}
Traceback (most recent call last):
  File "C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\engine\cmpdb.py", line 26, in <module>
    ra = A.execute(f"select * from '{t}'").fetchall()
sqlite3.OperationalError: Could not decode to UTF-8 column 'ref_name' with text 'O Vurd�n'
```

## 20:15:32 prof-vsdiag-blink20b (exit 0, 13.4 s, compilers busy at start: 0)
```nMicrosoft (R) VS Standard Collector

Session 52: {3497e42f-003d-4f91-a845-6404cf289e84}
  Running

start exit 0

Microsoft (R) VS Standard Collector

Collection result moved to 'C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\prof\vsdiag\blink20.diagsession'.
Session 52: {3497e42f-003d-4f91-a845-6404cf289e84}
  Stopped

stop exit 0 after 11.8 s
```

## 20:15:46 write-matrix (exit 0, 185.5 s, compilers busy at start: 0)
```nA_1 exit=0 wall_s=20.0 analyzed=19,378 failed=58 analysis_s=18.3 peakWS_MB=499 peakCommit_MB=631 
C_1 exit=0 wall_s=20.8 analyzed=19,378 failed=58 analysis_s=18.0 peakWS_MB=518 peakCommit_MB=641 PERF wall_ms=18043 workers=8 charts=18869 worker_busy_ms=50885 util=0.353 consume_ms=17145 save_ms=14532 commit_ms=0 commits=0 save_per_chart_ms=0.770 consume_per_chart_ms=0.909 last_start_ms=17780 nth_last_end_ms=17781 group=0 cache=1 order= PERFSAVE tempomap=150 begin=21 upsert=737 length=69 purge1=534 purge2=888 insert=112 nodes=533 gc=173 dynamics=308 commit=10993 
CG16_1 exit=0 wall_s=14.1 analyzed=19,378 failed=58 analysis_s=12.6 peakWS_MB=500 peakCommit_MB=640 PERF wall_ms=12646 workers=8 charts=18869 worker_busy_ms=51443 util=0.508 consume_ms=11858 save_ms=4403 commit_ms=5576 commits=1414 save_per_chart_ms=0.529 consume_per_chart_ms=0.628 last_start_ms=12360 nth_last_end_ms=12362 group=16 cache=1 order= PERFSAVE tempomap=139 begin=6 upsert=475 length=52 purge1=472 purge2=1216 insert=175 nodes=838 gc=185 dynamics=823 commit=10 
CG16k10_1 exit=0 wall_s=12.1 analyzed=19,378 failed=58 analysis_s=10.7 peakWS_MB=473 peakCommit_MB=625 PERF wall_ms=10714 workers=8 charts=18869 worker_busy_ms=48544 util=0.566 consume_ms=9980 save_ms=4265 commit_ms=3744 commits=1581 save_per_chart_ms=0.424 consume_per_chart_ms=0.529 last_start_ms=10489 nth_last_end_ms=10493 group=16 cache=1 order= PERFSAVE tempomap=116 begin=6 upsert=474 length=50 purge1=453 purge2=1180 insert=163 nodes=803 gc=187 dynamics=812 commit=11 
A_2 exit=0 wall_s=15.4 analyzed=19,378 failed=58 analysis_s=14.1 peakWS_MB=500 peakCommit_MB=630 
C_2 exit=0 wall_s=14.1 analyzed=19,378 failed=58 analysis_s=12.6 peakWS_MB=503 peakCommit_MB=641 PERF wall_ms=12623 workers=8 charts=18869 worker_busy_ms=44470 util=0.440 consume_ms=11802 save_ms=9579 commit_ms=0 commits=0 save_per_chart_ms=0.508 consume_per_chart_ms=0.625 last_start_ms=12392 nth_last_end_ms=12394 group=0 cache=1 order= PERFSAVE tempomap=118 begin=21 upsert=681 length=59 purge1=480 purge2=903 insert=102 nodes=549 gc=169 dynamics=297 commit=6186 
CG16_2 exit=0 wall_s=13.4 analyzed=19,378 failed=58 analysis_s=12.1 peakWS_MB=476 peakCommit_MB=617 PERF wall_ms=12128 workers=8 charts=18869 worker_busy_ms=47422 util=0.489 consume_ms=11432 save_ms=4169 commit_ms=5577 commits=1317 save_per_chart_ms=0.517 consume_per_chart_ms=0.606 last_start_ms=11888 nth_last_end_ms=11890 group=16 cache=1 order= PERFSAVE tempomap=127 begin=6 upsert=451 length=48 purge1=463 purge2=1146 insert=162 nodes=773 gc=184 dynamics=786 commit=11 
CG16k10_2 exit=0 wall_s=13.4 analyzed=19,378 failed=58 analysis_s=11.8 peakWS_MB=496 peakCommit_MB=624 PERF wall_ms=11815 workers=8 charts=18869 worker_busy_ms=48522 util=0.513 consume_ms=11048 save_ms=4244 commit_ms=4642 commits=1677 save_per_chart_ms=0.471 consume_per_chart_ms=0.586 last_start_ms=11568 nth_last_end_ms=11573 group=16 cache=1 order= PERFSAVE tempomap=117 begin=6 upsert=470 length=48 purge1=452 purge2=1163 insert=166 nodes=804 gc=185 dynamics=812 commit=11 
A_3 exit=0 wall_s=22.9 analyzed=19,378 failed=58 analysis_s=20.5 peakWS_MB=520 peakCommit_MB=644 
C_3 exit=0 wall_s=13.7 analyzed=19,378 failed=58 analysis_s=12.1 peakWS_MB=470 peakCommit_MB=603 PERF wall_ms=12094 workers=8 charts=18869 worker_busy_ms=42808 util=0.442 consume_ms=11324 save_ms=9209 commit_ms=0 commits=0 save_per_chart_ms=0.488 consume_per_chart_ms=0.600 last_start_ms=11886 nth_last_end_ms=11888 group=0 cache=1 order= PERFSAVE tempomap=108 begin=19 upsert=653 length=60 purge1=482 purge2=777 insert=98 nodes=444 gc=156 dynamics=285 commit=6111 
CG16_3 exit=0 wall_s=12.6 analyzed=19,378 failed=58 analysis_s=11.3 peakWS_MB=521 peakCommit_MB=649 PERF wall_ms=11332 workers=8 charts=18869 worker_busy_ms=45369 util=0.500 consume_ms=10636 save_ms=3909 commit_ms=5137 commits=1361 save_per_chart_ms=0.479 consume_per_chart_ms=0.564 last_start_ms=11085 nth_last_end_ms=11087 group=16 cache=1 order= PERFSAVE tempomap=106 begin=5 upsert=415 length=45 purge1=399 purge2=1073 insert=152 nodes=745 gc=178 dynamics=770 commit=10 
CG16k10_3 exit=0 wall_s=11.1 analyzed=19,378 failed=58 analysis_s=9.8 peakWS_MB=523 peakCommit_MB=659 PERF wall_ms=9793 workers=8 charts=18869 worker_busy_ms=46751 util=0.597 consume_ms=9024 save_ms=4035 commit_ms=3183 commits=1710 save_per_chart_ms=0.383 consume_per_chart_ms=0.478 last_start_ms=9560 nth_last_end_ms=9564 group=16 cache=1 order= PERFSAVE tempomap=98 begin=6 upsert=439 length=46 purge1=418 purge2=1115 insert=155 nodes=780 gc=174 dynamics=784 commit=10
```

## 20:19:35 write-workers (exit 0, 184.1 s, compilers busy at start: 0)
```nA_1 exit=0 wall_s=17.8 analyzed=19,378 failed=58 analysis_s=16.5 peakWS_MB=494 peakCommit_MB=625 
W8_1 exit=0 wall_s=13.2 analyzed=19,378 failed=58 analysis_s=11.6 peakWS_MB=504 peakCommit_MB=635 PERF wall_ms=11587 workers=8 charts=18869 worker_busy_ms=50094 util=0.540 consume_ms=10910 save_ms=4681 commit_ms=3714 commits=1413 save_per_chart_ms=0.445 consume_per_chart_ms=0.578 last_start_ms=11354 nth_last_end_ms=11355 group=16 cache=1 order= PERFSAVE tempomap=146 begin=7 upsert=520 length=53 purge1=495 purge2=1299 insert=180 nodes=894 gc=200 dynamics=863 commit=11 
W12_1 exit=0 wall_s=13.9 analyzed=19,378 failed=58 analysis_s=12.3 peakWS_MB=538 peakCommit_MB=658 PERF wall_ms=12323 workers=12 charts=18869 worker_busy_ms=54614 util=0.369 consume_ms=11527 save_ms=5665 commit_ms=3882 commits=1218 save_per_chart_ms=0.506 consume_per_chart_ms=0.611 last_start_ms=12028 nth_last_end_ms=12029 group=16 cache=1 order= PERFSAVE tempomap=183 begin=7 upsert=544 length=58 purge1=555 purge2=1813 insert=192 nodes=1137 gc=238 dynamics=911 commit=14 
W15_1 exit=0 wall_s=19.8 analyzed=19,378 failed=58 analysis_s=17.0 peakWS_MB=528 peakCommit_MB=650 PERF wall_ms=16951 workers=15 charts=18869 worker_busy_ms=56920 util=0.224 consume_ms=16043 save_ms=5325 commit_ms=7702 commits=1193 save_per_chart_ms=0.690 consume_per_chart_ms=0.850 last_start_ms=16570 nth_last_end_ms=16572 group=16 cache=1 order= PERFSAVE tempomap=210 begin=8 upsert=567 length=58 purge1=603 purge2=1499 insert=193 nodes=981 gc=263 dynamics=916 commit=13 
A_2 exit=0 wall_s=17.8 analyzed=19,378 failed=58 analysis_s=15.7 peakWS_MB=496 peakCommit_MB=620 
W8_2 exit=0 wall_s=12.6 analyzed=19,378 failed=58 analysis_s=11.1 peakWS_MB=485 peakCommit_MB=604 PERF wall_ms=11053 workers=8 charts=18869 worker_busy_ms=51934 util=0.587 consume_ms=10164 save_ms=4687 commit_ms=3536 commits=1699 save_per_chart_ms=0.436 consume_per_chart_ms=0.539 last_start_ms=10765 nth_last_end_ms=10768 group=16 cache=1 order= PERFSAVE tempomap=135 begin=6 upsert=501 length=53 purge1=473 purge2=1314 insert=172 nodes=949 gc=197 dynamics=862 commit=12 
W12_2 exit=0 wall_s=14.9 analyzed=19,378 failed=58 analysis_s=13.5 peakWS_MB=518 peakCommit_MB=641 PERF wall_ms=13453 workers=12 charts=18869 worker_busy_ms=78706 util=0.488 consume_ms=12145 save_ms=5273 commit_ms=4822 commits=1894 save_per_chart_ms=0.535 consume_per_chart_ms=0.644 last_start_ms=13039 nth_last_end_ms=13039 group=16 cache=1 order= PERFSAVE tempomap=172 begin=8 upsert=582 length=60 purge1=570 purge2=1420 insert=193 nodes=977 gc=235 dynamics=1030 commit=15 
W15_2 exit=0 wall_s=12.4 analyzed=19,378 failed=58 analysis_s=10.9 peakWS_MB=544 peakCommit_MB=666 PERF wall_ms=10878 workers=15 charts=18869 worker_busy_ms=51762 util=0.317 consume_ms=10067 save_ms=4665 commit_ms=3504 commits=1201 save_per_chart_ms=0.433 consume_per_chart_ms=0.534 last_start_ms=10531 nth_last_end_ms=10533 group=16 cache=1 order= PERFSAVE tempomap=176 begin=7 upsert=499 length=56 purge1=533 purge2=1301 insert=176 nodes=839 gc=225 dynamics=825 commit=15 
A_3 exit=0 wall_s=16.2 analyzed=19,378 failed=58 analysis_s=14.9 peakWS_MB=483 peakCommit_MB=613 
W8_3 exit=0 wall_s=11.5 analyzed=19,378 failed=58 analysis_s=10.1 peakWS_MB=513 peakCommit_MB=640 PERF wall_ms=10138 workers=8 charts=18869 worker_busy_ms=49556 util=0.611 consume_ms=9324 save_ms=4232 commit_ms=3327 commits=1919 save_per_chart_ms=0.401 consume_per_chart_ms=0.494 last_start_ms=9912 nth_last_end_ms=9916 group=16 cache=1 order= PERFSAVE tempomap=118 begin=6 upsert=463 length=50 purge1=452 purge2=1144 insert=164 nodes=801 gc=176 dynamics=838 commit=10 
W12_3 exit=0 wall_s=13.9 analyzed=19,378 failed=58 analysis_s=12.3 peakWS_MB=533 peakCommit_MB=660 PERF wall_ms=12324 workers=12 charts=18869 worker_busy_ms=53943 util=0.365 consume_ms=11497 save_ms=4821 commit_ms=4451 commits=1200 save_per_chart_ms=0.491 consume_per_chart_ms=0.609 last_start_ms=11955 nth_last_end_ms=11956 group=16 cache=1 order= PERFSAVE tempomap=171 begin=7 upsert=511 length=54 purge1=533 purge2=1367 insert=184 nodes=910 gc=217 dynamics=843 commit=12 
W15_3 exit=0 wall_s=18.1 analyzed=19,378 failed=58 analysis_s=15.5 peakWS_MB=562 peakCommit_MB=685 PERF wall_ms=15456 workers=15 charts=18869 worker_busy_ms=55217 util=0.238 consume_ms=14698 save_ms=4778 commit_ms=7305 commits=1185 save_per_chart_ms=0.640 consume_per_chart_ms=0.779 last_start_ms=15144 nth_last_end_ms=15147 group=16 cache=1 order= PERFSAVE tempomap=174 begin=7 upsert=504 length=54 purge1=561 purge2=1341 insert=178 nodes=867 gc=242 dynamics=825 commit=13
```

## 20:23:08 write-order (exit 0, 169.6 s, compilers busy at start: 0)
```nA7_1 exit=0 wall_s=16.1 analyzed=19,378 failed=58 analysis_s=14.6 peakWS_MB=511 peakCommit_MB=648 
G_1 exit=0 wall_s=11.7 analyzed=19,378 failed=58 analysis_s=10.4 peakWS_MB=519 peakCommit_MB=650 PERF wall_ms=10363 workers=8 charts=18869 worker_busy_ms=50325 util=0.607 consume_ms=9571 save_ms=4297 commit_ms=3484 commits=1878 save_per_chart_ms=0.412 consume_per_chart_ms=0.507 last_start_ms=10134 nth_last_end_ms=10136 group=16 cache=1 order= PERFSAVE tempomap=122 begin=6 upsert=474 length=51 purge1=460 purge2=1167 insert=164 nodes=816 gc=171 dynamics=844 commit=10 
GOS_1 exit=0 wall_s=12.1 analyzed=19,378 failed=58 analysis_s=10.8 peakWS_MB=516 peakCommit_MB=661 PERF wall_ms=10473 workers=8 charts=18869 worker_busy_ms=50455 util=0.602 consume_ms=9259 save_ms=4256 commit_ms=3255 commits=1908 save_per_chart_ms=0.398 consume_per_chart_ms=0.491 last_start_ms=10466 nth_last_end_ms=10465 group=16 cache=1 order=size PERFSAVE tempomap=113 begin=6 upsert=555 length=50 purge1=409 purge2=1154 insert=164 nodes=828 gc=143 dynamics=818 commit=8 
GOP_1 exit=0 wall_s=12.6 analyzed=19,378 failed=58 analysis_s=11.2 peakWS_MB=621 peakCommit_MB=808 PERF wall_ms=11172 workers=8 charts=18869 worker_busy_ms=51370 util=0.575 consume_ms=9701 save_ms=4371 commit_ms=3553 commits=2351 save_per_chart_ms=0.420 consume_per_chart_ms=0.514 last_start_ms=11166 nth_last_end_ms=11165 group=16 cache=1 order=file:C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\write\cost_prevtime.csv PERFSAVE tempomap=133 begin=6 upsert=558 length=53 purge1=437 purge2=1169 insert=165 nodes=824 gc=159 dynamics=845 commit=10 
A7_2 exit=0 wall_s=17.1 analyzed=19,378 failed=58 analysis_s=15.8 peakWS_MB=497 peakCommit_MB=634 
G_2 exit=0 wall_s=12.2 analyzed=19,378 failed=58 analysis_s=10.7 peakWS_MB=498 peakCommit_MB=629 PERF wall_ms=10707 workers=8 charts=18869 worker_busy_ms=51793 util=0.605 consume_ms=9885 save_ms=4526 commit_ms=3523 commits=1805 save_per_chart_ms=0.427 consume_per_chart_ms=0.524 last_start_ms=10480 nth_last_end_ms=10482 group=16 cache=1 order= PERFSAVE tempomap=122 begin=7 upsert=510 length=54 purge1=481 purge2=1231 insert=178 nodes=865 gc=189 dynamics=866 commit=11 
GOS_2 exit=0 wall_s=14.3 analyzed=19,378 failed=58 analysis_s=12.8 peakWS_MB=561 peakCommit_MB=725 PERF wall_ms=12436 workers=8 charts=18869 worker_busy_ms=52268 util=0.525 consume_ms=11153 save_ms=4468 commit_ms=4495 commits=1817 save_per_chart_ms=0.475 consume_per_chart_ms=0.591 last_start_ms=12428 nth_last_end_ms=12427 group=16 cache=1 order=size PERFSAVE tempomap=125 begin=7 upsert=588 length=54 purge1=455 purge2=1204 insert=171 nodes=850 gc=162 dynamics=830 commit=9 
GOP_2 exit=0 wall_s=19.3 analyzed=19,378 failed=58 analysis_s=16.5 peakWS_MB=614 peakCommit_MB=802 PERF wall_ms=16531 workers=8 charts=18869 worker_busy_ms=52688 util=0.398 consume_ms=15143 save_ms=4431 commit_ms=8072 commits=2365 save_per_chart_ms=0.663 consume_per_chart_ms=0.803 last_start_ms=16526 nth_last_end_ms=16525 group=16 cache=1 order=file:C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\write\cost_prevtime.csv PERFSAVE tempomap=128 begin=6 upsert=567 length=51 purge1=459 purge2=1202 insert=167 nodes=833 gc=169 dynamics=826 commit=11 
A7_3 exit=0 wall_s=16.3 analyzed=19,378 failed=58 analysis_s=14.9 peakWS_MB=511 peakCommit_MB=638 
G_3 exit=0 wall_s=11.5 analyzed=19,378 failed=58 analysis_s=10.2 peakWS_MB=517 peakCommit_MB=646 PERF wall_ms=10211 workers=8 charts=18869 worker_busy_ms=49836 util=0.610 consume_ms=9412 save_ms=4319 commit_ms=3378 commits=1888 save_per_chart_ms=0.408 consume_per_chart_ms=0.499 last_start_ms=9975 nth_last_end_ms=9975 group=16 cache=1 order= PERFSAVE tempomap=118 begin=6 upsert=469 length=51 purge1=460 purge2=1182 insert=167 nodes=833 gc=176 dynamics=835 commit=10 
GOS_3 exit=0 wall_s=12.2 analyzed=19,378 failed=58 analysis_s=10.8 peakWS_MB=580 peakCommit_MB=737 PERF wall_ms=10503 workers=8 charts=18869 worker_busy_ms=50709 util=0.604 consume_ms=9284 save_ms=4319 commit_ms=3253 commits=1876 save_per_chart_ms=0.401 consume_per_chart_ms=0.492 last_start_ms=10497 nth_last_end_ms=10495 group=16 cache=1 order=size PERFSAVE tempomap=113 begin=6 upsert=557 length=51 purge1=438 purge2=1170 insert=169 nodes=835 gc=144 dynamics=817 commit=8 
GOP_3 exit=0 wall_s=12.4 analyzed=19,378 failed=58 analysis_s=11.0 peakWS_MB=649 peakCommit_MB=826 PERF wall_ms=10989 workers=8 charts=18869 worker_busy_ms=51328 util=0.584 consume_ms=9511 save_ms=4337 commit_ms=3401 commits=2426 save_per_chart_ms=0.410 consume_per_chart_ms=0.504 last_start_ms=10984 nth_last_end_ms=10981 group=16 cache=1 order=file:C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\write\cost_prevtime.csv PERFSAVE tempomap=133 begin=6 upsert=548 length=51 purge1=450 purge2=1169 insert=162 nodes=807 gc=167 dynamics=813 commit=16
```

## 20:26:00 parse-profile (exit 0, 42.7 s, compilers busy at start: 0)
```ncharts 18869 failed 58 | parse(best of 1) 15.051s | dyn 3.848s | hash 937d9aed20f59dff
PP 0=0.419 1=2.773 2=0.097 3=3.352 4=0.314 5=0.235 6=0.134 7=4.961 8=0.949 9=0.905 10=0.128 11=0.066 12=0.650 13=3.456 14=6.183 15=0.000
charts 18869 failed 58 | parse(best of 1) 15.379s | dyn 3.865s | hash 937d9aed20f59dff
```

## 20:26:46 write-fresh-correctness (exit 0, 36.6 s, compilers busy at start: 0)
```nFA exit=0 wall_s=20.4 analyzed=19,378 failed=58 analysis_s=15.6 peakWS_MB=502 peakCommit_MB=626 
FB exit=0 wall_s=15.9 analyzed=19,378 failed=58 analysis_s=11.3 peakWS_MB=520 peakCommit_MB=676 PERF wall_ms=10977 workers=8 charts=18869 worker_busy_ms=53255 util=0.606 consume_ms=9863 save_ms=4166 commit_ms=3911 commits=1618 save_per_chart_ms=0.428 consume_per_chart_ms=0.523 last_start_ms=10970 nth_last_end_ms=10970 group=16 cache=1 order=size PERFSAVE tempomap=125 begin=6 upsert=654 length=59 purge1=257 purge2=151 insert=299 nodes=1998 gc=174 dynamics=426 commit=7
```

## 20:27:28 parse-profile2 (exit 0, 21 s, compilers busy at start: 0)
```ncharts 18869 failed 58 | parse(best of 1) 14.831s | dyn 3.858s | hash 937d9aed20f59dff
PP 0=0.410 1=2.709 2=0.095 3=3.356 4=0.310 5=0.237 6=0.133 7=4.903 8=0.916 9=0.874 10=0.126 11=0.067 12=0.640 13=3.458 14=6.079 15=215.000
```

## 20:28:58 parse-profile3 (exit 0, 22.5 s, compilers busy at start: 0)
```ncharts 18869 failed 58 | parse(best of 1) 16.151s | dyn 3.964s | hash 937d9aed20f59dff
PP 0=0.427 1=2.803 2=0.098 3=4.361 4=0.320 5=0.239 6=0.133 7=4.990 8=0.948 9=0.904 10=0.128 11=0.067 12=0.709 13=4.466 14=6.182 15=215.000
TSC(Gcyc) 16=16.788 17=5.259 18=0.268 19=18.734 20=13.429 21=0.000 22=0.000 23=0.000 24=0.000 25=0.000 26=0.000 27=0.000 28=0.000 29=0.000 30=0.000 31=0.000
```

## 20:35:29 parse-correct (exit 0, 66 s, compilers busy at start: 0)
```n== all (off='')
charts 18869 failed 58 | parse(best of 1) 8.845s | dyn 0.430s | hash 937d9aed20f59dff
== dyn (off='midi,chart')
charts 18869 failed 58 | parse(best of 1) 14.138s | dyn 0.438s | hash 937d9aed20f59dff
== midi (off='dyn,chart')
charts 18869 failed 58 | parse(best of 1) 13.242s | dyn 3.953s | hash 937d9aed20f59dff
== chart (off='dyn,midi')
charts 18869 failed 58 | parse(best of 1) 12.075s | dyn 3.955s | hash 937d9aed20f59dff
```

## 20:37:57 parse-altconfig (exit 0, 117.2 s, compilers busy at start: 0)
```ncharts 41 failed 32 | parse(best of 1) 0.004s | dyn 0.000s | hash 674b19836af270f1
charts 18869 failed 5744 | parse(best of 1) 105.003s | dyn 0.002s | hash 9ffa2d5b5e72906a
charts 41 failed 32 | parse(best of 1) 0.004s | dyn 0.000s | hash 674b19836af270f1
charts 18869 failed 5744 | parse(best of 1) 9.169s | dyn 0.001s | hash 9ffa2d5b5e72906a
```

## 20:40:21 parse-ab-all (exit 0, 154.6 s, compilers busy at start: 0)
```nlib A1 wall 27.19s | charts 18869 failed 58 | parse(best of 1) 20.241s | dyn 4.656s | hash 937d9aed20f59dff | busy after 0
lib B1 wall 14.43s | charts 18869 failed 58 | parse(best of 1) 11.548s | dyn 0.567s | hash 937d9aed20f59dff | busy after 0
lib A2 wall 42.29s | charts 18869 failed 58 | parse(best of 1) 35.779s | dyn 4.315s | hash 937d9aed20f59dff | busy after 0
lib B2 wall 14.15s | charts 18869 failed 58 | parse(best of 1) 11.430s | dyn 0.470s | hash 937d9aed20f59dff | busy after 0
lib A3 wall 22.88s | charts 18869 failed 58 | parse(best of 1) 16.603s | dyn 4.123s | hash 937d9aed20f59dff | busy after 0
lib B3 wall 12.97s | charts 18869 failed 58 | parse(best of 1) 10.201s | dyn 0.510s | hash 937d9aed20f59dff | busy after 0
slow50 A1 | charts 50 failed 0 | parse(best of 3) 1.465s | dyn 0.174s | hash 8a7768aeb2abce96
slow50 B1 | charts 50 failed 0 | parse(best of 3) 0.611s | dyn 0.019s | hash 8a7768aeb2abce96
slow50 A2 | charts 50 failed 0 | parse(best of 3) 1.380s | dyn 0.174s | hash 8a7768aeb2abce96
slow50 B2 | charts 50 failed 0 | parse(best of 3) 0.611s | dyn 0.019s | hash 8a7768aeb2abce96
slow50 A3 | charts 50 failed 0 | parse(best of 3) 1.360s | dyn 0.169s | hash 8a7768aeb2abce96
slow50 B3 | charts 50 failed 0 | parse(best of 3) 0.618s | dyn 0.019s | hash 8a7768aeb2abce96
```

## 20:43:08 parse-ab-alone (exit 0, 176.8 s, compilers busy at start: 0)
```nbase1 | charts 18869 failed 58 | parse(best of 1) 14.877s | dyn 3.731s | hash 937d9aed20f59dff
dyn1 | charts 18869 failed 58 | parse(best of 1) 15.323s | dyn 0.471s | hash 937d9aed20f59dff
midi1 | charts 18869 failed 58 | parse(best of 1) 13.126s | dyn 3.947s | hash 937d9aed20f59dff
chart1 | charts 18869 failed 58 | parse(best of 1) 11.693s | dyn 3.850s | hash 937d9aed20f59dff
all1 | charts 18869 failed 58 | parse(best of 1) 9.132s | dyn 0.452s | hash 937d9aed20f59dff
base2 | charts 18869 failed 58 | parse(best of 1) 15.313s | dyn 3.855s | hash 937d9aed20f59dff
dyn2 | charts 18869 failed 58 | parse(best of 1) 16.451s | dyn 0.486s | hash 937d9aed20f59dff
midi2 | charts 18869 failed 58 | parse(best of 1) 12.414s | dyn 3.820s | hash 937d9aed20f59dff
chart2 | charts 18869 failed 58 | parse(best of 1) 11.996s | dyn 4.034s | hash 937d9aed20f59dff
all2 | charts 18869 failed 58 | parse(best of 1) 9.919s | dyn 0.487s | hash 937d9aed20f59dff
```

## 20:46:32 parse-engine-hash (exit 0, 91.4 s, compilers busy at start: 0)
```ncharts 18811 failed 58 | parse 15.809s | graph 13.425s | analyze x1 17.788s | prepare 0.525s | hash 8d17f958172bd6f4
graph destroy 0.233s
charts 18811 failed 58 | parse 9.104s | graph 13.022s | analyze x1 17.196s | prepare 0.492s | hash 8d17f958172bd6f4
graph destroy 0.225s
```

## 20:48:12 parse-batch-fresh (exit 0, 38.1 s, compilers busy at start: 0)
```nA wall 21.4s | Found 19,436 charts. | [1/19436] FAILED Faster Than a Bullet: No Expert Pro Drums notes in this chart. | [112/19436] FAILED Regeneration: No Expert Pro Drums notes in this chart. | [139/19436] FAILED Piano Sonata No. 11 - Ronda Alla Turca (Mozart): No Expert Pro Drums notes in this chart. | [140/19436] FAILED Piano Sonata No. 14 - Moonlight Sonata (Beethoven): No Expert Pro Drums notes in this chart. | [142/19436] FAILED Flight of the Bumblebee (Rimsky-Korsakov - Piano Version): No Expert Pro Drums notes in this chart. | [143/19436] FAILED Gymnopédie No. 1 (Satie): No Expert Pro Drums notes in this chart. | [146/19436] FAILED The Entertainer (Joplin): No Expert Pro Drums notes in this chart. | [149/19436] FAILED Bagatelle No. 25 - Für Elise (Beethoven): No Expert Pro Drums notes in this chart. | [462/19436] FAILED SEX: No Expert Pro Drums notes in this chart. | [474/19436] FAILED Wabi-Sabi: No Expert Pro Drums notes in this chart. | [517/19436] FAILED Far Wanderings: No Expert Pro Drums notes in this chart. | [547/19436]    341,670  Senses Fail - Lady in a Blue Dress                   0 0 3                                | [603/19436] FAILED Hanuman: No Expert Pro Drums notes in this chart. | [605/19436] FAILED Buster Voodoo: No Expert Pro Drums notes in this chart. | [656/19436] FAILED Break: No Expert Pro Drums notes in this chart. | [660/19436] FAILED We Like the Moon: No Expert Pro Drums notes in this chart. | [875/19436] FAILED Lollytown: No Expert Pro Drums notes in this chart. | [925/19436] FAILED Toccata and Fugue in D Minor (Bach): No Expert Pro Drums notes in this chart. | [926/19436] FAILED Für Elise (Beethoven): No Expert Pro Drums notes in this chart. | [1079/19436] FAILED At the Cathedral: No Expert Pro Drums notes in this chart. | [1275/19436] FAILED Halftime: No Expert Pro Drums notes in this chart. | [1354/19436]    252,025  Fail Emotions - Wasted                               0 E0 2                               | [1356/19436]    527,055  Fail Emotions - Transformation Pt. 1                 2 0 1 2                              | [1358/19436]    467,955  Fail Emotions - Suit & Tie                           6 5 0                                | [1359/19436]    476,975  Fail Emotions - Dance Macabre                        3+ 1 2+ 3                            | [1360/19436]    577,745  Fail Emotions - Shades                               2 6 0 5                              | [1361/19436]    608,910  Fail Emotions - Makes Bad                            0 4 6 1 4                            | [1415/19436]    480,680  Edenshade - Another Purity Failing                   3 4 1                                | [1496/19436] FAILED Run Away With Me: No Expert Pro Drums notes in this chart. | [1540/19436] FAILED Upstream: No Expert Pro Drums notes in this chart. | [1558/19436] FAILED Canary: No Expert Pro Drums notes in this chart. | [1575/19436] FAILED Dawn of Spring: No Expert Pro Drums notes in this chart. | [1685/19436]    498,240  Buckner & Garcia - Found Me the Bomb                 3 0 1                                | [1746/19436] FAILED Santa Fe (RB3 version): No Expert Pro Drums notes in this chart. | [1747/19436] FAILED Santa Fe: No Expert Pro Drums notes in this chart. | [1886/19436] FAILED Words Cannot Express: No Expert Pro Drums notes in this chart. | [1891/19436]    321,155  an Unkindness - Foundation                           E2 1 0 E2                            | [2026/19436] FAILED !I: No Expert Pro Drums notes in this chart. | [2227/19436]    146,920  Stephen Sanchez - Until I Found You                  0 2                                  | [2378/19436]    429,780  New Found Glory - My Friends Over You                3 3 3 0                              | [2379/19436]    315,330  New Found Glory - Iris                               4 0 6 0                              | [2382/19436]    332,425  New Found Glory - Greatest of All Time               3 0 0 0 3                            | [2384/19436]    329,295  New Found Glory - All Downhill from Here             1 1 3 0                              | [2394/19436]    455,560  My Chemical Romance - The Foundations of Decay       0 2 6                                | [2476/19436] FAILED Love Yourself: No Expert Pro Drums notes in this chart. | [2571/19436]    459,065  Gin Blossoms - Found Out About You                   1 2 3 1                              | [2704/19436] FAILED Glycerine: No Expert Pro Drums notes in this chart. | [2723/19436]    319,280  Breaking Benjamin - Failure                          5 0 1 0                              | [2909/19436]    567,710  The Clash - Rudie Can't Fail                         0 0 E0 E1                            | [3203/19436] FAILED Oh My Love: No Expert Pro Drums notes in this chart. | [3268/19436] FAILED Landslide: No Expert Pro Drums notes in this chart. | [3282/19436] FAILED More Than Words: No Expert Pro Drums notes in this chart. | [3404/19436] FAILED She's Always a Woman: No Expert Pro Drums notes in this chart. | [3827/19436] FAILED The End: No Expert Pro Drums notes in this chart. | [4285/19436] FAILED Redemption Song: No Expert Pro Drums notes in this chart. | [4793/19436] FAILED Song of the Century: No Expert Pro Drums notes in this chart. | [4810/19436] FAILED Good Riddance (Time of Your Life): No Expert Pro Drums notes in this chart. | [4840/19436] FAILED She's Leaving Home: No Expert Pro Drums notes in this chart. | [4853/19436] FAILED Because: No Expert Pro Drums notes in this chart. | [5152/19436]    189,350  Senses Fail - Can't Be Saved (Guitar to Drums)       (No activations.)                    | [5276/19436] FAILED 2112 Pt. 3 - Discovery: No Expert Pro Drums notes in this chart. | [5491/19436] FAILED Spanish Fly: No Expert Pro Drums notes in this chart. | [5509/19436] FAILED Cathedral: No Expert Pro Drums notes in this chart. | [5951/19436] FAILED River of Sorrows (Unplugged): No Expert Pro Drums notes in this chart. | [6142/19436] FAILED Oceandust (Unrest): No Expert Pro Drums notes in this chart. | [7977/19436]    312,230  Last Chance to Reason - Coded to Fail                (No activations.)                    | [8565/19436]    535,295  August Burns Red - Forged by Failure                 (No activations.)                    | [8571/19436]    245,360  As the Structure Fails - The Promise                 (No activations.)                    | [8575/19436]    291,375  As The Structure Fails - In Vain                     (No activations.)                    | [8576/19436]    316,205  As The Structure Fails - Backdown                    (No activations.)                    | [8577/19436]    268,555  As The Structure Fails - Take My Life (feat. Manafes (No activations.)                    | [8578/19436]    405,815  As The Structure Fails - The Surface                 (No activations.)                    | [8579/19436]    217,585  As The Structure Fails - Osiris                      (No activations.)                    | [8580/19436]    272,770  As The Structure Fails - Say It to Me                (No activations.)                    | [8581/19436]    470,840  As The Structure Fails - All the Weight              0 4 0 0                              | [8582/19436]    333,445  As The Structure Fails - Soul Drown                  (No activations.)                    | [8583/19436]    326,345  As The Structure Fails - Poison Drip                 (No activations.)                    | [8585/19436]    210,870  As The Structure Fails - Hell or High Water          2 1+ 1                               | [8586/19436]    212,575  As The Structure Fails - Heartless                   (No activations.)                    | [8587/19436]    253,770  As The Structure Fails - Fallen                      (No activations.)                    | [8858/19436]    885,480  sasakure.UK - Lost and Found (Band Remaster)         0- 1 1+ 3 2 0                        | [9450/19436] FAILED Pennyroyal Tea: No Expert Pro Drums notes in this chart. | [9592/19436] FAILED entierro / por favor leer: No Expert Pro Drums notes in this chart. | [10026/19436]    464,620  Failure - Sergeant Politeness                        0 0- 3 4                             | [10489/19436]    473,170  Ingested - Expect to Fail (feat. Josh Middleton)     (No activations.)                    | [10822/19436]    682,830  Converge - You Fail Me                               3 0 2- 3 1+                          | [11481/19436] FAILED Trapped in Ice: No Expert Pro Drums notes in this chart. | [11482/19436] FAILED The Winged Bull: No Expert Pro Drums notes in this chart. | [11511/19436] FAILED Harmonic Sleep Engine: No Expert Pro Drums notes in this chart. | [11942/19436] FAILED Spanish Waltz: No Expert Pro Drums notes in this chart. | [11943/19436] FAILED Spanish Waltz: No Expert Pro Drums notes in this chart. | [11954/19436] FAILED Play: No Expert Pro Drums notes in this chart. | [12025/19436]    616,170  Senses Fail - Still Searching                        0 2- 6- 1                            | [12325/19436]    106,200  Killitorous - Have You Found Jesus Yet Gump?         (No activations.)                    | [13305/19436]  1,119,740  Here Comes The Kraken - Don't Fail Me Darko          0 1 1 0 1 2                          | [13814/19436]    521,760  Brandon Burkhalter - Newfound Rapture                (No activations.)                    | [13943/19436]  1,092,285  Arsis - Failing Winds Of Hopeless Greed              0 1+ 1 2                             | [14172/19436] FAILED Lullaby: No Expert Pro Drums notes in this chart. | [14173/19436] FAILED Near, but so Far Gone: No Expert Pro Drums notes in this chart. | [14816/19436] FAILED G.O.A.T. (Combined Guitar) (B): No Expert Pro Drums notes in this chart. | [15212/19436]    449,710  I Am King - Fortune In Your Failure                  1- 4+ 0 0 0 1- 1+                    | [15671/19436]    384,060  Knocked Loose - Contorted in the Faille (R)          (No activations.)                    | [15875/19436] FAILED Vacant: No Expert Pro Drums notes in this chart. | [17488/19436]    312,590  New Found Glory - Part of Your World                 1- 3+                                | [17583/19436]    407,980  My Chemical Romance - The Foundations of Decay       1 1- 5                               | [17877/19436] FAILED Steamed Hams But It's A Piano Dub: No Expert Pro Drums notes in this chart. | [18738/19436]    518,915  Fail Emotions - Transfornation Pt.1 (2x Bass Pedal)  2 0 1 2                              | [19269/19436]    298,320  Dance Gavin Dance - Born To Fail                     (No activations.)                    | Analyzed 19,378, skipped 0 already stored, 58 failed in 13.2s. | Failures:
B wall 16.5s | Found 19,436 charts. | [1/19436] FAILED Faster Than a Bullet: No Expert Pro Drums notes in this chart. | [112/19436] FAILED Regeneration: No Expert Pro Drums notes in this chart. | [138/19436] FAILED The Entertainer (Joplin): No Expert Pro Drums notes in this chart. | [140/19436] FAILED Piano Sonata No. 14 - Moonlight Sonata (Beethoven): No Expert Pro Drums notes in this chart. | [142/19436] FAILED Piano Sonata No. 11 - Ronda Alla Turca (Mozart): No Expert Pro Drums notes in this chart. | [145/19436] FAILED Gymnopédie No. 1 (Satie): No Expert Pro Drums notes in this chart. | [147/19436] FAILED Flight of the Bumblebee (Rimsky-Korsakov - Piano Version): No Expert Pro Drums notes in this chart. | [149/19436] FAILED Bagatelle No. 25 - Für Elise (Beethoven): No Expert Pro Drums notes in this chart. | [461/19436] FAILED SEX: No Expert Pro Drums notes in this chart. | [476/19436] FAILED Wabi-Sabi: No Expert Pro Drums notes in this chart. | [512/19436] FAILED Far Wanderings: No Expert Pro Drums notes in this chart. | [543/19436]    341,670  Senses Fail - Lady in a Blue Dress                   0 0 3                                | [603/19436] FAILED Hanuman: No Expert Pro Drums notes in this chart. | [605/19436] FAILED Buster Voodoo: No Expert Pro Drums notes in this chart. | [659/19436] FAILED Break: No Expert Pro Drums notes in this chart. | [660/19436] FAILED We Like the Moon: No Expert Pro Drums notes in this chart. | [875/19436] FAILED Lollytown: No Expert Pro Drums notes in this chart. | [926/19436] FAILED Toccata and Fugue in D Minor (Bach): No Expert Pro Drums notes in this chart. | [928/19436] FAILED Für Elise (Beethoven): No Expert Pro Drums notes in this chart. | [1077/19436] FAILED At the Cathedral: No Expert Pro Drums notes in this chart. | [1277/19436] FAILED Halftime: No Expert Pro Drums notes in this chart. | [1355/19436]    252,025  Fail Emotions - Wasted                               0 E0 2                               | [1357/19436]    527,055  Fail Emotions - Transformation Pt. 1                 2 0 1 2                              | [1358/19436]    467,955  Fail Emotions - Suit & Tie                           6 5 0                                | [1359/19436]    577,745  Fail Emotions - Shades                               2 6 0 5                              | [1361/19436]    476,975  Fail Emotions - Dance Macabre                        3+ 1 2+ 3                            | [1363/19436]    608,910  Fail Emotions - Makes Bad                            0 4 6 1 4                            | [1415/19436]    480,680  Edenshade - Another Purity Failing                   3 4 1                                | [1494/19436] FAILED Run Away With Me: No Expert Pro Drums notes in this chart. | [1537/19436] FAILED Upstream: No Expert Pro Drums notes in this chart. | [1558/19436] FAILED Canary: No Expert Pro Drums notes in this chart. | [1576/19436] FAILED Dawn of Spring: No Expert Pro Drums notes in this chart. | [1685/19436]    498,240  Buckner & Garcia - Found Me the Bomb                 3 0 1                                | [1745/19436] FAILED Santa Fe (RB3 version): No Expert Pro Drums notes in this chart. | [1747/19436] FAILED Santa Fe: No Expert Pro Drums notes in this chart. | [1887/19436] FAILED Words Cannot Express: No Expert Pro Drums notes in this chart. | [1891/19436]    321,155  an Unkindness - Foundation                           E2 1 0 E2                            | [2029/19436] FAILED !I: No Expert Pro Drums notes in this chart. | [2229/19436]    146,920  Stephen Sanchez - Until I Found You                  0 2                                  | [2380/19436]    315,330  New Found Glory - Iris                               4 0 6 0                              | [2381/19436]    429,780  New Found Glory - My Friends Over You                3 3 3 0                              | [2382/19436]    332,425  New Found Glory - Greatest of All Time               3 0 0 0 3                            | [2384/19436]    329,295  New Found Glory - All Downhill from Here             1 1 3 0                              | [2393/19436]    455,560  My Chemical Romance - The Foundations of Decay       0 2 6                                | [2479/19436] FAILED Love Yourself: No Expert Pro Drums notes in this chart. | [2571/19436]    459,065  Gin Blossoms - Found Out About You                   1 2 3 1                              | [2705/19436] FAILED Glycerine: No Expert Pro Drums notes in this chart. | [2723/19436]    319,280  Breaking Benjamin - Failure                          5 0 1 0                              | [2909/19436]    567,710  The Clash - Rudie Can't Fail                         0 0 E0 E1                            | [3208/19436] FAILED Oh My Love: No Expert Pro Drums notes in this chart. | [3269/19436] FAILED Landslide: No Expert Pro Drums notes in this chart. | [3278/19436] FAILED More Than Words: No Expert Pro Drums notes in this chart. | [3404/19436] FAILED She's Always a Woman: No Expert Pro Drums notes in this chart. | [3828/19436] FAILED The End: No Expert Pro Drums notes in this chart. | [4285/19436] FAILED Redemption Song: No Expert Pro Drums notes in this chart. | [4793/19436] FAILED Song of the Century: No Expert Pro Drums notes in this chart. | [4810/19436] FAILED Good Riddance (Time of Your Life): No Expert Pro Drums notes in this chart. | [4845/19436] FAILED She's Leaving Home: No Expert Pro Drums notes in this chart. | [4856/19436] FAILED Because: No Expert Pro Drums notes in this chart. | [5152/19436]    189,350  Senses Fail - Can't Be Saved (Guitar to Drums)       (No activations.)                    | [5278/19436] FAILED 2112 Pt. 3 - Discovery: No Expert Pro Drums notes in this chart. | [5489/19436] FAILED Spanish Fly: No Expert Pro Drums notes in this chart. | [5511/19436] FAILED Cathedral: No Expert Pro Drums notes in this chart. | [5953/19436] FAILED River of Sorrows (Unplugged): No Expert Pro Drums notes in this chart. | [6144/19436] FAILED Oceandust (Unrest): No Expert Pro Drums notes in this chart. | [7977/19436]    312,230  Last Chance to Reason - Coded to Fail                (No activations.)                    | [8566/19436]    535,295  August Burns Red - Forged by Failure                 (No activations.)                    | [8572/19436]    291,375  As The Structure Fails - In Vain                     (No activations.)                    | [8573/19436]    245,360  As the Structure Fails - The Promise                 (No activations.)                    | [8576/19436]    316,205  As The Structure Fails - Backdown                    (No activations.)                    | [8578/19436]    268,555  As The Structure Fails - Take My Life (feat. Manafes (No activations.)                    | [8579/19436]    470,840  As The Structure Fails - All the Weight              0 4 0 0                              | [8580/19436]    405,815  As The Structure Fails - The Surface                 (No activations.)                    | [8581/19436]    272,770  As The Structure Fails - Say It to Me                (No activations.)                    | [8582/19436]    333,445  As The Structure Fails - Soul Drown                  (No activations.)                    | [8583/19436]    217,585  As The Structure Fails - Osiris                      (No activations.)                    | [8584/19436]    326,345  As The Structure Fails - Poison Drip                 (No activations.)                    | [8585/19436]    210,870  As The Structure Fails - Hell or High Water          2 1+ 1                               | [8586/19436]    212,575  As The Structure Fails - Heartless                   (No activations.)                    | [8587/19436]    253,770  As The Structure Fails - Fallen                      (No activations.)                    | [8857/19436]    885,480  sasakure.UK - Lost and Found (Band Remaster)         0- 1 1+ 3 2 0                        | [9449/19436] FAILED Pennyroyal Tea: No Expert Pro Drums notes in this chart. | [9596/19436] FAILED entierro / por favor leer: No Expert Pro Drums notes in this chart. | [10026/19436]    464,620  Failure - Sergeant Politeness                        0 0- 3 4                             | [10490/19436]    473,170  Ingested - Expect to Fail (feat. Josh Middleton)     (No activations.)                    | [10825/19436]    682,830  Converge - You Fail Me                               3 0 2- 3 1+                          | [11481/19436] FAILED Trapped in Ice: No Expert Pro Drums notes in this chart. | [11483/19436] FAILED The Winged Bull: No Expert Pro Drums notes in this chart. | [11512/19436] FAILED Harmonic Sleep Engine: No Expert Pro Drums notes in this chart. | [11941/19436] FAILED Spanish Waltz: No Expert Pro Drums notes in this chart. | [11943/19436] FAILED Spanish Waltz: No Expert Pro Drums notes in this chart. | [11949/19436] FAILED Play: No Expert Pro Drums notes in this chart. | [12021/19436]    616,170  Senses Fail - Still Searching                        0 2- 6- 1                            | [12325/19436]    106,200  Killitorous - Have You Found Jesus Yet Gump?         (No activations.)                    | [13306/19436]  1,119,740  Here Comes The Kraken - Don't Fail Me Darko          0 1 1 0 1 2                          | [13815/19436]    521,760  Brandon Burkhalter - Newfound Rapture                (No activations.)                    | [13943/19436]  1,092,285  Arsis - Failing Winds Of Hopeless Greed              0 1+ 1 2                             | [14172/19436] FAILED Lullaby: No Expert Pro Drums notes in this chart. | [14173/19436] FAILED Near, but so Far Gone: No Expert Pro Drums notes in this chart. | [14819/19436] FAILED G.O.A.T. (Combined Guitar) (B): No Expert Pro Drums notes in this chart. | [15213/19436]    449,710  I Am King - Fortune In Your Failure                  1- 4+ 0 0 0 1- 1+                    | [15671/19436]    384,060  Knocked Loose - Contorted in the Faille (R)          (No activations.)                    | [15875/19436] FAILED Vacant: No Expert Pro Drums notes in this chart. | [17491/19436]    312,590  New Found Glory - Part of Your World                 1- 3+                                | [17584/19436]    407,980  My Chemical Romance - The Foundations of Decay       1 1- 5                               | [17880/19436] FAILED Steamed Hams But It's A Piano Dub: No Expert Pro Drums notes in this chart. | [18739/19436]    518,915  Fail Emotions - Transfornation Pt.1 (2x Bass Pedal)  2 0 1 2                              | [19268/19436]    298,320  Dance Gavin Dance - Born To Fail                     (No activations.)                    | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.2s. | Failures:
```

## 20:49:04 parse-batch-redo (exit 0, 82.2 s, compilers busy at start: 0)
```nA1 wall 15.1s | Analyzed 19,378, skipped 0 already stored, 58 failed in 14.2s.
B1 wall 20.6s | Analyzed 19,378, skipped 0 already stored, 58 failed in 19.3s.
A2 wall 13.8s | Analyzed 19,378, skipped 0 already stored, 58 failed in 12.9s.
B2 wall 10.4s | Analyzed 19,378, skipped 0 already stored, 58 failed in 9.4s.
A3 wall 11.2s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.3s.
B3 wall 10.8s | Analyzed 19,378, skipped 0 already stored, 58 failed in 9.8s.
```

## 20:50:34 parse-batch-redo2 (exit 0, 83.1 s, compilers busy at start: 0)
```nA1 wall 11.5s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.5s.
B1 wall 11.2s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.2s.
A2 wall 11.9s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.8s.
B2 wall 11.4s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.3s.
A3 wall 24.5s | Analyzed 19,378, skipped 0 already stored, 58 failed in 19.8s.
B3 wall 12.4s | Analyzed 19,378, skipped 0 already stored, 58 failed in 10.3s.
```

