#include <stdio.h>
#include <stdlib.h>

#define SILENT 0
#define VERBOSE 1
#define FALSE 0
#define TRUE 1

void Usage(char *message)
{
    printf("Usage: %s < datafile.txt", message);
}

// readData: lee el archivo de entrada y llena las matrices A y B.
//           Las matrices se llenan por filas
void readData(int *f, int *c1, int *c2, int ***A, int ***B)
{
    int i, j;

    scanf("%d", f);
    scanf("%d", c1);
    scanf("%d", c2);

    // Inicialización de matriz A
    *A = (int **) calloc(*f, sizeof(int *)); // Asignación de memoria para filas de A
    for(i = 0; i < *f; i = i + 1)
    {
        (*A)[i] = (int *) calloc(*c1, sizeof(int)); // Asignación de memoria para las columnas de A
    }

    // Llenar matriz A
    for(i = 0; i < *f; i = i + 1)
    {
        for(j = 0; j < *c1; j = j + 1)
        {
            scanf("%d", &(*A)[i][j]);
        }
    }


    // Inicialización de matriz B
    *B = (int **) calloc(*c1, sizeof(int *)); // Asignación de memoria para filas de B
    for(i = 0; i < *c1; i = i + 1)
    {
        (*B)[i] = (int *) calloc(*c2, sizeof(int)); // Asignación de memoria para las columnas de B
    }

    // Llenar matriz B
    for(i = 0; i < *c1; i = i + 1)
    {
        for(j = 0; j < *c2; j = j + 1)
        {
            scanf("%d", &(*B)[i][j]);
        }
    }
}

// printMatrix: imprime en pantalla la matriz
void printMatrix(int **A, int rows, int columns)
{
    int i, j;

    for(i = 0; i < rows; i = i + 1)
    {
        for(j = 0; j < columns; j = j + 1)
        {
            printf("%d\t", A[i][j]);
        }
        printf("\n");
    }
}

// freeMatrix: libera la memoria de la matriz
void freeMatrix(int **A, int rows)
{
    int i, j;

    for(i = 0; i < rows; i = i + 1)
    {
        free(A[i]);
    }

    free(A);
}

// Process: multiplicación de matrices A y B
int **Process(int **A, int **B, int f, int c1, int c2)
{
    int **C; // Matriz resultante de la multiplicación
    int i, j, k;

    // Inicialización de matriz C
    C = (int **) calloc(f, sizeof(int *));
    for(i = 0; i < f; i = i + 1)
    {
        C[i] = (int *) calloc(c2, sizeof(int));
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

int main()
{
    int **A, **B, **C;
    int f, c1, c2;

    readData(&f, &c1, &c2, &A, &B);

    printf("Matrix A:\n");
    printMatrix(A, f, c1);
    printf("\nMatrix B:\n");
    printMatrix(B, c1, c2);

    C = Process(A, B, f, c1, c2);

    printf("\nResultado multiplicación:\n");
    printMatrix(C, f, c2);


    freeMatrix(A, f);
    freeMatrix(B, c1);

    return 0;
}