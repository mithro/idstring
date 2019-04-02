
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

