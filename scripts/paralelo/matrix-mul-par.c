#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mpi.h"

#define SILENT 0
#define VERBOSE 1
#define MASTER 0
#define TAG_SENDTASK 1
#define TAG_TASKREADY 2
#define TAG_FINISH 3

MPI_Status status;

/*
    *** FUNCIONES ***
*/

// readData: lee el archivo de entrada y llena las matrices A y B.
//           Las matrices se llenan por filas
void genData(int *f, int *c1, int *c2, float ***A, float ***B)
{
    int i, j;

    scanf("%d", f);
    scanf("%d", c1);
    scanf("%d", c2);

    // Inicialización de matriz A
    *A = (float **) calloc(*f, sizeof(float *)); // Asignación de memoria para filas de A
    for(i = 0; i < *f; i = i + 1)
    {
        (*A)[i] = (float *) calloc(*c1, sizeof(float)); // Asignación de memoria para las columnas de A
    }

    // Llenar matriz A
    for(i = 0; i < *f; i = i + 1)
    {
        for(j = 0; j < *c1; j = j + 1)
        {
            (*A)[i][j] = 1.0;
        }
    }


    // Inicialización de matriz B
    *B = (float **) calloc(*c1, sizeof(float *)); // Asignación de memoria para filas de B
    for(i = 0; i < *c1; i = i + 1)
    {
        (*B)[i] = (float *) calloc(*c2, sizeof(float)); // Asignación de memoria para las columnas de B
    }

    // Llenar matriz B
    for(i = 0; i < *c1; i = i + 1)
    {
        for(j = 0; j < *c2; j = j + 1)
        {
            (*B)[i][j] = 2.0;
        }
    }
}

// printMatrix: imprime en pantalla la matriz
void printMatrix(float **A, int rows, int columns)
{
    int i, j;

    for(i = 0; i < rows; i = i + 1)
    {
        for(j = 0; j < columns; j = j + 1)
        {
            printf("%.1f\t", A[i][j]);
        }
        printf("\n");
    }
}

// freeMatrix: libera la memoria de la matriz
void freeMatrix(float **A, int rows)
{
    int i, j;

    for(i = 0; i < rows; i = i + 1)
    {
        free(A[i]);
    }

    free(A);
}

// Process: multiplicación de matrices A y B
float **Process(float **A, float **B, int f, int c1, int c2)
{
    float **C; // Matriz resultante de la multiplicación
    int i, j, k;

    // Inicialización de matriz C
    C = (float **) calloc(f, sizeof(float *));
    for(i = 0; i < f; i = i + 1)
    {
        C[i] = (float *) calloc(c2, sizeof(float));
    }

    // Proceso de multiplicación
    for(i = 0; i < f; i = i + 1)
    {
        for(j = 0; j < c2; j = j + 1)
        {
            C[i][j] = 0;

            for(k = 0; k < c1; k = k + 1)
            {
                C[i][j] = C[i][j] + (A[i][k] * B[k][j]);
            }
        }
    }

    return C;
}

int main(int argc, char **argv)
{
    
    
    MPI_Init(&argc, &argv);


    return 0;
}