#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define CANT_QUERYS 4
#define BUFF_SIZE 256
#define FALSE 0
#define FROM_TO 2

#define TRUE !FALSE
#define DELIM_FIELD ";"



typedef enum readingFieldType {R_YEAR = 0, R_MONTH, MDATE, R_DAY, ID, TIME, COUNTS , CANT_FIELDS_READING} readingFieldType;

typedef enum sensorFieldType {SENSOR_ID = 0, NAME, STATUS, CANT_FIELDS_SENSOR} sensorFieldType;

int main(int cantArgs, char * args[]) {

    if(cantArgs < 3 || cantArgs > 5){
        //función que de error
    }



// errores
    errno = 0;



//  Declaración de Archivos

    FILE * query1 = fopen("query1.csv", "w");
    FILE * query2 = fopen("query2.csv", "w");
    FILE * query3 = fopen("query3.csv", "w");
    FILE * query4 = fopen("query4.csv", "w");
    FILE * dataSensors = fopen(args[2], "r");
    FILE * dataReadings= fopen(args[1], "r");

    FILE * files[] = {dataSensors, dataReadings, query1, query2, query3, query4};
    size_t fileCount = CANT_QUERYS + cantArgs - 1;


//  Revisar que los archivos no sean NULL


    for(int i = 0; i < fileCount; i++) {
        if (files[i] == NULL) {
            //cerrar archivos y error
        }
    }



    peatonesADT  tad = newPeatones(); //  NUEVO TAD

    if (tad == NULL || errno == ENOMEM) { //  SI NO SE PUDO CREAR EL TAD
        //closeFiles(files, fileCount);
        //errNOut("No hay memoria disponible en el heap", ENOMEM);
    }
//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int id, flag;
    char* name;
    char status;
    char buff[BUFF_SIZE], * token; // en buff se van a ir llegando las lineas del .csv. BUFF_SIZE es un tamaño arbitrario

    if (fgets(buff, BUFF_SIZE, dataSensors) == NULL) {
    //si está vacío ERROR
    }

    while(fgets(buff, BUFF_SIZE, dataSensors) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        flag= TRUE;
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL

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
        }
    }

// *************************************************************** DATA READINGS ************************************************************************************

//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int sensorId, counts;
    int dateFormatted[DATE_FIELDS];
    int fromTo[FROM_TO] = { atoi(args[3]), atoi(args[4]) };
    if (fgets(buff, BUFF_SIZE, dataReadings) == NULL) {
        //si está vacío ERROR
    }

    while(fgets(buff, BUFF_SIZE, dataReadings) != NULL) { //leo las lineas del archivo hasta el final, guardo la linea en buff hasta BUFF_SIZE caracteres.

        token = strtok(buff, DELIM_FIELD);
        flag= TRUE;
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL

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
                    dateFormatted[MONTH] = atoi(token);
                    break;
                case R_DAY:
                    dateFormatted[DAY] = atoi(token);
                    break;
                case TIME:
                    dateFormatted[HOUR] = atoi(token);
                case COUNTS:
                    counts = atoi(token);
                    break;

            }
        }
        if(flag==TRUE) {     // se ignoran los sensores repetidos y desactivados
            addReading(tad, id, dateFormatted, counts, fromTo); // creo los sensores
        }

        if(errno == ENOMEM){
            // errores de memoria
        }
    }
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

