/*
* gen_ids.c
 * Gerador simples de lista_ids.txt com N IDs consecutivos.
 *
 * Uso:
 *   ./gen_ids              -> gera 1.000.000 IDs (padrão)
 *   ./gen_ids 1000         -> gera 1000 IDs
 *   ./gen_ids arquivo.txt 100 -> gera arquivo.txt com 100 IDs
 *
 * Compilação:
 *   gcc gen_ids.c -o gen_ids
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

int main(int argc, char *argv[]) {
    const char *filename = "lista_ids.txt";
    long long count = 1000000LL;  // padrão
    if (argc >= 2) count = atoll(argv[1]);
    if (argc >= 3) filename = argv[2];

    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Erro ao criar arquivo");
        return 1;
    }

    for (long long i = 1; i <= count; i++) {
        fprintf(f, "%" PRId64 "\n", (int64_t)i);
    }

    fclose(f);
    printf("Arquivo '%s' criado com %lld IDs.\n", filename, count);
    return 0;
}
