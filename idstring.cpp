
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

inline size_t istr_dptr_len(istr_dptr_t v) {
    assert(istr_dptr_internal_len(v) > 0 || v.external != NULL);
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


int istr_dptr_strcmp(istr_dptr_t a, const char* b) {
    if (istr_dptr_internal_len(a) == 0) {
        return strcmp(a.external, b);
    } else {
        return strncmp(a.internal, b, ISTR_DPTR_INTERNAL_MAXLEN);
    }
}

/*
int istr_dptr_cmp(istr_dptr_t a, istr_dptr_t b) {
    if (b.length != 0) {
        return istr_dptr_strcmp(a, b.internal, b.length);
    } else {
        return istr_dptr_strcmp(a, b.external, strlen(b.external));
    }
}
*/

inline istr_dptr_t create_istr_dptr(const char* str, size_t n) {
    istr_dptr_t v;
    if (n < ISTR_DPTR_INTERNAL_MAXLEN) {
        strncpy(v.internal, str, std::min(n, ISTR_DPTR_INTERNAL_MAXLEN));
        v.length = n;
    } else {
        v.external = strndup(str, n);
    }
    return v;
}

// id_strings_table - Stores 65,535 id_string_data_ptr values
// 65,535 elements * sizeof(id_string_data_ptr) / 1024 / 1024
// == 65,535 * 8 / 1024 / 1024
// == 0.5 megabytes

// 3 levels
typedef struct {
    uint16_t used = 1;
    istr_dptr_t strings[UINT16_MAX];
} istr_table_t;


uint16_t _istr_table_add_str(istr_table_t* table, const char* str, size_t n) {
    uint16_t i = 1;
    // Check if the string is already found at an index in the table
    for(; i < table->used; i++) {
        assert(table->strings[i].external != NULL);
        assert(istr_dptr_len(table->strings[i]) > 0);
        if (istr_dptr_strcmp(table->strings[i], str) == 0) {
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

uint16_t istr_table_add_str(istr_table_t* table, const char* str, size_t n) {
    assert(n > 0);
    uint16_t r = _istr_table_add_str(table, str, n);
    assert(r > 0);
    return r;
}

// id_string -- Collection of indexes into the istr_table structures.
#define ISTR_MAX 6

typedef struct { uint16_t istr_table_idx[ISTR_MAX]; } istr_t;

istr_table_t table_lvl[5];

void ab_print(const char* s, size_t end) {
    putchar('\'');
    for (size_t i = 0; i < end; i++) {
        putchar(s[i]);
    }
    putchar('\'');
}

istr_t* new_istr(const char* str, size_t n) {
    if (n == 0) {
        n = strlen(str);
    }

    const char* str_start = str;
    const char* str_end = str+n;

    const char* dot0 = index(str_start, '/');
    if (dot0 == NULL) {
        dot0 = str_end;
    } else {
        assert(dot0 < str_end);
        dot0++;
    }

    const char* dot1 = index(dot0, '/');
    if (dot1 == NULL) {
        dot1 = str_end;
    } else {
        assert(dot1 < str_end);
        dot1++;
    }

    const char* dot2 = index(dot1, '/');
    if (dot2 == NULL) {
        dot2 = str_end;
    } else {
        assert(dot2 < str_end);
        dot2++;
    }

    const char* dotn = rindex(dot2, '/');
    if (dotn == NULL) {
        dotn = str_end;
    } else {
        assert(dotn < str_end);
        dotn++;
    }

    size_t slen0 = dot0-str_start;
    size_t slen1 = dot1-dot0;
    size_t slen2 = dot2-dot1;
    size_t slen3 = dotn-dot2;
    size_t slen4 = str_end-dotn;

    istr_t* nstr = (istr_t*)malloc(ISTR_MAX*sizeof(uint16_t));
    uint8_t end = 0;
    if (slen0 > 0) {
        nstr->istr_table_idx[0] = istr_table_add_str(&(table_lvl[0]), str_start, slen0);

        printf("0-%d: (%p %d %d) - ", end, str_start, slen0, (int)(nstr->istr_table_idx[0]));
        ab_print(str_start, slen0);
        putchar(' ');
        putchar('"');
        istr_dptr_print(table_lvl[0].strings[nstr->istr_table_idx[0]]);
        putchar('"');
        printf("\n");

        end++;
    }

    if (slen1 > 0) {
        nstr->istr_table_idx[1] = istr_table_add_str(&(table_lvl[1]), dot0, slen1);

        printf("1-%d: (%p %d %d) - ", end, dot0, slen1, (int)(nstr->istr_table_idx[1]));
        ab_print(dot0, slen1);
        putchar(' ');
        putchar('"');
        istr_dptr_print(table_lvl[1].strings[nstr->istr_table_idx[1]]);
        putchar('"');
        printf("\n");

        end++;
    }

    if (slen2 > 0) {
        nstr->istr_table_idx[2] = istr_table_add_str(&(table_lvl[2]), dot1, slen2);

        printf("2-%d: (%p %d %d) - ", end, dot1, slen2, (int)(nstr->istr_table_idx[2]));
        ab_print(dot1, slen2);
        printf("\n");

        end++;
    }

    if (slen3 > 0) {
        nstr->istr_table_idx[3] = istr_table_add_str(&(table_lvl[3]), dot2, slen3);

        printf("3-%d: (%p %d %d) - ", end, dot2, slen3, (int)(nstr->istr_table_idx[3]));
        ab_print(dot2, slen3);
        printf("\n");

        end++;
    }

    if (slen4 > 0) {
        nstr->istr_table_idx[4] = istr_table_add_str(&(table_lvl[4]), dotn, slen4);

        printf("4-%d: (%p %d %d) - ", end, dotn, slen4, (int)(nstr->istr_table_idx[4]));
        ab_print(dotn, slen4);
        printf("\n");

        end++;
    }

    printf("--\n");
    nstr->istr_table_idx[end] = 0;
    return nstr;
}

void istr_print(istr_t* s) {
    size_t i = 0;
    while((s->istr_table_idx[i]) != 0) {
        istr_dptr_print(table_lvl[i].strings[s->istr_table_idx[i]]);
        i++;
    }
}

int main(int argc, char **argv) {

    istr_t* strings[64];

    size_t i = 0;
    strings[i++] = new_istr("A1", 0);
    strings[i++] = new_istr("A1/", 0);
    strings[i++] = new_istr("A1/B1", 0);
    strings[i++] = new_istr("A1/B1/C", 0);
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
    strings[i++] = NULL;

    for(size_t i = 0; strings[i] != NULL; i++) {
        printf("%d: '", i);
        istr_print(strings[i]);
        printf("'\n");
    }

    return 0;
}
