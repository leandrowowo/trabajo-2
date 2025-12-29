#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SILENT 0
#define VERBOSE 1

void Usage(char *message)
{
    printf("Usage: %s -O < datafile.txt\n\n", message);
    printf("Where: O in {V: Verbose; S: Silent}\n");
}

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
            C[i][j] = 0.0;

            for(k = 0; k < c1; k = k + 1)
            {
                C[i][j] = C[i][j] + (A[i][k] * B[k][j]);
            }
        }
    }

    return C;
}

void printData(int mode, float **A, float **B, float **C, int f, int c1, int c2)
{
    if(mode == VERBOSE)
    {
        printf("Matrix A:\n");
        printMatrix(A, f, c1);
        printf("\nMatrix B:\n");
        printMatrix(B, c1, c2);
        printf("\nResultado multiplicación:\n");
        printMatrix(C, f, c2);
    }
    else
    {
        printf("Dimensión matriz A: %dx%d\n", f, c1);
        printf("Dimensión matriz B: %dx%d\n", c1, c2);
        
    }
}

int main(int argc, char **argv)
{
    float **A, **B, **C; // Matrices
    int n; // Número de tareas
    int f, c1, c2; // Dimensiones de las matrices
    int mode;
    int i;

    clock_t CPU_start, CPU_finish;
    time_t Wall_start, Wall_finish;
    float CPU_time;
    long Wall_time;

    if(argc != 2)
    {
        Usage(argv[0]);
    }
    else
    {
        if(strcmp(argv[1], "-V") == 0)
        {
            mode = VERBOSE;
        }
        else
        {
            mode = SILENT;
        }
       
        scanf("%d", &n);
        printf("\nNúmero de tareas: %d\n", n);

        CPU_start = clock();
        Wall_start = time(NULL);

        for(i = 0; i < n; i = i + 1)
        {            
            printf("\n----------------------------\n");
            printf("Ejecutando tarea %d:", i+1);
            printf("\n----------------------------\n");

            genData(&f, &c1, &c2, &A, &B);
            C = Process(A, B, f, c1, c2);
            printData(mode, A, B, C, f, c1, c2);
            printf("----------------------------\n");

            freeMatrix(C, f);
        }

        Wall_finish = time(NULL);
        CPU_finish = clock();

        CPU_time = (float)(CPU_finish - CPU_start)/CLOCKS_PER_SEC;
        Wall_time = (long)(Wall_finish - Wall_start);

        printf("\nCPU Time (segundos): %f\n", CPU_time);
        printf("Tiempo total de ejecución (segundos): %ld\n", Wall_time);

        freeMatrix(A, f);
        freeMatrix(B, c1);
    }

    return 0;
}