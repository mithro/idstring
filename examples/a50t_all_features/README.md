This inserts all(3948533) features of the xc7a50t in a vector.

To get resource usage, run with `make run`, which uses `/usr/bin/time` to measure running time and peak memory.

Current output:
```
=== Tables ===
--- 0 level (2149 entries)---
...
0 of the 2148 strings are stored in the pointer.
--- 1 level (1246 entries)---
...
200 of the 1245 strings are stored in the pointer.
--- 2 level (1448 entries)---
...
119 of the 1447 strings are stored in the pointer.
--- 3 level (4 entries)---
...
0 of the 3 strings are stored in the pointer.
	Command being timed: "./main"
	User time (seconds): 1.62
	System time (seconds): 0.08
	Percent of CPU this job got: 81%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:02.09
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 40108
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 0
	Minor (reclaiming a frame) page faults: 3867
	Voluntary context switches: 33
	Involuntary context switches: 561
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 0
	Socket messages received: 0
	Signals delivered: 0
	Page size (bytes): 4096
	Exit status: 0
```
