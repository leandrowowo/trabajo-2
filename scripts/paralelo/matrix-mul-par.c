#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define SILENT 0
#define VERBOSE 1

// Variables globales
float **A, **B, **C;
int f, c1, c2; 

// Estructura para pasar mensajes a los hilos
struct Messages {
    int id;             // ID del hilo
    int start_index;    // Inicio índice
    int end_index;      // Fin índice
    int opmode;         // Modo de operación (VERBOSE/SILENT)   
};

/*
*
*/
void Usage(char *message)
{
    printf("Usage: %s k -O < datafile.txt\n\n", message);
    printf("Where:\n");
    printf("\tk: Number of threads\n");
    printf("\tO in {V: Verbose; S: Silent}\n");
}

/*
*
*/
void genData()
{
    int i, j;

    scanf("%d", &f);
    scanf("%d", &c1);
    scanf("%d", &c2);

    // Inicialización de matriz A
    A = (float **) calloc(f, sizeof(float *)); 
    for(i = 0; i < f; i = i + 1)
    {
        A[i] = (float *) calloc(c1, sizeof(float)); 
        for(j = 0; j < c1; j = j + 1) A[i][j] = 1.0;
    }

    // Inicialización de matriz B
    B = (float **) calloc(c1, sizeof(float *)); 
    for(i = 0; i < c1; i = i + 1)
    {
        B[i] = (float *) calloc(c2, sizeof(float)); 
        for(j = 0; j < c2; j = j + 1) B[i][j] = 2.0;
    }
}

/*
*
*/
void printMatrix(float **Mat, int rows, int columns)
{
    int i, j;
    for(i = 0; i < rows; i = i + 1)
    {
        for(j = 0; j < columns; j = j + 1)
        {
            printf("%.1f\t", Mat[i][j]);
        }
        printf("\n");
    }
}

/*
*
*/
void freeMatrix(float **Mat, int rows)
{
    int i;
    for(i = 0; i < rows; i = i + 1) free(Mat[i]);
    free(Mat);
}

/*
*
*/
void *Process(void *p)
{
    struct Messages *data = (struct Messages *) p;
    int i, j, k;

    // Si es VERBOSE, mostrar información del hilo
    if (data->opmode == VERBOSE)
    {
        printf("[Hilo %d] Procesando %d filas (Desde fila %d hasta fila %d)\n",
               data->id, 
               (data->end_index - data->start_index), 
               data->start_index, 
               data->end_index - 1);
    }

    // Multiplicación de matrices 
    for(i = data->start_index; i < data->end_index; i = i + 1)
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

    pthread_exit(NULL);
}

/*
*
*/
void printData(int mode)
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
    int n; 
    int mode;
    int i, t;
    int k;

    clock_t CPU_start, CPU_finish;
    time_t Wall_start, Wall_finish;
    float CPU_time;
    long Wall_time;

    pthread_t *thread;
    pthread_attr_t attribute;
    
    struct Messages **mess;
    void *exit_status;
    int s, rem, l; 

    if(argc != 3)
    {
        Usage(argv[0]);
        return -1;
    }

    k = atoi(argv[1]);

    if(strcmp(argv[2], "-V") == 0) mode = VERBOSE;
    else mode = SILENT;
    
    scanf("%d", &n);
    printf("\nNúmero de tareas: %d\n", n);

    pthread_attr_init(&attribute);
    pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);
    
    thread = calloc(k, sizeof(pthread_t));
    mess = calloc(k, sizeof(struct Messages *));
    for (i = 0; i < k; i = i + 1)
    {
        mess[i] = calloc(1, sizeof(struct Messages));
    }

    CPU_start = clock();
    Wall_start = time(NULL);

    for(i = 0; i < n; i = i + 1)
    {            
        printf("\n----------------------------\n");
        printf("Ejecutando tarea %d con %d hilos:", i+1, k);
        printf("\n----------------------------\n");

        genData();

        C = (float **) calloc(f, sizeof(float *));
        for(t = 0; t < f; t = t + 1)
        {
            C[t] = (float *) calloc(c2, sizeof(float));
        }

        s = f / k;
        rem = f % k;
        l = 0;                

        for (t = 0; t < k; t = t + 1) 
        {
            int current_chunk_size;
            if (rem > 0) {
                current_chunk_size = s + 1;
                rem = rem - 1;
            } else {
                current_chunk_size = s;
            }

            mess[t]->id = t;
            mess[t]->opmode = mode;
            mess[t]->start_index = l;                   
            mess[t]->end_index = l + current_chunk_size; 
            
            l = l + current_chunk_size;

            pthread_create(&thread[t], &attribute, Process, (void *) mess[t]);
        }

        for (t = 0; t < k; t = t + 1)
        {
            pthread_join(thread[t], &exit_status);
        }

        printData(mode);
        printf("----------------------------\n");

        freeMatrix(C, f);
        freeMatrix(A, f);
        freeMatrix(B, c1);
    }

    Wall_finish = time(NULL);
    CPU_finish = clock();

    CPU_time = (float)(CPU_finish - CPU_start)/CLOCKS_PER_SEC;
    Wall_time = (long)(Wall_finish - Wall_start);

    printf("\nCPU Time (segundos): %f\n", CPU_time);
    printf("Tiempo total de ejecución (segundos): %ld\n", Wall_time);

    pthread_attr_destroy(&attribute);
    for(i = 0; i < k; i = i + 1) free(mess[i]);
    free(mess);
    free(thread);

    return 0;
}