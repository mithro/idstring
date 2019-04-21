#include "idstring.h"

istr_dptr_table_t istr_dptr_tables[ISTR_LEVEL_MAX];

void istr_set_delimiter(char x){
	ISTR_DELIMITER = x;
}

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

uint16_t _istr_dptr_table_add_str(istr_dptr_table_t* table, const char* str, size_t n) {
    uint32_t i = 1;
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
