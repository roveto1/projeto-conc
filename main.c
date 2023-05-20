#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gol.h"
#include <pthread.h>
#include <math.h>

typedef struct Parametros {
    int id;

    int final;
    int comeco;
    int size;
    stats_t *stats_total;
    stats_t stats_step;
    cell_t **prev, **next;

} Parametros;

pthread_mutex_t mutex;

#ifdef DEBUG
int counter = 0;
#endif

void* Operacao(void* argumento) {
    Parametros* arg = (Parametros*) argumento;

    arg->stats_step = play(arg->prev, arg->next, arg->size, arg->comeco, arg->final);
    
    pthread_mutex_lock(&mutex);
    arg->stats_total->borns += arg->stats_step.borns;
    arg->stats_total->survivals += arg->stats_step.survivals;
    arg->stats_total->loneliness += arg->stats_step.loneliness;
    arg->stats_total->overcrowding += arg->stats_step.overcrowding;
    pthread_mutex_unlock(&mutex);

    #ifdef DEBUG
        printf("Step %d [THREAD %d] ----------\n", counter, arg->id);
        print_board(arg->next, arg->size);
        print_stats(*arg->stats_total);
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

    pthread_t threads[n_threads];
    Parametros arg[n_threads];

    for (int i = 0; i < steps; i++) {
        for (int j = 0; j < n_threads; j++) {
            arg[j].id = j;
            arg[j].prev = prev;
            arg[j].next = next;
            arg[j].size = size;
            arg[j].stats_step = stats_step;
            arg[j].stats_total = &stats_total;

            arg[j].comeco = (int) ceil((j*size)/n_threads);
            arg[j].final = (int) ceil((((j+1)*size)/n_threads));

            pthread_create(&threads[j], NULL, Operacao, (void*) &arg[j]);
        }

        for (int i = 0; i < n_threads; i++) {
            pthread_join(threads[i], NULL);
            
        }

        tmp = next;
        next = prev;
        prev = tmp;

        #ifdef DEBUG
        counter++;
        #endif
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