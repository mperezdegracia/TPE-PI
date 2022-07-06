#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>

#define CANT_QUERYS 4
#define BUFF_SIZE 512
#define FALSE 0
#define FROM_TO 2

#define TRUE !FALSE
#define DELIM_FIELD ";"



typedef enum readingFieldType {R_YEAR = 0, R_MONTH, MDATE, R_DAY, ID, TIME, COUNTS , CANT_FIELDS_READING} readingFieldType;

typedef enum sensorFieldType {SENSOR_ID = 0, NAME, STATUS, CANT_FIELDS_SENSOR} sensorFieldType;

//  Imprime un mensaje de error y aborta el programa
void errorExit (int errValue, char * errMessage, char * arg);

//  Imprime un mensaje de error y cierra todos los archivos y aborta el programa
void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount);

//  Cierraa todos los archivos
void closeAllFiles (FILE * files[], size_t fileCount);



//funcion auxiliar que pasa al siguiente token
char * update(char * token){
    return token = strtok(NULL, DELIM_FIELD);
}
//funcion auxiliar que devuelve el numero del mes
static int monthToNum(char*month){
    char * months[CANT_MONTH] = {"January", "February", "March","April","May", "June", "July",
                                 "August", "September", "October", "November", "December"};
    for(int i=0; i<CANT_MONTH; i++){
        if(strcmp(month, months[i])==0)return i+1;
    }
    return -1;
}

