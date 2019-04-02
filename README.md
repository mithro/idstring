
The aim of this library is to store arbitrary "id strings" in the same space as
a int64 (8 bytes).

The actual id string objects are an array of indexes into global tables.

The global tables hold id string data pointer objects.

The id string data pointer objects use
[Tagged Pointer](https://en.wikipedia.org/wiki/Tagged_pointer) to store small
strings (7 characters or less) directly in the pointer. Longer strings are
allocated and stored externally to the structure.

TODO:
 - [ ] Lots of clean up
 - [ ] Move to 6bit ASCII to get 10 characters inside a pointer.
 - [ ] Use some type of better allocator for external string memory.
 - [ ] Support interning `const char*` strings.
 - [ ] Support prebuilding the table.
 - [ ] Make the tests tests.
 - [ ] Documentation.

# Why

The primary goal is to make storing [FASM files](https://en.wikipedia.org/wiki/Tagged_pointer) much smaller in memory.

# License

Your choice of;
 * Apache 2.0
 * BSD / MIT
 * ISC

# Current Example Output

```
=== Tables ===
--- 0 level (28 entries)---
[0001] = "A1";
[0002] = "A1/";
[0003] = "A2";
[0004] = "AAA";
[0005] = "AAAA";
[0006] = "AAAAA";
[0007] = "AAAAAA";
[0008] = "AAAAAAA";
[0009] = "AAAAAAAA";
[0010] = "AAAAAAAAA";
[0011] = "AAAAAAAAAA";
[0012] = "CMT_FIFO_L_X107Y97";
[0013] = "CMT_TOP_L_UPPER_T_X106Y96";
[0014] = "DSP_L_X34Y85";
[0015] = "HCLK_R_X110Y78";
[0016] = "HCLK_R_X64Y130";
[0017] = "INT_INTERFACE_L_X34Y87";
[0018] = "INT_INTERFACE_L_X42Y87";
[0019] = "INT_INTERFACE_R_X23Y126";
[0020] = "INT_L_X24Y126";
[0021] = "INT_L_X26Y123";
[0022] = "INT_L_X30Y123";
[0023] = "INT_L_X32Y87";
[0024] = "INT_L_X34Y87";
[0025] = "INT_L_X36Y87";
[0026] = "INT_L_X38Y87";
[0027] = "INT_L_X40Y87";
--- 1 level (20 entries)---
[0001] = "B1";
[0002] = "B2";
[0003] = "B3";
[0004] = "CMT_FIFO_LH2_0";
[0005] = "CMT_TOP_LH2_0";
[0006] = "DSP_LH10_2";
[0007] = "HCLK_LV5";
[0008] = "HCLK_NN2BEG2";
[0009] = "INT_INTERFACE_LH10";
[0010] = "INT_INTERFACE_LH2";
[0011] = "INT_INTERFACE_WW2END1";
[0012] = "WW2A1";
[0013] = "WW4C3";
[0014] = "WW4A3";
[0015] = "LH11";
[0016] = "LH9";
[0017] = "LH7";
[0018] = "LH5";
[0019] = "LH3";
--- 2 level (4 entries)---
[0001] = "C";
[0002] = "C/D";
[0003] = "C/D/E";
--- 3 level (4 entries)---
[0001] = "D";
[0002] = "E";
[0003] = "F";


=== Strings ===
   0: 'A1;
      i"A1"
   1: 'A1/;
      i"A1/"
   2: 'A1/B1;
      i"A1" / i"B1"
   3: 'A1/B1/C;
      i"A1" / i"B1" / i"C"
   4: 'A1/B1/C/D;
      i"A1" / i"B1" / i"C" / i"D"
   5: 'A1/B2/C/D/E;
      i"A1" / i"B2" / i"C/D" / i"E"
   6: 'A1/B3/C/D/E/F;
      i"A1" / i"B3" / i"C/D/E" / i"F"
   7: 'A2/B1/C/D/E/F;
      i"A2" / i"B1" / i"C/D/E" / i"F"
   8: 'AAA/B1/C/D/E/F;
      i"AAA" / i"B1" / i"C/D/E" / i"F"
   9: 'AAAA/B1/C/D/E/F;
      i"AAAA" / i"B1" / i"C/D/E" / i"F"
  10: 'AAAAA/B1/C/D/E/F;
      i"AAAAA" / i"B1" / i"C/D/E" / i"F"
  11: 'AAAAAA/B1/C/D/E/F;
      i"AAAAAA" / i"B1" / i"C/D/E" / i"F"
  12: 'AAAAAAA/B1/C/D/E/F;
      i"AAAAAAA" / i"B1" / i"C/D/E" / i"F"
  13: 'AAAAAAAA/B1/C/D/E/F;
      e"AAAAAAAA" / i"B1" / i"C/D/E" / i"F"
  14: 'AAAAAAAAA/B1/C/D/E/F;
      e"AAAAAAAAA" / i"B1" / i"C/D/E" / i"F"
  15: 'AAAAAAAAAA/B1/C/D/E/F;
      e"AAAAAAAAAA" / i"B1" / i"C/D/E" / i"F"
  16: 'CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0;
      e"CMT_FIFO_L_X107Y97" / e"CMT_FIFO_LH2_0"
  17: 'CMT_TOP_L_UPPER_T_X106Y96/CMT_TOP_LH2_0;
      e"CMT_TOP_L_UPPER_T_X106Y96" / e"CMT_TOP_LH2_0"
  18: 'DSP_L_X34Y85/DSP_LH10_2;
      e"DSP_L_X34Y85" / e"DSP_LH10_2"
  19: 'HCLK_R_X110Y78/HCLK_LV5;
      e"HCLK_R_X110Y78" / e"HCLK_LV5"
  20: 'HCLK_R_X64Y130/HCLK_NN2BEG2;
      e"HCLK_R_X64Y130" / e"HCLK_NN2BEG2"
  21: 'INT_INTERFACE_L_X34Y87/INT_INTERFACE_LH10;
      e"INT_INTERFACE_L_X34Y87" / e"INT_INTERFACE_LH10"
  22: 'INT_INTERFACE_L_X42Y87/INT_INTERFACE_LH2;
      e"INT_INTERFACE_L_X42Y87" / e"INT_INTERFACE_LH2"
  23: 'INT_INTERFACE_R_X23Y126/INT_INTERFACE_WW2END1;
      e"INT_INTERFACE_R_X23Y126" / e"INT_INTERFACE_WW2END1"
  24: 'INT_L_X24Y126/WW2A1;
      e"INT_L_X24Y126" / i"WW2A1"
  25: 'INT_L_X26Y123/WW4C3;
      e"INT_L_X26Y123" / i"WW4C3"
  26: 'INT_L_X30Y123/WW4A3;
      e"INT_L_X30Y123" / i"WW4A3"
  27: 'INT_L_X32Y87/LH11;
      e"INT_L_X32Y87" / i"LH11"
  28: 'INT_L_X34Y87/LH9;
      e"INT_L_X34Y87" / i"LH9"
  29: 'INT_L_X36Y87/LH7;
      e"INT_L_X36Y87" / i"LH7"
  30: 'INT_L_X38Y87/LH5;
      e"INT_L_X38Y87" / i"LH5"
  31: 'INT_L_X40Y87/LH3;
      e"INT_L_X40Y87" / i"LH3"


=== Comparisons ===
   0: 'A1'
   1: 'A1/' == 'A1/B1/C/D'
   2: 'A1/B1'
   3: 'A1/B1/C'
   4: 'A1/B1/C/D' == 'A1/B1/C/D'
   5: 'A1/B2/C/D/E'
   6: 'A1/B3/C/D/E/F'
   7: 'A2/B1/C/D/E/F'
   8: 'AAA/B1/C/D/E/F'
   9: 'AAAA/B1/C/D/E/F'
  10: 'AAAAA/B1/C/D/E/F'
  11: 'AAAAAA/B1/C/D/E/F'
  12: 'AAAAAAA/B1/C/D/E/F'
  13: 'AAAAAAAA/B1/C/D/E/F'
  14: 'AAAAAAAAA/B1/C/D/E/F'
  15: 'AAAAAAAAAA/B1/C/D/E/F'
  16: 'CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0' == 'CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0'
  17: 'CMT_TOP_L_UPPER_T_X106Y96/CMT_TOP_LH2_0'
  18: 'DSP_L_X34Y85/DSP_LH10_2'
  19: 'HCLK_R_X110Y78/HCLK_LV5'
  20: 'HCLK_R_X64Y130/HCLK_NN2BEG2'
  21: 'INT_INTERFACE_L_X34Y87/INT_INTERFACE_LH10'
  22: 'INT_INTERFACE_L_X42Y87/INT_INTERFACE_LH2'
  23: 'INT_INTERFACE_R_X23Y126/INT_INTERFACE_WW2END1'
  24: 'INT_L_X24Y126/WW2A1'
  25: 'INT_L_X26Y123/WW4C3'
  26: 'INT_L_X30Y123/WW4A3'
  27: 'INT_L_X32Y87/LH11'
  28: 'INT_L_X34Y87/LH9'
  29: 'INT_L_X36Y87/LH7'
  30: 'INT_L_X38Y87/LH5'
  31: 'INT_L_X40Y87/LH3' == 'INT_L_X40Y87/LH3'
```
