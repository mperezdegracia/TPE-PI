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
void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount, peatonesADT tad);

//  Cierraa todos los archivos
void closeAllFiles (FILE * files[], size_t fileCount);

int loadQuery1(peatonesADT tad, FILE* query1);
int loadQuery4(peatonesADT tad, FILE* query4);
int loadQuery3(peatonesADT tad, FILE* query3);
int loadQuery2(peatonesADT tad, FILE* query2);

//recibe un string y devuelve 1 si es un numero y 0 en caso contrario
int isnumber(const char * num){
    while(*num != 0){
        if (!isdigit(*num)) return 0;
        num++;
    }
    return 1;
}

//funcion auxiliar que pasa al siguiente token
char * update(const char * token){
    return strtok(NULL, DELIM_FIELD);
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
    if ((argc > 3 && !isnumber(argv[3])) || (argc == 5 && (!isnumber(argv[4]) || atoi(argv[3]) > atoi(argv[4])))) {
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
    size_t fileCount = CANT_QUERYS + argc - 3;


    // Revisamos que los archivos no sean NULL, de lo contrario cierra todos, da un mensaje de error y aborta el programa
    // ENOENT: no existe dicho archivo.
    // ENOMEM: memoria insuficiente
    if (files[0] == NULL || files[1] == NULL) {
        closeAllFiles(files, fileCount);
        errorExit(ENOENT, "Los parametros son incorrectos", argv[0]);
    }
    for(size_t i = 2; i < fileCount; i++) {
        if (files[i] == NULL) {
            closeAllFiles(files, fileCount);
            errorExit(ENOMEM, "No se pudo abrir uno de los archivos", argv[0]);
        }
    }

    peatonesADT  tad = newPeatones(); //  NUEVO TAD

    if (tad == NULL || errno == ENOMEM) { //  SI NO SE PUDO CREAR EL TAD
        closeAllFiles(files, fileCount);
        errorExit(ENOMEM, "No hay memoria suficiente en el heap", argv[0]);
    }
//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    int id, flag;
    char* name;
    char status;
    char buff[BUFF_SIZE], * token; // en buff se van a ir llegando las lineas del .csv. BUFF_SIZE es un tamaño arbitrario

    // Si la primer linea del archivo dataSensors esta vacia, retorna un mensaje de error y aborta el programa
    if (fgets(buff, BUFF_SIZE, dataSensors) == NULL) {
        closeExit(files, EINVAL, "El archivo ingresado esta vacio", argv[0], fileCount, tad);
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
            if (token[0] == 'A'){
                putSensor(tad, id, name); // creo los sensores
            }
        }
    }

// *************************************************************** DATA READINGS ************************************************************************************

//  VARIABLES QUE LLENAMOS CON DATA_SENSORS
    char * Wday;
    int sensorId, counts;
    int dateFormatted[DATE_FIELDS];
    int fromTo[FROM_TO] = { atoi(argv[3]), atoi(argv[4]) };
    if (fgets(buff, BUFF_SIZE, dataReadings) == NULL) {
        closeExit(files, EINVAL, "El archivo ingresado esta vacio", argv[0], fileCount, tad);
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

        if (sensorExists(tad, sensorId)) {     // se ignoran los sensores repetidos y desactivados
            addReading(tad, sensorId, dateFormatted, Wday, counts, fromTo); // creo los sensores
        }
    }
    eliminaCeros(tad);
    loadQuery1(tad, query1);
    loadQuery2(tad, query2);
    loadQuery3(tad, query3);
    loadQuery4(tad, query4);

    closeAllFiles(files, fileCount);
    freePeatones(tad);
}


//función que escribe sobre el archivo query2.csv con los resultados de la consulta
//devuelve 1 si no hubo problemas en la escritura del archivo o 0 si hubo un error
int loadQuery2(peatonesADT pea, FILE * query2){
    toBeginYear(pea);
    int year, status;
    long int counts;
    while (hasNextYear(pea)){
        year = getYear(pea);
        counts = getCount(pea);
        status = fprintf(query2, "%d;%li\n", year, counts);;
        if (status < 0){
            return EFILE;
        }
        nextYear(pea);
    }
    return OK;
}

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
        int res = fprintf(query4, "%s;%d;%d;%d/%d/%d\n", name, count, dateFormatted[3], dateFormatted[0], dateFormatted[1], dateFormatted[2]);
        if(res < 0) return EFILE;
    }
    return OK;
}
int loadQuery3(peatonesADT pea, FILE * query3){
    long int nightCount, dayCount;
    for(int day = MONDAY; day<CANT_DAYS; day++){
        nightCount = getDailyCount(pea, day, TRUE);
        dayCount = getDailyCount(pea, day, FALSE);
        if ( nightCount < 0 || dayCount < 0){
            return 0;
        }
        fprintf(query3, "%d;%li;%li;%li\n", day, dayCount, nightCount, dayCount+nightCount);
    }
    return 1;
}


void errorExit (int errValue, char * errMessage, char * arg) {
    fprintf(stderr, "%s : %s\n", arg, errMessage);
    exit(errValue);
}

void closeExit (FILE * files[], int errValue, char * errMessage, char * arg, size_t fileCount, peatonesADT tad) {
    freePeatones(tad);
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
