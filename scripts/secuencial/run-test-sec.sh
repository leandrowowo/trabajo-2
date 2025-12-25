#!/bin/bash

# Nombre del archivo de salida
OUTPUT_FILE="resultados.txt"

# 1. Ejecutar el programa y capturar toda la salida
SALIDA=$(./matrix-mul-sec.exe -S < datafile-test.txt)

# 2. Extraer N (Número de tareas)
N=$(echo "$SALIDA" | grep "Número de tareas" | awk '{print $4}')

# 3. Extraer Tiempo
TIME=$(echo "$SALIDA" | grep "Tiempo total" | awk '{print $6}')

# 4. Guardar en el archivo
echo "$N $TIME" >> "$OUTPUT_FILE"

# Feedback en consola
echo "Registro guardado: Cantidad de tareas $N - Tiempo: $TIME segundos"