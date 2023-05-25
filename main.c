#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gol.h"
#include <pthread.h>
#include <math.h>
#include <sys/sysinfo.h>

typedef struct Parametros {
    int id;

    int comeco;
    int final;
    int size;

    stats_t stats_step;
    cell_t **prev, **next;

} Parametros;

#ifdef DEBUG
// int counter = 0;
#endif

void* Operacao(void* argumento) {
    Parametros* arg = (Parametros*) argumento;

    arg->stats_step = play(arg->prev, arg->next, arg->size, arg->comeco, arg->final);

    #ifdef DEBUG
        // printf("Step %d [THREAD %d] ----------\n", counter, arg->id);
        // print_board(arg->next, arg->size);
        // print_stats(arg->stats_step);
    #endif

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{   

    int size, steps;
    FILE *f;
    cell_t **prev, **next, **tmp;
    stats_t stats_total = {0, 0, 0, 0};
    stats_t stats_step = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro> <numero de threads>!\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    fscanf(f, "%d %d", &size, &steps);

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    // printf("Initial:\n");
    // print_board(prev, size);
    // print_stats(stats_step);
#endif
    int n_threads = atoi(argv[2]);

    if (n_threads > size) {
        n_threads = size;
    }

    pthread_t threads[n_threads];
    Parametros arg[n_threads];


    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < n_threads; j++) {
            arg[j].id = j;
            arg[j].prev = prev;
            arg[j].next = next;
            arg[j].size = size;
            arg[j].stats_step = stats_step;

            arg[j].comeco = (j*size)/n_threads;
            arg[j].final = ((j+1)*size)/n_threads;


            pthread_create(&threads[j], NULL, Operacao, (void*) &arg[j]);
        }

        for (int j = 0; j < n_threads; j++) {
            pthread_join(threads[j], NULL);
            stats_total.borns += arg[j].stats_step.borns;
            stats_total.survivals += arg[j].stats_step.survivals;
            stats_total.loneliness += arg[j].stats_step.loneliness;
            stats_total.overcrowding += arg[j].stats_step.overcrowding;
        }

        tmp = next;
        next = prev;
        prev = tmp;

        #ifdef DEBUG
        // counter++;
        #endif
    }

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    free_board(prev, size);
    free_board(next, size);
}