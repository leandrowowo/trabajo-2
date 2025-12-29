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
#define FALSE 0
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
    *** FUNCIONES ***
*/

void Usage(char *message)
{
    printf("Usage: mpirun -np P %s -O < datafile.txt\n\n", message);
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
float **MatrixMult(float **A, float **B, int f, int c1, int c2)
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

// Process: función que se encarga de establecer la comunicación y tareas entre nodo maestro y nodos trabajadores
void Process(int n_node, int n_task)
{
    int tasks_sent; // Número de tareas enviadas a los nodos trabajadores
    int workers_active; // Número de tabajadores activos
    int ready;

    float **A, **B, **C_Send; // Matrices A y B a multiplicar. Matriz C resultante de la multiplicación
    int f, c1, c2; // Dimensiones de matrices de entrada
    int dimensiones[3]; // Arreglo con las dimensiones
    int dim[2]; // Dimensiones de matriz C que recibe el maestro
    int f_Recv, c_Recv; // Número de filas y columnas, respectivamente, que recibe el maestro
    float **C_Recv; // Matriz C que arma el maestro a medida que recibe las filas que envía el trabajador

    int i;

    if(nodeID == MASTER) // Nodo maestro
    {
        tasks_sent = 0;
        workers_active = 0;

        // Reparto de las tareas entre los trabajadores
        for(i = 1; i < n_node; i = i + 1)
        {
            printf("\nMaestro: Asignando tarea %d\n", i);
            if(tasks_sent < n_task)
            {
                printf("\nMaestro: Leyendo dimensiones\n");
                scanf("%d %d %d", &dimensiones[0], &dimensiones[1], &dimensiones[2]);

                printf("\nMaestro: Enviando dimensiones de A y B a tabajadores\n");
                MPI_Send(&dimensiones, 3, MPI_INT, i, TAG_SENDTASK, MPI_COMM_WORLD); // Envío de las dimensiones de las matrices a los nodos trabajadores
                printf("\nMaestro: Dimensiones enviadas\n");
                tasks_sent = tasks_sent + 1;
                workers_active = workers_active + 1;
            }
            else
            {
                printf("\nMaestro: Apagando nodo trabajador\n");
                MPI_Send(NULL, 0, MPI_INT, i, TAG_FINISH, MPI_COMM_WORLD); // Si no hay más tareas por asignar, se apagan los nodos que no tienen tareas
                printf("\nMaestro: Nodo trabajador apagado\n");
            }
        }


        // En caso de que queden tareas por asignar, se le asigna una a un nodo que se haya desocupado
        while(workers_active > 0)
        {
            printf("\nMaestro: Recibiendo dimensiones de matriz C resultante\n");
            MPI_Recv(&dim, 2, MPI_INT, MPI_ANY_SOURCE, TAG_SENDDIM, MPI_COMM_WORLD, &status); // El maestro recibe las dimensiones del matriz C
            printf("\nMaestro: Dimensiones recibidas\n");
            f_Recv = dim[0];
            c_Recv = dim[1];

            // Asiganción de memoria para matriz C del maestro
            C_Recv = (float **) calloc(f_Recv, sizeof(float *));
            for(i = 0; i < f_Recv; i = i + 1)
            {
                C_Recv[i] = (float *) calloc(c_Recv, sizeof(float));
            }

            // Maestro recibe filas de C del trabajador
            for(i = 0; i < f_Recv; i = i + 1)
            {
                printf("\nMaestro: Recibiendo fila %d de matriz C\n", i);
                MPI_Recv(C_Recv[i], c_Recv, MPI_FLOAT, status.MPI_SOURCE, TAG_SENDROW, MPI_COMM_WORLD, &status);
                printf("\nMaestro: Fila %d de matriz C recibida\n", i);
            }

            printf("\nMaestro: Mostrando matriz resultante\n");
            printMatrix(C_Recv, f_Recv, c_Recv);

            printf("\nMaestro: Liberando memoria de matriz C\n");
            freeMatrix(C_Recv, f_Recv);
            printf("\nMaestro: Memoria de matriz C liberada\n");

            if(tasks_sent < n_task)
            {
                printf("\nMaestro: Asignando tareas restantes\n");
                printf("\nMaestro: Leyendo dimensiones\n");
                scanf("%d %d %d", &dimensiones[0], &dimensiones[1], &dimensiones[2]);

                printf("\nMaestro: Enviando dimensiones a tabajadores\n");
                MPI_Send(&dimensiones, 3, MPI_INT, status.MPI_SOURCE, TAG_SENDTASK, MPI_COMM_WORLD); // Si quedan tareas por asignar, se le asigna al mismo nodo que se desocupó
                printf("\nMaestro: Dimensiones enviadas\n");
                tasks_sent = tasks_sent + 1;
            }
            else
            {
                printf("\nMaestro: Apagando nodo trabajador\n");
                MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, TAG_FINISH, MPI_COMM_WORLD); // Si hay nodos sin tareas asignadas, se apagan
                printf("\nMaestro: Nodo trabajador apagado\n");
                workers_active = workers_active - 1;
            }
        }
    }
    else // Nodo trabajador
    {
        while(TRUE)
        {
            printf("\nTrabajador: Recibiendo dimensiones de matrices A y B\n");
            MPI_Recv(&dimensiones, 3, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // El nodo trabajador recibe las dimensiones del maestro
            printf("\nTrabajador: Dimensiones recibidas\n");

            if(status.MPI_TAG == TAG_FINISH) 
            {
                printf("\nTrabajador: Tarea terminada\n");
                break; // Si el nodo terminó la tarea, se libera
            }
            else if(status.MPI_TAG == TAG_SENDTASK) // Si se le asigna una tarea, realiza el cálculo de la multiplicación
            {
                printf("\nTrabajador: Tarea asignada\n");
                f = dimensiones[0];
                c1 = dimensiones[1];
                c2 = dimensiones[2];

                genData(f, c1, c2, &A, &B);

                printf("\nTrabajador: Calculando multiplicación\n");
                C_Send = MatrixMult(A, B, f, c1, c2);
                printf("\nTrabajador: Multiplicación lista\n");

                dim[0] = f;
                dim[1] = c2;

                // Envío de las dimensiones de matriz C al maestro
                printf("\nTrabajador: Enviando dimensiones de C al maestro\n");
                MPI_Send(&dim, 2, MPI_INT, MASTER, TAG_SENDDIM, MPI_COMM_WORLD);

                // Envío de las filas de matriz C al maestro
                for(i = 0; i < f; i = i + 1)
                {
                    printf("\nTrabajador: Enviando fila %d de matriz C resultante\n");
                    MPI_Send(C_Send[i], c2, MPI_FLOAT, MASTER, TAG_SENDROW, MPI_COMM_WORLD); // Manda mensaje al maestro que terminó la tarea
                    printf("\nTrabajador: Fila %d de matriz C enviada\n"); 
                }
                           
                // Liberación de memoria de las matrices
                printf("\nTrabajador: Liberando memoria de matrices A, B y C\n");
                freeMatrix(A, f);
                freeMatrix(B, c1);
                freeMatrix(C_Send, f);
                printf("\nTrabajador: Memoria liberada\n");
            }
        }
    }
}

int main(int argc, char **argv)
{
    int n_task, n_nodes;
    int me;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    n_task = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_nodes);
    MPI_Comm_rank(MPI_COMM_WORLD, &nodeID);
    MPI_Get_processor_name(processor_name, &me);

    printf("\n\nProcess [%d] Alive on %s\n", nodeID, processor_name);
    fflush(stdout);

    if(nodeID == MASTER)
    {
        scanf("%d", &n_task);
        printf("Maestro: Se detectaron %d tareas en el archivo\n", n_task);
    }

    Process(n_nodes, n_task);

    MPI_Finalize();

    return 0;
}