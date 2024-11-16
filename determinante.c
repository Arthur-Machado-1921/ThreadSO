#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_THREADS 8

typedef struct {
    double **matriz;
    int n;
    int linha_inicio;
    int linha_fim;
    int pivo;
} ThreadData;

void trocaLinhas(double **matriz, int linha1, int linha2, int n) {
    for (int j = 0; j < n; j++) {
        double aux = matriz[linha1][j];
        matriz[linha1][j] = matriz[linha2][j];
        matriz[linha2][j] = aux;
    }
}

void *eliminacaoGaussiana(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double **matriz = data->matriz;
    int n = data->n;
    int pivo = data->pivo;

    for (int i = data->linha_inicio; i < data->linha_fim; i++) {
        double fator = matriz[i][pivo] / matriz[pivo][pivo];
        for (int j = pivo; j < n; j++) {
            matriz[i][j] -= fator * matriz[pivo][j];
        }
    }
    return NULL;
}

double determinanteGaussiano(double **matriz, int n) {
    double det = 1.0;

    for (int i = 0; i < n; i++) {
        if (matriz[i][i] == 0) {
            bool trocado = false;
            for (int k = i + 1; k < n; k++) {
                if (matriz[k][i] != 0) {
                    trocaLinhas(matriz, i, k, n);
                    det = -det;
                    trocado = true;
                    break;
                }
            }
            if (!trocado) {
                return 0;
            }
        }

        pthread_t threads[MAX_THREADS];
        ThreadData Threads[MAX_THREADS];
        int linhas_por_thread = (n - i - 1) / MAX_THREADS;

        for (int t = 0; t < MAX_THREADS; t++) {
            Threads[t].matriz = matriz;
            Threads[t].n = n;
            Threads[t].pivo = i;
            Threads[t].linha_inicio = i + 1 + t * linhas_por_thread;
            Threads[t].linha_fim = (t == MAX_THREADS - 1) ? n : Threads[t].linha_inicio + linhas_por_thread;
            pthread_create(&threads[t], NULL, eliminacaoGaussiana, &Threads[t]);
        }

        for (int t = 0; t < MAX_THREADS; t++) {
            pthread_join(threads[t], NULL);
        }

        det *= matriz[i][i];
    }

    return det;
}

double **lerMatrizArquivo(const char *nomeArquivo, int *n) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", nomeArquivo);
        return NULL;
    }

    if (fscanf(arquivo, "%d", n) != 1 || *n <= 0) {
        printf("Erro ao ler o tamanho da matriz ou tamanho inválido.\n");
        fclose(arquivo);
        return NULL;
    }

    double **matriz = (double **)malloc(*n * sizeof(double *));
    if (!matriz) {
        printf("Erro ao alocar memória para a matriz.\n");
        fclose(arquivo);
        return NULL;
    }
    for (int i = 0; i < *n; i++) {
        matriz[i] = (double *)malloc(*n * sizeof(double));
        if (!matriz[i]) {
            printf("Erro ao alocar memória para a linha %d.\n", i);
            fclose(arquivo);
            return NULL;
        }
    }

    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *n; j++) {
            if (fscanf(arquivo, "%lf", &matriz[i][j]) != 1) {
                printf("Erro ao ler elemento da matriz (%d, %d).\n", i, j);
                fclose(arquivo);
            }
        }
    }

    fclose(arquivo);
    return matriz;
}

int main() {
    int n;
    double **matriz = lerMatrizArquivo("matriz.txt", &n);

    if (!matriz) {
        return 1;
    }

    double det = determinanteGaussiano(matriz, n);
    printf("Determinante: %lf\n", det);

    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);

    return 0;
}
