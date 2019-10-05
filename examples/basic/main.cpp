#include "idstring.h"

int main(int argc, char **argv) {
    assert(sizeof(istr_dptr_t) == sizeof(char*));  // The promise

    istr_t strings[64];
    ISTR_DELIMITER = '/';

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
        for(size_t k = 1; k < UINT16_MAX; k++) {
            if(istr_dptr_tables[j].strings[k].external == NULL) continue;
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
