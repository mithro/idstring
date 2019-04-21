This inserts all(3948533) features of the xc7a50t in a vector.

To get resource usage, run with `make run`, which uses `/usr/bin/time` to measure running time and peak memory.

Current output:
```
=== Tables ===
--- 0 level (2149 entries)---
...
0 of the 2148 strings are stored in the pointer.
--- 1 level (1232 entries)---
...
196 of the 1231 strings are stored in the pointer.
--- 2 level (1138 entries)---
...
119 of the 1137 strings are stored in the pointer.
--- 3 level (1 entries)---
0 of the 0 strings are stored in the pointer.
	Command being timed: "./main"
	User time (seconds): 695.78
	System time (seconds): 22.78
	Percent of CPU this job got: 99%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 12:04.46
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 38168
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 0
	Minor (reclaiming a frame) page faults: 2784
	Voluntary context switches: 28
	Involuntary context switches: 106472
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 0
	Socket messages received: 0
	Signals delivered: 0
	Page size (bytes): 4096
	Exit status: 0
```
