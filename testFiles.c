#include "PeatonesADT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define CANT_QUERYS 4
#define BUFF_SIZE 512
#define FALSE 0
#define FROM_TO 2

#define TRUE !FALSE
#define DELIM_FIELD ";"
#define EFILE (-2)


typedef enum readingFieldType {R_YEAR = 0, R_MONTH, MDATE, R_DAY, ID, TIME, COUNTS , CANT_FIELDS_READING} readingFieldType;

typedef enum sensorFieldType {SENSOR_ID = 0, NAME, STATUS, CANT_FIELDS_SENSOR} sensorFieldType;


void closeFiles(FILE* files[], int cantFiles);
int loadQuery1(peatonesADT tad, FILE* query1);
int loadQuery4(peatonesADT tad, FILE* query4);
void addLineQuery2 (int year, long int counts, FILE * query2);
void addLineQuery1 (char * sensor, long int counts, FILE * query1);
void addLineQuery3 (int day, long int dayCounts, long int nightCounts, FILE * query3);
void addLineQuery4 (char * sensor, long int maxCount, int * dateFormated, FILE * query4);

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


    FILE * dataSensors = fopen(argv[2], "r");
    FILE * dataReadings= fopen(argv[1], "r");
    FILE * query1 = fopen("query1.csv", "w+");
    FILE * query2 = fopen("query2.csv", "w+");
    FILE * query3 = fopen("query3.csv", "w+");
    FILE * query4 = fopen("query4.csv", "w+");
    FILE * files[] = {dataSensors, dataReadings, query1, query2, query3, query4};
    size_t fileCount = CANT_QUERYS + argc - 1;


//  Revisar que los archivos no sean NULL


    for(int i = 0; i < fileCount; i++) {
        if (files[i] == NULL) {
            printf("ERROR -- ARCHIVO VACIO");
            exit(-1);//cerrar archivos y error
        }
        else printf("file %d detected! \n", i);
    }

    peatonesADT  tad = newPeatones(); //  NUEVO TAD

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
        flag = TRUE;
        // despues de la llamada inicial, strtok debe llevar NULL como primer argumento
        // una vez que se termine la linea, token queda en NULL
        for (int field = 0; field < CANT_FIELDS_SENSOR && token != NULL && flag; field++, token = strtok(NULL, DELIM_FIELD)) {

            switch (field) {
                case SENSOR_ID:
                    id = atoi(token); //str to int  "12"->12
                    if (sensorExists(tad, id)) flag = FALSE; // id duplicado
                    break;
                case NAME:
                    name = token;
                    break;
                case STATUS:
                    if (token[0] == 'R') {
                        flag = FALSE;  // removido
                    }
                    break;
            }
        }
        if (flag == TRUE) {     // se ignoran los sensores repetidos y desactivados
            putSensor(tad, id, name); // creo los sensores
        }
    }

// *************************************************************** DATA READINGS ************************************************************************************

//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    char * Wday;
    int sensorId, counts;
    int dateFormatted[DATE_FIELDS];
    int fromTo[FROM_TO]= {0,0};
    //int fromTo[FROM_TO] = { atoi(argv[3]), atoi(argv[4]) };
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
    }
    //limpio el vector de sensores, eliminando los espacios vacío asi es mas fácil ordenar.
    eliminaCeros(tad);

    //IMPRIR LOS HEADS DE LOS ARCHIVOS
    loadQuery1(tad, query1);
    loadQuery4(tad, query4);



    closeFiles(files, fileCount);
    freePeatones(tad);
}
void closeFiles(FILE* files[], int cantFiles){
    for(int i=0; i<cantFiles; i++){
        fclose(files[i]);
    }
}
void errorExit(FILE * files[], int cantFiles, char * msg, int code, peatonesADT tad ){
    closeFiles(files, cantFiles);
    freePeatones(tad);
    printf("%s", msg);
    exit(code);
}
// Se podria hacer un switch dentro del main

int loadQuery1(peatonesADT tad, FILE* query1){
    sortTotal(tad);
    long int count;
    char* name;
    for(int i=1; i <= getCantSensores(tad); i++) {
        count = getSensorCount(tad, i);
        name = getNameById(tad, i);
        if (name == NULL || count == EID) {
            // errorExit();  // hubo algún tipo de error al cargar
        }
        int res = fprintf(query1, "%s;%li\n", name, count);
        if (res < 0) return EFILE;
    }
    return OK;

}
int loadQuery4(peatonesADT tad, FILE* query4){
    sortMax(tad);
    int count, date;
    char* name;
    int dateFormatted[DATE_FIELDS];
    for(int i=1; i <= getCantSensores(tad); i++){
        count = getMaxCount(tad, i);
        name = getNameById(tad, i);
        date = getDate(tad, i, dateFormatted);
        if(name == NULL || count == EID || date == EID){
            // errorExit();
        }
        int res = fprintf(query4, "%s;%ld;%d;%d/%d/%d\n", name, count, dateFormatted[3], dateFormatted[0], dateFormatted[1], dateFormatted[2]);
        if(res < 0) return EFILE;
    }
    return OK;
}



