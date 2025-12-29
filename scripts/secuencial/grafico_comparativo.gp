# 1. Configuración de salida
set terminal pngcairo size 1000,700 enhanced font 'Arial,12'
set output 'grafica_mult_sec.png'

# 2. Títulos y etiquetas
set title "Gráfico rendimiento multiplicación de matrices (versión secuencial)"
set xlabel "Cantidad de tareas"
set ylabel "Tiempo de ejecución total (segundos)"

# 3. Configuración de Ejes y Logaritmos
set grid

unset key

# 4. Estilos de línea (Colores distintivos para cada hilo)
set style line 1 lc rgb '#E41A1C' lt 1 lw 2 pt 7 ps 1.5 # Rojo (5 Hilos)

# 5. EL COMANDO PLOT (Graficar múltiples archivos)
plot "resultados.txt"  using 1:2 with linespoints ls 1 notitle