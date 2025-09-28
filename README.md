Este proyecto implementa una shell en C.

## Funciones
* Comandos básicos. Puede ejecutar comandos del sistema Linux.
* Permite encadenar comandos usando pipes, por ejemplo:\
$ ls -l | grep .c | wc -l

* Redirige la salida estándar a un archivo.
* Salir de la shell mediante comando exit

Para compilar:
    
    gcc prueba3.c -o shell

Funciona en Linux con las librerías estándar. Para usar comandos, se deben introducir cuando la consola muestre:
    
    shell: $

y luego presionar Enter. Para salir, hay que usar el comando exit



## Comando miprof

* Comando para medir rendimiento de programas, limitar tiempo de ejecución y guardar resultados.


Para compilar:
    
    gcc prueba5.c -o shell2


Ejemplo de salida:

Al usar: 
    
    shell: $ miprof ejec ls -l
La salida es:

    total 64
    (listado de contenidos de la carpeta actual)
    Comando: ls
    Tiempo real: 0.002268 s
    Tiempo usuario: 0.001889 s
    Tiempo sistema: 0.000000 s
    Memoria máxima: 2948 KB

![alt text](/image.png)