# Trabajo Computacional II - Algoritmos Distibuidos
Este trabajo implementa una versión paralelizada del algoritmo de multiplicación de matrices usando los esquemas de memoria privada y memoria compartida, apoyandose de las librerías MPICH y pthread.

## Instalación de librerías
### MPICH
Para Ubuntu, Debian, Mint
```bash
sudo apt update
sudo apt install mpich
```
Para Fedora, RedHat, CentOS
```bash
sudo dnf update
sudo dnf install mpich
```

### Pthreads
Pthreads ya viene instalada en sistemas basados en Unix/Linux, por lo que solo hace falta instalar el paquete de herramientas básicas

Para Ubuntu, Debian, Mint
```bash
sudo apt update
sudo apt install build-essential
```
Para Fedora, RedHat, CentOS
```bash
sudo dnf update
sudo dnf groupinstall "Development Tools"
```

## Compilación
Para la compilación del código debe usarse el compilador de MPICH en conjunto con la librería pthread:
```bash
mpicc t2.c -o t2.exe -lpthread
```

## Ejecución
Para ejecutar el código, debe usarse el comando mpirun de MPICH y entregando los argumentos correspondientes:
```bash
mpirun -np <Cantidad de nodos> ./t2.exe <Cantidad de hilos> [OPCIÓN] < datafile.txt 
```
Las opciones son:
- -V: Modo Verboso
- -S: Modo Silencioso

El modo verboso muestra las matrices de entrada y el resultado de la multiplicación. El modo silencioso muestra la cantidad de nodos participantes y el Wall-Time (Tiempo de ejecución total).
