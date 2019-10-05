#include <algorithm>    // std::min
#include <bits/hash_bytes.h>
#include <endian.h>

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

#define ISTR_DPTR_INTERNAL_MAXLEN (sizeof(char*) - sizeof(uint8_t))
#define ISTR_DPTR_MASK 0b111

//#if INTPTR_MAX == 4294967295
//#define ISTR_DPTR_MASK 0b11
//#elif INTPTR_MAX == 18446744073709551615
//#endif


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
static_assert(sizeof(istr_dptr_t) == sizeof(char*), "Size mismatch");

inline uint8_t istr_dptr_internal_len(istr_dptr_t v) {
    return v.length & ISTR_DPTR_MASK;
}

inline bool istr_dptr_internal_data(istr_dptr_t v) {
    return istr_dptr_internal_len > 0;
}

inline size_t istr_dptr_len(istr_dptr_t v) {
    assert(istr_dptr_internal_data(v) || v.external != NULL);
    return istr_dptr_internal_len(v) || strlen(v.external);
}

void istr_dptr_print(istr_dptr_t v) {
    size_t l = istr_dptr_internal_len(v);
    if (l == 0) {
        printf("%s", v.external);
    } else {
        for(size_t i = 0; i < l; i++) {
            putchar(v.internal[i]);
        }
    }
}

void istr_dptr_repr(istr_dptr_t v) {
    size_t l = istr_dptr_internal_len(v);
    if (l == 0) {
        printf("e\"%s\"", v.external);
    } else {
        putchar('i');
        putchar('"');
        for(size_t i = 0; i < l; i++) {
            putchar(v.internal[i]);
        }
        putchar('"');
    }
}

int istr_dptr_strcmp(istr_dptr_t a, const char* b, size_t n) {
    if (istr_dptr_internal_len(a) == 0) {
        return strncmp(a.external, b, n);
    } else {
        return strncmp(a.internal, b, std::min(ISTR_DPTR_INTERNAL_MAXLEN, n));
    }
}

/* If a string is stored inside the pointer and the other is not, there is no way
 * they can be equal. In that case, we run strncmp through the smaller(internal) string.
 * If they are equal to that point, we "imagine" the internal string is zero-terminated
 * and return the difference between the characters at the end of the internal string
 * which is just the external string's character at that point. */
int istr_dptr_cmp(istr_dptr_t a, istr_dptr_t b){
    int na, nb, x;
    na = istr_dptr_internal_len(a);
    nb = istr_dptr_internal_len(b);
    if(na == 0 && nb == 0){
        return strcmp(a.external, b.external);
    }else if(na == 0 && nb != 0){
        x = strncmp(a.external, b.internal, nb);
        return x != 0 ? x : a.external[nb];
    }else if(na != 0 && nb == 0){
        x = strncmp(a.internal, b.external, na);
        return x != 0 ? x : -b.external[na];
    }else{
        return strncmp(a.internal, b.internal, ISTR_DPTR_INTERNAL_MAXLEN);
    }
}

inline istr_dptr_t create_istr_dptr(const char* str, size_t n) {
    istr_dptr_t v = { .external = NULL };
    if (n < ISTR_DPTR_INTERNAL_MAXLEN) {
        memcpy(v.internal, str, n);
        v.length = n;
    } else {
        v.external = strndup(str, n);
    }
    assert(istr_dptr_strcmp(v, str, n) == 0);
    return v;
}

// id_strings_table - Stores 65,535 id_string_data_ptr values
// 65,535 elements * sizeof(id_string_data_ptr) / 1024 / 1024
// == 65,535 * 8 / 1024 / 1024
// == 0.5 megabytes
typedef struct {
    uint16_t used = 1;
    istr_dptr_t strings[UINT16_MAX+1];
} istr_dptr_table_t;

uint16_t _istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n) {
    uint16_t i = std::max((size_t)1, std::_Hash_bytes(str, n, 0));
    // Probe the index found by hashing.
    // If an entry is found, check if it's our string and look for empty space if it isn't.
    while(table->strings[i].external != NULL){
        if (istr_dptr_strcmp(table->strings[i], str, n) == 0) {
            return i;
        }
        i = std::max(1, i+1);
    }
    // Otherwise add the string.
    table->strings[i] = create_istr_dptr(str, n);
    assert(table->strings[i].external != NULL);
    assert(istr_dptr_len(table->strings[i]) > 0);
    table->used += 1;
    return i;
}

