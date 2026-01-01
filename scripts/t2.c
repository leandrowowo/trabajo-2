/***************************************************************************************
 * 
 * matrix-mul-par.c: Programa que calcula la multiplicación de dos matrices de manera paralela
 *                   a través del modelo de "granja" o "maestro-trabajador" para la asignación
 *                   de tareas usando el esquema de memoria privada y el cálculo de la
 *                   multiplicación a través del esquema de memoria compartida usando los 
 *                   hilos de los nodos trabajadores. 
 *
 * Programmer: Leandro Aballay Henriquez - Delian Santis Lopez
 *
 * Santiago de Chile, 05/01/2026
 *
 **************************************************************************************/
/*
    *** LIBRERÍAS ***
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <pthread.h>
#include "mpi.h"

/*
    *** DEFINE's ***
*/

// *** Modos de impresión de resultados ***
#define SILENT 0
#define VERBOSE 1

// *** Constantes lógicas ***
#define TRUE 1

// *** Etiquetas para nodos ***
#define MASTER 0

// *** Etiquetas para tareas *** 
#define TAG_SENDTASK 1  // Etiqueta para mandar tareas a los nodos trabajadores
#define TAG_TASKREADY 2  // Etiqueta para mandar al master que la tarea se finalizó
#define TAG_FINISH 3  // Etiqueta para que el maestro le diga al trabajador que se apague si no tiene más tareas que asignar
#define TAG_SENDDIM 4 // Etiqueta para que el trabajador le mande al maestro las dimensiones de la matrixz C
#define TAG_SENDROW 5 // Etiqueta para que el trabajador le mande al maestro la fila de C correspondiente

int nodeID;
MPI_Status status;

/*
    *** STRUCT ***
*/
struct Messages
{
    int id;             // ID del hilo
    int start_index;    // Inicio índice
    int end_index;      // Fin índice

    int f;              // Dimensiones de matrices de entrada
    int c1;
    int c2;

    float **A;          // Matrices a operar (Memoria compartida)
    float **B;
    float **C;
};

/*
    *** FUNCIONES ***
*/

void Usage(char *message)
{
    printf("Usage: mpirun -np P %s k -O < datafile.txt\n\n", message);
    printf("Where: O in {V: Verbose; S: Silent}\n");
    printf("       P: cantidad de nodos");
}

// readData: lee el archivo de entrada y llena las matrices A y B.
void genData(int f, int c1, int c2, float ***A, float ***B)
{
    int i, j;

    // Inicialización de matriz A
    *A = (float **) calloc(f, sizeof(float *)); // Asignación de memoria para filas de A
    for(i = 0; i < f; i = i + 1)
    {
        (*A)[i] = (float *) calloc(c1, sizeof(float)); // Asignación de memoria para las columnas de A
    }

    // Llenar matriz A
    for(i = 0; i < f; i = i + 1)
    {
        for(j = 0; j < c1; j = j + 1)
        {
            (*A)[i][j] = 1.0;
        }
    }

    // Inicialización de matriz B
    *B = (float **) calloc(c1, sizeof(float *)); // Asignación de memoria para filas de B
    for(i = 0; i < c1; i = i + 1)
    {
        (*B)[i] = (float *) calloc(c2, sizeof(float)); // Asignación de memoria para las columnas de B
    }

    // Llenar matriz B
    for(i = 0; i < c1; i = i + 1)
    {
        for(j = 0; j < c2; j = j + 1)
        {
            (*B)[i][j] = 2.0;
        }
    }
}

