#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#define NUM_THREADS 5
#define MAX_LINE 64

FILE *arquivo_ids;
FILE *arquivo_logs;
pthread_mutex_t mutex_ids;
pthread_mutex_t mutex_logs;

void gerar_json_simulado(char *saida, int id) {
    const char *nomes[] = {"Lucas", "Maria", "João", "Ana", "Rafa", "Paulo"};
    double valor = (rand() % 10000) / 100.0;
    const char *nome = nomes[rand() % 6];
    sprintf(saida, "{\"id\":%d,\"status\":\"ok\",\"valor\":{\"nome\":\"%s\",\"valor\":%.2f}}",
            id, nome, valor);
}

void *processar_ids(void *tid) {
    intptr_t thread_id = (intptr_t) tid;
    char linha[MAX_LINE];
    clock_t inicio = clock();

    printf("Thread %lld iniciada\n", (long long) thread_id);

    while (1) {
        pthread_mutex_lock(&mutex_ids);
        if (fgets(linha, sizeof(linha), arquivo_ids) == NULL) {
            pthread_mutex_unlock(&mutex_ids);
            break;
        }
        pthread_mutex_unlock(&mutex_ids);

        int id = atoi(linha);
        char resposta_json[256];
        gerar_json_simulado(resposta_json, id);

        time_t agora = time(NULL);
        struct tm *t = localtime(&agora);
        char data[64];
        strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", t);

        double tempo_exec = (double) (clock() - inicio) / CLOCKS_PER_SEC;

        pthread_mutex_lock(&mutex_logs);
        fprintf(arquivo_logs,
                "Thread %lld, ID %d, Tempo %.3f s, Resposta %s, Data %s\n",
                (long long) thread_id, id, tempo_exec, resposta_json, data);
        fflush(arquivo_logs);
        pthread_mutex_unlock(&mutex_logs);
    }

    printf("Thread %lld finalizada\n", (long long) thread_id);
    return NULL;
}

int executar_P1() {
    srand((unsigned int) time(NULL));

    arquivo_ids = fopen("lista_ids.txt", "r");
    arquivo_logs = fopen("logs", "w");

    if (!arquivo_ids || !arquivo_logs) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    pthread_mutex_init(&mutex_ids, NULL);
    pthread_mutex_init(&mutex_logs, NULL);

    pthread_t threads[NUM_THREADS];
    for (intptr_t i = 1; i <= NUM_THREADS; i++) {
        printf("Criando Thread %lld\n", (long long) i);
        pthread_create(&threads[i - 1], NULL, processar_ids, (void *) (intptr_t) i);
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&mutex_ids);
    pthread_mutex_destroy(&mutex_logs);
    fclose(arquivo_ids);
    fclose(arquivo_logs);

    printf(" Processo P1 finalizado com sucesso.\n");
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "child") == 0) {
        printf("Processo P1 iniciado (PID: %lu)\n", GetCurrentProcessId());
        return executar_P1();
    }

    printf("Processo P0 iniciado (PID: %lu)\n", GetCurrentProcessId());

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char comando[512];
    GetModuleFileName(NULL, comando, sizeof(comando));
    strcat(comando, " child");

    if (!CreateProcess(
        NULL,
        comando,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        printf("Erro ao criar processo filho (P1).\n");
        return 1;
    }

    printf("Processo P1 criado (PID: %lu). Aguardando término...\n", pi.dwProcessId);
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode == 0) {
        FILE *f = fopen("logs", "r");
        if (f) {
            printf(" Processamento concluído com sucesso. Arquivo 'logs' encontrado.\n");
            fclose(f);
        } else {
            printf(" Erro: arquivo de logs não encontrado.\n");
        }
    } else {
        printf(" Erro: Processo P1 terminou com código %lu.\n", exitCode);
    }

    return 0;
}
