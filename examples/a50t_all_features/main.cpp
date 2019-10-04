#include "idstring.h"

#include <stdio.h>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    std::vector<istr_t> strings;
    std::string W;
    ISTR_DELIMITER = '.';
    FILE *F = fopen("a50t_all_features.fasm", "r");
    char line[1024];
    while(fgets(line, 1024, F) != NULL){
        strings.push_back(new_istr(line, strlen(line)-1));
    }

    printf("\n\n=== Tables ===\n");
    for(size_t j = 0; j < ISTR_LEVEL_MAX; j++) {
        int x = 0;
        printf("--- %d level (%d entries)---\n", j, istr_dptr_tables[j].used);
        for(size_t k = 1; k < UINT16_MAX; k++) {
            if(istr_dptr_tables[j].strings[k].external == NULL) continue;
            printf("[%04d] = \"", k);
            istr_dptr_print(istr_dptr_tables[j].strings[k]);
            if(istr_dptr_internal_len(istr_dptr_tables[j].strings[k]))
            	x++;
            printf("\";\n");
        }
        printf("%d of the %d strings are stored in the pointer.\n", x, istr_dptr_tables[j].used-1);
    }
    return 0;
}
