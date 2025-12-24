#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

/*
    *** STRUCT ***
*/
struct Message
{
    int thread_id; // ID del hilo
    int num_threads; // Cantidad de hilos
    int f, c1, c2; // Dimensiones de las matrices A y B
};


int **B; // Matriz B definida de forma global para que cada nodo acceda a ella
MPI_Status status;

void Process(int **A, int **C)
{

}

int main(int argc, char **argv)
{
    
    
    MPI_Init(&argc, &argv);


    return 0;
}