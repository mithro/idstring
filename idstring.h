#include <algorithm>    // std::min

#include <endian.h>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdio>

// Future optimization: Use 6-bit ASCII
//  - [A-Z] - 26
//  - [0-9] - 10 - 36 -> 6bits
// (2**6 - 36) == 28 spare characters
//
// - 32/6, 32%6 == 5 characters, 2bit == 4
// - 64/6, 64%6 == 10 characters, 4bit == 16

// On 64bit platforms a pointer takes 8 bytes. Hence we could easily fit a 7
// byte string into a pointer.
//
// Assuming
//  - 32bit alignment, bottom 2bit static - 2**2 = 4, 4 bytes == 3 bytes storage
//  - 64bit alignment, bottom 3bit static - 2**3 = 8, 8 bytes == 7 bytes storage

// id_string_data_ptr == istr_dptr
// Pointer which allows storing small strings directly inside the
// pointer.

#define ISTR_DPTR_INTERNAL_MAXLEN (sizeof(char*))
#define ISTR_DPTR_MASK 0b111

//#if INTPTR_MAX == 4294967295
//#define ISTR_DPTR_MASK 0b11
//#elif INTPTR_MAX == 18446744073709551615
//#endif

// id_string -- Collection of indexes into the istr_dptr_table structures.
// sizeof(uint64_t)/sizeof(uint16_t)
// 8/2 == 4
#define ISTR_LEVEL_MAX 4
static char ISTR_DELIMITER = '.';
void istr_set_delimiter(char x);

typedef union {
    struct {
#if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t length;
#endif
        char internal[ISTR_DPTR_INTERNAL_MAXLEN];

#if BYTE_ORDER == BIG_ENDIAN
        uint8_t length;
#endif
    };
    const char* external;
} istr_dptr_t;

// id_strings_table - Stores 65,535 id_string_data_ptr values
// 65,535 elements * sizeof(id_string_data_ptr) / 1024 / 1024
// == 65,535 * 8 / 1024 / 1024
// == 0.5 megabytes
typedef struct {
    uint16_t used = 1; // We leave the zero index empty.
    istr_dptr_t strings[UINT16_MAX];
} istr_dptr_table_t;

// Tables for storing string data pointers, one for each level.
extern istr_dptr_table_t istr_dptr_tables[ISTR_LEVEL_MAX];

// String structure == 8bytes == uint64_t
// Array of uint16_t indexes into istr_dptr_tables
typedef struct {
    union {
        uint16_t dptr_idx[ISTR_LEVEL_MAX];
        uint64_t v;
    };
} istr_t;

inline uint8_t istr_dptr_internal_len(istr_dptr_t v);
inline bool istr_dptr_internal_data(istr_dptr_t v);
inline size_t istr_dptr_len(istr_dptr_t v);
void istr_dptr_print(istr_dptr_t v);
void istr_dptr_repr(istr_dptr_t v);
int istr_dptr_strcmp(istr_dptr_t a, const char* b, size_t n);
int istr_dptr_cmp(istr_dptr_t a, istr_dptr_t b);
inline istr_dptr_t create_istr_dptr(const char* str, size_t n);
uint16_t _istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n);
uint16_t istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n);
void ab_print(const char* s, size_t end);
istr_t new_istr(const char* str, size_t n);
void istr_print(istr_t s);
void istr_repr(istr_t s);
inline bool istr_eq(istr_t a, istr_t b);
int istr_cmp(istr_t a, istr_t b);
