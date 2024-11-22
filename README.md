# Proyecto 2: Sistemas Operativos

La parte 1 de este proyecto es la cola circular, usando un monitor para simular el problema de productor y consumidor.
## Características principales
  - **Cola Circular**
  - **Log**
  - **Monitor**
## Instrucciones de compilación
Para compilar el programa, ejecuta el siguiente comando en la terminal:
```bash
g++ -o cola productor_consumidor.cpp
```
```bash
./cola -p 10 -c 5 -s 50 -t 1
```

La parte 2 de este proyecto pide implementar algoritmos de reemplazo de páginas de memoria virtual, siendo: **FIFO**, **LRU**, **RS** (Reloj Simple) y **Óptimo**. También se decidió incluir la opción de ejecutar todos los algoritmos secuencialmente mediante la opción **All**. 
## Características principales
- Implementación de los algoritmos:
  - **FIFO** (First In, First Out)
  - **LRU** (Least Recently Used)
  - **RS** (Reloj Simple)
  - **Óptimo**
  - **All** (Ejecución combinada de todos los algoritmos con la opción)
- Manejo de referencias de memoria desde un archivo de texto.
- Resultados detallados mostrando el estado de los marcos de memoria y los fallos acumulados para cada algoritmo.

## Instrucciones de compilación
Para compilar el programa, ejecuta el siguiente comando en la terminal:

```bash
g++ -o simulador simulador.cpp
```
```bash
./simulador -m <num_marcos> -a <algoritmo> -f <archivo>
```
-m <num_marcos>: Define el número de marcos de memoria. Este valor debe ser un número entero positivo.
-a <algoritmo>: Especifica el algoritmo de reemplazo de páginas a ejecutar. Las opciones disponibles son:
- FIFO
- LRU
- RS
- OPTIMO
- All (Ejecuta todos los algoritmos secuencialmente).

-f <archivo_referencias>: Especifica el archivo de texto que contiene las referencias de página. Este archivo debe estar en el mismo directorio que el ejecutable o indicar la ruta completa.