int main(int argc, char * argv[]) {

    // Validacion de parametros (EINVAL: argumento invalido)
    if (argc < 3 || argc > 5) {
        errorExit(EINVAL, "Cantidad invalida de argumentos", argv[0]);
    }
    if ((argc > 3 && !isnumber(*argv[3])) || (argc == 5 && (!isnumber(*argv[4]) || atoi(argv[3]) > atoi(argv[4])))) {
        errorExit(EINVAL, "Los parametros son incorrectos", argv[0]);
    }



// errores
    errno = 0;



//  Declaración de Archivos


    FILE * dataSensors = fopen(argv[2], "r");
    FILE * dataReadings= fopen(argv[1], "r");
    FILE * query1 = fopen("query1.csv", "w");
    FILE * query2 = fopen("query2.csv", "w");
    FILE * query3 = fopen("query3.csv", "w");
    FILE * query4 = fopen("query4.csv", "w");
    FILE * files[] = {dataSensors, dataReadings, query1, query2, query3, query4};
    size_t fileCount = CANT_QUERYS + argc - 1;


    // Revisamos que los archivos no sean NULL, de lo contrario cierra todos, da un mensaje de error y aborta el programa
    // ENOENT: no existe dicho archivo.
    // ENOMEM: memoria insuficiente
    if (files[0] == NULL || files[1] == NULL) {
        closeExit(files, ENOENT, "Los parametros son incorrectos", argv[0], fileCount);
    }
    for(size_t i = 2; i < fileCount; i++) {
        if (files[i] == NULL) {
            closeExit(files, ENOMEM, "No se pudo abrir uno de los archivos", argv[0], fileCount);
        }
    }


    peatonesADT  tad = newPeatones(); //  NUEVO TAD

    if (tad == NULL || errno == ENOMEM) { //  SI NO SE PUDO CREAR EL TAD
        closeExit(files, ENOMEM, "No hay memoria suficiente en el heap", argv[0], fileCount);
    }
//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int id, flag;
    char* name;
    char status;
    char buff[BUFF_SIZE], * token; // en buff se van a ir llegando las lineas del .csv. BUFF_SIZE es un tamaño arbitrario

    // Si la primer linea del archivo dataSensors esta vacia, retorna un mensaje de error y aborta el programa
    if (fgets(buff, BUFF_SIZE, dataSensors) == NULL) {
        freePeatones(tad);
        closeExit(files, EINVAL, "El archivo ingresado esta vacio", argv[0], fileCount);
    }

    while(fgets(buff, BUFF_SIZE, dataSensors) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // flag= TRUE;
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
       id = atoi(token);
        if(!sensorExists(tad, id)) { // si el id no es duplicado
            token = update(token);
            name = token;
            token = update(token);
            if (strcmp(token, "A")==0){
                putSensor(tad, id, name); // creo los sensores
            }
        }
    }

/*
        for(int field = 0; field < CANT_FIELDS_SENSOR && token != NULL && flag ; field++, token = strtok(NULL, DELIM_FIELD)) {

            switch(field) {
                case SENSOR_ID:
                    id = atoi(token); //str to int  "12"->12
                    if(sensorExists(tad, id)) flag = FALSE; // id duplicado
                    break;
                case NAME:
                    name = token;
                    break;
                case STATUS:
                    if (strcmp(token, "R")==0) flag = FALSE;  // removido
                    break;
            }
        }
        if(flag==TRUE) {     // se ignoran los sensores repetidos y desactivados
            putSensor(tad, id, name); // creo los sensores
        }

        if(errno == ENOMEM){
            // errores de memoria
            freePeatones(tad);
            closeExit(files, ENOMEM, "No hay memoria suficiente", argv[0], fileCount);
        }
    }
*/
// *************************************************************** DATA READINGS ************************************************************************************

//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    char * Wday;
    int sensorId, counts;
    int dateFormatted[DATE_FIELDS];
    int fromTo[FROM_TO] = { atoi(argv[3]), atoi(argv[4]) };
    if (fgets(buff, BUFF_SIZE, dataReadings) == NULL) {
        freePeatones(tad);
        closeExit(files, EINVAL, "El archivo ingresado esta vacio", argv[0], fileCount);
    }

    while(fgets(buff, BUFF_SIZE, dataReadings) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        // flag= TRUE;
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        dateFormatted[YEAR] = atoi(token);
        token = update(token);
        dateFormatted[MONTH] = monthToNum(token);
        token = update(token);
        dateFormatted[DAY] = atoi(token);
        token = update(token);
        Wday = token;
        token = update(token);
        sensorId = atoi(token);
        token = update(token);
        dateFormatted[HOUR] = atoi(token);
        token = update(token);
        counts = atoi(token);
        token = update(token); //token vale NULL

        if(sensorExists(tad, id)) {     // se ignoran los sensores repetidos y desactivados
            addReading(tad, id, dateFormatted, Wday, counts, fromTo); // creo los sensores
        }
        /*
        for(int field = 0; field < CANT_FIELDS_READING && token != NULL && flag ; field++, token = strtok(NULL, DELIM_FIELD)) {

            switch(field) {
                case ID:
                    id = atoi(token); //str to int  "12"->12
                    if(!sensorExists(tad, id)) flag = FALSE; // no hay un sensor asociado al reading
                    break;
                case R_YEAR:
                    dateFormatted[YEAR] = atoi(token);
                    break;
                case R_MONTH:
                    dateFormatted[MONTH] = monthToNum(token);
                    break;
                case R_DAY:
                    dateFormatted[DAY] = atoi(token);
                    break;
                case TIME:
                    dateFormatted[HOUR] = atoi(token);
                    break;
                case COUNTS:
                    counts = atoi(token);
                    break;
                case MDATE:
                    Wday = token;
                    break;
            }
        }
        if(flag==TRUE) {     // se ignoran los sensores repetidos y desactivados
            addReading(tad, id, dateFormatted, Wday, counts, fromTo); // creo los sensores
        }

        if(errno == ENOMEM){
            // errores de memoria
            freePeatones(tad);
            closeExit(files, ENOMEM, "No hay memoria suficiente, argv[0], fileCount");
        }
        */
    }


    //  Cerramos los archivos y liberamos la memoria

    closeAllFiles(files, fileCount);
    freePeatones(tad);
}

// Se podria hacer un switch dentro del main
void addLineQuery1 (char * sensor, long int counts, FILE * query1) {
    fprintf(query1, "%s;%li\n", sensor, counts);
}

void addLineQuery2 (int year, long int counts, FILE * query2) {
    fprintf(query2, "%d;%li\n", year, counts);
}

void addLineQuery3 (int day, long int dayCounts, long int nightCounts, FILE * query3) {
    fprintf(query3, "%d;%li;%li;%li\n", day, dayCounts, nightCounts, dayCounts+nightCounts);
}

void addLineQuery4 (char * sensor, long int maxCount, int * dateFormated, FILE * query4) {
    fprintf(query4, "%s;%ld;%d;%d/%d/%d\n", sensor, maxCount, dateFormated[3], dateFormated[0], dateFormated[1], dateFormated[2]);
}

void errorExit (int errValue, char * errMessage, char * arg) {
    fprintf(stderr, "%s : %s\n", arg, errMessage);
    exit(errValue);
}

void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount) {
    closeAllFiles(files, fileCount);
    errorExit(errValue, errMessage, arg);
}

void closeAllFiles (FILE * files[], size_t fileCount) {
    for(size_t i = 0; i < fileCount; i++) {
        if (files[i] != NULL) {
            fclose(files[i]);
        }
    }
}
