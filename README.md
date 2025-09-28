# Shell en C

Este proyecto implementa una shell en C con soporte para:

- Ejecución de comandos normales
- Comandos encadenados con pipes
- Redirección de salida

Para compilar:
    
    gcc prueba3.c -o shell

Funciona en Linux con las librerías estándar.


## Funciones
* Comandos básicos. Puede ejecutar cualquier comando del sistema Linux.
* Permite encadenar comandos usando pipes, por ejemplo:\
$ ls -l | grep .c | wc -l

* Redirige la salida estándar a un archivo.
* Salir de la shell mediante comando exit

## Comando miprof

* Comando para medir rendimiento de programas, limitar tiempo de ejecución y guardar resultados.


Para compilar:
    
    gcc prueba5.c -o shell2


Ejemplo de salida:

Al usar: 
    
    $ miprof ejec ls -l
La salida es:

    total 64
    (listado de contenidos de la carpeta actual)
    Comando: ls
    Tiempo real: 0.002268 s
    Tiempo usuario: 0.001889 s
    Tiempo sistema: 0.000000 s
    Memoria máxima: 2948 KB

![alt text](/image.png)