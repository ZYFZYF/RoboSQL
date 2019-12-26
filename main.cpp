#include <cstdio>

extern FILE *yyin;

extern int yyparse();

int main() {
    const char test_sql[] = "../test/insert.sql";
    yyin = fopen(test_sql, "rb");
    if (yyin == nullptr) {
        printf("Open file  %s\n failed", test_sql);
        return 0;
    }

    while (!feof(yyin)) {
        yyparse();
    }

    fclose(yyin);
    yyin = nullptr;

    return 0;
}