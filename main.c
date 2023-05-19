#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gol.h"
#include <pthread.h>
#include <math.h>

typedef struct Parametros {
    int final;

    int steps;
    int size;
    stats_t stats_step;

} Parametros;

pthread_mutex_t mutex;
stats_t stats_total = {0, 0, 0, 0};
cell_t **prev, **next, **tmp;

int counter = 0;

void* Operacao(void* argumento) {
    Parametros* arg = (Parametros*) argumento;
    int size = arg->size;
    int final = arg->final;
    stats_t stats_step = arg->stats_step;
    // printf("\nFINAL: %d\n", final);

    for (int i = 0; i < final; i++)
        {   
            stats_step = play(prev, next, size);
            
            pthread_mutex_lock(&mutex);
            stats_total.borns += stats_step.borns;
            stats_total.survivals += stats_step.survivals;
            stats_total.loneliness += stats_step.loneliness;
            stats_total.overcrowding += stats_step.overcrowding;
            tmp = next;
            next = prev;
            prev = tmp;
            counter += 1;
            pthread_mutex_unlock(&mutex);
   
            #ifdef DEBUG
                    printf("Step %d ----------\n", counter);
                    print_board(next, size);
                    print_stats(stats_total);
            #endif

        }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{   

    int size, steps;
    FILE *f;
    stats_t stats_step = {0, 0, 0, 0};

    if (argc != 3)
    {
        printf("ERRO! Você deve digitar %s <nome do arquivo do tabuleiro>!\n\n", argv[0]);
        return 0;
    }

    if ((f = fopen(argv[1], "r")) == NULL)
    {
        printf("ERRO! O arquivo de tabuleiro '%s' não existe!\n\n", argv[1]);
        return 0;
    }

    pthread_mutex_init(&mutex, NULL);

    fscanf(f, "%d %d", &size, &steps);

    prev = allocate_board(size);
    next = allocate_board(size);

    read_file(f, prev, size);

    fclose(f);

#ifdef DEBUG
    printf("Initial:\n");
    print_board(prev, size);
    print_stats(stats_step);
#endif
    int n_threads = atoi(argv[2]);

    if (n_threads > steps) {
        n_threads = steps;
    }

    pthread_t threads[n_threads];
    Parametros arg[n_threads];
    int check = steps % n_threads;

    for (int i = 0; i < n_threads; i++) {
        arg[i].steps = steps;
        arg[i].size = size;
        arg[i].stats_step = stats_step;
        if (check == 0) {
            arg[i].final = (int) steps/n_threads;
        } else {
            arg[i].final = (int) floor(steps/n_threads);
            if (check > 0) {
                arg[i].final++;
                check--;
            }
        }

        pthread_create(&threads[i], NULL, Operacao, (void*) &arg[i]);
    }

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

#ifdef RESULT
    printf("Final:\n");
    print_board(prev, size);
    print_stats(stats_total);
#endif

    pthread_mutex_destroy(&mutex);

    free_board(prev, size);
    free_board(next, size);
}