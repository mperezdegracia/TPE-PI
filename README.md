## TPE- PROGRAMACIÓN IMPERATIVA - GRUPO 4
    - Primer cuatrimestre 2022
    - Fecha de final: 11/07/2022
    
## AUTORES:
    . Sol Rodriguez, 63029
    . Catalina Müller, 63199
    . Mateo Roman Pérez de Gracia, 63401
    . Uriel Ángel Arias, 63504

## CÓMO GENERAR EJECUTABLES
    - Correr en la consola el comando: 'make' o 'make all', lo que compila el programa y genera automáticamente el archivo ejecutable "pedestrians".
    - Si se desea que la compilación se realice con el flag -g (debug) se debe correr 'make debug'.
    
## EJECUCIÓN
    - Correr el comando: ./pedestrians 'path del archivo CSV de mediciones' 'path del archivo CSV de sensores' 'año de inicio' 'año de fin'(el rango de años es opcional)                                                       SV de sensores' 'año de inicio' 'año de fin' (el rango de años es opcional)
    - OBSERVACIONES: . El orden en el que se pasen los archivos CSV (primero las mediciones y luego los sensores, o viceversa) es indistinto.
                     . Si se pasa un rango de años debe especificarse siempre en números y después de los paths de los archivos CSV.
    
## CÓMO LIMPIAR LOS ARCHIVOS CREADOS
    - Para borrar todos los archivos creados en la compilación y ejecución del programa, correr el comando: 'make cleanAll'
    - Para borrar solo los archivos CSV generados para las queries, correr el comando: 'make cleanQueries'
    - Para borrar solo el archivo ejecutable, correr el comando: 'make cleanExecutable'
    - Para borrar solo los archivos objeto generados, correr el comando: 'make cleanObjects'
    