// printMatrix: imprime en pantalla la matriz
void printMatrix(float **A, int rows, int columns)
{
    int i, j;

    printf("\n");
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

// MatrixMult: multiplicación de matrices A y B
void *MatrixMult(void *p)
{
    struct Messages *me;
    int i, j, k;

    me = (struct Messages *) p;

    // Multiplicación de matrices 
    for(i = me->start_index; i < me->end_index; i = i + 1)
    {
        for(j = 0; j < me->c2; j = j + 1)
        {
            me->C[i][j] = 0.0;

            for(k = 0; k < me->c1; k = k + 1)
            {
                me->C[i][j] = me->C[i][j] + (me->A[i][k] * me->B[k][j]);
            }
        }
    }

    pthread_exit(NULL);
}

// Process: función que se encarga de establecer la comunicación y tareas entre nodo maestro y nodos trabajadores,
void Process(int mode, int n_node, int n_task, int n_thread)
{
    int tasks_sent; // Número de tareas enviadas a los nodos trabajadores
    int workers_active; // Número de tabajadores activos

    float **A, **B, **C_Send; // Matrices A y B a multiplicar. Matriz C resultante de la multiplicación
    int f, c1, c2; // Dimensiones de matrices de entrada
    int dim_Send[3]; // Arreglo con las dimensiones de matrices A y B que manda el maestro al trabajador
    int dim_Recv[2]; // Dimensiones de matriz C que recibe el maestro
    int f_Recv, c_Recv; // Número de filas y columnas, respectivamente, que recibe el maestro
    float **C_Recv; // Matriz C que arma el maestro a medida que recibe las filas que envía el trabajador

    clock_t CPU_start, CPU_finish;
    time_t Wall_start, Wall_finish;
    float CPU_time;
    long Wall_time;

    pthread_t *threads;
    pthread_attr_t attribute;
    
    struct Messages **mess;
    void *exit_status;
    int chunk_size, remainder, index, current_chunk;

    int i, t;

    if(nodeID == MASTER) // Nodo maestro
    {
        tasks_sent = 0;
        workers_active = 0;

        CPU_start = clock();
        Wall_start = time(NULL);

        /*
            *** Master: Asignación de tareas a nodos trabajadores
        */
        for(i = 1; i < n_node; i = i + 1)
        {
            printf("\nMaestro: Asignando tarea %d\n", i);
            if(tasks_sent < n_task)
            {
                scanf("%d %d %d", &dim_Send[0], &dim_Send[1], &dim_Send[2]);

                MPI_Send(&dim_Send, 3, MPI_INT, i, TAG_SENDTASK, MPI_COMM_WORLD); // Envío de las dimensiones de las matrices a los nodos trabajadores
                tasks_sent = tasks_sent + 1;
                workers_active = workers_active + 1;
            }
            else
            {
                MPI_Send(NULL, 0, MPI_INT, i, TAG_FINISH, MPI_COMM_WORLD); // Si no hay más tareas por asignar, se apagan los nodos que no tienen tareas
            }
        }

        // En caso de que queden tareas por asignar, se le asigna una a un nodo que se haya desocupado
        while(workers_active > 0)
        {
            MPI_Recv(&dim_Recv, 2, MPI_INT, MPI_ANY_SOURCE, TAG_SENDDIM, MPI_COMM_WORLD, &status); // El maestro recibe del trabajador las dimensiones de matriz C
            f_Recv = dim_Recv[0];
            c_Recv = dim_Recv[1];

            // Asiganción de memoria para matriz C del maestro
            C_Recv = (float **) calloc(f_Recv, sizeof(float *));
            for(i = 0; i < f_Recv; i = i + 1)
            {
                C_Recv[i] = (float *) calloc(c_Recv, sizeof(float));
            }

            // Maestro recibe filas de C del trabajador
            for(i = 0; i < f_Recv; i = i + 1)
            {
                MPI_Recv(C_Recv[i], c_Recv, MPI_FLOAT, status.MPI_SOURCE, TAG_SENDROW, MPI_COMM_WORLD, &status);
            }

            if(mode == VERBOSE)
            {
                printf("\nMaestro: Mostrando matrices\n");
                printf("\nMatriz A:\n");
                printMatrix(A, f, c1);
                printf("\nMatriz B:\n");
                printMatrix(B, c1, c2);
                printf("\nMatriz C resultante:\n");
                printMatrix(C_Recv, f_Recv, c_Recv);
            }

            freeMatrix(C_Recv, f_Recv);

            if(tasks_sent < n_task)
            {
                scanf("%d %d %d", &dim_Send[0], &dim_Send[1], &dim_Send[2]);

                MPI_Send(&dim_Send, 3, MPI_INT, status.MPI_SOURCE, TAG_SENDTASK, MPI_COMM_WORLD); // Si quedan tareas por asignar, se le asigna al mismo nodo que se desocupó
                tasks_sent = tasks_sent + 1;
            }
            else
            {
                MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, TAG_FINISH, MPI_COMM_WORLD); // Si hay nodos sin tareas asignadas, se apagan
                workers_active = workers_active - 1;
            }
        }

        Wall_finish = time(NULL);
        CPU_finish = clock();

        CPU_time = (float)((CPU_finish - CPU_start)/CLOCKS_PER_SEC);
        Wall_time = (long)(Wall_finish - Wall_start);

        if(mode == SILENT)
        {
            printf("\n\n-------------------------\n");
            printf("Cantidad de nodos: %d\n", n_node);
            printf("Tiempo de ejecución total (Wall-time): %ld\n", Wall_time);
            printf("-------------------------\n");
        }
    }
    else // Nodo trabajador
    {
        while(TRUE)
        {
            MPI_Recv(&dim_Send, 3, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // El nodo trabajador recibe las dimensiones del maestro

            if(status.MPI_TAG == TAG_FINISH) 
            {
                break; // Si el nodo terminó la tarea, se libera
            }
            else if(status.MPI_TAG == TAG_SENDTASK) // Si se le asigna una tarea, realiza el cálculo de la multiplicación
            {
                // Asignación de dimensiones
                f = dim_Send[0];
                c1 = dim_Send[1];
                c2 = dim_Send[2];

                genData(f, c1, c2, &A, &B);

                // Asignación de memoria de matriz C para enviar al maestro
                C_Send = (float **) calloc(f, sizeof(float *));
                for(i = 0; i < f; i = i + 1)
                {
                    C_Send[i] = (float *) calloc(c2, sizeof(float));
                }

                /*
                    *** Cálculo de multiplicación de matrices con memoria compartida ***
                */

                // Cálculo de tamaños de chunks de cada hilo 
                chunk_size = f / n_thread;
                remainder = f % n_thread;
                index = 0;

                // Asignación de memoria para hilos
                threads = calloc(n_thread, sizeof(pthread_t*));
                mess = calloc(n_thread, sizeof(struct Messages *));
                for(i = 0; i < n_thread; i = i + 1)
                {
                    mess[i] = calloc(1, sizeof(struct Messages));
                }

                pthread_attr_init(&attribute);
                pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);

                for(t = 0; t < n_thread; t = t + 1)
                {
                    mess[t]->id = t;

                    if(remainder != 0)
                    {
                        current_chunk = chunk_size + 1;
                        remainder = remainder - 1;
                    }
                    else
                    {
                        current_chunk = chunk_size;
                    }

                    // Asignación de variables a cada hilo
                    mess[t]->f = f;
                    mess[t]->c1 = c1;
                    mess[t]->c2 = c2;
                    mess[t]->A = A;
                    mess[t]->B = B;
                    mess[t]->C = C_Send;
                    mess[t]->start_index = index;
                    mess[t]->end_index = index + current_chunk;

                    index = index + current_chunk;

                    pthread_create(&threads[t], &attribute, MatrixMult, (void *)mess[t]);
                }

                for(t = 0; t < n_thread; t = t + 1)
                {
                    pthread_join(threads[t], &exit_status);
                    free(mess[t]);
                }

                free(threads);
                free(mess);
                pthread_attr_destroy(&attribute);
                
                /*
                    *** Proceso de envío de matriz C resultante al maestro ***
                */

                dim_Recv[0] = f;
                dim_Recv[1] = c2;

                // Envío de las dimensiones de matriz C al maestro
                MPI_Send(&dim_Recv, 2, MPI_INT, MASTER, TAG_SENDDIM, MPI_COMM_WORLD);

                // Envío de las filas de matriz C al maestro
                for(i = 0; i < f; i = i + 1)
                {
                    MPI_Send(C_Send[i], c2, MPI_FLOAT, MASTER, TAG_SENDROW, MPI_COMM_WORLD); // Manda mensaje al maestro que terminó la tarea
                }
                           
                // Liberación de memoria de las matrices
                freeMatrix(A, f);
                freeMatrix(B, c1);
                freeMatrix(C_Send, f);
            }
        }
    }
}

int main(int argc, char **argv)
{
    int n_task, n_nodes, n_threads;
    int me, mode;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    n_task = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_nodes);
    MPI_Comm_rank(MPI_COMM_WORLD, &nodeID);
    MPI_Get_processor_name(processor_name, &me);

    if(argc != 3 || (strcmp(argv[2], "-V") && strcmp(argv[2], "-S")))
    {
        Usage(argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    else
    {
        if(strcmp(argv[2], "-V") == 0)
        {
            mode = VERBOSE;
        }
        else if(strcmp(argv[2], "-S") == 0)
        {
            mode = SILENT;
        }

        n_threads = atoi(argv[1]);
    }

    if(nodeID == MASTER)
    {
        scanf("%d", &n_task);
        printf("\nMaestro: Se detectaron %d tareas en el archivo\n", n_task);
    }

    Process(mode, n_nodes, n_task, n_threads);

    MPI_Finalize();

    return 0;
}