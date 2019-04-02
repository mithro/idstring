
#include <algorithm>    // std::min

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

#define ISTR_DPTR_INTERNAL_MAXLEN (sizeof(char*))
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

int istr_dptr_cmp(istr_dptr_t a, istr_dptr_t b) {
    if (b.length != 0) {
        return istr_dptr_strcmp(a, b.internal, b.length);
    } else {
        return istr_dptr_strcmp(a, b.external, strlen(b.external));
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
    uint16_t used = 1; // We leave the zero index empty.
    istr_dptr_t strings[UINT16_MAX];
} istr_dptr_table_t;

uint16_t _istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n) {
    uint16_t i = 1;
    // Check if the string is already found at an index in the table
    for(; i < table->used; i++) {
        assert(table->strings[i].external != NULL);
        assert(istr_dptr_len(table->strings[i]) > 0);
        if (istr_dptr_strcmp(table->strings[i], str, n) == 0) {
            return i;
        }
    }
    assert(i < UINT16_MAX);
    table->used += 1;
    // Otherwise add the string
    table->strings[i] = create_istr_dptr(str, n);
    assert(table->strings[i].external != NULL);
    assert(istr_dptr_len(table->strings[i]) > 0);
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

#define ISTR_DELIMITER '/'

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

    for (size_t i = 0; i < ISTR_LEVEL_MAX; i++) {
        printf("%d: ", i);
        ab_print(del[i], slen[i]);
        printf("\n");
    }
    printf("--\n");

    istr_t nstr = { 0 }; // = (istr_t*)malloc(ISTR_LEVEL_MAX*sizeof(uint16_t));
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
        istr_dptr_repr(istr_dptr_tables[i].strings[s.dptr_idx[i]]);
        i++;
        if ((s.dptr_idx[i]) == 0)
            break;
        putchar(' ');
        putchar(ISTR_DELIMITER);
        putchar(' ');
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


int main(int argc, char **argv) {

    istr_t strings[64];

    size_t i = 0;
    strings[i++] = new_istr("A1", 0);
    assert(istr_dptr_tables[0].used == 2);
    assert(istr_dptr_tables[1].used == 1);
    assert(istr_dptr_tables[2].used == 1);
    assert(istr_dptr_tables[3].used == 1);
    strings[i++] = new_istr("A1/", 0);
    assert(istr_dptr_tables[0].used == 3);
    assert(istr_dptr_tables[1].used == 1);
    assert(istr_dptr_tables[2].used == 1);
    assert(istr_dptr_tables[3].used == 1);
    strings[i++] = new_istr("A1/B1", 0);
    assert(istr_dptr_tables[0].used == 3);
    assert(istr_dptr_tables[1].used == 2);
    assert(istr_dptr_tables[2].used == 1);
    assert(istr_dptr_tables[3].used == 1);
    //strings[i++] = new_istr("A1//B1", 0);
    strings[i++] = new_istr("A1/B1/C", 0);
    assert(istr_dptr_tables[0].used == 3);
    assert(istr_dptr_tables[1].used == 2);
    assert(istr_dptr_tables[2].used == 2);
    assert(istr_dptr_tables[3].used == 1);
    strings[i++] = new_istr("A1/B1/C/D", 0);
    strings[i++] = new_istr("A1/B2/C/D/E", 0);
    strings[i++] = new_istr("A1/B3/C/D/E/F", 0);
    strings[i++] = new_istr("A2/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAAAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAAAAAA/B1/C/D/E/F", 0);
    strings[i++] = new_istr("AAAAAAAAAA/B1/C/D/E/F", 0);

    strings[i++] = new_istr("CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0", 0);
    strings[i++] = new_istr("CMT_TOP_L_UPPER_T_X106Y96/CMT_TOP_LH2_0", 0);
    strings[i++] = new_istr("DSP_L_X34Y85/DSP_LH10_2", 0);
    strings[i++] = new_istr("HCLK_R_X110Y78/HCLK_LV5", 0);
    strings[i++] = new_istr("HCLK_R_X64Y130/HCLK_NN2BEG2", 0);
    strings[i++] = new_istr("INT_INTERFACE_L_X34Y87/INT_INTERFACE_LH10", 0);
    strings[i++] = new_istr("INT_INTERFACE_L_X42Y87/INT_INTERFACE_LH2", 0);
    strings[i++] = new_istr("INT_INTERFACE_R_X23Y126/INT_INTERFACE_WW2END1", 0);
    strings[i++] = new_istr("INT_L_X24Y126/WW2A1", 0);
    strings[i++] = new_istr("INT_L_X26Y123/WW4C3", 0);
    strings[i++] = new_istr("INT_L_X30Y123/WW4A3", 0);
    strings[i++] = new_istr("INT_L_X32Y87/LH11", 0);
    strings[i++] = new_istr("INT_L_X34Y87/LH9", 0);
    strings[i++] = new_istr("INT_L_X36Y87/LH7", 0);
    strings[i++] = new_istr("INT_L_X38Y87/LH5", 0);
    strings[i++] = new_istr("INT_L_X40Y87/LH3", 0);
    strings[i++].v = 0;

    printf("\n\n=== Tables ===\n");
    for(size_t j = 0; j < ISTR_LEVEL_MAX; j++) {
        printf("--- %d level (%d entries)---\n", j, istr_dptr_tables[j].used);
        for(size_t k = 1; k < istr_dptr_tables[j].used; k++) {
            printf("[%04d] = \"", k);
            istr_dptr_print(istr_dptr_tables[j].strings[k]);
            printf("\";\n");
        }
    }

    printf("\n\n=== Strings ===\n");
    for(size_t j = 0; strings[j].v != 0; j++) {
        printf("% 4d: '", j);
        istr_print(strings[j]);
        printf(";\n      ");
        istr_repr(strings[j]);
        printf("\n");
    }

    istr_t cmp1 = new_istr("A1/B1/C/D", 0);
    istr_t cmp2 = new_istr("CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0", 0);
    istr_t cmp3 = new_istr("INT_L_X40Y87/LH3", 0);

    printf("\n\n=== Comparisons ===\n");
    for(size_t j = 0; strings[j].v != 0; j++) {
        istr_t a = strings[j];
        printf("% 4d: '", j);
        istr_print(a);
        printf("'");
        if (istr_cmp(a, cmp1) == 0) {
            printf(" == 'A1/B1/C/D'");
        }
        if (istr_cmp(a, cmp2) == 0) {
            printf(" == 'CMT_FIFO_L_X107Y97/CMT_FIFO_LH2_0'");
        }
        if (istr_cmp(a, cmp3) == 0) {
            printf(" == 'INT_L_X40Y87/LH3'");
        }
        printf("\n");
    }
    return 0;
}