uint16_t istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n) {
    assert(n > 0);
    uint16_t r = _istr_dptr_table_add_str(table, str, n);
    assert(r > 0);
    return r;
}

// id_string -- Collection of indexes into the istr_dptr_table structures.
// sizeof(uint64_t)/sizeof(uint16_t)
// 8/2 == 4
#define ISTR_LEVEL_MAX 4
static char ISTR_DELIMITER = '.';

// Tables for storing string data pointers, one for each level.
istr_dptr_table_t istr_dptr_tables[ISTR_LEVEL_MAX];

// String structure == 8bytes == uint64_t
// Array of uint16_t indexes into istr_dptr_tables
typedef struct {
    union {
        uint16_t dptr_idx[ISTR_LEVEL_MAX];
        uint64_t v;
    };
} istr_t;

void ab_print(const char* s, size_t end) {
    putchar('\'');
    for (size_t i = 0; i < end; i++) {
        putchar(s[i]);
    }
    putchar('\'');
}

istr_t new_istr(const char* str, size_t n) {
    if (n == 0) {
        n = strlen(str);
    }

    const char* str_start = str;
    const char* str_end = str+n;

    const char* del[ISTR_LEVEL_MAX+1];
    size_t j = 0;
    del[j++] = str_start;
    assert(j == 1);

    while (j < ISTR_LEVEL_MAX-1) {
        del[j] = index(del[j-1], ISTR_DELIMITER);
        if (del[j] == NULL) {
            break;
        } else {
            del[j]++;
            j++;
        }
    }

    del[j] = rindex(del[j-1], ISTR_DELIMITER);
    if(del[j] != NULL) {
        del[j]++;
        j++;
    }

    while (j < ISTR_LEVEL_MAX+1) {
        del[j] = str_end;
        j++;
    }
    assert(ISTR_LEVEL_MAX+1 == j);

    size_t slen[ISTR_LEVEL_MAX];
    for (size_t i = 0; i < ISTR_LEVEL_MAX; i++) {
        slen[i] = del[i+1]-del[i];
        if (del[i+1] != str_end) {
            slen[i]--;
        }
    }

    istr_t nstr = { 0 };
    for (size_t i = 0; i < ISTR_LEVEL_MAX; i++) {
        if (slen[i] > 0) {
            nstr.dptr_idx[i] = istr_dptr_table_add_str(&(istr_dptr_tables[i]), del[i], slen[i]);
        } else {
            nstr.dptr_idx[i] = 0;
        }
    }
    return nstr;
}

void istr_print(istr_t s) {
    size_t i = 0;
    while(i < ISTR_LEVEL_MAX) {
        if (i > 0) {
            putchar(ISTR_DELIMITER);
        }
        istr_dptr_print(istr_dptr_tables[i].strings[s.dptr_idx[i]]);
        i++;
        if ((s.dptr_idx[i]) == 0)
            break;
    }
}

void istr_repr(istr_t s) {
    size_t i = 0;
    while(i < ISTR_LEVEL_MAX) {
        if (i > 0) {
            putchar(' ');
            putchar(ISTR_DELIMITER);
            putchar(' ');
        }
        istr_dptr_repr(istr_dptr_tables[i].strings[s.dptr_idx[i]]);
        i++;
        if ((s.dptr_idx[i]) == 0)
            break;
    }
}

inline bool istr_eq(istr_t a, istr_t b) {
    for(size_t i = 0; i < ISTR_LEVEL_MAX; i++) {
        if (a.dptr_idx[i] != b.dptr_idx[i]) {
            return false;
        }
    }
    return true;
}

int istr_cmp(istr_t a, istr_t b) {
    for(size_t i = 0; i < ISTR_LEVEL_MAX; i++) {
        uint16_t idx_a = a.dptr_idx[i];
        uint16_t idx_b = b.dptr_idx[i];
        if (idx_a == idx_b)
            continue;
        istr_dptr_t dptr_a = istr_dptr_tables[i].strings[idx_a];
        istr_dptr_t dptr_b = istr_dptr_tables[i].strings[idx_b];
        // FIXME: Make this work...
        if (idx_a == 0) {
            return -1;
        }
        if (idx_b == 0) {
            return +1;
        }
        return istr_dptr_cmp(dptr_a, dptr_b);
    }
    return 0;
}
